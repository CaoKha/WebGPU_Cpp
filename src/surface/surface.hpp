#pragma once
#include <common.h>

class Surface {
public:
  WGPUSurface wgpu_surface;
  Surface(WGPUInstance instance, GLFWwindow* window) {
    wgpu_surface =
        glfwGetWGPUSurface(instance, window);
  };
  ~Surface(){};
};
