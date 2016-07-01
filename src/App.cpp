#include "RenderEngine.h"
#include "App.h"
#include "Camera.h"
#include "System.h"
#include "Helpers.h"
#include "noise/noise.h"
#include "Log.h"
#include "glm/glm.hpp"

#include "InputManager.h"
#include "UIManager.h"
#include "PlayerController.h"
#include "ConsoleUI.h"
#include "ChunkedTerrain.h"
#include "Spatial.h"
#include "Config.h"
#include "Simulation.h"
#include "Skybox.h"
#include <glm/gtx/transform.hpp>

uint32_t frame_count     = 0;
double accumulate        = 0;
double total_frame_count = 0;
Camera cam;
input::InputManager* inputManager;
controllers::PlayerController* playerController;
RenderEngine* renderEngine;
Simulation simulation;
RenderView* playerView;
Viewport* playerViewport;
ui::UIManager* uiManager;
ui::ConsoleUI* consoleUI;


SimObj* CreateSkybox() {
    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
    if (!fs::IsPathDirectory(assetDirPath)) {
        LOG_E("Invalid Directory Path given for AssetDirectory.");
    }
    SimObj* skyObj = simulation.AddSimObj();
    Skybox* sky    = skyObj->AddComponent<Skybox>(ComponentType::Skybox);

    sky->imagePaths[0] = assetDirPath + "/skybox/TropicalSunnyDayLeft2048.png";
    sky->imagePaths[1] = assetDirPath + "/skybox/TropicalSunnyDayRight2048.png";
    sky->imagePaths[2] = assetDirPath + "/skybox/TropicalSunnyDayUp2048.png";
    sky->imagePaths[3] = assetDirPath + "/skybox/TropicalSunnyDayDown2048.png";
    sky->imagePaths[4] = assetDirPath + "/skybox/TropicalSunnyDayFront2048.png";
    sky->imagePaths[5] = assetDirPath + "/skybox/TropicalSunnyDayBack2048.png";
    
    renderEngine->Register(skyObj, RendererType::Skybox);
    return skyObj;
}

SimObj* CreateTerrain(float size, const glm::mat4& rotation, const glm::mat4& translation) {
    SimObj* terrainChunk = simulation.AddSimObj();
    ChunkedTerrain* terrain = terrainChunk->AddComponent<ChunkedTerrain>(ComponentType::ChunkedTerrain);
    terrain->size = size;
    terrain->translation = translation;
    terrain->rotation = rotation;
    terrain->heightmapGenerator = [&](double x, double y, double z) -> double 
    {
        noise::module::RidgedMulti mountain;
        mountain.SetSeed(32);
        mountain.SetFrequency(0.05);
        mountain.SetOctaveCount(8);
        return mountain.GetValue(x * 0.01f, y * 0.01f, z * 0.01f);
    };

    Spatial* spatial = terrainChunk->AddComponent<Spatial>(ComponentType::Spatial);
    spatial->pos = glm::dvec3(0, 0, 0);
    terrain->renderObj = renderEngine->Register(terrainChunk, RendererType::ChunkedTerrain);
    assert(terrain->renderObj);

    return terrainChunk;
}

void SetupInputBindings() {

    // 'hardcoded' mouse x, y and click
    inputManager->AddAxisMapping("MousePosX", input::InputCode::INPUT_MOUSE_XAXIS, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MousePosY", input::InputCode::INPUT_MOUSE_YAXIS, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddActionMapping("MouseKey1", input::InputCode::INPUT_MOUSE_KEY1, input::InputManager::ActionConfig(false, false, false));

    // Console Trigger
    inputManager->AddActionMapping("ToggleConsole", input::InputCode::INPUT_KEY_BACKTICK, input::InputManager::ActionConfig(true, true, false));

    // Handle Key Mappings
    // Keyboard n mouse for player
    inputManager->AddAxisMapping("MoveForward", input::InputCode::INPUT_KEY_W, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveBackward", input::InputCode::INPUT_KEY_S, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("MoveRight", input::InputCode::INPUT_KEY_D, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveLeft", input::InputCode::INPUT_KEY_A, input::InputManager::AxisConfig(-1.0, 0));

    inputManager->AddAxisMapping("ShimmyUp", input::InputCode::INPUT_KEY_Z, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("ShimmyDown", input::InputCode::INPUT_KEY_X, input::InputManager::AxisConfig(-1.0, 0));

    /*inputManager->AddActionMapping("MoveSpeedSlow", input::InputCode::INPUT_KEY_3, input::InputManager::ActionConfig(true, true, false));
    inputManager->AddActionMapping("MoveSpeedNormal", input::InputCode::INPUT_KEY_2, input::InputManager::ActionConfig(true, true, false));
    inputManager->AddActionMapping("MoveSpeedFast", input::InputCode::INPUT_KEY_1, input::InputManager::ActionConfig(true, true, false));*/
    inputManager->AddActionMapping("ToggleWireFrameMode", input::InputCode::INPUT_KEY_4, input::InputManager::ActionConfig(true, true, false));
    inputManager->AddActionMapping("LookMode", input::InputCode::INPUT_MOUSE_KEY1, input::InputManager::ActionConfig(false, false, true));

    inputManager->AddAxisMapping("LookUp", input::InputCode::INPUT_MOUSE_YAXISRELATIVE, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("LookDown", input::InputCode::INPUT_MOUSE_YAXISRELATIVE, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("LookLeft", input::InputCode::INPUT_MOUSE_XAXISRELATIVE, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("LookRight", input::InputCode::INPUT_MOUSE_XAXISRELATIVE, input::InputManager::AxisConfig(-1.0, 0));

    // Controller
    inputManager->AddActionMapping("ToggleConsole", input::InputCode::INPUT_GAMEPAD_SELECT, input::InputManager::ActionConfig(true, true, false));
    inputManager->AddActionMapping("ToggleWireFrameMode", input::InputCode::INPUT_GAMEPAD_START, input::InputManager::ActionConfig(true, true, false));

    inputManager->AddAxisMapping("MoveForward", input::InputCode::INPUT_GAMEPAD_UP, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveBackward", input::InputCode::INPUT_GAMEPAD_DOWN, input::InputManager::AxisConfig(-1.0, 0));
    inputManager->AddAxisMapping("MoveRight", input::InputCode::INPUT_GAMEPAD_RIGHT, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MoveLeft", input::InputCode::INPUT_GAMEPAD_LEFT, input::InputManager::AxisConfig(-1.0, 0));

    inputManager->AddAxisMapping("LookUp", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(-1.0, 0.15f));
    inputManager->AddAxisMapping("LookDown", input::InputCode::INPUT_GAMEPAD_RSTICKY, input::InputManager::AxisConfig(-1.0, 0.15f));
    inputManager->AddAxisMapping("LookLeft", input::InputCode::INPUT_GAMEPAD_RSTICKX,
                                 input::InputManager::AxisConfig(-1.0, 0.15f));
    inputManager->AddAxisMapping("LookRight", input::InputCode::INPUT_GAMEPAD_RSTICKX,
                                 input::InputManager::AxisConfig(-1.0, 0.15f));

    inputManager->AddAxisMapping("ShimmyUp", input::InputCode::INPUT_GAMEPAD_LSHOULDER,
                                 input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("ShimmyDown", input::InputCode::INPUT_GAMEPAD_RSHOULDER,
                                 input::InputManager::AxisConfig(-1.0, 0));

    // Create context and controllers
    input::InputContext* inputContext =
        inputManager->CreateNewContext(input::InputManager::ContextPriority::CONTEXT_PLAYER);
    playerController = new controllers::PlayerController(&cam, inputContext);

    // ----- EUGENE KILLED THIS DREAM
    // kappa
    // hook up wireframe toggle
    // inputContext->BindContext<input::ContextBindingType::Action>(
    //   "ToggleWireFrameMode", BIND_MEM_CB(&ChunkedLoDTerrainRenderer::ToggleWireFrameMode, terrain_renderer));
}

void SetupUI(gfx::RenderDevice* renderDevice, Viewport* viewport) {
    input::InputContext* uiContext = inputManager->CreateNewContext(input::InputManager::ContextPriority::CONTEXT_MENU);
    uiManager                      = new ui::UIManager(inputManager->GetKeyboardManager(), uiContext, *viewport);

    // TODO: Should use renderEngine interface instead of directly accessing renderer but we need to figure out object
    // model first
    uiManager->SetUIRenderer(static_cast<UIRenderer*>(renderEngine->GetRenderer(RendererType::Ui)));

    // Hard code consoleFrame for now, meh...stuff like this could be lua/xml, eventually
    consoleUI = new ui::ConsoleUI(uiManager, uiContext);
}

void App::OnStart() {
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    playerViewport                = new Viewport();
    playerViewport->width         = static_cast<float>(windowSize.width);
    playerViewport->height        = static_cast<float>(windowSize.height);
    playerView                    = new RenderView(&cam, playerViewport);
    renderEngine                  = new RenderEngine(renderDevice, playerView);
    inputManager = new input::InputManager();
    SetupUI(renderDevice, playerViewport);

    SetupInputBindings();

    // cam.MoveTo(-2826, 1620, 1600);
    cam.MoveTo(0, 0, 1000);
    cam.LookAt(0, 0, 0);

    // gogoogog planet
    glm::dvec3 origin(0, 0, 0);
    glm::mat4 rotate;
    glm::mat4 translate;
    float diamater = 2500;
    float radius   = diamater / 2.f;

    // translate = glm::translate(glm::vec3(origin.x, origin.y, origin.z + radius));
    // CreateTerrain(diamater, glm::mat4(), translate); // front

    // translate = glm::translate(glm::vec3(origin.x, origin.y + radius, origin.z));
    // rotate = glm::rotate(glm::mat4(), -3.1415f / 2.f, glm::normalize(glm::vec3(1, 0, 0)));
    // CreateTerrain(diamater, rotate, translate); // top

    CreateSkybox();

    // rotate = glm::rotate(glm::mat4(), 3.1415f/2.f, glm::normalize(glm::vec3(1, 0, 0)));
    // CreateTerrain(diamater, rotate, glm::mat4()); // bottom

    // rotate = glm::rotate(glm::mat4(), 3.1415f/2.f, glm::normalize(glm::vec3(0, 1, 0)));
    // CreateTerrain(diamater, rotate, glm::mat4()); // right

    // rotate = glm::rotate(glm::mat4(), -3.1415f/2.f, glm::normalize(glm::vec3(0, 1, 0)));
    // CreateTerrain(diamater, rotate, glm::mat4()); // left

    // rotate = glm::rotate(glm::mat4(), -3.1415f, glm::normalize(glm::vec3(0, 1, 0)));
    // CreateTerrain(diamater, rotate, glm::mat4()); // back
}

void App::OnFrame(const std::vector<float>& inputValues, float dt) {
    // TODO:: Maybe have system pump events instead of polling?
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    if (   windowSize.width != static_cast<uint32_t>(playerViewport->width)
        || windowSize.height != static_cast<uint32_t>(playerViewport->height)) {

        playerViewport->width  = static_cast<float>(windowSize.width);
        playerViewport->height = static_cast<float>(windowSize.height);

        uiManager->UpdateViewport(*playerViewport);
    }

    // input
    inputManager->ProcessInputs(inputValues, dt * 1000);
    sys::ShowCursor(inputManager->ShouldShowCursor());

    // update
    playerController->DoUpdate(dt);
    uiManager->DoUpdate(dt * 1000);
    simulation.Update(dt);

    // render
    renderEngine->RenderFrame();

    // timers
    accumulate += dt;
    ++frame_count;
    ++total_frame_count;

    if (accumulate > 1.0) {
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
