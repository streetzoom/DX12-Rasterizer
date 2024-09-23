#pragma once

#include <d3d12.h>
#include <dxgi1_3.h>

struct dx12_rasterizer
{
    IDXGIAdapter1* adapter;
    ID3D12Device* device;
    ID3D12CommandQueue* command_queue;
    
    IDXGISwapChain1* swap_chain;
    UINT32 current_frame_index;
    ID3D12Resource* frame_buffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE frame_buffer_descriptors[2];

    ID3D12CommandAllocator* command_allocator;
    ID3D12GraphicsCommandList* command_list;

    ID3D12Fence* fence;
    UINT64 fence_value;

    ID3D12DescriptorHeap* rtv_heap;
};
