#include <immintrin.h>

#include <random>
#include <vector>

#include "consts.h"

namespace soa_struct {
void dot_product(std::vector<float> &xxs, std::vector<float> &yys,
                 std::vector<float> &ans) {
  const size_t elems = xxs.size();

  for (size_t i = 0; i < elems - AVX_BLOCK; i += AVX_BLOCK) {
    __m256 xs = _mm256_loadu_ps(&xxs[i]);
    __m256 ys = _mm256_loadu_ps(&yys[i]);

    __m256 res = _mm256_add_ps(_mm256_mul_ps(xs, xs), _mm256_mul_ps(ys, ys));
    _mm256_storeu_ps(&ans[i], res);
  }
}
} // namespace soa_struct

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  std::vector<float> xs;
  std::vector<float> ys;
  xs.resize(SIZE);
  ys.resize(SIZE);

  for (int i = 0; i < SIZE; ++i) {
    xs[i] = dist(rng);
    ys[i] = dist(rng);
  }

  std::vector<float> ans;
  ans.resize(SIZE);

  soa_struct::dot_product(xs, ys, ans);
  return 0;
}
