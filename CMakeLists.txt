cmake_minimum_required(VERSION 4.4)

project(perf-homework LANGUAGES CXX)

find_package(benchmark REQUIRED)
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(dot_product)
