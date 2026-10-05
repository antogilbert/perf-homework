#pragma once

#include <cstddef>
#include <cstring>
#include <vector>

#include "consts.h"

constexpr size_t FRAME_SIZE = 2 * KB;

using Bytes = std::vector<std::byte>;
void print_bytes(const Bytes& bs);

struct FixedFrame {
  std::byte data[FRAME_SIZE];
};

template <size_t CAP>
struct spscffq {
  struct Handle {
    ~Handle() {
      if (frame) q.commit_read();
    }

    FixedFrame copy() { return *frame; }

   private:
    FixedFrame* frame;
    spscffq& q;
  };

  explicit spscffq() : _data(CAP) {}

  FixedFrame* peek() {
    if (_readWM == _writeWM) { return nullptr; }

    return &_data[_readWM];
  }

  bool write(const FixedFrame& data) {
    auto nextWM = (_writeWM + 1) % _data.size();

    // Sacrifice one frame to ensure we can reliably say the queue is full
    if (nextWM == _readWM) return false;

    _data[_writeWM] = data;

    _writeWM = nextWM;
    return true;
  };

  void commit_read() {
    if (_readWM == _writeWM) return;

    _readWM = (_readWM + 1) % _data.size();
  };

  size_t readWatermark() { return _readWM; }
  size_t writeWatermark() { return _writeWM; }

 private:
  size_t _readWM{0};
  size_t _writeWM{0};
  std::vector<FixedFrame> _data;
};
