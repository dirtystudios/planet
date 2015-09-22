#ifndef __application_h__
#define __application_h__

#include "rendering/backend/RenderDevice.h"
#include "input/InputCodes.h"

namespace app {
    class Application {
    public:
        graphics::RenderDevice* renderDevice;
        virtual void OnStart(uint32_t windowWidth, uint32_t windowHeight) = 0;
        virtual void OnFrame(const std::vector<float>& inputValues, float dt) = 0;
        virtual void OnShutdown() = 0;
    };
}

#endif