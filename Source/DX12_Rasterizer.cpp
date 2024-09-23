#include "DX12_Rasterizer.hpp"
#include <stdexcept>

void throw_if_failed(const HRESULT result)
{
    if (result != S_OK)
    {
        throw std::runtime_error("Failed to create device");
    }
}

dx12_rasterizer dx12_rasterizer_create(const HWND window_handle, const UINT width, const UINT height)
{
    dx12_rasterizer result = {};
    
    IDXGIFactory2* factory = nullptr;
    throw_if_failed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

    ID3D12Debug* debug = nullptr;
    throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));
    debug->EnableDebugLayer();

    for (UINT32 i = 0; factory->EnumAdapters1(i, &result.adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        result.adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(result.adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&result.device))))
        {
            break;
        }
    }
    ID3D12Device* device = result.device;

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        throw_if_failed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&result.command_queue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        throw_if_failed(factory->CreateSwapChainForHwnd(result.command_queue, window_handle, &desc, nullptr, nullptr,
                                                        &result.swap_chain));

        result.current_frame_index = 0;
        throw_if_failed(
            result.swap_chain->GetBuffer(result.current_frame_index, IID_PPV_ARGS(&result.frame_buffers[0])));
        throw_if_failed(
            result.swap_chain->GetBuffer(result.current_frame_index, IID_PPV_ARGS(&result.frame_buffers[1])));
    }
    throw_if_failed(
        device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&result.command_allocator)));
    throw_if_failed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, result.command_allocator, nullptr,
                                              IID_PPV_ARGS(&result.command_list)));
    throw_if_failed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&result.fence)));
    result.fence_value = 0;

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 2;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        throw_if_failed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&result.rtv_heap)));

        const UINT32 rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        result.frame_buffer_descriptors[0] = result.rtv_heap->GetCPUDescriptorHandleForHeapStart();
        device->CreateRenderTargetView(result.frame_buffers[0], nullptr, result.frame_buffer_descriptors[0]);
        result.frame_buffer_descriptors[1].ptr = result.frame_buffer_descriptors[0].ptr + rtv_descriptor_size;
        device->CreateRenderTargetView(result.frame_buffers[1], nullptr, result.frame_buffer_descriptors[1]);
    }

    return result;
}

int main()
{
    return 0;
}
