#pragma once

#include "RenderDevice.h"
#include "ShaderCache.h"
#include "PipelineStateCache.h"
#include "VertexLayoutCache.h"
#include "MeshCache.h"
#include <unordered_map>
#include "ConstantBufferManager.h"
#include "MaterialCache.h"

class RenderServiceLocator {
private:    
public:    
    virtual ~RenderServiceLocator() {}
    virtual MaterialCache* GetMaterialCache() = 0;
    virtual MeshCache* GetMeshCache() = 0;
    virtual ShaderCache* GetShaderCache() = 0;
    virtual VertexLayoutCache* GetVertexLayoutCache() = 0;
    virtual PipelineStateCache* GetPipelineStateCache() = 0;
    virtual graphics::RenderDevice* GetRenderDevice() = 0;
    virtual ConstantBufferManager* GetConstantBufferManager() = 0;
};