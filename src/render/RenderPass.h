#pragma once

#include <array>
#include "ResourceTypes.h"

enum class RenderPassType {
    Invalid = 0,
    Standard,
};

static std::array<RenderPassType, 1> renderPassTypeArray = { RenderPassType::Standard };

struct RenderPass {
    RenderPassType type{RenderPassType::Invalid};
    gfx::RenderPassId passId{0};
    size_t colorAttachmentCount{0};
    gfx::TextureId colorAttachments[8];
    gfx::TextureId depthAttachment{0};
    gfx::TextureId stencilAttachment{0};
};
