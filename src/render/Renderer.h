#pragma once

#include "RenderObj.h"
#include "RenderQueue.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"

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

    // Populate Renderer related data
    virtual void Register(RenderObj* renderObj) = 0;

    virtual void Unregister(RenderObj* renderObj) = 0;
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;
};

template <typename T>
class TypedRenderer : public Renderer {
public:
    TypedRenderer() { static_assert(std::is_base_of<RenderObj, T>::value, "Derived class is not derived from RenderObj"); }

    virtual ~TypedRenderer(){};

    virtual void Register(T* typedRenderObjects)   = 0;
    virtual void Unregister(T* typedRenderObjects) = 0;
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView) = 0;

    void Register(RenderObj* renderObj) final { Register(reinterpret_cast<T*>(renderObj)); }
    void Unregister(RenderObj* renderObj) final { Unregister(reinterpret_cast<T*>(renderObj)); }
};
