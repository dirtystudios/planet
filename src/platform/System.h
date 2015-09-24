#include "Application.h"

namespace sys {
    struct SysWindowSize {
        unsigned int width, height;
    };
    void SetWindowTitle(const char* title);
    SysWindowSize GetWindowSize();
    float GetTime();
    int Run(app::Application* app);
}