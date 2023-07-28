cmake_minimum_required(VERSION 3.0...3.25)

project(
  LearnWebGPU
  VERSION 0.1.0
  LANGUAGES CXX C)

# setup
include(utils.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    # Add any other imgui source files here if needed
)

# add lib, generate executable
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/webgpu)
add_subdirectory(vendor/glfw3webgpu)

include_directories(vendor/imgui)

# Create the imgui library
add_library(imgui ${IMGUI_SOURCES})

# Set the include directories for the library
add_executable(App src/main.cpp src/webgpu-utils.cpp)

target_include_directories(App PRIVATE headers imgui)
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)
set_target_properties(
  App PROPERTIES CXX_STANDARD 17 VS_DEBUGGER_ENVIRONMENT
                                 "DAWN_DEBUG_BREAK_ON_ERROR=1")
target_treat_all_warning_as_errors(App)
target_copy_webgpu_binaries(App) # provided by webgpu

# move compile_commands.json to project root
add_custom_target(
  copy-compile-commands ALL
  ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_LIST_DIR})
