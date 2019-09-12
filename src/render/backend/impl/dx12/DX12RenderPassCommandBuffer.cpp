#include "DX12RenderPassCommandBuffer.h"
#include "DGAssert.h"
#include "DX12Assert.h"
#include "DX12Resources.h"
#include "ResourceManager.h"

namespace gfx {
    using namespace Microsoft::WRL;

    DX12RenderPassCommandBuffer::DX12RenderPassCommandBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdlist, ResourceManager* rm) {
        dg_assert_nm(rm != nullptr);
        dg_assert_nm(cmdlist != nullptr);
        _rm = rm;
        _cmdlist = cmdlist;
    }

    void DX12RenderPassCommandBuffer::reset(ID3D12CommandAllocator* cmdAlloc) {
        _vbufferId = 0;
        _inputLayoutId = 0;
        DX12_CHECK(_cmdlist->Reset(cmdAlloc, nullptr));
    }

    void DX12RenderPassCommandBuffer::close() {
        DX12_CHECK(_cmdlist->Close());
    }

    void DX12RenderPassCommandBuffer::setPipelineState(PipelineStateId psId) {
        auto pstate = _rm->GetResource<PipelineStateDX12>(psId);
        dg_assert_nm(pstate != nullptr);

        _inputLayoutId = pstate->vertexLayoutId;
        _cmdlist->SetPipelineState(pstate->pipelineState.Get());
        // todo: is topology needed? 
        //_cmdlist->IASetPrimitiveTopology(pstate->primitiveType);
    }

    void DX12RenderPassCommandBuffer::setVertexBuffer(BufferId vertexBuffer) {
        _vbufferId = vertexBuffer;
    }

    void DX12RenderPassCommandBuffer::setShaderBuffer(BufferId buffer, uint8_t index, ShaderStageFlags stages) {

    }

    void DX12RenderPassCommandBuffer::setShaderTexture(TextureId texture, uint8_t index, ShaderStageFlags stages) {

    }

    void DX12RenderPassCommandBuffer::drawCommon() {
        dg_assert_nm(_vbufferId != 0 && _inputLayoutId != 0);

        auto vb = _rm->GetResource<BufferDX12>(_vbufferId);
        auto il = _rm->GetResource<InputLayoutDX12>(_inputLayoutId);
        dg_assert_nm(vb != nullptr && il != nullptr);

        D3D12_VERTEX_BUFFER_VIEW vbview;
        vbview.BufferLocation = vb->buffer->GetGPUVirtualAddress();
        vbview.SizeInBytes = vb->size;
        vbview.StrideInBytes = il->stride;

        _cmdlist->IASetVertexBuffers(0, 1, &vbview);
    }

    void DX12RenderPassCommandBuffer::drawPrimitives(uint32_t startOffset, uint32_t vertexCount) {
        drawCommon();
        _cmdlist->DrawInstanced(vertexCount, 1, startOffset, 0);
    }

    void DX12RenderPassCommandBuffer::drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) {
        drawCommon();
        auto indexBuffer = _rm->GetResource<BufferDX12>(indexBufferId);
        dg_assert_nm(indexBuffer != nullptr);

        D3D12_INDEX_BUFFER_VIEW idxView;
        idxView.BufferLocation = indexBuffer->buffer->GetGPUVirtualAddress();
        idxView.SizeInBytes = indexBuffer->size;
        idxView.Format = DXGI_FORMAT_R32_UINT;

        _cmdlist->IASetIndexBuffer(&idxView);
        _cmdlist->DrawIndexedInstanced(indexCount, 1, indexOffset, baseVertexOffset, 0);
    }
}