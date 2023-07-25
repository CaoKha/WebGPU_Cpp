#pragma once
#include <common.h>
#include <webgpu-utils.h>

class Device {
public:
  WGPUDeviceDescriptor wgpu_device_desc = {};
  WGPUDevice wgpu_device;
  Device(WGPUAdapter adapter) {
    std::cout << "Requesting device..." << std::endl;
    setupDeviceDescriptor();
    wgpu_device = requestDevice(adapter, &wgpu_device_desc);
    std::cout << "Got device: " << wgpu_device << std::endl;
  };
  ~Device() { wgpuDeviceRelease(wgpu_device); }
  void setupDeviceDescriptor();
  void runCheck();
};
