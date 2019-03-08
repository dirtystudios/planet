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

    FrameView() = default;

    FrameView(glm::vec3 ep, glm::mat4 proj, glm::mat4 v, glm::mat4 o, glm::vec3 l, Frustum f, Viewport vp) :
        eyePos(ep),
        projection(proj),
        view(v),
        ortho(o),
        look(l),
        frustum(proj, v),
        viewport(vp)
    {}

    FrameView(const FrameView& o) :
        eyePos(o.eyePos),
        projection(o.projection),
        view(o.view),
        ortho(o.ortho),
        look(o.look),
        frustum(o.projection, o.view),
        viewport(o.viewport)
    {}

    bool operator==(const FrameView& other) const {
        return glm::all(glm::lessThan(eyePos - other.eyePos, glm::vec3(0.01f))) &&
            projection == other.projection &&
            view == other.view &&
            ortho == other.ortho &&
            look == other.look &&
            viewport == other.viewport;
    }
};

struct RenderView {
    RenderView(Camera* camera, Viewport* viewport) : camera(camera), viewport(viewport){};

    Camera*   camera{nullptr};
    Viewport* viewport{nullptr};

    FrameView frameView() {
        glm::mat4 proj  = camera->BuildProjection();
        glm::mat4 view  = camera->BuildView();
        glm::mat4 ortho = glm::ortho(0.0f, viewport->width, 0.0f, viewport->height);
        return FrameView{camera->pos, proj, view, ortho, camera->look, {proj, view}, *viewport};
    }
};
