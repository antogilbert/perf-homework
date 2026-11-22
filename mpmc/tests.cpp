#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "spsc.h"

static std::vector<std::byte> THREE = {
    std::byte('t'), std::byte('h'), std::byte('r'), std::byte('e'), std::byte('e'),
};

static std::vector<std::byte> FOUR = {
    std::byte('f'),
    std::byte('o'),
    std::byte('u'),
    std::byte('r'),
};

static std::vector<std::byte> SIX = {
    std::byte('s'),
    std::byte('i'),
    std::byte('x'),
};

using namespace ::testing;

TEST(SPSC, create_def) {
    SPSC::sb_q<8> q;
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 0);
}

TEST(SPSC, simple_write) {
    SPSC::sb_q<8> q;

    EXPECT_TRUE(q.push(FOUR));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 4);
    EXPECT_TRUE(q.push(SIX));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 7);
}

/*
TEST(SPSC, write_then_read) {
    SPSC::sb_q<4> q;

    EXPECT_TRUE(q.push(FOUR));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);
    EXPECT_TRUE(q.push(SIX));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 2);

    for (int i = 0; i < 2; ++i) {
        auto r1 = q.peek();
        EXPECT_EQ(q.readwm(), 0);
        EXPECT_EQ(r1.len, FOUR.size());
        std::string r1_s;
        for (int i = 0; i < r1.len; ++i) { r1_s.push_back(static_cast<char>(*(r1.data + i))); }
        EXPECT_THAT(r1_s, Eq("four"));
    }

    auto r = q.commit_read(7);
    EXPECT_EQ(q.readwm(), 1);
    EXPECT_EQ(r, 7);

    auto r2 = q.commit_read(7);
    EXPECT_EQ(r2, 0);
}

TEST(SPSC, wrap_around_data) {
    spscq<3> q(3);

    Data four = {FOUR.size(), (std::byte*)FOUR.data()};
    Data six = {SIX.size(), (std::byte*)SIX.data()};
    Data three = {THREE.size(), (std::byte*)THREE.data()};

    EXPECT_TRUE(q.push(four));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);

    EXPECT_TRUE(q.push(six));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 2);

    q.commit_read();
    EXPECT_EQ(q.readwm(), 1);

    EXPECT_TRUE(q.push(three));
    EXPECT_EQ(q.writewm(), 0);

    q.commit_read();
    EXPECT_EQ(q.readwm(), 2);

    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(q.readwm(), 2);
        auto r = q.peek().copy();
        std::string r1_s;
        for (auto b : r) { r1_s.push_back(static_cast<char>(b)); }
        EXPECT_THAT(r1_s, Eq("three"));
    }

    q.commit_read();
    EXPECT_TRUE(q.push(four));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);
    auto r = q.peek().copy();
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

    EXPECT_TRUE(q.push(f));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);
    EXPECT_TRUE(q.push(x));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 2);
}

TEST(SPSCFF, write_then_read) {
    spscffq<3> q;
    auto f = FixedFrame();
    auto x = FixedFrame();
    std::memcpy(&f.data, FOUR.data(), FOUR.size());
    std::memcpy(&x.data, SIX.data(), SIX.size());

    EXPECT_TRUE(q.push(f));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);
    EXPECT_TRUE(q.push(x));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 2);

    for (int i = 0; i < 2; ++i) {
        auto r1 = *q.peek();
        EXPECT_EQ(q.readwm(), 0);
        std::string r1_s;
        for (auto b : r1.data) { r1_s.push_back(static_cast<char>(b)); }
        EXPECT_THAT(r1_s.find_first_of("four"), Eq(0));
        EXPECT_THAT(r1_s.find_last_of("four"), Eq(3));
        EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
    }

    q.commit_read();
    EXPECT_EQ(q.readwm(), 1);

    auto r2 = q.peek();
    EXPECT_EQ(q.readwm(), 1);

    q.commit_read();
    EXPECT_EQ(q.readwm(), 2);
}

TEST(SPSCFF, wrap_around_data) {
    spscffq<3> q;

    auto four = FixedFrame();
    auto six = FixedFrame();
    auto three = FixedFrame();
    std::memcpy(&four.data, FOUR.data(), FOUR.size());
    std::memcpy(&six.data, SIX.data(), SIX.size());
    std::memcpy(&three.data, THREE.data(), THREE.size());

    EXPECT_TRUE(q.push(four));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);

    EXPECT_TRUE(q.push(six));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 2);

    q.commit_read();
    EXPECT_EQ(q.readwm(), 1);

    EXPECT_TRUE(q.push(three));
    EXPECT_EQ(q.writewm(), 0);

    q.commit_read();
    EXPECT_EQ(q.readwm(), 2);

    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(q.readwm(), 2);
        auto r = *q.peek();
        std::string r1_s;
        for (auto b : r.data) { r1_s.push_back(static_cast<char>(b)); }
        EXPECT_THAT(r1_s.find_first_of("three"), Eq(0));
        EXPECT_THAT(r1_s.find_last_of("three"), Eq(4));
        EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
    }

    q.commit_read();
    EXPECT_TRUE(q.push(four));
    EXPECT_EQ(q.readwm(), 0);
    EXPECT_EQ(q.writewm(), 1);
    auto r = *q.peek();
    std::string r1_s;
    for (auto b : r.data) { r1_s.push_back(static_cast<char>(b)); }
    EXPECT_THAT(r1_s.find_first_of("four"), Eq(0));
    EXPECT_THAT(r1_s.find_last_of("four"), Eq(3));
    EXPECT_THAT(r1_s.size(), Eq(FRAME_SIZE));
}
*/
