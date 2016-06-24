#pragma once

#include "RenderObj.h"
#include "RendererType.h"
#include "ResourceTypes.h"
#include "ConstantBuffer.h"

struct SkyboxRenderObj : public RenderObj {
    SkyboxRenderObj() : RenderObj(RendererType::Skybox) {}

    gfx::TextureId textureCubeId{0};
    gfx::BufferId vertexBuffer{0};
    ConstantBuffer* constantBuffer{0};
    uint32_t vertexCount{0};
    uint32_t vertexOffset{0};
};
