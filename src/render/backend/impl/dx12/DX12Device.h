#pragma once
#pragma comment(lib, "d3d12")

#include "RenderDevice.h"
#include "SimpleShaderLibrary.h"
#include "ResourceManager.h"
#include "Pool.h"
#include "DMath.h"
#include "DX12Resources.h"
#include "ResourceManager.h"
#include "DX12CpuDescHeap.h"
#include "DX12GpuDescHeap.h"
#include "DX12GpuHeaps.h"

#include "d3dx12.h"
#include "d3dx12Residency.h"

#include <memory>
#include <cstring>
#include <unordered_map>
#include <map>
#include <array>
#include <vector>

#include <wrl.h>

#include <dxgi1_4.h>
#include <d3d12.h>

namespace gfx {
    using namespace Microsoft::WRL;

    class DX12Device final : public RenderDevice {
    private:

        struct ManagedUpload {
            ComPtr<ID3D12Resource> uploadBuffer;
            D3DX12Residency::ResidencySet* residencySet;
            uint64_t fenceValue;

            ManagedUpload(ComPtr<ID3D12Resource> buf, D3DX12Residency::ResidencySet* set, uint64_t val) {
                uploadBuffer.Swap(buf);
                residencySet = set;
                fenceValue = val;
            }
        };

        std::vector<ManagedUpload> m_managedUploads;

        static const uint32_t FrameCount = 2;
        bool m_usePrebuiltShaders;

        ComPtr<ID3D12Device> m_dev;

        std::unique_ptr<DX12CpuDescHeap> m_rtvHeap{ nullptr };
        std::unique_ptr<DX12CpuDescHeap> m_dsvHeap{ nullptr };
        std::unique_ptr<DX12CpuDescHeap> m_cpuSrvHeap{ nullptr };
        std::unique_ptr<DX12CpuDescHeap> m_cpuSamplerHeap{ nullptr };
        std::unique_ptr<DX12GpuDescHeap> m_gpuSrvHeap{ nullptr };
        std::unique_ptr<DX12GpuDescHeap> m_gpuSamplerHeap{ nullptr };

        ComPtr<ID3D12CommandAllocator> m_copyCommandAllocator;
        ComPtr<ID3D12CommandQueue> m_copyQueue;
        ComPtr<ID3D12GraphicsCommandList> m_copyCommandList;
        ComPtr<ID3D12Fence> m_copyQueueFence;
        uint64_t m_copyQueueFenceValue{ 0 };

        ComPtr<ID3D12CommandQueue> m_directCommandQueue;
        ComPtr<ID3D12GraphicsCommandList> m_directCommandList;
        ComPtr<ID3D12CommandAllocator> m_directCommandAllocator;
        ComPtr<ID3D12Fence> m_directQueueFence;
        uint64_t m_directQueueFenceValue{ 0 };

        std::unordered_map<size_t, PipelineStateId> m_pipelinestates;
        std::unordered_map<size_t, VertexLayoutId> m_inputLayouts;
        std::vector<ComPtr<ID3D12RootSignature>> m_rootSigs;

        std::vector<CommandBuffer*> m_submittedBuffers;

        SimpleShaderLibrary m_shaderLibrary;

        D3DX12Residency::ResidencyManager m_residencyManager;

        ResourceManager* m_resourceManager{ nullptr };
        DX12GpuHeaps _heapInfo;

    public:
        DX12Device() = delete;
        DX12Device(IDXGIAdapter3* adapter, ResourceManager* resourceManager, bool usePrebuiltShaders = false);
        ~DX12Device();

        RenderDeviceApi GetDeviceApi() final { return RenderDeviceApi::D3D12; };

        BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData);

        ShaderId GetShader(ShaderType type, const std::string& functionName);
        void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData);

        PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
        RenderPassId CreateRenderPass(const RenderPassInfo& renderPassInfo) final;

        TextureId CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName = "") final;
        TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName) final { return 0; }
        TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName) final { return 0; }
        VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc) final;

        CommandBuffer* CreateCommandBuffer() final;
        void UpdateTexture(TextureId textureId, uint32_t slice, const void* srcData) final { dg_assert_fail_nm(); };
        void Submit(const std::vector<CommandBuffer*>& cmdBuffers) final;

        uint8_t* MapMemory(BufferId buffer, BufferAccess) final;
        void UnmapMemory(BufferId buffer) final;

        ID3D12Device* GetID3D12Dev() { return m_dev.Get(); }

        //todo:
        void DestroyResource(ResourceId resourceId) final {}

        // dx12 specific helpers
        ID3D12CommandQueue* GetCommandQueue() { return m_directCommandQueue.Get(); }

    private:

        ShaderId CreateShader(ShaderType type, const std::string& source);
        ComPtr<ID3DBlob> CompileShader(ShaderType shaderType, const std::string& source);

        D3D12_RASTERIZER_DESC CreateRasterState(const RasterState& state);
        D3D12_BLEND_DESC CreateBlendState(const BlendState& state);
        D3D12_DEPTH_STENCIL_DESC CreateDepthState(const DepthState& state);

        // todo: make this stuff shared between dx
        void* TextureDataConverter(const D3D12_RESOURCE_DESC& tDesc, PixelFormat reqFormat, void* data, std::unique_ptr<byte>& dataRef);
        int GetFormatByteSize(DXGI_FORMAT dxFormat) {
            switch (dxFormat) {
            case DXGI_FORMAT_R8_UINT: return 1;
            case DXGI_FORMAT_R8_UNORM: return 1;
            case DXGI_FORMAT_R32_FLOAT: return 4;
            case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
            case DXGI_FORMAT_R32G32B32_FLOAT: return 12;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
            default: return 0;
            }
        }
    };
}