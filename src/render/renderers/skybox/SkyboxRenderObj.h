#pragma once

#include "RenderObj.h"
#include "RendererType.h"
#include "ResourceTypes.h"
#include "ConstantBuffer.h"
#include "StateGroup.h"
#include "DrawItem.h"

struct SkyboxRenderObj : public RenderObj {
    SkyboxRenderObj() : RenderObj(RendererType::Skybox) {}

    gfx::TextureId textureCubeId{0};
    gfx::BufferId vertexBuffer{0};
    ConstantBuffer* constantBuffer{0};

    const gfx::StateGroup* group{nullptr};
    const gfx::DrawItem* item{nullptr};
};
