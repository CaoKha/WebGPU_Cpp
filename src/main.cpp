/**
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://github.com/eliemichel/LearnWebGPU
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

// An optional library that makes displaying enum values much easier
#include <cstddef>
#include <magic_enum.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <webgpu/webgpu.h>
#include <common.h>

#define WEBGPU_CPP_IMPLEMENTATION
#include <webgpu/webgpu.hpp>

using namespace wgpu;
namespace fs = std::filesystem;

/**
 * The same structure as in the shader
 */
struct MyUniforms {
  // offset = 0 * sizeof(vec4f)
  std::array<float, 4> color;
  // offset = 4 * sizeof(f32) = 16
  float time;
  float _pad[3]; // we don't use this array but we need it cuz the constraint of
                 // uniform layout
  // time -> sizeof(float) = 4 bytes -> need an array of 3 floats (size = 3*4 =
  // 12 bytes) in order to make the  total size of `MyUniforms` struct = (16 + 4
  // + 12) % 16 == 0
};
//  Have the compiler check byte alignment
static_assert(sizeof(MyUniforms) % 16 == 0);

/**
 * Round 'value' up to the next multiplier of 'step'.
 */
uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
  uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
  return step * divide_and_ceil;
}

ShaderModule loadShaderModule(const fs::path &path, Device device);
bool loadGeometry(const fs::path &path, std::vector<float> &pointData,
                  std::vector<uint16_t> &indexData, int dimensions);

int main(int, char **) {
  // Instance
  Instance instance = createInstance(InstanceDescriptor{});
  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
    return 1;
  }

  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW!" << std::endl;
    return 1;
  }

  // Window
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow *window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
  if (!window) {
    std::cerr << "Could not open window!" << std::endl;
    return 1;
  }

  // Adapter
  std::cout << "Requesting adapter..." << std::endl;
  Surface surface = glfwGetWGPUSurface(instance, window);
  RequestAdapterOptions adapterOpts;
  adapterOpts.compatibleSurface = surface;
  Adapter adapter = instance.requestAdapter(adapterOpts);
  std::cout << "Got adapter: " << adapter << std::endl;

  // limit init
  SupportedLimits supportedLimits;
  adapter.getLimits(
      &supportedLimits); // getLimits(&supportedLimits) -> actually initialize
                         // `supportedLimits` correspond to `adapter`

  // Device
  std::cout << "Requesting device..." << std::endl;
  RequiredLimits requiredLimits = Default;
  requiredLimits.limits.maxVertexAttributes = 2;
  requiredLimits.limits.maxVertexBuffers = 1;
  requiredLimits.limits.maxBufferSize = 15 * 5 * sizeof(float);
  requiredLimits.limits.maxVertexBufferArrayStride = 6 * sizeof(float);
  requiredLimits.limits.minStorageBufferOffsetAlignment =
      supportedLimits.limits.minStorageBufferOffsetAlignment;
  requiredLimits.limits.minUniformBufferOffsetAlignment =
      supportedLimits.limits.minUniformBufferOffsetAlignment;
  requiredLimits.limits.maxInterStageShaderComponents = 3;
  requiredLimits.limits.maxBindGroups = 1;
  requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
  // Uniform structs have a size of maximum 16 float
  requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4;
  requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
  requiredLimits.limits.maxTextureDimension1D = 480;
  requiredLimits.limits.maxTextureDimension2D = 640;
  requiredLimits.limits.maxTextureArrayLayers = 1;

  DeviceDescriptor deviceDesc;
  deviceDesc.label = "My Device";
  deviceDesc.requiredFeaturesCount = 0;
  deviceDesc.requiredLimits = &requiredLimits;
  deviceDesc.defaultQueue.label = "The default queue";
  Device device = adapter.requestDevice(deviceDesc);
  std::cout << "Got device: " << device << std::endl;

  // Get device limits
  SupportedLimits deviceSupportedLimits;
  device.getLimits(&deviceSupportedLimits);
  Limits deviceLimits = deviceSupportedLimits.limits;

  // Add an error callback for more debug info
  auto h = device.setUncapturedErrorCallback(
      [](ErrorType type, char const *message) {
        std::cout << "Device error: type " << type;
        if (message)
          std::cout << " (message: " << message << ")";
        std::cout << std::endl;
      });

  // Queue
  Queue queue = device.getQueue();

  // Swap chain
  std::cout << "Creating swapchain..." << std::endl;
#ifdef WEBGPU_BACKEND_WGPU
  TextureFormat swapChainFormat = surface.getPreferredFormat(adapter);
#else
  TextureFormat swapChainFormat = TextureFormat::BGRA8Unorm;
#endif
  SwapChainDescriptor swapChainDesc;
  swapChainDesc.width = 640;
  swapChainDesc.height = 480;
  swapChainDesc.usage = TextureUsage::RenderAttachment;
  swapChainDesc.format = swapChainFormat;
  swapChainDesc.presentMode = PresentMode::Fifo;
  SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);
  std::cout << "Swapchain: " << swapChain << std::endl;
  // If the format contains "Srgb", we will have a gamma issue
  std::cout << "Swapchain format: "
            << magic_enum::enum_name<WGPUTextureFormat>(swapChainFormat)
            << std::endl;

  // Shader
  std::cout << "Creating shader module..." << std::endl;
  ShaderModule shaderModule =
      loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);
  std::cout << "Shader module: " << shaderModule << std::endl;

  // Render pipeline
  std::cout << "Creating render pipeline..." << std::endl;
  RenderPipelineDescriptor pipelineDesc;

  // Vertex fetch
  std::vector<VertexAttribute> vertexAttribs(2);

  // Position attribute
  vertexAttribs[0].shaderLocation = 0;
  vertexAttribs[0].format = VertexFormat::Float32x3;
  vertexAttribs[0].offset = 0;

  // Color attribute
  vertexAttribs[1].shaderLocation = 1;
  vertexAttribs[1].format = VertexFormat::Float32x3;
  vertexAttribs[1].offset = 3 * sizeof(float);

  VertexBufferLayout vertexBufferLayout;
  vertexBufferLayout.attributeCount = (uint32_t)vertexAttribs.size();
  vertexBufferLayout.attributes = vertexAttribs.data();
  vertexBufferLayout.arrayStride = 6 * sizeof(float);
  vertexBufferLayout.stepMode = VertexStepMode::Vertex;

  pipelineDesc.vertex.bufferCount = 1;
  pipelineDesc.vertex.buffers = &vertexBufferLayout;

  pipelineDesc.vertex.module = shaderModule;
  pipelineDesc.vertex.entryPoint = "vs_main";
  pipelineDesc.vertex.constantCount = 0;
  pipelineDesc.vertex.constants = nullptr;

  pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
  pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
  pipelineDesc.primitive.frontFace = FrontFace::CCW;
  pipelineDesc.primitive.cullMode = CullMode::None;

  FragmentState fragmentState;
  pipelineDesc.fragment = &fragmentState;
  fragmentState.module = shaderModule;
  fragmentState.entryPoint = "fs_main";
  fragmentState.constantCount = 0;
  fragmentState.constants = nullptr;

  BlendState blendState;
  blendState.color.srcFactor = BlendFactor::SrcAlpha;
  blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
  blendState.color.operation = BlendOperation::Add;
  blendState.alpha.srcFactor = BlendFactor::Zero;
  blendState.alpha.dstFactor = BlendFactor::One;
  blendState.alpha.operation = BlendOperation::Add;

  ColorTargetState colorTarget;
  colorTarget.format = swapChainFormat;
  colorTarget.blend = &blendState;
  colorTarget.writeMask = ColorWriteMask::All;

  fragmentState.targetCount = 1;
  fragmentState.targets = &colorTarget;

  // We setup a depth buffer state for the render pipeline
  DepthStencilState depthStencilState = Default;
  // Keep a fragment only if its depth is lower than the previously blended one
  depthStencilState.depthCompare = CompareFunction::Less;
  // Each time a fragment is blended into the target, we update the value of
  // Z-buffer
  depthStencilState.depthWriteEnabled = true;
  // Store the format in a variable as later parts of the code depend on it
  TextureFormat depthTextureFormat = TextureFormat::Depth24Plus;
  depthStencilState.format = depthTextureFormat;
  // Deactivate the stencil alltogether, never read or write to the stencil
  // buffer
  depthStencilState.stencilReadMask = 0;
  depthStencilState.stencilWriteMask = 0;

  pipelineDesc.depthStencil = &depthStencilState;

  pipelineDesc.multisample.count = 1;
  pipelineDesc.multisample.mask = ~0u;
  pipelineDesc.multisample.alphaToCoverageEnabled = false;

  // Binding Layout
  BindGroupLayoutEntry bindingLayout = Default;
  bindingLayout.binding = 0;
  bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
  bindingLayout.buffer.type = BufferBindingType::Uniform;
  bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);
  bindingLayout.buffer.hasDynamicOffset = true;

  // Bind group Layout
  BindGroupLayoutDescriptor bindGroupLayoutDesc;
  bindGroupLayoutDesc.entryCount = 1;
  bindGroupLayoutDesc.entries = &bindingLayout;
  BindGroupLayout bindGroupLayout =
      device.createBindGroupLayout(bindGroupLayoutDesc);

  PipelineLayoutDescriptor layoutDesc;
  layoutDesc.bindGroupLayoutCount = 1;
  layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *)&bindGroupLayout;
  PipelineLayout layout = device.createPipelineLayout(layoutDesc);
  pipelineDesc.layout = layout;

  RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);
  std::cout << "Render pipeline: " << pipeline << std::endl;

  // Create the depth texture
  TextureDescriptor depthTextureDesc;
  depthTextureDesc.dimension = TextureDimension::_2D;
  depthTextureDesc.format = depthTextureFormat;
  depthTextureDesc.mipLevelCount = 1;
  depthTextureDesc.sampleCount = 1;
  depthTextureDesc.size = {640, 480, 1};
  depthTextureDesc.usage = TextureUsage::RenderAttachment;
  depthTextureDesc.viewFormatCount = 1;
  depthTextureDesc.viewFormats = (WGPUTextureFormat *)&depthTextureFormat;
  Texture depthTexture = device.createTexture(depthTextureDesc);
  std::cout << "Depth Texture: " << depthTexture << std::endl;

  // Create the view of depth texture manipulated by the rasterizer
  TextureViewDescriptor depthTextureViewDesc;
  depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
  depthTextureViewDesc.baseArrayLayer = 0;
  depthTextureViewDesc.arrayLayerCount = 1;
  depthTextureViewDesc.baseMipLevel = 0;
  depthTextureViewDesc.mipLevelCount = 1;
  depthTextureViewDesc.dimension = TextureViewDimension::_2D;
  depthTextureViewDesc.format = depthTextureFormat;
  TextureView depthTextureView = depthTexture.createView(depthTextureViewDesc);
  std::cout << "Depth texture view: " << depthTextureView << std::endl;

  std::vector<float> pointData;
  std::vector<uint16_t> indexData;

  bool success =
      loadGeometry(RESOURCE_DIR "/pyramid.txt", pointData, indexData, 3);
  if (!success) {
    std::cerr << "Could not load geometry!" << std::endl;
    return 1;
  }

  // Create vertex buffer
  BufferDescriptor bufferDesc;
  bufferDesc.size = pointData.size() * sizeof(float);
  bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
  bufferDesc.mappedAtCreation = false;
  Buffer vertexBuffer = device.createBuffer(bufferDesc);
  queue.writeBuffer(vertexBuffer, 0, pointData.data(), bufferDesc.size);

  // Index Buffer alignment
  int indexCount = static_cast<int>(indexData.size());

  // Create index buffer
  // (we reuse the bufferDesc initialized for the vertexBuffer)
  bufferDesc.size = indexData.size() * sizeof(float);
  bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
  bufferDesc.mappedAtCreation = false;
  Buffer indexBuffer = device.createBuffer(bufferDesc);
  queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);

  uint32_t uniformStride = ceilToNextMultiple(
      (uint32_t)sizeof(MyUniforms),
      (uint32_t)deviceLimits.minUniformBufferOffsetAlignment);

  // Uniform buffer
  bufferDesc.size = uniformStride + sizeof(MyUniforms);
  bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
  bufferDesc.mappedAtCreation = false;
  Buffer uniformBuffer = device.createBuffer(bufferDesc);

  // Upload initial value of the uniforms
  MyUniforms uniforms;
  uniforms.time = 1.0f;
  uniforms.color = {0.0f, 1.0f, 0.4f, 1.0f};
  queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(MyUniforms));

  // Upload second value
  uniforms.time = -1.0f;
  uniforms.color = {1.0f, 1.0f, 1.0f, 0.7f};
  queue.writeBuffer(uniformBuffer, uniformStride, &uniforms,
                    sizeof(MyUniforms));

  // Create a binding
  BindGroupEntry binding;
  binding.binding = 0;
  binding.buffer = uniformBuffer;
  binding.offset = 0;
  binding.size = sizeof(MyUniforms);

  // A bindgroup contains one or multiple bindings
  BindGroupDescriptor bindGroupDesc;
  bindGroupDesc.layout = bindGroupLayout;
  bindGroupDesc.entryCount = bindGroupLayoutDesc.entryCount;
  bindGroupDesc.entries = &binding;
  BindGroup bindGroup = device.createBindGroup(bindGroupDesc);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Update uniform buffer
    uniforms.time =
        static_cast<float>(2.0 * glfwGetTime()); // glfwGetTime returns a double
    queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &uniforms.time,
                      sizeof(MyUniforms::time));

    uniforms.color = {1.0f, 0.5f, 0.0f, 1.0f};
    queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, color),
                      &uniforms.color, sizeof(MyUniforms::color));

    uniforms.time =
        static_cast<float>(2.0 * glfwGetTime()); // glfwGetTime returns a double
    queue.writeBuffer(uniformBuffer,
                      offsetof(MyUniforms, time) + sizeof(MyUniforms),
                      &uniforms.time, sizeof(MyUniforms::time));

    uniforms.color = {0.0f, 1.0f, 0.5f, 1.0f};
    queue.writeBuffer(uniformBuffer,
                      offsetof(MyUniforms, color) + sizeof(MyUniforms),
                      &uniforms.color, sizeof(MyUniforms::color));

    TextureView nextTexture = swapChain.getCurrentTextureView();
    if (!nextTexture) {
      std::cerr << "Cannot acquire next swap chain texture" << std::endl;
      return 1;
    }

    CommandEncoderDescriptor commandEncoderDesc;
    commandEncoderDesc.label = "Command Encoder";
    CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);

    RenderPassDescriptor renderPassDesc;

    RenderPassColorAttachment renderPassColorAttachment{};
    renderPassColorAttachment.view = nextTexture;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = LoadOp::Clear;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    renderPassColorAttachment.clearValue = Color{0.05, 0.05, 0.05, 1.0};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;

    RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.view = depthTextureView;
    depthStencilAttachment.depthClearValue = 1.0f;
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false;

    depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
    depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
#else
    depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = LoadOp::Undefined;
#endif
    depthStencilAttachment.stencilReadOnly = true;

    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;
    RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

    renderPass.setPipeline(pipeline);

    // Set both vertex and index buffers
    renderPass.setVertexBuffer(0, vertexBuffer, 0,
                               pointData.size() * sizeof(float));
    // The second argument must correspond to the choice of uint16_t or uint32_t
    // we've done when creating the index buffer.
    renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0,
                              indexData.size() * sizeof(uint16_t));

    uint32_t dynamicOffset = 0;

    // set binding group
    dynamicOffset = 0 * uniformStride;
    renderPass.setBindGroup(0, bindGroup, 1, &dynamicOffset);
    // Replace `draw()` with `drawIndexed()` and `vertexCount` with `indexCount`
    // The extra argument is an offset within the index buffer.
    renderPass.drawIndexed(indexCount, 1, 0, 0, 0);

    // Set another binding group
    // dynamicOffset = 1 * uniformStride;
    // renderPass.setBindGroup(0, bindGroup, 1, &dynamicOffset);
    // renderPass.drawIndexed(indexCount, 1, 0, 0, 0);

    renderPass.end();

    nextTexture.release();

    CommandBufferDescriptor cmdBufferDescriptor;
    cmdBufferDescriptor.label = "Command buffer";
    CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    queue.submit(command);

    swapChain.present();

#ifdef WEBGPU_BACKEND_DAWN
    // Check for pending error callbacks
    device.tick();
#endif
  }

  vertexBuffer.destroy();
  vertexBuffer.release();
  indexBuffer.destroy();
  indexBuffer.release();

  depthTextureView.release();
  depthTexture.destroy();
  depthTexture.release();

  swapChain.release();
  device.release();
  adapter.release();
  instance.release();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

ShaderModule loadShaderModule(const fs::path &path, Device device) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return nullptr;
  }
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  std::string shaderSource(size, ' ');
  file.seekg(0);
  file.read(shaderSource.data(), size);

  ShaderModuleWGSLDescriptor shaderCodeDesc;
  shaderCodeDesc.chain.next = nullptr;
  shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
  shaderCodeDesc.code = shaderSource.c_str();
  ShaderModuleDescriptor shaderDesc;
  shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
  shaderDesc.hintCount = 0;
  shaderDesc.hints = nullptr;
#endif

  return device.createShaderModule(shaderDesc);
}

bool loadGeometry(const fs::path &path, std::vector<float> &pointData,
                  std::vector<uint16_t> &indexData, int dimensions) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return false;
  }

  pointData.clear();
  indexData.clear();

  enum class Section {
    None,
    Points,
    Indices,
  };
  Section currentSection = Section::None;

  float value;
  uint16_t index;
  std::string line;
  while (!file.eof()) {
    getline(file, line);
    if (line == "[points]") {
      currentSection = Section::Points;
    } else if (line == "[indices]") {
      currentSection = Section::Indices;
    } else if (line[0] == '#' || line.empty()) {
      // Do nothing, this is a comment
    } else if (currentSection == Section::Points) {
      std::istringstream iss(line);
      // Get x, y, r, g, b
      for (int i = 0; i < dimensions + 3; ++i) {
        iss >> value;
        pointData.push_back(value);
      }
    } else if (currentSection == Section::Indices) {
      std::istringstream iss(line);
      // Get corners #0 #1 and #2
      for (int i = 0; i < 3; ++i) {
        iss >> index;
        indexData.push_back(index);
      }
    }
  }
  return true;
}
