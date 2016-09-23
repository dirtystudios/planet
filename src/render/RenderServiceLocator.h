#pragma once

#include <unordered_map>
#include "ConstantBufferManager.h"
#include "DebugDrawInterface.h"
#include "MaterialCache.h"
#include "MeshCache.h"
#include "PipelineStateCache.h"
#include "RenderDevice.h"
#include "ShaderCache.h"
#include "VertexLayoutCache.h"

class RenderServiceLocator {
public:
    virtual ~RenderServiceLocator() {}
    virtual MaterialCache*         GetMaterialCache()         = 0;
    virtual MeshCache*             GetMeshCache()             = 0;
    virtual ShaderCache*           GetShaderCache()           = 0;
    virtual VertexLayoutCache*     GetVertexLayoutCache()     = 0;
    virtual PipelineStateCache*    GetPipelineStateCache()    = 0;
    virtual gfx::RenderDevice*     GetRenderDevice()          = 0;
    virtual ConstantBufferManager* GetConstantBufferManager() = 0;
};
