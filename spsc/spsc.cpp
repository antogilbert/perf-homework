#include <array>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <print>
#include <string_view>

struct frame {
  size_t len;
  std::byte *data;
};

template <size_t CAP> struct spscq {
  frame read() {
    auto frame_start = _readWM == 0 ? _pos.back() : _pos[_readWM - 1];
    auto len = _pos[_readWM] - frame_start;
    std::println("read {} start {} len {} pos read {}", _readWM, frame_start,
                 len, _pos[_readWM]);
    auto *data = (std::byte *)&_data[frame_start];
    return frame{.len = len, .data = data};
  };

  void write(frame &f) {
    auto frame_start = _writeWM == 0 ? _pos.back() : _pos[_writeWM - 1];
    _pos[_writeWM] = frame_start + f.len;
    std::memcpy(&_data[frame_start], f.data, f.len);
    std::println("pos {} data {} writing {} at {}", _pos.size(), _data.size(),
                 f.len, frame_start);
    ++_writeWM;
  };

  void commit_read() {
    auto curr = _readWM;
    std::println("Committing read: R {} W {}", _readWM, _writeWM);

    ++_readWM;

    if (_readWM == _pos.size()) {
      _readWM = 0;
    }

    if (_readWM == _writeWM) {
      _readWM = curr;
      std::println("SC Committed read: R {} W {}", _readWM, _writeWM);
      return;
    }

    std::println("Committed read: R {} W {}", _readWM, _writeWM);
  };

  void print() {
    for (auto b : _data) {
      std::print("{}", (char)b);
    }
    std::println();
  };

private:
  size_t _readWM{0};
  size_t _writeWM{0};
  std::array<std::byte, CAP> _data;
  std::array<size_t, CAP / 20> _pos{};
};

int main() {
  spscq<120> q;
  static constexpr std::string_view four = "four";
  static constexpr std::string_view six = "six";

  frame f = {.len = four.size(), .data = (std::byte *)four.data()};
  frame g = {.len = six.size(), .data = (std::byte *)six.data()};

  q.write(f);
  q.print();
  q.write(g);
  q.print();

  auto h = q.read();
  q.print();

  std::string_view s((char *)h.data, h.len);
  std::println("{}", s);

  auto h2 = q.read();
  std::string_view s2((char *)h2.data, h2.len);
  std::println("{}", s2);

  q.commit_read();
  auto h3 = q.read();
  std::string_view s3((char *)h3.data, h3.len);
  std::println("{}", s3);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    auto h3 = q.read();
    std::string_view s3((char *)h3.data, h3.len);
    std::println("{}", s3);
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
