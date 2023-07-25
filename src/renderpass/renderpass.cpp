#include "renderpass.hpp"

void RenderPass::setupColorAttachment() {
  wgpu_rp_color_attach.view = nullptr;
  wgpu_rp_color_attach.resolveTarget = nullptr;
  wgpu_rp_color_attach.loadOp = WGPULoadOp_Clear;
  wgpu_rp_color_attach.storeOp = WGPUStoreOp_Store;
  wgpu_rp_color_attach.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
}

void RenderPass::getNextView(WGPUTextureView next_view) {
  wgpu_rp_color_attach.view = next_view;
}

void RenderPass::setupRenderPassDesciptor() {
  wgpu_rp_desc.colorAttachmentCount = 1;
  wgpu_rp_desc.colorAttachments = &wgpu_rp_color_attach;
  wgpu_rp_desc.depthStencilAttachment = nullptr;
  wgpu_rp_desc.timestampWriteCount = 0;
  wgpu_rp_desc.timestampWrites = nullptr;
  wgpu_rp_desc.nextInChain = nullptr;
}

void RenderPass::endRenderPassEncoder() {
  wgpuRenderPassEncoderEnd(wgpu_renderpass_encoder);
}

void RenderPass::releaseView(WGPUTextureView view) {
  wgpuTextureViewRelease(view);
}
