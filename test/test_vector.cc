#include <gtest/gtest.h>

#include <vector>

#include "cds_vector.h"

using cds::cds_vector;

struct ThrowType {
  static bool should_throw;

  ThrowType() {
    if (should_throw) {
      throw std::runtime_error("Test exception");
    }
  }
};

bool ThrowType::should_throw = false;

TEST(TestVector, TestEmptyConstructor) {
  cds_vector<int> a;
  EXPECT_EQ(a.size(), 0);
  EXPECT_TRUE(a.empty());
  EXPECT_EQ(a.capacity(), 0);
}

TEST(TestVector, TestAllocatorConstructor) {
  std::allocator<int> alloc;
  cds_vector<int> a(alloc);
  EXPECT_EQ(a.size(), 0);
  EXPECT_TRUE(a.empty());
  EXPECT_EQ(a.capacity(), 0);
}

TEST(TestVector, TestCountValueConstructor) {
  std::size_t count = 10;
  const int value = 3;

  cds_vector a(count, value);
  EXPECT_EQ(a.size(), count);
  EXPECT_FALSE(a.empty());
  EXPECT_EQ(a.capacity(), count);
  EXPECT_EQ(a[0], value);
  EXPECT_EQ(a[count - 1], value);

  count = 0;
  cds_vector<int> b(count, value);
  EXPECT_EQ(b.size(), 0);
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.capacity(), 0);
}

TEST(TestVector, TestCountConstructor) {
  std::size_t count = 10;

  cds_vector<int> a(count);
  EXPECT_EQ(a.size(), count);
  EXPECT_FALSE(a.empty());
  EXPECT_EQ(a.capacity(), count);
  EXPECT_EQ(a[0], 0);
  EXPECT_EQ(a[count - 1], 0);

  count = 0;
  cds_vector<int> b(count);
  EXPECT_EQ(b.size(), 0);
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.capacity(), 0);

  ThrowType::should_throw = true;
  count = 10;
  EXPECT_THROW(cds_vector<ThrowType> c(count), std::runtime_error);
  ThrowType::should_throw = false;
}

TEST(TestVector, TestIterConstructor) {
  std::vector<int> v = {1, 2, 3, 4, 5};
  cds_vector<int> a(v.begin(), v.end());
  EXPECT_EQ(a.size(), 5);
  for (std::size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], a[i]);
  }

  std::vector<int> u;
  cds_vector<int> b(u.begin(), u.end());
  EXPECT_EQ(b.size(), 0);
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.capacity(), 0);

  std::allocator<int> alloc;
  cds_vector<int> c(v.begin(), v.end(), alloc);
  EXPECT_EQ(c.size(), 5);
  for (std::size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], c[i]);
  }
}

TEST(TestVector, TestCopyConstructor) {
  std::size_t count = 10;
  const int value = 3;

  cds_vector<int> a(count, value);
  cds_vector<int> b(a);
  EXPECT_EQ(a.size(), b.size());
  for (std::size_t i = 0; i < count; ++i) {
    EXPECT_EQ(a[i], b[i]);
  }

  count = 0;
  cds_vector<int> u;
  cds_vector<int> v(u);
  EXPECT_TRUE(u.empty());
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(u.size(), v.size());
}

TEST(TestVector, TestCopyAllocConstructor) {
  // TODO
}

TEST(TestVector, TestMoveConstructor) {
  // TODO
}

TEST(TestVector, TestMoveAllocConstructor) {
  // TODO
}

TEST(TestVector, TestInitListConstructor) {
  cds_vector<int> a = {1, 2, 3, 4, 5};
  EXPECT_EQ(a.size(), 5);
  EXPECT_EQ(a[2], 3);

  cds_vector<int> b = {};
  EXPECT_EQ(b.size(), 0);
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.capacity(), 0);
}
