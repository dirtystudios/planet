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
#include "Memory.h"
#include "DrawItemDecoder.h"


int32_t GetSlotFromString(const std::string& source) {
    static constexpr uint32_t kMaxSlot = 10;
    
    for(uint32_t slotIdx = 0; slotIdx < kMaxSlot; ++slotIdx) {
        std::string searchString = "_slot" + std::to_string(slotIdx) + "_";
        if (source.find(searchString) == 0) {
            return slotIdx;
        }
    }
    return -1;
};



namespace graphics {

GLDevice::GLDevice() {
    this->DeviceConfig.ShaderExtension    = ".glsl";
    this->DeviceConfig.DeviceAbbreviation = "GL";
    _drawItemByteBuffer.Resize(memory::KilobytesToBytes(1));
    _drawItemEncoder                = new DrawItemEncoder(&_drawItemByteBuffer);
}

GLDevice::~GLDevice() {
    delete _drawItemEncoder;
}

void GLDevice::ResizeWindow(uint32_t width, uint32_t height) {}

void GLDevice::PrintDisplayAdapterInfo() {
    printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    printf("GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
}

BufferId GLDevice::AllocateBuffer(BufferType type, size_t size, BufferUsage usage) {
    GLBuffer* buffer = new GLBuffer();
    GL_CHECK(glGenBuffers(1, &buffer->id));
    assert(buffer->id);
    buffer->usage = GLEnumAdapter::Convert(usage);
    buffer->type = GLEnumAdapter::Convert(type);

    if (size != 0) {
        _context.WriteBufferData(buffer, nullptr, size);
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

            switch (uniform->type) {
                case GL_SAMPLER_1D_ARRAY:
                case GL_SAMPLER_2D_ARRAY:
                case GL_INT_SAMPLER_1D:
                case GL_INT_SAMPLER_2D:
                case GL_INT_SAMPLER_3D:
                case GL_INT_SAMPLER_CUBE:
                case GL_INT_SAMPLER_1D_ARRAY:
                case GL_INT_SAMPLER_2D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_1D:
                case GL_UNSIGNED_INT_SAMPLER_2D:
                case GL_UNSIGNED_INT_SAMPLER_3D:
                case GL_UNSIGNED_INT_SAMPLER_CUBE:
                case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
                case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: {
                    assert(uniform->name.find("_slot") == 0); // samplers need to define their slot in their name..yay opengl 4.1
                    GLSamplerMetadata samplerMetadata;
                    samplerMetadata.uniform = uniform;
                    samplerMetadata.slot = GetSlotFromString(uniform->name);
                    metadata->samplers.push_back(samplerMetadata);
                    break;
                }
            }
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
            assert(block->name.find("_slot") == 0); // buffers need to define their slot in their name..yay opengl 4.1

            GLint* uniformIndices = new GLint[uniformCount];
            glGetActiveUniformBlockiv(shader->id, idx, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices);

            // get parameters of all uniform variables in uniform block
            for (uint32_t jdx = 0; jdx < uniformCount; ++jdx) {
                assert(uniformIndices[jdx] >= 0);

                GLint uniformName_len           = 0;
                GLBlockUniformMetadata* uniform = &block->uniforms[jdx];
                uniform->index                  = uniformIndices[jdx];

                // get length of name of uniform variable
                glGetActiveUniformsiv(shader->id, 1, &uniform->index, GL_UNIFORM_NAME_LENGTH, &uniformName_len);

                // get name of uniform variable
                char* uniformName = new char[uniformName_len];
                GLsizei length;
                GLint size;
                GLenum type;                
                glGetActiveUniform(shader->id, uniform->index, uniformName_len, &length, &size, &type, uniformName);
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

            block->index = glGetUniformBlockIndex(shader->id, name);
            block->slot = GetSlotFromString(block->name);
            GL_CHECK(glUniformBlockBinding(shader->id, block->index, block->slot));
        }
        delete[] name;
    }

    uint32_t handle = GenerateId();
    _shaders.insert(std::make_pair(handle, shader));
   
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
    _context.BindTexture(0, texture);
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
    _context.BindTexture(0, texture);
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
    _context.BindTexture(0, texture);

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

CommandBuffer* GLDevice::CreateCommandBuffer() {
    GLCommandBuffer* cmdBuffer = _commandBufferPool.Get();
    assert(cmdBuffer);
    cmdBuffer->Reset();
    return cmdBuffer;
        
}
void GLDevice::Submit(const std::vector<CommandBuffer*>& cmdBuffers) {
    _submittedBuffers.insert(end(_submittedBuffers), begin(cmdBuffers), end(cmdBuffers));
}

DrawItemEncoder* GLDevice::GetDrawItemEncoder() {
    return _drawItemEncoder;
}

void GLDevice::Execute(GLCommandBuffer* cmdBuffer) {
    
    ByteBuffer _byteBuffer = cmdBuffer->GetByteBuffer();
    GLCommandBuffer::CommandType cmd;
    while(_byteBuffer.ReadPos() < _byteBuffer.WritePos()) {
        _byteBuffer >> cmd;
        switch(cmd) {
            case GLCommandBuffer::CommandType::DrawItem: {
                LOG_D("%s", "DrawItem");
                const DrawItem* item;
                _byteBuffer >> item;
                
                DrawItemDecoder decoder(item);
                
                GLPipelineState* pipelineState = GetResource(_pipelineStates, decoder.pipelineState());
                assert(pipelineState);
                _context.BindPipelineState(pipelineState);
                
                assert(decoder.vertexStreamCount() == 1); // > 1 not supported
                const VertexStream& stream = decoder.streams()[0];
                GLShaderProgram* vertexShader = pipelineState->vertexShader;
                GLVertexLayout* vertexLayout = pipelineState->vertexLayout;
                GLBuffer* vertexBuffer = GetResource(_buffers, stream.vertexBuffer);
                GLBuffer* indexBuffer = decoder.indexBuffer() ? GetResource(_buffers, decoder.indexBuffer()) : nullptr;
                assert(vertexBuffer && vertexBuffer->type == GL_ARRAY_BUFFER);
                assert(vertexLayout);
                
                GLVertexArrayObject* vao = GetOrCreateVertexArrayObject(vertexShader, vertexBuffer, indexBuffer, vertexLayout);
                assert(vao);
                
                _context.BindVertexArrayObject(vao);
                
                // bindings
                for (uint32_t idx = 0; idx < decoder.bindingCount(); ++idx) {
                    const Binding& binding = decoder.bindings()[idx];
                    switch (binding.type) {
                        case Binding::Type::ConstantBuffer: {
                            _context.BindUniformBufferToSlot(GetResource(_buffers, binding.resource), binding.slot);
                            break;
                        }
                        case Binding::Type::Texture: {
                            GLTexture* texture = GetResource(_textures, binding.resource);
                            assert(texture);
                            _context.BindTextureAsShaderResource(ShaderStage::Vertex, binding.slot, texture);
                            _context.BindTextureAsShaderResource(ShaderStage::Pixel, binding.slot, texture);
                            break;
                        }
                    }
                }
                
                
                const DrawCall* drawCall = decoder.drawCall();
                switch (drawCall->type) {
                    case DrawCall::Type::Arrays: {
                        GL_CHECK(glDrawArrays(pipelineState->topology, (GLint) drawCall->offset, (GLsizei) drawCall->primitiveCount));
                        break;
                    }
                    case DrawCall::Type::Indexed: {
                        assert(indexBuffer);
                        GL_CHECK(glDrawElements(pipelineState->topology, drawCall->primitiveCount, GL_UNSIGNED_INT, ((char *) 0 + (drawCall->offset))));
                        break;
                    }
                }
                
                
                break;
            }
            case GLCommandBuffer::CommandType::Clear: {
                LOG_D("%s", "Clear");                
                GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                break;
            }
            case GLCommandBuffer::CommandType::BindResource: {
                LOG_D("%s", "BindResource");
                Binding binding;
                _byteBuffer >> binding;
                
                // TODO:: this is copy pasted from above, dont do that.
                switch (binding.type) {
                    case Binding::Type::ConstantBuffer: {
                        _context.BindUniformBufferToSlot(GetResource(_buffers, binding.resource), binding.slot);
                        break;
                    }
                    case Binding::Type::Texture: {
                        GLTexture* texture = GetResource(_textures, binding.resource);
                        assert(texture);
                        _context.BindTextureAsShaderResource(ShaderStage::Vertex, binding.slot, texture);
                        _context.BindTextureAsShaderResource(ShaderStage::Pixel, binding.slot, texture);
                        break;
                    }
                }
                

                break;
            }
        }
    }
}
    
uint8_t* GLDevice::MapMemory(BufferId bufferId, BufferAccess access) {
    GLBuffer* buffer = GetResource(_buffers, bufferId);
    return _context.Map(buffer, access);
}

void GLDevice::UnmapMemory(BufferId bufferId) {
    GLBuffer* buffer = GetResource(_buffers, bufferId);
    _context.Unmap(buffer);
}
    

void GLDevice::RenderFrame() {
    LOG_D("%s", "RenderFrame");
    for (uint32_t idx = 0; idx < _submittedBuffers.size(); ++idx) {
        GLCommandBuffer* glBuffer = reinterpret_cast<GLCommandBuffer*>(_submittedBuffers[idx]);
        Execute(glBuffer);
        glBuffer->Reset();
    }
    _submittedBuffers.clear();
    _drawItemByteBuffer.Reset();
    
    
}
}
