#include "DX11Device.h"
#include "Log.h"
#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include "EnumTraits.h"
#include "SimpleShaderLibrary.h"

#include "DX11Utils.h"
#include "Memory.h"
#include "DrawItemDecoder.h"

#define SafeGet(id, idx) id[(uint32_t)idx]

namespace gfx {
    std::vector<std::unique_ptr<const char[]>> DX11SemanticNameCache::m_semanticNameCache = {};

    BufferId DX11Device::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
        ComPtr<ID3D11Buffer> buffer = NULL;

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
        case BufferUsageFlags::VertexBufferBit:
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case BufferUsageFlags::IndexBufferBit:
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case BufferUsageFlags::ConstantBufferBit:
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
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

        bufferDesc.ByteWidth = static_cast<UINT>(buffSize);

        if (bufferDesc.Usage == D3D11_USAGE_DYNAMIC)
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        else
            bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        if (initialData) {
            initData.pSysMem = initialData;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;
        }

        DX11_CHECK_RET0(m_dev->CreateBuffer(&bufferDesc, initialData ? &initData : NULL, &buffer));
        BufferDX11 bufferdx11;
        bufferdx11.buffer.Swap(buffer);
        return GenerateHandleEmplaceConstRef<ResourceType::Buffer>(m_buffers, bufferdx11);
    }

    ShaderLibrary* DX11Device::CreateShaderLibrary(const std::vector<ShaderDataDesc>& dataDescs) {
        SimpleShaderLibrary* lib = new SimpleShaderLibrary();

        for (const ShaderDataDesc& dataDesc : dataDescs) {
            const ShaderData& shaderData = dataDesc.data;
            assert(shaderData.type == ShaderDataType::Source);

            for (const ShaderFunctionDesc& function : dataDesc.functions) {
                ShaderId shaderId = CreateShader(function, shaderData);
                assert(shaderId);
                lib->AddShader(shaderId, function);
            }
        }

        m_libraries.push_back(lib);
        return lib;
    }


    ShaderId DX11Device::CreateShader(const ShaderFunctionDesc& funcDesc, const ShaderData& shaderData) {
        assert(shaderData.type == ShaderDataType::Source);
        return CreateShader(funcDesc.type, reinterpret_cast<const char*>(shaderData.data));
    }

    ShaderId DX11Device::CreateShader(ShaderType type, const std::string& source) {
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
            m_lastCompiledVertexShader.Swap(blob);
            return GenerateHandleEmplaceConstRef<ResourceType::Shader>(m_shaders, *shaderDX11);
            break;
        }
        case ShaderType::PixelShader:
        {
            ID3D11PixelShader* pixelShader;
            DX11_CHECK_RET0(m_dev->CreatePixelShader(bufPtr, bufSize, NULL, &pixelShader));

            shaderDX11->pixelShader = pixelShader;
            return GenerateHandleEmplaceConstRef<ResourceType::Shader>(m_shaders, *shaderDX11);
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

        HRESULT hr;

        // ---- Compile Shader Source 
        switch (shaderType) {
        case ShaderType::PixelShader:
            entryPoint = "PSMain";
            target = "ps_4_0";
            break;
        case ShaderType::VertexShader:
            entryPoint = "VSMain";
            target = "vs_4_0";
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

    ShaderParamId DX11Device::CreateShaderParam(ShaderId shader, const char* param, ParamType paramType) {
        CBufDescEntry entry;
        entry.shaderId = shader;
        entry.name = std::string(param);
        entry.type = paramType;

        ShaderParamId shaderParamId = GenerateHandleEmplaceConstRef<ResourceType::Buffer>(m_cBufferParams, entry);

        auto cbufferdx11 = m_shaderCBufferMapping.find(shader);
        if (cbufferdx11 == m_shaderCBufferMapping.end()) {
            ConstantBufferDX11 newCbufferDx11;
            newCbufferDx11.cBufferDescs.emplace_back(&m_cBufferParams[shaderParamId]);
            m_shaderCBufferMapping[shader] = newCbufferDx11;
        }
        else {
            cbufferdx11->second.sizeChanged = true;
            cbufferdx11->second.cBufferDescs.emplace_back(&m_cBufferParams[shaderParamId]);
        }
        return shaderParamId;
    }

    PipelineStateId DX11Device::CreatePipelineState(const PipelineStateDesc & desc) {
        PipelineStateDX11 stateDx11;

        ShaderDX11* ps = GetResource(m_shaders, desc.pixelShader);
        stateDx11.pixelShader = ps->pixelShader;
        stateDx11.pixelShaderHandle = desc.pixelShader;

        ShaderDX11* vs = GetResource(m_shaders, desc.vertexShader);
        stateDx11.vertexShader = vs->vertexShader;
        stateDx11.vertexShaderHandle = desc.vertexShader;

        stateDx11.topology = SafeGet(PrimitiveTypeDX11, desc.topology);

        auto layout = GetResourceFromSizeMap(m_inputLayouts, desc.vertexLayout);
        stateDx11.vertexLayout = layout->inputLayout.Get();
        stateDx11.vertexLayoutHandle = desc.vertexLayout;
        stateDx11.vertexLayoutStride = layout->stride;

        size_t hash = 0;
        HashCombine(hash, desc.blendState);
        auto blendState = m_blendStates.find(hash);
        if (blendState == m_blendStates.end()) {
            stateDx11.blendState = CreateBlendState(desc.blendState);
        }
        else {
            stateDx11.blendState = blendState->second.Get();
        }
        stateDx11.blendStateHandle = hash;

        hash = 0;
        HashCombine(hash, desc.depthState);
        auto depthState = m_depthStates.find(hash);
        if (depthState == m_depthStates.end()) {
            stateDx11.depthState = CreateDepthState(desc.depthState);
        }
        else {
            stateDx11.depthState = depthState->second.Get();
        }
        stateDx11.depthStateHandle = hash;

        hash = 0;
        HashCombine(hash, desc.rasterState);
        auto rasterState = m_rasterStates.find(hash);
        if (rasterState == m_rasterStates.end()) {
            stateDx11.rasterState = CreateRasterState(desc.rasterState);
        }
        else {
            stateDx11.rasterState = rasterState->second.Get();
        }
        stateDx11.rasterStateHandle = hash;

        return GenerateHandleEmplaceConstRef<ResourceType::PipelineState>(m_pipelineStates, stateDx11);
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

        ComPtr<ID3D11DepthStencilState> depthStateDX;
        DX11_CHECK_RET0(m_dev->CreateDepthStencilState(&dsDesc, &depthStateDX));
        size_t hash = 0;
        HashCombine(hash, state);
        UseHandleEmplaceConstRef<ComPtr<ID3D11DepthStencilState>>(m_depthStates, hash, depthStateDX.Get());
        return depthStateDX.Get();
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

        ComPtr<ID3D11RasterizerState> rasterStateDX;
        DX11_CHECK_RET0(m_dev->CreateRasterizerState(&rasterDesc, &rasterStateDX));
        size_t hash = 0;
        HashCombine(hash, state);
        UseHandleEmplaceConstRef<ComPtr<ID3D11RasterizerState>>(m_rasterStates, hash, rasterStateDX.Get());
        return rasterStateDX.Get();
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

        ComPtr<ID3D11BlendState> blendStateDX;
        DX11_CHECK_RET0(m_dev->CreateBlendState(&blendDesc, &blendStateDX));
        size_t hash = 0;
        HashCombine(hash, state);
        UseHandleEmplaceConstRef<ComPtr<ID3D11BlendState>>(m_blendStates, hash, blendStateDX.Get());
        return blendStateDX.Get();
    }

    void DX11Device::CreateSetDefaultSampler() {
        D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
        //samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        DX11_CHECK(m_dev->CreateSamplerState(&samplerDesc, &m_defaultSampler));
    }

    VertexLayoutId DX11Device::CreateVertexLayout(const VertexLayoutDesc& layoutDesc) {
        // not internal, check hash first
        size_t hash = 0;
        HashCombine(hash, layoutDesc);
        auto layoutCheck = GetResourceFromSizeMap(m_inputLayouts, hash);
        if (layoutCheck != nullptr)
            return hash;

        bool first = true;
        std::vector<D3D11_INPUT_ELEMENT_DESC> ieds;
        uint32_t stride = 0;
        for (auto layout : layoutDesc.elements) {
            D3D11_INPUT_ELEMENT_DESC ied;
            ied.SemanticName = DX11SemanticNameCache::AddGetSemanticNameToCache(VertexAttributeUsageToString(layout.usage).c_str());
            ied.SemanticIndex = 0;
            ied.InputSlot = 0;
            ied.AlignedByteOffset = first ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
            ied.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            ied.InstanceDataStepRate = 0;
            ied.Format = SafeGet(VertexAttributeTypeDX11, layout.type);
            stride += GetByteCount(layout.type);
            first = false;
            ieds.emplace_back(ied);
        }
        ComPtr<ID3D11InputLayout> inputLayout;
        // screw it, here we loop over every shader and try to find the stupid one that 
        //  has the same inputlayout
        
        for (auto shader : m_shaders) {
            HRESULT hr = m_dev->CreateInputLayout(ieds.data(), static_cast<UINT>(ieds.size()), 
                m_lastCompiledVertexShader->GetBufferPointer(), m_lastCompiledVertexShader->GetBufferSize(), &inputLayout);

            if (hr == 0) {
                break;
            }
        }
        assert(inputLayout.Get() != 0);
        InputLayoutDX11 layout;
        layout.inputLayout.Swap(inputLayout);
        layout.stride = stride;

        return UseHandleEmplaceConstRef(m_inputLayouts, hash, layout);
    }

    TextureId DX11Device::CreateTexture2D(TextureFormat format, uint32_t width, uint32_t height, void* data) {
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
        tdesc.Format = SafeGet(TextureFormatDX11, format);;

        std::unique_ptr<byte> dataByteRef;
        D3D11_SUBRESOURCE_DATA srd;
        if (data) {
            srd.pSysMem = TextureDataConverter(tdesc, format, data, dataByteRef);;
            srd.SysMemPitch = GetFormatByteSize(tdesc.Format) * tdesc.Width;
            srd.SysMemSlicePitch = 0;
        }

        ComPtr<ID3D11Texture2D> texture;

        DX11_CHECK_RET0(m_dev->CreateTexture2D(&tdesc, data ? &srd : NULL, &texture));

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipLevels = 1;
        viewDesc.Texture2D.MostDetailedMip = 0;

        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &shaderResourceView));
        if (!shaderResourceView) {
            texture.Reset();
            return 0;
        }

        TextureDX11 textureDX11 = {};
        textureDX11.texture.Swap(texture);
        textureDX11.shaderResourceView.Swap(shaderResourceView);
        textureDX11.format = tdesc.Format;
        textureDX11.requestedFormat = format;
        return GenerateHandleEmplaceConstRef<ResourceType::Texture>(m_textures, textureDX11);
    }

    TextureId DX11Device::CreateTextureArray(TextureFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) {
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
        tdesc.Format = SafeGet(TextureFormatDX11, format);

        ComPtr<ID3D11Texture2D> texture;

        DX11_CHECK_RET0(m_dev->CreateTexture2D(&tdesc, NULL, &texture));

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        viewDesc.Texture2DArray.MipLevels = levels;
        viewDesc.Texture2DArray.FirstArraySlice = 0;
        viewDesc.Texture2DArray.ArraySize = depth;

        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &shaderResourceView));
        if (!shaderResourceView) {
            texture.Reset();
            return 0;
        }

        TextureDX11 textureDX11 = {};
        textureDX11.texture.Swap(texture);
        textureDX11.shaderResourceView.Swap(shaderResourceView);
        textureDX11.format = tdesc.Format;
        textureDX11.requestedFormat = format;
        return GenerateHandleEmplaceConstRef<ResourceType::Texture>(m_textures, textureDX11);
    }

    TextureId DX11Device::CreateTextureCube(TextureFormat format, uint32_t width, uint32_t height, void** data) {
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
        tdesc.Format = SafeGet(TextureFormatDX11, format);

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

        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = tdesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        viewDesc.TextureCube.MipLevels = tdesc.MipLevels;
        viewDesc.TextureCube.MostDetailedMip = 0;

        ComPtr<ID3D11ShaderResourceView> shaderResourceView;
        DX11_CHECK(m_dev->CreateShaderResourceView(texture.Get(), &viewDesc, &shaderResourceView));
        if (!shaderResourceView) {
            texture.Reset();
            return 0;
        }

        TextureDX11 textureDX11 = {};
        textureDX11.texture.Swap(texture);
        textureDX11.shaderResourceView.Swap(shaderResourceView);
        textureDX11.format = tdesc.Format;
        textureDX11.requestedFormat = format;
        return GenerateHandleEmplaceConstRef<ResourceType::Texture>(m_textures, textureDX11);
    }

    void* DX11Device::TextureDataConverter(const D3D11_TEXTURE2D_DESC& tDesc, TextureFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef) {
        // currently we only care about converting 24 bit textures
        if (reqFormat == TextureFormat::RGB_U8) {
            size_t numPixels = tDesc.Width * tDesc.Height;
            dataRef.reset(new byte[numPixels * 4]);
            Convert24BitTo32Bit(reinterpret_cast<uintptr_t>(data), 
                reinterpret_cast<uintptr_t>(dataRef.get()), numPixels);
            return reinterpret_cast<void*>(dataRef.get());
        }
        return data;
    }

    CommandBuffer* DX11Device::CreateCommandBuffer() {
        DX11CommandBuffer* cmdBuffer = m_commandBufferPool.Get();
        assert(cmdBuffer);
        cmdBuffer->Reset();
        return cmdBuffer;
    }

    uint8_t* DX11Device::MapMemory(BufferId buffer, BufferAccess access) {
        if (access != BufferAccess::Write && access != BufferAccess::WriteInit)
            assert(false);
        BufferDX11* bufferdx11 = GetResourceFromSizeMap(m_buffers, buffer);
        assert(bufferdx11);
        return static_cast<uint8_t*>(m_context->MapBufferPointer(bufferdx11->buffer.Get(), SafeGet(MapAccessDX11, access)));
    }

    void DX11Device::UnmapMemory(BufferId buffer) {
        BufferDX11* bufferdx11 = GetResourceFromSizeMap(m_buffers, buffer);
        assert(bufferdx11);
        m_context->UnMapBufferPointer(bufferdx11->buffer.Get());
    }

    void DX11Device::Submit(const std::vector<CommandBuffer*>& cmdBuffers) {
        m_submittedBuffers.insert(end(m_submittedBuffers), begin(cmdBuffers), end(cmdBuffers));
    }

    void DX11Device::Execute(DX11CommandBuffer* cmdBuffer) {
        ByteBuffer& _byteBuffer = cmdBuffer->GetByteBuffer();
        DX11CommandBuffer::CommandType cmd;
        while (_byteBuffer.ReadPos() < _byteBuffer.WritePos()) {
            _byteBuffer >> cmd;
            switch (cmd) {
            case DX11CommandBuffer::CommandType::DrawItem: {
                const DrawItem* item;
                _byteBuffer >> item;

                DrawItemDecoder decoder(item);

                PipelineStateId psId;
                DrawCall drawCall;
                BufferId indexBufferId;
                size_t streamCount = decoder.GetStreamCount();
                size_t bindingCount = decoder.GetBindingCount();
                std::vector<VertexStream> streams(streamCount);
                std::vector<Binding> bindings(bindingCount);
                VertexStream* streamPtr = streams.data();
                Binding* bindingPtr = bindings.data();

                assert(streamCount == 1); // > 1 not supported

                assert(decoder.ReadDrawCall(&drawCall));
                assert(decoder.ReadPipelineState(&psId));
                assert(decoder.ReadIndexBuffer(&indexBufferId));
                assert(decoder.ReadVertexStreams(&streamPtr));
                if (bindingCount > 0) {
                    assert(decoder.ReadBindings(&bindingPtr));
                }

            
                PipelineStateDX11* pipelineState = GetResourceFromSizeMap(m_pipelineStates, psId);
                SetPipelineState(*pipelineState);

                assert(streamCount == 1); // > 1 not supported
                const VertexStream& stream = streamPtr[0];

                auto vertexBuffer = GetResource(m_buffers, stream.vertexBuffer);
                if (vertexBuffer)
                    m_context->SetVertexBuffer(stream.vertexBuffer, vertexBuffer->buffer.Get());

                // bindings
                for (uint32_t idx = 0; idx <bindingCount; ++idx) {
                    const Binding& binding = bindingPtr[idx];
                    switch (binding.type) {
                    case Binding::Type::ConstantBuffer: {
                        BufferDX11* cbuffer = GetResource(m_buffers, binding.resource);
                        m_context->SetVertexCBuffer(binding.resource, binding.slot, cbuffer->buffer.Get());
                        m_context->SetPixelCBuffer(binding.resource, binding.slot, cbuffer->buffer.Get());
                        break;
                    }
                    case Binding::Type::Texture: {
                        TextureDX11* texturedx11 = GetResource(m_textures, binding.resource);
                        m_context->SetVertexShaderTexture(binding.slot, texturedx11->shaderResourceView.Get(), m_defaultSampler.Get());
                        m_context->SetPixelShaderTexture(binding.slot, texturedx11->shaderResourceView.Get(), m_defaultSampler.Get());
                        break;
                    }
                    }
                }
                
                switch (drawCall.type) {
                case DrawCall::Type::Arrays: {
                    m_context->DrawPrimitive(pipelineState->topology, drawCall.offset, drawCall.primitiveCount, false);
                    break;
                }
                case DrawCall::Type::Indexed: {
                    assert(indexBufferId);
                    auto indexBuffer = GetResource(m_buffers, indexBufferId);
                    m_context->SetIndexBuffer(indexBufferId, indexBuffer->buffer.Get());
                    m_context->DrawPrimitive(pipelineState->topology, drawCall.offset, drawCall.primitiveCount, true);
                    break;
                }
                }
                break;
            }
            case DX11CommandBuffer::CommandType::Clear: {
                m_context->Clear(0.f, 0.f, 0.f, 0.f);
                break;
            }
            case DX11CommandBuffer::CommandType::BindResource: {
                Binding binding;
                _byteBuffer >> binding;

                // TODO:: this is copy pasted from above, dont do that.
                switch (binding.type) {
                case Binding::Type::ConstantBuffer: {
                    BufferDX11* cbuffer = GetResource(m_buffers, binding.resource);
                    m_context->SetVertexCBuffer(binding.resource, binding.slot, cbuffer->buffer.Get());
                    m_context->SetPixelCBuffer(binding.resource, binding.slot, cbuffer->buffer.Get());
                    break;
                }
                case Binding::Type::Texture: {
                    TextureDX11* texturedx11 = GetResource(m_textures, binding.resource);
                    m_context->SetVertexShaderTexture(binding.slot, texturedx11->shaderResourceView.Get(), m_defaultSampler.Get());
                    m_context->SetPixelShaderTexture(binding.slot, texturedx11->shaderResourceView.Get(), m_defaultSampler.Get());
                    break;
                }
                }
                break;
            }
            }
        }
    }

    void DX11Device::SetPipelineState(const PipelineStateDX11& state) {
        m_context->SetDepthState(state.depthStateHandle, state.depthState);
        m_context->SetRasterState(state.rasterStateHandle, state.rasterState);
        m_context->SetBlendState(state.blendStateHandle, state.blendState);

        m_context->SetVertexShader(state.vertexShaderHandle, state.vertexShader);
        m_context->SetPixelShader(state.pixelShaderHandle, state.pixelShader);
        
        m_context->SetInputLayout(state.vertexLayoutHandle, state.vertexLayoutStride, state.vertexLayout);
    }

    void DX11Device::DestroyResource(ResourceId resourceId) {
        size_t key = ExtractResourceKey(resourceId);
        switch (ExtractResourceType(resourceId)) {
        case ResourceType::Shader: {
            DestroyShader(key);
            break;
        }
        default:
            assert(false); // worry about this when we actually need ot
        }
    }

    void DX11Device::RenderFrame() {
        //m_context->Clear(0.f, 0.f, 0.f, 0.f);
        for (uint32_t idx = 0; idx < m_submittedBuffers.size(); ++idx) {
            DX11CommandBuffer* dx11Buffer = reinterpret_cast<DX11CommandBuffer*>(m_submittedBuffers[idx]);
            Execute(dx11Buffer);
            dx11Buffer->Reset();
        }

        DX11_CHECK(m_swapchain->Present(1, 0));

        m_submittedBuffers.clear();
        m_drawItemByteBuffer.Reset();
    }

    void DX11Device::ResetDepthStencilTexture() {
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

    void DX11Device::ResetViewport() {
        D3D11_VIEWPORT vp;
        vp.Width = (float)m_winWidth;
        vp.Height = (float)m_winHeight;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        m_context->SetViewport(1, vp);
    }

    void DX11Device::ResizeWindow(uint32_t width, uint32_t height) {
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
        m_context->SetRenderTarget(m_renderTarget.Get(), m_depthStencilView.Get());
    }

    int32_t DX11Device::InitializeDevice(const DeviceInitialization & deviceInit) {
        m_winWidth = deviceInit.windowWidth;
        m_winHeight = deviceInit.windowHeight;
        m_usePrebuiltShaders = deviceInit.usePrebuiltShaders;

        if (m_usePrebuiltShaders) {
            DeviceConfig.DeviceAbbreviation = "DX11";
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

        ComPtr<ID3D11DeviceContext> context;
#ifdef DX11_3_API
        // k for uwp, we have to make 11 dev and then upgrade them to the newer versions
        ComPtr<ID3D11Device> device;
        DX11_CHECK_RET0(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, FeatureLevelsRequested, numLevelsRequested, D3D11_SDK_VERSION,
            &device, nullptr, &context));

        DX11_CHECK_RET0(device.As(&m_dev));

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
            reinterpret_cast<IUnknown*>(deviceInit.windowHandle),
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
            &m_dev, nullptr, &context));

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
        std::memset(&sd, 0, sizeof(sd));
        sd.BufferDesc.Width = m_winWidth;
        sd.BufferDesc.Height = m_winHeight;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.BufferCount = 2;
        sd.OutputWindow = static_cast<HWND>(deviceInit.windowHandle);
        sd.Windowed = true;
        DX11_CHECK_RET0(m_factory->CreateSwapChain(m_dev.Get(), &sd, &m_swapchain));
#endif

        m_context.reset(new DX11Context(context.Get()));

        ComPtr<ID3D11Texture2D> backbuffer;
        DX11_CHECK_RET0(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));

        DX11_CHECK_RET0(m_dev->CreateRenderTargetView(backbuffer.Get(), nullptr, m_renderTarget.GetAddressOf()));

        // Create and use default rasterize / depth
        size_t hash = 0;
        HashCombine(hash, RasterState());
        m_context->SetRasterState(hash, CreateRasterState(RasterState()));
        hash = 0;
        HashCombine(hash, DepthState());
        m_context->SetDepthState(hash, CreateDepthState(DepthState()));

        ResetDepthStencilTexture();
        ResetViewport();

        m_context->SetRenderTarget(m_renderTarget.Get(), m_depthStencilView.Get());

        // todo: deal with this?
        CreateSetDefaultSampler();

        m_drawItemByteBuffer.Resize(memory::KilobytesToBytes(1));

        return 1;
    }

    void DX11Device::PrintDisplayAdapterInfo() {
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
    DX11Device::~DX11Device() {
        m_dev.Reset();
        m_swapchain.Reset();
        m_factory.Reset();
        m_depthStencilView.Reset();
        m_renderTarget.Reset();
    }
}