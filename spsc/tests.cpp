#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "spsc.h"

static constexpr std::string_view THREE = "three";
static constexpr std::string_view FOUR = "four";
static constexpr std::string_view SIX = "six";

using namespace ::testing;

TEST(SPSC, simple_write) {
  spscq<9, 3> q;
  Data f = {FOUR.size(), (std::byte*)FOUR.data()};
  Data x = {SIX.size(), (std::byte*)SIX.data()};

  q.write(f);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  q.write(x);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);
}

TEST(SPSC, write_then_read) {
  spscq<9, 3> q;
  Data f = {FOUR.size(), (std::byte*)FOUR.data()};
  Data x = {SIX.size(), (std::byte*)SIX.data()};

  q.write(f);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  q.write(x);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);

  for (int i = 0; i < 2; ++i) {
    auto r1 = q.read().copy();
    EXPECT_EQ(q.readWatermark(), 0);
    EXPECT_EQ(r1.size(), FOUR.size());
    std::string r1_s;
    for (auto b : r1) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s, Eq("four"));
  }

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);

  auto r2 = q.read().copy();
  EXPECT_EQ(q.readWatermark(), 1);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);
}

TEST(SPSC, wrap_around_data) {
  spscq<9, 3> q;

  Data four = {FOUR.size(), (std::byte*)FOUR.data()};
  Data six = {SIX.size(), (std::byte*)SIX.data()};
  Data three = {THREE.size(), (std::byte*)THREE.data()};

  q.write(four);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  q.write(six);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);

  q.write(three);
  EXPECT_EQ(q.writeWatermark(), 0);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 2);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    EXPECT_EQ(q.readWatermark(), 2);
    auto r = q.read().copy();
    std::string r1_s;
    for (auto b : r) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s, Eq("three"));
  }

  q.write(four);
  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  auto r = q.read().copy();
  std::string r1_s;
  for (auto b : r) { r1_s.push_back(static_cast<char>(b)); }
  EXPECT_THAT(r1_s, Eq("four"));
}

TEST(SPSC, wrap_around_watermarks) {
  spscq<3, 1> q;

  constexpr auto A = std::byte('a');
  constexpr auto B = std::byte('b');
  constexpr auto C = std::byte('c');
  constexpr auto D = std::byte('d');

  q.write({1, &A});
  q.commit_read();
  q.write({1, &B});
  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);
  EXPECT_EQ(q.writeWatermark(), 2);

  q.write({1, &C});
  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 2);
  EXPECT_EQ(q.writeWatermark(), 0);

  q.write({1, &D});
  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
}
