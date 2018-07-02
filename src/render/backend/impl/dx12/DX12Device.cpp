#include "DX12Device.h"
#include "DX12Assert.h"
#include "DX12EnumAdapter.h"

#include "SemanticNameCache.h"
#include "SimpleShaderLibrary.h"
#include "TexConvert.h"
#include "Memory.h"

#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include <d3d12.h>
#include "d3dx12.h"

#define SafeGet(id, idx) id[(uint32_t)idx]
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL    ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace gfx {

    BufferId DX12Device::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
        ComPtr<ID3D12Resource> resource;
        uint32_t unused = 0;
        uint32_t bufferSize = desc.size;

        switch (desc.usageFlags) {
        case BufferUsageFlags::VertexBufferBit:
            break;
        case BufferUsageFlags::IndexBufferBit:
            break;
        case BufferUsageFlags::ConstantBufferBit:
            bufferSize = (bufferSize + (D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1)) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
            break;
        default:
            assert(false);
        }

        // copypasterino
        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        DX12_CHECK_RET0(m_dev->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&resource)));

        if (initialData) {
            uint8_t* pData;
            CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
            DX12_CHECK_RET0(resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
            memcpy(pData, initialData, desc.size);
            resource->Unmap(0, nullptr);
        }

        BufferDX12 bufDx12; 

        bufDx12.accessFlags = desc.accessFlags;
        bufDx12.usageFlags = desc.usageFlags;
        bufDx12.buffer.Swap(resource);

        return GenerateHandleEmplaceConstRef<ResourceType::Buffer>(m_buffers, bufDx12);
    }

    ShaderId DX12Device::GetShader(ShaderType type, const std::string& functionName) {
        return m_shaderLibrary.GetShader(type, functionName);
    }

    void DX12Device::AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) {
        for (const ShaderData& shaderData : shaderData) {
            assert(shaderData.type == ShaderDataType::Source);

            std::string funcName = shaderData.name.substr(0, shaderData.name.length() - 5);
            {
                ShaderType shaderType = ShaderType::PixelShader;

                ShaderId existingShaderId = m_shaderLibrary.GetShader(shaderType, funcName);
                ShaderId newID = CreateShader(shaderType, std::string(reinterpret_cast<const char*>(shaderData.data), shaderData.len));

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
                ShaderId newID = CreateShader(shaderType, std::string(reinterpret_cast<const char*>(shaderData.data), shaderData.len));

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

    ShaderId DX12Device::CreateShader(ShaderType type, const std::string& source) {
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
            LOG_E("DX12Device: bufPtr is 0 in CreateShader");
            return 0;
        }

        ShaderDX12* shaderDX12 = new ShaderDX12();
        shaderDX12->shaderType = type;
        shaderDX12->blob.Swap(blob);
        return GenerateHandleEmplaceConstRef<ResourceType::Shader>(m_shaders, *shaderDX12);
    }

    ComPtr<ID3DBlob> DX12Device::CompileShader(ShaderType shaderType, const std::string& source) {
        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> errorBlob;
        char *entryPoint;
        char *target;

        HRESULT hr;

        // ---- Compile Shader Source 
        switch (shaderType) {
        case ShaderType::PixelShader:
            entryPoint = "PSMain";
            target = "ps_5_0";
            break;
        case ShaderType::VertexShader:
            entryPoint = "VSMain";
            target = "vs_5_0";
            break;
        default:
            LOG_E("DX12Device: Unsupported shader type supplied. Type: %d", shaderType);
            return blob;
        }

        uint32_t flags = 0;
#ifdef DEBUG_DX12
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;
#endif 

        hr = D3DCompile(&source[0], source.length(), NULL, NULL, NULL, entryPoint, target, flags, 0, &blob, &errorBlob);

        if (FAILED(hr)) {
            if (errorBlob) {
                LOG_E("DX12Device: Failed to Compile Shader. Error: %s", errorBlob->GetBufferPointer());
            }
            else {
                LOG_E("DX12Device: Failed to compile Shader. Hr: 0x%x", hr);
            }
            return blob;
        }
        return blob;
    }

    VertexLayoutId DX12Device::CreateVertexLayout(const VertexLayoutDesc& layoutDesc) {
        // not internal, check hash first
        VertexLayoutId hash = 0;
        HashCombine(hash, layoutDesc);
        auto layoutCheck = GetResource(m_inputLayouts, hash);
        if (layoutCheck != nullptr)
            return hash;

        bool first = true;
        uint32_t stride = 0;
        InputLayoutDX12 il;
        for (auto layout : layoutDesc.elements) {
            D3D12_INPUT_ELEMENT_DESC ied;
            ied.SemanticName = SemanticNameCache::AddGetSemanticNameToCache(VertexAttributeUsageToString(layout.usage).c_str());
            ied.SemanticIndex = 0;
            ied.InputSlot = 0;
            ied.AlignedByteOffset = first ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;
            ied.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            ied.InstanceDataStepRate = 0;
            ied.Format = SafeGet(VertexAttributeTypeDX12, layout.type);
            first = false;
            stride += GetByteCount(layout);
            il.elements.emplace_back(ied);
        }
        il.stride = stride;
        return UseHandleEmplaceConstRef(m_inputLayouts, hash, il);
    }

    TextureId DX12Device::CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* data) {
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = SafeGet(PixelFormatDX12, format);
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        ComPtr<ID3D12Resource> resource;

        DX12_CHECK_RET0(m_dev->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())));

        std::unique_ptr<byte> dataByteRef;
        D3D12_SUBRESOURCE_DATA srd;
        if (data) {
            srd.pData = TextureDataConverter(texDesc, format, data, dataByteRef);
            srd.RowPitch = GetFormatByteSize(texDesc.Format) * texDesc.Width;
            srd.SlicePitch = 0;// srd.RowPitch * height;
        }

        //CommandContext::InitializeTexture(*this, 1, &texResource);

        D3D12_CPU_DESCRIPTOR_HANDLE descHandle;

        // stupid heap

        /*if (descHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            descHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_dev->CreateShaderResourceView(m_pResource.Get(), nullptr, m_hCpuDescriptorHandle);*/
    }

    D3D12_RASTERIZER_DESC DX12Device::CreateRasterState(const RasterState& state) {
        D3D12_RASTERIZER_DESC rasterDesc;
        rasterDesc.AntialiasedLineEnable = true;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.MultisampleEnable = true;
        rasterDesc.SlopeScaledDepthBias = 0.0f;
        rasterDesc.ForcedSampleCount = 0;
        rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        rasterDesc.CullMode = SafeGet(CullModeDX12, state.cullMode);
        rasterDesc.FillMode = SafeGet(FillModeDX12, state.fillMode);
        rasterDesc.FrontCounterClockwise = state.windingOrder == WindingOrder::FrontCCW ? true : false;
        return rasterDesc;
    }

    D3D12_BLEND_DESC DX12Device::CreateBlendState(const BlendState& state) {
        D3D12_BLEND_DESC blendDesc{ 0 };
        blendDesc.IndependentBlendEnable = false;
        blendDesc.AlphaToCoverageEnable = 0;

        blendDesc.RenderTarget[0].BlendEnable = (state.enable) ? TRUE : FALSE;
        blendDesc.RenderTarget[0].SrcBlendAlpha = SafeGet(BlendFuncDX12, state.srcAlphaFunc);
        blendDesc.RenderTarget[0].DestBlendAlpha = SafeGet(BlendFuncDX12, state.dstAlphaFunc);
        blendDesc.RenderTarget[0].SrcBlend = SafeGet(BlendFuncDX12, state.srcRgbFunc);
        blendDesc.RenderTarget[0].DestBlend = SafeGet(BlendFuncDX12, state.dstRgbFunc);
        blendDesc.RenderTarget[0].BlendOpAlpha = SafeGet(BlendModeDX12, state.alphaMode);
        blendDesc.RenderTarget[0].BlendOp = SafeGet(BlendModeDX12, state.rgbMode);

        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        return blendDesc;
    }

    D3D12_DEPTH_STENCIL_DESC DX12Device::CreateDepthState(const DepthState& state) {
        D3D12_DEPTH_STENCIL_DESC depthState{ 0 };

        depthState.StencilReadMask = 0xFF;
        depthState.StencilWriteMask = 0xFF;

        depthState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_INCR;
        depthState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        depthState.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        depthState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_DECR;
        depthState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depthState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        depthState.StencilEnable = state.enable;
        depthState.DepthEnable = state.enable;        
        depthState.DepthWriteMask = SafeGet(DepthWriteMaskDX12, state.depthWriteMask);
        depthState.DepthFunc = SafeGet(DepthFuncDX12, state.depthFunc);

        return depthState;
    }

    PipelineStateId DX12Device::CreatePipelineState(const PipelineStateDesc& desc) {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        InputLayoutDX12* layout = GetResource(m_inputLayouts, desc.vertexLayout);
        assert(layout);
        psoDesc.InputLayout = { layout->elements.data(), static_cast<UINT>(layout->elements.size())  };

        ShaderDX12* ps = GetResource(m_shaders, desc.pixelShader);
        assert(ps);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->blob.Get());

        ShaderDX12* vs = GetResource(m_shaders, desc.vertexShader);
        assert(vs);
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->blob.Get());

        // DONT KNOW WHAT TO DO HERE YET
        //psoDesc.pRootSignature = m_rootSignature.Get();

        psoDesc.RasterizerState = CreateRasterState(desc.rasterState);
        psoDesc.BlendState = CreateBlendState(desc.blendState);
        psoDesc.DepthStencilState = CreateDepthState(desc.depthState);
        
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = SafeGet(PrimitiveTypeDX12, desc.topology);
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        ComPtr<ID3D12PipelineState> pipelineState;

        DX12_CHECK_RET0(m_dev->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

        PipelineStateDX12* pso = new PipelineStateDX12();;
        pso->primitiveType = desc.topology;
        pso->pipelineState.Swap(pipelineState);

        return GenerateHandleEmplaceConstRef<ResourceType::PipelineState>(m_pipelinestates, *pso);
    }

    void* DX12Device::TextureDataConverter(const D3D12_RESOURCE_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef) {
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

    uint8_t* DX12Device::MapMemory(BufferId buffer, BufferAccess access) {
        if (access != BufferAccess::Write && access != BufferAccess::WriteInit)
            assert(false);

        BufferDX12* bufferDX12 = GetResource(m_buffers, buffer);

        uint8_t* pData;
        CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
        DX12_CHECK_RET0(bufferDX12->buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
        return static_cast<uint8_t*>(pData);
    }

    void DX12Device::UnmapMemory(BufferId buffer) {
        BufferDX12* bufferDX12 = GetResource(m_buffers, buffer);
        assert(bufferDX12);
        bufferDX12->buffer->Unmap(0, nullptr);
    }

    void DX12Device::RenderFrame() {
        for (uint32_t idx = 0; idx < m_submittedBuffers.size(); ++idx) {
            CommandBuffer* dx12Buffer = reinterpret_cast<CommandBuffer*>(m_submittedBuffers[idx]);
            //Execute(dx12Buffer);
            dx12Buffer->Reset();
        }

        DX12_CHECK(m_swapchain->Present(1, 0));

        m_submittedBuffers.clear();
        m_drawItemByteBuffer.Reset();
    }

    int32_t DX12Device::InitializeDevice(const DeviceInitialization& deviceInit) {
        m_winWidth = deviceInit.windowWidth;
        m_winHeight = deviceInit.windowHeight;
        m_usePrebuiltShaders = deviceInit.usePrebuiltShaders;

		DeviceConfig.DeviceAbbreviation = "DX12";
		DeviceConfig.ShaderDir = "DX";

        if (m_usePrebuiltShaders) {
            DeviceConfig.ShaderExtension = ".cso";
        }
        else {
            DeviceConfig.ShaderExtension = ".hlsl";
        }

#ifdef DEBUG_DX12
        // Enable the D3D12 debug layer.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
            }
        }
#endif

        DX12_CHECK_RET0(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));

        ComPtr<IDXGIAdapter1> adapter;

        DX12_CHECK_RET0(D3D12CreateDevice(
            adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_dev)
        ));


        // Sanity check, just want the highest versions for now, not sure if 1.0 will ignore
        //   but it looks like 1.1 gives us more flags n such for driver optimizations,
        //   also, if i remember, i think the d3dx12 will convert in code definitions for us automatically
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        DX12_CHECK_RET0(m_dev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        DX12_CHECK_RET0(m_dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Width = m_winWidth;
        swapChainDesc.Height = m_winHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        DX12_CHECK_RET0(m_factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
            static_cast<HWND>(deviceInit.windowHandle),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain
        ));

        // disable fullscreen transitions.
        DX12_CHECK_RET0(m_factory->MakeWindowAssociation(static_cast<HWND>(deviceInit.windowHandle), DXGI_MWA_NO_ALT_ENTER));

        DX12_CHECK_RET0(swapChain.As(&m_swapchain));
        m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DX12_CHECK_RET0(m_dev->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create frame resources.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV and a command allocator for each frame.
        for (uint32_t n = 0; n < FrameCount; n++)
        {
            DX12_CHECK_RET0(m_swapchain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_dev->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);

            DX12_CHECK_RET0(m_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
        }

        m_drawItemByteBuffer.Resize(memory::KilobytesToBytes(1));

        return 1;
    }

    void DX12Device::PrintDisplayAdapterInfo() {
        ComPtr<IDXGIAdapter> adapter;
        DX12_CHECK(m_factory->EnumAdapters(0, &adapter));
        auto adapterDesc = DXGI_ADAPTER_DESC();
        DX12_CHECK(adapter->GetDesc(&adapterDesc));

        char buffer[128];
        wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);

        LOG_D("DisplayAdaterDesc: %s", buffer);
        LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
    }

    DX12Device::~DX12Device() {
        m_dev.Reset();
        m_swapchain.Reset();
        CloseHandle(m_fenceEvent);
    }
}