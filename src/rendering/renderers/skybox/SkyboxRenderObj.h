#pragma once

#include "RenderObj.h"
#include "RendererType.h"
#include "ResourceTypes.h"
#include "ConstantBuffer.h"

struct SkyboxRenderObj : public RenderObj {
    SkyboxRenderObj() : RenderObj(RendererType::Skybox) {}

    graphics::TextureId textureCubeId{0};
    graphics::BufferId vertexBuffer{0};
    ConstantBuffer* constantBuffer{0};
    uint32_t vertexCount{0};
    uint32_t vertexOffset{0};
};