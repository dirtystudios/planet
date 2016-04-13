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
#include <string>

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
        LINESTRIP,
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
        R_UBYTE,
        RGB_UBYTE,
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
    
    
    
    enum class BlendMode : uint32_t {
        ADD = 0,
        SUBTRACT,
        REVERSE_SUBTRACT,
        MIN,
        MAX,
        COUNT
    };
    
    enum class BlendFunc : uint32_t {
        ZERO = 0,
        ONE, 
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE,
        SRC1_COLOR,
        ONE_MINUS_SRC1_COLOR,
        SRC1_ALPHA,
        ONE_MINUS_SRC1_ALPHA,
        COUNT
    };
    
    enum class FillMode : uint32_t {
        WIREFRAME = 0,
        SOLID,
        COUNT
    };
    
    enum class CullMode : uint32_t {
        NONE = 0,
        FRONT,
        BACK,
        COUNT
    };
    
    enum class WindingOrder : uint32_t {
        FRONT_CCW = 0,
        FRONT_CW,
        COUNT
    };
    
    enum class DepthWriteMask : uint32_t {
        ZERO = 0,
        ALL,
        COUNT
    };
    
    enum class DepthFunc : uint32_t {
        NEVER        = 0,
        LESS         ,
        EQUAL        ,
        LESS_EQUAL   ,
        GREATER      ,
        NOT_EQUAL    ,
        GREATER_EQUAL,
        ALWAYS       ,
        COUNT
    };
    
    struct DepthState {
        bool enable                     { false };
        DepthWriteMask depth_write_mask { DepthWriteMask::ALL };
        DepthFunc depth_func            { DepthFunc::LESS };
    };
    
    struct RasterState {
        FillMode fill_mode          { FillMode::SOLID };
        CullMode cull_mode          { CullMode::NONE };
        WindingOrder winding_order  { WindingOrder::FRONT_CCW };        
    };
    
    struct BlendState {
        bool enable                 { false };
        BlendFunc src_rgb_func      { graphics::BlendFunc::ONE  };
        BlendFunc src_alpha_func    { graphics::BlendFunc::ZERO };
        BlendFunc dst_rgb_func      { graphics::BlendFunc::ONE  };
        BlendFunc dst_alpha_func    { graphics::BlendFunc::ZERO };
        BlendMode rgb_mode          { graphics::BlendMode::ADD  };
        BlendMode alpha_mode        { graphics::BlendMode::ADD  };
    };

    struct DeviceConfiguration {
        std::string DeviceAbbreviation;
        // Extension including dot
        std::string ShaderExtension;
    };

    struct DeviceInitialization {
        void* windowHandle { 0 };
        uint32_t windowHeight { 0 };
        uint32_t windowWidth { 0 };
        bool usePrebuiltShaders { false };
    };

    typedef uint32_t VertexBufferHandle;
    typedef uint32_t ShaderHandle;
    typedef uint32_t ProgramHandle;
    typedef uint32_t TextureHandle;
    typedef uint32_t IndexBufferHandle;    

    class RenderDevice {
    public:
        DeviceConfiguration             DeviceConfig;

        virtual int                     InitializeDevice(DeviceInitialization deviceInitialization) = 0;
        virtual IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyIndexBuffer(IndexBufferHandle handle) = 0;

        virtual VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) = 0;
        virtual void                    DestroyVertexBuffer(VertexBufferHandle handle) = 0;

        virtual ShaderHandle            CreateShader(ShaderType shader_type, const char **source) = 0;
        virtual void                    DestroyShader(ShaderHandle handle) = 0;

        virtual TextureHandle           CreateTexture2D(TextureFormat tex_format, uint32_t width, uint32_t height, void* data) = 0;
        virtual TextureHandle           CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) = 0;
        virtual TextureHandle           CreateTextureCube(TextureFormat tex_format, uint32_t width, uint32_t height, void** data) = 0;
        virtual void                    DestroyTexture(TextureHandle handle) = 0;

        virtual void                    SwapBuffers() = 0;
        virtual void                    ResizeWindow(uint32_t width, uint32_t height) = 0;
        virtual void                    PrintDisplayAdapterInfo()=0;

        // "Commands"
        virtual void SetBlendState(const BlendState& blend_state) = 0;
        virtual void SetDepthState(const DepthState& depth_state) = 0;
        virtual void SetRasterState(const RasterState& raster_state) = 0;
        
        virtual void UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t height, void* data) = 0;
        virtual void UpdateTexture(TextureHandle handle, void* data, size_t size) = 0;
        virtual void UpdateVertexBuffer(VertexBufferHandle vertexBufferHandle, void* data, size_t size) = 0;

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
