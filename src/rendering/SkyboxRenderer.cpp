//
// Created by Eugene Sturm on 4/22/15.
//

#include "glm/glm.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SkyboxRenderer.h"

#include "File.h"
#include "Log.h"
#include "Config.h"
#include "Image.h"
#include "Helpers.h"

struct SkyboxVertex {
    glm::vec3 pos;

    static graphics::VertLayout layout;
};

void MakeCube(SkyboxVertex* vertices, float x, float y, float z, float scale = 1.f, bool flip = true) {
    static float positions[6][6][3] = {
            { { -0.5, -0.5, 0.5 }, { 0.5, -0.5, 0.5 }, { 0.5, 0.5, 0.5 },
                    { 0.5, 0.5, 0.5 }, { -0.5, 0.5, 0.5 }, { -0.5, -0.5, 0.5 } }, // front

            { { 0.5, -0.5, -0.5 }, { -0.5, -0.5, -0.5 }, { -0.5, 0.5, -0.5 },
                    { -0.5, 0.5, -0.5 }, { 0.5, 0.5, -0.5 }, { 0.5, -0.5, -0.5 } }, // back

            { { -0.5, -0.5, -0.5 }, { -0.5, -0.5, 0.5 }, { -0.5, 0.5, 0.5 },
                    { -0.5, 0.5, 0.5 }, { -0.5, 0.5, -0.5 }, { -0.5, -0.5, -0.5 } }, // left

            { { 0.5, -0.5, 0.5 }, { 0.5, -0.5, -0.5 }, { 0.5, 0.5, -0.5 },
                    { 0.5, 0.5, -0.5 }, { 0.5, 0.5, 0.5 }, { 0.5, -0.5, 0.5 } }, // right

            { { -0.5, 0.5, 0.5 }, { 0.5, 0.5, 0.5 }, { 0.5, 0.5, -0.5 },
                    { 0.5, 0.5, -0.5 }, { -0.5, 0.5, -0.5 }, { -0.5, 0.5, 0.5 } }, // top

            { { 0.5, -0.5, 0.5 }, { -0.5, -0.5, 0.5 }, { -0.5, -0.5, -0.5 },
                    { -0.5, -0.5, -0.5 }, { 0.5, -0.5, -0.5 }, { 0.5, -0.5, 0.5 } }  // bottom
    };

    static float tex_coords[6][2] = {
            {0.f, 0.f }, //bl
            {1.f, 0.f }, //br
            {1.f, 1.f }, //tr
            {1.f, 1.f }, //tr
            {0.f, 1.f }, //tl
            {0.f, 0.f }  //bl
    };

    for (uint32_t f = 0; f < 6; ++f) { // for each face
        for (uint32_t v = 0; v < 6; ++v) { // for each vertex
            vertices->pos[0] = scale * positions[f][flip ? 5 - v : v][0] + x;
            vertices->pos[1] = scale * positions[f][flip ? 5 - v : v][1] + y;
            vertices->pos[2] = scale * positions[f][flip ? 5 - v : v][2] + z;

            //  vertices->tex[0] = tex_coords[v][0];
            //  vertices->tex[1] = tex_coords[v][1];

            vertices++;
        }
    }
}

graphics::VertLayout SkyboxVertex::layout = { { graphics::ParamType::Float3 } };

SkyboxRenderer::SkyboxRenderer(graphics::RenderDevice *device) : _device(device) {
    std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    if (!fs::IsPathDirectory(shaderDirPath)) {
        LOG_E("%s","Invalid Directory Path given for ShaderDirectory. Attempting default.");
        shaderDirPath = fs::AppendPathProcessDir("/shaders");
    }

    std::string vs_contents = ReadFileContents(shaderDirPath +"/" + _device->DeviceConfig.DeviceAbbreviation + "/sky_vs" + _device->DeviceConfig.ShaderExtension);
    const char* vs_src = vs_contents.c_str();

    std::string fs_contents = ReadFileContents(shaderDirPath + "/" + _device->DeviceConfig.DeviceAbbreviation + "/sky_ps" + _device->DeviceConfig.ShaderExtension);
    const char* fs_src = fs_contents.c_str();

    // Note(eugene): cleanup shaders
    _sky_shaders[0] = _device->CreateShader(graphics::ShaderType::VERTEX_SHADER, &vs_src);
    _sky_shaders[1] = _device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, &fs_src);

    assert(_sky_shaders[0] && _sky_shaders[1]);

    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
    if (!fs::IsPathDirectory(assetDirPath)) {
        LOG_E("%s", "Invalid Directory Path given for AssetDirectory. Attempting default.");
        shaderDirPath = fs::AppendPathProcessDir("/assets");
    }

    Image skybox_images[6] = { 0 };
    std::string imagePaths[6] = {
        "/skybox/TropicalSunnyDayLeft2048.png",
        "/skybox/TropicalSunnyDayRight2048.png",
        "/skybox/TropicalSunnyDayUp2048.png",
        "/skybox/TropicalSunnyDayDown2048.png",
        "/skybox/TropicalSunnyDayFront2048.png",
        "/skybox/TropicalSunnyDayBack2048.png"
    };

    for (int x = 0; x < 6; ++x) {
        if (!LoadImageFromFile((assetDirPath + imagePaths[x]).c_str(), &skybox_images[x])) { LOG_D("Failed to load image: %s", imagePaths[x]); }
    }

    LOG_D("w: %d h:%d", skybox_images[0].width, skybox_images[0].height);

    void* datas[6] = {
            skybox_images[0].data,
            skybox_images[1].data,
            skybox_images[2].data,
            skybox_images[3].data,
            skybox_images[4].data,
            skybox_images[5].data
    };

    _sky_box_texture = _device->CreateTextureCube(graphics::TextureFormat::RGB_UBYTE, skybox_images[0].width, skybox_images[0].height, datas);

    assert(_sky_box_texture || "Failed to create skybox texture");

    SkyboxVertex vertices[36];
    MakeCube(&vertices[0], 0.f, 0.f, 0.f, 2.f, false);
    _sky_box_mesh = _device->CreateVertexBuffer(SkyboxVertex::layout, vertices, sizeof(vertices), graphics::BufferUsage::STATIC);

    assert(_sky_box_mesh || "Failed to create vertex buffer");

    _vertex_count = 36;
}


SkyboxRenderer::~SkyboxRenderer() {
    //_device->DestroyProgram(_sky_program);
    //_device->DestroyTexture(_sky_box_texture);
    //_device->DestroyVertexBuffer(_sky_box_mesh);
}

void SkyboxRenderer::OnSubmit(Camera *camera) {

    glm::mat4 persp_view = camera->BuildProjection() * camera->BuildView();
    glm::mat4 translate = glm::translate(glm::mat4(), camera->pos);
    glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(50.f, 50.f, 50.f));
    glm::mat4 model = translate * scale;
    
    graphics::RasterState raster_state;
    raster_state.cull_mode = graphics::CullMode::NONE;
    graphics::DepthState depth_state;
    depth_state.depth_func = graphics::DepthFunc::LESS_EQUAL;
    depth_state.enable = true;
    
    _device->SetVertexShader(_sky_shaders[0]);
    _device->SetPixelShader(_sky_shaders[1]);
    _device->SetVertexBuffer(_sky_box_mesh);
    _device->SetShaderParameter(_sky_shaders[0], graphics::ParamType::Float4x4, "world", (void*)glm::value_ptr(persp_view * model));
    _device->SetShaderTexture(_sky_shaders[1], _sky_box_texture, graphics::TextureSlot::BASE);
    _device->SetDepthState(depth_state);
    _device->SetRasterState(raster_state);

    _device->DrawPrimitive(graphics::PrimitiveType::TRIANGLES, 0, _vertex_count);
}


