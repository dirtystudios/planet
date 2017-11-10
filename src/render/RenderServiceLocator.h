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
#include "AnimationCache.h"

class RenderServiceLocator {
public:
    virtual ~RenderServiceLocator() {}
    virtual MaterialCache*         materialCache()         = 0;
    virtual MeshCache*             meshCache()             = 0;
    virtual ShaderCache*           shaderCache()           = 0;
    virtual VertexLayoutCache*     vertexLayoutCache()     = 0;
    virtual PipelineStateCache*    pipelineStateCache()    = 0;
    virtual ConstantBufferManager* constantBufferManager() = 0;
    virtual DebugDrawInterface*    debugDraw()             = 0;
    virtual AnimationCache*        animationCache()        = 0;
};
