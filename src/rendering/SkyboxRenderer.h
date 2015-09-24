//
// Created by Eugene Sturm on 4/22/15.
//

#ifndef DG_SKYBOXRENDERER_H
#define DG_SKYBOXRENDERER_H

#include "RenderDevice.h"
#include "Camera.h"

class SkyboxRenderer {
private:
    graphics::RenderDevice* _device { 0 };
    graphics::ShaderHandle _sky_shaders[2] { 0 };
    graphics::TextureHandle  _sky_box_texture { 0 };
    graphics::VertexBufferHandle  _sky_box_mesh { 0 };
    uint32_t _vertex_count { 0 };
public:
    SkyboxRenderer(graphics::RenderDevice* device);
    ~SkyboxRenderer();
    void OnSubmit(Camera *camera);
};


#endif //DG_SKYBOXRENDERER_H
