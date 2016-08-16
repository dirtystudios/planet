#include "DX11Debug.h"


namespace gfx {
    // Debug Helper Functions

#pragma region Debug Helper Functions

    // Helper sets a D3D resource name string (used by PIX and debug layer leak reporting).
    inline void SetDebugObjectName(_In_ ID3D11DeviceChild* resource, _In_z_ const char *name) {
#ifdef DEBUG_DX11
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(name)), name);
#else
        UNREFERENCED_PARAMETER(resource);
        UNREFERENCED_PARAMETER(name);
#endif
    }

    void OutputLiveObjects(ID3D11Device* dev) {
#ifdef DEBUG_DX11
        ID3D11Debug *d3dDebug = nullptr;
        if (SUCCEEDED(dev->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
            DX11_CHECK(d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY));
            d3dDebug->Release();
        }
#endif
    }

    // Debug Initializtion

    void InitDX11DebugLayer(ID3D11Device* dev) {

#ifdef DEBUG_DX11
        ID3D11Debug *d3dDebug = nullptr;
        if (SUCCEEDED(dev->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
            ID3D11InfoQueue *d3dInfoQueue = nullptr;
            if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue))) {
                //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                //d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

                D3D11_MESSAGE_ID hide[] = {
                    D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                    // Add more message IDs here as needed
                };

                /*D3D11_INFO_QUEUE_FILTER filter;
                memset(&filter, 0, sizeof(filter));
                filter.DenyList.NumIDs = _countof(hide);
                filter.DenyList.pIDList = hide;
                d3dInfoQueue->AddStorageFilterEntries(&filter);*/
                d3dInfoQueue->Release();
            }
            else {
                LOG_E("DX11Render: Failed to Create d3dDebugInfoQueue.");
                d3dDebug->Release();
            }
        }
        else {
            LOG_E("DX11Render: Failed to Create d3dDebug Interface.");
        }
#endif
    }
#pragma endregion
}
