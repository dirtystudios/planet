#include "Renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer() { OnDestroy(); }

void Renderer::Init(RenderServiceLocator* serviceLocator) {
    _renderServiceLocator = serviceLocator;
    OnInit();
}

ShaderCache* Renderer::GetShaderCache() { return _renderServiceLocator->GetShaderCache(); }

PipelineStateCache* Renderer::GetPipelineStateCache() { return _renderServiceLocator->GetPipelineStateCache(); }

gfx::RenderDevice* Renderer::GetRenderDevice() { return _renderServiceLocator->GetRenderDevice(); }

VertexLayoutCache* Renderer::GetVertexLayoutCache() { return _renderServiceLocator->GetVertexLayoutCache(); }

MeshCache* Renderer::GetMeshCache() { return _renderServiceLocator->GetMeshCache(); }

ConstantBufferManager* Renderer::GetConstantBufferManager() { return _renderServiceLocator->GetConstantBufferManager(); }
MaterialCache* Renderer::GetMaterialCache() { return _renderServiceLocator->GetMaterialCache(); }

void Renderer::OnInit() {}
void Renderer::OnDestroy() {}
