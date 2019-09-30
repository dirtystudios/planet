#include "DX12GpuDescHeap.h"

#include "DX12Assert.h"

#include "d3dx12.h"

namespace gfx {
    DX12GpuDescHeap::DX12GpuDescHeap(ID3D12Device* dev, D3D12_DESCRIPTOR_HEAP_TYPE type, const std::string& name = "")
        : _dev(dev), _type(type), _debugName(name) {

        _descSize = _dev->GetDescriptorHandleIncrementSize(type);

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = kHeapSize;
        desc.Type = _type;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        DX12_CHECK_RET(_dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_heap)));

        D3D_SET_OBJECT_NAME_A(_heap, _debugName.c_str());
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12GpuDescHeap::GetNextFrameAllocation() {
        auto rtn = CD3DX12_GPU_DESCRIPTOR_HANDLE(_heap->GetGPUDescriptorHandleForHeapStart(), _currentIdx, _descSize);

        _currentIdx = (_currentIdx + kHeapNumGpuDescPerFrame) % kHeapSize;
        return rtn;
    }
}