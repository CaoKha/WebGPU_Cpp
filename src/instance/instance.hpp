#pragma once
#include <common.h>

class Instance {
public:
  WGPUInstanceDescriptor wgpu_instance_desc = {};
  WGPUInstance wgpu_instance;
  Instance() {
    wgpu_instance_desc.nextInChain = nullptr;
    wgpu_instance = wgpuCreateInstance(&wgpu_instance_desc);
    if (!wgpu_instance) {
      std::cerr << "Could not initialize WebGPU!" << std::endl;
      exit(1);
    }
  };
  ~Instance() { wgpuInstanceRelease(wgpu_instance); };
};
