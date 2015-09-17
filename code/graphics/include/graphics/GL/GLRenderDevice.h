//
// Created by Eugene Sturm on 4/5/15.
//

#ifndef DG_RENDER_DEVICE_GL_H
#define DG_RENDER_DEVICE_GL_H

#include "graphics/RenderDevice.h"
#include "graphics/GL/GLUtils.h"
#include "graphics/AttribLayout.h"
#include <unordered_map>
#include <cassert>

namespace graphics {

    struct ShaderParameter {
        ShaderParameter() {

        }
        ShaderParameter(ShaderHandle handle, const char *name, ParamType type, void *data) 
        : handle(handle), param_name(name), param_type(type) {
            size_t size = SizeofParam(param_type);
            param_data = malloc(size);
            memcpy(param_data, data, size);
        }        

        ~ShaderParameter() {
            if(param_data) {
                free(param_data);
            }
        }

        ShaderHandle handle;
        const char *param_name;
        ParamType param_type;
        void *param_data { 0 };
    };

    struct ShaderParameters {
        std::vector<ShaderParameter*> parameters;

        void Set(ShaderHandle handle, ParamType type, const char *name, void* data) {
            ShaderParameter* param = new ShaderParameter(handle, name, type, data);
            parameters.push_back(param);
        }        

        void Clear() {
            for(ShaderParameter* param : parameters) {
                delete param;
            }
            parameters.clear();
        }
    };

    struct ShaderTexture {
        ShaderHandle shader_handle;
        TextureHandle texture_handle;
        TextureSlot slot;
    };

    struct ShaderTextures {
        std::vector<ShaderTexture> textures;
      
        void Set(ShaderHandle shader_handle, TextureHandle texture_handle, TextureSlot slot) {
            ShaderTexture texture;
            texture.shader_handle = shader_handle;
            texture.texture_handle = texture_handle;
            texture.slot = slot;
            textures.push_back(texture);
        }

        void Clear() {
            textures.clear();
        }
    };

    struct ContextStateGL {
        float clear_color[3] { -1, -1, -1 };

        ShaderParameters shader_parameters;
        ShaderTextures shader_textures;

        ShaderHandle vertex_shader_handle;
        ShaderHandle pixel_shader_handle;

        uint32_t raster_state;
        uint32_t blend_state;
        uint32_t depth_state;

        IndexBufferHandle index_buffer_handle;
        VertexBufferHandle vertex_buffer_handle;
    };    

    static GLenum buffer_usage_mapping[(uint32_t)BufferUsage::COUNT] = {
        GL_STATIC_DRAW,
        GL_DYNAMIC_DRAW
    };

    static GLenum shader_type_mapping[(uint32_t)ShaderType::COUNT] = {
        GL_VERTEX_SHADER,
        GL_FALSE,
        GL_FALSE,
        GL_FRAGMENT_SHADER
    };

    struct GLTextureFormatDesc {
        GLenum internal_format;
        GLenum data_type;
        GLenum data_format;
    };

    static GLTextureFormatDesc texture_format_mapping[(uint32_t)TextureFormat::COUNT] = {
        { GL_R32F, GL_FLOAT, GL_RED },
        { GL_RGB32F, GL_FLOAT, GL_RGB },
        { GL_RGBA32F, GL_FLOAT, GL_RGBA }
    };

    static GLenum data_format_mapping[(uint32_t)DataFormat::COUNT] = {
        GL_RED,
        GL_RGB,
        GL_RGBA,
    };

    static GLenum data_type_mapping[(uint32_t)DataType::COUNT] = {
        GL_FLOAT
    };

    static GLenum primitive_type_mapping[(uint32_t)PrimitiveType::COUNT] = {
        GL_TRIANGLES
    };


    struct IndexBufferGL {
        GLuint id { 0 };
    };


    struct VertexBufferGL {
        GLuint id { 0 };
        VertLayout layout;
    };

    struct ShaderGL {
        GLuint id { 0 };
        GLenum type { GL_FALSE };
    };

    struct ProgramGL {
        GLuint id { 0 };
        AttribLayout layout;
        std::unordered_map<const char*, GLint> parameter_map_cache;

        GLint GetLocation(const char* param_name) {
            auto it = parameter_map_cache.find(param_name);
            if(it == parameter_map_cache.end()) {
                GLint loc = glGetUniformLocation(id, param_name);
                assert(loc >= 0);
                parameter_map_cache[param_name] = loc;
            }
            return parameter_map_cache[param_name];
        }
    };

    struct TextureGL {
        GLuint id { 0 };
        GLenum target { GL_FALSE };
        GLTextureFormatDesc* texture_desc { NULL };
    };

    struct VertexArrayObjectGL {
        GLuint id { 0 };
    };


    struct ConstantBufferGL {
        GLuint id { 0 };
        MemoryLayout layout;
    };


    class RenderDeviceGL : public RenderDevice {
    private:
        std::unordered_map<uint32_t, IndexBufferGL> _index_buffers;
        std::unordered_map<uint32_t, VertexBufferGL> _vertex_buffers;
        std::unordered_map<uint32_t, ProgramGL> _programs;
        std::unordered_map<uint32_t, ShaderGL> _shaders;
        std::unordered_map<uint32_t, TextureGL> _textures;
        std::unordered_map<uint32_t, ConstantBufferGL> _constant_buffers;
        std::unordered_map<uint32_t, VertexArrayObjectGL> _vao_cache;
        //std::unordered_map<uint32_t, GLuint> _program_pipelines;

        GLuint _pipeline;

        ContextStateGL _current_state;
        ContextStateGL _pending_state;
    public:
      RenderDeviceGL();
      virtual int                     InitializeDevice(void *windowHandle, uint32_t windowHeight, uint32_t windowWidth) { return 1; };
      virtual IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage);
      virtual void                    DestroyIndexBuffer(IndexBufferHandle handle);

      virtual VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage);
      virtual void                    DestroyVertexBuffer(VertexBufferHandle handle);

      virtual ShaderHandle            CreateShader(ShaderType shader_type, const char **source);
      virtual void                    DestroyShader(ShaderHandle handle);

      virtual TextureHandle           CreateTexture2D(TextureFormat tex_format, uint32_t width, uint32_t height, void* data);
      virtual TextureHandle           CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth);
      virtual TextureHandle           CreateTextureCube(TextureFormat tex_format, uint32_t width, uint32_t height, void** data);
      virtual void                    DestroyTexture(TextureHandle handle);

      void                            SwapBuffers() {};
      void                            PrintDisplayAdapterInfo();

      // "Commands"
      virtual void UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t height, void* data);
      virtual void UpdateTexture(TextureHandle handle, void* data, size_t size);
      virtual void SetRasterizerState(uint32_t state);
      virtual void SetDepthState(uint32_t state);
      virtual void SetBlendState(uint32_t state);

      virtual void Clear(float r, float g, float b, float a);
      virtual void SetVertexShader(ShaderHandle shader_handle);
      virtual void SetPixelShader(ShaderHandle shader_handle);
      virtual void SetShaderParameter(ShaderHandle handle, ParamType param_type, const char *param_name, void *data);
      virtual void SetShaderTexture(ShaderHandle shader_handle, TextureHandle texture_handle, TextureSlot slot);
      virtual void SetVertexBuffer(VertexBufferHandle handle);
      virtual void DrawPrimitive(PrimitiveType primitive_type, uint32_t start_vertex, uint32_t num_vertices);

    private:
        template <class T> T* Get(std::unordered_map<uint32_t, T> &map, uint32_t handle) {
            auto it = map.find(handle);
            if(it == map.end()) {
                return nullptr;
            }
            return &(*it).second;
        }

        ShaderGL*   GetShader(ShaderHandle handle);
        uint32_t    GenerateHandle();
        bool        BindAttributes(const VertLayout &vert_layout, const AttribLayout &attrib_layout);


    };
}




#endif //DG_RENDER_DEVICE_GL_H
