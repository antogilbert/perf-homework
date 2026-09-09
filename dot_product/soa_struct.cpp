#include <random>
#include <vector>

#include "consts.h"

struct Points {
  std::vector<float> xs;
  std::vector<float> ys;
  Points() : xs(SIZE), ys(SIZE) {}
};

namespace soa {
void dot_product(const Points &points, std::vector<float> &ans) {
  for (int i = 0; i < SIZE; ++i) {
    ans[i] = points.xs[i] * points.xs[i] + points.ys[i] * points.ys[i];
  }
}
} // namespace soa

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  Points points;
  for (int i = 0; i < SIZE; ++i) {
    points.xs[i] = dist(rng);
    points.ys[i] = dist(rng);
  }

  std::vector<float> ans;
  ans.resize(SIZE);

  soa::dot_product(points, ans);
  return 0;
}
