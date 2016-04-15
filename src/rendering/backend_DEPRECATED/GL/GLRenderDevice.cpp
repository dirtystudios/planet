//#include "GLRenderDevice.h"
//
//#ifdef _WIN32
//    #include <GL/glew.h>
//#else
//    #include <OpenGL/gl3.h>
//    #include <OpenGL/gl3ext.h>
//#endif
//
//#include "Log.h"
//
//#define ARRAY_LEN(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
//#define SafeGet(x, idx) x[idx]
//
////inline GLenum SafeGet(GLenum* x, uint32_t idx) {
////    assert(idx < ARRAY_LEN(x));
////    return x[idx];
////};
//
// namespace graphics {
//    RenderDeviceGL::RenderDeviceGL() {
//        DeviceConfig.ShaderExtension = ".glsl";
//        DeviceConfig.DeviceAbbreviation = "GL";
//
//        glClearColor(0.1f, 0.1f, 0.1f, 1.f);
//        glClearDepth(1.0f);
//        glDepthFunc(GL_LESS);
//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_CULL_FACE);
//        glFrontFace(GL_CCW);
//        glCullFace(GL_BACK);
//
//        //glEnable(GL_BLEND);
//        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//
//        GL_CHECK(glGenProgramPipelines(1, &_pipeline));
//        GL_CHECK(glBindProgramPipeline(_pipeline));
//    }
//
//    IndexBufferHandle RenderDeviceGL::CreateIndexBuffer(void* data, size_t size, BufferUsage usage) {
//        uint32_t handle = GenerateHandle();
//
//        GLuint id = 0;
//        GL_CHECK(glGenBuffers(1, &id));
//
//        IndexBufferGL ib = {};
//        ib.id = id;
//
//        GLenum gl_usage = SafeGet(buffer_usage_mapping, (uint32_t)usage);
//
//        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id));
//        GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_usage));
//
//        _index_buffers.insert(std::make_pair(handle, ib));
//
//        return handle;
//    }
//
//    void RenderDeviceGL::DestroyIndexBuffer(IndexBufferHandle handle) {
//        auto it = _index_buffers.find(handle);
//        if(it == _index_buffers.end()) {
//            LOG_E("Could not find index buffer with handle %d", handle);
//            return;
//        }
//        IndexBufferGL &ib = (*it).second;
//
//        if(ib.id) {
//            GL_CHECK(glDeleteBuffers(1, &ib.id));
//        } else {
//            assert(false);
//        }
//        _index_buffers.erase(it);
//    }
//
//    VertexBufferHandle RenderDeviceGL::CreateVertexBuffer(const VertLayout &layout, void *data, size_t size,
//    BufferUsage usage) {
//        uint32_t handle = GenerateHandle();
//
//        GLuint id = 0;
//        GL_CHECK(glGenBuffers(1, &id));
//
//        VertexBufferGL vb = {};
//        vb.id = id;
//
//        GLenum gl_usage = SafeGet(buffer_usage_mapping, (uint32_t)usage);
//
//        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb.id));
//        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage));
//        vb.layout = layout;
//
//        _vertex_buffers.insert(std::make_pair(handle, vb));
//
//        return handle;
//    }
//
//    void RenderDeviceGL::DestroyVertexBuffer(VertexBufferHandle handle) {
//        auto it = _vertex_buffers.find(handle);
//        if(it == _vertex_buffers.end()) {
//            LOG_E("Could not find vertex buffer with handle %d", handle);
//            return;
//        }
//        VertexBufferGL &vb = (*it).second;
//
//        if(vb.id) {
//            GL_CHECK(glDeleteBuffers(1, &vb.id));
//        } else {
//            assert(false);
//        }
//        _vertex_buffers.erase(it);
//    }
//
//    ShaderHandle RenderDeviceGL::CreateShader(ShaderType shader_type, const std::string& source) {
//        GLenum gl_shader_type = SafeGet(shader_type_mapping, (uint32_t)shader_type);
//        const char* shaderSource = source.c_str();
//        GLuint id = glCreateShaderProgramv(gl_shader_type, 1, (const GLchar**)&shaderSource);
//        assert(id);
//        GLint linked = 0;
//        GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, &linked));
//        if(!linked) {
//            char log[1024];
//            GL_CHECK(glGetProgramInfoLog(id, 1024, nullptr, log));
//            LOG_E("Failed to link program: %s", log);
//            exit(1);
//        }
//
//        GLint attrib_count, max_len;
//        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attrib_count));
//        GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len));
//
//        char* name = new char[max_len];
//
//        AttribLayout layout;
//        for(int32_t idx = 0; idx < attrib_count; ++idx) {
//            GLint size;
//            GLenum type;
//
//            GL_CHECK(glGetActiveAttrib(id, idx, max_len, NULL, &size, &type, name));
//            LOG_D("Attribute [loc:%d, name:%s, type:%s, size:%d]", idx, name, glEnumName(type), size);
//
//            if(size == 1) {
//                if(type == GL_FLOAT_VEC2) {
//                    layout.Add(ParamType::Float2, name, idx, size);
//                } else if(type == GL_FLOAT_VEC3) {
//                    layout.Add(ParamType::Float3, name, idx, size);
//                } else if(type == GL_FLOAT_VEC4) {
//                    layout.Add(ParamType::Float4, name, idx, size);
//                } else {
//                    LOG_W("%s", "Attribute not registered");
//                }
//            } else {
//                LOG_W("%s", "Attribute not registered, size > 1");
//            }
//        }
//        delete [] name;
//
//
//        uint32_t handle = GenerateHandle();
//        ProgramGL shader = {};
//        shader.id = id;
//        shader.layout = layout;
//        _programs.insert(std::make_pair(handle, shader));
//        return handle;
//    }
//
//    void RenderDeviceGL::DestroyShader(ShaderHandle handle) {
//        auto it = _shaders.find(handle);
//        if(it == _shaders.end()) {
//            LOG_E("Could not find shader with handle %d",handle);
//            return;
//        }
//        ShaderGL &shader = (*it).second;
//
//        if(shader.id) {
//            GL_CHECK(glDeleteShader(shader.id));
//        } else {
//            assert(false);
//        }
//
//        _shaders.erase(it);
//    }
//
////     ProgramHandle RenderDeviceGL::CreateProgram(ShaderHandle* shader_handles, uint32_t num_shaders) {
////         bool vs = false, tcs = false, tes = false, fs = false;
////         for(uint32_t idx = 0; idx < num_shaders; ++idx) {
////             ShaderGL *shader = GetShader(shader_handles[idx]);
////             assert(shader);
////             switch(shader->type) {
////                 case GL_VERTEX_SHADER: {
////                     assert(!vs);
////                     vs = true;
////                     break;
////                 }
////                 case GL_TESS_CONTROL_SHADER: {
////                     assert(!tcs);
////                     tcs = true;
////                     break;
////                 }
////                 case GL_TESS_EVALUATION_SHADER: {
////                     assert(!tes);
////                     tes = true;
////                     break;
////                 }
////                 case GL_FRAGMENT_SHADER: {
////                     assert(!fs);
////                     fs = true;
////                     break;
////                 }
////             }
////         }
////
////         GLuint id = glCreateProgram();
////         for(uint32_t idx = 0; idx < num_shaders; ++idx) {
////             ShaderGL *shader = GetShader(shader_handles[idx]);
////             GL_CHECK(glAttachShader(id, shader->id));
////         }
////         GL_CHECK(glLinkProgram(id));
////
////         GLint linked = 0;
////         GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, &linked));
////         if(!linked) {
////             char log[1024];
////             GL_CHECK(glGetProgramInfoLog(id, 1024, nullptr, log));
////             SLOG_FATAL("gfx", "Failed to link program: " << log);
////             exit(1);
////         }
////
////         ProgramGL program;
////         program.id = id;
////
////         GLint max_len;
////         GLint attrib_count = 0;
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &attrib_count));
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len));
////         SLOG_D("gfx", "numattribs:" << attrib_count);
////         char* name = new char[max_len];
////         for(int32_t idx = 0; idx < attrib_count; ++idx) {
////             GLint size;
////             GLenum type;
////             GL_CHECK(glGetActiveUniform(id, idx, max_len, nullptr, &size, &type, name));
//// //            GL_CHECK(glGetActiveUniformsiv(id, idx, max_len, GL_UNIFORM_BLOCK_INDEX, &size, &type, name));
////             SLOG_D("gfx", "Program uniform: " << name);
////         }
////         delete [] name;
////
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &attrib_count));
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len));
////
////         name = new char[max_len];
////
////         AttribLayout &layout = program.layout;
////         for(int32_t idx = 0; idx < attrib_count; ++idx) {
////             GLint size;
////             GLenum type;
////
////             GL_CHECK(glGetActiveAttrib(id, idx, max_len, NULL, &size, &type, name));
////             SLOG_D("gfx", "Attribute [loc:" << idx << ", name:" << name << ", type:" << glEnumName(type) <<",
///size:" << size << "]");
////
////             if(size == 1) {
////                 if(type == GL_FLOAT_VEC2) {
////                     layout.Add(ParamType::Float2, name, idx, size);
////                 } else if(type == GL_FLOAT_VEC3) {
////                     layout.Add(ParamType::Float3, name, idx, size);
////                 } else if(type == GL_FLOAT_VEC4) {
////                     layout.Add(ParamType::Float4, name, idx, size);
////                 } else {
////                     SLOG_W("gfx", "Attribute not registered");
////                 }
////             } else {
////                 SLOG_W("gfx", "Attribute not registered, size > 1");
////             }
////         }
////         delete [] name;
////
////         GLint uniform_blocks_count = 0;
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_blocks_count));
////         GL_CHECK(glGetProgramiv(id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &max_len));
////         name = new char[max_len];
////
////         for(int32_t idx = 0; idx < attrib_count; ++idx) {
////             GLint len;
////             glGetActiveUniformBlockName(id, idx, max_len, &len, name);
////             SLOG_D("gfx", "Program Uniform Block Name:" << name);
////
////             GLint ubo_size;
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size);
////
////             GLint block_uniforms_count;
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &block_uniforms_count);
////
////             GLint* uniform_indices = new GLint[block_uniforms_count];
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniform_indices);
////             // get parameters of all uniform variables in uniform block
////             for(uint32_t jdx = 0; jdx < block_uniforms_count; ++jdx) {
////                   if(uniform_indices[jdx] > 0) {
////
////                      // index of uniform variable
////                      GLuint uniform_index = uniform_indices[jdx];
////
////                      GLint uniform_name_len, uniform_offset, uniform_size;
////                      GLint uniform_type, array_stride, matrix_stride;
////
////                      // get length of name of uniform variable
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                  GL_UNIFORM_NAME_LENGTH, &uniform_name_len);
////                      // get name of uniform variable
////                      GLchar* uniform_name = new GLchar[uniform_name_len];
////                      glGetActiveUniform(id, uniform_index, uniform_name_len,
////                                     nullptr, nullptr, nullptr, uniform_name);
////
////                      // get offset of uniform variable related to start of uniform block
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                     GL_UNIFORM_OFFSET, &uniform_offset);
////                      // get size of uniform variable (number of elements)
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                     GL_UNIFORM_SIZE, &uniform_size);
////                      // get type of uniform variable (size depends on this value)
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                     GL_UNIFORM_TYPE, &uniform_type);
////                      // offset between two elements of the array
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                     GL_UNIFORM_ARRAY_STRIDE, &array_stride);
////                      // offset between two vectors in matrix
////                      glGetActiveUniformsiv(id, 1, &uniform_index,
////                                     GL_UNIFORM_MATRIX_STRIDE, &matrix_stride);
////
////                      // Size of uniform variable in bytes
////                      //GLuint sizeInBytes = uniform_size * SizeOfUniform(uniform_size)
////
////                      // output data
////                      SLOG_D("gfx", "name:" << uniform_name);
////                      SLOG_D("gfx", "size:" << uniform_size);
////                      SLOG_D("gfx", "offset:" << uniform_offset);
////                      SLOG_D("gfx", "type:" << glEnumName(uniform_type));
////                      SLOG_D("gfx", "array_stride:" << array_stride);
////                      SLOG_D("gfx", "matrix_stride:" << matrix_stride);
////                      delete [] uniform_name;
////                 }
////             }
////
////             GLint in_vs, in_tcs, in_tes, in_gs, in_fs;
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER, &in_vs);
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER, &in_tcs);
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER, &in_tes);
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER, &in_gs);
////             glGetActiveUniformBlockiv(id, idx, GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER, &in_fs);
////
////             SLOG_D("gfx", "GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER Binding:" << in_vs);
////             SLOG_D("gfx", "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER Binding:" << in_tcs);
////             SLOG_D("gfx", "GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER:" << in_tes);
////             SLOG_D("gfx", "GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER:" << in_gs);
////             SLOG_D("gfx", "GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:" << in_fs);
////             delete [] uniform_indices;
////
////         }
////         delete [] name;
////
////         uint32_t handle = GenerateHandle();
////         _programs.insert(std::make_pair(handle, program));
////
////         SLOG_D("gfx", "Created Program " << handle);
////
////         return handle;
////     }
//
//
//    TextureHandle RenderDeviceGL::CreateTexture2D(TextureFormat tex_format, uint32_t width, uint32_t height, void*
//    data) {
//        GLTextureFormatDesc* gl_texture_desc = &SafeGet(texture_format_mapping, (uint32_t)tex_format);
//
//        GLuint id = 0;
//        GL_CHECK(glGenTextures(1, &id));
//        GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
//        if(tex_format == TextureFormat::R_UBYTE) {
//            GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
//        }
//        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, gl_texture_desc->internal_format, width, height, 0,
//        gl_texture_desc->data_format, gl_texture_desc->data_type, data));
//        if(tex_format == TextureFormat::R_UBYTE) {
//            GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
//        }
//        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//
//        uint32_t handle = GenerateHandle();
//        TextureGL texture = {};
//        texture.id = id;
//        texture.target = GL_TEXTURE_2D;
//        texture.texture_desc = gl_texture_desc;
//        _textures.insert(std::make_pair(handle, texture));
//        return handle;
//    }
//
//    TextureHandle RenderDeviceGL::CreateTextureArray(TextureFormat tex_format, uint32_t levels, uint32_t width,
//    uint32_t height, uint32_t depth) {
//        GLTextureFormatDesc* gl_texture_desc = &SafeGet(texture_format_mapping, (uint32_t)tex_format);
//
//        GLuint id = 0;
//        GL_CHECK(glGenTextures(1, &id));
//        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, id));
//        GL_CHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, gl_texture_desc->internal_format, width, height, depth));
//        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
//        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//        uint32_t handle = GenerateHandle();
//        TextureGL texture = {};
//        texture.id = id;
//        texture.target = GL_TEXTURE_2D_ARRAY;
//        texture.texture_desc = gl_texture_desc;
//        _textures.insert(std::make_pair(handle, texture));
//
//        return handle;
//    }
//
//    TextureHandle RenderDeviceGL::CreateTextureCube(TextureFormat tex_format, uint32_t width, uint32_t height, void**
//    data) {
//        GLTextureFormatDesc* gl_texture_desc = &SafeGet(texture_format_mapping, (uint32_t)tex_format);
//        assert(width == height);
//        GLuint id = 0;
//        GL_CHECK(glGenTextures(1, &id));
//        GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, id));
//        for(uint32_t side = 0; side < 6; ++side) {
//            GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, gl_texture_desc->internal_format, width,
//            height, 0,
//                                  gl_texture_desc->data_format, gl_texture_desc->data_type, data[side]));
//        }
//
//        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//        GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//
//        uint32_t handle = GenerateHandle();
//        TextureGL texture = {};
//        texture.id = id;
//        texture.target = GL_TEXTURE_CUBE_MAP;
//        texture.texture_desc = gl_texture_desc;
//        _textures.insert(std::make_pair(handle, texture));
//        return handle;
//    }
//
//    void RenderDeviceGL::DestroyTexture(TextureHandle handle) {
//        auto it = _textures.find(handle);
//        if(it == _textures.end()) {
//            LOG_E("Could not find texture with handle %d", handle);
//            return;
//        }
//        TextureGL &texture = (*it).second;
//
//        if(texture.id) {
//            GL_CHECK(glDeleteTextures(1, &texture.id));
//        } else {
//            assert(false);
//        }
//
//        _textures.erase(it);
//    }
//
//
//    void RenderDeviceGL::UpdateTextureArray(TextureHandle handle, uint32_t array_index, uint32_t width, uint32_t
//    height, void* data) {
//        TextureGL* texture = Get<TextureGL>(_textures, handle);
//        assert(texture);
//        assert(texture->target == GL_TEXTURE_2D_ARRAY);
//
//        GLTextureFormatDesc* format_desc = texture->texture_desc;
//
//        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, texture->id));
//        uint32_t xoffset = 0, yoffset = 0, level = 0, depth = 1;
//        GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xoffset, yoffset, array_index, width, height, depth,
//                                 format_desc->data_format, format_desc->data_type, data));
//    }
//    void RenderDeviceGL::UpdateTexture(TextureHandle handle, void* data, size_t size) {
//        assert(false);
//    }
//
//    void RenderDeviceGL::UpdateVertexBuffer(VertexBufferHandle handle, void* data, size_t size) {
//        VertexBufferGL* vertex_buffer = Get<VertexBufferGL>(_vertex_buffers, handle);
//        assert(vertex_buffer);
//
//        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->id));
//        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
//
//    }
//    void RenderDeviceGL::Clear(float r, float g, float b, float a) {
//        glClearColor(r, g, b, a);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    }
//
//    void RenderDeviceGL::SetVertexShader(ShaderHandle shader_handle) {
//        _pending_state.vertex_shader_handle = shader_handle;
//    }
//    void RenderDeviceGL::SetPixelShader(ShaderHandle shader_handle) {
//        _pending_state.pixel_shader_handle = shader_handle;
//    }
//    void RenderDeviceGL::SetShaderParameter(ShaderHandle handle, ParamType param_type, const char *param_name, void
//    *data) {
//        _pending_state.shader_parameters.Set(handle, param_type, param_name, data);
//    }
//    void RenderDeviceGL::SetShaderTexture(ShaderHandle shader_handle, TextureHandle texture_handle, TextureSlot slot)
//    {
//        _pending_state.shader_textures.Set(shader_handle, texture_handle, slot);
//    }
//    void RenderDeviceGL::SetVertexBuffer(VertexBufferHandle handle) {
//        _pending_state.vertex_buffer_handle = handle;
//    }
//    void RenderDeviceGL::SetBlendState(const BlendState& blend_state) {
//        memcpy(&_pending_state.blend_state, &blend_state, sizeof(BlendState));
//    }
//    void RenderDeviceGL::SetDepthState(const DepthState& depth_state) {
//        memcpy(&_pending_state.depth_state, &depth_state, sizeof(DepthState));
//    }
//    void RenderDeviceGL::SetRasterState(const RasterState& raster_state) {
//        memcpy(&_pending_state.raster_state, &raster_state, sizeof(RasterState));
//    }
//
//
//    // TODO: BIG TODO - need to cache currently bound state to avoid re-setting it
//    void RenderDeviceGL::DrawPrimitive(PrimitiveType primitive_type, uint32_t start_vertex, uint32_t num_vertices) {
//        //ResolveContextState(_current_state, _pending_state);
//
//        // BIG TODO: Renderstate does not persist -- this is bad
//        if(_pending_state.raster_state.cull_mode == CullMode::NONE) {
//            GL_CHECK(glDisable(GL_CULL_FACE));
//        } else {
//            GL_CHECK(glEnable(GL_CULL_FACE));
//            GLenum cull_mode = SafeGet(cull_mode_mapping, (uint32_t)_pending_state.raster_state.cull_mode);
//            GL_CHECK(glCullFace(cull_mode));
//        }
//
//        GLenum winding_order = SafeGet(winding_order_mapping, (uint32_t)_pending_state.raster_state.winding_order);
//        GL_CHECK(glFrontFace(winding_order));
//
//
//        if(!_pending_state.blend_state.enable) {
//            GL_CHECK(glDisable(GL_BLEND));
//        } else {
//            GLenum rgb_mode = SafeGet(blend_mode_mapping, (uint32_t)_pending_state.blend_state.rgb_mode);
//            GLenum alpha_mode = SafeGet(blend_mode_mapping, (uint32_t)_pending_state.blend_state.alpha_mode);
//            GLenum src_rgb_func = SafeGet(blend_func_mapping, (uint32_t)_pending_state.blend_state.src_rgb_func);
//            GLenum src_alpha_func = SafeGet(blend_func_mapping, (uint32_t)_pending_state.blend_state.src_alpha_func);
//            GLenum dst_rgb_func = SafeGet(blend_func_mapping, (uint32_t)_pending_state.blend_state.dst_rgb_func);
//            GLenum dst_alpha_func = SafeGet(blend_func_mapping, (uint32_t)_pending_state.blend_state.dst_alpha_func);
//            GL_CHECK(glEnable(GL_BLEND));
//            GL_CHECK(glBlendEquationSeparate(rgb_mode, alpha_mode));
//            GL_CHECK(glBlendFuncSeparate(src_rgb_func, dst_rgb_func, src_alpha_func, dst_alpha_func));
//        }
//
//        if(!_pending_state.depth_state.enable) {
//            GL_CHECK(glDisable(GL_DEPTH_TEST));
//        } else {
//            GL_CHECK(glEnable(GL_DEPTH_TEST));
//            GLenum depth_write_mask = SafeGet(depth_write_mask_mapping,
//            (uint32_t)_pending_state.depth_state.depth_write_mask);
//            GL_CHECK(glDepthMask(depth_write_mask));
//
//            GLenum depth_func = SafeGet(depth_func_mapping, (uint32_t)_pending_state.depth_state.depth_func);
//            GL_CHECK(glDepthFunc(depth_func));
//        }
//
//        //if(_current_state.pixel_shader_handle != _pending_state.pixel_shader_handle) {
//            ProgramGL* ps = Get<ProgramGL>(_programs, _pending_state.pixel_shader_handle);
//            assert(ps);
//            GL_CHECK(glUseProgramStages(_pipeline, GL_FRAGMENT_SHADER_BIT, ps->id));
//        //}
//
//        //if(_current_state.vertex_shader_handle != _pending_state.vertex_shader_handle) {
//            ProgramGL* vs = Get<ProgramGL>(_programs, _pending_state.vertex_shader_handle);
//            assert(vs);
//            GL_CHECK(glUseProgramStages(_pipeline, GL_VERTEX_SHADER_BIT, vs->id));
//        //}
//
//        for(ShaderParameter* parameter : _pending_state.shader_parameters.parameters) {
//            ProgramGL* program = Get<ProgramGL>(_programs, parameter->handle);
//            assert(program);
//            const char *param_name = parameter->param_name;
//            ParamType  param_type = parameter->param_type;
//            void *data = parameter->param_data;
//
//            GLint location = program->GetLocation(param_name);
//            //GL_CHECK();
//            if(location < 0) {
//                LOG_E("Failed to find uniform by name '%s'", param_name);
//                continue;
//            }
//
//            switch(param_type) {
//                case ParamType::Float: {
//                    GL_CHECK(glProgramUniform1f(program->id, location, (float)*((float*)data)));
//                    break;
//                }
//                case ParamType::Int32: {
//                    GL_CHECK(glProgramUniform1i(program->id, location, (int)*((int*)data)));
//                    break;
//                }
//                case ParamType::Float4x4: {
//                    GL_CHECK(glProgramUniformMatrix4fv(program->id, location, 1, GL_FALSE, (float*)data));
//                    break;
//                }
//                case ParamType::Float3: {
//                    GL_CHECK(glProgramUniform3fv(program->id, location, 1, (float*)data));
//                    break;
//                }
//                case ParamType::Float2: {
//                    GL_CHECK(glProgramUniform2fv(program->id, location, 1, (float*)data));
//                    break;
//                }
//                case ParamType::Float4: {
//                    GL_CHECK(glProgramUniform4fv(program->id, location, 1, (float*)data));
//                    break;
//                }
//                default: {
//                    LOG_E("Unsupported uniform type %d", param_type);
//                }
//            }
//        }
//
//        for(ShaderTexture shader_texture : _pending_state.shader_textures.textures) {
//            ProgramGL* program = Get<ProgramGL>(_programs, shader_texture.shader_handle);
//            TextureGL* texture = Get<TextureGL>(_textures, shader_texture.texture_handle);
//            assert(program && texture);
//
//            GLint location = -1;
//            GLint slot = -1;
//            switch(shader_texture.slot) {
//                case TextureSlot::BASE:
//                    location = program->GetLocation("base_texture");
//                    slot = 0;
//                break;
//                case TextureSlot::NORMAL:
//                    location = program->GetLocation("normal_texture");
//                    slot = 1;
//                break;
//                default:
//                break;
//            }
//            assert(location >= 0 && slot >= 0);
//            GL_CHECK(glActiveTexture(GL_TEXTURE0 + slot));
//            GL_CHECK(glBindTexture(texture->target, texture->id));
//            GL_CHECK(glProgramUniform1i(program->id, location, slot));
//        }
//
//
//
//
//          VertexBufferGL* vb = Get<VertexBufferGL>(_vertex_buffers, _pending_state.vertex_buffer_handle);
//          assert(vb);
//
//        VertexArrayObjectGL *vao = Get(_vao_cache, _pending_state.vertex_buffer_handle);
//        if(!vao) {
//            GLuint id = 0;
//            GL_CHECK(glGenVertexArrays(1, &id));
//            GL_CHECK(glBindVertexArray(id));
//            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vb->id));
//            if(!BindAttributes(vb->layout, vs->layout)) {
//                // TODO: need to roll back vao on failure because it has already been cached and created
//                LOG_E("gfx", "Vertex layout does not match program inputs");
//                return;
//            }
//
//            VertexArrayObjectGL vertex_array = {};
//            vertex_array.id = id;
//            _vao_cache.insert(std::make_pair(_pending_state.vertex_buffer_handle, vertex_array));
//            GL_CHECK(glBindVertexArray(id));
//        } else {
//            GL_CHECK(glBindVertexArray(vao->id));
//        }
//
//        // eventually we will keep a current state
//        _current_state = _pending_state;
//
//        GLenum gl_primitive_type = SafeGet(primitive_type_mapping, (uint32_t)primitive_type);
//
//        GL_CHECK(glDrawArrays(gl_primitive_type, start_vertex, num_vertices));
//
//        _pending_state.shader_textures.Clear();
//        _pending_state.shader_parameters.Clear();
//    }
//
//    bool RenderDeviceGL::BindAttributes(const VertLayout &vert_layout, const AttribLayout &attrib_layout) {
//        if(!Matches(attrib_layout, vert_layout)) {
//            return false;
//        }
//
//        size_t offset = 0;
//        for(uint32_t idx = 0; idx < attrib_layout.elements.size(); ++idx) {
//            const AttribElement* attrib_elem = &attrib_layout.elements[idx];
//            const VertElement* vert_elem = &vert_layout.elements[idx];
//
//
//
//            GLenum type = GL_FALSE;
//            GLuint size = 0;
//            switch(attrib_elem->type) {
//                case ParamType::Float:
//                    type = GL_FLOAT;
//                    size = 1;
//                    break;
//                case ParamType::Float2:
//                    type = GL_FLOAT;
//                    size = 2;
//                    break;
//                case ParamType::Float3:
//                    type = GL_FLOAT;
//                    size = 3;
//                    break;
//                case ParamType::Float4:
//                    type = GL_FLOAT;
//                    size = 4;
//                    break;
//                default:
//                    LOG_E("%s", "Unknown vertex element type");
//                    return false;
//            }
//
//            LOG_D("Binding attribute (size:%d, type:%d, stride:%d, offset:%d) to location:%d", size, type,
//            vert_layout.stride, offset, attrib_elem->location);
//            GL_CHECK(glEnableVertexAttribArray(attrib_elem->location));
//            GL_CHECK(glVertexAttribPointer(attrib_elem->location, size, type, GL_FALSE, vert_layout.stride, (const
//            void*)offset));
//            offset += SizeofParam(vert_elem->type);
//        }
//        return true;
//    }
//
//    ShaderGL* RenderDeviceGL::GetShader(ShaderHandle handle) {
//        return Get(_shaders, handle);
//    }
//
//    uint32_t RenderDeviceGL::GenerateHandle() {
//        static uint32_t key = 0;
//        return ++key;
//    }
//
//    void RenderDeviceGL::PrintDisplayAdapterInfo() {
//        LOG_D("GL_VERSION: %s", glGetString(GL_VERSION));
//        LOG_D("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
//        LOG_D("GL_VENDOR: %s", glGetString(GL_VENDOR));
//        LOG_D("GL_RENDERER: %s", glGetString(GL_RENDERER));
//    }
//}
