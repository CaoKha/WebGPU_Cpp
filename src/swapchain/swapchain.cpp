#include "swapchain.hpp"

void SwapChain::setupSwapChainDescriptor(WGPUSurface surface,
                                         WGPUAdapter adapter) {
#ifdef WEBGPU_BACKEND_WGPU
  WGPUTextureFormat swapchain_format =
      wgpuSurfaceGetPreferredFormat(surface, adapter);
#else
  WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;
#endif
  wgpu_swapchain_desc.width = 640;
  wgpu_swapchain_desc.height = 480;
  wgpu_swapchain_desc.usage = WGPUTextureUsage_RenderAttachment;
  wgpu_swapchain_desc.format = swapchain_format;
  wgpu_swapchain_desc.presentMode = WGPUPresentMode_Fifo;
}
