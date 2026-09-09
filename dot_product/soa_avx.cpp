#include <immintrin.h>
#include <iostream>
#include <print>
#include <random>
#include <vector>

#include "consts.h"

struct Points {
  std::vector<float> xs;
  std::vector<float> ys;
  Points() : xs(AVX_SIZE), ys(AVX_SIZE) {}
};

namespace soa_avx {
void dot_product(const Points &points, std::vector<float> &ans) {
  const size_t elems = points.xs.size();

  for (size_t i = 0; i < elems - AVX_BLOCK; i += AVX_BLOCK) {
    __m256 xs = _mm256_loadu_ps(&points.xs[i]);
    __m256 ys = _mm256_loadu_ps(&points.ys[i]);

    __m256 res = _mm256_add_ps(_mm256_mul_ps(xs, xs), _mm256_mul_ps(ys, ys));
    _mm256_storeu_ps(&ans[i], res);
  }
}
} // namespace soa_avx

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  Points points;
  for (int i = 0; i < AVX_SIZE; ++i) {
    points.xs[i] = dist(rng);
    points.ys[i] = dist(rng);
  }

  std::vector<float> ans;
  ans.resize(AVX_SIZE);

  std::print("Problem size:\n");
  std::print(" - AVX BITS  {}\n", AVX_BITS);
  std::print(" - AVX BLOCK {}\n", AVX_BLOCK);
  std::print(" - AVX SIZE  {}\n", AVX_SIZE);
  std::print(" - 8 PACK ?  {}\n", AVX_SIZE % 8);
  soa_avx::dot_product(points, ans);
  return 0;
}
