
#include "Log.h"
#include "Helpers.h"

#define MICROPROFILE_ENABLED 1
#define MICROPROFILEUI_ENABLED 1
#define MICROPROFILE_GPU_TIMERS_D3D11 1
#define MICROPROFILE_IMPL 1
#define MICROPROFILEUI_IMPL 1
#define MICROPROFILE_PER_THREAD_BUFFER_SIZE (1024 * 1024 * 10)
#define MICROPROFILE_USE_THREAD_NAME_CALLBACK 1
#define MICROPROFILE_WEBSERVER_MAXFRAMES 3
#define MICROPROFILE_PRINTF LOG_D
#define MICROPROFILE_WEBSERVER 0
#define MICROPROFILE_DEBUG 0
#define MICROPROFILE_MAX_THREADS 128
#include <microprofile/microprofile.h>
#include "Profiler.h"
#include <microprofile/microprofileui.h>

const char* MicroProfileGetThreadName() { return "TODO: get thread name!"; }

void MicroProfileDrawBox(int nX, int nY, int nX1, int nY1, uint32_t nColor, MicroProfileBoxType type) {
    // todo: support stupid boxtype
    auto renderer = Profiler::Renderer();
    if (renderer)
        renderer->AddRect2D({ { nX, nY }, { nX1, nY1 } }, dUtils::ConvertRGBColor(nColor));
}

void MicroProfileDrawLine2D(uint32_t nVertices, float* pVertices, uint32_t nColor) {
    auto renderer = Profiler::Renderer();
    if (renderer) {
        for (int i=1;i<=nVertices;++i)
            renderer->AddLine2D({pVertices[(i - 1) * 2], pVertices[(i - 1) * 2 + 1] }, { pVertices[i * 2], pVertices[i * 2 + 1] }, ConvertRGBColor(nColor));
    }
}

void MicroProfileDrawText(int nX, int nY, uint32_t nColor, const char* pText, uint32_t nLen) {
    auto renderer = Profiler::Renderer();
    if (renderer) 
        // todo:
}

void Profiler::Initialize(DebugDrawInterface* debug) {
    MicroProfileSetEnableAllGroups(true);

    MicroProfileInitUI();
    g_MicroProfileUI.bShowSpikes = true;
    g_MicroProfileUI.nOpacityBackground = 0x40u << 24;
    g_MicroProfileUI.nOpacityForeground = 0xc0u << 24;
    MicroProfileSetDisplayMode(1);
}

void Profiler::Shutdown() {
    MicroProfileShutdown();
}