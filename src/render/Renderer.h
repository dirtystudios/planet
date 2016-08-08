#pragma once

#include "RenderObj.h"
#include "RenderQueue.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"
#include "SimObj.h"

class Renderer : public RenderServiceLocator {
private:
    RenderServiceLocator* _renderServiceLocator{nullptr};

protected:
    bool _isActive{true};

public:
    Renderer();
    virtual ~Renderer();
    virtual void OnInit();
    virtual void OnDestroy();

    void Init(RenderServiceLocator* serviceLocator);

    void SetActive(bool isActive) { _isActive = isActive; }
    bool                IsActive() const { return _isActive; }

    // RenderServiceLocation impl
    ShaderCache*           GetShaderCache() override;
    PipelineStateCache*    GetPipelineStateCache() override;
    gfx::RenderDevice*     GetRenderDevice() override;
    VertexLayoutCache*     GetVertexLayoutCache() override;
    MeshCache*             GetMeshCache() override;
    ConstantBufferManager* GetConstantBufferManager() override;
    MaterialCache*         GetMaterialCache() override;

    // Required
    virtual RenderObj* Register(SimObj* simObj)   = 0;
    virtual void Unregister(RenderObj* renderObj) = 0;
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;
};
