#pragma once
#include <common.h>

namespace Command {
class Encoder {
public:
  WGPUCommandEncoderDescriptor wgpu_cmd_encoder_desc = {};
  WGPUCommandEncoder wgpu_cmd_encoder;
  Encoder(WGPUDevice device) {
    setupEncoderDesciptor();
    wgpu_cmd_encoder =
        wgpuDeviceCreateCommandEncoder(device, &wgpu_cmd_encoder_desc);
  };
  ~Encoder(){};
  void setupEncoderDesciptor();
};

class Buffer {
public:
  WGPUCommandBufferDescriptor wgpu_cmd_buffer_desc = {};
  WGPUCommandBuffer wgpu_cmd_buffer;
  Buffer() {
    setupCommandBufferDescriptor();
    wgpu_cmd_buffer = nullptr;
  };
  ~Buffer(){};
  void setupCommandBufferDescriptor();
  void buildFinishEncoderCmd(WGPUCommandEncoder encoder);
};

} // namespace Command
