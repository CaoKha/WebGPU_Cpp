/**
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://eliemichel.github.io/LearnWebGPU
 *
 * MIT License
 * Copyright (c) 2022-2023 Elie Michel
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../headers/webgpu-utils.h"

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#include <webgpu/webgpu.h>
#include "window/window.hpp"

#include <cassert>
#include <iostream>

#define UNUSED(x) (void)x;

int main(int, char **) {
  // Init WGPU Instance
  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;
  WGPUInstance instance = wgpuCreateInstance(&desc);
  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
    return 1;
  }

  // Init Window
  Window window(640, 480, "Learn WebGPU");

  // Init adapter
  std::cout << "Requesting adapter..." << std::endl;
  WGPUSurface surface = glfwGetWGPUSurface(instance, window.glfw_window);
  WGPURequestAdapterOptions adapterOpts = {};
  adapterOpts.nextInChain = nullptr;
  adapterOpts.compatibleSurface = glfwGetWGPUSurface(instance, window.glfw_window);
  WGPUAdapter adapter = requestAdapter(instance, &adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  // Init device
  std::cout << "Requesting device..." << std::endl;
  WGPUDeviceDescriptor deviceDesc = {};
  deviceDesc.nextInChain = nullptr;
  deviceDesc.label = "My Device"; // anything works here, that's your call
  deviceDesc.requiredFeaturesCount =
      0;                               // we do not require any specific feature
  deviceDesc.requiredLimits = nullptr; // we do not require any specific limit
  deviceDesc.defaultQueue.nextInChain = nullptr;
  deviceDesc.defaultQueue.label = "The default queue";
  WGPUDevice device = requestDevice(adapter, &deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  // Add a callback that gets executed upon errors in our use of the device
  auto onDeviceError = [](WGPUErrorType type, char const *message,
                          void * /* pUserData */) {
    std::cout << "Uncaptured device error: type " << type;
    if (message)
      std::cout << " (" << message << ")";
    std::cout << std::endl;
  };
  // Error check callback
  wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError,
                                       nullptr /* pUserData */);
  inspectDevice(device);

  // Get the main and only command queue used to send instruction to the GPU
  WGPUQueue queue = wgpuDeviceGetQueue(device);

  std::cout << "Creating swapchain device... " << std::endl;

  // We describe the Swap Chain that is used to present rendered textures on
  // screen. Note that it is specific to a given window size so don't resize.
  WGPUSwapChainDescriptor swapChainDesc = {};
  swapChainDesc.width = 640;
  swapChainDesc.height = 480;

  // Like buffers, texture are allocated for a  specific usage. In our case,
  // we will use them as the target of a Render Pass so it needs to be created
  // with the `RenderAttachment` usage flag.
  swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;

#ifdef WEBGPU_BACKEND_WGPU
  WGPUTextureFormat swapChainFormat =
      wgpuSurfaceGetPreferredFormat(surface, adapter);
#else
  WGPUTextureFormat swapChainFormat = WGPUTextureFormat_BGRA8Unorm;
#endif
  swapChainDesc.format = swapChainFormat;

  // FIFO -> texture is always the oldest one, like a regular queue
  swapChainDesc.presentMode = WGPUPresentMode_Fifo;

  // create the Swap Chain
  WGPUSwapChain swapChain =
      wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);

  std::cout << "SwapChain: " << swapChain << std::endl;

  while (!glfwWindowShouldClose(window.glfw_window)) {
    glfwPollEvents();
    WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
    if (!nextTexture) {
      std::cerr << "Cannot acquire next swap chain texture" << std::endl;
      break;
    }
    // std::cout << "nextTexture: " << nextTexture << std::endl;
    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    commandEncoderDesc.nextInChain = nullptr;
    commandEncoderDesc.label = "Command Encoder";
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);

    // Describe a render pass, which targets the texture view
    WGPURenderPassDescriptor renderPassDesc = {};

    WGPURenderPassColorAttachment renderPassColorAttachment = {};
    // The attachment is tighed to the view returned by the swap chain, so that
    // the render pass draws directly on screen.
    renderPassColorAttachment.view = nextTexture;
    // Not relevant here because we do not use multi-sampling
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = WGPULoadOp_Clear;
    renderPassColorAttachment.storeOp = WGPUStoreOp_Store;
    renderPassColorAttachment.clearValue = WGPUColor{0.9, 0.1, 0.2, 1.0};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    // No depth buffer for now
    renderPassDesc.depthStencilAttachment = nullptr;

    // We do not use timers for now neither
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;

    renderPassDesc.nextInChain = nullptr;

    // Create a render pass. We end it immediately because we use its built-in
    // mechanism for clearing the screen when it begins (see descriptor).
    WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderEnd(renderPass);

    wgpuTextureViewRelease(nextTexture);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = nullptr;
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(queue, 1, &command);

    // We can tell the swap chain to present the next texture.
    wgpuSwapChainPresent(swapChain);
  }

  wgpuSwapChainRelease(swapChain);
  wgpuDeviceRelease(device);
  wgpuAdapterRelease(adapter);
  wgpuInstanceRelease(instance);
  // glfwDestroyWindow(window.glfw_window);
  glfwTerminate();

  return 0;
}
