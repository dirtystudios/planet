#define NOMINMAX
#include "DX12Device.h"
#include "DX12Assert.h"
#include "DX12EnumAdapter.h"
#include "DX12CommandBuffer.h"

#include "SemanticNameCache.h"
#include "SimpleShaderLibrary.h"
#include "TexConvert.h"
#include "Memory.h"
#include "ResourceManager.h"

#include "DrawItemDecoder.h"

#include <d3dcompiler.h>
#include <d3dcompiler.inl>

#include <d3d12.h>
#include "d3dx12.h"

#define SafeGet(id, idx) id[(uint32_t)idx]
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL    ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

namespace gfx {

    BufferId DX12Device::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
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

        ComPtr<ID3D12Resource> resource;
        ComPtr<ID3D12Resource> cpubuffer;
        D3DX12Residency::ManagedObject managedObject;
        // use upload heap for constant buffers, and the dedicated upload pipeline for any others
        // todo: it may be beneficial to support using this heap for others as well, but we may need a new flag?
        if (desc.usageFlags == BufferUsageFlags::ConstantBufferBit) {
            DX12_CHECK_RET0(m_dev->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&resource)));

            managedObject.Initialize(resource.Get(), bufferSize);
            m_residencyManager.BeginTrackingObject(&managedObject);

            if (initialData) {
                uint8_t* pData;
                CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
                DX12_CHECK_RET0(resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
                memcpy(pData, initialData, desc.size);
                resource->Unmap(0, nullptr);
            }
        }
        else {
            DX12_CHECK_RET0(m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)));

            ComPtr<ID3D12Resource> uploadBuffer;
            if (initialData) {
                D3D12_SUBRESOURCE_DATA srd;
                srd.pData = initialData;
                srd.RowPitch = bufferSize;
                srd.SlicePitch = 0;// srd.RowPitch * height;

                // todo: is a lock needed for commandlist/fences maybe??
                // todo: switch uploading to a dynamic ringbuffer system
                DX12_CHECK_RET0(m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                    D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uploadBuffer)));

                std::string uploadBufferDebugName = desc.debugName + "uploadBuffer";
                D3D_SET_OBJECT_NAME_A(uploadBuffer, uploadBufferDebugName.c_str());

                D3DX12Residency::ResidencySet* residencySet = m_residencyManager.CreateResidencySet();
                residencySet->Open();
                UpdateSubresources<1>(m_copyCommandList.Get(), resource.Get(), uploadBuffer.Get(), 0, 0, 1, &srd);

                residencySet->Insert(&managedObject);

                DX12_CHECK_RET0(m_copyCommandList->Close());
                DX12_CHECK_RET0(residencySet->Close());

                ID3D12CommandList* ppCommandLists[] = { m_copyCommandList.Get() };
                D3DX12Residency::ResidencySet* ppResidencySets[] = { residencySet };
                m_copyQueue->Wait(m_directQueueFence.Get(), m_directQueueFenceValue);
                DX12_CHECK(m_residencyManager.ExecuteCommandLists(m_copyQueue.Get(), ppCommandLists, ppResidencySets, _countof(ppCommandLists)));
                m_copyQueue->Signal(m_copyQueueFence.Get(), ++m_copyQueueFenceValue);

                // todo: clear these outsomewhere, probly on execute/frameclear
                m_managedUploads.emplace_back(uploadBuffer, residencySet, m_copyQueueFenceValue);
                m_copyCommandList->Reset(m_copyCommandAllocator.Get(), nullptr);
            }

            // todo: this is a bit nasty but it works for now
            if ((desc.accessFlags & BufferAccessFlags::CpuWriteBit) || (desc.accessFlags & BufferAccessFlags::CpuReadBit)) {
                DX12_CHECK_RET0(m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
                    D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&cpubuffer)));

                std::string uploadBufferDebugName = desc.debugName + "cpuBuffer";
                D3D_SET_OBJECT_NAME_A(cpubuffer, uploadBufferDebugName.c_str());

                if (initialData) {
                    uint8_t* pData;
                    CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
                    DX12_CHECK_RET0(cpubuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
                    memcpy(pData, initialData, desc.size);
                    cpubuffer->Unmap(0, nullptr);
                }
            }
        }

        D3D_SET_OBJECT_NAME_A(resource, desc.debugName.c_str());

        BufferDX12* bufDx12 = new BufferDX12();

        bufDx12->accessFlags = desc.accessFlags;
        bufDx12->usageFlags = desc.usageFlags;
        bufDx12->size = desc.size;
        bufDx12->currentState = D3D12_RESOURCE_STATE_COMMON;
        bufDx12->copyFenceValue = m_copyQueueFenceValue;
        bufDx12->trackingHandle = managedObject;
        bufDx12->cpubuffer = cpubuffer;
        bufDx12->buffer.Swap(resource);

        return m_resourceManager->AddResource(bufDx12);
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
        return m_resourceManager->AddResource(shaderDX12);
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
        if (kDebugDx12)
            flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_AVOID_FLOW_CONTROL;

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
        size_t hash = std::hash<VertexLayoutDesc>()(layoutDesc);
        auto layoutCheck = m_inputLayouts.find(hash);
        if (layoutCheck != m_inputLayouts.end())
            return layoutCheck->second;

        bool first = true;
        uint32_t stride = 0;
        InputLayoutDX12* il = new InputLayoutDX12();
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
            il->elements.emplace_back(ied);
        }
        il->stride = stride;
        auto id = m_resourceManager->AddResource(il);
        m_inputLayouts.emplace(hash, id);
        return id;
    }

    TextureId DX12Device::CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName) {

        auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(SafeGet(PixelFormatDX12, format), width, height);

        ComPtr<ID3D12Resource> resource;

        DX12_CHECK_RET0(m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &texDesc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource)));
        D3D_SET_OBJECT_NAME_A(resource, debugName.c_str());

        auto info = m_dev->GetResourceAllocationInfo(0, 1, &texDesc);

        D3DX12Residency::ManagedObject managedObject;
        managedObject.Initialize(resource.Get(), info.SizeInBytes);
        m_residencyManager.BeginTrackingObject(&managedObject);

        if (data) {
            std::unique_ptr<byte> dataByteRef;
            D3D12_SUBRESOURCE_DATA srd;
            srd.pData = TextureDataConverter(texDesc, format, data, dataByteRef);
            srd.RowPitch = GetFormatByteSize(texDesc.Format) * texDesc.Width;
            srd.SlicePitch = 0;// srd.RowPitch * height;

            // todo: is a lock needed for commandlist/fences maybe??
            // todo: switch uploading to a dynamic ringbuffer system
            ComPtr<ID3D12Resource> uploadBuffer;
            D3DX12Residency::ResidencySet* residencySet = m_residencyManager.CreateResidencySet();
            // gpu upload buffer
            DX12_CHECK_RET0(m_dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(info.SizeInBytes), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

            std::string uploadBufferDebugName = debugName + "uploadBuffer";
            D3D_SET_OBJECT_NAME_A(uploadBuffer, uploadBufferDebugName.c_str());

            residencySet->Open();
            UpdateSubresources<1>(m_copyCommandList.Get(), resource.Get(), uploadBuffer.Get(), 0, 0, 1, &srd);

            residencySet->Insert(&managedObject);

            DX12_CHECK_RET0(m_copyCommandList->Close());
            DX12_CHECK_RET0(residencySet->Close());

            ID3D12CommandList* ppCommandLists[] = { m_copyCommandList.Get() };
            D3DX12Residency::ResidencySet* ppResidencySets[] = { residencySet };
            m_copyQueue->Wait(m_directQueueFence.Get(), m_directQueueFenceValue);
            DX12_CHECK(m_residencyManager.ExecuteCommandLists(m_copyQueue.Get(), ppCommandLists, ppResidencySets, _countof(ppCommandLists)));
            m_copyQueue->Signal(m_copyQueueFence.Get(), ++m_copyQueueFenceValue);

            // todo: clear these outsomewhere, probly on execute/frameclear
            m_managedUploads.emplace_back(uploadBuffer, residencySet, m_copyQueueFenceValue);
            m_copyCommandList->Reset(m_copyCommandAllocator.Get(), nullptr);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE srvDescCpuHandle{};
        D3D12_CPU_DESCRIPTOR_HANDLE uavDescCpuHandle{};
        D3D12_CPU_DESCRIPTOR_HANDLE dsvDescCpuHandle{};
        D3D12_CPU_DESCRIPTOR_HANDLE rtvDescCpuHandle{};

        if (usage & TextureUsageFlags::ShaderRead) {
            srvDescCpuHandle = m_cpuSrvHeap->AllocateDescriptor();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            m_dev->CreateShaderResourceView(resource.Get(), &srvDesc, srvDescCpuHandle);
        }

        if (usage & TextureUsageFlags::ShaderWrite) {
            dg_assert_fail_nm();
            //uavDescCpuHandle = m_cpuSrvHeap->AllocateDescriptor();
            //D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
            //uavDesc.Format = 
        }

        if (usage & TextureUsageFlags::RenderTarget) {
            if (IsDepthFormat(format)) {
                dsvDescCpuHandle = m_dsvHeap->AllocateDescriptor();

                D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
                dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Format = texDesc.Format;
                dsvDesc.Texture2D.MipSlice = 0;

                m_dev->CreateDepthStencilView(resource.Get(), &dsvDesc, dsvDescCpuHandle);
            }
            else {
                rtvDescCpuHandle = m_rtvHeap->AllocateDescriptor();
                m_dev->CreateRenderTargetView(resource.Get(), nullptr, rtvDescCpuHandle);
            }
        }

        TextureDX12* texDx12 = new TextureDX12();
        texDx12->trackingHandle = managedObject;
        texDx12->resource.Swap(resource);
        texDx12->srv = srvDescCpuHandle;
        texDx12->uav = uavDescCpuHandle;
        texDx12->dsv = dsvDescCpuHandle;
        texDx12->rtv = rtvDescCpuHandle;
        texDx12->size = info.SizeInBytes;
        texDx12->requestedFormat = format;
        texDx12->height = height;
        texDx12->width = width;
        texDx12->usage = usage;
        texDx12->format = texDesc.Format;
        texDx12->currentState = D3D12_RESOURCE_STATE_COMMON;
        texDx12->copyFenceValue = m_copyQueueFenceValue;
        return m_resourceManager->AddResource(texDx12);
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
        size_t descHash = std::hash<PipelineStateDesc>()(desc);
        auto pc = m_pipelinestates.find(descHash);
        if (pc != m_pipelinestates.end()) {
            return pc->second;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

        InputLayoutDX12* layout = m_resourceManager->GetResource<InputLayoutDX12>(desc.vertexLayout);
        assert(layout);
        psoDesc.InputLayout = { layout->elements.data(), static_cast<UINT>(layout->elements.size())  };

        ShaderDX12* ps = m_resourceManager->GetResource<ShaderDX12>(desc.pixelShader);
        assert(ps);
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps->blob.Get());

        ShaderDX12* vs = m_resourceManager->GetResource<ShaderDX12>(desc.vertexShader);
        assert(vs);
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vs->blob.Get());

        ComPtr<ID3D12RootSignature> rootSig;
        DX12_CHECK_RET0(m_dev->CreateRootSignature(0, vs->blob->GetBufferPointer(), vs->blob->GetBufferSize(), IID_PPV_ARGS(&rootSig)));
        psoDesc.pRootSignature = rootSig.Get();
        m_rootSigs.emplace_back(rootSig);

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
        pso->vertexLayoutId = desc.vertexLayout;
        pso->pipelineState.Swap(pipelineState);

        auto id = m_resourceManager->AddResource(pso);
        m_pipelinestates.emplace(id, descHash);
        return id;
    }

    RenderPassId DX12Device::CreateRenderPass(const RenderPassInfo& renderPassInfo) {
        RenderPassDX12* renderPass = new RenderPassDX12();
        renderPass->info = renderPassInfo;
        return m_resourceManager->AddResource(renderPass);
    }

    CommandBuffer* DX12Device::CreateCommandBuffer() {
        auto rtn = dynamic_cast<CommandBuffer*>(new DX12CommandBuffer(m_dev.Get(), _heapInfo, m_resourceManager));
        _heapInfo.offset = m_gpuSrvHeap->GetNextFrameOffset();
        m_gpuSamplerHeap->GetNextFrameOffset();
        return rtn;
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
        if (access != BufferAccess::Write && access != BufferAccess::WriteNoOverwrite)
            dg_assert_fail_nm(false);

        BufferDX12* bufferDX12 = m_resourceManager->GetResource<BufferDX12>(buffer);
        if (bufferDX12->usageFlags == BufferUsageFlags::ConstantBufferBit) {
            uint8_t* pData;
            CD3DX12_RANGE readRange(0, 0);
            DX12_CHECK_RET0(bufferDX12->buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
            return static_cast<uint8_t*>(pData);
        }
        else {
            dg_assert_nm(bufferDX12->cpubuffer != nullptr);
            uint8_t* pData;
            CD3DX12_RANGE readRange(0, 0);
            DX12_CHECK_RET0(bufferDX12->cpubuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
            return static_cast<uint8_t*>(pData);
        }
    }

    void DX12Device::UnmapMemory(BufferId buffer) {
        BufferDX12* bufferDX12 = m_resourceManager->GetResource<BufferDX12>(buffer);
        assert(bufferDX12);
        if (bufferDX12->cpubuffer != nullptr) {
            // todo: is a lock needed for commandlist/fences maybe??
            // todo: switch uploading to a dynamic ringbuffer system??
            D3DX12Residency::ResidencySet* residencySet = m_residencyManager.CreateResidencySet();
            residencySet->Open();

            m_copyCommandList->CopyBufferRegion(bufferDX12->buffer.Get(), 0, bufferDX12->cpubuffer.Get(), 0, bufferDX12->size);

            residencySet->Insert(&bufferDX12->trackingHandle);

            DX12_CHECK(m_copyCommandList->Close());
            DX12_CHECK(residencySet->Close());

            ID3D12CommandList* ppCommandLists[] = { m_copyCommandList.Get() };
            D3DX12Residency::ResidencySet* ppResidencySets[] = { residencySet };
            if (m_directCommandQueue != 0)
                m_copyQueue->Wait(m_directQueueFence.Get(), m_directQueueFenceValue);
            DX12_CHECK(m_residencyManager.ExecuteCommandLists(m_copyQueue.Get(), ppCommandLists, ppResidencySets, _countof(ppCommandLists)));
            m_copyQueue->Signal(m_copyQueueFence.Get(), ++m_copyQueueFenceValue);

            m_copyCommandList->Reset(m_copyCommandAllocator.Get(), nullptr);
            m_residencyManager.DestroyResidencySet(residencySet);
        }
        else {
            bufferDX12->buffer->Unmap(0, nullptr);
        }
    }

    void DX12Device::Submit(const std::vector<CommandBuffer*>& cmdBuffers) {
        uint64_t maxCopyFenceValue = 0;
        std::vector<D3DX12Residency::ResidencySet*> residencySets;
        std::vector<ID3D12CommandList*> cmdLists;
        for (CommandBuffer* cmdBuffer : cmdBuffers) {
            auto dx12cmdbuf = dynamic_cast<DX12CommandBuffer*>(cmdBuffer);
            cmdLists.push_back(dx12cmdbuf->getCmdList());

            D3DX12Residency::ResidencySet* residencySet = m_residencyManager.CreateResidencySet();
            residencySet->Open();
            for (auto handle : dx12cmdbuf->getTrackingHandles()) {
                residencySet->Insert(&handle);
            }
            DX12_CHECK(residencySet->Close());
            residencySets.push_back(residencySet);

            maxCopyFenceValue = std::max(maxCopyFenceValue, dx12cmdbuf->getMaxCopyFenceValue());
        }

        m_directCommandQueue->Wait(m_copyQueueFence.Get(), maxCopyFenceValue);
        m_residencyManager.ExecuteCommandLists(m_directCommandQueue.Get(), cmdLists.data(), residencySets.data(), cmdLists.size());
        m_directCommandQueue->Signal(m_directQueueFence.Get(), ++m_directQueueFenceValue);

        for (CommandBuffer* cmdBuffer : cmdBuffers) {
            delete dynamic_cast<DX12CommandBuffer*>(cmdBuffer);
        }
        for (auto set : residencySets) {
            m_residencyManager.DestroyResidencySet(set);
        }
    }

    DX12Device::DX12Device(IDXGIAdapter3* adapter, ResourceManager* resourceManager, bool usePrebuiltShaders)
        : m_resourceManager(resourceManager) {
        m_usePrebuiltShaders = usePrebuiltShaders;

		DeviceConfig.DeviceAbbreviation = "DX12";
		DeviceConfig.ShaderDir = "DX";

        if (m_usePrebuiltShaders) {
            DeviceConfig.ShaderExtension = ".cso";
        }
        else {
            DeviceConfig.ShaderExtension = ".hlsl";
        }

        // Enable the D3D12 debug layer.
        if (kDebugDx12) {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
            }
        }

        DX12_CHECK_RET(D3D12CreateDevice(
            adapter,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_dev)
        ));

        // Sanity check, just want the highest versions for now, not sure if 1.0 will ignore
        //   but it looks like 1.1 gives us more flags n such for driver optimizations,
        //   also, if i remember, i think the d3dx12 will convert in code definitions for us automatically
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        DX12_CHECK_RET(m_dev->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        DX12_CHECK_RET(m_dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_directCommandQueue)));

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        DX12_CHECK_RET(m_dev->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_copyQueue)));

        m_cpuSrvHeap = std::make_unique<DX12CpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "cpuSrvHeap");
        m_cpuSamplerHeap = std::make_unique<DX12CpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "cpuSamplerHeap");
        m_rtvHeap = std::make_unique<DX12CpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "devRtvHeap");
        m_dsvHeap = std::make_unique<DX12CpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "devDsvHeap");

        m_gpuSrvHeap = std::make_unique<DX12GpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "gpuSrvHeap");
        m_gpuSamplerHeap = std::make_unique<DX12GpuDescHeap>(m_dev.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, "gpuSamplerHeap");

        _heapInfo.numAllocated = m_gpuSamplerHeap->GetNumDescriptorsPerAllocation();
        _heapInfo.offset = 0;
        _heapInfo.samplerDescSize = m_gpuSamplerHeap->GetDescSize();
        _heapInfo.srvDescSize = m_gpuSrvHeap->GetDescSize();
        _heapInfo.samplerHeap = m_gpuSamplerHeap->GetHeap();
        _heapInfo.srvHeap = m_gpuSrvHeap->GetHeap();

        DX12_CHECK_RET(m_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directCommandAllocator)));
        DX12_CHECK_RET(m_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_directCommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_directCommandList)));
        DX12_CHECK_RET(m_dev->CreateFence(m_directQueueFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_directQueueFence)));

        DX12_CHECK_RET(m_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_copyCommandAllocator)));
        DX12_CHECK_RET(m_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_copyCommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_copyCommandList)));
        DX12_CHECK_RET(m_dev->CreateFence(m_copyQueueFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_copyQueueFence)));

        // todo: pick out correct adapter
        m_residencyManager.Initialize(m_dev.Get(), 0, adapter, 10);
    }

    DX12Device::~DX12Device() {
        m_dev.Reset();
    }
}