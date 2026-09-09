#include <random>
#include <vector>

#include "consts.h"

struct Point {
  float x;
  float y;
};

namespace aos {
void dot_product(const std::vector<Point> &points, std::vector<float> &ans) {
  for (int i = 0; i < SIZE; ++i) {
    ans[i] = points[i].x * points[i].x + points[i].y * points[i].y;
  }
}
} // namespace aos

int main() {

  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<float> dist(1, 100);

  std::vector<Point> points;
  points.resize(SIZE);

  for (int i = 0; i < SIZE; ++i) {
    points[i] = Point{dist(rng), dist(rng)};
  }

  std::vector<float> ans;
  ans.resize(SIZE);

  aos::dot_product(points, ans);
  return 0;
}
