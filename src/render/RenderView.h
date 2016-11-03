#pragma once

#include "Camera.h"
#include "Frustum.h"
#include "Viewport.h"

struct RenderViewInstance {
    glm::dvec3 eyePos;
    glm::mat4  projection;
    glm::mat4  view;
    Frustum    frustum;
};

struct RenderView {
    RenderView(Camera* camera, Viewport* viewport) : camera(camera), viewport(viewport){};

    Camera*   camera{nullptr};
    Viewport* viewport{nullptr};

    //    RenderViewInstance build() {
    //        RenderViewInstance instance;
    //
    //
    //        return instance;
    //    }
};
