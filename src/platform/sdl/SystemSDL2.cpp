#include "System.h"

#ifdef _WIN32
#include "SDL.h"
#include "SDL_syswm.h"
#include <GL/glew.h>
#include "DX11RenderDevice.h"
#else
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#endif

#include "GLRenderDevice.h"

#include "Log.h"
#include "Config.h"
#include <map>

SDL_Window* _window = NULL;
SDL_Event _e;
uint32_t _window_width = 800;
uint32_t _window_height = 600;
const char* _window_title = "dirty";
app::Application* _app = { nullptr };
bool shouldQuit = false;
std::vector<float> inputValues((int)input::InputCode::COUNT);
std::map<int, input::InputCode> sdlkMapping;

int filterSDLEvents(void* userdata, SDL_Event* event){
    //what im going to do here is just weed out any key's we dont want
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym){
        case SDLK_a:
        case SDLK_b:
        case SDLK_c:
        case SDLK_d:
        case SDLK_e:
        case SDLK_f:
        case SDLK_g:
        case SDLK_h:
        case SDLK_i:
        case SDLK_j:
        case SDLK_k:
        case SDLK_l:
        case SDLK_m:
        case SDLK_n:
        case SDLK_o:
        case SDLK_p:
        case SDLK_q:
        case SDLK_r:
        case SDLK_s:
        case SDLK_t:
        case SDLK_u:
        case SDLK_v:
        case SDLK_w:
        case SDLK_x:
        case SDLK_y:
        case SDLK_z:
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_ESCAPE:
        case SDLK_BACKQUOTE:
            return 1;
            break;
        default:
            return 0;
        }
    }
    //keep any other events...for now
    return 1;
}

#define MAP_KEY_SDL(x, y) sdlkMapping.insert(std::make_pair(SDLK_##x, input::InputCode::INPUT_KEY_##y));
void PopulateKeyMapping() {
    for (int x = 0; x < (int)input::InputCode::INPUT_KEY_Z; ++x) {
        sdlkMapping.insert(std::make_pair(SDLK_a + x, (input::InputCode)x));
    }

    for (int x = (int)input::InputCode::INPUT_KEY_0; x < (int)input::InputCode::INPUT_KEY_9; ++x) {
        sdlkMapping.insert(std::make_pair(SDLK_0 + x, (input::InputCode)x));
    }

    MAP_KEY_SDL(LSHIFT, LEFT_SHIFT);
    MAP_KEY_SDL(RSHIFT, RIGHT_SHIFT);
    MAP_KEY_SDL(BACKQUOTE, TILDE);
    MAP_KEY_SDL(BACKSPACE, BACKSPACE);
}

input::InputCode GetKeyCodeFromSDLKey(int glfw_key) {
    auto it = sdlkMapping.find(glfw_key);
    return it == sdlkMapping.end() ? input::InputCode::KEY_UNKNOWN : it->second;
}

void sys::SetWindowTitle(const char* title) {
    SDL_SetWindowTitle(_window, title);
}

float sys::GetTime() {
    //return seconds?
    return (float)(SDL_GetTicks()) / 1000.0;
}
int sys::Run(app::Application* app){
    _app = app;

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0){
        LOG_E("SDL Init Failed: %s", SDL_GetError());
        return -1;
    }

    std::string renderDeviceConfig =  config::Config::getInstance().GetConfigString("RenderDeviceSettings", "RenderDevice");
    if (renderDeviceConfig != "directx11" && renderDeviceConfig != "opengl") {
        renderDeviceConfig = "";
        LOG_D("Invalid RenderDevice set in ini. Using Default for system. Given: %s", renderDeviceConfig);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _window = SDL_CreateWindow(_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_width, _window_height, SDL_WINDOW_OPENGL);

    if (_window == NULL){
        LOG_E("SDL could not create window: %s", SDL_GetError());
        return -1;
    }

    SDL_GLContext glcontext = SDL_GL_CreateContext(_window);
    printf("%s\n", SDL_GetError() );
    SDL_GL_MakeCurrent(_window, glcontext);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    const char *subsystem = "Unknown System!";
    int initRtn = 0;
    void* windowHandle = 0;
    if (SDL_GetWindowWMInfo(_window, &info)){
        switch (info.subsystem){
        case SDL_SYSWM_UNKNOWN: break;
        case SDL_SYSWM_X11: subsystem = "X Window System"; break;
        case SDL_SYSWM_WINDOWS:
            subsystem = "Microsoft Windows";
#ifdef _WIN32
            if (renderDeviceConfig == "opengl") {
                // start GLEW extension handler
                glewExperimental = GL_TRUE;
                glewInit();
                glGetError();
                _app->renderDevice = new graphics::RenderDeviceGL();
            }
            else {
                _app->renderDevice = new graphics::RenderDeviceDX11();
                _app->renderDevice->InitializeDevice(info.info.win.window, _window_height, _window_width);
            }
#endif
            break;
        case SDL_SYSWM_COCOA: 
            subsystem = "Apple OS X"; 
            _app->renderDevice = new graphics::RenderDeviceGL();
            break;
        //case SDL_SYSWM_ANDROID: subsystem = "Android"; break;
        }
    }
    else {
        LOG_E("Couldn't get window information: %s\n", SDL_GetError());
    }
	
    SDL_SetEventFilter(filterSDLEvents, NULL);
    PopulateKeyMapping();

    LOG_D("SDL Initialized. Version: %d.%d.%d on %s", info.version.major, info.version.minor, info.version.patch, subsystem);

    if (initRtn != 0){
        LOG_E("%s", "DeviceRender Init Failed.");
        return -1;
    }

    _app->renderDevice->PrintDisplayAdapterInfo();

    int numJoysticks = SDL_NumJoysticks();
    LOG_D("SDL: Num Joysticks Connected : %d", numJoysticks);

    for (int x = 0; x < numJoysticks; ++x)
    {
        if (!SDL_IsGameController(x))
        {
            continue;
        }
        SDL_GameControllerOpen(x);
        LOG_D("SDL: Opened Controller index: %d", x);
    }


    _app->OnStart(_window_width, _window_height);

    double dt = 0;
    while (!shouldQuit) {
        double start = GetTime();

        // Reset mouse movement, otherwise camera constantly moves around
        inputValues[(int)input::InputCode::INPUT_MOUSE_XAXISRELATIVE] = 0;
        inputValues[(int)input::InputCode::INPUT_MOUSE_YAXISRELATIVE] = 0;
        
        // Going to do this here, not sure if i should or not..may want to test how this works
        int temp = SDL_GetModState();
        inputValues[(int)input::InputCode::INPUT_KEY_CAPSLOCK] = ((temp & KMOD_CAPS) == KMOD_CAPS);

        while (SDL_PollEvent(&_e) != 0) {
            if (_e.type == SDL_QUIT){
                shouldQuit = true;
            }
            else if (_e.type == SDL_KEYDOWN || _e.type == SDL_KEYUP) {
                // To enable more / new keys, add them to filter function and make sure map is set in GetKeyCode
                switch (_e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    shouldQuit = true;
                default:
                    inputValues[(int)GetKeyCodeFromSDLKey(_e.key.keysym.sym)] = _e.type == SDL_KEYDOWN ? true : false;
                    break;
                }
            }
            else if (_e.type == SDL_MOUSEMOTION){
                inputValues[(int)input::InputCode::INPUT_MOUSE_XAXISRELATIVE] = _e.motion.xrel;
                inputValues[(int)input::InputCode::INPUT_MOUSE_YAXISRELATIVE] = _e.motion.yrel;
            }
            else if (_e.type == SDL_MOUSEBUTTONDOWN || _e.type == SDL_MOUSEBUTTONUP) {
                switch (_e.button.button) {
                case SDL_BUTTON_LEFT:
                    inputValues[(int)input::InputCode::INPUT_MOUSE_KEY1] = _e.button.type == SDL_MOUSEBUTTONDOWN ? true : false;
                    break;
                case SDL_BUTTON_RIGHT:
                    inputValues[(int)input::InputCode::INPUT_MOUSE_KEY2] = _e.button.type == SDL_MOUSEBUTTONDOWN ? true : false;
                    break;
                }
            }
            // todo: redo controller stuff here
            else if (_e.type == SDL_CONTROLLERBUTTONDOWN || _e.type == SDL_CONTROLLERBUTTONUP) {
                switch (_e.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_DOWN] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_UP] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LEFT] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RIGHT] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSHOULDER] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LSHOULDER] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                default: break;
                }
            }
            else if (_e.type == SDL_CONTROLLERAXISMOTION) {
                switch (_e.caxis.axis) {
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSTICKX] = (float)_e.caxis.value / 32767.0;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSTICKY] = (float)_e.caxis.value / 32767.0;
                    break;
                default: break;
                }
            }
        }

        _app->OnFrame(inputValues, dt);

        // is there a different way we can swapbuffer?
        if (renderDeviceConfig == "opengl") {
            SDL_GL_SwapWindow(_window);
        }
        _app->renderDevice->SwapBuffers();
        dt = GetTime() - start;
    }

    _app->OnShutdown();

    // TOOD:: cleaned dyanmic memory
    SDL_DestroyWindow(_window);
    SDL_Quit();
    return 0;
}