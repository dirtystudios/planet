#include "DX11ComputePassCommandBuffer.h"
#include "ResourceManager.h"

#include <wrl.h>

namespace gfx {
    using namespace Microsoft::WRL;

    DX11ComputePassCommandBuffer::DX11ComputePassCommandBuffer(ComPtr<ID3D11Device> dev, ComPtr<ID3D11DeviceContext> ctx, ResourceManager* rm, DX11Cache* cache)
        : _dev(dev), _rm(rm), _cache(cache) {
        _cmdBuf.reset(new DX11Context(ctx));
    }

    void DX11ComputePassCommandBuffer::setPipelineState(PipelineStateId pipelineState) {
        PipelineStateDX11* state = _rm->GetResource<PipelineStateDX11>(pipelineState);
        _cmdBuf->SetComputeShader(state->computeShader);
    }

    void DX11ComputePassCommandBuffer::setBuffer(BufferId buffer, uint8_t index) {
        BufferDX11* cbuffer = _rm->GetResource<BufferDX11>(buffer);
        // todo: hack
        //_cmdBuf->SetComputeShaderUAV(index, cbuffer->uav.Get());
        _cmdBuf->SetComputeShaderTexture(index, cbuffer->srv.Get(), nullptr);
    }

    void DX11ComputePassCommandBuffer::setCBuffer(BufferId buffer, uint8_t index) {
        BufferDX11* cbuffer = _rm->GetResource<BufferDX11>(buffer);
        _cmdBuf->SetComputeCBuffer(index, cbuffer->buffer.Get());
    }

    void DX11ComputePassCommandBuffer::setTexture(TextureId buffer, uint8_t index) {
        auto* tex = _rm->GetResource<TextureDX11>(buffer);
        // todo: hack
        //_cmdBuf->SetComputeShaderTexture(index, tex->srv.Get(), tex->sampler.Get());
        _cmdBuf->SetComputeShaderUAV(index, tex->uav.Get());
    }

    void DX11ComputePassCommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
        _cmdBuf->Dispatch(groupCountX, groupCountY, groupCountZ);
    }
}