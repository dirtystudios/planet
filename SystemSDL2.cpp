#include "System.h"

#include "SDL.h"
#include "SDL_syswm.h"

#ifdef _WIN32
#include <GL/glew.h>
#include <stdint.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include "Log.h"
#include <map>

#ifdef _WIN32
//#include "DeviceRenderDX11.h"
#endif

#ifndef WIN32
//typedef void* HWND;
#endif

//DeviceRender* _deviceRender = NULL;
SDL_Window* _window = NULL;
SDL_Event _e;
int32_t _window_width = 800;
int32_t _window_height = 600;
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

    _window = SDL_CreateWindow(_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_width, _window_height, SDL_WINDOW_OPENGL);

    if (_window == NULL){
        LOG_E("SDL could not create window: %s", SDL_GetError());
        return -1;
    }

    SDL_GL_CreateContext(_window);

#ifdef _WIN32
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
            // This can be ignored for the time being
            /*_deviceRender = new DeviceRenderDX11();
            initRtn = _deviceRender->init(info.info.win.window);
            _app->SetDeviceRender(_deviceRender);*/
            break;
        case SDL_SYSWM_COCOA: subsystem = "Apple OS X"; break;
        case SDL_SYSWM_ANDROID: subsystem = "Android"; break;
        }
    }
	
    SDL_SetEventFilter(filterSDLEvents, NULL);

    LOG_D("SDL Initialized. Version: %d.%d.%d on %s", info.version.major, info.version.minor, info.version.patch, subsystem);

    if (initRtn != 0){
        LOG_E("DeviceRender Init Failed.");
        return -1;
    }

    //_app->SetDeviceRender(_deviceRender);
    _app->OnStart();

    double dt = 0;
    while (!shouldQuit) {
        double start = GetTime();

        //handle events

        // Reset mouse movement, otherwise camera constantly moves around
        _app_state.cursor_state.delta_x = 0;
        _app_state.cursor_state.delta_y = 0;
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
        }

        _app->OnFrame(&_app_state, dt);
        //_app->GetDeviceRender().SwapBuffers();
        SDL_GL_SwapWindow(_window);
        dt = GetTime() - start;
    }

    _app->OnShutdown();
    SDL_DestroyWindow(_window);
    SDL_Quit();
    return 0;
}