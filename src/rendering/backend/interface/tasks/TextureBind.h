#pragma once

#include "ResourceTypes.h"
#include "ShaderStage.h"

namespace graphics {
struct TextureBind {
	TextureBind(ShaderStage stage, TextureId id, TextureSlot slot) : stage(stage), textureId(id), slot(slot) {}

    TextureId textureId{0};
    TextureSlot slot{TextureSlot::Base};
    ShaderStage stage{ShaderStage::Vertex};
};
}