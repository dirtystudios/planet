//
// Created by Eugene Sturm on 4/5/15.
//

#include "GLRenderDevice.h"
#include "../../GLHelpers.h"

#define ARRAY_LEN(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define SafeGet(x, idx) x[idx]
    
//inline GLenum SafeGet(GLenum* x, uint32_t idx) {
//    assert(idx < ARRAY_LEN(x)); 
//    return x[idx];
//};

namespace graphics {
    IndexBufferHandle RenderDeviceGL::CreateIndexBuffer(void* data, size_t size, BufferUsage usage) {
        uint32_t handle = GenerateHandle();
        
        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        IndexBufferGL ib = {};
        ib.id = id;

        GLenum gl_usage = SafeGet(buffer_usage_mapping, (uint32_t)usage);

        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_usage));
        
        _index_buffers.insert(std::make_pair(handle, ib));
        
        return handle;
    }
    
    void RenderDeviceGL::DestroyIndexBuffer(IndexBufferHandle handle) {
        auto it = _index_buffers.find(handle);
        if(it == _index_buffers.end()) {
            SLOG_E("gfx", "Could not find index buffer with handle " << handle);
            return;
        }
        IndexBufferGL &ib = (*it).second;
        
        if(ib.id) {
            GL_CHECK(glDeleteBuffers(1, &ib.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        _index_buffers.erase(it);
    }
    
    VertexBufferHandle RenderDeviceGL::CreateVertexBuffer(const VertLayout &layout, void *data, size_t size, BufferUsage usage) {
        uint32_t handle = GenerateHandle();

        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        VertexBufferGL vb = {};
        vb.id = id;
        
        GLenum gl_usage = SafeGet(buffer_usage_mapping, (uint32_t)usage);

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb.id));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage));
        vb.layout = layout;

        _vertex_buffers.insert(std::make_pair(handle, vb));
        
        return handle;
    }


    void RenderDeviceGL::DestroyVertexBuffer(VertexBufferHandle handle) {
        auto it = _vertex_buffers.find(handle);
        if(it == _vertex_buffers.end()) {
            SLOG_E("gfx", "Could not find vertex buffer with handle " << handle);
            return;
        }
        VertexBufferGL &vb = (*it).second;
        
        if(vb.id) {
            GL_CHECK(glDeleteBuffers(1, &vb.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        _vertex_buffers.erase(it);
    }
    
    ShaderHandle RenderDeviceGL::CreateShader(ShaderType shader_type, const char** source) {
        GLenum gl_shader_type = SafeGet(shader_type_mapping, (uint32_t)shader_type);
        GLuint id = glCreateShader(gl_shader_type);
        
        GL_CHECK(glShaderSource(id, 1, (const GLchar**)source, NULL));
        GL_CHECK(glCompileShader(id));
        
        
        GLint compiled = 0;
        GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &compiled) );
        
        if (!compiled) {
            GLsizei len;
            char log[1024];
            GL_CHECK(glGetShaderInfoLog(id, sizeof(log), &len, log));
            SLOG_E("gfx", "Failed to compile shader. " << compiled << " : " << log);
        } else {
            std::string shader_str = "";
            switch(gl_shader_type){
                case GL_VERTEX_SHADER: {
                    shader_str = "VertexShader";
                    break;
                }
                case GL_FRAGMENT_SHADER: {
                    shader_str = "FragmentShader";
                    break;
                }
                case GL_TESS_CONTROL_SHADER: {
                    shader_str = "TessControlShader";
                    break;
                }
                case GL_TESS_EVALUATION_SHADER: {
                    shader_str = "TessEvalShader";
                    break;
                }

            }
            SLOG_D("gfx", "Compiled Shader (" << shader_str << ")");
        }
        
        uint32_t handle = GenerateHandle();
        ShaderGL shader = {};
        shader.id = id;
        
        _shaders.insert(std::make_pair(handle, shader));
        return handle;
    }
    
    void RenderDeviceGL::DestroyShader(ShaderHandle handle) {
        auto it = _shaders.find(handle);
        if(it == _shaders.end()) {
            SLOG_E("gfx", "Could not find shader with handle " << handle);
            return;
        }
        ShaderGL &shader = (*it).second;
        
        if(shader.id) {
            GL_CHECK(glDeleteShader(shader.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        
        _shaders.erase(it);
    }
    
    ConstantBufferHandle RenderDeviceGL::CreateConstantBuffer(const MemoryLayout& layout, void* data, BufferUsage usage) {
        uint32_t handle = GenerateHandle();
        
        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        ConstantBufferGL cb = {};
        cb.id = id;
        cb.layout = layout;
        
        GLenum gl_usage = SafeGet(buffer_usage_mapping, (uint32_t)usage);

        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, cb.id));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, cb.layout.stride, data, gl_usage));
        
        _constant_buffers.insert(std::make_pair(handle, cb));
        return handle;
    }
    
    void RenderDeviceGL::DestroyConstantBuffer(ConstantBufferHandle handle) {
        auto it = _constant_buffers.find(handle);
        if(it == _constant_buffers.end()) {
            SLOG_E("gfx", "Could not find constant buffer with handle " << handle);
            return;
        }
        ConstantBufferGL &cb = (*it).second;
        
        if(cb.id) {
            GL_CHECK(glDeleteBuffers(1, &cb.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        
        _constant_buffers.erase(it);
    }
    
    ProgramHandle RenderDeviceGL::CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders) {
        bool vs = false, tcs = false, tes = false, fs = false;
        for(uint32_t idx = 0; idx < num_shaders; ++idx) {
            ShaderGL *shader = GetShader(shader_handles[idx]);
            assert(shader);
            switch(shader->type) {
                case GL_VERTEX_SHADER: {
                    assert(!vs);
                    vs = true;
                    break;
                }
                case GL_TESS_CONTROL_SHADER: {
                    assert(!tcs);
                    tcs = true;
                    break;
                }
                case GL_TESS_EVALUATION_SHADER: {
                    assert(!tes);
                    tes = true;
                    break;
                }
                case GL_FRAGMENT_SHADER: {
                    assert(!fs);
                    fs = true;
                    break;
                }
            }
        }
        
        GLuint id = glCreateProgram();
        for(uint32_t idx = 0; idx < num_shaders; ++idx) {
            ShaderGL *shader = GetShader(shader_handles[idx]);
            GL_CHECK(glAttachShader(id, shader->id));
        }
        GL_CHECK(glLinkProgram(id));
        
        GLint linked = 0;
        GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, &linked));
        if(!linked) {
            char log[1024];
            GL_CHECK(glGetProgramInfoLog(id, 1024, nullptr, log));
            SLOG_FATAL("gfx", "Failed to link program: " << log);
            exit(1);
        }
        
        ProgramGL program;
        program.id = id;
        
        GLint max_len;
        GLint attrib_count = 0;
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &attrib_count));
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len));
        SLOG_D("gfx", "numattribs:" << attrib_count);
        char* name = new char[max_len];
        for(int32_t idx = 0; idx < attrib_count; ++idx) {
            GLint size;
            GLenum type;
            GL_CHECK(glGetActiveUniform(id, idx, max_len, nullptr, &size, &type, name));
            SLOG_D("gfx", "Program uniform: " << name);
        }
        delete [] name;
        
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attrib_count));
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len));
        
        name = new char[max_len];
        
        AttribLayout &layout = program.layout;
        for(int32_t idx = 0; idx < attrib_count; ++idx) {
            GLint size;
            GLenum type;
            
            GL_CHECK(glGetActiveAttrib(id, idx, max_len, NULL, &size, &type, name));
            SLOG_D("gfx", "Attribute [loc:" << idx << ", name:" << name << ", type:" << glEnumName(type) <<", size:" << size << "]");
            
            if(size == 1) {
                if(type == GL_FLOAT_VEC2) {
                    layout.Add(ParamType::Float2, name, idx, size);
                } else if(type == GL_FLOAT_VEC3) {
                    layout.Add(ParamType::Float3, name, idx, size);
                } else if(type == GL_FLOAT_VEC4) {
                    layout.Add(ParamType::Float4, name, idx, size);
                } else {
                    SLOG_W("gfx", "Attribute not registered");
                }
            } else {
                SLOG_W("gfx", "Attribute not registered, size > 1");
            }
        }
        delete [] name;
        
        uint32_t handle = GenerateHandle();
        _programs.insert(std::make_pair(handle, program));
        
        SLOG_D("gfx", "Created Program " << handle);
        
        return handle;
    }
    
    void RenderDeviceGL::DestroyProgram(ProgramHandle handle) {
        auto it = _programs.find(handle);
        if(it == _programs.end()) {
            SLOG_E("gfx", "Could not find program with handle " << handle);
            return;
        }
        ProgramGL &program = (*it).second;
        
        if(program.id) {
            GL_CHECK(glDeleteProgram(program.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        
        _programs.erase(it);
        SLOG_D("gfx", "Destroyed Program " << handle);
    }
    
    TextureHandle RenderDeviceGL::CreateTexture2D(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void* data) {
        GLenum gl_tex_format = SafeGet(texture_format_mapping, (uint32_t)tex_format);
        GLenum gl_data_type = SafeGet(data_type_mapping, (uint32_t)data_type);
        GLenum gl_data_format = SafeGet(data_format_mapping, (uint32_t)data_format);

        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, gl_tex_format, width, height, 0, gl_data_format, gl_data_type, data));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
        uint32_t handle = GenerateHandle();
        TextureGL texture = {};
        texture.id = id;
        texture.target = GL_TEXTURE_2D;
        _textures.insert(std::make_pair(handle, texture));
        return handle;
    }
    
    TextureHandle RenderDeviceGL::CreateTextureCube(TextureFormat tex_format, DataType data_type, DataFormat data_format, uint32_t width, uint32_t height, void** data) {
        GLenum gl_tex_format = SafeGet(texture_format_mapping, (uint32_t)tex_format);
        GLenum gl_data_type = SafeGet(data_type_mapping, (uint32_t)data_type);
        GLenum gl_data_format = SafeGet(data_format_mapping, (uint32_t)data_format);

        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
        for(uint32_t side = 0; side < 6; ++side) {
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, gl_tex_format, width, height, 0, gl_data_type, gl_data_format, data[side]));
        }
        
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        
        uint32_t handle = GenerateHandle();
        TextureGL texture = {};
        texture.id = id;
        texture.target = GL_TEXTURE_CUBE_MAP;
        _textures.insert(std::make_pair(handle, texture));
        return handle;
    }

    void RenderDeviceGL::DestroyTexture(TextureHandle handle) {
        auto it = _textures.find(handle);
        if(it == _textures.end()) {
            SLOG_E("gfx", "Could not find texture with handle " << handle);
            return;
        }
        TextureGL &texture = (*it).second;
        
        if(texture.id) {
            GL_CHECK(glDeleteTextures(1, &texture.id));
        } else {
            SLOG_W("gfx", "que?");
        }
        
        _textures.erase(it);
    }

    void RenderDeviceGL::UpdateConstantBuffer(ConstantBufferHandle handle, void* data, size_t size, size_t offset) {

    }
    
    void RenderDeviceGL::UpdateTexture(TextureHandle handle, void* data, size_t size) {

    }

    void RenderDeviceGL::BindProgram(ProgramHandle handle) {

    }

    void RenderDeviceGL::SetRasterizerState(uint32_t state) {

    }

    void RenderDeviceGL::SetDepthState(uint32_t state) {

    }

    void RenderDeviceGL::SetBlendState(uint32_t state) {

    }

    void RenderDeviceGL::DrawArrays(VertexBufferHandle handle, uint32_t start_vertex, uint32_t num_vertices) {
        
    }

    void RenderDeviceGL::BindTexture(TextureHandle handle, uint32_t slot) {

    }

    void RenderDeviceGL::BindConstantBuffer(ConstantBufferHandle handle, uint32_t slot) {

    }
    
    bool RenderDeviceGL::BindAttributes(const VertLayout &vert_layout, const AttribLayout &attrib_layout) {
        if(!Matches(attrib_layout, vert_layout)) {
            return false;
        }
        
        size_t offset = 0;
        for(uint32_t idx = 0; idx < attrib_layout.elements.size(); ++idx) {
            const AttribElement* attrib_elem = &attrib_layout.elements[idx];
            const VertElement* vert_elem = &vert_layout.elements[idx];
            
            GLenum type = GL_FALSE;
            GLuint size = 0;
            switch(attrib_elem->type) {
                case ParamType::Float:
                    type = GL_FLOAT;
                    size = 1;
                    break;
                case ParamType::Float2:
                    type = GL_FLOAT;
                    size = 2;
                    break;
                case ParamType::Float3:
                    type = GL_FLOAT;
                    size = 3;
                    break;
                case ParamType::Float4:
                    type = GL_FLOAT;
                    size = 4;
                    break;
                default:
                    SLOG_E("gfx", "Unknown vertex element type");
                    return false;
            }
            
            SLOG_D("gfx", "Binding attribute (size:" << size << ", type:" << type << ", stride:" << vert_layout.stride << ", offset: " << offset << ") to location:" << attrib_elem->location);
            GL_CHECK(glEnableVertexAttribArray(attrib_elem->location));
            GL_CHECK(glVertexAttribPointer(attrib_elem->location, size, type, GL_FALSE, vert_layout.stride, (const void*)offset));
            offset += SizeofParam(vert_elem->type);
        }
        return true;
    }
    
    ShaderGL* RenderDeviceGL::GetShader(ShaderHandle handle) {
        return Get(_shaders, handle);
    }
    
    uint32_t RenderDeviceGL::GenerateHandle() {
        static uint32_t key = 0;
        return ++key;
    }

}
