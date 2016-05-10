#pragma once

#include <stdint.h>
#include <vector>
#include "TextureSlot.h"
#include "ShaderParamUpdate.h"
#include "ResourceTypes.h"
#include "BufferUpdate.h"
#include "TextureUpdate.h"
#include "TextureBind.h"
#include <string>

namespace graphics {
class DrawTask {
public:
    std::vector<BufferUpdate*> bufferUpdates;
    std::vector<TextureUpdate*> textureUpdates;
    std::vector<TextureBind*> textureBinds;
    std::vector<ShaderParamUpdate*> shaderParamUpdates;

    void BindTexture(ShaderStage stage, TextureId textureId, TextureSlot slot) {
        textureBinds.push_back(new TextureBind(stage, textureId, slot));        
    }

    template <typename T>
    void UpdateShaderParam(ShaderParamId paramId, T* data, uint32_t count = 1) {
        shaderParamUpdates.push_back(new ShaderParamUpdate(paramId, data, sizeof(T) * count));
    }

    PipelineStateId pipelineState{0};

    BufferId vertexBuffer{0};
    uint32_t vertexOffset{0};
    uint32_t vertexCount{0};

    BufferId indexBuffer{0};
    uint32_t indexOffset{0};
    uint32_t indexCount{0};

    std::string ToString() {
        return "DrawTask [textureUpdates:" + std::to_string(textureUpdates.size()) + ", bufferUpdates: " +
               std::to_string(bufferUpdates.size()) + ", textureBinds: " + std::to_string(textureBinds.size()) +
               ", shaderParamUpdates: " + std::to_string(shaderParamUpdates.size()) + "]";
    }
};
}