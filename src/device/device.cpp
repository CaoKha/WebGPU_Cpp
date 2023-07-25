#include "device.hpp"
#include <webgpu-utils.h>

void Device::setupDeviceDescriptor() {
  wgpu_device_desc.nextInChain = nullptr;
  wgpu_device_desc.label = "My Device"; // anything works here, that's your call
  wgpu_device_desc.requiredFeaturesCount =
      0; // we do not require any specific feature
  wgpu_device_desc.requiredLimits =
      nullptr; // we do not require any specific limit
  wgpu_device_desc.defaultQueue.nextInChain = nullptr;
  wgpu_device_desc.defaultQueue.label = "The default queue";
}

void Device::runCheck() {
  auto onDeviceError = [](WGPUErrorType type, char const *message,
                          void * /* pUserData */) {
    std::cout << "Uncaptured device error: type " << type;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };
  // Error check callback
  wgpuDeviceSetUncapturedErrorCallback(wgpu_device, onDeviceError, nullptr);
  inspectDevice(wgpu_device);
}
