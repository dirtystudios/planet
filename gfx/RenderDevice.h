//
//  Graphics.h
//  dg
//
//  Created by Eugene Sturm on 5/18/15.
//
//

#ifndef __dg__Graphics__
#define __dg__Graphics__

#include <stdint.h>
#include "VertLayout.h"
#include "MemLayout.h"

namespace graphics {
    enum class PrimitiveType : uint32_t {
        TRIANGLES,
        PATCHES_4
    };
    
    enum class BufferUsage : uint32_t {
        STATIC = 0,
        DYNAMIC,
        COUNT
    };
    
    enum class ShaderType : uint32_t {
        VERTEX_SHADER = 0,
        TESS_CONTROL_SHADER,
        TESS_EVAL_SHADER,
        FRAGMENT_SHADER,
        COUNT
    };
    
    enum class TextureFormat : uint32_t{
        R32F = 0,
        RGB32F,
        COUNT
    };
    
    enum class DataFormat : uint32_t {
        RED = 0,
        RGB,
        COUNT
    };

    enum class DataType : uint32_t {
        FLOAT = 0,
        COUNT
    };

    
    typedef uint32_t VertexBufferHandle;
    typedef uint32_t ShaderHandle;
    typedef uint32_t ProgramHandle;
    typedef uint32_t TextureHandle;
    typedef uint32_t IndexBufferHandle;
    typedef uint32_t UniformHandle;
    typedef uint32_t ConstantBufferHandle;
        
    class RenderDevice {
    public:
        virtual IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyIndexBuffer(IndexBufferHandle handle) = 0;
        
        virtual VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyVertexBuffer(VertexBufferHandle handle) = 0;
        
        virtual ShaderHandle            CreateShader(ShaderType shader_type, const char **source) = 0;
        virtual void                    DestroyShader(ShaderHandle handle) = 0;
        
        virtual ProgramHandle           CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders) = 0;
        virtual void                    DestroyProgram(ProgramHandle handle) = 0;
        
        virtual ConstantBufferHandle    CreateConstantBuffer(const MemoryLayout &layout, void *data, BufferUsage usage) = 0;
        virtual void                    DestroyConstantBuffer(ConstantBufferHandle handle) = 0;
        
        virtual TextureHandle           CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data) = 0;
        virtual TextureHandle           CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data) = 0;
        virtual void                    DestroyTexture(TextureHandle handle) = 0;



        // "Commands"
        virtual void UpdateConstantBuffer(ConstantBufferHandle handle, void* data, size_t size, size_t offset) = 0;
        virtual void UpdateTexture(TextureHandle handle, void* data, size_t size) = 0;
        virtual void BindProgram(ProgramHandle handle) = 0;
        virtual void SetRasterizerState(uint32_t state) = 0;
        virtual void SetDepthState(uint32_t state) = 0;
        virtual void SetBlendState(uint32_t state) = 0;
        virtual void DrawArrays(VertexBufferHandle handle, uint32_t start_vertex, uint32_t num_vertices) = 0;
        virtual void BindTexture(TextureHandle handle, uint32_t slot) = 0;
        virtual void BindConstantBuffer(ConstantBufferHandle handle, uint32_t slot) = 0;
    };

}

#endif /* defined(__dg__Graphics__) */
