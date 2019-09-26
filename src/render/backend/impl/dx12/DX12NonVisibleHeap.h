#pragma once

#include <vector>
#include <string>
#include <wrl.h>

#include <d3d12.h>

namespace gfx {
    using namespace Microsoft::WRL;

    constexpr UINT kHeapSize = 100;

    class DX12NonVisibleHeap {
    private:
        ID3D12Device* _dev;
        UINT _descSize;
        D3D12_DESCRIPTOR_HEAP_TYPE _type;
        std::string _debugName;
        UINT _currentIdx{0};

        std::vector<ComPtr<ID3D12DescriptorHeap>> _heaps;
    public:
        DX12NonVisibleHeap() = delete;
        DX12NonVisibleHeap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, const std::string& name = "");
        D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor();

    private:
        ID3D12DescriptorHeap* AllocateNewHeap();
    };
};