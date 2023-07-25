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

#include "adapter/adapter.hpp"
#include "command/command.hpp"
#include "device/device.hpp"
#include "instance/instance.hpp"
#include "queue/queue.hpp"
#include "renderpass/renderpass.hpp"
#include "surface/surface.hpp"
#include "swapchain/swapchain.hpp"
#include "window/window.hpp"
#include <common.h>
#include <webgpu-utils.h>

#define UNUSED(x) (void)x;

int main(int, char **) {
  // Initialisation
  Instance instance;
  Window window(640, 480, "Learn WebGPU");
  Surface surface(instance.wgpu_instance, window.glfw_window);
  Adapter adapter(instance.wgpu_instance, window.glfw_window);
  Device device(adapter.wgpu_adapter);
  device.runCheck(); // inspect device before continuing
  Queue queue(device.wgpu_device);
  SwapChain swapchain(surface.wgpu_surface, adapter.wgpu_adapter,
                      device.wgpu_device);

  while (!glfwWindowShouldClose(window.glfw_window)) {
    glfwPollEvents();
    WGPUTextureView nextTexture =
        wgpuSwapChainGetCurrentTextureView(swapchain.wgpu_swapchain);
    if (!nextTexture) {
      std::cerr << "Cannot acquire next swap chain texture" << std::endl;
      break;
    }
    // std::cout << "nextTexture: " << nextTexture << std::endl;

    Command::Encoder cmd_encoder(device.wgpu_device);

    RenderPass renderpass(cmd_encoder.wgpu_cmd_encoder, nextTexture);
    renderpass.endRenderPassEncoder();
    renderpass.releaseView(nextTexture);

    Command::Buffer cmd_buffer;
    cmd_buffer.setupCommandBufferDescriptor();
    cmd_buffer.buildFinishEncoderCmd(cmd_encoder.wgpu_cmd_encoder);

    wgpuQueueSubmit(queue.wgpu_queue, 1, &cmd_buffer.wgpu_cmd_buffer);

    // We can tell the swap chain to present the next texture.
    wgpuSwapChainPresent(swapchain.wgpu_swapchain);
  }

  glfwTerminate();

  return 0;
}
