cmake_minimum_required(VERSION 3.20)

project(slime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)

# Gives an error if the right compiler version of C++ can't be found
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(program
  src/main.cpp
  src/tgaimage.cpp
  src/obj_decoder.cpp
  src/drawing.cpp
  src/lighting.cpp
)

target_include_directories(program PRIVATE include)


