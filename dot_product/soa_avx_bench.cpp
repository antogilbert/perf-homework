#include <benchmark/benchmark.h>

#include <cstdlib>
#include <immintrin.h>

#include <iostream>
#include <new>
#include <random>

#include "consts.h"

namespace soa_avx {
void dot_product(float *xxs, float *yys, float *ans, size_t size) {

  for (size_t i = 0; i + AVX_BLOCK <= size; i += AVX_BLOCK) {
    __m256 xs = _mm256_load_ps(&xxs[i]);
    __m256 ys = _mm256_load_ps(&yys[i]);
    // benchmark::DoNotOptimize(xs);
    // benchmark::DoNotOptimize(ys);
    // benchmark::ClobberMemory();

    __m256 res = _mm256_add_ps(_mm256_mul_ps(xs, xs), _mm256_mul_ps(ys, ys));
    // benchmark::DoNotOptimize(res);
    // benchmark::ClobberMemory();

    _mm256_store_ps(&ans[i], res);
    // benchmark::ClobberMemory();
  }
}
} // namespace soa_avx

float *allocate(size_t size) {
  void *mem = std::aligned_alloc(ALIGNMENT, size * sizeof(float));
  if (!mem)
    throw std::bad_alloc();

  return static_cast<float *>(mem);
}

static void BM_soa_avx_dot_prod(benchmark::State &state) {
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  const size_t size = state.range(0);
  float *xs = allocate(size);
  float *ys = allocate(size);

  for (int i = 0; i < size; ++i) {
    xs[i] = dist(rng);
    ys[i] = dist(rng);
  }

  float *ans = allocate(size);

  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    soa_avx::dot_product(xs, ys, ans, size);
    benchmark::DoNotOptimize(ans);
    benchmark::ClobberMemory();
  }

  free(xs);
  free(ys);
  free(ans);
}

// Register the function as a benchmark
BENCHMARK(BM_soa_avx_dot_prod)
    ->Arg(1 << 12)
    ->Arg(1 << 14)
    ->Arg(1 << 16)
    ->Arg(1 << 18)
    ->Arg(1 << 20)
    ->Arg(1 << 22)
    ->Arg(1 << 24)
    ->Arg(1 << 26);

namespace soa_struct {
void dot_product(std::vector<float> &xs, std::vector<float> &ys,
                 std::vector<float> &ans, size_t size) {
  for (int i = 0; i < size; ++i) {
    ans[i] = xs[i] * xs[i] + ys[i] * ys[i];
  }
}
} // namespace soa_struct

static void BM_soa_straight(benchmark::State &state) {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  const size_t size = state.range(0);
  std::vector<float> xs;
  std::vector<float> ys;
  xs.resize(size);
  ys.resize(size);

  for (int i = 0; i < size; ++i) {
    xs[i] = dist(rng);
    ys[i] = dist(rng);
  }

  std::vector<float> ans;
  ans.resize(size);

  for (auto _ : state) {
    soa_struct::dot_product(xs, ys, ans, size);
    benchmark::DoNotOptimize(ans.data());
    benchmark::ClobberMemory();
  }
}

BENCHMARK(BM_soa_straight)
    ->Arg(1 << 12)
    ->Arg(1 << 14)
    ->Arg(1 << 16)
    ->Arg(1 << 18)
    ->Arg(1 << 20)
    ->Arg(1 << 22)
    ->Arg(1 << 24)
    ->Arg(1 << 26);

// Run the benchmark
BENCHMARK_MAIN();
