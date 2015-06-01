#include "App.h"
#include "GLHelpers.h"
#include <glm/glm.hpp>
#include "Camera.h"
#include <vector>
#include "System.h"
#include <sstream>
#include "TerrainRenderer.h"
#include "TerrainRenderer2.h"
#include "DebugRenderer.h"
#include "Frustum.h"


uint32_t frame_count = 0;
double curr_frame_time = 0;
double prev_frame_time = 0;
double accumulate = 0;
double total_frame_count = 0;
double frame_time = 0;
Camera cam;
glm::mat4 world;



void HandleInput(const app::KeyState& key_state, const app::CursorState& cursor_state, float dt) {
    float mouse_speed = 1.f;
    float walk_speed = 300.f;

    glm::vec3 translation(0, 0, 0);
    if (key_state.is_w_pressed) {
        translation.z += walk_speed * dt;
    }

    if (key_state.is_a_pressed) {
        translation.x -= walk_speed * dt;
    }

    if (key_state.is_s_pressed) {
        translation.z -= walk_speed * dt;
    }

    if (key_state.is_d_pressed) {
        translation.x += walk_speed * dt;
    }

    if (key_state.is_q_pressed) {
        translation.y += walk_speed * dt;
    }

    if (key_state.is_e_pressed) {
        translation.y -= walk_speed * dt;
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

TerrainRenderer2* tr;
DebugRenderer* debug_renderer;
Frustum frustum;


void App::OnStart() {    
    fprintf(stdout, "%s\n", glGetString(GL_VERSION));
    fprintf(stdout, "%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(stdout, "%s\n", glGetString(GL_VENDOR));
    fprintf(stdout, "%s\n", glGetString(GL_RENDERER));
//    glEnable (GL_DEPTH_TEST);
    
    glEnable(GL_CULL_FACE);
       glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    tr = new TerrainRenderer2();
    debug_renderer = DebugRenderer::GetInstance();

    
    cam.MoveTo(glm::vec3(0, 0, 1000));
    cam.LookAt(glm::vec3(0, 0, 0));

    frustum.ComputeFrustumPlanes(cam.GetProjection(), cam.GetView());
    
   
    for(float i = -1000; i < 1000; i += 50) {
        for(float j = -1000; j < 1000; j += 50) {       
            for(float k = -1000; k < 1000; k += 50) {       
                if(frustum.ContainsPoint(glm::vec3(i, k, j))) {
                    //debug_renderer->DrawPoint(glm::vec3(i, k, j));    
                }     
            }           
        }
    }
    

    

}

void App::OnFrame(const app::AppState* app_state, float dt) {    
    HandleInput(app_state->key_state, app_state->cursor_state, 1.f/60.f);
    
    tr->Update(cam.GetPos());
   
     

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4 m = cam.GetProjection() * cam.GetView();    
    frustum.ComputeFrustumPlanes(cam.GetProjection(), cam.GetView());
    tr->Render(cam);
    debug_renderer->Render(m);
    

    
    accumulate += dt;
    ++frame_count;
    ++total_frame_count;    

    if(accumulate > 1.0) {
        std::stringstream ss;
        ss << "gfx | FPS: " << frame_count << " | Frame: " << total_frame_count;
        sys::SetWindowTitle(ss.str().c_str());
        frame_count = 0;
        accumulate = 0.0;
    }
}
void App::OnShutdown() {

}
