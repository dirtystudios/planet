#include "App.h"
#include "Camera.h"
#include "Helpers.h"
#include "Log.h"
#include "RenderEngine.h"
#include "System.h"
#include "glm/glm.hpp"
#include "noise/noise.h"

#include <glm/gtx/transform.hpp>
#include "ChunkedTerrain.h"
#include "Config.h"
#include "ConsoleUI.h"
#include "DebugUI.h"
#include "FlatTerrain.h"
#include "InputManager.h"
#include "LabelUI.h"
#include "PlayerController.h"
#include "Simulation.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"
#include "Skybox.h"
#include "Spatial.h"
#include "TaskScheduler.h"
#include "UI.h"
#include "UIManager.h"
#include "MeshGeneration.h"

uint32_t frame_count = 0;
double taccumulate = 0;
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
ui::DebugUI* debugUI;
std::unique_ptr<FlatTerrain> terrain;
std::unique_ptr<MeshRenderObj> meshRO;

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
    uiManager                      = new ui::UIManager(inputManager->GetKeyboardManager(), uiContext, inputManager->GetDebugContext(), *viewport);

    SimObj*  worldFrame = simulation.AddSimObj();
    UI*      ui         = worldFrame->AddComponent<UI>(ComponentType::UI);
    ui->isWorldFrame = true;
    Spatial* spatial    = worldFrame->AddComponent<Spatial>(ComponentType::Spatial);
    spatial->pos        = glm::vec3(0.f, 0.f, 0.f);
    spatial->direction  = glm::vec3(0.f, 0.f, 0.f);

    uiManager->SetUIRenderer(renderEngine->Renderers().ui.get());
    uiManager->SetTextRenderer(renderEngine->Renderers().text.get());
    uiManager->SetDebugRenderer(renderEngine->debugDraw());

    // todo: make it so consoleUI reference doesnt have to persist
    consoleUI = new ui::ConsoleUI(ui, uiContext);
    debugUI = new ui::DebugUI(ui);

    // Show/Hide sample text test with this call
    ui::LabelUI::AttachLabel(ui, "hey look im a label");

    uiManager->AddFrameObj(worldFrame);
}

void AddWorldText() {
    // Attempt to add 3d world space text
    SimObj* worldText = simulation.AddSimObj();
    UI* ui = worldText->AddComponent<UI>(ComponentType::UI);
    Spatial* spatial = worldText->AddComponent<Spatial>(ComponentType::Spatial);
    spatial->pos = glm::vec3(100.f, 0.f, 0.f);
    spatial->direction = glm::vec3(0.f, 0.f, 0.f);

    ui::LabelUI::AttachLabel(ui, "Roxas");

    uiManager->AddFrameObj(worldText);
}

void App::OnStart() {
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    playerViewport                = new Viewport();
    playerViewport->width         = static_cast<float>(windowSize.width);
    playerViewport->height        = static_cast<float>(windowSize.height);
    playerView                    = new RenderView(&cam, playerViewport);
    renderEngine                  = new RenderEngine(renderDevice, playerView);
    inputManager                  = new input::InputManager();
    SetupUI(renderDevice, playerViewport);
    AddWorldText();

    SetupInputBindings();

    // cam.MoveTo(-2826, 1620, 1600);
    cam.MoveTo(0, 0, 2000);
    cam.LookAt(0, 0, 0);

    SkyboxRenderObj* skybox = CreateSkybox();
    renderEngine->Renderers().sky->Register(skybox);

    terrain.reset(new FlatTerrain(10000));
    renderEngine->Renderers().terrain->Register(terrain.get());
    
//    // whoops, this doesnt get deleted
//    MeshPtr mesh = renderEngine->meshCache()->Create("sponza", "sponza/sponza.obj");
//    MaterialPtr material = renderEngine->materialCache()->Create("sponza", "sponza/sponza.obj");
//    
//    MeshRenderObj* renderObj = new MeshRenderObj(mesh, material);
//    renderEngine->Renderers().mesh->Register(renderObj);
    
    MaterialData materialData;
    MeshGeometryData geometryData;
    
    dgen::GenerateIcoSphere(3, &geometryData);
    
    materialData.kd = glm::vec3(1, 0, 0);
    materialData.shadingModel = ShadingModel::Blinn;
    
    MeshPtr sphereMesh = renderEngine->meshCache()->Create("ico_sphere_3", std::move(geometryData));
    MaterialPtr redMaterial = renderEngine->materialCache()->Create("red_blinn", std::move(materialData));
    
    meshRO.reset(new MeshRenderObj(sphereMesh, redMaterial));
    meshRO->transform()->scale(50);

    renderEngine->Renderers().mesh->Register(meshRO.get());
}

void App::OnFrame(const std::vector<float>& inputValues, float dt) {
    // TODO:: Maybe have system pump events instead of polling?
    sys::SysWindowSize windowSize = sys::GetWindowSize();
    if (windowSize.width != static_cast<uint32_t>(playerViewport->width) || windowSize.height != static_cast<uint32_t>(playerViewport->height)) {

        playerViewport->width = static_cast<float>(windowSize.width);
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
    
    meshRO->transform()->reset();
    meshRO->transform()->scale(50.f + 10.f * cos(total_frame_count / 60.f));
    
    // render
    RenderScene scene;
    //scene.renderObjects.push_back(meshRO.get());
    
    renderEngine->RenderFrame(&scene);

    // timers
    taccumulate += dt;
    ++frame_count;
    ++total_frame_count;

    if (taccumulate > 1.0) {
        debugUI->AddKeyValue("FPS", std::to_string(frame_count));
        debugUI->AddKeyValue("DrawCalls", std::to_string(renderDevice->DrawCallCount()));
        debugUI->AddKeyValue("U", ToString(cam.up));
        debugUI->AddKeyValue("L", ToString(cam.look));
        debugUI->AddKeyValue("R", ToString(cam.right));

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
