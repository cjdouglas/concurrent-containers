#include <gtest/gtest.h>

#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "cds_vector.h"

using cds::cds_vector;

namespace {
static bool random_bool() {
  static auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                              std::default_random_engine());
  return gen();
}
}  // namespace

struct RandomThrowType {
  RandomThrowType() {
    if (random_bool()) {
      throw std::runtime_error("Test exception");
    }
  }
};

struct CountAllocatorState {
  bool allocated = false;
  bool deallocated = false;
  int constructed = 0;
  int destructed = 0;
};

template <typename T>
struct CountAllocator : public std::allocator<T> {
  CountAllocator() : state(std::make_shared<CountAllocatorState>()) {}

  CountAllocator(const CountAllocator& other)
      : std::allocator<T>(other), state(other.state) {}

  T* allocate(const std::size_t n) {
    T* ptr = std::allocator<T>::allocate(n);
    state->allocated = true;
    return ptr;
  }

  void deallocate(T* p, const std::size_t n) {
    std::allocator<T>::deallocate(p, n);
    state->deallocated = true;
  }

  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    std::allocator<T>::construct(p, std::forward<Args>(args)...);
    ++state->constructed;
  }

  template <typename U>
  void destroy(U* p) {
    std::allocator<T>::destroy(p);
    ++state->destructed;
  }

  std::shared_ptr<CountAllocatorState> state;
};

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

  CountAllocator<int> count_alloc;

  cds_vector<int, CountAllocator<int>> b(count_alloc);
  EXPECT_EQ(b.size(), 0);
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.capacity(), 0);
  EXPECT_FALSE(count_alloc.state->allocated);
  EXPECT_EQ(count_alloc.state->constructed, 0);
}

TEST(TestVector, TestCountValueConstructor) {
  std::size_t count = 10;
  const int value = 3;

  cds_vector<int> a(count, value);
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
}

TEST(TestVector, TestCountConstructorException) {
  using T = RandomThrowType;
  std::size_t count = 30;
  CountAllocator<T> alloc;
  EXPECT_THROW((cds_vector<T, CountAllocator<T>>(count, alloc)),
               std::runtime_error);
  EXPECT_TRUE(alloc.state->allocated);
  EXPECT_TRUE(alloc.state->deallocated);
  EXPECT_EQ(alloc.state->constructed, alloc.state->destructed);
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
