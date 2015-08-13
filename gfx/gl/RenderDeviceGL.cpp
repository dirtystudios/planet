//
// Created by Eugene Sturm on 4/5/15.
//

#include "RenderDeviceGL.h"
#include "../../Assert.h"

namespace gfx {
    IndexBufferHandle RenderDeviceGL::CreateIndexBuffer(void* data, size_t size, GLenum usage) {
        uint32_t handle = GenerateHandle();
        
        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        IndexBufferGL ib = {};
        ib.id = id;
        
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id));
        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage));
        
        _index_buffers.insert(std::make_pair(handle, ib));
        
        return handle;
    }
    
    void RenderDeviceGL::DestroyIndexBuffer(IndexBufferHandle handle) {
        auto it = _index_buffers.find(handle);
        if(it == _index_buffers.end()) {
            LOG_E("gfx", "Could not find index buffer with handle " << handle);
            return;
        }
        IndexBufferGL &ib = (*it).second;
        
        if(ib.id) {
            GL_CHECK(glDeleteBuffers(1, &ib.id));
        } else {
            LOG_W("gfx", "que?");
        }
        _index_buffers.erase(it);
    }
    
    VertexBufferHandle RenderDeviceGL::CreateVertexBuffer() {
        uint32_t handle = GenerateHandle();

        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        VertexBufferGL vb = {};
        vb.id = id;
        
        _vertex_buffers.insert(std::make_pair(handle, vb));
        
        return handle;
    }

    void RenderDeviceGL::UpdateVertexBuffer(VertexBufferHandle handle,
                                   const gfx::VertLayout &layout,
                                   void *data,
                                   size_t size,
                                   GLenum usage) {
        
        VertexBufferGL *vb = Get(_vertex_buffers, handle);

        if(!vb) {
            LOG_E("gfx", "Could not find vertex buffer with handle " << handle);
            return;
        }
        
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb->id));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, usage));
        vb->layout = layout;
    }

    void RenderDeviceGL::DestroyVertexBuffer(VertexBufferHandle handle) {
        auto it = _vertex_buffers.find(handle);
        if(it == _vertex_buffers.end()) {
            LOG_E("gfx", "Could not find vertex buffer with handle " << handle);
            return;
        }
        VertexBufferGL &vb = (*it).second;
        
        if(vb.id) {
            GL_CHECK(glDeleteBuffers(1, &vb.id));
        } else {
            LOG_W("gfx", "que?");
        }
        _vertex_buffers.erase(it);
    }
    
    ShaderHandle RenderDeviceGL::CreateShader(GLenum shader_type, const char** source) {
        GLuint id = glCreateShader(shader_type);
        
        GL_CHECK(glShaderSource(id, 1, (const GLchar**)source, NULL));
        GL_CHECK(glCompileShader(id));
        
        
        GLint compiled = 0;
        GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &compiled) );
        
        if (!compiled) {
            GLsizei len;
            char log[1024];
            GL_CHECK(glGetShaderInfoLog(id, sizeof(log), &len, log));
            LOG_E("gfx", "Failed to compile shader. " << compiled << " : " << log);
        } else {
            std::string shader_str = "";
            switch(shader_type){
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
            LOG_D("gfx", "Compiled Shader (" << shader_str << ")");
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
            LOG_E("gfx", "Could not find shader with handle " << handle);
            return;
        }
        ShaderGL &shader = (*it).second;
        
        if(shader.id) {
            GL_CHECK(glDeleteShader(shader.id));
        } else {
            LOG_W("gfx", "que?");
        }
        
        _shaders.erase(it);
    }
    
    ConstantBufferHandle RenderDeviceGL::CreateConstantBuffer(const MemoryLayout& layout, void* data, GLenum usage) {
        uint32_t handle = GenerateHandle();
        
        GLuint id = 0;
        GL_CHECK(glGenBuffers(1, &id));
        
        ConstantBufferGL cb = {};
        cb.id = id;
        cb.layout = layout;
        
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, cb.id));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, cb.layout.stride, data, usage));
        
        _constant_buffers.insert(std::make_pair(handle, cb));
        return handle;
    }
    
    void RenderDeviceGL::DestroyConstantBuffer(ConstantBufferHandle handle) {
        auto it = _constant_buffers.find(handle);
        if(it == _constant_buffers.end()) {
            LOG_E("gfx", "Could not find constant buffer with handle " << handle);
            return;
        }
        ConstantBufferGL &cb = (*it).second;
        
        if(cb.id) {
            GL_CHECK(glDeleteBuffers(1, &cb.id));
        } else {
            LOG_W("gfx", "que?");
        }
        
        _constant_buffers.erase(it);
    }
    
    ProgramHandle RenderDeviceGL::CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders) {
        bool vs = false, tcs = false, tes = false, fs = false;
        for(uint32_t idx = 0; idx < num_shaders; ++idx) {
            ShaderGL *shader = GetShader(shader_handles[idx]);
            ASSERT(shader, "Failed to find shader");
            switch(shader->type) {
                case GL_VERTEX_SHADER: {
                    ASSERT(!vs, "Can't have multiple");
                    vs = true;
                    break;
                }
                case GL_TESS_CONTROL_SHADER: {
                    ASSERT(!tcs, "Can't have multiple");
                    tcs = true;
                    break;
                }
                case GL_TESS_EVALUATION_SHADER: {
                    ASSERT(!tes, "Can't have multiple");
                    tes = true;
                    break;
                }
                case GL_FRAGMENT_SHADER: {
                    ASSERT(!fs, "Can't have multiple");
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
            LOG_FATAL("gfx", "Failed to link program: " << log);
            exit(1);
        }
        
        ProgramGL program;
        program.id = id;
        
        GLint max_len;
        GLint attrib_count = 0;
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &attrib_count));
        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len));
        LOG_D("gfx", "numattribs:" << attrib_count);
        char* name = new char[max_len];
        for(int32_t idx = 0; idx < attrib_count; ++idx) {
            GLint size;
            GLenum type;
            GL_CHECK(glGetActiveUniform(id, idx, max_len, nullptr, &size, &type, name));
            LOG_D("gfx", "Program uniform: " << name);
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
            LOG_D("gfx", "Attribute [loc:" << idx << ", name:" << name << ", type:" << glEnumName(type) <<", size:" << size << "]");
            
            if(size == 1) {
                if(type == GL_FLOAT_VEC2) {
                    layout.Add(ParamType::Float2, name, idx, size);
                } else if(type == GL_FLOAT_VEC3) {
                    layout.Add(ParamType::Float3, name, idx, size);
                } else if(type == GL_FLOAT_VEC4) {
                    layout.Add(ParamType::Float4, name, idx, size);
                } else {
                    LOG_W("gfx", "Attribute not registered");
                }
            } else {
                LOG_W("gfx", "Attribute not registered, size > 1");
            }
        }
        delete [] name;
        
        uint32_t handle = GenerateHandle();
        _programs.insert(std::make_pair(handle, program));
        
        LOG_D("gfx", "Created Program " << handle);
        
        return handle;
    }
    
    ProgramHandle RenderDeviceGL::CreateProgram(ShaderHandle vertex_shader_handle, ShaderHandle fragment_shader_handle) {
        ShaderHandle handles[2] = { vertex_shader_handle, fragment_shader_handle };
        return CreateProgram(handles, 2);
    }
    
    void RenderDeviceGL::DestroyProgram(ProgramHandle handle) {
        auto it = _programs.find(handle);
        if(it == _programs.end()) {
            LOG_E("gfx", "Could not find program with handle " << handle);
            return;
        }
        ProgramGL &program = (*it).second;
        
        if(program.id) {
            GL_CHECK(glDeleteProgram(program.id));
        } else {
            LOG_W("gfx", "que?");
        }
        
        _programs.erase(it);
        LOG_D("gfx", "Destroyed Program " << handle);
    }
    
    TextureHandle RenderDeviceGL::CreateTexture2D(GLenum tex_format, GLenum data_type, GLenum data_format, uint32_t width, uint32_t height, void* data) {
        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, tex_format, width, height, 0, data_format, data_type, data));
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
    
    TextureHandle RenderDeviceGL::CreateTextureCube(GLenum tex_format, GLenum data_type, GLenum data_format, uint32_t width, uint32_t height, void** data) {
        GLuint id = 0;
        GL_CHECK(glGenTextures(1, &id));
        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
        for(uint32_t side = 0; side < 6; ++side) {
            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, tex_format, width, height, 0, data_format, data_type, data[side]));
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
            LOG_E("gfx", "Could not find texture with handle " << handle);
            return;
        }
        TextureGL &texture = (*it).second;
        
        if(texture.id) {
            GL_CHECK(glDeleteTextures(1, &texture.id));
        } else {
            LOG_W("gfx", "que?");
        }
        
        _textures.erase(it);
    }
    
    
    
    void RenderDeviceGL::Submit(RenderTask *tasks, uint32_t count) {
        for(uint32_t idx = 0; idx < count; ++idx) {
            RenderTask &task = tasks[idx];
            
            ProgramGL *prog = Get(_programs, task.program);
            VertexBufferGL *vb = Get(_vertex_buffers, task.vertex_buffer);
            ASSERT(prog, "Couldn't find program");
            ASSERT(vb, "Couldn't find vertex buffer");

            // find/create the correct vao
            {
                size_t hash = 0;
                hash_combine(hash, prog->id);
                hash_combine(hash, vb->id);
                VertexArrayObjectGL *vao = Get(_vao_cache, hash);
                if(vao) {
                    GL_CHECK(glBindVertexArray(vao->id));
                } else {
                    GLuint id = 0;
                    GL_CHECK(glGenVertexArrays(1, &id));
                    GL_CHECK(glBindVertexArray(id));
                    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb->id));
                    if(!BindAttributes(vb->layout, prog->layout)) {
                        // TODO: need to roll back vao on failure because it has already been cached and created
                        LOG_E("gfx", "Vertex layout does not match program inputs");
                        return;
                    }
                    
                    VertexArrayObjectGL vertex_array = {};
                    vertex_array.id = id;
                    _vao_cache.insert(std::make_pair(hash, vertex_array));
                }
            }
            
            GL_CHECK(glUseProgram(prog->id));

            // update uniforms
            {
                for(auto it = task.uniform_data.begin(); it != task.uniform_data.end(); ++it) {
                    const char* uniform_name = it->first.c_str();
                    ParamType param_type = it->second.first;
                    void* data = it->second.second;
                    
                    GLint location = glGetUniformLocation(prog->id, uniform_name);
                    GL_CHECK();
                    if(location < 0) {
                        LOG_E("gfx", "Failed to find uniform by name '" << uniform_name << "'");
                        continue;
                    }

                    
                    switch(param_type) {
                        case ParamType::Float: {
                            GL_CHECK(glUniform1f(location, (float)*((float*)data)));
                            break;
                        }
                        case ParamType::Int32: {
                            GL_CHECK(glUniform1i(location, (int)*((int*)data)));
                            break;
                        }
                        case ParamType::Float4x4: {
                            GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data));
                            break;
                        }
                        default: {
                            LOG_E("gfx", "Unsupported uniform type");
                        }
                    }
                }
                uint32_t slot = 0;
                for(auto it = task.textures.begin(); it != task.textures.end(); ++it) {
                    const char* uniform_name = it->first.c_str();
                    TextureHandle texture_handle = it->second;
                    TextureGL *texture = Get(_textures, texture_handle);
                    if(!texture) {
                        LOG_W("gfx", "no texture");
                        continue;
                    }
                    
                    GLint location = glGetUniformLocation(prog->id, uniform_name);
                    GL_CHECK();
                    if(location < 0) {
                        LOG_E("gfx", "Failed to find uniform by name '" << uniform_name << "'");
                        continue;
                    }
                    
                    GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
                    GL_CHECK(glBindTexture(texture->target, texture->id));
                    GL_CHECK(glUniform1i(location, slot));
                    slot++;
                }
                
            }
            IndexBufferGL *ib = Get(_index_buffers, task.index_buffer);
            if(ib != NULL) {
                GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib->id));
                glPatchParameteri(GL_PATCH_VERTICES, 4);
                GL_CHECK(glDrawElements(GL_PATCHES, task.vertex_count, GL_UNSIGNED_INT, 0));
                GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
                
            } else {
                GL_CHECK(glDrawArrays(GL_TRIANGLES, task.vertex_start, task.vertex_count));
            }
            

        }
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
                    LOG_E("gfx", "Unknown vertex element type");
                    return false;
            }
            
            LOG_D("gfx", "Binding attribute (size:" << size << ", type:" << type << ", stride:" << vert_layout.stride << ", offset: " << offset << ") to location:" << attrib_elem->location);
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
