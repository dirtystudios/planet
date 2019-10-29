#pragma once
#include "CommandBuffer.h"
#include "DX12RenderPassCommandBuffer.h"
#include "DX12Resources.h"
#include "DX12GpuHeaps.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <string>

namespace gfx {
    class ResourceManager;

    class DX12CommandBuffer final : public CommandBuffer {
    private:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _directAllocator{ nullptr };
        std::unique_ptr<DX12RenderPassCommandBuffer> _rpPass{ nullptr };

        ResourceManager* _resourceManager{ nullptr };

        std::string _passName = "";
        bool _inPass{ false };
    public:
        DX12CommandBuffer() = delete;
        DX12CommandBuffer(ID3D12Device* dev, const DX12GpuHeaps& heapInfo, ResourceManager* resourceManager);

        uint64_t GetMaxCopyFenceValue();
        ID3D12GraphicsCommandList* CloseAndGetCmdList();

        RenderPassCommandBuffer* beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name = "") final;
        void endRenderPass(RenderPassCommandBuffer* commandBuffer) final;
    };
}