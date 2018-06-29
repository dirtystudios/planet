#ifndef __app_h__
#define __app_h__

#include "Application.h"

class App : public app::Application {
public:
    void OnStart() final;
    // dt expects to be in seconds
    void OnFrame(const std::vector<float>& inputValues, float dt) final;
    void OnShutdown() final;
    void OnWindowResize(uint32_t width, uint32_t height) final;
    
};

#endif
