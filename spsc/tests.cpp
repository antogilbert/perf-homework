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

  EXPECT_TRUE(q.write(f));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  EXPECT_TRUE(q.write(x));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);
}

TEST(SPSC, write_then_read) {
  spscq<9, 3> q;
  Data f = {FOUR.size(), (std::byte*)FOUR.data()};
  Data x = {SIX.size(), (std::byte*)SIX.data()};

  EXPECT_TRUE(q.write(f));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  EXPECT_TRUE(q.write(x));
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
  EXPECT_EQ(q.readWatermark(), 2);
}

TEST(SPSC, wrap_around_data) {
  spscq<9, 3> q;

  Data four = {FOUR.size(), (std::byte*)FOUR.data()};
  Data six = {SIX.size(), (std::byte*)SIX.data()};
  Data three = {THREE.size(), (std::byte*)THREE.data()};

  EXPECT_TRUE(q.write(four));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);

  EXPECT_TRUE(q.write(six));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);

  EXPECT_TRUE(q.write(three));
  EXPECT_EQ(q.writeWatermark(), 0);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 2);

  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(q.readWatermark(), 2);
    auto r = q.read().copy();
    std::string r1_s;
    for (auto b : r) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s, Eq("three"));
  }

  q.commit_read();
  EXPECT_TRUE(q.write(four));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  auto r = q.read().copy();
  std::string r1_s;
  for (auto b : r) { r1_s.push_back(static_cast<char>(b)); }
  EXPECT_THAT(r1_s, Eq("four"));
}
