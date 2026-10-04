#include <benchmark/benchmark.h>

#include <random>
#include <string>

#include "spsc.h"

constexpr size_t KB = 1024;
constexpr size_t MB = 1024 * KB;
constexpr size_t MIN_TASK_SIZE = 20;
constexpr size_t MAX_TASK_SIZE = 2 * KB;
constexpr size_t MAX_TASKS = 255;  // so that max mem required is divisible by MIN_SIZE

// Adapted to 256 chars -> 256 bytes
constexpr std::string_view LITANY =
    "I must not fear. Fear is the mind-killer.Fear is the little-death that brings total "
    "obliteration.I will permit it to pass over me and through me.When it has gone past I will "
    "turn the inner eye to see its path. Where the fear has gone there will be nothing.";
static_assert(LITANY.size() == 256);

static const std::string ONE_KB_PKT =
    std::string(LITANY) + std::string(LITANY) + std::string(LITANY) + std::string(LITANY);
static const std::string ONE_AND_QUART_KB_PKT = ONE_KB_PKT + std::string(LITANY);
static const std::string ONE_AND_HALF_KB_PKT = ONE_AND_QUART_KB_PKT + std::string(LITANY);
static const std::string ONE_AND_3QUART_KB_PKT = ONE_AND_HALF_KB_PKT + std::string(LITANY);
static const std::string TWO_KB_PKT = ONE_KB_PKT + ONE_KB_PKT;

static const std::array<std::string, 5> PKTS = {
    ONE_KB_PKT, ONE_AND_QUART_KB_PKT, ONE_AND_HALF_KB_PKT, ONE_AND_3QUART_KB_PKT, TWO_KB_PKT,
};

// I've seen what makes you cheer.
constexpr std::string_view BOOS = "YourBoosMeanNothing.";
static_assert(BOOS.size() == 20);

constexpr int RATIO = 10;

static void BM_spsc(benchmark::State& state) {
  spscq<MAX_TASKS * MAX_TASK_SIZE, MIN_TASK_SIZE> q;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<uint8_t> dist(0, 4);
  for (auto _ : state) {
    auto ops = state.range(0);
    for (int i = 0; i < ops; i += RATIO) {
      benchmark::DoNotOptimize(BOOS.data());
      q.write({BOOS.size(), reinterpret_cast<const std::byte*>(BOOS.data())});
      benchmark::ClobberMemory();
      q.read();
      q.commit_read();
      for (int j = 0; j < RATIO; ++j) {
        const auto& pkt = PKTS[dist(rng)];
        benchmark::DoNotOptimize(pkt.data());
        q.write({pkt.size(), reinterpret_cast<const std::byte*>(pkt.data())});
        benchmark::ClobberMemory();
        q.read();
        q.commit_read();
      }
    }
  }
}

BENCHMARK(BM_spsc)
    ->Arg(1 << 12)
    ->Arg(1 << 14)
    ->Arg(1 << 16)
    ->Arg(1 << 18)
    ->Arg(1 << 20)
    ->Arg(1 << 22)
    ->Arg(1 << 24)
    ->Arg(1 << 26)
    ->Arg(1 << 28);
// ->Arg(1 << 30)
// ->Arg(1 << 31);

// Run the benchmark
BENCHMARK_MAIN();
