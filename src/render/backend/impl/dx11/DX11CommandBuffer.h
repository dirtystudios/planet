#pragma once
#include "CommandBuffer.h"
#include "DX11RenderPassCommandBuffer.h"
#include "DX11Context.h"
#include "DX11Resources.h"
#include "DX11Cache.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

namespace gfx {
    class ResourceManager;

    class DX11CommandBuffer final : public CommandBuffer {
    private:
        Microsoft::WRL::ComPtr<ID3D11Device> _dev{ nullptr };
        std::unique_ptr<DX11RenderPassCommandBuffer> _cmdBuf{ nullptr };
        Microsoft::WRL::ComPtr<ID3D11CommandList> _cmdList{ nullptr };

        ResourceManager* _resourceManager{ nullptr };
        DX11Cache* _cache{ nullptr };

        std::string _passName = "";
        bool _inPass{ false };
    public:
        DX11CommandBuffer() = delete;
        DX11CommandBuffer(Microsoft::WRL::ComPtr<ID3D11Device> dev, ResourceManager* resourceManager, DX11Cache* cache);

        Microsoft::WRL::ComPtr<ID3D11CommandList> GetCmdList();

        RenderPassCommandBuffer *beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name = "") final;
        void endRenderPass(RenderPassCommandBuffer* commandBuffer) final;
    };
}