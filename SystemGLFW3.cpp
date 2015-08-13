#include "System.h"

#ifdef _WIN32
#include <GL/glew.h>
#include <stdint.h>
#else
#include <unistd.h>
#endif

#include <glfw/glfw3.h>
#include <iostream>
#include "Log.h"
#include <map>

GLFWwindow* _window = NULL;
int32_t _window_width = 800;
int32_t _window_height = 600;
const char* _window_title = "dirty";
double curr_cursor[2] = {0};
double prev_cursor[2] = {0};
bool first = true;

app::AppState _app_state;
app::Application* _app = { nullptr };

#define MAP_KEY_GLFW(x) GLFW_KEY_##x, app::KeyCode::KEY_##x
app::KeyCode GetKeyCodeFromGLFWKey(int glfw_key) {
    static std::map<int, app::KeyCode> glfw_mapping = {
        {MAP_KEY_GLFW(A)},
        {MAP_KEY_GLFW(B)},
        {MAP_KEY_GLFW(C)},
        {MAP_KEY_GLFW(D)},
        {MAP_KEY_GLFW(E)},
        {MAP_KEY_GLFW(F)},
        {MAP_KEY_GLFW(G)},
        {MAP_KEY_GLFW(H)},
        {MAP_KEY_GLFW(I)},
        {MAP_KEY_GLFW(J)},
        {MAP_KEY_GLFW(K)},
        {MAP_KEY_GLFW(L)},
        {MAP_KEY_GLFW(M)},
        {MAP_KEY_GLFW(N)},
        {MAP_KEY_GLFW(O)},
        {MAP_KEY_GLFW(P)},
        {MAP_KEY_GLFW(P)},
        {MAP_KEY_GLFW(Q)},
        {MAP_KEY_GLFW(R)},
        {MAP_KEY_GLFW(S)},
        {MAP_KEY_GLFW(T)},
        {MAP_KEY_GLFW(U)},
        {MAP_KEY_GLFW(V)},
        {MAP_KEY_GLFW(W)},
        {MAP_KEY_GLFW(X)},
        {MAP_KEY_GLFW(Y)},
        {MAP_KEY_GLFW(Z)},
        {MAP_KEY_GLFW(1)},
        {MAP_KEY_GLFW(2)},
        {MAP_KEY_GLFW(3)},
        {MAP_KEY_GLFW(4)},
        {MAP_KEY_GLFW(5)},
        {MAP_KEY_GLFW(6)},
        {MAP_KEY_GLFW(7)},
        {MAP_KEY_GLFW(8)},
        {MAP_KEY_GLFW(9)},
        {MAP_KEY_GLFW(0)},
        {MAP_KEY_GLFW(LEFT_SHIFT)}
    };
    auto it = glfw_mapping.find(glfw_key);
    return it == glfw_mapping.end() ? app::KeyCode::KEY_UNKNOWN : it->second;
}

static void ErrorCallback(const int error, const char *description) {    
    glfwSetWindowShouldClose(_window, GL_TRUE);
}

static void KeyCallback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(_window, GL_TRUE);
    }    
    _app_state.key_state.pressed[(int)GetKeyCodeFromGLFWKey(key)] = action >= 1 ? true : false;  
}

static void FramebufferResizeCallback(GLFWwindow *window, int width, int height) {
//    LOG_D("sys", "framebuffer resize -> width:" << width << " height:" << height);
    _window_width = width;
    _window_height = height;
}

static void CursorPosCallback(GLFWwindow* window, double x, double y) {
//    LOG_D("sys", "cursor pos -> x:" << x << ", y:" << y);
    curr_cursor[0] = x;
    curr_cursor[1] = y;
    
    /**
     * theres no way to get the mouse position before a mouse event happes.
     * glfwGetCursorPos() always return (0,0) even though first movement jumps
     * to some other value. To prevent the first delta from being gigantic, need to
     * catch first ever mouse event and set prev vals;
     */
    if(first) {
        prev_cursor[0] = x;
        prev_cursor[1] = y;
        first = false;
    }
}

static void MouseButtonCallback(GLFWwindow* window, int btn, int action, int mods) {    
}

static void CursorEnterCallback(GLFWwindow* window, int entered) {    
    _app_state.cursor_state.entered = entered;
}


int sys::Run(app::Application* app) {    
    int ret = -1;

    glfwSetErrorCallback(ErrorCallback);

    if(!glfwInit()) {
        return ret;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);    
    
    _window = glfwCreateWindow(_window_width, _window_height, _window_title, NULL, NULL);
     if (!_window) {
        glfwTerminate();
        return ret;
    }
    _app = app;
    glfwMakeContextCurrent(_window);

#ifdef _WIN32
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	glGetError();
#endif

    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
    glfwSetCursorPosCallback(_window, CursorPosCallback);
    glfwSetMouseButtonCallback(_window, MouseButtonCallback);
    glfwSetKeyCallback(_window, KeyCallback);
    glfwSetCursorEnterCallback(_window, CursorEnterCallback);
//    glfwSwapInterval(0);
    //glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetWindowSize(_window, &_window_width, &_window_height);
    //glfwSetWindowPos(_window, 1000, 500);    
                
    _app->OnStart();

    double dt = 0;
    while(!glfwWindowShouldClose(_window)) {
        double start = glfwGetTime();

        glfwPollEvents();

        _app_state.cursor_state.delta_x = curr_cursor[0] - prev_cursor[0];
        _app_state.cursor_state.delta_y = curr_cursor[1] - prev_cursor[1];
        prev_cursor[0] = curr_cursor[0];
        prev_cursor[1] = curr_cursor[1];
                 
        _app->OnFrame(&_app_state, dt);   

        memcpy(_app_state.key_state.prev_pressed, _app_state.key_state.pressed, sizeof(bool) * 256);         
            
        glfwSwapBuffers(_window);
        dt = glfwGetTime() - start;
    }

    _app->OnShutdown();
    glfwDestroyWindow(_window);
    glfwTerminate();

    return ret;
}

void sys::SetWindowTitle(const char* title) {
    glfwSetWindowTitle(_window, title);
}

float sys::GetTime() {
    return glfwGetTime();
}
