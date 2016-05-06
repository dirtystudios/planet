#include "DX11RenderDevice.h"
#include "Log.h"
#include <d3dcompiler.h>
#include <d3dcompiler.inl>
#include "DX11ConstantBufferHelpers.h"
#include "xxhash.h"

#include <fstream>
#include "DX11Utils.h"

#include "File.h"

#define SafeGet(id, idx) id[idx];

namespace graphics {

#pragma region Create Commands

    IndexBufferHandle RenderDeviceDX11::CreateIndexBuffer(void* data, size_t size, BufferUsage usage) {
        ComPtr<ID3D11Buffer> indexBuffer = NULL;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = static_cast<UINT>(size);
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = { 0 };
        initData.pSysMem = data;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        DX11_CHECK_RET0(m_dev->CreateBuffer(&bufferDesc, &initData, &indexBuffer));
        uint32_t handle = GenerateHandle();
        IndexBufferDX11 ib;
        ib.indexBuffer = indexBuffer.Get();

        indexBuffer.Get()->AddRef();
        m_indexBuffers.emplace(handle, ib);
        return handle;
    }
    
    VertexBufferHandle RenderDeviceDX11::CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) {
        ComPtr<ID3D11Buffer> vertexBuffer;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = SafeGet(BufferUsageDX11, (uint32_t)usage);
        bufferDesc.ByteWidth = static_cast<UINT>(size);
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        if (bufferDesc.Usage == D3D11_USAGE_DYNAMIC)
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        else
            bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        if (data) {
            initData.pSysMem = data;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
        }

        DX11_CHECK_RET0(m_dev->CreateBuffer(&bufferDesc, data ? &initData : NULL, &vertexBuffer));
        uint32_t handle = GenerateHandle();
        VertexBufferDX11 vb = {};
        vb.vertexBuffer = vertexBuffer.Get();
        vb.layout = layout;

        vertexBuffer.Get()->AddRef();
        m_vertexBuffers.emplace(handle, vb);
        return handle;
    }

    ComPtr<ID3DBlob> RenderDeviceDX11::CompileShader(ShaderType shaderType, const std::string& source) {
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        char *entryPoint;
        char *target;

        HRESULT hr;


        // ---- Compile Shader Source 
        switch (shaderType) {
        case ShaderType::FRAGMENT_SHADER:
            entryPoint = "PSMain";
            target = "ps_5_0";
            break;
        case ShaderType::VERTEX_SHADER:
            entryPoint = "VSMain";
            target = "vs_5_0";
            break;
        default:
            LOG_E("DX11RenderDev: Unsupported shader type supplied. Type: %d", shaderType);
            return blob;
        }

        uint32_t flags = 0;
#ifdef DEBUG_DX11
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;
#endif 

        hr = D3DCompile(&source[0], source.length(), NULL, NULL, NULL, entryPoint, target, flags, 0, &blob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                LOG_E("DX11RenderDev: Failed to Compile Shader. Error: %s", errorBlob->GetBufferPointer());
            }
            else {
                LOG_E("DX11RenderDev: Failed to compile Shader. Hr: 0x%x", hr);
            }
            return blob;
        }
        return blob;
    }

    ShaderHandle RenderDeviceDX11::CreateShader(ShaderType shaderType, const std::string& source) {
        ShaderDX11 shader = {};
        uint32_t shaderHandle = 0;
        shader.shaderType = shaderType;

        uint32_t cBufferHandle = 0;
        uint32_t inputLayoutHandle = 0;

        ComPtr<ID3DBlob> blob;
        void* bufPtr;
        size_t bufSize;
        uint32_t shaderHash = 0;

        if (m_usePrebuiltShaders) {
            bufPtr = (void*)&source[0];
            bufSize = source.length();

            shaderHash = XXH32(&source[0], bufSize, 0);
            cBufferHandle = CreateConstantBuffer(shaderHash);
            if (shaderType == ShaderType::VERTEX_SHADER)
                inputLayoutHandle = CreateInputLayout(shaderHash, source);
        }
        else {
            blob.Swap(CompileShader(shaderType, source));

            cBufferHandle = CreateConstantBuffer(blob.Get());
            if (shaderType == ShaderType::VERTEX_SHADER)
                inputLayoutHandle = CreateInputLayout(blob.Get());

            bufPtr = blob->GetBufferPointer();
            bufSize = blob->GetBufferSize();
        }
        if (!bufPtr) {
            LOG_E("DX11RenderDev: bufPtr is 0 in CreateShader");
            return 0;
        }

        switch (shaderType) {
        case ShaderType::VERTEX_SHADER:
        {
            ComPtr<ID3D11VertexShader> vertexShader;
            DX11_CHECK_RET0(m_dev->CreateVertexShader(bufPtr, bufSize, NULL, &vertexShader));

            vertexShader.Get()->AddRef();
            shaderHandle = GenerateHandle();
            shader.vertexShader = vertexShader.Get();
            shader.cbHandle = cBufferHandle;
            shader.inputLayoutHandle = inputLayoutHandle;
            break;
        }
        case ShaderType::FRAGMENT_SHADER:
        {
            // todo: anything else for fragment shader
            ComPtr<ID3D11PixelShader> pixelShader;
            DX11_CHECK_RET0(m_dev->CreatePixelShader(bufPtr, bufSize, NULL, &pixelShader));

            pixelShader.Get()->AddRef();
            shaderHandle = GenerateHandle();
            shader.pixelShader = pixelShader.Get();
            shader.cbHandle = cBufferHandle;
            break;
        }
        // todo: the rest of the shaders
        default:
            LOG_E("DX11RenderDev: Invalid/Unimplemented Shader Type to compile.");
            return 0;
            break;
        }
        m_shaders.emplace(shaderHandle, shader);
        return shaderHandle;
    }

    uint32_t RenderDeviceDX11::CreateInputLayout(ID3DBlob *shader) {
        InputLayoutCacheHandle ilHandle = inputLayoutCache.InsertInputLayout(shader);

        if (!ilHandle) {
            LOG_D("DX11Render: Invalid or empty input layout defined in shader.");
            return 0;
        }

		//reuse previously created one if possible
		InputLayoutDX11* cachedInputLayout = Get(m_inputLayouts, ilHandle);
		if (cachedInputLayout) {
			return ilHandle;
		}

		ComPtr<ID3D11InputLayout> inputLayout;
        DX11_CHECK_RET0(m_dev->CreateInputLayout(inputLayoutCache.GetInputLayoutData(ilHandle),
            static_cast<UINT>(inputLayoutCache.GetInputLayoutSize(ilHandle)), shader->GetBufferPointer(), shader->GetBufferSize(), &inputLayout));

        inputLayout.Get()->AddRef();
        InputLayoutDX11 inputLayoutDx11;
        inputLayoutDx11.inputLayout = inputLayout.Get();
        m_inputLayouts.emplace(ilHandle, inputLayoutDx11);
        return ilHandle;
    }

    uint32_t RenderDeviceDX11::CreateInputLayout(uint32_t shaderHash, const std::string& byteCode) {
        std::ifstream ilContents(fs::AppendPathProcessDir("shaders/DX11Prebuilt/" + std::to_string(shaderHash) + ".il"));
        if (ilContents.fail()) {
            LOG_D("DX11Render-CreateInputLayout-Prebuilt Failed to open file");
            return 0;
        }

        DX11InputLayoutDescriptor ild;
        if (ilContents >> ild) {
            InputLayoutCacheHandle ilHandle = inputLayoutCache.InsertInputLayout(ild);
            if (!ilHandle) {
                LOG_D("DX11Render: Invalid or empty input layout defined in shader.");
                return 0;
            }

            //reuse previously created one if possible
            InputLayoutDX11* cachedInputLayout = Get(m_inputLayouts, ilHandle);
            if (cachedInputLayout) {
                return ilHandle;
            }

            ComPtr<ID3D11InputLayout> inputLayout;
            DX11_CHECK_RET0(m_dev->CreateInputLayout(inputLayoutCache.GetInputLayoutData(ilHandle), 
                static_cast<UINT>(inputLayoutCache.GetInputLayoutSize(ilHandle)), &byteCode[0], byteCode.length(), &inputLayout));

            inputLayout.Get()->AddRef();
            InputLayoutDX11 inputLayoutDx11;
            inputLayoutDx11.inputLayout = inputLayout.Get();
            m_inputLayouts.emplace(ilHandle, inputLayoutDx11);
            return ilHandle;
        }
        LOG_E("DX11RenderDev-CreateInputLayout-Prebuilt Failed!");
        return 0;
    }

    uint32_t RenderDeviceDX11::CreateConstantBuffer(uint32_t shaderHash) {
        std::ifstream cbContents(fs::AppendPathProcessDir("shaders/DX11Prebuilt/" + std::to_string(shaderHash) + ".cb"));
        if (cbContents.fail()) {
            LOG_D("DX11Render-CreateConstantBuffer-Prebuilt Failed to open file");
            return 0;
        }
        //todo, support multiple cbuffers
        CBufferDescriptor *cbd = new CBufferDescriptor();
        if (cbContents >> *cbd) {
            std::vector<ID3D11Buffer*> constantBuffers;

            ID3D11Buffer* cBuffer;
            D3D11_BUFFER_DESC cbDesc;
            cbDesc.ByteWidth = static_cast<UINT>(cbd->totalSize);
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;
            DX11_CHECK_RET0(m_dev->CreateBuffer(&cbDesc, NULL, &cBuffer));
            constantBuffers.emplace_back(cBuffer);

            uint32_t handle = GenerateHandle();
            ConstantBufferDX11 cb = {};
            cb.cBufferDescs.assign(&cbd, &cbd + 1);
            cb.constantBuffers = constantBuffers;
            m_constantBuffers.emplace(handle, cb);

            return handle;
        }
        LOG_E("DX11RenderDev-CreateConstantBuffer-Prebuilt Failed!");
        return 0;
    }

    uint32_t RenderDeviceDX11::CreateConstantBuffer(ID3DBlob *shaderBlob) {
        std::vector<ID3D11Buffer*> constantBuffers;

        size_t numCBuffers = 0;
        CBufferDescriptor* cBuffers = graphics::dx11::GenerateConstantBuffer(shaderBlob, &numCBuffers);

        for (uint32_t x = 0; x < numCBuffers; ++x) {

            ID3D11Buffer* cBuffer;
            D3D11_BUFFER_DESC cbDesc;
            cbDesc.ByteWidth = static_cast<UINT>(cBuffers[x].totalSize);
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;

            DX11_CHECK_RET0(m_dev->CreateBuffer(&cbDesc, NULL, &cBuffer));
            if (!cBuffer) {
                for (ID3D11Buffer* pcBuffer : constantBuffers) {
                    pcBuffer->Release();
                }
                delete[] cBuffers;
                return 0;
            }
            constantBuffers.emplace_back(cBuffer);
        }
        uint32_t handle = GenerateHandle();
        ConstantBufferDX11 cb = {};
        cb.cBufferDescs.assign(&cBuffers, &cBuffers + numCBuffers);
        cb.constantBuffers = constantBuffers;
        m_constantBuffers.emplace(handle, cb);

        return handle;
    }

    TextureHandle RenderDeviceDX11::CreateTexture2D(TextureFormat texFormat, uint32_t width, uint32_t height, void* data) {
        DXGI_FORMAT dxFormat = SafeGet(TextureFormatDX11, (uint32_t)texFormat);
        D3D11_TEXTURE2D_DESC tdesc = { 0 };
        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = 1;
        tdesc.ArraySize = 1;
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = 0;
        tdesc.Format = dxFormat;

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipLevels = 1;
        viewDesc.Texture2D.MostDetailedMip = 0;

        return Texture2DCreator(&tdesc, texFormat, &viewDesc, data);
    }

    TextureHandle RenderDeviceDX11::CreateTextureCube(TextureFormat texFormat, uint32_t width, uint32_t height, void** data) {
        DXGI_FORMAT dxFormat = SafeGet(TextureFormatDX11, (uint32_t)texFormat);
        D3D11_TEXTURE2D_DESC tdesc = { 0 };
        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = 1;
        tdesc.ArraySize = 6;
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        tdesc.Format = dxFormat;

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        viewDesc.TextureCube.MipLevels = tdesc.MipLevels;
        viewDesc.TextureCube.MostDetailedMip = 0;
        

        return Texture2DCreator(&tdesc, texFormat, &viewDesc, data);
    }

    TextureHandle RenderDeviceDX11::CreateTextureArray(TextureFormat texFormat, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) {
        DXGI_FORMAT dxFormat = SafeGet(TextureFormatDX11, (uint32_t)texFormat);
        D3D11_TEXTURE2D_DESC tdesc = { 0 };
        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = /*Avicii - */levels;
        tdesc.ArraySize = depth;
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = 0;
        tdesc.Format = dxFormat;

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        viewDesc.Texture2DArray.MipLevels = levels;
        viewDesc.Texture2DArray.FirstArraySlice = 0;
        viewDesc.Texture2DArray.ArraySize = depth;

        return Texture2DCreator(&tdesc, texFormat, &viewDesc, 0);
    }

    TextureHandle RenderDeviceDX11::Texture2DCreator(D3D11_TEXTURE2D_DESC* tDesc, TextureFormat texFormat, D3D11_SHADER_RESOURCE_VIEW_DESC* viewDesc, void* data) {
        // todo: try not to use m/calloc?
        ComPtr<ID3D11Texture2D> texture;
        D3D11_SUBRESOURCE_DATA srd[6];

        void* newData = 0;
        uint32_t *cubeData[6];
        if (texFormat == TextureFormat::RGB_UBYTE) {
            if (tDesc->MiscFlags == D3D11_RESOURCE_MISC_TEXTURECUBE) {
                uint32_t numPixels = tDesc->Width * tDesc->Height;
                for (int x = 0; x < 6; ++x) {
                    cubeData[x] = (uint32_t *)calloc(numPixels, 4);
                    void** dataCast = (void**)data;
                    Convert24BitTo32Bit(
                        reinterpret_cast<uintptr_t>(dataCast[x]), reinterpret_cast<uintptr_t>(cubeData[x]), numPixels);
                }
                newData = &cubeData[0];
            }
        }
        if (data) {
            if (tDesc->MiscFlags == D3D11_RESOURCE_MISC_TEXTURECUBE) {
                for (int x = 0; x < 6; ++x) {
                    char** tempCast = (char**)(newData ? newData : data);
                    srd[x].pSysMem = tempCast[x];
                    srd[x].SysMemPitch = GetFormatByteSize(tDesc->Format) * tDesc->Width;
                    srd[x].SysMemSlicePitch = 0;
                }
            }
            else {
                srd[0].pSysMem = newData ? newData : data;
                srd[0].SysMemPitch = GetFormatByteSize(tDesc->Format) * tDesc->Width;
                srd[0].SysMemSlicePitch = 0;
            }
        }
        DX11_CHECK(m_dev->CreateTexture2D(tDesc, data ? srd : NULL, &texture));
        if (newData) {
            if (tDesc->MiscFlags == D3D11_RESOURCE_MISC_TEXTURECUBE) {
                for (int x = 0; x < 6; ++x) {
                    void** dataCast = (void**)newData;
                    free(dataCast[x]);
                }
            }
        }
        if (!texture)
            return 0;


        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK(m_dev->CreateShaderResourceView(texture.Get(), viewDesc, &shaderResourceView));
        if (!shaderResourceView) {
            texture.Reset();
            return 0;
        }

        uint32_t handle = GenerateHandle();
        texture.Get()->AddRef();
        shaderResourceView.Get()->AddRef();
        TextureDX11 textureDX11 = {};
        textureDX11.texture = texture.Get();
        textureDX11.shaderResourceView = shaderResourceView.Get();
        textureDX11.format = tDesc->Format;
        textureDX11.requestedFormat = texFormat;
        m_textures.emplace(handle, textureDX11);
        return handle;
    }
#pragma endregion

#pragma region Destroy Commands

    void RenderDeviceDX11::DestroyIndexBuffer(IndexBufferHandle handle) {
        IndexBufferDX11* indexBuff = Get(m_indexBuffers, handle);
        if (!indexBuff) {
            LOG_D("DX11RenderDev: Failed to DestroyIndexBuffer: %d", handle);
            return;
        }
        if (indexBuff->indexBuffer)
            indexBuff->indexBuffer->Release();
        m_indexBuffers.erase(handle);
    }

    void RenderDeviceDX11::DestroyVertexBuffer(VertexBufferHandle handle) {
        VertexBufferDX11* vertexBuff = Get(m_vertexBuffers, handle);
        if (!vertexBuff) {
            LOG_D("DX11RenderDev: Failed to DestroyVertexBuffer: %d", handle);
            return;
        }
        if (vertexBuff->vertexBuffer)
            vertexBuff->vertexBuffer->Release();
        m_vertexBuffers.erase(handle);
    }

    void RenderDeviceDX11::DestroyShader(ShaderHandle handle) {
        ShaderDX11* shader = Get(m_shaders, handle);
        if (!shader) {
            LOG_D("DX11RenderDev: Invalid handle given to DestroyShader");
            return;
        }

        if (shader->inputLayoutHandle)
            DestroyInputLayout(shader->inputLayoutHandle);
        
        if (shader->cbHandle)
            DestroyConstantBuffer(shader->cbHandle);
    
        if (shader->pixelShader)
            shader->pixelShader->Release();
       
        if (shader->vertexShader)
            shader->vertexShader->Release();
      
        //todo: samplers
        m_shaders.erase(handle);
    }

    void RenderDeviceDX11::DestroyConstantBuffer(ConstantBufferHandle handle) {
        ConstantBufferDX11* cBuffer = Get(m_constantBuffers, handle);
        if (!cBuffer) {
            LOG_D("DX11RenderDev: Invalid handle given to DestroyShader");
            return;
        }
        for (auto pcBuffer : cBuffer->constantBuffers) {
            pcBuffer->Release();
        }
        cBuffer->cBufferDescs.clear();
        m_constantBuffers.erase(handle);
    }

    void RenderDeviceDX11::DestroyInputLayout(uint32_t handle) {
        InputLayoutDX11* inputLayout = Get(m_inputLayouts, handle);
        if (!inputLayout) {
            LOG_D("DX11RenderDev: Invalid handle given to DestroyInputLayout");
            return;
        }
        if (inputLayout->inputLayout)
            inputLayout->inputLayout->Release();
        m_inputLayouts.erase(handle);
        inputLayoutCache.RemoveInputLayout(handle);
    }

    void RenderDeviceDX11::DestroyTexture(TextureHandle handle) {
        TextureDX11* texture = Get(m_textures, handle);
        if (!texture) {
            LOG_D("DX11RenderDev: Invalid handle given to DestroyTexture.");
            return;
        }

        if (texture->texture)
            texture->texture->Release();
        if (texture->shaderResourceView)
            texture->shaderResourceView->Release();

        m_textures.erase(handle);
    }

#pragma endregion

    void RenderDeviceDX11::SetIndexBuffer(IndexBufferHandle handle) {
        if (m_currentState.indexBufferHandle == handle) {
            m_pendingState.indexBufferHandle = m_currentState.indexBufferHandle;
            m_pendingState.indexBuffer = m_currentState.indexBuffer;
            return;
        }

        IndexBufferDX11* indexBuff = Get(m_indexBuffers, handle);
        if (!indexBuff) {
            LOG_E("DX11RenderDev: Invalid Handle given to SetIndexBuffer.");
            return;
        }
        m_pendingState.indexBufferHandle = handle;
        m_pendingState.indexBuffer = indexBuff;
    }

    void RenderDeviceDX11::SetVertexBuffer(VertexBufferHandle handle) {
        if (m_currentState.vertexBufferHandle == handle) {
            m_pendingState.vertexBufferHandle = m_currentState.vertexBufferHandle;
            m_pendingState.vertexBuffer = m_currentState.vertexBuffer;
            return;
        }

        VertexBufferDX11* vb = Get(m_vertexBuffers, handle);
        if (!vb) {
            LOG_E("DX11RenderDev: Invalid Handle given to SetVertexBuffer.");
            return;
        }

        m_pendingState.vertexBufferHandle = handle;
        m_pendingState.vertexBuffer = vb;
    }

    void RenderDeviceDX11::SetVertexShader(ShaderHandle shaderHandle) {
        if (m_currentState.vertexShaderHandle == shaderHandle) {
            m_pendingState.vertexShaderHandle = m_currentState.vertexShaderHandle;
            m_pendingState.vertexShader = m_currentState.vertexShader;
            return;
        }

        ShaderDX11* shader = Get(m_shaders, shaderHandle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid Shader handle given to SetVertexShader");
            return;
        }
        if (shader->shaderType != ShaderType::VERTEX_SHADER) {
            LOG_E("DX11RenderDev: Non Vertex Shader given to SetVertexShader.");
            return;
        }

        m_pendingState.vertexShaderHandle = shaderHandle;
        m_pendingState.vertexShader = shader;

        // Input Layouts are checked, as they can be the same between shaders
        if (shader->inputLayoutHandle) {
            SetInputLayout(shader->inputLayoutHandle);
        }

        // CBuffers are unique per shader, no other way. So they get set as long as they exist.
        ConstantBufferDX11 *cb = Get(m_constantBuffers, shader->cbHandle);
        if (cb) {
            m_pendingState.vsCBuffer = cb;
        }

        //samplers
    }

    void RenderDeviceDX11::SetPixelShader(ShaderHandle shaderHandle) {
        if (m_currentState.pixelShaderHandle == shaderHandle) {
            m_pendingState.pixelShaderHandle = m_currentState.pixelShaderHandle;
            m_pendingState.pixelShader = m_currentState.pixelShader;
            return;
        }

        ShaderDX11* shader = Get(m_shaders, shaderHandle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid Shader handle given to SetPixelShader");
            return;
        }
        if (shader->shaderType != ShaderType::FRAGMENT_SHADER) {
            LOG_E("DX11RenderDev: Non Pixel Shader given to SetPixelShader.");
            return;
        }

        m_pendingState.pixelShaderHandle = shaderHandle;
        m_pendingState.pixelShader = shader;

        // CBuffers are unique per shader, no other way. So they get set as long as they exist.
        ConstantBufferDX11 *cb = Get(m_constantBuffers, shader->cbHandle);
        if (cb) {
            m_pendingState.psCBuffer = cb;
        }

        //todo: samplers, etc...
    }

    void RenderDeviceDX11::SetInputLayout(uint32_t inputLayoutHandle) {
        if (m_currentState.inputLayoutHandle == inputLayoutHandle) {
            m_pendingState.inputLayoutHandle = inputLayoutHandle;
            m_pendingState.inputLayout = m_currentState.inputLayout;
            return;
        }

        InputLayoutDX11* inputLayout = Get(m_inputLayouts, inputLayoutHandle);
        if (!inputLayout) {
            LOG_E("DX11RenderDev: Invalid handle give to SetInputLayout.");
            return;
        }
        m_pendingState.inputLayoutHandle = inputLayoutHandle;
        m_pendingState.inputLayout = inputLayout;
    }

    void RenderDeviceDX11::Clear(float r, float g, float b, float a) {
        float RGBA[4] = { r, g, b, a };
        m_devcon->ClearRenderTargetView(m_renderTarget.Get(), RGBA);
        m_devcon->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void RenderDeviceDX11::SwapBuffers() {
        DX11_CHECK(m_swapchain->Present(1, 0));
    }

    void RenderDeviceDX11::SetShaderParameter(ShaderHandle handle, ParamType paramType, const char *paramName, void *data) {
        // todo: here we are just updating 'cache' 
        // hopefully this is fast enough, bleh
        ShaderDX11 *shader = Get(m_shaders, handle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid handle given to SetShaderParameter.");
            return;
        }

        ConstantBufferDX11 *cBuffer = Get(m_constantBuffers, shader->cbHandle);
        if (!cBuffer) {
            LOG_E("DX11RenderDev: No Cbuffer set for shader in SetShaderParameter.");
            return;
        }

        CBufferDescriptor *cBufferToUpdate = 0;
        CBufferVariable *cBufVarToUpdate = 0;
        uint32_t slot;
        for (slot = 0; slot < cBuffer->cBufferDescs.size(); ++slot) {
            auto it = cBuffer->cBufferDescs[slot]->details.find(paramName);
            if (it == cBuffer->cBufferDescs[slot]->details.end()) {
                continue;
            }
            cBufferToUpdate = cBuffer->cBufferDescs[slot];
            cBufVarToUpdate = &it->second;
            break;
        }

        if (!cBufferToUpdate || !cBufVarToUpdate) {
            LOG_E("DX11RenderDev: Unknown paramName given to SetShaderParameter. Got: %s", paramName);
            return;
        }
        if (cBufVarToUpdate->size != SizeofParam(paramType)) {
            LOG_E("DX11RenderDev: Size Mismatch in SetShaderParameter. Expected %d. Got %d.", cBufVarToUpdate->size, SizeofParam(paramType));
            return;
        }
        cBufferToUpdate->UpdateBufferData(cBufVarToUpdate, data);
        
        switch (shader->shaderType) {
        case ShaderType::VERTEX_SHADER:
            if (std::find(m_pendingState.vsCBufferDirtySlots.begin(), m_pendingState.vsCBufferDirtySlots.end(), slot) == m_pendingState.vsCBufferDirtySlots.end()) {
                m_pendingState.vsCBufferDirtySlots.push_back(slot);
            }
            break;
        case ShaderType::FRAGMENT_SHADER:
            if (std::find(m_pendingState.psCBufferDirtySlots.begin(), m_pendingState.psCBufferDirtySlots.end(), slot) == m_pendingState.psCBufferDirtySlots.end()) {
                m_pendingState.psCBufferDirtySlots.push_back(slot);
            }

            break;
        default:
            LOG_E("DX11RenderDev: Unsupported ShaderType for update in SetShaderParam.")
            break;
        }
    }

    void RenderDeviceDX11::UpdateConstantBuffer(ConstantBufferDX11* cb, std::vector<uint32_t> dirtySlots) {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        
        for (uint32_t x = 0; x < dirtySlots.size(); ++x) {
            uint32_t slot = dirtySlots[x];

            DX11_CHECK_RET(m_devcon->Map(cb->constantBuffers[slot], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

            memcpy(mappedResource.pData, cb->cBufferDescs[slot]->bufferData, cb->cBufferDescs[slot]->totalSize );

            m_devcon->Unmap(cb->constantBuffers[slot], 0);
        }
    }
    
    void RenderDeviceDX11::UpdateTexture(TextureHandle handle, void* data, size_t size) {
        LOG_E("DX11RenderDev: Unimplemented Function UpdateTexture");
    }

    void RenderDeviceDX11::UpdateTextureArray(TextureHandle handle, uint32_t arrayIndex, uint32_t width, uint32_t height, void* data) {
        TextureDX11 *texture = Get(m_textures, handle);
        if (!texture){
            LOG_E("DX11RenderDev: Invalid handle given to UpdateTextureArray.");
            return;
        }

        D3D11_BOX box = { 0 };
        box.left = 0;
        box.top = 0;
        box.front = 0;
        box.back = 1;
        box.right = width;
        box.bottom = height;

        int formatByteSize = GetFormatByteSize(texture->format);
        if (formatByteSize == 0) {
            LOG_E("DX11RenderDev: Unsupported TextureFormat for UpdateTextureArray.");
            return;
        }
        if (texture->requestedFormat == TextureFormat::RGB_UBYTE) {
            LOG_E("DX11RenderDev: Unsupported TextureFormat to Update.")
        }
        m_devcon->UpdateSubresource(texture->texture, D3D11CalcSubresource(0, arrayIndex, 1), &box, data, width * formatByteSize, width * formatByteSize * height);
    }

    void RenderDeviceDX11::SetRasterState(const RasterState& rasterState) {
        uint32_t hash = XXH32(&rasterState, sizeof(RasterState), 0);

        if (m_currentState.rasterStateHash == hash) {
            m_pendingState.rasterStateHash = hash;
            m_pendingState.rasterState = m_currentState.rasterState;
            return;
        }

        RasterStateDX11 *rasterStateCheck = Get(m_rasterStates, hash);
        if (rasterStateCheck) {
            m_pendingState.rasterStateHash = hash;
            m_pendingState.rasterState = rasterStateCheck;
            return;
        }

        D3D11_RASTERIZER_DESC rasterDesc;
        rasterDesc.AntialiasedLineEnable = true;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.MultisampleEnable = true;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        rasterDesc.CullMode = SafeGet(CullModeDX11, (uint32_t)rasterState.cull_mode);
        rasterDesc.FillMode = SafeGet(FillModeDX11, (uint32_t)rasterState.fill_mode);
        rasterDesc.FrontCounterClockwise = rasterState.winding_order == WindingOrder::FRONT_CCW ? true : false;

        ComPtr<ID3D11RasterizerState> rasterStateDX;
        DX11_CHECK_RET(m_dev->CreateRasterizerState(&rasterDesc, &rasterStateDX));

        RasterStateDX11 rasterStateDX11 = {};
        rasterStateDX11.rasterState = rasterStateDX.Get();
        auto it = m_rasterStates.emplace(hash, rasterStateDX11);
        rasterStateDX.Get()->AddRef();

        m_pendingState.rasterState = &it.first->second;
        m_pendingState.rasterStateHash = hash;
    }

    void RenderDeviceDX11::SetDepthState(const DepthState& depthState) {
        uint32_t hash = XXH32(&depthState, sizeof(DepthState), 0);
        if (m_currentState.depthStateHash == hash) {
            m_pendingState.depthStateHash = hash;
            m_pendingState.depthState = m_currentState.depthState;
            return;
        }

        DepthStateDX11* depthStateCheck = Get(m_depthStates, hash);
        if (depthStateCheck) {
            m_pendingState.depthStateHash= hash;
            m_pendingState.depthState= depthStateCheck;
            return;
        }

        //todo: what to do about all these other options?
        D3D11_DEPTH_STENCIL_DESC dsDesc;
        dsDesc.StencilReadMask = 0xFF;
        dsDesc.StencilWriteMask = 0xFF;
        dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        dsDesc.DepthEnable = depthState.enable;
        dsDesc.StencilEnable = depthState.enable;
        dsDesc.DepthWriteMask = SafeGet(DepthWriteMaskDX11, (uint32_t)depthState.depth_write_mask);
        dsDesc.DepthFunc = SafeGet(DepthFuncDX11, (uint32_t)depthState.depth_func);

        ComPtr<ID3D11DepthStencilState> depthStateDX;
        DX11_CHECK_RET(m_dev->CreateDepthStencilState(&dsDesc, &depthStateDX));

        DepthStateDX11 depthStateDX11= {};
        depthStateDX11.depthState = depthStateDX.Get();
        auto it = m_depthStates.emplace(hash, depthStateDX11);
        depthStateDX.Get()->AddRef();

        m_pendingState.depthState = &it.first->second;
        m_pendingState.depthStateHash = hash;
    }

    void RenderDeviceDX11::SetBlendState(const BlendState& blendState) {
        uint32_t hash = XXH32(&blendState, sizeof(BlendState), 0);
        if (m_currentState.blendStateHash == hash) {
            m_pendingState.blendStateHash = hash;
            m_pendingState.blendState = m_currentState.blendState;
            return;
        }

        BlendStateDX11 *blendStateCheck = Get(m_blendStates, hash);
        if (blendStateCheck) {
            m_pendingState.blendStateHash = hash;
            m_pendingState.blendState = blendStateCheck;
            return;
        }

        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.IndependentBlendEnable = false;

        blendDesc.RenderTarget[0].BlendEnable = (blendState.enable) ? TRUE : FALSE;
        blendDesc.RenderTarget[0].SrcBlendAlpha = SafeGet(BlendFuncDX11, (uint32_t)blendState.src_alpha_func);
        blendDesc.RenderTarget[0].DestBlendAlpha = SafeGet(BlendFuncDX11, (uint32_t)blendState.dst_alpha_func);
        blendDesc.RenderTarget[0].SrcBlend = SafeGet(BlendFuncDX11, (uint32_t)blendState.src_rgb_func);
        blendDesc.RenderTarget[0].DestBlend = SafeGet(BlendFuncDX11, (uint32_t)blendState.dst_rgb_func);
        blendDesc.RenderTarget[0].BlendOpAlpha = SafeGet(BlendModeDX11, (uint32_t)blendState.alpha_mode);
        blendDesc.RenderTarget[0].BlendOp = SafeGet(BlendModeDX11, (uint32_t)blendState.rgb_mode);

        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        ComPtr<ID3D11BlendState> blendStateDX;
        DX11_CHECK_RET(m_dev->CreateBlendState(&blendDesc, &blendStateDX));

        BlendStateDX11 blendStateDX11 = {};
        blendStateDX11.blendState = blendStateDX.Get();
        auto it = m_blendStates.emplace(hash, blendStateDX11);
        blendStateDX.Get()->AddRef();

        m_pendingState.blendState = &it.first->second;
        m_pendingState.blendStateHash = hash;
    }

    void RenderDeviceDX11::UpdateVertexBuffer(VertexBufferHandle vertexBufferHandle, void* data, size_t size) {
        VertexBufferDX11* vBuffer = Get(m_vertexBuffers, vertexBufferHandle);
        if (!vBuffer) {
            LOG_E("DX11RenderDev: Invalid VertexBufferHandle given to UpdateVertexBuffer.");
            return;
        }
        D3D11_BUFFER_DESC bufferDesc;
        vBuffer->vertexBuffer->GetDesc(&bufferDesc);
        if (bufferDesc.Usage != D3D11_USAGE_DYNAMIC) {
            LOG_E("DX11RenderDev: Only dynamic buffer type updating allowed.");
            return;
        }

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DX11_CHECK_RET(m_devcon->Map(vBuffer->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        memcpy(mappedResource.pData, data, size);

        m_devcon->Unmap(vBuffer->vertexBuffer, 0);

    }
    //todo...batch these calls!
    void RenderDeviceDX11::SetShaderTexture(ShaderHandle shaderHandle, TextureHandle textureHandle, TextureSlot slot) {
        ShaderDX11* shader = Get(m_shaders, shaderHandle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid ShaderHandle given to SetShaderTexture.");
            return;
        }
        TextureDX11* texture = Get(m_textures, textureHandle);
        if (!texture) {
            LOG_E("DX11RenderDev: Invalid TextureHandle give to SetShaderTexture.");
            return;
        }

        switch (shader->shaderType) {
        case ShaderType::FRAGMENT_SHADER:
        {
            auto it = m_currentState.psTextures.find((uint32_t)slot);
            if (it == m_currentState.psTextures.end()) {
                m_pendingState.psTextures.emplace((uint32_t)slot, texture->shaderResourceView);
                m_pendingState.psDirtyTextureSlots.emplace_back((uint32_t)slot);
            }
            else if (it->second != texture->shaderResourceView) {
                m_pendingState.psTextures.at((uint32_t)slot) = texture->shaderResourceView;
                m_pendingState.psDirtyTextureSlots.emplace_back((uint32_t)slot);
            }
        }
            break;
        case ShaderType::VERTEX_SHADER:
        {
            auto it = m_currentState.vsTextures.find((uint32_t)slot);
            if (it == m_currentState.vsTextures.end()) {
                m_pendingState.vsTextures.emplace((uint32_t)slot, texture->shaderResourceView);
                m_pendingState.vsDirtyTextureSlots.emplace_back((uint32_t)slot);
            }
            else if (it->second != texture->shaderResourceView) {
                m_pendingState.vsTextures.at((uint32_t)slot) = texture->shaderResourceView;
                m_pendingState.vsDirtyTextureSlots.emplace_back((uint32_t)slot);
            }
        }
            break;
        default:
            LOG_E("DX11RenderDev: Invalid ShaderType given to SetShaderTexture.");
            break;
        }

        //This goes here for now
        SetSampler(1, shaderHandle, (uint32_t)slot);
    }

    void RenderDeviceDX11::DrawPrimitive(PrimitiveType primitiveType, uint32_t startVertex, uint32_t numVertices) {

        if (m_pendingState.pixelShaderHandle != 0 && m_currentState.pixelShaderHandle != m_pendingState.pixelShaderHandle) {
            m_devcon->PSSetShader(m_pendingState.pixelShader->pixelShader, 0, 0);
            if (m_pendingState.psCBuffer)
                m_devcon->PSSetConstantBuffers(
                    0, static_cast<UINT>(m_pendingState.psCBuffer->constantBuffers.size()), m_pendingState.psCBuffer->constantBuffers.data());
        }

        if (m_pendingState.vertexShaderHandle != 0 && m_currentState.vertexShaderHandle != m_pendingState.vertexShaderHandle) {
            m_devcon->VSSetShader(m_pendingState.vertexShader->vertexShader, 0, 0);
            if (m_pendingState.vsCBuffer)
                m_devcon->VSSetConstantBuffers(
                    0, static_cast<UINT>(m_pendingState.vsCBuffer->constantBuffers.size()), m_pendingState.vsCBuffer->constantBuffers.data());
        }

        if (m_pendingState.vertexShaderHandle != 0 && m_pendingState.vsCBuffer)
            UpdateConstantBuffer(m_pendingState.vsCBuffer, m_pendingState.vsCBufferDirtySlots);

        if (m_pendingState.pixelShaderHandle != 0 && m_pendingState.psCBuffer)
            UpdateConstantBuffer(m_pendingState.psCBuffer, m_pendingState.psCBufferDirtySlots);

        if (m_pendingState.inputLayoutHandle != 0 && m_currentState.inputLayoutHandle != m_pendingState.inputLayoutHandle)
            m_devcon->IASetInputLayout(m_pendingState.inputLayout->inputLayout);

        if (m_pendingState.vertexBufferHandle != 0 && m_currentState.vertexBufferHandle != m_pendingState.vertexBufferHandle) {
            uint32_t offset = 0;
            uint32_t stride = static_cast<UINT>(m_pendingState.vertexBuffer->layout.stride);
            m_devcon->IASetVertexBuffers(0, 1, &m_pendingState.vertexBuffer->vertexBuffer, &stride, &offset);
        }

        if (m_pendingState.vsDirtyTextureSlots.size()) {
            // todo: batch these calls
            for (uint32_t x = 0; x < m_pendingState.vsDirtyTextureSlots.size(); ++x) {
                m_devcon->VSSetShaderResources((uint32_t)m_pendingState.vsDirtyTextureSlots[x], 1, &m_pendingState.vsTextures.at(x));
            }
        }

        if (m_pendingState.psDirtyTextureSlots.size()) {
            for (uint32_t x = 0; x < m_pendingState.psDirtyTextureSlots.size(); ++x) {
                m_devcon->PSSetShaderResources((uint32_t)m_pendingState.psDirtyTextureSlots[x], 1, &m_pendingState.psTextures.at(x));
            }
        }

        // todo: format?
        if (m_pendingState.indexBufferHandle != 0 && m_currentState.indexBufferHandle != m_pendingState.indexBufferHandle)
            m_devcon->IASetIndexBuffer(m_pendingState.indexBuffer->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        D3D11_PRIMITIVE_TOPOLOGY type = SafeGet(PrimitiveTypeDX11, (uint32_t)primitiveType);
        m_pendingState.primitiveType = type;

        if (m_currentState.primitiveType != type)
            m_devcon->IASetPrimitiveTopology(type);

        if (m_pendingState.blendStateHash != 0 && m_currentState.blendStateHash != m_pendingState.blendStateHash) {
            float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = 0xffffffff;
            m_devcon->OMSetBlendState(m_pendingState.blendState->blendState, blendFactor, sampleMask);
        }

        if (m_pendingState.rasterStateHash != 0 && m_currentState.rasterStateHash != m_pendingState.rasterStateHash)
            m_devcon->RSSetState(m_pendingState.rasterState->rasterState);

        if (m_pendingState.depthStateHash != 0 && m_currentState.depthStateHash != m_pendingState.depthStateHash)
            m_devcon->OMSetDepthStencilState(m_pendingState.depthState->depthState, 1);

        // We use SWAP_FLIP_SEQUENTIAL with 11.3, which apparently needs this call everytime....ugh
#ifdef DX11_3_API
        m_devcon->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());
#endif

        m_devcon->Draw(numVertices, startVertex);
        
        //cleanup state before set
        m_pendingState.psCBufferDirtySlots.clear();
        m_pendingState.psDirtyTextureSlots.clear();
        m_pendingState.vsCBufferDirtySlots.clear();
        m_pendingState.vsDirtyTextureSlots.clear();
        m_currentState = m_pendingState;
    }

    // Currently ignoreing samplerhandle, just using default
    void RenderDeviceDX11::SetSampler(SamplerHandle samplerHandle, ShaderHandle shaderHandle, uint32_t location) {
        // TODO-Jake: FIXME
        SamplerDX11 *sampler = Get(m_samplers, defaultSamplerHandle);
        if (!sampler) {
            LOG_E("DX11RenderDev: Invalid handle given to SetSampler.");
        }

        ShaderDX11* shader = Get(m_shaders, shaderHandle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid ShaderHandle given to SetSampler.");
            return;
        }
        switch (shader->shaderType) {
        case ShaderType::VERTEX_SHADER:
            m_devcon->VSSetSamplers(location, 1, &sampler->sampler);
            break;
        case ShaderType::FRAGMENT_SHADER:
            m_devcon->PSSetSamplers(location, 1, &sampler->sampler);
            break;
        default:
            LOG_E("DX11RenderDev: Invalid ShaderType given to SetSampler.");
            break;
        }
    }

    // Just call this in initialize to get the default made to use
    SamplerHandle RenderDeviceDX11::CreateSampler() {
        D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
        //samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        ID3D11SamplerState* samplerState;
        DX11_CHECK_RET0(m_dev->CreateSamplerState(&samplerDesc, &samplerState));

        uint32_t handle = GenerateHandle();
        SamplerDX11 samplerDX11 = {};
        samplerDX11.sampler = samplerState;//samplerState.Get();
        m_samplers.emplace(handle, samplerDX11);

        //hack for now
        defaultSamplerHandle = handle;

        return handle;
    }

    void RenderDeviceDX11::DestroySampler(SamplerHandle handle) {
        SamplerDX11* sampler = Get(m_samplers, handle);
        if (!sampler) {
            LOG_E("DX11RenderDev: Invalid handle given to DestroySampler.");
            return;
        }
        sampler->sampler->Release();
        m_samplers.erase(handle);
    }

    void RenderDeviceDX11::ResetDepthStencilTexture() {
        ComPtr<ID3D11Texture2D> depthTex;
        D3D11_TEXTURE2D_DESC descDepth;
        descDepth.Width = m_winWidth;
        descDepth.Height = m_winHeight;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        DX11_CHECK_RET(m_dev->CreateTexture2D(&descDepth, NULL, &depthTex));

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        descDSV.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        descDSV.Flags = 0;
        DX11_CHECK_RET(m_dev->CreateDepthStencilView(depthTex.Get(), &descDSV, &m_depthStencilView));
    }

    void RenderDeviceDX11::ResetViewport() {
        D3D11_VIEWPORT vp;
        vp.Width = (float)m_winWidth;
        vp.Height = (float)m_winHeight;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_devcon->RSSetViewports(1, &vp);
    }

    void RenderDeviceDX11::ResizeWindow(uint32_t width, uint32_t height) {
        if (m_winWidth == width && m_winHeight == height)
            return; 

        m_winWidth = width;
        m_winHeight = height;

        m_renderTarget.Reset();

        DX11_CHECK_RET(m_swapchain->ResizeBuffers(2, width, height, DXGI_FORMAT_UNKNOWN, 0));
        ComPtr<ID3D11Texture2D> backBufferPtr;
        DX11_CHECK_RET(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBufferPtr)));
        DX11_CHECK_RET(m_dev->CreateRenderTargetView(backBufferPtr.Get(), 0, m_renderTarget.GetAddressOf()));
        ResetDepthStencilTexture();
        ResetViewport();
        m_devcon->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());
    }

    int RenderDeviceDX11::InitializeDevice(DeviceInitialization deviceInitialization) {
        m_winWidth = deviceInitialization.windowWidth;
        m_winHeight = deviceInitialization.windowHeight;
        m_usePrebuiltShaders = deviceInitialization.usePrebuiltShaders;

        if (m_usePrebuiltShaders) {
            DeviceConfig.DeviceAbbreviation = "DX11Prebuilt";
            DeviceConfig.ShaderExtension = ".cso";
        }
        else {
            DeviceConfig.DeviceAbbreviation = "DX11";
            DeviceConfig.ShaderExtension = ".hlsl";
        }

        D3D_FEATURE_LEVEL FeatureLevelsRequested[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0, // cool xbox
        };

        uint32_t numLevelsRequested = ARRAYSIZE(FeatureLevelsRequested);
        uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG_DX11
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        DX11_CHECK_RET0(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));

#ifdef DX11_3_API
        // k for uwp, we have to make 11 dev and then upgrade them to the newer versions
        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;
        DX11_CHECK_RET0(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &device, nullptr, &context));

        DX11_CHECK_RET0(device.As(&m_dev));

        DX11_CHECK_RET0(context.As(&m_devcon));

        // unknown if this works with 11_3 api ?
        //InitDX11DebugLayer(m_dev.Get());

        DXGI_SWAP_CHAIN_DESC1 sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.Width = m_winWidth;
        sd.Height = m_winHeight;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1; //cant use msaa
        sd.SampleDesc.Quality = 0;
        // flip sequential is needed for store apps
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        sd.BufferCount = 2;
        sd.Scaling = DXGI_SCALING_NONE;
        sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        sd.Flags = 0;

        DX11_CHECK_RET0(m_factory->CreateSwapChainForCoreWindow(m_dev.Get(),
            reinterpret_cast<IUnknown*>(deviceInitialization.windowHandle),
            &sd,
            nullptr,
            &m_swapchain)
        );

        // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
        // ensures that the application will only render after each VSync, minimizing power consumption.
        ComPtr<IDXGIDevice3> dxgiDevice;
        DX11_CHECK_RET0(m_dev.As(&dxgiDevice));
        DX11_CHECK_RET0(dxgiDevice->SetMaximumFrameLatency(1));
#else
        DX11_CHECK_RET0(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &m_dev, nullptr, &m_devcon));

        // note to self, this might be needed for win7 support with just 11.0
        // although initial tests show otherwise
        /*if (hr == E_INVALIDARG)
        {
        hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, &lvl[1], _countof(lvl) - 1,
        D3D11_SDK_VERSION, &pDevice, &fl, &pContext);
        }*/

        // commenting this out seems to make diagnostic debugging work?, ugh
        //InitDX11DebugLayer(m_dev.Get());

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = m_winWidth;
        sd.BufferDesc.Height = m_winHeight;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.BufferCount = 2;
        sd.OutputWindow = static_cast<HWND>(deviceInitialization.windowHandle);
        sd.Windowed = true;
        DX11_CHECK_RET0(m_factory->CreateSwapChain(m_dev.Get(), &sd, &m_swapchain));
#endif

        ComPtr<ID3D11Texture2D> backbuffer;
        DX11_CHECK_RET0(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));

        DX11_CHECK_RET0(m_dev->CreateRenderTargetView(backbuffer.Get(), nullptr, m_renderTarget.GetAddressOf()));

        // Create and use default rasterize / depth
        SetRasterState(RasterState());
        SetDepthState(DepthState());

        ResetDepthStencilTexture();
        ResetViewport();

        m_devcon->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());

        // todo: deal with this?
        CreateSampler();

        return 1;
    }

    void RenderDeviceDX11::PrintDisplayAdapterInfo() {
        ComPtr<IDXGIAdapter> adapter;
        DX11_CHECK(m_factory->EnumAdapters(0, &adapter));
        auto adapterDesc = DXGI_ADAPTER_DESC();
        DX11_CHECK(adapter->GetDesc(&adapterDesc));

        char buffer[128];
        wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);
        char *dxLevel = "Unknown";
        switch (m_dev->GetFeatureLevel()) {
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1:
            dxLevel = "11.1";
            break;
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0:
            dxLevel = "11.0";
            break;
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0:
            dxLevel = "10.0";
        }

        LOG_D("DirectX Version: v%s", dxLevel);
        LOG_D("DisplayAdaterDesc: %s", buffer);
        LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
    }

    RenderDeviceDX11::~RenderDeviceDX11() {
        ReleaseAllFromMap(m_indexBuffers, &IndexBufferDX11::indexBuffer);
        ReleaseAllFromMap(m_vertexBuffers, &VertexBufferDX11::vertexBuffer);
        for (auto it = m_constantBuffers.begin(); it != m_constantBuffers.end(); ++it) {
            for (auto pcBuffer : it->second.constantBuffers) {
                pcBuffer->Release();
            }
            it->second.cBufferDescs.clear();
        }
        ReleaseAllFromMap(m_shaders, &ShaderDX11::pixelShader);
        ReleaseAllFromMap(m_shaders, &ShaderDX11::vertexShader);
        ReleaseAllFromMap(m_inputLayouts, &InputLayoutDX11::inputLayout);
        ReleaseAllFromMap(m_textures, &TextureDX11::texture);
        ReleaseAllFromMap(m_textures, &TextureDX11::shaderResourceView);
        ReleaseAllFromMap(m_samplers, &SamplerDX11::sampler);
        ReleaseAllFromMap(m_blendStates, &BlendStateDX11::blendState);
        ReleaseAllFromMap(m_rasterStates, &RasterStateDX11::rasterState);
        ReleaseAllFromMap(m_depthStates, &DepthStateDX11::depthState);

        m_depthStencilView.Reset();
        m_devcon.Reset();
        m_renderTarget.Reset();
        m_swapchain.Reset();
        m_factory.Reset();
        m_dev.Reset();
    }
}
