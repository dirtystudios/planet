#pragma once
#include "RenderPassCommandBuffer.h"
#include "Framebuffer.h"
#include "DX12Resources.h"

#include <wrl.h>
#include <memory>
#include <d3d12.h>

namespace gfx {
    class ResourceManager;

    class DX12RenderPassCommandBuffer final : public RenderPassCommandBuffer {
    private:
        ResourceManager* _rm{ nullptr };

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdlist;

        BufferId _vbufferId{ 0 };
        VertexLayoutId _inputLayoutId{ 0 };
    public:
        DX12RenderPassCommandBuffer() = delete;
        DX12RenderPassCommandBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdlist, ResourceManager* rm);

        void reset(ID3D12CommandAllocator* cmdAlloc);
        void close();

        void setPipelineState(PipelineStateId psId) final;
        void setVertexBuffer(BufferId vertexBuffer) final;
        void setShaderBuffer(BufferId buffer, uint8_t index, ShaderStageFlags stages) final;
        void setShaderTexture(TextureId texture, uint8_t index, ShaderStageFlags stages) final;
        void drawPrimitives(uint32_t startOffset, uint32_t vertexCount) final;
        void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) final;

    private:
        void drawCommon();
    };
}