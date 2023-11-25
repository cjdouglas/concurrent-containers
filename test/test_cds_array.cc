#include <gtest/gtest.h>

#include <array>
#include <iostream>

#include "cds_array.h"

using cds::cds_array;

TEST(TestCdsArray, TestConstructor) {
  const int N = 3;
  std::array<int, N> a{1};
  std::cout << a[1] << std::endl;
  cds_array<int, N> arr{1, 2, 3};
}