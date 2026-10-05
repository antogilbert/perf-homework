#include <benchmark/benchmark.h>

#include <random>
#include <string>

#include "consts.h"
#include "spsc.h"
#include "spsc_fix_frame.h"

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
  spscq<MAX_TASKS - 1> q;

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<uint8_t> dist(0, 4);
  for (auto _ : state) {
    auto ops = state.range(0);
    for (int i = 0; i < ops; i += RATIO) {
      benchmark::DoNotOptimize(BOOS.data());
      q.write({BOOS.size(), reinterpret_cast<const std::byte*>(BOOS.data())});
      benchmark::ClobberMemory();
      q.peek();
      q.commit_read();
      for (int j = 0; j < RATIO; ++j) {
        const auto& pkt = PKTS[dist(rng)];
        benchmark::DoNotOptimize(pkt.data());
        q.write({pkt.size(), reinterpret_cast<const std::byte*>(pkt.data())});
        benchmark::ClobberMemory();
        q.peek();
        q.commit_read();
      }
    }
  }
}

static void BM_spsc_fix_frame(benchmark::State& state) {
  // 256
  spscffq<MAX_TASKS> q;

  auto min_frame = FixedFrame();
  std::memcpy(&min_frame.data, BOOS.data(), BOOS.size());

  auto one_kb_frame = FixedFrame();
  std::memcpy(&one_kb_frame.data, ONE_KB_PKT.data(), ONE_KB_PKT.size());

  auto one_and_quart_kb_frame = FixedFrame();
  std::memcpy(&one_and_quart_kb_frame.data, ONE_AND_QUART_KB_PKT.data(),
              ONE_AND_QUART_KB_PKT.size());

  auto one_and_half_kb_frame = FixedFrame();
  std::memcpy(&one_and_half_kb_frame.data, ONE_AND_HALF_KB_PKT.data(), ONE_AND_HALF_KB_PKT.size());

  auto one_and_3quart_kb_frame = FixedFrame();
  std::memcpy(&one_and_3quart_kb_frame.data, ONE_AND_3QUART_KB_PKT.data(),
              ONE_AND_3QUART_KB_PKT.size());

  auto two_kb_frame = FixedFrame();
  std::memcpy(&two_kb_frame.data, TWO_KB_PKT.data(), TWO_KB_PKT.size());

  static const std::array<FixedFrame, 5> FRAMES = {
      one_kb_frame, one_and_quart_kb_frame, one_and_half_kb_frame, one_and_3quart_kb_frame,
      two_kb_frame,
  };

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_int_distribution<uint8_t> dist(0, 4);
  for (auto _ : state) {
    auto ops = state.range(0);
    for (int i = 0; i < ops; i += RATIO) {
      q.write(min_frame);
      benchmark::ClobberMemory();
      q.peek();
      q.commit_read();
      for (int j = 0; j < RATIO; ++j) {
        const auto& frame = FRAMES[dist(rng)];
        q.write(frame);
        benchmark::ClobberMemory();
        q.peek();
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
    ->Arg(1 << 26);
// ->Arg(1 << 28);

BENCHMARK(BM_spsc_fix_frame)
    ->Arg(1 << 12)
    ->Arg(1 << 14)
    ->Arg(1 << 16)
    ->Arg(1 << 18)
    ->Arg(1 << 20)
    ->Arg(1 << 22)
    ->Arg(1 << 24)
    ->Arg(1 << 26);
// ->Arg(1 << 28);
// Run the benchmark
BENCHMARK_MAIN();
