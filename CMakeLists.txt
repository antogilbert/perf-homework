cmake_minimum_required(VERSION 4.4)

project(perf-homework LANGUAGES CXX)

# find_package(benchmark REQUIRED)
# list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")

add_subdirectory(dot_product)
