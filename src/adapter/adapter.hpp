#pragma once
#include <common.h>
#include <webgpu-utils.h>

class Adapter {
public:
  WGPUAdapter wgpu_adapter;
  WGPURequestAdapterOptions wgpu_adapter_opts;
  Adapter(WGPUInstance instance, GLFWwindow *window) {
    std::cout << "Requesting adapter..." << std::endl;
    setupAdapterDescriptor(instance, window);
    wgpu_adapter = requestAdapter(instance, &wgpu_adapter_opts);
    std::cout << "Got adapter: " << wgpu_adapter << std::endl;
  };
  ~Adapter() { wgpuAdapterRelease(wgpu_adapter); };
  void setupAdapterDescriptor(WGPUInstance instance, GLFWwindow *window) {
    wgpu_adapter_opts.nextInChain = nullptr;
    wgpu_adapter_opts.compatibleSurface = glfwGetWGPUSurface(instance, window);
  };
};
