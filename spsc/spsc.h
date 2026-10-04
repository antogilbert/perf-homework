#include <array>
// #include <benchmark/benchmark.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <print>
#include <vector>

using Bytes = std::vector<std::byte>;

struct Data {
  size_t len;
  std::byte* data;
};

struct Frame {
  Frame() = delete;
  Frame(size_t len, std::byte* data) : data_len(len), wrap_len(0), data(data), data_wrap(nullptr) {}

  void wrap_around(size_t len, std::byte* wrap) {
    data_wrap = wrap;
    wrap_len = len;
  }

  Bytes copy() {
    Bytes b(data_len + wrap_len);

    for (int i = 0; i < data_len; ++i) { b[i] = data[i]; }

    for (int i = 0; i < wrap_len; ++i) { b[data_len + i] = data_wrap[i]; }

    return b;
  }

 private:
  size_t data_len;
  size_t wrap_len;
  std::byte* data;
  std::byte* data_wrap;
};

void print_bytes(const Bytes& bs) {
  std::print("BYTES: ");
  for (auto b : bs) { std::print("{}", static_cast<char>(b)); }
  std::println();
}

template <size_t CAP, size_t MIN>
struct spscq {
  static_assert(CAP % MIN == 0);

  Frame read() {
    auto frame_start = _readWM == 0 ? _pos.back() : _pos[_readWM - 1];
    auto frame_end = _pos[_readWM];

    if (frame_end > frame_start) {
      auto len = frame_end - frame_start;
      // std::println("NORMAL read {} start {} len {} pos read {}", _readWM,
      // frame_start, len, _pos[_readWM]);
      auto* data = static_cast<std::byte*>(&_data[frame_start]);
      return Frame(len, data);
    }

    auto data_len = _data.size() - frame_start;
    auto wrap_len = frame_end;
    auto* data = static_cast<std::byte*>(&_data[frame_start]);
    auto* data_wrap = static_cast<std::byte*>(&_data[0]);
    // std::println(
    // "SPLIT CAP {} data_len {} wrap_len {} frame_start {} frame_end {}", CAP,
    // data_len, wrap_len, frame_start, frame_end);
    auto f = Frame(data_len, data);
    f.wrap_around(wrap_len, data_wrap);
    return f;
  }

  void write(Data f) {
    auto frame_start = _writeWM == 0 ? _pos.back() : _pos[_writeWM - 1];
    auto remaining = _data.size() - frame_start;
    // std::println("remaining {}", remaining);
    // std::println("pos {} data {} writing {} at {}", _pos.size(),
    // _data.size(), f.len, frame_start);
    if (remaining >= f.len) {
      std::memcpy(&_data[frame_start], f.data, f.len);
      _pos[_writeWM] = frame_start + f.len;
    } else {
      // std::println("Writing remaining {} on pos {}: {}", remaining,
      // frame_start, static_cast<char>(f.data[0]));
      std::memcpy(&_data[frame_start], f.data, remaining);
      // std::println("Writing wrap-around remaining {} on pos 0: {}",
      // f.len - remaining, static_cast<char>(*(f.data + remaining)));
      std::memcpy(&_data[0], f.data + remaining, f.len - remaining);
      _pos[_writeWM] = f.len - remaining;
    }

    ++_writeWM;
  };

  void commit_read() {
    auto curr = _readWM;
    // std::println("Committing read: R {} W {}", _readWM, _writeWM);

    ++_readWM;

    if (_readWM == _writeWM) {
      _readWM = curr;
      // std::println("SC Committed read: R {} W {}", _readWM, _writeWM);
      return;
    }

    if (_readWM == _pos.size()) { _readWM = 0; }

    // std::println("Committed read: R {} W {}", _readWM, _writeWM);
  };

  void print() {
    std::print("SPSCQ DATA: '");
    for (auto b : _data) { std::print("{}", static_cast<char>(b)); }
    std::print("'");
    std::println();
  };

  size_t readWatermark() { return _readWM; }
  size_t writeWatermark() { return _writeWM; }

 private:
  size_t _readWM{0};
  size_t _writeWM{0};
  std::array<std::byte, CAP> _data;
  std::array<size_t, CAP / MIN> _pos{};
};
