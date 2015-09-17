//
//  Graphics.h
//  dg
//
//  Created by Eugene Sturm on 5/18/15.
//
//

#ifndef __dg__Graphics__
#define __dg__Graphics__

#include "VertLayout.h"
#include "MemLayout.h"

namespace graphics {
    enum class TextureSlot : uint32_t {
        BASE = 0,
        NORMAL,
        COUNT
    };

    enum class ConstantBufferSlot : uint32_t {
        ENGINE_PER_FRAME = 0,
        ENGINE_PER_VIEW,
        ENGINE_PER_INSTANCE,
        USER_PER_FRAME,
        USER_PER_VIEW,
        USER_PER_INSTANCE,
        COUNT
    };

    enum class PrimitiveType : uint32_t {
        TRIANGLES = 0,
        PATCHES_4,
        COUNT
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
        RGBA32F,
        COUNT
    };

    enum class DataFormat : uint32_t {
        RED = 0,
        RGB,
        RGBA,
        COUNT
    };

    enum class DataType : uint32_t {
        FLOAT = 0,
        COUNT
    };

    struct DeviceConfiguration {
        std::string DeviceAbbreviation;
        // Extension including dot
        std::string ShaderExtension;
    };

    typedef uint32_t VertexBufferHandle;
    typedef uint32_t ShaderHandle;
    typedef uint32_t ProgramHandle;
    typedef uint32_t TextureHandle;
    typedef uint32_t IndexBufferHandle;    

    class RenderDevice {
    public:
        DeviceConfiguration             DeviceConfig;

        virtual int                     InitializeDevice(void *windowHandle, uint32_t windowHeight, uint32_t windowWidth) = 0;
        virtual IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyIndexBuffer(IndexBufferHandle handle) = 0;

        virtual VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyVertexBuffer(VertexBufferHandle handle) = 0;

        virtual ShaderHandle            CreateShader(ShaderType shader_type, const char **source) = 0;
        virtual void                    DestroyShader(ShaderHandle handle) = 0;

        virtual TextureHandle           CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data) = 0;
        virtual TextureHandle           CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) = 0;
        virtual TextureHandle           CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data) = 0;
        virtual void                    DestroyTexture(TextureHandle handle) = 0;

        virtual void                    SwapBuffers() = 0;

        virtual void                    PrintDisplayAdapterInfo()=0;

        // "Commands"
        virtual void UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t height, DataType data_type, DataFormat data_format, void* data) = 0;
        virtual void UpdateTexture(TextureHandle handle, void* data, size_t size) = 0;
        virtual void SetRasterizerState(uint32_t state) = 0;
        virtual void SetDepthState(uint32_t state) = 0;
        virtual void SetBlendState(uint32_t state) = 0;

        virtual void Clear(float r, float g, float b, float a) = 0;
        virtual void SetVertexShader(ShaderHandle shader_handle) = 0;
        virtual void SetPixelShader(ShaderHandle shader_handle) = 0;
        virtual void SetShaderParameter(ShaderHandle handle, ParamType param_type, const char *param_name, void *data) = 0;
        virtual void SetShaderTexture(ShaderHandle shader_handle, TextureHandle texture_handle, TextureSlot slot) = 0;
        virtual void SetVertexBuffer(VertexBufferHandle handle) = 0;
        virtual void DrawPrimitive(PrimitiveType primitive_type, uint32_t start_vertex, uint32_t num_vertices) = 0;
    };



}

#endif /* defined(__dg__Graphics__) */
