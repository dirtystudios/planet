#include "DX12CommandBuffer.h"
#include "DX12Resources.h"
#include "DX12RenderPassCommandBuffer.h"
#include "DX12Assert.h"
#include "ResourceManager.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>

namespace gfx {
    using namespace Microsoft::WRL;

    DX12CommandBuffer::DX12CommandBuffer(ID3D12Device* dev, ResourceManager* resourceManager)
        : _resourceManager(resourceManager) {

        dg_assert_nm(resourceManager != nullptr);
        dg_assert_nm(dev != nullptr);

        DX12_CHECK(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_directAllocator)));

        ComPtr<ID3D12GraphicsCommandList> cmdList;
        DX12_CHECK(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _directAllocator.Get(), nullptr, IID_PPV_ARGS(&cmdList)));
        cmdList->Close();

        _rpPass.reset(new DX12RenderPassCommandBuffer(cmdList, resourceManager));
    }

    ID3D12GraphicsCommandList* DX12CommandBuffer::GetCmdList() {
        auto cmdlist = _rpPass.getCmdList();
        dg_assert_nm(cmdlist != nullptr);
        D3D_SET_OBJECT_NAME_A(cmdList, _passName.c_str());

        return cmdlist;
    }

    RenderPassCommandBuffer* DX12CommandBuffer::beginRenderPass(RenderPassId passId, const FrameBuffer& framebuffer, const std::string& name) {
        dg_assert_nm(_inPass == false);

        _inPass = true;

        _rpPass->reset(_directAllocator.Get());

        return _rpPass.get();
    }
}
