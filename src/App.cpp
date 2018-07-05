#include "App.h"
#include "Camera.h"
#include "Helpers.h"
#include "Log.h"
#include "RenderEngine.h"
#include "System.h"
#include "glm/glm.hpp"
#include "noise/noise.h"

#include <glm/gtx/transform.hpp>
#include "Config.h"
#include "ConsoleUI.h"
#include "DebugUI.h"
#include "FlatTerrain.h"
#include "InputManager.h"
#include "LabelUI.h"
#include "PlayerController.h"
#include "SimulationManager.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"
#include "Spatial.h"
#include "TaskScheduler.h"
#include "UI.h"
#include "UIManager.h"
#include "MeshGeneration.h"
#include "AnimationManager.h"
#include "SkinnedMesh.h"
#include "AnimationComponent.h"
#include "ComponentManager.h"

uint32_t frame_count = 0;
double taccumulate = 0;
double total_frame_count = 0;
Camera cam;
input::InputManager* inputManager;
controllers::PlayerController* playerController;
RenderEngine* renderEngine;
RenderView* playerView;
Viewport* playerViewport;

SimulationManager simulationManager;

ui::ConsoleUI* consoleUI;
ui::DebugUI* debugUI;
std::unique_ptr<FlatTerrain> terrain;
SkyboxRenderObj* skybox { nullptr };

SkyboxRenderObj* CreateSkybox() {
    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
    if (!fs::IsPathDirectory(assetDirPath)) {
        LOG_E("Invalid Directory Path given for AssetDirectory.");
    }

    std::array<std::string, 6> imagePaths;

    imagePaths[0] = assetDirPath + "/skybox/TropicalSunnyDayLeft2048.png";
    imagePaths[1] = assetDirPath + "/skybox/TropicalSunnyDayRight2048.png";
    imagePaths[2] = assetDirPath + "/skybox/TropicalSunnyDayUp2048.png";
    imagePaths[3] = assetDirPath + "/skybox/TropicalSunnyDayDown2048.png";
    imagePaths[4] = assetDirPath + "/skybox/TropicalSunnyDayFront2048.png";
    imagePaths[5] = assetDirPath + "/skybox/TropicalSunnyDayBack2048.png";

    return new SkyboxRenderObj(imagePaths);
}

void SetupInputBindings() {

    // 'hardcoded' mouse x, y and click
    inputManager->AddAxisMapping("MousePosX", input::InputCode::INPUT_MOUSE_XAXIS, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddAxisMapping("MousePosY", input::InputCode::INPUT_MOUSE_YAXIS, input::InputManager::AxisConfig(1.0, 0));
    inputManager->AddActionMapping("MouseKey1", input::InputCode::INPUT_MOUSE_KEY1, input::InputManager::ActionConfig(false, false, false));
    inputManager->AddActionMapping("MouseKey2", input::InputCode::INPUT_MOUSE_KEY2, input::InputManager::ActionConfig(false, false, false));

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
    inputManager->AddActionMapping("LookMode", input::InputCode::INPUT_MOUSE_KEY2, input::InputManager::ActionConfig(false, false, true));

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

    SimObj* worldFrame  = simulationManager.CreateSimObj();
    UI*      ui         = worldFrame->AddComponent<UI>();
    ui->isWorldFrame = true;
    Spatial* spatial    = worldFrame->AddComponent<Spatial>();
    spatial->pos        = glm::vec3(0.f, 0.f, 0.f);
    spatial->direction  = glm::vec3(0.f, 0.f, 0.f);

    // todo: make it so consoleUI reference doesnt have to persist
    consoleUI = new ui::ConsoleUI(ui, uiContext);
//    debugUI = new ui::DebugUI(ui);

    // Show/Hide sample text test with this call
//    ui::LabelUI::AttachLabel(ui, "hey look im a label");

    simulationManager.RegisterManager<ui::UIManager>({ ComponentType::UI, ComponentType::Spatial }, inputManager->GetKeyboardManager(), uiContext, inputManager->GetDebugContext(), *viewport,
        renderEngine->Renderers().text.get(), renderEngine->Renderers().ui.get(), renderEngine->debugDraw());
}

void AddWorldText() {
    // Attempt to add 3d world space text
    SimObj* worldText = simulationManager.CreateSimObj();
    UI* ui = worldText->AddComponent<UI>();
    Spatial* spatial = worldText->AddComponent<Spatial>();
    spatial->pos = glm::vec3(0, 0.f, 0.f);
    spatial->direction = glm::vec3(0.f, 0.f, 0.f);    

    ui::LabelUI::AttachLabel(ui, "A S G");
}

void AddArthas() {
    SimObj* arthas = simulationManager.CreateSimObj();
    SkinnedMesh* mesh = arthas->AddComponent<SkinnedMesh>();
    mesh->mesh = renderEngine->meshCache()->Create("arthas", "arthas/arthas.fbx");
    mesh->mat = renderEngine->materialCache()->Create("arthas", "arthas/arthas.fbx");

    AnimationComponent* anim = arthas->AddComponent<AnimationComponent>();
    anim->animData = renderEngine->animationCache()->Create("arthas", "arthas/arthas.fbx")->animData.begin()->second;
}

void AddRoxas() {
    SimObj* roxas = simulationManager.CreateSimObj();
    SkinnedMesh* mesh = roxas->AddComponent<SkinnedMesh>();
    mesh->mesh = renderEngine->meshCache()->Create("roxasps3", "roxasps3/roxasps3.fbx");
    mesh->mat = renderEngine->materialCache()->Create("roxasps3", "roxasps3/roxasps3.fbx");
    mesh->scale = 0.05f;

    AnimationComponent* anim = roxas->AddComponent<AnimationComponent>();
    anim->animData = renderEngine->animationCache()->Create("roxasps3", "roxasps3/roxasps3.fbx")->animData.begin()->second;
}

void App::OnStart() {
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    playerViewport                = new Viewport();
    playerViewport->width         = static_cast<float>(windowSize.width);
    playerViewport->height        = static_cast<float>(windowSize.height);
    playerView                    = new RenderView(&cam, playerViewport);
    renderEngine                  = new RenderEngine(renderDevice, swapchain, playerView);
    inputManager                  = new input::InputManager();
    
    SetupUI(renderDevice, playerViewport);
    AddWorldText();

    SetupInputBindings();

    // cam.MoveTo(-2826, 1620, 1600);
    cam.MoveTo(0, 0, 221);
    cam.LookAt(0, 0, 0);

//    skybox = CreateSkybox();
//    renderEngine->Renderers().sky->Register(skybox);

    terrain.reset(new FlatTerrain(10000));
    renderEngine->Renderers().terrain->Register(terrain.get());
    
    //AddArthas();
//    AddRoxas();

    simulationManager.RegisterManager<AnimationManager>({ ComponentType::SkinnedMesh, ComponentType::Animation }, renderEngine->Renderers().mesh.get());
}

void App::OnFrame(const std::vector<float>& inputValues, float dt) {
    // TODO:: Maybe have system pump events instead of polling?
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    if (windowSize.width != static_cast<uint32_t>(playerViewport->width) || windowSize.height != static_cast<uint32_t>(playerViewport->height)) {

        playerViewport->width = static_cast<float>(windowSize.width);
        playerViewport->height = static_cast<float>(windowSize.height);

        simulationManager.UpdateViewport(*playerViewport);
    }

    // input
    inputManager->ProcessInputs(inputValues, dt * 1000);
    sys::ShowCursor(inputManager->ShouldShowCursor());

    // update
    playerController->DoUpdate(dt);
//    float x = 500 * sin(sys::GetTime());
//    float y = 500 * cos(sys::GetTime());
//    float z = 0;
//    float x2 = 1000 * sin(sys::GetTime());
//    float y2 = 1000 * cos(sys::GetTime());
//    float z2 = 1000 * cos(sys::GetTime()) * sin(sys::GetTime());
//    cam.MoveTo(x, y, 2000 + z);
//    cam.LookAt(x2, y2, z2);

    simulationManager.DoUpdate(dt * 1000);

    // render
    // todo: link skinnedmesh's somehow to this correctly
    RenderScene scene;
//    scene.renderObjects.push_back(terrain.get());
    if (skybox) {
//        scene.renderObjects.push_back(skybox);
    }
    
    
    renderEngine->RenderFrame(&scene);
    
    // timers
    taccumulate += dt;
    ++frame_count;
    ++total_frame_count;

    if (taccumulate > 1.0) {
//        debugUI->AddKeyValue("FPS", std::to_string(frame_count));
////        debugUI->AddKeyValue("DrawCalls", std::to_string(renderDevice->DrawCallCount()));
//        debugUI->AddKeyValue("U", ToString(cam.up));
//        debugUI->AddKeyValue("L", ToString(cam.look));
//        debugUI->AddKeyValue("R", ToString(cam.right));

        std::stringstream ss;
        ss << "gfx Device: " << renderDevice->DeviceConfig.DeviceAbbreviation;
        ss << " | FPS: " << frame_count << " | Frame: " << total_frame_count;
        ss << " | Pos: " << cam.pos;
        sys::SetWindowTitle(ss.str().c_str());
        frame_count = 0;
        taccumulate = 0.0;
    }
}

void App::OnShutdown() {}

void App::OnWindowResize(uint32_t width, uint32_t height)
{
    renderEngine->CreateRenderTargets();
}
