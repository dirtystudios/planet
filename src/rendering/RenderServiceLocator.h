#pragma once

#include "RenderDevice.h"
#include "ShaderCache.h"
#include "PipelineStateCache.h"
#include "VertexLayoutCache.h"
#include "MeshCache.h"

class RenderServiceLocator {
public:
    virtual ~RenderServiceLocator() {}
    virtual MeshCache* GetMeshCache() = 0;
    virtual ShaderCache* GetShaderCache() = 0;
    virtual VertexLayoutCache* GetVertexLayoutCache() = 0;
    virtual PipelineStateCache* GetPipelineStateCache() = 0;
    virtual graphics::RenderDevice* GetRenderDevice() = 0;
};