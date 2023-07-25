#include "command.hpp"

void Command::Encoder::setupEncoderDesciptor(){
    wgpu_cmd_encoder_desc.nextInChain = nullptr;
    wgpu_cmd_encoder_desc.label = "Command Encoder";
}

void Command::Buffer::setupCommandBufferDescriptor() {
    wgpu_cmd_buffer_desc.nextInChain = nullptr;
    wgpu_cmd_buffer_desc.label = "Command buffer";
}

void Command::Buffer::buildFinishEncoderCmd(WGPUCommandEncoder encoder) {
    wgpu_cmd_buffer = wgpuCommandEncoderFinish(encoder, &wgpu_cmd_buffer_desc);
}
