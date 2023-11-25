#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>

#include "cds_array.h"

using cds::cds_array;

TEST(TestCdsArray, TestConstructor) {
  constexpr cds_array<int, 3> a{1, 2, 3};
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 3);

  constexpr cds_array<int, 3> b{42};
  EXPECT_EQ(b[0], 42);
  EXPECT_EQ(b[1], 0);
  EXPECT_EQ(b[2], 0);
}

TEST(TestCdsArray, TestSizeOps) {
  constexpr cds_array<int, 3> a{1, 2, 3};
  EXPECT_FALSE(a.empty());
  EXPECT_EQ(a.size(), 3);
  EXPECT_EQ(a.max_size(), 3);

  constexpr cds_array<int, 3> b{42};
  EXPECT_FALSE(b.empty());
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b.max_size(), 3);
}

TEST(TestCdsArray, TestElementAccess) {
  constexpr cds_array<int, 3> a{1, 2, 3};
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 3);
  EXPECT_EQ(a.at(0), 1);
  EXPECT_EQ(a.at(1), 2);
  EXPECT_EQ(a.at(2), 3);

  EXPECT_THROW(a[3], std::out_of_range);
  EXPECT_THROW(a.at(3), std::out_of_range);

  EXPECT_EQ(a.front(), 1);
  EXPECT_EQ(a.back(), 3);
}

TEST(TestCdsArray, TestSet) {
  cds_array<int, 3> a{};
  EXPECT_EQ(a[0], 0);
  EXPECT_EQ(a[1], 0);
  EXPECT_EQ(a[2], 0);

  a.set(0, 3);
  a.set(1, 6);
  a.set(2, 9);
  EXPECT_EQ(a[0], 3);
  EXPECT_EQ(a[1], 6);
  EXPECT_EQ(a[2], 9);
}

TEST(TestCdsArray, TestFill) {
  cds_array<int, 3> a{};
  EXPECT_EQ(a[0], 0);
  EXPECT_EQ(a[1], 0);
  EXPECT_EQ(a[2], 0);

  a.fill(-3);
  EXPECT_EQ(a[0], -3);
  EXPECT_EQ(a[1], -3);
  EXPECT_EQ(a[2], -3);
}

TEST(TestCdsArray, TestSwap) {
  cds_array<int, 3> a{3, 2, 1};
  cds_array<int, 3> b{1, 2, 3};
  EXPECT_EQ(a[0], 3);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 1);
  EXPECT_EQ(b[0], 1);
  EXPECT_EQ(b[1], 2);
  EXPECT_EQ(b[2], 3);

  a.swap(b);
  EXPECT_EQ(b[0], 3);
  EXPECT_EQ(b[1], 2);
  EXPECT_EQ(b[2], 1);
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 3);
}

TEST(TestCdsArray, TestScopedWrite) {
  cds_array<int, 3> a{1, 2, 3};
  EXPECT_EQ(a[0], 1);
  EXPECT_EQ(a[1], 2);
  EXPECT_EQ(a[2], 3);

  {
    auto scoped_write = a.new_scoped_write();
    scoped_write.set(0, 4);
    scoped_write.set(1, 5);
    scoped_write.set(2, 6);
    EXPECT_THROW(scoped_write.set(3, 7), std::out_of_range);
  }

  EXPECT_EQ(a[0], 4);
  EXPECT_EQ(a[1], 5);
  EXPECT_EQ(a[2], 6);
}

TEST(TestCdsArray, TestScopedRead) {
  cds_array<int, 3> a{1, 2, 3};

  {
    auto scoped_read = a.new_scoped_read();
    EXPECT_EQ(scoped_read[0], 1);
    EXPECT_EQ(scoped_read.at(0), 1);
    EXPECT_EQ(scoped_read[1], 2);
    EXPECT_EQ(scoped_read.at(2), 3);
    EXPECT_EQ(scoped_read[1], 2);
    EXPECT_EQ(scoped_read.at(2), 3);
  }
}

TEST(TestCdsArray, TestIterators) {
  cds_array<int, 3> a{1, 2, 3};

  std::size_t i = 0;
  for (auto it = a.begin(); it != a.end(); ++it) {
    EXPECT_EQ(*it, a[i++]);
  }

  i = 2;
  for (auto it = a.rbegin(); it != a.rend(); ++it) {
    EXPECT_EQ(*it, a[i--]);
  }

  i = 0;
  for (auto it = a.cbegin(); it != a.cend(); ++it) {
    EXPECT_EQ(*it, a[i++]);
  }

  i = 2;
  for (auto it = a.crbegin(); it != a.crend(); ++it) {
    EXPECT_EQ(*it, a[i--]);
  }
}

TEST(TestCdsArray, TestStdSort) {
  const std::size_t N = 5;
  cds_array<int, N> a{5, 2, 17, -1, 0};
  const int increasing[N] = {-1, 0, 2, 5, 17};
  const int decreasing[N] = {17, 5, 2, 0, -1};

  {
    auto scoped_write = a.new_scoped_write();
    std::sort(a.begin(), a.end());
  }
  {
    auto scoped_read = a.new_scoped_read();
    for (std::size_t i = 0; i < N; ++i) {
      EXPECT_EQ(a[i], increasing[i]);
    }
  }

  {
    auto scoped_write = a.new_scoped_write();
    std::sort(a.rbegin(), a.rend());
  }
  {
    auto scoped_read = a.new_scoped_read();
    for (std::size_t i = 0; i < N; ++i) {
      EXPECT_EQ(a[i], decreasing[i]);
    }
  }
}
