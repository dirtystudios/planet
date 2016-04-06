#include <emmintrin.h>
#include <tmmintrin.h>
#include <Windows.h>
#include <intrin.h>
#include <d3d11.h>
#include "Log.h"

#ifdef _DEBUG
#define DEBUG_DX11
#endif

//todo: make these inlined func and not crappy macros

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

namespace graphics {
    namespace dx11 {

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

        // *sigh* need this, apparently amd and nividia probly do this in the driver anyway, 
        //      but not for directx
        //--------------------------------
        //  Attempts detecting and using ssse3 instructions to 'expand' to dst, alpha channel contains garbage
        //      src and dst needs to be aligned to 16 bytes.
        //  If no ssse3 support or unaligned, slow path it is
        //--------------------------------
        void Convert24BitTo32Bit(void* src, void* dst, uint32_t numPixels) {
            int info[4];
            __cpuidex(info, 1, 0);
            bool hasSSSE3 = (info[2] & ((int)1 << 9)) != 0;
            bool alignedSource = (((unsigned long)src & 15) == 0);
            bool alignedDst = (((unsigned long)dst & 15) == 0);

            if (alignedSource && alignedDst && hasSSSE3) {
                __m128i *src128 = (__m128i*)src;
                __m128i *dst128 = (__m128i*)dst;
                __m128i mask = _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);

                for (uint32_t i = 0; i < numPixels; i += 16) {
                    __m128i sa = _mm_load_si128(src128);
                    __m128i sb = _mm_load_si128(src128 + 1);
                    __m128i sc = _mm_load_si128(src128 + 2);

                    __m128i val = _mm_shuffle_epi8(sa, mask);
                    _mm_store_si128(dst128, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sb, sa, 12), mask);
                    _mm_store_si128(dst128 + 1, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sb, 8), mask);
                    _mm_store_si128(dst128 + 2, val);
                    val = _mm_shuffle_epi8(_mm_alignr_epi8(sc, sc, 4), mask);
                    _mm_store_si128(dst128 + 3, val);

                    src128 += 3;
                    dst128 += 4;
                }
                return;
            }
            LOG_D("Convert24bit: Slow Path. HasSSSE3: %s. alignedSource: %d. alignedDst %d.",
                (hasSSSE3 ? "true" : "false"), alignedSource, alignedDst);
            uint32_t *src32 = (uint32_t*)src;
            uint32_t *dst32 = (uint32_t*)dst;

            for (uint32_t i = 0; i < numPixels; i += 4) {
                uint32_t sa = src32[0];
                uint32_t sb = src32[1];
                uint32_t sc = src32[2];

                dst32[i + 0] = sa;
                dst32[i + 1] = (sa >> 24) | (sb << 8);
                dst32[i + 2] = (sb >> 16) | (sc << 16);
                dst32[i + 3] = sc >> 8;

                src32 += 3;
            }
        }
    }
}