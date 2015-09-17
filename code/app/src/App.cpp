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
//#include "TextRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <noise/noise.h>

#include <functional>
#include <queue>
#include "input/InputManager.h"
#include "input/PlayerController.h"

uint32_t frame_count = 0;
double accumulate = 0;
double total_frame_count = 0;
Camera cam;
input::InputManager* inputManager;
controllers::PlayerController* playerController;
float mouse_speed = 1.f;
float walk_speed = 300.f;
bool z_toggle = false;
bool z_pressed = false;

void SetupInput() {
    inputManager = new input::InputManager();
    // Handle Key Mappings
    // Keyboard n mouse
    inputManager->AddAxisMapping("MoveForward", input::InputCode::INPUT_KEY_W, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveBackward", input::InputCode::INPUT_KEY_S, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("MoveRight", input::InputCode::INPUT_KEY_D, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveLeft", input::InputCode::INPUT_KEY_A, input::InputManager::AxisConfig(-1.0, 0));

    /*inputManager->AddActionMapping("MoveSpeedSlow", input::InputCode::INPUT_KEY_3, input::InputManager::ActionConfig(true));
    inputManager->AddActionMapping("MoveSpeedNormal", input::InputCode::INPUT_KEY_2, input::InputManager::ActionConfig(true));
    inputManager->AddActionMapping("MoveSpeedFast", input::InputCode::INPUT_KEY_1, input::InputManager::ActionConfig(true));*/
    inputManager->AddActionMapping("LookMode", input::InputCode::INPUT_MOUSE_KEY1, input::InputManager::ActionConfig(false));

    inputManager->AddAxisMapping("LookUp", input::InputCode::INPUT_MOUSE_YAXISRELATIVE, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("LookDown", input::InputCode::INPUT_MOUSE_YAXISRELATIVE, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("LookLeft", input::InputCode::INPUT_MOUSE_XAXISRELATIVE, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("LookRight", input::InputCode::INPUT_MOUSE_XAXISRELATIVE, input::InputManager::AxisConfig(1.0, 0));

    // Controller
    inputManager->AddAxisMapping("MoveForward", input::InputCode::INPUT_GAMEPAD_UP, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveBackwards", input::InputCode::INPUT_GAMEPAD_DOWN, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("MoveRight", input::InputCode::INPUT_GAMEPAD_RIGHT, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveLeft", input::InputCode::INPUT_GAMEPAD_LEFT, input::InputManager::AxisConfig(-1.0, 0));

    inputManager->AddAxisMapping("LookUp", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(1.0, 0.15));
    inputManager->AddAxisMapping("LookDown", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(1.0, 0.15));
    inputManager->AddAxisMapping("LookLeft", input::InputCode::INPUT_GAMEPAD_RSTICKX, input::InputManager::AxisConfig(1.0, 0.15));
    inputManager->AddAxisMapping("LookRight", input::InputCode::INPUT_GAMEPAD_RSTICKX, input::InputManager::AxisConfig(1.0, 0.15));

    // Create context and controllers
    input::InputContext* inputContext = inputManager->CreateNewContext(input::InputManager::ContextPriority::CONTEXT_PLAYER);
    playerController = new controllers::PlayerController(&cam, inputContext);
}

ChunkedLoDTerrainRenderer* terrain_renderer;

void App::OnStart() {  
    SetupInput();

    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);

    terrain_renderer = new ChunkedLoDTerrainRenderer(renderDevice);
    //text_renderer = new TextRenderer();


    cam.MoveTo(0, 0, 1000);
    cam.LookAt(0, 0, 0);

    ChunkedLoDTerrainDesc desc;
    desc.size = 5000;
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

void App::OnFrame(const std::vector<float>& inputValues, float dt) {
    inputManager->ProcessInputs(inputValues, 1.f / 60.f);
    playerController->DoUpdate(1.f / 60.f);

    glm::mat4 proj = cam.BuildProjection();
    glm::mat4 view = cam.BuildView();
    glm::mat4 world = glm::mat4();
    Frustum frustum(proj, view);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);

    terrain_renderer->Render(cam, frustum);
    //text_renderer->RenderText("asdfasdfasdsaasdf",0,0, 1.f, glm::vec3(1,0,0));
  
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
