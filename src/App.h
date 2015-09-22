#ifndef __app_h__
#define __app_h__

#include "Application.h"

class App : public app::Application {
public:
    void OnStart(uint32_t windowWidth, uint32_t windowHeight) final;
    void OnFrame(const std::vector<float>& inputValues, float dt) final;
    void OnShutdown() final;
    
};

#endif
