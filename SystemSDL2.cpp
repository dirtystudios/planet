#include "stdafx.h"
#include "System.h"

#include "SDL.h"
#include "SDL_syswm.h"

#ifdef _WIN32
#include <GL/glew.h>
#endif

#include "Log.h"

#ifdef _WIN32
#include "gfx/dx11/DX11RenderDevice.h"
#endif
#include "gfx/gl/GLRenderDevice.h"

//#define DX11_BACKEND
#define GL_BACKEND

SDL_Window* _window = NULL;
SDL_Event _e;
uint32_t _window_width = 800;
uint32_t _window_height = 600;
const char* _window_title = "dirty";
app::AppState _app_state;
app::Application* _app = { nullptr };
bool shouldQuit = false;

int filterSDLEvents(void* userdata, SDL_Event* event){
    //what im going to do here is just weed out any key's we dont want
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym){
        case SDLK_w:
        case SDLK_a:
        case SDLK_s:
        case SDLK_d:
        case SDLK_e:
        case SDLK_q:
        case SDLK_1:
        case SDLK_2:
        case SDLK_3:
        case SDLK_ESCAPE:
            return 1;
            break;
        default:
            return 0;
        }
    }
    //keep any other events...for now
    return 1;
}

#define MAP_KEY_SDL(x, y) SDLK_##x, app::KeyCode::KEY_##y
app::KeyCode GetKeyCodeFromGLFWKey(int glfw_key) {
    static std::map<int, app::KeyCode> glfw_mapping = {
        { MAP_KEY_SDL(a, A) },
        { MAP_KEY_SDL(d, D) },
        { MAP_KEY_SDL(e, E) },
        { MAP_KEY_SDL(q, Q) },
        { MAP_KEY_SDL(r, R) },
        { MAP_KEY_SDL(s, S) },
        { MAP_KEY_SDL(w, W) },
        { MAP_KEY_SDL(1, 1) },
        { MAP_KEY_SDL(2, 2) },
        { MAP_KEY_SDL(3, 3) },
    };
    auto it = glfw_mapping.find(glfw_key);
    return it == glfw_mapping.end() ? app::KeyCode::KEY_UNKNOWN : it->second;
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

    // Render device abstaction to come later, until then we can just init opengl for every version
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

#if  defined(_WIN32) && defined(GL_BACKEND)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();
#endif

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    const char *subsystem = "Unknown System!";
    int initRtn = 0;
    if (SDL_GetWindowWMInfo(_window, &info)){
        switch (info.subsystem){
        case SDL_SYSWM_UNKNOWN: break;
        case SDL_SYSWM_X11: subsystem = "X Window System"; break;
        case SDL_SYSWM_WINDOWS:
            subsystem = "Microsoft Windows";
#ifdef DX11_BACKEND
            _app->renderDevice = new graphics::RenderDeviceDX11();
            _app->renderDevice->InitializeDevice(info.info.win.window, _window_height, _window_width);
#elif defined(GL_BACKEND)
            _app->renderDevice = new graphics::RenderDeviceGL();
#endif
            break;
        case SDL_SYSWM_COCOA: 
            subsystem = "Apple OS X"; 
            _app->renderDevice = new graphics::RenderDeviceGL();
            break;
        //case SDL_SYSWM_ANDROID: subsystem = "Android"; break;
        }
    }
	
    SDL_SetEventFilter(filterSDLEvents, NULL);

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


    _app->OnStart();

    double dt = 0;
    while (!shouldQuit) {
        double start = GetTime();

        // Reset mouse movement, otherwise camera constantly moves around
        _app_state.cursor_state.delta_x = 0;
        _app_state.cursor_state.delta_y = 0;
        
        // need a 'deadzone' for stick reset;
        _app_state.controllerState.rightStick.x = abs(_app_state.controllerState.rightStick.x) < 0.15 ? 0 : _app_state.controllerState.rightStick.x;
        _app_state.controllerState.rightStick.y = abs(_app_state.controllerState.rightStick.y) < 0.15 ? 0 : _app_state.controllerState.rightStick.y;

        while (SDL_PollEvent(&_e) != 0) {
            //user quit
            if (_e.type == SDL_QUIT){
                shouldQuit = true;
            }
            else if (_e.type == SDL_KEYDOWN || _e.type == SDL_KEYUP) {

                switch (_e.key.keysym.sym){
                case SDLK_w:
                case SDLK_a:
                case SDLK_s:
                case SDLK_d:
                case SDLK_e:
                case SDLK_q:
                case SDLK_1:
                case SDLK_2:
                case SDLK_3:
                    _app_state.key_state.pressed[(int)GetKeyCodeFromGLFWKey(_e.key.keysym.sym)] = _e.type == SDL_KEYDOWN ? true : false;
                    break;
                case SDLK_ESCAPE:
                    shouldQuit = true;
                    break;
                default:
                    break;
                }
            }
            else if (_e.type == SDL_MOUSEMOTION){
                _app_state.cursor_state.delta_x = _e.motion.xrel;
                _app_state.cursor_state.delta_y = _e.motion.yrel;
            }
            else if (_e.type == SDL_CONTROLLERBUTTONDOWN || _e.type == SDL_CONTROLLERBUTTONUP) {
                switch (_e.cbutton.button) {
                case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_DOWN] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_UP] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_LEFT] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_RIGHT] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_RSHOULDER] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                    _app_state.controllerState.keyState.pressed[(int)app::ControllerKeyCode::KEY_LSHOULDER] = _e.type == SDL_CONTROLLERBUTTONDOWN ? true : false;
                    break;
                default: break;
                }
            }
            else if (_e.type == SDL_CONTROLLERAXISMOTION) {
                switch (_e.caxis.axis) {
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    _app_state.controllerState.rightStick.x = (float)_e.caxis.value / 32767.0;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    _app_state.controllerState.rightStick.y = (float)_e.caxis.value / 32767.0;
                    break;
                default: break;
                }
            }
        }

        _app->OnFrame(&_app_state, dt);
#ifdef GL_BACKEND
        SDL_GL_SwapWindow(_window);
#endif
        _app->renderDevice->SwapBuffers();
        dt = GetTime() - start;
    }

    _app->OnShutdown();

    // TOOD:: cleaned dyanmic memory
    SDL_DestroyWindow(_window);
    SDL_Quit();
    return 0;
}