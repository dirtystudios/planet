//
// Created by Eugene Sturm on 4/5/15.
//

#ifndef DG_RENDER_DEVICE_GL_H
#define DG_RENDER_DEVICE_GL_H

#include "GLUtils.h"
#include "../ParamType.h"
#include "../AttribLayout.h"
#include "../VertLayout.h"
#include <stdint.h>
#include <unordered_map>
#include <fstream>
#include <sstream>

namespace gfx {
    typedef uint32_t VertexBufferHandle;
    typedef uint32_t ShaderHandle;
    typedef uint32_t ProgramHandle;
    typedef uint32_t TextureHandle;
    typedef int32_t IndexBufferHandle;
    typedef uint32_t UniformHandle;
    typedef int32_t ConstantBufferHandle;
    
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
    
    
    struct StructureElement {
        ParamType type;
        uint32_t count;
        
        StructureElement(ParamType type, uint32_t count) : type(type), count(count) {};
    };
    
    struct MemoryLayout {
        std::vector<StructureElement> elements;
        size_t stride { 0 };
        
        MemoryLayout(std::vector<StructureElement> elements) {
            for(const StructureElement &elem : elements) {
                stride += SizeofParam(elem.type) * elem.count;
                elements.push_back(elem);
            }
        };
    };
    
    struct ConstantBufferGL {
        GLuint id { 0 };
        MemoryLayout layout;
    };
    
    struct RenderTask {
        ProgramHandle program { 0 };
        VertexBufferHandle vertex_buffer { 0 };
        IndexBufferHandle index_buffer { 0 };
        uint32_t vertex_count { 0 };
        uint32_t vertex_start { 0 };
        
        // Note(eugene): This is an utter hack.
        std::vector<std::pair<std::string, std::pair<ParamType, void*>>> uniform_data;
        // Note(eugene): need to know uniform name to set slot...another hack
        std::vector<std::pair<std::string, TextureHandle>> textures;
        
        void AddUniformData(const char* uniform_name, ParamType paramType, void* data_ptr) {
            uniform_data.push_back(std::make_pair(uniform_name, std::make_pair(paramType, data_ptr)));
        }
        
        void AddTexture(const char* uniform_name, TextureHandle handle) {
            textures.push_back(std::make_pair(uniform_name, handle));
        }
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
        IndexBufferHandle       CreateIndexBuffer(void* data, size_t size, GLenum usage);
        void                    DestroyIndexBuffer(IndexBufferHandle handle);
        
        VertexBufferHandle      CreateVertexBuffer();
        void                    UpdateVertexBuffer(VertexBufferHandle handle, const VertLayout &layout, void *data, size_t size, GLenum usage);
        void                    DestroyVertexBuffer(VertexBufferHandle handle);
        
        ShaderHandle            CreateShader(GLenum shader_type, const char **source);
        void                    DestroyShader(ShaderHandle handle);
        
        ConstantBufferHandle    CreateConstantBuffer(const MemoryLayout& layout, void* data, GLenum usage);
        void                    DestroyConstantBuffer(ConstantBufferHandle handle);
        
        ProgramHandle           CreateProgram(ShaderHandle vertex_shader, ShaderHandle fragment_shader);
        ProgramHandle           CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders);
        void                    DestroyProgram(ProgramHandle handle);
                    
        TextureHandle           CreateTexture2D(GLenum tex_format, GLenum data_type, GLenum data_format, uint32_t width, uint32_t height, void* data);
        TextureHandle           CreateTextureCube(GLenum tex_format, GLenum data_type, GLenum data_format, uint32_t width, uint32_t height, void** data);
        void                    DestroyTexture(TextureHandle handle);
        
        void                    Submit(RenderTask *task, uint32_t count);
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

static gfx::ShaderHandle CreateShaderFromFile(gfx::RenderDeviceGL *device, GLenum shader_type, const char *fpath) {
    std::ifstream fin(fpath);

    if(fin.fail()) {
        LOG_E("main", "Failed to open file '" << fpath << "'");
        return 0;
    }

    std::string ss((std::istreambuf_iterator<char>(fin)),
                    std::istreambuf_iterator<char>());

    const char* source = ss.c_str();
    gfx::ShaderHandle handle = device->CreateShader(shader_type, &source);
    fin.close();

    return handle;
}


#endif //DG_RENDER_DEVICE_GL_H
