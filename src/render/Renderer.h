#pragma once

#include "RenderObj.h"
#include "RenderQueue.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"

class RenderEngine;

class Renderer : public RenderServiceLocator {
private:
    friend RenderEngine;

private:
    RenderServiceLocator* _renderServiceLocator{nullptr};
    bool                  _isActive{true};

public:
    Renderer();
    virtual ~Renderer();

    virtual void SetActive(bool isActive) { _isActive = isActive; }
    virtual bool IsActive() const { return _isActive; }

    // RenderServiceLocation impl
    ShaderCache*           GetShaderCache() override;
    PipelineStateCache*    GetPipelineStateCache() override;
    gfx::RenderDevice*     GetRenderDevice() override;
    VertexLayoutCache*     GetVertexLayoutCache() override;
    MeshCache*             GetMeshCache() override;
    ConstantBufferManager* GetConstantBufferManager() override;
    MaterialCache*         GetMaterialCache() override;

protected:
    virtual void OnInit();
    virtual void OnDestroy();

private:
    void Init(RenderServiceLocator* serviceLocator);
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;
};

template <typename T>
class TypedRenderer : public Renderer {
public:
    TypedRenderer() { static_assert(std::is_base_of<RenderObj, T>::value, "Derived class is not derived from RenderObj"); }

    virtual ~TypedRenderer(){};

    virtual void Register(T* typedRenderObjects)   = 0;
    virtual void Unregister(T* typedRenderObjects) = 0;

private:
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;
};
