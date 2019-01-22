#include "DX11Device.h"
#include "Log.h"
#include "SimpleShaderLibrary.h"
#include "DX11Debug.h"
#include "TexConvert.h"
#include "DX11CommandBuffer.h"

#include <d3dcompiler.h>
#include <d3dcompiler.inl>


namespace gfx {
    using namespace Microsoft::WRL;

    BufferId DX11Device::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
        D3D11_BUFFER_DESC bufferDesc = { 0 };

        if ((desc.accessFlags & BufferAccessFlags::GpuReadCpuWriteBits) == BufferAccessFlags::GpuReadCpuWriteBits) {
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        }
        else if ((desc.accessFlags & BufferAccessFlags::GpuReadBit) == BufferAccessFlags::GpuReadBit) {
            bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        }
        else {
            assert(false && "unsupported access flags");
        }

        switch (desc.usageFlags) {
        case BufferUsageFlags::VertexBufferBit:   bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; break;
        case BufferUsageFlags::IndexBufferBit:    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; break;
        case BufferUsageFlags::ConstantBufferBit: bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
        case BufferUsageFlags::ShaderBufferBit:   bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; break;
        default:
            assert(false);
        }
        
        uint32_t buffSize = desc.size;

        if (bufferDesc.BindFlags == D3D11_BIND_CONSTANT_BUFFER) {
            // length must be multiple of 16
            uint32_t sizeCheck = buffSize % 16;
            if (sizeCheck != 0) {
                buffSize += (16 - sizeCheck);
            }
        }
        else if (bufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) {
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
            //bufferDesc.StructureByteStride = 4; // ??? assume output is rgba 8 bit channel
            if (desc.accessFlags & (BufferAccessFlags::GpuWriteBit))
                bufferDesc.BindFlags &= D3D11_BIND_UNORDERED_ACCESS;
        }

        bufferDesc.ByteWidth = static_cast<UINT>(buffSize);

        if (bufferDesc.Usage == D3D11_USAGE_DYNAMIC)
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        else
            bufferDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        if (initialData) {
            initData.pSysMem = initialData;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
        }

        ComPtr<ID3D11Buffer> buffer = NULL;

        DX11_CHECK_RET0(m_dev->CreateBuffer(&bufferDesc, initialData ? &initData : NULL, &buffer));
        if (desc.debugName != "")
            D3D_SET_OBJECT_NAME_A(buffer, desc.debugName.c_str());

        ComPtr<ID3D11ShaderResourceView> srv = NULL;
        ComPtr<ID3D11UnorderedAccessView> uav = NULL;
        if ((bufferDesc.BindFlags & (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS)) == (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS)) {
            D3D11_UNORDERED_ACCESS_VIEW_DESC desc{};
            desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.Buffer.FirstElement = 0;
            desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
            desc.Buffer.NumElements = buffSize / 4;
            DX11_CHECK(m_dev->CreateUnorderedAccessView(buffer.Get(), &desc, &uav));
        }
        else if ((bufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) == D3D11_BIND_SHADER_RESOURCE) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            //srvDesc.BufferEx.FirstElement = 0;
            //srvDesc.BufferEx.NumElements = buffSize / bufferDesc.StructureByteStride;
            DX11_CHECK(m_dev->CreateShaderResourceView(buffer.Get(), &srvDesc, &srv));
        }

        BufferDX11 *bufferdx11 = new BufferDX11();
        bufferdx11->buffer.Swap(buffer);
        bufferdx11->srv.Swap(srv);
        bufferdx11->uav.Swap(uav);
        return m_resourceManager->AddResource(bufferdx11);
    }

    ShaderId DX11Device::GetShader(ShaderType type, const std::string& functionName) {
        auto shaderId = m_shaderLibrary.GetShader(type, functionName);
        assert(shaderId);
        return shaderId;
    }

    void DX11Device::AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) {
        for (const ShaderData& shaderData : shaderData) {
            assert(shaderData.type == ShaderDataType::Source);

            std::string funcName = shaderData.name.substr(0, shaderData.name.length() - 5);
            {
                ShaderType shaderType = ShaderType::PixelShader;

                ShaderId existingShaderId = m_shaderLibrary.GetShader(shaderType, funcName);
                ShaderId newID = CreateShader(shaderType, std::string(reinterpret_cast<const char*>(shaderData.data), shaderData.len), funcName + "PS");

                assert(newID);

                ShaderFunctionDesc desc;
                desc.entryPoint = "";
                desc.functionName = funcName;
                desc.type = shaderType;

                if (existingShaderId == 0) {
                    m_shaderLibrary.AddShader(newID, desc);
                }
                else {
                    assert(false);
                }
            }
            {
                ShaderType shaderType = ShaderType::VertexShader;

                ShaderId existingShaderId = m_shaderLibrary.GetShader(shaderType, funcName);
                ShaderId newID = CreateShader(shaderType, std::string(reinterpret_cast<const char*>(shaderData.data), shaderData.len), funcName + "VS");

                assert(newID);

                ShaderFunctionDesc desc;
                desc.entryPoint = "";
                desc.functionName = funcName;
                desc.type = shaderType;

                if (existingShaderId == 0) {
                    m_shaderLibrary.AddShader(newID, desc);
                }
                else {
                    assert(false);
                }
            }
            {
                ShaderType shaderType = ShaderType::ComputeShader;

                ShaderId existingShaderId = m_shaderLibrary.GetShader(shaderType, funcName);
                ShaderId newID = CreateShader(shaderType, std::string(reinterpret_cast<const char*>(shaderData.data), shaderData.len), funcName + "CS");

                assert(newID);

                ShaderFunctionDesc desc;
                desc.entryPoint = "";
                desc.functionName = funcName;
                desc.type = shaderType;

                if (existingShaderId == 0) {
                    m_shaderLibrary.AddShader(newID, desc);
                }
                else {
                    assert(false);
                }
            }
        }
    }

    ShaderId DX11Device::CreateShader(ShaderType type, const std::string& source, const std::string& name) {
        ComPtr<ID3DBlob> blob;
        void* bufPtr;
        size_t bufSize;

        if (m_usePrebuiltShaders) {
            bufPtr = (void*)&source[0];
            bufSize = source.length();

            D3DCreateBlob(bufSize, blob.ReleaseAndGetAddressOf());
            memcpy(blob->GetBufferPointer(), bufPtr, bufSize);
        }
        else {
            blob.Swap(CompileShader(type, source));
            bufPtr = blob->GetBufferPointer();
            bufSize = blob->GetBufferSize();
        }
        if (!bufPtr) {
            LOG_E("DX11RenderDev: bufPtr is 0 in CreateShader");
            return 0;
        }

        ShaderDX11* shaderDX11 = new ShaderDX11();
        shaderDX11->shaderType = type;
        switch (type) {
        case ShaderType::VertexShader:
        {
            ID3D11VertexShader* vertexShader;
            DX11_CHECK_RET0(m_dev->CreateVertexShader(bufPtr, bufSize, NULL, &vertexShader));
            shaderDX11->vertexShader = vertexShader;
            shaderDX11->blob.Swap(blob);
            D3D_SET_OBJECT_NAME_A(vertexShader, name.c_str());
            return m_resourceManager->AddResource(shaderDX11);
            break;
        }
        case ShaderType::PixelShader:
        {
            ID3D11PixelShader* pixelShader;
            DX11_CHECK_RET0(m_dev->CreatePixelShader(bufPtr, bufSize, NULL, &pixelShader));

            shaderDX11->pixelShader = pixelShader;
            D3D_SET_OBJECT_NAME_A(pixelShader, name.c_str());
            return m_resourceManager->AddResource(shaderDX11);
            break;
        }
        case ShaderType::ComputeShader:
        {
            ID3D11ComputeShader* computeShader;
            DX11_CHECK_RET0(m_dev->CreateComputeShader(bufPtr, bufSize, NULL, &computeShader));
            shaderDX11->computeShader = computeShader;
            D3D_SET_OBJECT_NAME_A(computeShader, name.c_str());
            return m_resourceManager->AddResource(shaderDX11);
            break;
        }
        default:
            LOG_E("DX11RenderDev: Invalid/Unimplemented Shader Type to compile.");
            return 0;
            break;
        }
    }

    ComPtr<ID3DBlob> DX11Device::CompileShader(ShaderType shaderType, const std::string& source) {
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        char *entryPoint;
        char *target;
        
        auto featLvl = m_dev->GetFeatureLevel();
        switch (shaderType) {
        case ShaderType::PixelShader:
            entryPoint = "PSMain";
            target = featLvl >= D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0";
            break;
        case ShaderType::VertexShader:
            entryPoint = "VSMain";
            target = featLvl >= D3D_FEATURE_LEVEL_11_0 ? "vs_5_0" : "vs_4_0";
            break;
        case ShaderType::ComputeShader:
            entryPoint = "CSMain";
            target = featLvl >= D3D_FEATURE_LEVEL_11_0 ? "cs_5_0" : "cs_4_0";
            break;
        default:
            LOG_E("DX11RenderDev: Unsupported shader type supplied. Type: %d", shaderType);
            return blob;
        }

        uint32_t flags = 0;
#ifdef DEBUG_DX11
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;
#endif 

        HRESULT hr = D3DCompile(&source[0], source.length(), NULL, NULL, NULL, entryPoint, target, flags, 0, &blob, &errorBlob);

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

    PipelineStateId DX11Device::CreatePipelineState(const PipelineStateDesc & desc) {

        size_t descHash = std::hash<PipelineStateDesc>()(desc);
        auto pc = m_cache.pipelineCache.find(descHash);
        if (pc != m_cache.pipelineCache.end()) {
            return pc->second;
        }

        PipelineStateDX11* stateDx11 = new PipelineStateDX11();

        stateDx11->renderPassHandle = desc.renderPass;

        ShaderDX11* ps = m_resourceManager->GetResource<ShaderDX11>(desc.pixelShader);
        stateDx11->pixelShader = ps->pixelShader;

        ShaderDX11* vs = m_resourceManager->GetResource<ShaderDX11>(desc.vertexShader);
        stateDx11->vertexShader = vs->vertexShader;
        stateDx11->vertexShaderHandle = desc.vertexShader;

        ShaderDX11* cs = m_resourceManager->GetResource<ShaderDX11>(desc.computeShader);
        stateDx11->computeShader = vs->computeShader;

        stateDx11->topology = SafeGet(PrimitiveTypeDX11, desc.topology);

        auto layout = m_resourceManager->GetResource<InputLayoutDX11>(desc.vertexLayout);
        stateDx11->vertexLayout = layout->inputLayout.Get();
        stateDx11->vertexLayoutHandle = desc.vertexLayout;
        stateDx11->vertexLayoutStride = layout->stride;

        size_t hash = std::hash<BlendState>()(desc.blendState);
        auto blendState = m_cache.bsCache.find(hash);
        if (blendState == m_cache.bsCache.end()) {
            stateDx11->blendState = CreateBlendState(desc.blendState);
        }
        else {
            stateDx11->blendState = blendState->second.bs.Get();
        }

        hash = std::hash<DepthState>()(desc.depthState);
        auto depthState = m_cache.dsCache.find(hash);
        if (depthState == m_cache.dsCache.end()) {
            stateDx11->depthState = CreateDepthState(desc.depthState);
        }
        else {
            stateDx11->depthState = depthState->second.ds.Get();
        }

        hash = std::hash<RasterState>()(desc.rasterState);
        auto rasterState = m_cache.rsCache.find(hash);
        if (rasterState == m_cache.rsCache.end()) {
            stateDx11->rasterState = CreateRasterState(desc.rasterState);
        }
        else {
            stateDx11->rasterState = rasterState->second.rs.Get();
        }

        auto id = m_resourceManager->AddResource(stateDx11);
        m_cache.pipelineCache.emplace(id, descHash);
        return id;
    }

    ID3D11DepthStencilState* DX11Device::CreateDepthState(const DepthState& state) {
        // internal, so ignore hash check
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

        dsDesc.DepthEnable = state.enable;
        dsDesc.StencilEnable = state.enable;
        dsDesc.DepthWriteMask = SafeGet(DepthWriteMaskDX11, state.depthWriteMask);
        dsDesc.DepthFunc = SafeGet(DepthFuncDX11, state.depthFunc);

        ID3D11DepthStencilState* depthStateDX;
        DX11_CHECK_RET0(m_dev->CreateDepthStencilState(&dsDesc, &depthStateDX));

        size_t hash = std::hash<DepthState>()(state);
        DepthStateDX11 ds;
        ds.ds.Attach(depthStateDX);
        m_cache.dsCache.emplace(hash, ds);
        return depthStateDX;
    }

    ID3D11RasterizerState* DX11Device::CreateRasterState(const RasterState& state) {
        // internal, so ignore hash check
        D3D11_RASTERIZER_DESC rasterDesc;
        rasterDesc.AntialiasedLineEnable = true;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.MultisampleEnable = true;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        rasterDesc.CullMode = SafeGet(CullModeDX11, state.cullMode);
        rasterDesc.FillMode = SafeGet(FillModeDX11, state.fillMode);
        rasterDesc.FrontCounterClockwise = state.windingOrder == WindingOrder::FrontCCW ? true : false;

        ID3D11RasterizerState* rasterStateDX;
        DX11_CHECK_RET0(m_dev->CreateRasterizerState(&rasterDesc, &rasterStateDX));

        size_t hash = std::hash<RasterState>()(state);
        RasterStateDX11 rs;
        rs.rs.Attach(rasterStateDX);
        m_cache.rsCache.emplace(hash, rs);
        return rasterStateDX;
    }

    ID3D11BlendState* DX11Device::CreateBlendState(const BlendState& state) {
        // internal, so ignore hash check
        D3D11_BLEND_DESC blendDesc;
        ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
        blendDesc.IndependentBlendEnable = false;

        blendDesc.RenderTarget[0].BlendEnable = (state.enable) ? TRUE : FALSE;
        blendDesc.RenderTarget[0].SrcBlendAlpha = SafeGet(BlendFuncDX11, state.srcAlphaFunc);
        blendDesc.RenderTarget[0].DestBlendAlpha = SafeGet(BlendFuncDX11, state.dstAlphaFunc);
        blendDesc.RenderTarget[0].SrcBlend = SafeGet(BlendFuncDX11, state.srcRgbFunc);
        blendDesc.RenderTarget[0].DestBlend = SafeGet(BlendFuncDX11, state.dstRgbFunc);
        blendDesc.RenderTarget[0].BlendOpAlpha = SafeGet(BlendModeDX11, state.alphaMode);
        blendDesc.RenderTarget[0].BlendOp = SafeGet(BlendModeDX11, state.rgbMode);

        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        ID3D11BlendState* blendStateDX;
        DX11_CHECK_RET0(m_dev->CreateBlendState(&blendDesc, &blendStateDX));
        size_t hash = std::hash<BlendState>()(state);
        BlendStateDX11 bs;
        bs.bs.Attach(blendStateDX);
        m_cache.bsCache.emplace(hash, bs);
        return blendStateDX;
    }

    void DX11Device::CreateDefaultSampler() {
        D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
        //samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        DX11_CHECK(m_dev->CreateSamplerState(&samplerDesc, &m_defaultSampler));
    }

    VertexLayoutId DX11Device::CreateVertexLayout(const VertexLayoutDesc& layoutDesc) {
        // Here we just take the hash and shove the empty layout into our cache
        //  this will be created when needed on draw
        size_t hash = std::hash<VertexLayoutDesc>()(layoutDesc);
        auto layoutCheck = m_cache.ilCache.find(hash);
        if (layoutCheck != m_cache.ilCache.end())
            return layoutCheck->second;

        InputLayoutDX11 *layout = new InputLayoutDX11();
        layout->stride = 0;
        layout->layoutDesc.elements = layoutDesc.elements;
        auto id = m_resourceManager->AddResource(layout);
        m_cache.ilCache.emplace(hash, id);
        return id;
    }

    TextureId DX11Device::CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName) {
        D3D11_TEXTURE2D_DESC tdesc = { 0 };
        tdesc.Width = width;
        tdesc.Height = height;
        tdesc.MipLevels = 1;
        tdesc.ArraySize = 1;
        tdesc.SampleDesc.Count = 1;
        tdesc.SampleDesc.Quality = 0;
        tdesc.Usage = D3D11_USAGE_DEFAULT;
        tdesc.BindFlags = GetBindFlagsDX11(usage, format);
        tdesc.CPUAccessFlags = 0;
        tdesc.MiscFlags = 0;
        tdesc.Format = SafeGet(PixelFormatDX11, format);;

        std::unique_ptr<byte> dataByteRef;
        D3D11_SUBRESOURCE_DATA srd;
        if (data) {
            srd.pSysMem = TextureDataConverter(tdesc, format, data, dataByteRef);;
            srd.SysMemPitch = GetFormatByteSize(tdesc.Format) * tdesc.Width;
            srd.SysMemSlicePitch = 0;
        }

        ComPtr<ID3D11Texture2D> texture;

        DX11_CHECK_RET0(m_dev->CreateTexture2D(&tdesc, data ? &srd : NULL, &texture));
        D3D_SET_OBJECT_NAME_A(texture, debugName.c_str());

        ComPtr<ID3D11ShaderResourceView> srv;
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11DepthStencilView> dsv;

        if ((tdesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0) {
            D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
            viewDesc.Format = tdesc.Format;
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipLevels = 1;
            viewDesc.Texture2D.MostDetailedMip = 0;

            DX11_CHECK_RET0(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &srv));

            std::string tmp = debugName + "SRV";
            D3D_SET_OBJECT_NAME_A(srv, tmp.c_str());
        }

        if ((tdesc.BindFlags & D3D11_BIND_RENDER_TARGET) != 0) {
            DX11_CHECK_RET0(m_dev->CreateRenderTargetView(texture.Get(), NULL, &rtv));

            std::string tmp = debugName + "RTV";
            D3D_SET_OBJECT_NAME_A(rtv, tmp.c_str());
        }
        if ((tdesc.BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0) {
            // assert on shader_resource flag
            // todo: change and use typeless formats
            dg_assert_nm((tdesc.BindFlags == D3D11_BIND_DEPTH_STENCIL));
            DX11_CHECK_RET0(m_dev->CreateDepthStencilView(texture.Get(), NULL, &dsv));

            std::string tmp = debugName + "DSV";
            D3D_SET_OBJECT_NAME_A(dsv, tmp.c_str());
        }

        TextureDX11 *textureDX11 = new TextureDX11();
        textureDX11->texture.Swap(texture);
        textureDX11->sampler = m_defaultSampler;
        textureDX11->srv.Swap(srv);
        textureDX11->rtv.Swap(rtv);
        textureDX11->dsv.Swap(dsv);
        textureDX11->format = tdesc.Format;
        textureDX11->requestedFormat = format;
        textureDX11->width = width;
        textureDX11->height = height;
        return m_resourceManager->AddResource(textureDX11);
    }

    TextureId DX11Device::CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName) {
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
        tdesc.Format = SafeGet(PixelFormatDX11, format);

        ComPtr<ID3D11Texture2D> texture;

        DX11_CHECK_RET0(m_dev->CreateTexture2D(&tdesc, NULL, &texture));
        D3D_SET_OBJECT_NAME_A(texture, debugName.c_str());

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        viewDesc.Texture2DArray.MipLevels = levels;
        viewDesc.Texture2DArray.FirstArraySlice = 0;
        viewDesc.Texture2DArray.ArraySize = depth;

        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK_RET0(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &shaderResourceView));

        std::string tmp = debugName + "SRV";
        D3D_SET_OBJECT_NAME_A(shaderResourceView, tmp.c_str());

        TextureDX11 *textureDX11 = new TextureDX11();
        textureDX11->texture.Swap(texture);
        textureDX11->srv.Swap(shaderResourceView);
        textureDX11->sampler = m_defaultSampler;
        textureDX11->format = tdesc.Format;
        textureDX11->requestedFormat = format;
        textureDX11->width = width;
        textureDX11->height = height;
        return m_resourceManager->AddResource(textureDX11);
    }

    TextureId DX11Device::CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName) {
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
        tdesc.Format = SafeGet(PixelFormatDX11, format);

        std::unique_ptr<byte> dataByteRef[6];
        D3D11_SUBRESOURCE_DATA srd[6];

        if (data) {
            for (size_t i = 0; i < 6; ++i) {
                srd[i].pSysMem = reinterpret_cast<void*>(TextureDataConverter(tdesc, format, data[i], dataByteRef[i]));
                srd[i].SysMemPitch = GetFormatByteSize(tdesc.Format) * tdesc.Width;
                srd[i].SysMemSlicePitch = 0;
            }
        }

        ComPtr<ID3D11Texture2D> texture;

        DX11_CHECK_RET0(m_dev->CreateTexture2D(&tdesc, data ? &srd[0] : NULL, &texture));
        D3D_SET_OBJECT_NAME_A(texture, debugName.c_str());

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        viewDesc.TextureCube.MipLevels = tdesc.MipLevels;
        viewDesc.TextureCube.MostDetailedMip = 0;

        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK_RET0(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &shaderResourceView));
        
        std::string tmp = debugName + "SRV";
        D3D_SET_OBJECT_NAME_A(shaderResourceView, tmp.c_str());

        TextureDX11 *textureDX11 = new TextureDX11();
        textureDX11->texture.Swap(texture);
        textureDX11->srv.Swap(shaderResourceView);
        textureDX11->sampler = m_defaultSampler;
        textureDX11->format = tdesc.Format;
        textureDX11->requestedFormat = format;
        textureDX11->width = width;
        textureDX11->height = height;
        return  m_resourceManager->AddResource(textureDX11);
    }

    void* DX11Device::TextureDataConverter(const D3D11_TEXTURE2D_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef) {
        // currently we only care about converting 24 bit textures
        if (reqFormat == PixelFormat::RGB8Unorm) {
            size_t numPixels = tDesc.Width * tDesc.Height;
            dataRef.reset(new byte[numPixels * 4]);
            Convert24BitTo32Bit(reinterpret_cast<uintptr_t>(data), 
                reinterpret_cast<uintptr_t>(dataRef.get()), numPixels);
            return reinterpret_cast<void*>(dataRef.get());
        }
        return data;
    }

    RenderPassId DX11Device::CreateRenderPass(const RenderPassInfo& renderPassInfo) {
        RenderPassDX11* renderPass = new RenderPassDX11();
        renderPass->info = renderPassInfo;
        return m_resourceManager->AddResource(renderPass);
    }

    CommandBuffer* DX11Device::CreateCommandBuffer() {
        return dynamic_cast<CommandBuffer*>(new DX11CommandBuffer(m_dev, m_resourceManager, &m_cache));
    }

    void DX11Device::UpdateTexture(TextureId textureId, uint32_t slice, const void* srcData) {
        TextureDX11* texDX11 = m_resourceManager->GetResource<TextureDX11>(textureId);

        D3D11_BOX box = { 0 };
        box.back = 1;
        box.right = texDX11->width;
        box.bottom = texDX11->height;

        int formatByteSize = GetFormatByteSize(texDX11->format);
        assert(formatByteSize!=0);
        if (formatByteSize == 0) {
            LOG_E("DX11Device: got 0 for formatbytesize");
            return;
        }
        if (texDX11->requestedFormat == PixelFormat::RGB8Unorm) {
            LOG_E("DX11Device: unsupported pixelFormat update");
        }

        m_immediateContext->UpdateSubResource(texDX11->texture.Get(), slice, box, srcData, formatByteSize * texDX11->width, formatByteSize * texDX11->height * texDX11->width);
    }

    uint8_t* DX11Device::MapMemory(BufferId buffer, BufferAccess access) {
        if (access != BufferAccess::Write && access != BufferAccess::WriteNoOverwrite)
            assert(false);
        BufferDX11* bufferdx11 = m_resourceManager->GetResource<BufferDX11>(buffer);
        assert(bufferdx11);
        return static_cast<uint8_t*>(m_immediateContext->MapBufferPointer(bufferdx11->buffer.Get(), SafeGet(MapAccessDX11, access)));
    }

    void DX11Device::UnmapMemory(BufferId buffer) {
        BufferDX11* bufferdx11 = m_resourceManager->GetResource<BufferDX11>(buffer);
        assert(bufferdx11);
        m_immediateContext->UnMapBufferPointer(bufferdx11->buffer.Get());
    }

    void DX11Device::Submit(const std::vector<CommandBuffer*>& cmdBuffers) {
        for (CommandBuffer* cmdBuffer : cmdBuffers) {
            auto dx11cmdbuf = dynamic_cast<DX11CommandBuffer*>(cmdBuffer);
            auto cmdList = dx11cmdbuf->GetCmdList();
            dg_assert_nm(cmdList != nullptr);

            m_immediateContext->ExecuteCommandList(cmdList.Get(), FALSE);
            delete dx11cmdbuf;
        }
    }

    DX11Device::DX11Device(ResourceManager* resourceManager, bool usePrebuiltShaders)
        : m_resourceManager(resourceManager) {
        m_usePrebuiltShaders = usePrebuiltShaders;

        DeviceConfig.DeviceAbbreviation = "DX11";
        DeviceConfig.ShaderDir = "DX";
        if (m_usePrebuiltShaders)
            DeviceConfig.ShaderExtension = ".cso";
        else
            DeviceConfig.ShaderExtension = ".hlsl";

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

        ComPtr<ID3D11DeviceContext> context;
        DX11_CHECK_RET(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &m_dev, nullptr, &context));

        std::string tmp = "ImdCtx";
        D3D_SET_OBJECT_NAME_A(context, tmp.c_str());

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

        char *dxLevel = "Unknown";
        auto flvl = m_dev->GetFeatureLevel();
        switch (flvl) {
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1:
            dxLevel = "11.1";
            break;
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0:
            dxLevel = "11.0";
            break;
        case D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0:
            dxLevel = "10.0";
        }

        LOG_D("DirectX Feature Level: v%s", dxLevel);

        // spit out these cause im curious what cards support them
        D3D11_FEATURE_DATA_THREADING feat;
        m_dev->CheckFeatureSupport(D3D11_FEATURE_THREADING, &feat, sizeof(D3D11_FEATURE_DATA_THREADING));

        LOG_D("CommandListSupport: %s", feat.DriverCommandLists ? "True" : "False");
        LOG_D("ConcurrentCreateSupport: %s", feat.DriverConcurrentCreates ? "True" : "False");

        if (flvl < D3D_FEATURE_LEVEL_11_0) {
            D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS hwopts{};
            m_dev->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &hwopts, sizeof(hwopts));
            if (!hwopts.ComputeShaders_Plus_RawAndStructuredBuffers_Via_Shader_4_x)
            {
                LOG_E("No Compute Shader Support!");
            }
        }

        m_immediateContext.reset(new DX11Context(context));

        // todo: deal with this?
        CreateDefaultSampler();
    }
}