//
// Created by Eugene Sturm on 4/5/15.
//

#ifndef DG_RENDER_DEVICE_GL_H
#define DG_RENDER_DEVICE_GL_H

#include "../RenderDevice.h"
#include <unordered_map>
#include "GLUtils.h"
#include "../AttribLayout.h"

namespace graphics {

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

    static GLenum texture_format_mapping[(uint32_t)TextureFormat::COUNT] = {
        GL_R32F,
        GL_RGB32F
    };

    static GLenum data_format_mapping[(uint32_t)DataFormat::COUNT] = {
        GL_RED,
        GL_RGB
    };

    static GLenum data_type_mapping[(uint32_t)DataType::COUNT] = {
        GL_FLOAT
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
    };
    
    struct TextureGL {
        GLuint id { 0 };
        GLenum target { GL_FALSE };
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
    public:
        IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, BufferUsage usage);
        void                    DestroyIndexBuffer(IndexBufferHandle handle);
        
        VertexBufferHandle      CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage);
        void                    DestroyVertexBuffer(VertexBufferHandle handle);
        
        ShaderHandle            CreateShader(ShaderType shader_type, const char **source);
        void                    DestroyShader(ShaderHandle handle);
        
        ProgramHandle           CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders);
        void                    DestroyProgram(ProgramHandle handle);
        
        ConstantBufferHandle    CreateConstantBuffer(const MemoryLayout &layout, void *data, BufferUsage usage);
        void                    DestroyConstantBuffer(ConstantBufferHandle handle);

        TextureHandle           CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data);
        void                    DestroyTexture(TextureHandle handle);

        void UpdateConstantBuffer(ConstantBufferHandle handle, void* data, size_t size, size_t offset);
        void UpdateTexture(TextureHandle handle, void* data, size_t size);
        void BindProgram(ProgramHandle handle);
        void SetRasterizerState(uint32_t state);
        void SetDepthState(uint32_t state);
        void SetBlendState(uint32_t state);
        void DrawArrays(VertexBufferHandle handle, uint32_t start_vertex, uint32_t num_vertices);
        void BindTexture(TextureHandle handle, uint32_t slot);
        void BindConstantBuffer(ConstantBufferHandle handle, uint32_t slot);        
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
