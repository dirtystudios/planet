#ifndef __app_h__
#define __app_h__

#include "Application.h"

class App : public app::Application {
public:
    void OnStart() final;
    void OnFrame(const app::AppState* app_state, float dt) final;
    void OnShutdown() final;
    
};

#endif
