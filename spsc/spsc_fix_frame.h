#include <cstddef>
#include <cstring>
#include <optional>
#include <vector>

constexpr size_t KB = 1024;
constexpr size_t FRAME_SIZE = 2 * KB;

using Bytes = std::vector<std::byte>;
void print_bytes(const Bytes& bs);

struct FixedFrame {
  std::byte data[FRAME_SIZE];
};

struct Handle {
  size_t len;
  size_t offset;
};

template <size_t CAP>
struct spscffq {
  explicit spscffq() : _data(CAP) {}

  FixedFrame* read() {
    if (_readWM == _writeWM) { return nullptr; }

    return &_data[_readWM];
  }

  bool write(FixedFrame data) {
    auto nextWM = (_writeWM + 1) % _data.size();

    // Sacrifice one frame to ensure we can reliably say the queue is full
    if (nextWM == _readWM) return false;

    _data[_writeWM] = std::move(data);

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
