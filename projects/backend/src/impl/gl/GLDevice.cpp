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
#include "StringUtil.h"
#include "ShaderStageFlags.h"
#include "DrawItemDecoder.h"
#include "ResourceTypes.h"
#include "DGAssert.h"

int32_t GetSlotFromString(const std::string& source, gfx::Binding::Type type) {
    static constexpr uint32_t kMaxSlot = 10;
    std::string prefix = type == gfx::Binding::Type::ConstantBuffer ? "_b" : "_s";
    for (uint32_t slotIdx = 0; slotIdx < kMaxSlot; ++slotIdx) {
        std::string searchString = prefix + std::to_string(slotIdx) + "_";
        if (source.find(searchString) == 0) {
            return slotIdx;
        }
    }
    return -1;
};

namespace gfx {

GLDevice::GLDevice() {
    this->DeviceConfig.ShaderExtension    = ".glsl";
    this->DeviceConfig.DeviceAbbreviation = "GL";
    this->DeviceConfig.ShaderDir          = "GL";
}

RenderDeviceApi GLDevice::GetDeviceApi() { return RenderDeviceApi::OpenGL; }
GLDevice::~GLDevice() {}

void GLDevice::ResizeWindow(uint32_t width, uint32_t height) {}

void GLDevice::PrintDisplayAdapterInfo() {
    LOG_D("GL_VERSION: %s", glGetString(GL_VERSION));
    LOG_D("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOG_D("GL_VENDOR: %s", glGetString(GL_VENDOR));
    LOG_D("GL_RENDERER: %s", glGetString(GL_RENDERER));
}

BufferId GLDevice::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
    GLBuffer* buffer = new GLBuffer();
    GL_CHECK(glGenBuffers(1, &buffer->id));
    dg_assert_nm((buffer->id > 0));
    
    if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadCpuWriteBits)) {
        buffer->usage = GL_DYNAMIC_DRAW;
    } else if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadBit)) {
        buffer->usage = GL_STATIC_DRAW;
    } else {
        assert(false && "unsupported access flags");
    }

    // todo: dont support mutliple flags, just grab first one
    buffer->type = GLEnumAdapter::Convert(desc.usageFlags);

    if (desc.size != 0) {
        uint32_t buffSize = desc.size;
        if (buffer->type == GL_UNIFORM_BUFFER){
            // length must be multiple of 16
            uint32_t sizeCheck = buffSize % 16;
            if (sizeCheck != 0) {
                buffSize += (16 - sizeCheck);
            }
        }
        _context.WriteBufferData(buffer, initialData, buffSize);
    }
    
    return _resourceManager.AddResource(buffer);
}

ShaderId GLDevice::GetShader(ShaderType type, const std::string& functionName) {
    return _lib.GetShader(type, functionName);
}

void GLDevice::AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) {
    for (const ShaderData& shaderData : shaderData) {
        dg_assert_nm(shaderData.type == ShaderDataType::Source);

        // I regert nothing
        const char* src       = reinterpret_cast<const char*>(shaderData.data);
        const char* ptr       = src;
        const char* nameStart = 0;
        const char* typeStart = 0;
        while (*ptr++ != '\n') {
            if (*ptr == '/' || *ptr == ' ')
                continue;
            if (!nameStart)
                nameStart = ptr;
            else if (*ptr == '_')
                typeStart = ptr + 1;
        }

        std::string name(nameStart, typeStart - 1);
        std::string type(typeStart, ptr - 1);
        type = dutil::TrimRight(type);

        dg_assert_nm(type == "vertex" || type == "pixel");

        ShaderFunctionDesc function;
        function.type         = type == "pixel" ? ShaderType::PixelShader : ShaderType::VertexShader;
        function.functionName = name;
        function.entryPoint   = "main";

        ShaderId existingShaderId = _lib.GetShader(function.type, function.functionName);
        ShaderId shaderId         = CreateShader(function, shaderData);

        if (existingShaderId && shaderId) {
            GLShaderProgram* existing = _resourceManager.GetResource<GLShaderProgram>(existingShaderId);
            GLShaderProgram* updated = _resourceManager.GetResource<GLShaderProgram>(shaderId);
            dg_assert_nm(existing && updated);
            
            GL_CHECK(glDeleteShader(existing->id));
            existing->id = updated->id;
            existing->metadata = updated->metadata;
                        
        } else {
            _lib.AddShader(shaderId, function);
        }
    }
}

ShaderId GLDevice::CreateShader(const ShaderFunctionDesc& funcDesc, const ShaderData& shaderData) {
    assert(shaderData.type == ShaderDataType::Source);
    const char* csrc = reinterpret_cast<const char*>(shaderData.data);
    return CreateShader(funcDesc.type, csrc);
}

ShaderId GLDevice::CreateShader(ShaderType shaderType, const char* source) {
    GLShaderProgram* shader = new GLShaderProgram();

    shader->type               = GLEnumAdapter::Convert(shaderType);
    const char* shaderSource[] = {source};
    shader->id = glCreateShaderProgramv(shader->type, 1, (const GLchar**)shaderSource);
    assert(shader->id);

    GLint linked = 0;
    GL_CHECK(glGetProgramiv(shader->id, GL_LINK_STATUS, &linked));
    if (!linked) {
        char log[1024];
        GL_CHECK(glGetProgramInfoLog(shader->id, 1024, nullptr, log));
        LOG_E("Failed to compile/link program: %s\n--------------:\n%s\n", log, source);
        delete shader;
        return 0;
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
                case GL_SAMPLER_CUBE:
                case GL_SAMPLER_2D: {
                    dg_assert(
                        uniform->name.find("_s") == 0, "Invalid sampler name:%s. Sampler must be prefix with slot id. "
                                                       "(ex. \"_s<slot id>_<param_nam>\")",
                        uniform->name.c_str()); // samplers need to define their slot in their name..yay opengl 4.1
                    GLSamplerMetadata samplerMetadata;
                    samplerMetadata.uniform = uniform;
                    samplerMetadata.slot = GetSlotFromString(uniform->name, gfx::Binding::Type::Texture);
                    metadata->samplers.push_back(samplerMetadata);
                    break;
                }
                default: {
                    assert(uniform->name.find("_s") != 0); // right now just to catch my mistakes
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
            assert(block->name.find("_b") == 0); // buffers need to define their slot in their name..yay opengl 4.1

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
            block->slot = GetSlotFromString(block->name, gfx::Binding::Type::ConstantBuffer);
            GL_CHECK(glUniformBlockBinding(shader->id, block->index, block->slot));
        }
        delete[] name;
    }
    
    return _resourceManager.AddResource(shader);
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

        //LOG_D("idx(%d): %s -> %s", idx, element.ToString().c_str(), attributeAsElement.ToString().c_str());

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

    pipelineState->vertexShader = _resourceManager.GetResource<GLShaderProgram>(desc.vertexShader);
    if (pipelineState->vertexShader) {
        pipelineState->vertexShader->members.push_back(pipelineState);
    }

    assert(pipelineState->vertexShader);

    pipelineState->pixelShader = _resourceManager.GetResource<GLShaderProgram>(desc.pixelShader);
    if (pipelineState->pixelShader) {
        pipelineState->pixelShader->members.push_back(pipelineState);
    }

    assert(pipelineState->pixelShader);

    // TODO: Smart person would cache
    pipelineState->depthState  = GLEnumAdapter::Convert(desc.depthState);
    pipelineState->blendState  = GLEnumAdapter::Convert(desc.blendState);
    pipelineState->rasterState = GLEnumAdapter::Convert(desc.rasterState);

    pipelineState->vertexLayout = _resourceManager.GetResource<GLVertexLayout>(desc.vertexLayout);
    assert(pipelineState->vertexLayout);

    // TODO:: Do this first
    assert(isVertexLayoutValidWithShader(*pipelineState->vertexLayout, *pipelineState->vertexShader));

    pipelineState->topology = GLEnumAdapter::Convert(desc.topology);

    return _resourceManager.AddResource(pipelineState);
}

TextureId GLDevice::CreateTexture2D(PixelFormat texFormat, uint32_t width, uint32_t height, void* data, const std::string& debugName) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type      = GL_TEXTURE_2D;
    texture->height = height;
    texture->width = width;

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(0, texture);
    if (texFormat == PixelFormat::R8Unorm) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    }
    // TODO: Move to GLContext
    GL_CHECK(glTexImage2D(texture->type, 0, texture->format.internalFormat, width, height, 0, texture->format.dataFormat, texture->format.dataType, data));
    if (texFormat == PixelFormat::R8Unorm) {
        GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }

    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    
    return _resourceManager.AddResource(texture);
}

TextureId GLDevice::CreateTextureArray(PixelFormat texFormat, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type      = GL_TEXTURE_2D_ARRAY;
    texture->height = height;
    texture->width = width;

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(0, texture);
    // TODO: Move to GLContext
    GL_CHECK(glTexStorage3D(texture->type, levels, texture->format.internalFormat, width, height, depth));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(texture->type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    return _resourceManager.AddResource(texture);
}

TextureId GLDevice::CreateTextureCube(PixelFormat texFormat, uint32_t width, uint32_t height, void** data, const std::string& debugName) {
    GLTexture* texture = new GLTexture();
    texture->format    = GLEnumAdapter::Convert(texFormat);
    texture->type      = GL_TEXTURE_CUBE_MAP;
    assert(width == height);
    texture->height = height;
    texture->width = width;

    GL_CHECK(glGenTextures(1, &texture->id));
    _context.BindTexture(0, texture);

    // TODO: Move to GLContext
    for (uint32_t side = 0; side < 6; ++side) {
        GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, texture->format.internalFormat, width, height, 0, texture->format.dataFormat, texture->format.dataType,
                              data[side]));
    }

    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    return _resourceManager.AddResource(texture);
}

VertexLayoutId GLDevice::CreateVertexLayout(const VertexLayoutDesc& desc) {
    assert(desc.elements.size() > 0);

    // size_t key = std::hash<VertexLayoutDesc>()(desc);
    // GLVertexLayout* vertexLayout = GetOrCreateVertexLayout(desc):

    GLVertexLayout* vertexLayout = new GLVertexLayout();
    for (const VertexLayoutElement& element : desc.elements) {
        GLVertexElement glElement = GLEnumAdapter::Convert(element);
        vertexLayout->stride += GetByteCount(element);
        vertexLayout->elements.push_back(glElement);
    }

    return _resourceManager.AddResource(vertexLayout);
}

void GLDevice::DestroyResource(ResourceId resourceId) {
    _resourceManager.DestroyResource(resourceId);
}

size_t GLDevice::BuildKey(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLVertexLayout* vertexLayout) {
    size_t key = 0;
    HashCombine(key, vertexShader);
    HashCombine(key, vertexBuffer);
    HashCombine(key, vertexLayout);
    return key;
}

GLVertexArrayObject* GLDevice::GetOrCreateVertexArrayObject(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer,
                                                            const VertexStream& stream, GLBuffer* indexBuffer,
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
    if (indexBuffer) {
        _context.ForceBindBuffer(indexBuffer);
    }

    size_t                                  offset     = 0;                                 // not sure if this is correct
    const std::vector<GLAttributeMetadata>& attributes = vertexShader->metadata.attributes; // should be sorted
    for (uint32_t idx = 0; idx < vertexLayout->elements.size(); ++idx) {
        const GLVertexElement&     element   = vertexLayout->elements[idx];
        const GLAttributeMetadata& attribute = attributes[idx];

        // Attributes of a vertex shader are limited compared to attribute of other shaders
        const GLVertexElement& attributeAsElement = GLEnumAdapter::Convert(attribute);

        assert(attribute.location == idx);
        assert(attributeAsElement.count == element.count);
        assert(attributeAsElement.type == element.type);

        LOG_D("Binding attribute (size:%d, type:%s, stride:%d, offset:%d) to location:%d", attributeAsElement.count,
              GLEnumToString(attributeAsElement.type).c_str(), vertexLayout->stride, offset, attribute.location);

        GL_CHECK(glEnableVertexAttribArray(attribute.location));
        GL_CHECK(glVertexAttribPointer(attribute.location, element.count, element.type, GL_FALSE, vertexLayout->stride, (const void*)offset));

        offset += GetByteCount(GLEnumAdapter::ConvertParamType(element.type)) * element.count;
    }

    _vaoCache.insert(std::make_pair(key, vao));

    return vao;
}

void GLDevice::UpdateTexture(TextureId texture, uint32_t slice, const void* srcData) {
    GLTexture* tex = _resourceManager.GetResource<GLTexture>(texture);
    dg_assert_nm(texture != 0);
    _context.WriteTextureData(tex, srcData, slice);
}

CommandBuffer* GLDevice::CreateCommandBuffer() {
    return new CommandBuffer();
}
void GLDevice::Submit(const std::vector<CommandBuffer*>& cmdBuffers) { _submittedBuffers.insert(end(_submittedBuffers), begin(cmdBuffers), end(cmdBuffers)); }

void GLDevice::BindResource(const Binding& binding) {
    if (binding.stageFlags != gfx::ShaderStageFlags::AllStages) {
        LOG_W("Unsupported ShaderStageFlags configuration. Currently only support "
              "AllStages. Will render as AllStages");
    }

    switch (binding.type) {
        case Binding::Type::ConstantBuffer: {
            _context.BindUniformBufferToSlot(_resourceManager.GetResource<GLBuffer>(binding.resource), binding.slot);
            break;
        }
        case Binding::Type::Texture: {
            GLTexture* texture = _resourceManager.GetResource<GLTexture>(binding.resource);
            dg_assert_nm(texture != 0);
            _context.BindTextureAsShaderResource(texture, binding.slot);
            break;
        }
    }
}


void GLDevice::Execute(CommandBuffer* cmdBuffer) {
    const std::vector<const DrawItem*>* items = cmdBuffer->GetDrawItems();
    for (const DrawItem* item : *items) {
        //LOG_D("%s", "DrawItem");

        DrawItemDecoder decoder(item);

        PipelineStateId           psId;
        DrawCall                  drawCall;
        BufferId                  indexBufferId;
        size_t                    streamCount  = decoder.GetStreamCount();
        size_t                    bindingCount = decoder.GetBindingCount();
        std::vector<VertexStream> streams(streamCount);
        std::vector<Binding>      bindings(bindingCount);
        VertexStream*             streamPtr  = streams.data();
        Binding*                  bindingPtr = bindings.data();

        assert(streamCount == 1); // > 1 not supported

        dg_assert_nm(decoder.ReadDrawCall(&drawCall));
        dg_assert_nm(decoder.ReadPipelineState(&psId));
        dg_assert_nm(decoder.ReadIndexBuffer(&indexBufferId));
        dg_assert_nm(decoder.ReadVertexStreams(&streamPtr));
        if (bindingCount > 0) {
            dg_assert_nm(decoder.ReadBindings(&bindingPtr));
        }

        GLPipelineState* pipelineState = _resourceManager.GetResource<GLPipelineState>(psId);
        assert(pipelineState);
        _context.BindPipelineState(pipelineState);

        const VertexStream& stream       = streams[0];
        GLShaderProgram*    vertexShader = pipelineState->vertexShader;
        GLVertexLayout*     vertexLayout = pipelineState->vertexLayout;
        GLBuffer*           vertexBuffer = _resourceManager.GetResource<GLBuffer>(stream.vertexBuffer);
        GLBuffer*           indexBuffer  = indexBufferId ? _resourceManager.GetResource<GLBuffer>(indexBufferId) : nullptr;
        assert(vertexBuffer && vertexBuffer->type == GL_ARRAY_BUFFER);
        assert(vertexLayout);

        GLVertexArrayObject* vao = GetOrCreateVertexArrayObject(vertexShader, vertexBuffer, stream, indexBuffer, vertexLayout);
        assert(vao);

        _context.BindVertexArrayObject(vao);

        // bindings
        for (const Binding& binding : bindings) {
            BindResource(binding);
        }

        switch (drawCall.type) {
            case DrawCall::Type::Arrays: {
                GL_CHECK(glDrawArrays(pipelineState->topology, (GLint)drawCall.startOffset, (GLsizei)drawCall.primitiveCount));
                break;
            }
            case DrawCall::Type::Indexed: {
                assert(indexBuffer);
                GL_CHECK(glDrawElementsBaseVertex(pipelineState->topology, drawCall.primitiveCount, GL_UNSIGNED_INT, ((char*)0 + (drawCall.startOffset * sizeof(uint32_t))), drawCall.baseVertexOffset));
                break;
            }
        }
        ++_drawCallCount;
    }
}

uint8_t* GLDevice::MapMemory(BufferId bufferId, BufferAccess access) {
    GLBuffer* buffer = _resourceManager.GetResource<GLBuffer>(bufferId);
    return _context.Map(buffer, access);
}

void GLDevice::UnmapMemory(BufferId bufferId) {
    GLBuffer* buffer = _resourceManager.GetResource<GLBuffer>(bufferId);
    _context.Unmap(buffer);
}

void GLDevice::RenderFrame() {
    //LOG_D("%s", "RenderFrame");
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    _drawCallCount = 0;
    for (uint32_t idx = 0; idx < _submittedBuffers.size(); ++idx) {
        CommandBuffer* cmdBuffer = _submittedBuffers[idx];
        Execute(cmdBuffer);
        cmdBuffer->Reset();
    }
    _submittedBuffers.clear();
}
}
