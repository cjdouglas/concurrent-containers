#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <vector>

#include "cds_array.h"

using cds::cds_array;

TEST(TestArrayConcurrency, ConcurrentReads) {
  cds_array<int, 10> a{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  const std::size_t n_threads = 4;
  std::vector<std::thread> threads;

  for (std::size_t i = 0; i < n_threads; ++i) {
    threads.emplace_back([&a, i]() {
      auto read = a.new_scoped_read();
      for (std::size_t j = 0; j < a.size(); ++j) {
        EXPECT_EQ(read[j], j);
      }
    });
  }

  for (std::thread& t : threads) {
    t.join();
  }
}

TEST(TestArrayConcurrency, ConcurrentWrites) {
  cds_array<int, 10> a{};

  const std::size_t n_threads = 4;
  std::vector<std::thread> threads;

  for (std::size_t i = 0; i < n_threads; ++i) {
    threads.emplace_back([&a, i]() {
      auto write = a.new_scoped_write();
      for (std::size_t j = 0; j < a.size(); ++j) {
        write[j] = static_cast<int>(i);
      }

      // Assert no other threads have modified the array between write and read
      for (std::size_t j = 0; j < a.size(); ++j) {
        EXPECT_EQ(write[j], i);
      }
    });
  }

  for (std::thread& t : threads) {
    t.join();
  }
}

TEST(TestArrayConcurrency, ConcurrentReadsWrites) {
  cds_array<int, 10> a{};

  const std::size_t n_threads = 4;
  std::vector<std::thread> writers;
  std::vector<std::thread> readers;

  for (std::size_t i = 0; i < n_threads; ++i) {
    writers.emplace_back([&a, i]() {
      auto write = a.new_scoped_write();
      for (std::size_t j = 0; j < a.size(); ++j) {
        write[j] = static_cast<int>(i);
      }
    });

    // In this block, we verify all values are equivalent (ensuring no writers
    // are active during the read)
    readers.emplace_back([&a, i, n_threads]() {
      auto read = a.new_scoped_read();
      const int expected = read[0];
      EXPECT_TRUE(expected >= 0 && expected < static_cast<int>(n_threads));
      for (std::size_t j = 0; j < a.size(); ++j) {
        EXPECT_EQ(read[j], expected);
      }
    });
  }

  for (std::thread& t : writers) {
    t.join();
  }

  for (std::thread& t : readers) {
    t.join();
  }
}

TEST(TestArrayConcurrency, SwapNoDeadlock) {
  const std::size_t N = 100;
  cds_array<int, N> a;
  a.fill(0);
  cds_array<int, N> b;
  b.fill(1);

  const int n_swaps = 1000;
  auto a_swap_b = [&a, &b, n_swaps]() {
    for (int i = 0; i < n_swaps; ++i) {
      a.swap(b);
    }
  };

  auto b_swap_a = [&a, &b, n_swaps]() {
    for (int i = 0; i < n_swaps; ++i) {
      b.swap(a);
    }
  };

  std::thread t1(a_swap_b);
  std::thread t2(b_swap_a);
  std::thread t3(a_swap_b);
  std::thread t4(b_swap_a);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  // One final swap to swap values again
  a.swap(b);
  for (std::size_t i = 0; i < N; ++i) {
    EXPECT_EQ(a[i], 1);
    EXPECT_EQ(b[i], 0);
  }
}

TEST(TestArrayConcurrency, FillIsUnique) {
  const std::size_t N = 100;
  cds_array<int, N> a;
  a.fill(-1);

  const int n_fills = 1000;
  auto fill = [&a, n_fills](const int val) {
    for (int i = 0; i < n_fills; ++i) {
      a.fill(val);
    }
  };

  std::thread t1(fill, 0);
  std::thread t2(fill, 1);
  std::thread t3(fill, 2);
  std::thread t4(fill, 3);

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  const int val = a[0];
  EXPECT_TRUE(val >= 0 && val <= 3);
  for (std::size_t i = 0; i < N; ++i) {
    EXPECT_EQ(a[i], val);
  }
}
