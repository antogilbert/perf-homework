#pragma once

#include <cstddef>
constexpr size_t SIZE = 8'388'608;
constexpr size_t AVX_SIZE = 8'388'608;
constexpr size_t AVX_BITS = 256; // machine supports only avx2
constexpr size_t AVX_BLOCK = AVX_BITS / 8 / sizeof(float);

// TODO: try to convert all code using intrinsics _mm512_load & _mm512_mul (512
// is the num of bits) look at intrinsics
