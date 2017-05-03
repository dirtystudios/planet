#ifndef __application_h__
#define __application_h__

#include "RenderDevice.h"
#include "input/InputCodes.h"

namespace app {
class Application {
public:
    gfx::RenderDevice* renderDevice;
    virtual void OnStart() = 0;
    virtual void OnFrame(const std::vector<float>& inputValues, float dt) = 0;
    virtual void OnShutdown() = 0;
};
}

#endif