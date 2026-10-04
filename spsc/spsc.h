#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <print>
#include <vector>

using Bytes = std::vector<std::byte>;

void print_bytes(const Bytes& bs);

struct Data {
  size_t len;
  const std::byte* data;
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
  std::byte* data;
  size_t wrap_len;
  std::byte* data_wrap;
};

template <size_t CAP, size_t MIN>
struct spscq {
  static_assert(CAP % MIN == 0);
  explicit spscq() : _data(CAP), _pos(CAP / MIN) {}

  Frame read() {
    if (_readWM == _writeWM) { return Frame(0, nullptr); }

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

  bool write(Data f) {
    auto nextWM = (_writeWM + 1) % _pos.size();

    // Sacrifice one frame to ensure we can reliably say the queue is full
    if (nextWM == _readWM) return false;

    // TODO: check that the new data won't corrupt existing data in case the queue is approaching
    // full state
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

    _writeWM = nextWM;
    return true;
  };

  void commit_read() {
    if (_readWM == _writeWM) return;

    _readWM = (_readWM + 1) % _pos.size();
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
  std::vector<std::byte> _data;  // CAP
  std::vector<size_t> _pos;      // CAP / MIN
};
