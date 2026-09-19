#include <cstdlib>
#include <immintrin.h>

#include <iostream>
#include <new>
#include <random>

#include "consts.h"

namespace soa_struct {
void dot_product(float *xxs, float *yys, float *ans) {

  for (size_t i = 0; i < AVX_SIZE - AVX_BLOCK; i += AVX_BLOCK) {
    __m256 xs = _mm256_load_ps(&xxs[i]);
    __m256 ys = _mm256_load_ps(&yys[i]);

    __m256 res = _mm256_add_ps(_mm256_mul_ps(xs, xs), _mm256_mul_ps(ys, ys));

    _mm256_store_ps(&ans[i], res);
  }
}
} // namespace soa_struct

float *allocate() {
  void *mem = std::aligned_alloc(ALIGNMENT, AVX_SIZE * sizeof(float));
  if (!mem)
    throw std::bad_alloc();

  return static_cast<float *>(mem);
}

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  float *xs = allocate();
  float *ys = allocate();

  for (int i = 0; i < AVX_SIZE; ++i) {
    xs[i] = dist(rng);
    ys[i] = dist(rng);
  }

  float *ans = allocate();

  soa_struct::dot_product(xs, ys, ans);

  free(xs);
  free(ys);
  free(ans);
  return 0;
}
