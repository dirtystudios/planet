#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "Log.h"
#include <cassert>

#ifdef _DEBUG
#define DEBUG_DX11
#endif

#define DX11_CHECK(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX11 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); }  \
}  \
while (false)

#define DX11_CHECK_RET0(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX11 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); return 0; }  \
}  \
while (false)

#define DX11_CHECK_RET(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX11 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); return; }  \
}  \
while (false)

namespace gfx {
    // Debug Helper Functions

        // Helper sets a D3D resource name string (used by PIX and debug layer leak reporting).
    inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char *name);

    // Debug Initializtion

    void InitDX11DebugLayer(ID3D11Device* dev);
}
