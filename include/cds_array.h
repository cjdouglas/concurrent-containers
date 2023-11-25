#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace cds {
template <typename T, std::size_t N>
class cds_array {
 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <typename... Ts>
  constexpr cds_array(Ts... ts) : buffer_{ts...} {}

  void fill(const_reference val) { std::fill(begin(), end(), val); }

  iterator begin() { return &buffer_[0]; }
  iterator end() { return &buffer_[N]; }
  const_iterator cbegin() const { return &buffer_[0]; }
  const_iterator cend() const { return &buffer_[N]; }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }

  constexpr reference at(const size_type pos) {
    if (pos >= size()) {
      throw std::out_of_range("\"at\" access out of range");
    }

    return buffer_[pos];
  }
  constexpr const_reference at(const size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("\"at\" access out of range");
    }

    return buffer_[pos];
  }

  constexpr reference operator[](const size_type pos) { return buffer_[pos]; }
  constexpr const_reference operator[](const size_type) const {
    return buffer_[pos];
  }

  constexpr reference front(const size_type pos) {}

  constexpr bool empty() const noexcept { return N == 0; }
  constexpr size_type size() const noexcept { return N; }
  constexpr size_type max_size() const noexcept { return N; }

 private:
  T buffer_[N ? N : 1];
};
}  // namespace cds