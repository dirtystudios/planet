#pragma once

#include "SimObj.h"
#include "RenderObj.h"
#include "RenderView.h"
#include "RenderQueue.h"
#include "RenderServiceLocator.h"

class Renderer : public RenderServiceLocator {
private:
    RenderServiceLocator* _renderServiceLocator{nullptr};

public:
    Renderer();
    virtual ~Renderer();
    virtual void OnInit();
    virtual void OnDestroy();

    void Init(RenderServiceLocator* serviceLocator);

    // RenderServiceLocation impl
    ShaderCache* GetShaderCache() override;
    PipelineStateCache* GetPipelineStateCache() override;
    graphics::RenderDevice* GetRenderDevice() override;
    VertexLayoutCache* GetVertexLayoutCache() override;
    MeshCache* GetMeshCache() override;
    ConstantBufferManager* GetConstantBufferManager() override;
    MaterialCache* GetMaterialCache() override;

    // Required
    virtual RenderObj* Register(SimObj* simObj) = 0;
    virtual void Unregister(RenderObj* renderObj) = 0;
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;
};