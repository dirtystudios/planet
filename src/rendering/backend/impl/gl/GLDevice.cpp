#include "GLDevice.h"
#include <cassert>
#include <vector>
#include "GLEnumAdapter.h"
#include <algorithm>
#ifdef _WIN32
#else
#include <OpenGL/gl3ext.h>
#endif
#include "Log.h"
#include "ShaderStage.h"

namespace graphics {

GLDevice::GLDevice() {
    this->DeviceConfig.ShaderExtension    = ".glsl";
    this->DeviceConfig.DeviceAbbreviation = "GL";
    this->_currentFrame                   = new Frame();
}

void GLDevice::ResizeWindow(uint32_t width, uint32_t height) {}

void GLDevice::PrintDisplayAdapterInfo() {
    printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    printf("GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
}

BufferId GLDevice::CreateBuffer(BufferType type, void* data, size_t size, BufferUsage usage) {
    GLBuffer* buffer = new GLBuffer();
    GL_CHECK(glGenBuffers(1, &buffer->id));
    assert(buffer->id);
    buffer->usage = GLEnumAdapter::Convert(usage);
    buffer->type  = GLEnumAdapter::Convert(type);

    if (data) {
        _context.WriteBufferData(buffer, data, size);
    }

    uint32_t handle = GenerateId();
    _buffers.insert(std::make_pair(handle, buffer));

    return handle;
}

ShaderId GLDevice::CreateShader(ShaderType shaderType, const std::string& source) {
    GLShaderProgram* shader = new GLShaderProgram();

    shader->type               = GLEnumAdapter::Convert(shaderType);
    const char* shaderSource[] = {source.c_str()};
    shader->id = glCreateShaderProgramv(shader->type, 1, (const GLchar**)shaderSource);
    assert(shader->id);

    GLint linked = 0;
    GL_CHECK(glGetProgramiv(shader->id, GL_LINK_STATUS, &linked));
    if (!linked) {
        char log[1024];
        GL_CHECK(glGetProgramInfoLog(shader->id, 1024, nullptr, log));
        LOG_E("Failed to compile/link program: %s\n--------------:\n%s\n", log, source.c_str());
        assert(false);
    }

    GLShaderMetadata* metadata = &shader->metadata;

    // extract attribute metadata
    {
        GLint maxLen = 0, count = 0;
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_ATTRIBUTES, &count));
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen));
        metadata->attributes.resize(count);

        char* name = new char[maxLen];
        for (int32_t idx = 0; idx < count; ++idx) {

            GLAttributeMetadata* attribute = &metadata->attributes[idx];

            GL_CHECK(glGetActiveAttrib(shader->id, idx, maxLen, NULL, &attribute->size, &attribute->type, name));

            attribute->name     = std::string(name);
            attribute->location = glGetAttribLocation(shader->id, attribute->name.c_str());
            GL_CHECK("");
            assert(attribute->location >= 0);
        }

        std::sort(
            begin(metadata->attributes), end(metadata->attributes),
            [](const GLAttributeMetadata& v1, const GLAttributeMetadata& v2) { return v1.location < v2.location; });
        delete[] name;
    }

    // extract uniform metadata
    {
        GLint maxLen = 0, count = 0;
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_UNIFORMS, &count));
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen));
        metadata->uniforms.resize(count);
        char* name = new char[maxLen];

        for (int32_t idx = 0; idx < count; ++idx) {
            GLUniformMetadata* uniform = &metadata->uniforms[idx];
            GL_CHECK(glGetActiveUniform(shader->id, idx, maxLen, nullptr, &uniform->size, &uniform->type, name));

            uniform->name     = std::string(name);
            uniform->location = glGetUniformLocation(shader->id, uniform->name.c_str());
            GL_CHECK("");
            assert(uniform->location >= 0);
        }
        delete[] name;
    }

    // extract uniform block metadatas
    {
        GLint maxLen = 0, count = 0;
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_UNIFORM_BLOCKS, &count));
        GL_CHECK(glGetProgramiv(shader->id, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxLen));
        metadata->blocks.resize(count);

        char* name = new char[maxLen];
        for (uint32_t idx = 0; idx < count; ++idx) {
            GLUniformBlockMetadata* block = &metadata->blocks[idx];
            GLint len                     = 0;
            GLint uniformCount            = 0;

            glGetActiveUniformBlockName(shader->id, idx, maxLen, &len, name);
            glGetActiveUniformBlockiv(shader->id, idx, GL_UNIFORM_BLOCK_DATA_SIZE, &block->size);
            glGetActiveUniformBlockiv(shader->id, idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniformCount);

            block->name = std::string(name);
            block->uniforms.resize(uniformCount);

            GLint* uniformIndices = new GLint[uniformCount];
            glGetActiveUniformBlockiv(shader->id, idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices);

            // get parameters of all uniform variables in uniform block
            for (uint32_t jdx = 0; jdx < uniformCount; ++jdx) {
                assert(uniformIndices[jdx] > 0);

                GLint uniformName_len           = 0;
                GLBlockUniformMetadata* uniform = &block->uniforms[jdx];
                uniform->index                  = uniformIndices[jdx];

                // get length of name of uniform variable
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_NAME_LENGTH, &uniformName_len);

                // get name of uniform variable
                char* uniformName = new char[uniformName_len];
                glGetActiveUniform(shader->id, uniform->index, uniformName_len, nullptr, nullptr, nullptr, uniformName);
                uniform->name = std::string(uniformName);
                delete[] uniformName;

                // get offset of uniform variable related to start of uniform
                // block
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_OFFSET, &uniform->offset);
                // get size of uniform variable (number of elements)
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_SIZE, &uniform->size);
                // get type of uniform variable (size depends on this value)
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_TYPE, &uniform->type);
                // offset between two elements of the array
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_ARRAY_STRIDE, &uniform->arrayStride);
                // offset between two vectors in matrix
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_MATRIX_STRIDE, &uniform->matrixStride);
            }
            delete[] uniformIndices;
        }
        delete[] name;
    }

    uint32_t handle = GenerateId();
    _shaders.insert(std::make_pair(handle, shader));

    printf("Shader:\n");
    printf("\tBlocks(%d):\n", shader->metadata.blocks.size());
    for (const GLUniformBlockMetadata& block : shader->metadata.blocks) {
        printf("\t\tName:%s\n", block.name.c_str());
        printf("\t\tSize:%d\n", block.size);
        printf("\t\tBlockUniforms(%d):\n", block.uniforms.size());
        for (const GLBlockUniformMetadata& uniform : block.uniforms) {
            printf("\t\t\tName:%s\n", uniform.name.c_str());
            printf("\t\t\tIdx:%d\n", uniform.index);
            printf("\t\t\tSize:%d\n", uniform.size);
            printf("\t\t\tType:%s\n", GLEnumAdapter::Convert(static_cast<GLenum>(uniform.type)).c_str());
            printf("\t\t\tOffset:%d\n", uniform.offset);
            printf("\t\t\tArrayStride:%d\n", uniform.arrayStride);
            printf("\t\t\tMatrixStride:%d\n", uniform.matrixStride);
            printf("\t\t\t------ \n");
        }
    }
    printf("\tAttribs(%d):\n", shader->metadata.attributes.size());
    for (const GLAttributeMetadata& attrib : shader->metadata.attributes) {
        printf("\t\tName:%s\n", attrib.name.c_str());
        printf("\t\tType:%s (%d)\n", GLEnumAdapter::Convert(static_cast<GLenum>(attrib.type)).c_str(), attrib.type);
        printf("\t\tLoc:%d\n", attrib.location);
        printf("\t\tSize:%d\n", attrib.size);
        printf("\t\t------ \n");
    }
    printf("\tUniforms(%d):\n", shader->metadata.uniforms.size());
    for (const GLUniformMetadata& uniform : shader->metadata.uniforms) {
        printf("\t\tName:%s\n", uniform.name.c_str());
        printf("\t\tType:%s (%d)\n", GLEnumAdapter::Convert(static_cast<GLenum>(uniform.type)).c_str(),
        uniform.type);
        printf("\t\tLoc:%d\n", uniform.location);
        printf("\t\tSize:%d\n", uniform.size);
        printf("\t\t------ \n");
    }

    return handle;
}

ShaderParamId GLDevice::CreateShaderParam(ShaderId shaderHandle, const char* paramName, ParamType paramType) {
    GLShaderProgram* shader = GetResource<GLShaderProgram>(_shaders, shaderHandle);
    assert(shader);

    const std::vector<GLUniformMetadata>& uniforms = shader->metadata.uniforms;
    GLenum glParamType                             = GLEnumAdapter::Convert(paramType);
    auto pred                                      = [&](const GLUniformMetadata& a) -> bool { return a.name == paramName && a.type == glParamType; };

    auto it = std::find_if(uniforms.begin(), uniforms.end(), pred);
    if (it == uniforms.end()) {
        LOG_W("Failed to create shaderParam (shader:%d, param:%s)", shaderHandle, paramName);
        return 0;
    }

    GLShaderParameter* shaderParam = new GLShaderParameter();
    shaderParam->program           = shader;
    shaderParam->metadata          = (*it);

    uint32_t handle = GenerateId();
    _shaderParams.insert(std::make_pair(handle, shaderParam));

    return handle;
}

bool isVertexLayoutValidWithShader(const GLVertexLayout& layout, const GLShaderProgram& shader) {
    assert(shader.type == GL_VERTEX_SHADER);
    assert(layout.elements.size() == shader.metadata.attributes.size());

    const std::vector<GLAttributeMetadata>& attributes = shader.metadata.attributes; // should be sorted
    for (uint32_t idx = 0; idx < layout.elements.size(); ++idx) {
        const GLVertexElement& element       = layout.elements[idx];
        const GLAttributeMetadata& attribute = attributes[idx];
        // Attributes use different enumerations to define vars since attribuets can be for any type of shader.
        // Vertex shader's have limited types of attributes that can be.        
        const GLVertexElement& attributeAsElement = GLEnumAdapter::Convert(attribute);

        LOG_D("idx(%d): %s -> %s", idx, element.ToString().c_str(), attributeAsElement.ToString().c_str());

        if (attribute.location != idx) {
            LOG_W("Attribute location mismatch with VertexLayout");
            return false;
        }
        if (element.type != attributeAsElement.type) {
            LOG_W("Attribute type mismatch with VertexElement type");
            return false;
        }
        if (element.count != attributeAsElement.count) {
            LOG_W("Attribute count mismatch with VertexElement count");
            return false;
        }
    }

    return true;
}

PipelineStateId GLDevice::CreatePipelineState(const PipelineStateDesc& desc) {
    GLPipelineState* pipelineState = new GLPipelineState();

    pipelineState->vertexShader = GetResource<GLShaderProgram>(_shaders, desc.vertexShader);
    if (pipelineState->vertexShader) {
        pipelineState->vertexShader->members.push_back(pipelineState);
    }

    assert(pipelineState->vertexShader);

    pipelineState->pixelShader = GetResource<GLShaderProgram>(_shaders, desc.pixelShader);
    if (pipelineState->pixelShader) {
        pipelineState->pixelShader->members.push_back(pipelineState);
    }

    assert(pipelineState->pixelShader);

    // TODO: Smart person would cache
    pipelineState->depthState  = GLEnumAdapter::Convert(desc.depthState);
    pipelineState->blendState  = GLEnumAdapter::Convert(desc.blendState);
    pipelineState->rasterState = GLEnumAdapter::Convert(desc.rasterState);

    pipelineState->vertexLayout = GetResource<GLVertexLayout>(_vertexLayouts, desc.vertexLayout);
    assert(pipelineState->vertexLayout);

    // TODO:: Do this first
    assert(isVertexLayoutValidWithShader(*pipelineState->vertexLayout, *pipelineState->vertexShader));    

    pipelineState->topology = GLEnumAdapter::Convert(desc.topology);

    uint32_t handle = GenerateId();
    _pipelineStates.insert(std::make_pair(handle, pipelineState));

    return handle;
}

TextureId GLDevice::CreateTexture2D(TextureFormat texFormat, uint32_t width, uint32_t height, void* data) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type      = GL_TEXTURE_2D;

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(TextureSlot::Base, texture);
    if (texFormat == TextureFormat::R_U8) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    }
    // TODO: Move to GLContext
    GL_CHECK(glTexImage2D(texture->type, 0, texture->format.internalFormat, width, height, 0,
                          texture->format.dataFormat, texture->format.dataType, data));
    if (texFormat == TextureFormat::R_U8) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }

    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    uint32_t handle = GenerateId();
    _textures.insert(std::make_pair(handle, texture));
    return handle;
}

TextureId GLDevice::CreateTextureArray(TextureFormat texFormat, uint32_t levels, uint32_t width, uint32_t height,
                                       uint32_t depth) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type      = GL_TEXTURE_2D_ARRAY;

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(TextureSlot::Base, texture);
    // TODO: Move to GLContext
    GL_CHECK(glTexStorage3D(texture->type, levels, texture->format.internalFormat, width, height, depth));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    uint32_t handle = GenerateId();
    _textures.insert(std::make_pair(handle, texture));

    return handle;
}

TextureId GLDevice::CreateTextureCube(TextureFormat texFormat, uint32_t width, uint32_t height, void** data) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type = GL_TEXTURE_CUBE_MAP;
    assert(width == height);

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(TextureSlot::Base, texture);

    // TODO: Move to GLContext
    for (uint32_t side = 0; side < 6; ++side) {
        GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, texture->format.internalFormat, width, height,
                              0, texture->format.dataFormat, texture->format.dataType, data[side]));
    }

    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    uint32_t handle = GenerateId();
    _textures.insert(std::make_pair(handle, texture));
    return handle;
}

VertexLayoutId GLDevice::CreateVertexLayout(const VertexLayoutDesc& desc) {
    assert(desc.elements.size() > 0);

    // size_t key = std::hash<VertexLayoutDesc>()(desc);
    // GLVertexLayout* vertexLayout = GetOrCreateVertexLayout(desc):

    GLVertexLayout* vertexLayout = new GLVertexLayout();
    size_t stride = 0;
    for (const VertexLayoutElement& element : desc.elements) {
        GLVertexElement glElement = GLEnumAdapter::Convert(element);
        vertexLayout->stride += GetByteCount(element.type);
        vertexLayout->elements.push_back(glElement);
    }

    uint32_t handle = GenerateId();
    _vertexLayouts.insert(std::make_pair(handle, vertexLayout));
    return handle;
}

void GLDevice::Execute(const std::vector<BufferUpdate*>& tasks) {
    for (BufferUpdate* task : tasks) {
        assert(task);

        GLBuffer* buffer = GetResource(_buffers, task->bufferId);
        assert(buffer);
        _context.WriteBufferData(buffer, task->data, task->len);
    }
}

void GLDevice::Execute(const std::vector<TextureUpdate*>& tasks) {
    for (TextureUpdate* task : tasks) {
        assert(task);

        GLTexture* texture = GetResource(_textures, task->textureId);
        assert(texture);
        //        _context.WriteTextureData(texture, task->data, task->len);
    }
}

void GLDevice::Execute(const std::vector<ShaderParamUpdate*>& tasks) {
    for (ShaderParamUpdate* task : tasks) {
        assert(task);

        LOG_D("%s", task->ToString().c_str());

        GLShaderParameter* param = GetResource(_shaderParams, task->paramId);
        assert(param);
        _context.WriteShaderParamater(param, task->data, task->len);
    }
}

void GLDevice::Execute(const std::vector<TextureBind*>& tasks) {
    for (TextureBind* task : tasks) {
        assert(task);

        GLTexture* texture = GetResource(_textures, task->textureId);
        assert(texture);        
        _context.BindTextureAsShaderResource(task->stage, task->slot, texture);
    }
}

void GLDevice::Execute(const std::vector<DrawTask*>& tasks) {
    for (DrawTask* task : tasks) {
        assert(task);

        LOG_D("%s", task->ToString().c_str());

        Execute(task->bufferUpdates);
        Execute(task->textureUpdates);

        GLPipelineState* pipelineState = GetResource(_pipelineStates, task->pipelineState);
        assert(pipelineState);
        _context.BindPipelineState(pipelineState);

        GLShaderProgram* vertexShader = pipelineState->vertexShader;
        GLVertexLayout* vertexLayout  = pipelineState->vertexLayout;
        GLBuffer* vertexBuffer = GetResource(_buffers, task->vertexBuffer);
        GLBuffer* indexBuffer = task->indexBuffer ? GetResource(_buffers, task->indexBuffer) : nullptr;
        assert(vertexBuffer && vertexBuffer->type == GL_ARRAY_BUFFER);
        assert(vertexLayout);

        GLVertexArrayObject* vao = GetOrCreateVertexArrayObject(vertexShader, vertexBuffer, indexBuffer, vertexLayout);
        assert(vao);

        _context.BindVertexArrayObject(vao);

        Execute(task->shaderParamUpdates);
        Execute(task->textureBinds);

        if(indexBuffer) {
            GL_CHECK(glDrawElements(pipelineState->topology, task->indexCount, GL_UNSIGNED_INT, ((char *)NULL + (task->indexOffset))));
        } else {
            GL_CHECK(glDrawArrays(pipelineState->topology, task->vertexOffset, task->vertexCount));
        }
    }
}

Frame* GLDevice::BeginFrame() {     
    return _currentFrame; 
}

void GLDevice::SubmitFrame() {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    LOG_D("SUBMITFRAME:%s", _currentFrame->ToString().c_str());
    Execute(_currentFrame->GetBufferUpdates());
    Execute(_currentFrame->GetTextureUpdates());
    Execute(_currentFrame->GetDrawTasks());
    _currentFrame->Clear();    
}

void GLDevice::DestroyVertexLayout(VertexLayoutId layout) { assert(false && "TODO"); }

void GLDevice::DestroyBuffer(BufferId handle) {
    // warning: buffer could be referenced by context state
    auto func = [](GLBuffer* buffer) -> void { GL_CHECK(glDeleteBuffers(1, &buffer->id)); };
    assert(DestroyResource<GLBuffer>(handle, _buffers, func));
}

void GLDevice::DestroyShader(ShaderId handle) {
    auto func = [](GLShaderProgram* shader) -> void {
        for (GLPipelineState* pipeline : shader->members) {
            // Note(eugene): We have pipelines referencing a shader that is
            // being destroy. How should this be handled?
            // Also, ShaderParameters reference ShaderPrograms
        }
        GL_CHECK(glDeleteProgram(shader->id));
    };
    assert(DestroyResource<GLShaderProgram>(handle, _shaders, func));
}

void GLDevice::DestroyShaderParam(ShaderParamId handle) {
    GLShaderParameter* param = GetResource<GLShaderParameter>(_shaderParams, handle);
    if (param) {
        _shaderParams.erase(handle);
        delete param;
    }
}

void GLDevice::DestroyPipelineState(PipelineStateId handle) {
    GLPipelineState* pipelineState = GetResource<GLPipelineState>(_pipelineStates, handle);
    if (pipelineState) {
        if (pipelineState->vertexShader) {
            std::vector<GLPipelineState*>& shaderPipelineRefs = pipelineState->vertexShader->members;
            shaderPipelineRefs.erase(std::remove(shaderPipelineRefs.begin(), shaderPipelineRefs.end(), pipelineState),
                                     shaderPipelineRefs.end());
        }
        if (pipelineState->pixelShader) {
            std::vector<GLPipelineState*>& shaderPipelineRefs = pipelineState->pixelShader->members;
            shaderPipelineRefs.erase(std::remove(shaderPipelineRefs.begin(), shaderPipelineRefs.end(), pipelineState),
                                     shaderPipelineRefs.end());
        }

        _pipelineStates.erase(handle);
        delete pipelineState;
    }
}

void GLDevice::DestroyTexture(TextureId handle) {
    auto func = [](GLTexture* tex) -> void { GL_CHECK(glDeleteTextures(1, &tex->id)); };
    assert(DestroyResource<GLTexture>(handle, _textures, func));
}

uint32_t GLDevice::GenerateId() {
    static uint32_t key = 0;
    assert(key < std::numeric_limits<uint32_t>::max());
    return ++key;
}

size_t GLDevice::BuildKey(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLVertexLayout* vertexLayout) {
    size_t key = 0;
    HashCombine(key, vertexShader);
    HashCombine(key, vertexBuffer);
    HashCombine(key, vertexLayout);
    return key;
}


GLVertexArrayObject* GLDevice::GetOrCreateVertexArrayObject(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLBuffer* indexBuffer,
                                                            GLVertexLayout* vertexLayout) {
    assert(vertexShader && vertexShader->type == GL_VERTEX_SHADER);
    assert(vertexBuffer && vertexBuffer->type == GL_ARRAY_BUFFER);
    assert(indexBuffer ? indexBuffer->type == GL_ELEMENT_ARRAY_BUFFER : true);
    assert(vertexLayout);

    size_t key = BuildKey(vertexShader, vertexBuffer, vertexLayout);
 
    auto it = _vaoCache.find(key);
    if (it != _vaoCache.end()) {
        return (*it).second;
    }

    GLVertexArrayObject* vao = new GLVertexArrayObject();
    GL_CHECK(glGenVertexArrays(1, &vao->id));
    _context.BindVertexArrayObject(vao);
    _context.ForceBindBuffer(vertexBuffer);
    if(indexBuffer) {
        _context.ForceBindBuffer(indexBuffer);
    }

    size_t offset                                      = 0;
    const std::vector<GLAttributeMetadata>& attributes = vertexShader->metadata.attributes; // should be sorted
    for (uint32_t idx = 0; idx < vertexLayout->elements.size(); ++idx) {
        const GLVertexElement& element       = vertexLayout->elements[idx];
        const GLAttributeMetadata& attribute = attributes[idx];

        // Attributes of a vertex shader are limited compared to attribute of other shaders         
        const GLVertexElement& attributeAsElement = GLEnumAdapter::Convert(attribute);

        assert(attribute.location == idx);
        assert(attributeAsElement.count == element.count);
        assert(attributeAsElement.type == element.type);

        LOG_D("Binding attribute (size:%d, type:%s, stride:%d, offset:%d) to location:%d", attributeAsElement.count,
              GLEnumAdapter::Convert(attributeAsElement.type).c_str(), vertexLayout->stride, offset, attribute.location);

        GL_CHECK(glEnableVertexAttribArray(attribute.location));
        GL_CHECK(glVertexAttribPointer(attribute.location, element.count, element.type, GL_FALSE,
                                       vertexLayout->stride, (const void*)offset));

        offset += GetByteCount(GLEnumAdapter::ConvertParamType(element.type)) * element.count;
    }

    _vaoCache.insert(std::make_pair(key, vao));

    return vao;
}
}