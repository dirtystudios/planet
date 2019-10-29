#define NOMINMAX
#include "DX12RenderPassCommandBuffer.h"
#include "DGAssert.h"
#include "DX12Assert.h"
#include "DX12Resources.h"
#include "ResourceManager.h"

#include <array>
#include <vector>
#include <d3dx12.h>

namespace gfx {
    using namespace Microsoft::WRL;

    DX12RenderPassCommandBuffer::DX12RenderPassCommandBuffer(ID3D12Device* dev, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdlist, const DX12GpuHeaps& heapinfo, ResourceManager* rm) {
        dg_assert_nm(rm != nullptr);
        dg_assert_nm(cmdlist != nullptr);
        _heapInfo = heapinfo;
        _rm = rm;
        _cmdlist = cmdlist;
        _dev = dev;

        ID3D12DescriptorHeap* ppheaps[] = { _heapInfo.srvHeap, _heapInfo.samplerHeap };
        _cmdlist->SetDescriptorHeaps(2, ppheaps);
        _cmdlist->SetGraphicsRootDescriptorTable(0, CD3DX12_GPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetGPUDescriptorHandleForHeapStart(), _heapInfo.offset, _heapInfo.srvDescSize));
    }

    void DX12RenderPassCommandBuffer::reset(ID3D12CommandAllocator* cmdAlloc) {
        _vbufferId = 0;
        _inputLayoutId = 0;
        DX12_CHECK(_cmdlist->Reset(cmdAlloc, nullptr));
        _srvDescCopyInfo.clear();
        _maxCopyFenceValue = 0;
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

        std::vector<CD3DX12_RESOURCE_BARRIER> barriers(8);
        std::array<CD3DX12_CPU_DESCRIPTOR_HANDLE, 8> rtvHandles{};
        for (uint32_t i = 0; i < framebuffer.colorCount; ++i) {
            auto tex = _rm->GetResource<TextureDX12>(framebuffer.color[i]);
            dg_assert_nm(tex != nullptr);
            rtvHandles[(size_t)i] = tex->rtv;
            if (tex->currentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
                barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(tex->resource.Get(), tex->currentState, D3D12_RESOURCE_STATE_RENDER_TARGET));
                tex->currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
            }
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv{};
        if (framebuffer.depth != 0) {
            auto depth = _rm->GetResource<TextureDX12>(framebuffer.depth);
            dg_assert_nm(depth != nullptr);
            dsv = depth->dsv;
            if (depth->currentState != D3D12_RESOURCE_STATE_DEPTH_WRITE) {
                barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(depth->resource.Get(), depth->currentState, D3D12_RESOURCE_STATE_DEPTH_WRITE));
                depth->currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
            }
        }
        _cmdlist->OMSetRenderTargets(framebuffer.colorCount, rtvHandles.data(), true, rp.info.hasDepth ? &dsv : nullptr);
        _cmdlist->ResourceBarrier(barriers.size(), barriers.data());

        for (uint32_t i = 0; i < rp.info.attachmentCount; ++i) {
            const auto& a = rp.info.attachments[i];
            if (a.loadAction == LoadAction::Clear) {
                std::array<FLOAT, 4> color = { 0.f, 0.f, 0.f, 0.f };
                _cmdlist->ClearRenderTargetView(rtvHandles[i], color.data(), 0, nullptr);
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

        // todo: batch these?
        D3D12_RESOURCE_STATES destState;
        if (stages & ShaderStageFlags::PixelBit)
            destState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (stages & ShaderStageFlags::VertexBit)
            destState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        // todo: other stages and double check read/write
        destState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

        if ((destState & buf->currentState) != destState)
            _cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buf->buffer.Get(), buf->currentState, destState));

        auto dest = CD3DX12_CPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetCPUDescriptorHandleForHeapStart(), _heapInfo.offset + index, _heapInfo.srvDescSize);
        DescInfo tmp;
        tmp.src = buf->cbv;
        tmp.dest = dest;
        _srvDescCopyInfo[index] = tmp;
        _maxCopyFenceValue = std::max(_maxCopyFenceValue, buf->copyFenceValue);
    }

    void DX12RenderPassCommandBuffer::setShaderTexture(TextureId textureId, uint8_t index, ShaderStageFlags stages) {
        auto tex = _rm->GetResource<TextureDX12>(textureId);
        dg_assert_nm(tex != nullptr);
        dg_assert(index < _heapInfo.numAllocated, "dx12: index exceeds desc tex range");
        dg_assert_nm(tex->srv.ptr);

        // todo: batch these?
        D3D12_RESOURCE_STATES destState;
        if (stages & ShaderStageFlags::PixelBit)
            destState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        if (stages & ShaderStageFlags::VertexBit)
            destState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        // todo: other stages

        if ((destState & tex->currentState) != destState)
            _cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->resource.Get(), tex->currentState, destState));

        auto dest = CD3DX12_CPU_DESCRIPTOR_HANDLE(_heapInfo.srvHeap->GetCPUDescriptorHandleForHeapStart(), _heapInfo.offset + index, _heapInfo.srvDescSize);
        DescInfo tmp;
        tmp.src = tex->srv;
        tmp.dest = dest;
        _srvDescCopyInfo[index] = tmp;
        _maxCopyFenceValue = std::max(_maxCopyFenceValue, tex->copyFenceValue);
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

        if ((D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER & vb->currentState) != D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            _cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vb->buffer.Get(), vb->currentState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

        _cmdlist->IASetVertexBuffers(0, 1, &vbview);

        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> dests;
        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> srcs;
        std::vector<UINT> copySizes;
        int check = 0;
        for (const auto& info : _srvDescCopyInfo) {
            if (check != (info.first + 1))
                dg_assert_fail_nm();
            srcs.emplace_back(info.second.src);
            dests.emplace_back(info.second.dest);
            copySizes.emplace_back(1);
            check++;
        }

        _dev->CopyDescriptors(dests.size(), dests.data(), copySizes.data(), srcs.size(), srcs.data(), copySizes.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        _maxCopyFenceValue = std::max(_maxCopyFenceValue, vb->copyFenceValue);
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

        if (indexBuffer->currentState != D3D12_RESOURCE_STATE_INDEX_BUFFER)
            _cmdlist->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer->buffer.Get(), indexBuffer->currentState, D3D12_RESOURCE_STATE_INDEX_BUFFER));

        _cmdlist->DrawIndexedInstanced(indexCount, 1, indexOffset, baseVertexOffset, 0);
        _maxCopyFenceValue = std::max(_maxCopyFenceValue, indexBuffer->copyFenceValue);
    }
}