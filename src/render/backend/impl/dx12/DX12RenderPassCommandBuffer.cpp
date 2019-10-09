#include "DX12RenderPassCommandBuffer.h"
#include "DGAssert.h"
#include "DX12Assert.h"
#include "DX12Resources.h"
#include "ResourceManager.h"

#include <array>
#include <d3dx12.h>

namespace gfx {
    using namespace Microsoft::WRL;

    DX12RenderPassCommandBuffer::DX12RenderPassCommandBuffer(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdlist, const DX12GpuHeaps& heapinfo, ResourceManager* rm) {
        dg_assert_nm(rm != nullptr);
        dg_assert_nm(cmdlist != nullptr);
        _heapInfo = heapinfo;
        _rm = rm;
        _cmdlist = cmdlist;

        ID3D12DescriptorHeap* ppheaps[] = { _heapInfo.srvHeap, _heapInfo.samplerHeap };
        _cmdlist->SetDescriptorHeaps(2, ppheaps);
        _cmdlist->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetGPUDescriptorHandleForHeapStart(), _heapInfo.offset, _heapInfo.srvDescSize));
    }

    void DX12RenderPassCommandBuffer::reset(ID3D12CommandAllocator* cmdAlloc) {
        _vbufferId = 0;
        _inputLayoutId = 0;
        DX12_CHECK(_cmdlist->Reset(cmdAlloc, nullptr));
        _srvDescCopyInfo.clear();
        ID3D12DescriptorHeap* ppheaps[] = { _heapInfo.srvHeap, _heapInfo.samplerHeap };
        _cmdlist->SetDescriptorHeaps(2, ppheaps);
        _cmdlist->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetGPUDescriptorHandleForHeapStart(), _heapInfo.offset, _heapInfo.srvDescSize));
    }

    void DX12RenderPassCommandBuffer::SetViewPort(uint32_t height, uint32_t width) {
        D3D12_VIEWPORT vp;
        vp.Width = (float)width;
        vp.Height = (float)height;
        vp.MinDepth = 0;
        vp.MaxDepth = 1;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        _cmdlist->RSSetViewports(1, &vp);
    }


    void DX12RenderPassCommandBuffer::SetRenderTargets(const FrameBuffer& framebuffer, const RenderPassDX12& rp) {
        if (framebuffer.colorCount > 0) {
            auto buf = _rm->GetResource<TextureDX12>(framebuffer.color[0]);
            D3D12_RESOURCE_DESC desc = buf->resource->GetDesc();
            SetViewPort(desc.Height, desc.Width);
        }
        else if (framebuffer.depth != 0) {
            auto buf = _rm->GetResource<TextureDX12>(framebuffer.depth);
            D3D12_RESOURCE_DESC desc = buf->resource->GetDesc();
            SetViewPort(desc.Height, desc.Width);
        }

        std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, 8> rtvs{};
        for (uint32_t i = 0; i < framebuffer.colorCount; ++i) {
            auto tex = _rm->GetResource<TextureDX12>(framebuffer.color[i]);
            dg_assert_nm(tex != nullptr);
            rtvs[(size_t)i] = tex->rtv;
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv{};
        if (framebuffer.depth != 0) {
            auto depth = _rm->GetResource<TextureDX12>(framebuffer.depth);
            dg_assert_nm(depth != nullptr);
            dsv = depth->dsv;
        }

        _cmdlist->OMSetRenderTargets(framebuffer.colorCount, rtvs.data(), true, rp.info.hasDepth ? &dsv : nullptr);

        for (uint32_t i = 0; i < rp.info.attachmentCount; ++i) {
            const auto& a = rp.info.attachments[i];
            if (a.loadAction == LoadAction::Clear) {
                std::array<FLOAT, 4> color = { 0.f, 0.f, 0.f, 0.f };
                _cmdlist->ClearRenderTargetView(rtvs[i], color.data(), 0, nullptr);
            }
        }
        if (rp.info.hasDepth || rp.info.hasStencil) {
            D3D12_CLEAR_FLAGS clearFlags{};
            bool clearDepth = rp.info.hasDepth && rp.info.depthAttachment.loadAction == LoadAction::Clear;
            bool clearStencil = rp.info.hasStencil && rp.info.stencilAttachment.loadAction == LoadAction::Clear;

            if (clearDepth)
                clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
            if (clearStencil)
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;

            if (clearFlags) {
                auto buf = _rm->GetResource<TextureDX12>(framebuffer.depth);
                dg_assert_nm(buf != nullptr);
                _cmdlist->ClearDepthStencilView(buf->dsv, clearFlags, 1.f, 0, 0, nullptr);
            }
        }
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

    void DX12RenderPassCommandBuffer::setShaderBuffer(BufferId bufferId, uint8_t index, ShaderStageFlags stages) {
        auto buf = _rm->GetResource<BufferDX12>(bufferId);
        dg_assert_nm(buf != nullptr);
        dg_assert(index < _heapInfo.numAllocated, "dx12: index exceeds desc buffer range");

        auto dest = CD3DX12_CPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetCPUDescriptorHandleForHeapStart(), _heapInfo.offset + index, _heapInfo.srvDescSize);
        DescInfo tmp;
        tmp.src = buf->cbv;
        tmp.dest = dest;
        _srvDescCopyInfo[index] = tmp;
    }

    void DX12RenderPassCommandBuffer::setShaderTexture(TextureId textureId, uint8_t index, ShaderStageFlags stages) {
        auto tex = _rm->GetResource<TextureDX12>(textureId);
        dg_assert_nm(tex != nullptr);
        dg_assert(index < _heapInfo.numAllocated, "dx12: index exceeds desc tex range");
        dg_assert_nm(tex->srv.ptr);

        auto dest = CD3DX12_CPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetCPUDescriptorHandleForHeapStart(), _heapInfo.offset + index, _heapInfo.srvDescSize);
        DescInfo tmp;
        tmp.src = tex->srv;
        tmp.dest = dest;
        _srvDescCopyInfo[index] = tmp;
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

        // todo: copy descriptors to gpu heap
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