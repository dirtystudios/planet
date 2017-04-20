#pragma once

#include "RenderObj.h"
#include "RenderQueue.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"

class RenderEngine;

class Renderer {
private:
    friend RenderEngine;

private:
    gfx::RenderDevice*    _device{};
    RenderServiceLocator* _renderServiceLocator{nullptr};
    bool                  _isActive{true};

protected:
public:
    Renderer();
    virtual ~Renderer();

    virtual void SetActive(bool isActive) { _isActive = isActive; }
    virtual bool                IsActive() const { return _isActive; }

    gfx::RenderDevice*    device() { return _device; }
    RenderServiceLocator* services() { return _renderServiceLocator; };

protected:
    virtual void OnInit();
    virtual void OnDestroy();

private:
    void Init(gfx::RenderDevice* device, RenderServiceLocator* serviceLocator);
    virtual void Submit(RenderQueue* renderQueue, const FrameView* view) = 0;
};

template <typename T>
class TypedRenderer : public Renderer {
public:
    TypedRenderer() { static_assert(std::is_base_of<RenderObj, T>::value, "Derived class is not derived from RenderObj"); }

    virtual ~TypedRenderer(){};

    virtual void Register(T* typedRenderObjects)   = 0;
    virtual void Unregister(T* typedRenderObjects) = 0;

private:
    virtual void Submit(RenderQueue* renderQueue, const FrameView* view) = 0;
};
