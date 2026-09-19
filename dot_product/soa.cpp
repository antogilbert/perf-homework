
#include <random>
#include <vector>

#include "consts.h"

namespace soa_struct {
void dot_product(std::vector<float> &xs, std::vector<float> &ys,
                 std::vector<float> &ans) {
  for (int i = 0; i < SIZE; ++i) {
    ans[i] = xs[i] * xs[i] + ys[i] * ys[i];
  }
}
} // namespace soa_struct

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  std::vector<float> xs;
  std::vector<float> ys;
  xs.resize(SIZE);
  ys.resize(SIZE);

  for (int i = 0; i < SIZE; ++i) {
    xs[i] = dist(rng);
    ys[i] = dist(rng);
  }

  std::vector<float> ans;
  ans.resize(SIZE);

  soa_struct::dot_product(xs, ys, ans);
  return 0;
}
