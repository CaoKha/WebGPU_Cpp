#pragma once
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

class Window {
public:
  GLFWwindow *glfw_window;
  Window(int width, int height, const char *title) {
    if (!glfwInit()) {
      std::cerr << "Could not initialize GLFW!" << std::endl;
      exit(1);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!glfw_window) {
      std::cerr << "Could not open window!" << std::endl;
      exit(1);
    }
  }
  ~Window() { glfwDestroyWindow(glfw_window); }
};
