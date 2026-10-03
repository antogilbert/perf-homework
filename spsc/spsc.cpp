#include <array>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <print>
#include <string_view>

using bytes = std::vector<std::byte>;

struct frame {
  size_t total_len;
  size_t data_len;
  std::byte *data;
  std::byte *data_wrap;

  bytes copy() {
    bytes b(total_len);
    for (int i = 0; i < data_len; ++i) {
      b.push_back(data[i]);
    }

    if (data_len < total_len) {
      auto remaining = total_len - data_len;
      for (int i = 0; i < remaining; ++i) {
        b.push_back(data_wrap[i]);
      }
    }

    return b;
  }
};

void print_bytes(const bytes &bs) {
  std::print("BYTES: ");
  for (auto b : bs) {
    std::print("{}", (char)b);
  }
  std::println();
}

template <size_t CAP, size_t MIN> struct spscq {
  static_assert(CAP % MIN == 0);
  frame read() {
    auto frame_start = _readWM == 0 ? _pos.back() : _pos[_readWM - 1];
    auto frame_end = _pos[_readWM];

    if (frame_end > frame_start) {
      auto len = frame_end - frame_start;
      std::println("NORMAL read {} start {} len {} pos read {}", _readWM,
                   frame_start, len, _pos[_readWM]);
      auto *data = (std::byte *)&_data[frame_start];
      return frame{.total_len = len,
                   .data_len = len,
                   .data = data,
                   .data_wrap = nullptr};
    }

    auto data_len = CAP - frame_start;
    auto wrap_len = frame_end;
    auto *data = (std::byte *)&_data[frame_start];
    auto *data_wrap = (std::byte *)&_data[0];
    std::println(
        "SPLIT CAP {} data_len {} wrap_len {} frame_start {} frame_end {}", CAP,
        data_len, wrap_len, frame_start, frame_end);
    return frame{.total_len = data_len + wrap_len,
                 .data_len = data_len,
                 .data = data,
                 .data_wrap = data_wrap};
  }

  void write(frame &f) {
    auto frame_start = _writeWM == 0 ? _pos.back() : _pos[_writeWM - 1];
    auto remaining = _data.size() - frame_start;
    std::println("remaining {}", remaining);
    std::println("pos {} data {} writing {} at {}", _pos.size(), _data.size(),
                 f.total_len, frame_start);
    if (remaining >= f.total_len) {
      std::memcpy(&_data[frame_start], f.data, f.total_len);
      _pos[_writeWM] = frame_start + f.total_len;
    } else {
      std::println("Writing remaining {} on pos {}: {}", remaining, frame_start,
                   (char)f.data[0]);
      std::memcpy(&_data[frame_start], f.data, remaining);
      std::println("Writing wrap-around remaining {} on pos 0: {}",
                   f.total_len - remaining, (char)*(f.data + remaining));
      std::memcpy(&_data[0], f.data + remaining, f.total_len - remaining);
      _pos[_writeWM] = f.total_len - remaining;
    }

    ++_writeWM;
  };

  void commit_read() {
    auto curr = _readWM;
    std::println("Committing read: R {} W {}", _readWM, _writeWM);

    ++_readWM;

    if (_readWM == _writeWM) {
      _readWM = curr;
      std::println("SC Committed read: R {} W {}", _readWM, _writeWM);
      return;
    }

    if (_readWM == _pos.size()) {
      _readWM = 0;
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
  std::array<size_t, CAP / MIN> _pos{};
};

int main() {
  spscq<9, 3> q;
  static constexpr std::string_view three = "three";
  static constexpr std::string_view four = "four";
  static constexpr std::string_view six = "six";

  frame f = {.total_len = four.size(), .data = (std::byte *)four.data()};
  frame x = {.total_len = six.size(), .data = (std::byte *)six.data()};
  frame t = {.total_len = three.size(), .data = (std::byte *)three.data()};

  q.write(f);
  q.print();
  q.write(x);
  q.print();

  auto h = q.read().copy();
  q.print();
  print_bytes(h);

  std::string_view s((char *)h.data(), h.size());
  std::println("s {}", s);

  auto h2 = q.read().copy();
  print_bytes(h2);
  std::string_view s2((char *)h2.data(), h2.size());
  std::println("s2 {}", s2);

  q.commit_read();
  auto h3 = q.read().copy();
  print_bytes(h3);
  std::string_view s3((char *)h3.data(), h3.size());
  std::println("s3 {}", s3);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    auto h3 = q.read().copy();
    print_bytes(h3);
    std::string_view s3((char *)h3.data(), h3.size());
    std::println("s3({}) {}", i, s3);
  }

  std::println("---------------------------------");
  q.write(t);
  q.print();

  auto ht = q.read().copy();
  q.print();
  print_bytes(ht);

  std::string_view st((char *)ht.data(), ht.size());
  std::println("s {}", st);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    auto h3 = q.read().copy();
    print_bytes(h3);
    std::string_view s3((char *)h3.data(), h3.size());
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
