#include "DX12CpuDescHeap.h"
#include "DX12Device.h"

#include "DX12Assert.h"

namespace gfx {

    DX12CpuDescHeap::DX12CpuDescHeap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, const std::string& name = "")
        : _dev(dev),  _type(type), _debugName(name) {

        _descSize = _dev->GetDescriptorHandleIncrementSize(type);
        AllocateNewHeap();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12CpuDescHeap::AllocateDescriptor() {
        ID3D12DescriptorHeap* heap;
        if (_currentIdx == kHeapNumDescriptors) {
            // allocate new heap;
            heap = AllocateNewHeap();
            LOG_D("DX12: Allocating heap #%d for %s", _heaps.size(), _debugName.c_str());
            _currentIdx = 0;
        }
        else
            heap = _heaps.back().Get();

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtnHandle(heap->GetCPUDescriptorHandleForHeapStart(), _currentIdx, _descSize);
        _currentIdx++;
        return { rtnHandle.ptr };
    }

    ID3D12DescriptorHeap* DX12CpuDescHeap::AllocateNewHeap() {
        ComPtr<ID3D12DescriptorHeap> heap;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = kHeapNumDescriptors;
        desc.Type = _type;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DX12_CHECK_RET0(_dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap)));

        std::string tmp = _debugName + std::to_string(_heaps.size());
        D3D_SET_OBJECT_NAME_A(heap, tmp.c_str());

        _heaps.emplace_back(heap);
        return heap.Get();
    }
};