#include "Renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer() { OnDestroy(); }

void Renderer::Init(gfx::RenderDevice* device, RenderServiceLocator* serviceLocator) {
    _renderServiceLocator = serviceLocator;
    _device               = device;
    OnInit();
}

// ShaderCache* Renderer::shaderCache() { return _renderServiceLocator->shaderCache(); }
//
// PipelineStateCache* Renderer::pipelineStateCache() { return _renderServiceLocator->pipelineStateCache(); }
//
// gfx::RenderDevice* Renderer::GetRenderDevice() { return _renderServiceLocator->GetRenderDevice(); }
//
// VertexLayoutCache* Renderer::vertexLayoutCache() { return _renderServiceLocator->vertexLayoutCache(); }
//
// MeshCache* Renderer::meshCache() { return _renderServiceLocator->meshCache(); }
//
// ConstantBufferManager* Renderer::constantBufferManager() { return _renderServiceLocator->constantBufferManager(); }
// MaterialCache* Renderer::GetMaterialCache() { return _renderServiceLocator->GetMaterialCache(); }
//
void Renderer::OnInit() {}
void Renderer::OnDestroy() {}
