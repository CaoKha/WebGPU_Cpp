cmake_minimum_required(VERSION 3.0...3.25)

project(
  LearnWebGPU
  VERSION 0.1.0
  LANGUAGES CXX C)

# setup 
include(utils.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add lib, generate executable
add_subdirectory(lib/glfw)
add_executable(App src/main.cpp)

# linking
target_link_libraries(App PRIVATE glfw)
set_target_properties(App PROPERTIES CXX_STANDARD 17)

# move compile_commands.json to project root
add_custom_target(
  copy-compile-commands ALL
  ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_LIST_DIR})
