#include "Application.h"

namespace sys {
    struct SysWindowSize {
        unsigned int width, height;
    };
    void SetWindowTitle(const char* title);
    SysWindowSize GetWindowSize();
    // needs to return back in seconds
    float GetTime();
    void ShowCursor(bool showCursor);
    int Run(app::Application* app);
}