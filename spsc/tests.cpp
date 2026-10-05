#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "spsc.h"
#include "spsc_fix_frame.h"

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

// Fixed Frame tests

TEST(SPSCFF, simple_write) {
  spscffq<3> q;
  auto f = FixedFrame();
  auto x = FixedFrame();
  std::memcpy(&f.data, FOUR.data(), FOUR.size());
  std::memcpy(&x.data, SIX.data(), SIX.size());

  EXPECT_TRUE(q.write(f));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  EXPECT_TRUE(q.write(x));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);
}

TEST(SPSCFF, write_then_read) {
  spscffq<3> q;
  auto f = FixedFrame();
  auto x = FixedFrame();
  std::memcpy(&f.data, FOUR.data(), FOUR.size());
  std::memcpy(&x.data, SIX.data(), SIX.size());

  EXPECT_TRUE(q.write(f));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  EXPECT_TRUE(q.write(x));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 2);

  for (int i = 0; i < 2; ++i) {
    auto r1 = *q.read();
    EXPECT_EQ(q.readWatermark(), 0);
    std::string r1_s;
    for (auto b : r1.data) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s.find_first_of("four"), Eq(0));
    EXPECT_THAT(r1_s.find_last_of("four"), Eq(3));
    EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
  }

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 1);

  auto r2 = q.read();
  EXPECT_EQ(q.readWatermark(), 1);

  q.commit_read();
  EXPECT_EQ(q.readWatermark(), 2);
}

TEST(SPSCFF, wrap_around_data) {
  spscffq<3> q;

  auto four = FixedFrame();
  auto six = FixedFrame();
  auto three = FixedFrame();
  std::memcpy(&four.data, FOUR.data(), FOUR.size());
  std::memcpy(&six.data, SIX.data(), SIX.size());
  std::memcpy(&three.data, THREE.data(), THREE.size());

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
    auto r = *q.read();
    std::string r1_s;
    for (auto b : r.data) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s.find_first_of("three"), Eq(0));
    EXPECT_THAT(r1_s.find_last_of("three"), Eq(4));
    EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
  }

  q.commit_read();
  EXPECT_TRUE(q.write(four));
  EXPECT_EQ(q.readWatermark(), 0);
  EXPECT_EQ(q.writeWatermark(), 1);
  auto r = *q.read();
  std::string r1_s;
  for (auto b : r.data) { r1_s.push_back(static_cast<char>(b)); }
  EXPECT_THAT(r1_s.find_first_of("four"), Eq(0));
  EXPECT_THAT(r1_s.find_last_of("four"), Eq(3));
  EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
}
