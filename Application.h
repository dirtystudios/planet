#ifndef __application_h__
#define __application_h__
#include "gfx/RenderDevice.h"
#include "input/InputCodes.h"

namespace app {
    class Application {
    public:
        graphics::RenderDevice* renderDevice;
        virtual void OnStart() = 0;
        virtual void OnFrame(const std::vector<float>& inputValues, float dt) = 0;
        virtual void OnShutdown() = 0;
    };
}

#endif