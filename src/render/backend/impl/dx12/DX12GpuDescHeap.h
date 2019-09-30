#pragma once

#include <string>

#include <d3d12.h>
#include <wrl.h>

namespace gfx {
    constexpr UINT kHeapNumGpuDescPerFrame = 50;
    constexpr UINT kHeapSize = kHeapNumGpuDescPerFrame * 10;

    class DX12GpuDescHeap {
    private:
        const std::string _debugName{ "" };
        const D3D12_DESCRIPTOR_HEAP_TYPE _type;
        UINT _descSize;
        ID3D12Device* _dev;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _heap;
        UINT _currentIdx{ 0 };
    public:
        DX12GpuDescHeap() = delete;
        DX12GpuDescHeap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, const std::string& name = "");

        D3D12_GPU_DESCRIPTOR_HANDLE GetNextFrameAllocation();
        const UINT GetNumDescriptorsPerAllocation() { return kHeapNumGpuDescPerFrame; }
        const UINT GetDescSize() { return _descSize; }
    };
}