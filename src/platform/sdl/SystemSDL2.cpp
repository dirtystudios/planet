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

#include "GLDevice.h"

#include "Log.h"
#include "Config.h"
#include <map>

SDL_Window* _window = NULL;
SDL_Event _e;
uint32_t _window_width    = 1200;
uint32_t _window_height   = 800 ;
const char* _window_title = "dirty";
app::Application* _app = { nullptr };
bool shouldQuit = false;
std::vector<float> inputValues((int)input::InputCode::COUNT);
std::map<int, input::InputCode> sdlkMapping;

#define MAP_KEY_SDL(x, y) sdlkMapping.insert(std::make_pair(SDLK_##x, input::InputCode::INPUT_KEY_##y));
void PopulateKeyMapping() {
    for (int x = 0; x <= (int)input::InputCode::INPUT_KEY_Z; ++x) {
        sdlkMapping.insert(std::make_pair(SDLK_a + x, (input::InputCode)x));
    }

    int range = (int)input::InputCode::INPUT_KEY_9 - (int)input::InputCode::INPUT_KEY_0;
    for (int x = 0; x <= range; ++x) {
        sdlkMapping.insert(std::make_pair(SDLK_0 + x, (input::InputCode)(x + (int)input::InputCode::INPUT_KEY_0)));
    }

    MAP_KEY_SDL(BACKQUOTE, BACKTICK);
    MAP_KEY_SDL(MINUS, HYPEN);
    MAP_KEY_SDL(EQUALS, EQUALS);
    MAP_KEY_SDL(LEFTBRACKET, LEFTSQUARE);
    MAP_KEY_SDL(RIGHTBRACKET, RIGHTSQUARE);
    MAP_KEY_SDL(BACKSLASH, BACKSLASH);
    MAP_KEY_SDL(SEMICOLON, SEMICOLON);
    MAP_KEY_SDL(QUOTE, SINGLEQUOTE);
    MAP_KEY_SDL(COMMA, COMMA);
    MAP_KEY_SDL(PERIOD, DOT);
    MAP_KEY_SDL(SLASH, FORWARDSLASH);
    MAP_KEY_SDL(SPACE, SPACE);

    MAP_KEY_SDL(LALT, LEFT_ALT);
    MAP_KEY_SDL(RALT, RIGHT_ALT);
    MAP_KEY_SDL(LCTRL, LEFT_CTRL);
    MAP_KEY_SDL(RCTRL, RIGHT_CTRL);
    MAP_KEY_SDL(LGUI, WINDOWS);
    MAP_KEY_SDL(LSHIFT, LEFT_SHIFT);
    MAP_KEY_SDL(RSHIFT, RIGHT_SHIFT);

    MAP_KEY_SDL(CAPSLOCK, CAPSLOCK);
    MAP_KEY_SDL(BACKSPACE, BACKSPACE);
    MAP_KEY_SDL(INSERT, INSERT);
    MAP_KEY_SDL(HOME, HOME);
    MAP_KEY_SDL(PAGEUP, PAGEUP);
#undef DELETE
    MAP_KEY_SDL(DELETE, DELETE);
    MAP_KEY_SDL(END, END);
    MAP_KEY_SDL(PAGEDOWN, PAGEDOWN);
    MAP_KEY_SDL(TAB, TAB);
    MAP_KEY_SDL(ESCAPE, ESCAPE);
    MAP_KEY_SDL(RETURN, ENTER);

    MAP_KEY_SDL(LEFT, LEFT);
    MAP_KEY_SDL(RIGHT, RIGHT);
    MAP_KEY_SDL(UP, UP);
    MAP_KEY_SDL(DOWN, DOWN);
}

input::InputCode GetKeyCodeFromSDLKey(int glfw_key) {
    auto it = sdlkMapping.find(glfw_key);
    return it == sdlkMapping.end() ? input::InputCode::KEY_UNKNOWN : it->second;
}

void sys::SetWindowTitle(const char* title) {
    SDL_SetWindowTitle(_window, title);
}

sys::SysWindowSize sys::GetWindowSize() {
    SysWindowSize windowSize;
    windowSize.height = _window_height;
    windowSize.width = _window_width;
    return windowSize;
}

float sys::GetTime() {
    //return seconds?
    return (float)(SDL_GetTicks()) / 1000.0;
}

void sys::ShowCursor(bool showCursor) {
    if (SDL_ShowCursor(showCursor ? SDL_ENABLE : SDL_DISABLE) < 0) {
        LOG_E("SDL_ShowCursor failed setting %d. Error: %s", showCursor, SDL_GetError());
    }
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
        LOG_D("Invalid RenderDevice set in ini. Using Default for system. Given: %s", renderDeviceConfig.c_str());
    }

    // TODO: context creation should probably be moved to render backend
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _window = SDL_CreateWindow(_window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_width, _window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

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
                std::string usePrebuiltShadersConfig = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "UsePrebuiltShaders");

                graphics::DeviceInitialization devInit;
                devInit.windowHandle       = info.info.win.window;
                devInit.windowHeight       = _window_height;
                devInit.windowWidth        = _window_width;
                devInit.usePrebuiltShaders = usePrebuiltShadersConfig == "y" ? true : false;
                _app->renderDevice->InitializeDevice(devInit);
            }
#endif
            break;
        case SDL_SYSWM_COCOA:
            subsystem = "Apple OS X";
            //            _app->renderDevice = new graphics::RenderDeviceGL();
            _app->renderDevice = new graphics::GLDevice();
            break;
            // case SDL_SYSWM_ANDROID: subsystem = "Android"; break;
        }
    } else {
        LOG_E("Couldn't get window information: %s\n", SDL_GetError());
    }

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


    _app->OnStart();

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
            else if (_e.type == SDL_WINDOWEVENT) {
                if (_e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    _window_width = (unsigned)_e.window.data1;
                    _window_height = (unsigned)_e.window.data2;
                    _app->renderDevice->ResizeWindow(_window_width, _window_height);
                }
            }
            else if (_e.type == SDL_KEYDOWN || _e.type == SDL_KEYUP) {
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

                inputValues[(int)input::InputCode::INPUT_MOUSE_XAXIS] = _e.motion.x;
                inputValues[(int)input::InputCode::INPUT_MOUSE_YAXIS] = _e.motion.y;
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