#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "Frustum.h"
#include "Viewport.h"
#include "RenderObj.h"

struct FrameView {
    const glm::vec3 eyePos;
    const glm::mat4 projection;
    const glm::mat4 view;
    const glm::mat4 ortho;
    const glm::vec3 look;
    const Frustum   frustum;
    const Viewport  viewport;
    
    std::vector<RenderObj*> _visibleObjects;
};

struct RenderView {
    RenderView(Camera* camera, Viewport* viewport) : camera(camera), viewport(viewport){};

    Camera*   camera{nullptr};
    Viewport* viewport{nullptr};

    FrameView frameView() {
        glm::mat4 proj  = camera->BuildProjection();
        glm::mat4 view  = camera->BuildView();
        glm::mat4 ortho = glm::ortho(0.0f, viewport->width, 0.0f, viewport->height);
        return {camera->pos, proj, view, ortho, camera->look, {proj, view}, *viewport};
    }
};
