#pragma once
#include "RenderPassCommandBuffer.h"
#include "DX11Context.h"
#include "Framebuffer.h"
#include "DX11Resources.h"
#include "DX11Cache.h"

#include <wrl.h>
#include <memory>
#include <d3d11.h>

namespace gfx {
    class ResourceManager;

    class DX11RenderPassCommandBuffer : public RenderPassCommandBuffer {
    private:
        std::unique_ptr<DX11Context> _cmdBuf{ nullptr };
        // need this to create input layout on draw if needed
        // luckily its thread safe, so doing this should be fine
        Microsoft::WRL::ComPtr<ID3D11Device> _dev{ nullptr };

        ResourceManager* _rm{ nullptr };
        DX11Cache* _cache{ nullptr };

    public:
        DX11RenderPassCommandBuffer(Microsoft::WRL::ComPtr<ID3D11Device> dev, Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx, ResourceManager* rm, DX11Cache* cache);

        ID3D11DeviceContext *GetCtx() { return _cmdBuf->GetD3D11Context(); }
        void SetupRenderTargets(const FrameBuffer& frameBuffer, const RenderPassDX11& renderPass);

        void setPipelineState(PipelineStateId psId) final;
        void setVertexBuffer(BufferId vertexBuffer) final;
        void setShaderBuffer(BufferId buffer, uint8_t index, ShaderStageFlags stages) final;
        void setShaderTexture(TextureId texture, uint8_t index, ShaderStageFlags stages) final;
        void drawPrimitives(uint32_t startOffset, uint32_t vertexCount) final;
        void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) final;

    private:
        ID3D11InputLayout* CreateInputLayout(InputLayoutDX11* state, ShaderId shaderId);
        void SetViewPort(uint32_t height, uint32_t width);
    };
}