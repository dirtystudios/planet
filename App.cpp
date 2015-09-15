#include "stdafx.h"
#include "App.h"
#include "Camera.h"
#include "System.h"
#include "Helpers.h"
#include <noise/noise.h>
#include "Frustum.h"
#include "DebugRenderer.h"
#include "Log.h"
#include "ChunkedLODTerrainRenderer.h"

uint32_t frame_count = 0;
double curr_frame_time = 0;
double prev_frame_time = 0;
double accumulate = 0;
double total_frame_count = 0;
double frame_time = 0;
Camera cam;
float mouse_speed = 1.f;
float walk_speed = 300.f;

void HandleInput(const app::KeyState& key_state, const app::CursorState& cursor_state, float dt) {

    glm::vec3 translation(0, 0, 0);
    if (key_state.IsPressed(app::KeyCode::KEY_1)) {
        walk_speed = 600.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_2)) {
        walk_speed = 100.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_3)) {
        walk_speed = 15.f;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_W)) {
        translation.z += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_A)) {
        translation.x -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_S)) {
        translation.z -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_D)) {
        translation.x += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_Q)) {
        translation.y += walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_E)) {
        translation.y -= walk_speed * dt;
    }

    if (key_state.IsPressed(app::KeyCode::KEY_W)) {
        translation.z += walk_speed * dt;
    }

    cam.Translate(translation);
    if (translation != glm::vec3(0, 0, 0)) {

    }
    

    glm::vec2 mouse_delta(cursor_state.delta_x, cursor_state.delta_y);
    if(mouse_delta.x != 0 || mouse_delta.y != 0) {
        float pitch = mouse_speed * dt * mouse_delta.y;
        float yaw = mouse_speed * dt * mouse_delta.x;
        cam.Pitch(pitch);
        cam.Yaw(-yaw);
    }
}

struct Transform {
    glm::quat rotation;
    glm::vec3 position;
};


ChunkedLoDTerrainRenderer* terrain_renderer;

void App::OnStart() {  
    /*glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);*/

 //   glEnable(GL_BLEND);
 //   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 //   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);

    terrain_renderer = new ChunkedLoDTerrainRenderer(renderDevice);

    cam.MoveTo(0, 0, 1000);
    cam.LookAt(0, 0, 0);

    ChunkedLoDTerrainDesc desc;
    desc.size = 10000;
    desc.x = 0;
    desc.y = 0;
    desc.heightmap_generator = [&](double x, double y, double z) -> double {
                        noise::module::RidgedMulti mountain;
                        mountain.SetSeed(32);
                        mountain.SetFrequency(0.05);
                        mountain.SetOctaveCount(8);
                        return mountain.GetValue(x * 0.01f, y * 0.01f, z * 0.01f);
                    };

    terrain_renderer->RegisterTerrain(desc);

}


void App::OnFrame(const app::AppState* app_state, float dt) {
    HandleInput(app_state->key_state, app_state->cursor_state, 1.f/60.f);

    glm::mat4 proj = cam.BuildProjection();
    glm::mat4 view = cam.BuildView();
    glm::mat4 world = glm::mat4();
    Frustum frustum(proj, view);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);

    terrain_renderer->Render(cam, frustum);

    accumulate += dt;
    ++frame_count;
    ++total_frame_count;

    if(accumulate > 1.0) {
        std::stringstream ss;
        ss << "gfx Device: " << renderDevice->DeviceConfig.DeviceAbbreviation;
        ss << " | FPS: " << frame_count << " | Frame: " << total_frame_count;
        ss << " | Pos: " << cam.pos;
        sys::SetWindowTitle(ss.str().c_str());
        frame_count = 0;
        accumulate = 0.0;
    }
}
void App::OnShutdown() {

}
