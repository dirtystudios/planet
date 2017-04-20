#pragma once

#include "ConstantBuffer.h"
#include "DrawItem.h"
#include "RenderObj.h"
#include "RendererType.h"
#include "ResourceTypes.h"
#include "StateGroup.h"

class SkyRenderer;

class SkyboxRenderObj : public RenderObj {
private:
    friend SkyRenderer;

public:
    SkyboxRenderObj(const std::array<std::string, 6>& imagePaths) : RenderObj(RendererType::Skybox), _imagePaths(imagePaths) {}
    ~SkyboxRenderObj() {}

private:
    gfx::TextureId  _textureCubeId{0};
    gfx::BufferId   _vertexBuffer{0};
    ConstantBuffer* _constantBuffer{0};

    const gfx::StateGroup* _group{nullptr};
    const gfx::DrawItem*   _item{nullptr};

    std::array<std::string, 6> _imagePaths;
};
