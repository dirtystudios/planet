#pragma once
#include "DX11Context.h"
#include "DX11Debug.h"
#include "DX11Resources.h"
#include "DX11Cache.h"
#include "DX11CommandBuffer.h"
#include "DX11RenderPassCommandBuffer.h"
#include "ResourceManager.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

namespace gfx {
    using namespace Microsoft::WRL;

    DX11CommandBuffer::DX11CommandBuffer(ComPtr<ID3D11Device> dev, ResourceManager* resourceManager, DX11Cache* cache)
        : _dev(dev), _resourceManager(resourceManager), _cache(cache) {
        dg_assert_nm(resourceManager != nullptr);
        dg_assert_nm(cache != nullptr);

        ComPtr<ID3D11DeviceContext> defCtx;
        DX11_CHECK_RET(_dev->CreateDeferredContext(0, &defCtx));
        _cmdBuf.reset(new DX11RenderPassCommandBuffer(_dev, defCtx, resourceManager, cache));
        _ccmdBuf.reset(new DX11ComputePassCommandBuffer(_dev, defCtx, resourceManager, cache));
    }

    ComPtr<ID3D11CommandList> DX11CommandBuffer::GetCmdList() {
        ComPtr<ID3D11CommandList> rtnlist;
        rtnlist.Swap(_cmdList);
        return rtnlist;
    }

    RenderPassCommandBuffer *DX11CommandBuffer::beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name) {
        dg_assert_nm(_cmdBuf != nullptr);
        dg_assert_nm(_cmdList == nullptr); // not supporting multiple passes with same command buf for now
        dg_assert_nm(_inPass == false);

        RenderPassDX11* rp = _resourceManager->GetResource<RenderPassDX11>(passId);
        dg_assert_nm(rp != nullptr);

        _passName = name;
        _inPass = true;
        _cmdBuf->SetupRenderTargets(frameBuffer, *rp);

        return _cmdBuf.get();
    }

    void DX11CommandBuffer::endRenderPass(RenderPassCommandBuffer* commandBuffer) {
        dg_assert_nm(_cmdBuf != nullptr);
        dg_assert_nm(_inPass);

        DX11_CHECK_RET(_cmdBuf->GetCtx()->FinishCommandList(FALSE, &_cmdList));

        D3D_SET_OBJECT_NAME_A(_cmdList, _passName.c_str());

        _inPass = false;
    }

    ComputePassCommandBuffer* DX11CommandBuffer::beginComputePass(const std::string& name) {
        dg_assert_nm(_ccmdBuf != nullptr);
        dg_assert_nm(_cmdList == nullptr); // not supporting multiple passes with same command buf for now
        dg_assert_nm(_inPass == false);

        _passName = name;
        _inPass = true;

        return _ccmdBuf.get();
    }

    void DX11CommandBuffer::endComputePass(ComputePassCommandBuffer* commandBuffer) {
        dg_assert_nm(_ccmdBuf != nullptr);
        dg_assert_nm(_inPass);

        DX11_CHECK_RET(_ccmdBuf->GetCtx()->FinishCommandList(FALSE, &_cmdList));

        D3D_SET_OBJECT_NAME_A(_cmdList, _passName.c_str());

        _inPass = false;
    }
}