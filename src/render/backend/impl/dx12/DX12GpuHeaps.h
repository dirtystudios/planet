#pragma once

namespace gfx {

    struct DX12GpuHeaps {
        ID3D12DescriptorHeap* srvHeap;
        UINT srvDescSize;
        ID3D12DescriptorHeap* samplerHeap;
        UINT samplerDescSize;

        UINT offset;
        UINT numAllocated;
    };
}
