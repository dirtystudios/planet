#include "DX11RenderDevice.h"
#include "Log.h"
#include <d3dcompiler.h>
#include <d3dcompiler.inl>
#include "DX11ConstantBufferHelpers.h"

#define SafeGet(id, idx) id[idx];

#define DEBUG_DX11

// Debug Helper Functions

#pragma region Debug Helper Functions

// Helper sets a D3D resource name string (used by PIX and debug layer leak reporting).
inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char *name)
{
#ifdef DEBUG_DX11
    resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(name)), name);
#else
    UNREFERENCED_PARAMETER(resource);
    UNREFERENCED_PARAMETER(name);
#endif
}

// Debug Initializtion

void InitDX11DebugLayer(ID3D11Device* dev){

#ifdef DEBUG_DX11
    ID3D11Debug *d3dDebug = nullptr;
    if (SUCCEEDED(dev->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
    {
        ID3D11InfoQueue *d3dInfoQueue = nullptr;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
        {
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

            D3D11_MESSAGE_ID hide[] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // Add more message IDs here as needed
            };

            /*D3D11_INFO_QUEUE_FILTER filter;
            memset(&filter, 0, sizeof(filter));
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);*/
            d3dInfoQueue->Release();
        }
        else {
            LOG_E("DX11Render: Failed to Create d3dDebugInfoQueue.");
            d3dDebug->Release();
        }
    }
    else {
        LOG_E("DX11Render: Failed to Create d3dDebug Interface.");
    }
#endif
}

#pragma endregion

namespace graphics {

#pragma region Create Commands

    IndexBufferHandle RenderDeviceDX11::CreateIndexBuffer(void* data, size_t size, BufferUsage usage) {
        ID3D11Buffer* indexBuffer = NULL;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = size;
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = { 0 };
        initData.pSysMem = data;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        HRESULT hr = m_dev->CreateBuffer(&bufferDesc, &initData, &indexBuffer);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed creating Index Buffer. HR: 0x%x", hr);
            return 0;
        }
        else {
            uint32_t handle = GenerateHandle();
            IndexBufferDX11 ib;
            ib.indexBuffer = indexBuffer;//indexBuffer.Get();

            m_indexBuffers.insert(std::make_pair(handle, ib));
            return handle;
        }
    }
    
    VertexBufferHandle RenderDeviceDX11::CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) {
        ID3D11Buffer* vertexBuffer = NULL;

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        bufferDesc.Usage = SafeGet(BufferUsageDX11, (uint32_t)usage);
        bufferDesc.ByteWidth = size;
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

        HRESULT hr = m_dev->CreateBuffer(&bufferDesc, data ? &initData : NULL, &vertexBuffer);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Failed to create Vertex buffer. HResult: 0x%x", hr);
            return 0;
        }
        uint32_t handle = GenerateHandle();
        VertexBufferDX11 vb = {};
        vb.vertexBuffer = vertexBuffer;//vertexBuffer.Get();
        vb.layout = layout;

        m_vertexBuffers.insert(std::make_pair(handle, vb));
        return handle;
    }

    ShaderHandle RenderDeviceDX11::CreateShader(ShaderType shaderType, const char** source) {
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr;

        char *entryPoint;
        char *target;

        // may want to break these up...meh

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
            return 0;
        }

        uint32_t flags = 0;
#ifdef DEBUG_DX11
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;
#endif 

        hr = D3DCompile(source[0], strlen(source[0]), NULL, NULL, NULL, entryPoint, target, flags, 0, &blob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                LOG_E("DX11RenderDev: Failed to Compile Shader. Error: %s", errorBlob->GetBufferPointer());
            }
            else {
                LOG_E("DX11RenderDev: Failed to compile Shader. Hr: 0x%x", hr);
            }
            return 0;
        }

        // ---- Create Shader and do necessary reflection 
        // todo: samplers?
        ShaderDX11 shader = {};
        uint32_t shaderHandle = 0;
        shader.shaderType = shaderType;

        switch (shaderType) {
        case ShaderType::VERTEX_SHADER:
        {
            ID3D11VertexShader* vertexShader;
            hr = m_dev->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &vertexShader);
            if (FAILED(hr)) {
                LOG_E("DX11RenderDev: Failed to Create VertexShader in CreateProgram. Hr: 0x%x", hr);
                return 0;
            }

            uint32_t cBufferHandle = GenerateHandle();
            cBufferHandle = CreateConstantBuffer(blob.Get());

            uint32_t ilHandle = CreateInputLayout(blob.Get());

            shaderHandle = GenerateHandle();
            shader.vertexShader = vertexShader;//vertexShader.Get();
            shader.cbHandle = cBufferHandle;
            shader.inputLayoutHandle = ilHandle;
            break;
        }
        case ShaderType::FRAGMENT_SHADER:
        {
            // todo: anything else for fragment shader
            ID3D11PixelShader* pixelShader;
            hr = m_dev->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pixelShader);
            if (FAILED(hr)) {
                LOG_E("DX11RenderDev: Failed to Create PixelShader in CreateProgram. Hr: 0x%x", hr);
                return 0;
            }

            uint32_t cBufferHandle = GenerateHandle();
            cBufferHandle = CreateConstantBuffer(blob.Get());

            shaderHandle = GenerateHandle();
            shader.pixelShader = pixelShader;//pixelShader.Get();
            shader.cbHandle = cBufferHandle;
            break;
        }
        // todo: the rest of the shaders
        default:
            LOG_E("DX11RenderDev: Invalid/Unimplemented Shader Type to compile.");
            return 0;
            break;
        }
        m_shaders.insert(std::make_pair(shaderHandle, shader));
        return shaderHandle;
    }

    uint32_t RenderDeviceDX11::CreateInputLayout(ID3DBlob *shader) {
        InputLayoutCacheHandle ilHandle = inputLayoutCache.InsertInputLayout(shader);
        ID3D11InputLayout* inputLayout;

        //todo, handle cache / same hash/handle
        if (!ilHandle) {
            LOG_D("DX11Render: Invalid or empty input layout defined in shader.");
            return 0;
        }

        HRESULT hr = m_dev->CreateInputLayout(inputLayoutCache.GetInputLayoutData(ilHandle), inputLayoutCache.GetInputLayoutSize(ilHandle), shader->GetBufferPointer(), shader->GetBufferSize(), &inputLayout);
        if (FAILED(hr)) {
            LOG_E("DX11Render: Failed to Create Input Layout. HR: 0x%x", hr);
            return 0;
        }
        InputLayoutDX11 inputLayoutDx11;
        inputLayoutDx11.inputLayout = inputLayout;//inputLayout.Get();
        m_inputLayouts.insert(std::make_pair(ilHandle, inputLayoutDx11));
        return ilHandle;
    }

    uint32_t RenderDeviceDX11::CreateConstantBuffer(ID3DBlob *shaderBlob) {
        std::vector<ID3D11Buffer*> constantBuffers;

        size_t numCBuffers = 0;
        graphics::dx11::CBufferDescriptor* cBuffers = graphics::dx11::GenerateConstantBuffer(shaderBlob, &numCBuffers);

        for (uint32_t x = 0; x < numCBuffers; ++x) {

            ID3D11Buffer* cBuffer;
            D3D11_BUFFER_DESC cbDesc;
            cbDesc.ByteWidth = cBuffers[x].totalSize;
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;

            HRESULT hr = m_dev->CreateBuffer(&cbDesc, NULL, &cBuffer);
            if (FAILED(hr)) {
                LOG_E("DX11RenderDev: Failed to Create ConstantBuffer. Hr: 0x%x", hr);
                //todo: delete cbuffers?
                return 0;
            }
            constantBuffers.emplace_back(cBuffer);
        }
        uint32_t handle = GenerateHandle();
        ConstantBufferDX11 cb = {};
        cb.cBufferDescs.assign(&cBuffers, &cBuffers + numCBuffers);
        cb.constantBuffers = constantBuffers;
        //cb.constantBuffer = constantBuffer;//constantBuffer.Get();
        m_constantBuffers.insert(std::make_pair(handle, cb));

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

        return Texture2DCreator(&tdesc, &viewDesc, data);
    }

    TextureHandle RenderDeviceDX11::CreateTextureCube(TextureFormat texFormat, uint32_t width, uint32_t height, void** data) {
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
        tdesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
        tdesc.Format = dxFormat;

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        viewDesc.TextureCube.MipLevels = tdesc.MipLevels;
        viewDesc.TextureCube.MostDetailedMip = 0;

        return Texture2DCreator(&tdesc, &viewDesc, data);
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

        return Texture2DCreator(&tdesc, &viewDesc, 0);
    }

    TextureHandle RenderDeviceDX11::Texture2DCreator(D3D11_TEXTURE2D_DESC* tDesc, D3D11_SHADER_RESOURCE_VIEW_DESC* viewDesc, void* data) {
        ID3D11Texture2D* texture;
        D3D11_SUBRESOURCE_DATA srd;
        if (data) {
            srd.pSysMem = data;
            srd.SysMemPitch = GetFormatByteSize(tDesc->Format) * tDesc->Width;
            srd.SysMemSlicePitch = 0; //?
        }

        HRESULT hr = m_dev->CreateTexture2D(tDesc, data ? &srd : NULL, &texture);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Failed Creating Texture Hr: 0x%x", hr);
            return 0;
        }

        ID3D11ShaderResourceView* shaderResourceView;
        hr = m_dev->CreateShaderResourceView(texture, viewDesc, &shaderResourceView);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Failed Creating shaderResourceView Hr: 0x%x", hr);
            //texture.ReleaseAndGetAddressOf();
            texture->Release();
            return 0;
        }

        uint32_t handle = GenerateHandle();
        TextureDX11 textureDX11 = {};
        textureDX11.texture = texture;//texture.Get();
        textureDX11.shaderResourceView = shaderResourceView;//shaderResourceView.Get();
        textureDX11.format = tDesc->Format;
        m_textures.insert(std::make_pair(handle, textureDX11));

        return handle;

    }
#pragma endregion

#pragma region Destroy Commands

    void RenderDeviceDX11::DestroyIndexBuffer(IndexBufferHandle handle) {
        IndexBufferDX11* indexBuff = Get(m_indexBuffers, handle);
        if (!indexBuff) {
            LOG_E("DX11RenderDev: Failed to DestroyIndexBuffer: %d", handle)
                return;
        }
        if (indexBuff->indexBuffer)
            indexBuff->indexBuffer->Release();
        m_indexBuffers.erase(handle);
    }


    void RenderDeviceDX11::DestroyVertexBuffer(VertexBufferHandle handle) {
        auto it = m_vertexBuffers.find(handle);
        if (it == m_vertexBuffers.end()) {
            LOG_E("DX11RenderDev: Failed to DestroyVertexBuffer: %d", handle)
                return;
        }
        VertexBufferDX11 &vb = (*it).second;

        if (vb.vertexBuffer)
            vb.vertexBuffer->Release();
        m_vertexBuffers.erase(it);
    }

    void RenderDeviceDX11::DestroyShader(ShaderHandle handle) {
        ShaderDX11* shader = Get(m_shaders, handle);
        if (!shader) {
            LOG_E("DX11RenderDev: Invalid handle given to DestroyShader");
        }
        if (shader->inputLayoutHandle) {
            InputLayoutDX11* il = Get(m_inputLayouts, shader->inputLayoutHandle);
            il->inputLayout->Release();
            m_inputLayouts.erase(shader->inputLayoutHandle);
        }
        if (shader->cbHandle) {
            // todo : delete me 
            /*ConstantBufferDX11* cBuffer = Get(ntBuffers, shader->cbHandle);
            cBuffer->constantBuffer->Release();
            m_constantBuffers.erase(shader->cbHandle);*/
        }
        //todo: release rest of stuff
        m_shaders.erase(handle);
    }

    void RenderDeviceDX11::DestroyConstantBuffer(ConstantBufferHandle handle) {
        auto it = m_constantBuffers.find(handle);
        if (it == m_constantBuffers.end()) {
            return;
        }
        ConstantBufferDX11 &cb = (*it).second;

        // todo: delete me 

        /*if(cb.constantBuffer) {
        cb.constantBuffer->Release();
        }
        m_constantBuffers.erase(it);*/
    }

    void RenderDeviceDX11::DestroyInputLayout(uint32_t handle) {
        InputLayoutDX11* inputLayout = Get(m_inputLayouts, handle);
        if (!inputLayout) {
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
            LOG_E("DX11RenderDev: Invalid handle given to DestroyTexture.");
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

            // Todo: state for cbuffers and friends
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
        m_devcon->ClearRenderTargetView(renderTarget.Get(), RGBA);
    }

    void RenderDeviceDX11::SwapBuffers() {
        HRESULT hr = m_swapchain->Present(1, 0);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Error on present? Hr: 0x%x", hr);
        }
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
        HRESULT hr = 0;
        
        for (uint32_t x = 0; x < dirtySlots.size(); ++x) {
            uint32_t slot = dirtySlots[x];

            hr = m_devcon->Map(cb->constantBuffers[slot], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            if (FAILED(hr)) {
                LOG_E("DX11RenderDev: Failed to map constant buffer. HR: 0x%x", hr);
                return;
            }
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
        m_devcon->UpdateSubresource(texture->texture, D3D11CalcSubresource(0, arrayIndex, 1), &box, data, width * formatByteSize, width * formatByteSize * height);
    }

    void RenderDeviceDX11::SetRasterizerState(uint32_t state) {
        //uhhh
    }

    void RenderDeviceDX11::SetDepthState(uint32_t state) {
        //bleh
    }

    void RenderDeviceDX11::SetBlendState(uint32_t state) {
        //no 
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
        HRESULT hr = 0;
        hr = m_devcon->Map(vBuffer->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr)) {
            LOG_E("DX11RenderDev: Failed to map vertex buffer. HR: 0x%x", hr);
            return;
        }
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
                m_pendingState.psTextures.insert(std::make_pair((uint32_t)slot, texture->shaderResourceView));
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
                m_pendingState.vsTextures.insert(std::make_pair((uint32_t)slot, texture->shaderResourceView));
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
                m_devcon->PSSetConstantBuffers(0, m_pendingState.psCBuffer->constantBuffers.size(), m_pendingState.psCBuffer->constantBuffers.data());
        }

        if (m_pendingState.vertexShaderHandle != 0 && m_currentState.vertexShaderHandle != m_pendingState.vertexShaderHandle) {
            m_devcon->VSSetShader(m_pendingState.vertexShader->vertexShader, 0, 0);
            if (m_pendingState.vsCBuffer)
                m_devcon->VSSetConstantBuffers(0, m_pendingState.vsCBuffer->constantBuffers.size(), m_pendingState.vsCBuffer->constantBuffers.data());
        }

        if (m_pendingState.vertexShaderHandle != 0 && m_pendingState.vsCBuffer)
            UpdateConstantBuffer(m_pendingState.vsCBuffer, m_pendingState.vsCBufferDirtySlots);

        if (m_pendingState.pixelShaderHandle != 0 && m_pendingState.psCBuffer)
            UpdateConstantBuffer(m_pendingState.psCBuffer, m_pendingState.psCBufferDirtySlots);

        if (m_pendingState.inputLayoutHandle != 0 && m_currentState.inputLayoutHandle != m_pendingState.inputLayoutHandle)
            m_devcon->IASetInputLayout(m_pendingState.inputLayout->inputLayout);

        if (m_pendingState.vertexBufferHandle != 0 && m_currentState.vertexBufferHandle != m_pendingState.vertexBufferHandle) {
            uint32_t offset = 0;
            m_devcon->IASetVertexBuffers(0, 1, &m_pendingState.vertexBuffer->vertexBuffer, &m_pendingState.vertexBuffer->layout.stride, &offset);
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

        m_devcon->OMSetRenderTargets(1, renderTarget.GetAddressOf(), NULL);
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
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        ID3D11SamplerState* samplerState;
        HRESULT hr = m_dev->CreateSamplerState(&samplerDesc, &samplerState);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Failed creating samplerState HR: 0x%x", hr);
        }

        uint32_t handle = GenerateHandle();
        SamplerDX11 samplerDX11 = {};
        samplerDX11.sampler = samplerState;//samplerState.Get();
        m_samplers.insert(std::make_pair(handle, samplerDX11));

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

    int RenderDeviceDX11::InitializeDevice(void *windowHandle, uint32_t windowHeight, uint32_t windowWidth) {
        DeviceConfig.DeviceAbbreviation = "DX11";
        DeviceConfig.ShaderExtension = ".hlsl";

        m_hwnd = static_cast<HWND>(windowHandle);

        D3D_FEATURE_LEVEL FeatureLevelsRequested[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };

        uint32_t numLevelsRequested = 2;
        uint32_t creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG_DX11
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &m_dev, nullptr, &m_devcon);

        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating D3D11Device: 0x%x", hr);
            return hr;
        }

        InitDX11DebugLayer(m_dev.Get());

        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&m_factory));
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating DXGIFactory: 0x%x", hr);
            return hr;
        }

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.BufferCount = 2;
        sd.OutputWindow = m_hwnd;
        sd.Windowed = true;

        hr = m_factory->CreateSwapChain(m_dev.Get(), &sd, &m_swapchain);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating Swapchain: 0x%x", hr);
            return hr;
        }

        ComPtr<ID3D11Texture2D> backbuffer;
        m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backbuffer);

        // todo: what to do about this?
        m_dev->CreateRenderTargetView(backbuffer.Get(), nullptr, &renderTarget);

        // TODO-Jake: move this to setRasterizerState
        D3D11_RASTERIZER_DESC rasterDesc;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.FrontCounterClockwise = true;
        rasterDesc.AntialiasedLineEnable = true;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.MultisampleEnable = true;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        ComPtr<ID3D11RasterizerState> rasterState;

        hr = m_dev->CreateRasterizerState(&rasterDesc, &rasterState);
        if (FAILED(hr)){
            LOG_E("DX11RenderDev: Error Creating RasterState: 0x%x", hr);
            return hr;
        }

        m_devcon->RSSetState(rasterState.Get());

        //viewport?
        D3D11_VIEWPORT vp;
        vp.Width = (float)windowWidth;
        vp.Height = (float)windowHeight;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_devcon->RSSetViewports(1, &vp);

        // todo: deal with this?
        CreateSampler();

        // todo: move this
        ID3D11BlendState* blendState;
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.IndependentBlendEnable = false;
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        m_dev->CreateBlendState(&blendDesc, &blendState);
        float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        UINT sampleMask = 0xffffffff;
        m_devcon->OMSetBlendState(blendState, blendFactor, sampleMask);

        return hr;
    }

    void RenderDeviceDX11::PrintDisplayAdapterInfo() {
        ComPtr<IDXGIAdapter> adapter;
        m_factory->EnumAdapters(0, &adapter);
        auto adapterDesc = DXGI_ADAPTER_DESC();
        adapter->GetDesc(&adapterDesc);

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
        }

        LOG_D("DirectX Version: v%s", dxLevel);
        LOG_D("DisplayAdaterDesc: %s", buffer);
        LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
    }
}
