#include "Renderer.h"

Renderer::~Renderer() { OnDestroy(); }

void Renderer::Init(gfx::RenderDevice* device, RenderServiceLocator* serviceLocator) {
    _renderServiceLocator = serviceLocator;
    _device               = device;
    OnInit();
}

void Renderer::OnInit() {}
void Renderer::OnDestroy() {}
