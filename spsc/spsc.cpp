#include "spsc.h"

#include <string_view>

constexpr size_t KB = 1024;
constexpr size_t MB = 1024 * KB;

// Adapted to 250 chars -> 250 bytes
constexpr std::string_view LITANY =
    "I must not fear. Fear is the mind-killer.Fear is the little-death that brings total "
    "obliteration.I will permit it to pass over me and through me.When it has gone past I will "
    "turn the inner eye to see its path. Where the fear has gone there will be nothing.";

static_assert(LITANY.size() == 256);

int main() {
  spscq<9, 3> q;
  static constexpr std::string_view THREE = "three";
  static constexpr std::string_view FOUR = "four";
  static constexpr std::string_view SIX = "six";

  Data f = {FOUR.size(), (std::byte*)FOUR.data()};
  Data x = {SIX.size(), (std::byte*)SIX.data()};
  Data t = {THREE.size(), (std::byte*)THREE.data()};

  q.write(f);
  q.print();
  q.write(x);
  q.print();

  auto h = q.read().copy();
  q.print();
  print_bytes(h);

  std::string_view s((char*)h.data(), h.size());
  std::println("s {}", s);

  auto h2 = q.read().copy();
  print_bytes(h2);
  std::string_view s2((char*)h2.data(), h2.size());
  std::println("s2 {}", s2);

  q.commit_read();
  auto h3 = q.read().copy();
  print_bytes(h3);
  std::string_view s3((char*)h3.data(), h3.size());
  std::println("s3 {}", s3);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    auto h3 = q.read().copy();
    print_bytes(h3);
    std::string_view s3((char*)h3.data(), h3.size());
    std::println("s3({}) {}", i, s3);
  }

  std::println("---------------------------------");
  q.write(t);
  q.print();

  auto ht = q.read().copy();
  q.print();
  print_bytes(ht);

  std::string_view st((char*)ht.data(), ht.size());
  std::println("s {}", st);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    auto h3 = q.read().copy();
    print_bytes(h3);
    std::string_view s3((char*)h3.data(), h3.size());
    std::println("s3({}) {}", i, s3);
  }

  return 0;
}

/*
static void BM_spsc(benchmark::State &state) {
  for (auto _ : state) {
    // benchmark::DoNotOptimize(ans.data());
    benchmark::ClobberMemory();
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

// Run the benchmark
BENCHMARK_MAIN();
*/
