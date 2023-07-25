#pragma once
#include <common.h>
#include <webgpu/webgpu.h>


class RenderPass {
public:
  WGPURenderPassDescriptor wgpu_rp_desc = {};
  WGPURenderPassColorAttachment wgpu_rp_color_attach = {};
  WGPURenderPassEncoder wgpu_renderpass_encoder;
  RenderPass(WGPUCommandEncoder encoder, WGPUTextureView next_view) {
    setupColorAttachment();
    getNextView(next_view);
    setupRenderPassDesciptor();
    wgpu_renderpass_encoder =
        wgpuCommandEncoderBeginRenderPass(encoder, &wgpu_rp_desc);
  };
  ~RenderPass(){};

  void setupColorAttachment();
  void getNextView(WGPUTextureView next_view);
  void setupRenderPassDesciptor();
  void endRenderPassEncoder();
  void releaseView(WGPUTextureView view);
};
