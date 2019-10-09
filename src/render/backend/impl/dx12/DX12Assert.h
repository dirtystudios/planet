#pragma once

#include "Log.h"

#include <cassert>

#ifdef _DEBUG
#define DEBUG_DX12
constexpr bool kDebugDx12 = true;
#else
constexpr bool kDebugDx12 = false;
#endif

#define DX12_CHECK(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX12 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); }  \
}  \
while (false)

#define DX12_CHECK_RET0(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX12 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); return 0; }  \
}  \
while (false)

#define DX12_CHECK_RET(func) \
do { \
HRESULT hr = func;\
if(FAILED(hr)) { LOG_E("DX12 error in function: %s. line: %d HR: 0x%x Stmt: %s\n", __FUNCTION__, __LINE__ ,hr , #func); assert(false); return; }  \
}  \
while (false)