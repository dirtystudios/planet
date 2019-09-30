#pragma once
#include "RenderPassCommandBuffer.h"
#include "Framebuffer.h"
#include "DX12Resources.h"
#include "Framebuffer.h"

#include <wrl.h>
#include <memory>
#include <map>
#include <d3d12.h>

namespace gfx {
    class ResourceManager;

    struct DX12GpuHeaps {
        ID3D12DescriptorHeap* srvHeap;
        UINT srvSize;
        ID3D12DescriptorHeap* samplerHeap;
        UINT samplerSize;

        UINT offset;
        UINT maxSize;
    };

    class DX12RenderPassCommandBuffer final : public RenderPassCommandBuffer {
    private:
        struct DescInfo {
            D3D12_CPU_DESCRIPTOR_HANDLE src;
            D3D12_CPU_DESCRIPTOR_HANDLE dest;

            void operator=(const DescInfo& d) {
                src = d.src;
                dest = d.dest;
            }
        };

        ResourceManager* _rm{ nullptr };
        DX12GpuHeaps _heapInfo;

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdlist;


        std::map<int, DescInfo> _srvDescCopyInfo;
        BufferId _vbufferId{ 0 };
        VertexLayoutId _inputLayoutId{ 0 };
    public:
        DX12RenderPassCommandBuffer() = delete;
        DX12RenderPassCommandBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdlist, const DX12GpuHeaps& heapinfo, ResourceManager* rm);

        void SetRenderTargets(const FrameBuffer& framebuffer, const RenderPassDX12& rp);

        void reset(ID3D12CommandAllocator* cmdAlloc);
        void close();

        void setPipelineState(PipelineStateId psId) final;
        void setVertexBuffer(BufferId vertexBuffer) final;
        void setShaderBuffer(BufferId bufferId, uint8_t index, ShaderStageFlags stages) final;
        void setShaderTexture(TextureId textureId, uint8_t index, ShaderStageFlags stages) final;
        void drawPrimitives(uint32_t startOffset, uint32_t vertexCount) final;
        void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) final;

    private:
        void drawCommon();
    };
}