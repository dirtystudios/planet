#pragma once
#include "ComputePassCommandBuffer.h"
#include "DX11Context.h"
#include "DX11Resources.h"
#include "DX11Cache.h"

#include <wrl.h>
#include <memory>
#include <d3d11.h>

namespace gfx {
    class ResourceManager;

    class DX11ComputePassCommandBuffer final : public ComputePassCommandBuffer {
    private:
        std::unique_ptr<DX11Context> _cmdBuf{ nullptr };
        // need this to create input layout on draw if needed
        // luckily its thread safe, so doing this should be fine
        Microsoft::WRL::ComPtr<ID3D11Device> _dev{ nullptr };

        ResourceManager* _rm{ nullptr };
        DX11Cache* _cache{ nullptr };

    public:
        DX11ComputePassCommandBuffer(Microsoft::WRL::ComPtr<ID3D11Device> dev, Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx, ResourceManager* rm, DX11Cache* cache);

        ID3D11DeviceContext *GetCtx() { return _cmdBuf->GetD3D11Context(); }

        void setPipelineState(PipelineStateId pipelineState) final;
        void setCBuffer(BufferId buffer, uint8_t index) final;
        void setBuffer(BufferId buffer, uint8_t index, ShaderBindingFlags bindingflags) final;
        void setTexture(TextureId texture, uint8_t index, ShaderBindingFlags bindingflags) final;
        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) final;
    };
}