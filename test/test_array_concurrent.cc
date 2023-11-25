#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <vector>

#include "cds_array.h"

using cds::cds_array;

TEST(TestArrayConcurrent, ConcurrentReads) {
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

TEST(TestArrayConcurrent, ConcurrentWrites) {
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

TEST(TestArrayConcurrent, ConcurrentReadsWrites) {
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
    readers.emplace_back([&a, i]() {
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
