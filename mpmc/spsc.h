#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <vector>

#include "consts.h"

using MO = std::memory_order;

namespace SPSC {

struct Buf {
    Buf() : len(0), data(nullptr) {}
    Buf(uint32_t len, std::byte* data) : len(len), data(data) {}
    uint32_t len;
    std::byte* data;
};

template <uint32_t CAP>
struct sb_q {
    static_assert((CAP & (CAP - 1)) == 0);
    sb_q() : _buf(CAP) {}

    auto peek() -> Buf {
        const auto r = _r.load(MO::relaxed);
        const auto w = _w.load(MO::acquire);
        // TODO: handle when we wrap around w but not about r
        return {w - r, &_buf[r]};
    }

    auto commit_read(uint32_t len) -> uint32_t {
        auto r = _r.load(MO::relaxed);
        const auto w = _w.load(MO::acquire);

        if (r == w) return 0;

        // TODO: handle (r + len > w) || (r + len > CAP)
        r = (r + len) & (CAP - 1);

        _r.store(r, MO::relaxed);
        return len;
    }

    auto push(std::vector<std::byte> data) -> bool {
        auto w = _w.load(MO::relaxed);
        const auto r = _r.load(MO::acquire);
        const auto s = data.size();

        if (CAP - w < s) {
            if (r < s) { return false; }
            w = 0;
        }

        std::memcpy(&_buf[w], data.data(), s);
        _w.fetch_add(s, MO::relaxed);
        return true;
    }

    auto readwm() -> uint32_t { return _r.load(MO::seq_cst); }
    auto writewm() -> uint32_t { return _w.load(MO::seq_cst); }

   private:
    alignas(std::hardware_destructive_interference_size) std::atomic<uint32_t> _w{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<uint32_t> _r{0};
    std::vector<std::byte> _buf;
};

}  // namespace SPSC
