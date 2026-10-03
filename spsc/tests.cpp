#include "spsc.h"

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

static constexpr std::string_view THREE = "three";
static constexpr std::string_view FOUR = "four";
static constexpr std::string_view SIX = "six";

using namespace ::testing;

TEST(SPSC, simple_write) {
  spscq<9, 3> q;
  Data f = {FOUR.size(), (std::byte *)FOUR.data()};
  Data x = {SIX.size(), (std::byte *)SIX.data()};

  q.write(f);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  q.write(x);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);
}

TEST(SPSC, write_then_read) {
  spscq<9, 3> q;
  Data f = {FOUR.size(), (std::byte *)FOUR.data()};
  Data x = {SIX.size(), (std::byte *)SIX.data()};

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
    for (auto b : r1) {
      r1_s.push_back(static_cast<char>(b));
    }
    EXPECT_THAT(r1_s, Eq("four"));
  }

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);

  auto r2 = q.read().copy();
  EXPECT_EQ(q.readWatermark(), 1);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);
}

TEST(SPSC, wrap_around) {
  spscq<9, 3> q;

  Data f = {FOUR.size(), (std::byte *)FOUR.data()};
  Data x = {SIX.size(), (std::byte *)SIX.data()};
  Data t = {THREE.size(), (std::byte *)THREE.data()};

  q.write(f);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  q.write(x);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);
  q.write(t);
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 3);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);
  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 2);

  for (int i = 0; i < 3; ++i) {
    q.commit_read();
    EXPECT_EQ(q.readWatermark(), 2);
    auto r = q.read().copy();
    std::string r1_s;
    for (auto b : r) {
      r1_s.push_back(static_cast<char>(b));
    }
    EXPECT_THAT(r1_s, Eq("three"));
  }
}
