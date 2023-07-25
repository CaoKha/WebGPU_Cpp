#pragma once
#include <common.h>
#include <webgpu/webgpu.h>

class SwapChain {
public:
  WGPUSwapChainDescriptor wgpu_swapchain_desc = {};
  WGPUSwapChain wgpu_swapchain;
  SwapChain(WGPUSurface surface, WGPUAdapter adapter, WGPUDevice device) {
    std::cout << "Creating swapchain device... " << std::endl;
    setupSwapChainDescriptor(surface, adapter);
    wgpu_swapchain =
        wgpuDeviceCreateSwapChain(device, surface, &wgpu_swapchain_desc);
    std::cout << "SwapChain: " << wgpu_swapchain << std::endl;
  };
  ~SwapChain() { wgpuSwapChainRelease(wgpu_swapchain); };
  void setupSwapChainDescriptor(WGPUSurface surface, WGPUAdapter adapter);
};
