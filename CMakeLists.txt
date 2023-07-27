cmake_minimum_required(VERSION 3.0...3.25)

project(
  LearnWebGPU
  VERSION 0.1.0
  LANGUAGES CXX C)

# setup
include(utils.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    # Add any other imgui source files here if needed
)

# add lib, generate executable
add_subdirectory(lib/glfw)
add_subdirectory(lib/webgpu)
add_subdirectory(lib/glfw3webgpu)

# Create the imgui library
add_library(imgui ${IMGUI_SOURCES})

# Set the include directories for the library
add_executable(App src/main.cpp src/webgpu-utils.cpp)

# linking
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(App PRIVATE headers)
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
