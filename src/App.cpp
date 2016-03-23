#include "App.h"
#include "Camera.h"
#include "System.h"
#include "Helpers.h"
#include "noise/noise.h"
#include "Frustum.h"
#include "Log.h"
#include "ChunkedLoDTerrainRenderer.h"
#include "TextRenderer.h"
#include "SkyboxRenderer.h"

#include "glm/glm.hpp"

#include "InputManager.h"
#include "UIManager.h"
#include "PlayerController.h"
#include "ConsoleUI.h"
#include "KeyboardManager.h"

uint32_t frame_count = 0;
double accumulate = 0;
double total_frame_count = 0;
Camera cam;
input::InputManager* inputManager;
controllers::PlayerController* playerController;
ChunkedLoDTerrainRenderer* terrain_renderer;
SkyboxRenderer* skybox_renderer;
ui::UIManager* uiManager;
ui::ConsoleUI* consoleUI;
TextRenderer* text_renderer;
uint32_t windowWidth = 0, windowHeight = 0;

void SetupInput(uint32_t width, uint32_t height) {
    inputManager = new input::InputManager();

	// 'hardcoded' mouse x, y and click
	inputManager->AddAxisMapping("MousePosX", input::InputCode::INPUT_MOUSE_XAXIS, input::InputManager::AxisConfig(1.0, 0));
	inputManager->AddAxisMapping("MousePosY", input::InputCode::INPUT_MOUSE_YAXIS, input::InputManager::AxisConfig(1.0, 0));
	inputManager->AddActionMapping("MouseKey1", input::InputCode::INPUT_MOUSE_KEY1, input::InputManager::ActionConfig(false));

    // Console Trigger
    inputManager->AddActionMapping("ToggleConsole", input::InputCode::INPUT_KEY_BACKTICK, input::InputManager::ActionConfig(true));

    // Handle Key Mappings
    // Keyboard n mouse for player
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

    inputManager->AddAxisMapping("LookUp", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(1.0, 0.15f));
    inputManager->AddAxisMapping("LookDown", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(1.0, 0.15f));
    inputManager->AddAxisMapping("LookLeft", input::InputCode::INPUT_GAMEPAD_RSTICKX, input::InputManager::AxisConfig(1.0, 0.15f));
    inputManager->AddAxisMapping("LookRight", input::InputCode::INPUT_GAMEPAD_RSTICKX, input::InputManager::AxisConfig(1.0, 0.15f));

    // Create context and controllers
    input::InputContext* inputContext = inputManager->CreateNewContext(input::InputManager::ContextPriority::CONTEXT_PLAYER);
    playerController = new controllers::PlayerController(&cam, inputContext);
}

void SetupUI(graphics::RenderDevice* renderDevice, uint32_t width, uint32_t height) {
    input::InputContext* uiContext = inputManager->CreateNewContext(input::InputManager::ContextPriority::CONTEXT_MENU);
    uiManager = new ui::UIManager(inputManager->GetKeyboardManager(), uiContext, renderDevice, width, height);
    // Hard code consoleFrame for now, meh...stuff like this could be lua/xml, eventually
    consoleUI = new ui::ConsoleUI(uiManager, uiContext);
}

void App::OnStart() {
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    windowWidth = windowSize.width;
    windowHeight = windowSize.height;

    SetupInput(windowWidth, windowHeight);
    SetupUI(renderDevice, windowWidth, windowHeight);

    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);

    terrain_renderer = new ChunkedLoDTerrainRenderer(renderDevice);
    text_renderer = new TextRenderer(renderDevice);
    uiManager->SetTextRenderer(text_renderer);
    skybox_renderer = new SkyboxRenderer(renderDevice);

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
    inputManager->ProcessInputs(inputValues, dt * 1000 );
    playerController->DoUpdate(dt);

    glm::mat4 proj = cam.BuildProjection();
    glm::mat4 view = cam.BuildView();
    glm::mat4 world = glm::mat4();
    Frustum frustum(proj, view);

    renderDevice->Clear(0.1f, 0.1f, 0.1f, 0.1f);
    skybox_renderer->OnSubmit(&cam);
    terrain_renderer->Render(cam, frustum);

// TODO:: On Opengl ui frames flicker -- fix state management
    uiManager->DoUpdate(dt * 1000);
    text_renderer->RenderText("asdfasdfasdsaasdf",0,0, 1.f, glm::vec3(1,0,0));
    text_renderer->RenderCursor("asdfasdfasdsaasdf", 4, 0, 0, 1.f, glm::vec3(1, 1, 1));
    
  
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
