#include "spsc.h"

void print_bytes(const Bytes& bs) {
  std::print("BYTES: ");
  for (auto b : bs) { std::print("{}", static_cast<char>(b)); }
  std::println();
}
