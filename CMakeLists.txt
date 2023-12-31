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
# We add an option to enable different settings when developping the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)

# add lib, generate executable
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/webgpu)
add_subdirectory(vendor/glfw3webgpu)
add_subdirectory(vendor/magic_enum)
add_subdirectory(vendor/glm)

include_directories(vendor/imgui)

# Create the imgui library
add_library(imgui ${IMGUI_SOURCES})

# Set the include directories for the library
add_executable(App src/main.cpp )

target_include_directories(App PRIVATE headers imgui)
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu magic_enum glm)
set_target_properties(
  App PROPERTIES CXX_STANDARD 17 VS_DEBUGGER_ENVIRONMENT
                                 "DAWN_DEBUG_BREAK_ON_ERROR=1")
target_treat_all_warning_as_errors(App)
target_copy_webgpu_binaries(App) # provided by webgpu

# load file from folders
if(DEV_MODE)
  # In dev mode, we load resources from the source tree, so that when we
  # dynamically edit resources (like shaders), these are correctly versionned.
  target_compile_definitions(
    App PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
else()
  # In release mode, we just load resources relatively to wherever the
  # executable is launched from, so that the binary is portable
  target_compile_definitions(App PRIVATE RESOURCE_DIR="./resources")
endif()


# move compile_commands.json to project root
add_custom_target(
  copy-compile-commands ALL
  ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_LIST_DIR})
