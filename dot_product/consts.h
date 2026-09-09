#pragma once

#include <cstddef>
constexpr size_t SIZE = 8'388'608;
constexpr size_t AVX_SIZE = 8'388'608;
constexpr size_t AVX_BITS = 256; // machine supports only avx2
constexpr size_t AVX_BLOCK = AVX_BITS / 8 / sizeof(float);
