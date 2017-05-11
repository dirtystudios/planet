#pragma once

#define MICROPROFILE_MAX_THREADS 128
#include <microprofile/microprofile.h>
#include "DebugDrawInterface.h"

extern void MicroProfileGpuInitD3D11(struct ID3D11Device* pDevice);

class Profiler {
private:
    static DebugDrawInterface* m_debugRenderer;
public:
    static void Initialize(DebugDrawInterface* debug);
    static bool OnKeyDown(int key_code);
    static bool OnKeyUp(int key_code);
    static void OnMouseDown(bool left_button, bool right_button);
    static void OnMouseUp();
    static void OnMouseMove(int x, int y);
    static void OnMouseWheel(int x, int y, int dy);
    static void ToggleDisplay();
    static void TogglePause();

    static void Shutdown();

    static DebugDrawInterface* Renderer() { return m_debugRenderer; }
    static void Draw();
    static void Flip();
};