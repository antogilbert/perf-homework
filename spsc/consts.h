#pragma once

#include <cstddef>

constexpr size_t KB = 1024;
constexpr size_t MB = 1024 * KB;

constexpr size_t MIN_TASK_SIZE = 20;
constexpr size_t MAX_TASK_SIZE = 2 * KB;
constexpr size_t MAX_TASKS = 256;  // so that max mem required is divisible by MIN_SIZE
