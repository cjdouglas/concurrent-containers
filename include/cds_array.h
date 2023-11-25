#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>

namespace cds {
template <typename T, std::size_t N>
class cds_array {
  static_assert(N, "cds_array does not support empty arrays");

 public:
  // Type definitions
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  // An object which acquires a write lock at construction, allowing for
  // efficient batch writes.
  struct scoped_write {
    explicit scoped_write(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    scoped_write(const scoped_write&) = delete;
    scoped_write& operator=(const scoped_write&) = delete;
    scoped_write(scoped_write&&) = default;
    scoped_write& operator=(scoped_write&&) = default;

    void set(const size_type pos, const_reference value) {
      if (pos >= array_.size()) {
        throw std::out_of_range("element access out of range");
      }

      array_.buffer_[pos] = value;
    }

   private:
    cds_array& array_;
    std::unique_lock<std::shared_mutex> lock_;
  };

  // An object which acquires a read lock at construction, allowing for
  // efficient batch reads.
  struct scoped_read {
    explicit scoped_read(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    scoped_read(const scoped_read&) = delete;
    scoped_read& operator=(const scoped_read&) = delete;
    scoped_read(scoped_read&&) = default;
    scoped_read& operator=(scoped_read&&) = default;

    const_reference at(const size_type pos) const {
      if (pos >= array_.size()) {
        throw std::out_of_range("element access out of range");
      }

      return array_.buffer_[pos];
    }
    const_reference operator[](const size_type pos) const { return at(pos); }

   private:
    cds_array& array_;
    std::shared_lock<std::shared_mutex> lock_;
  };

  template <typename... Ts>
  constexpr cds_array(Ts... ts) : buffer_{ts...} {}

  // Write interface

  void set(const size_type pos, const_reference value) {
    if (pos >= size()) {
      throw std::out_of_range("element access out of range");
    }

    std::unique_lock write(mutex_);
    buffer_[pos] = value;
  }

  scoped_write new_scoped_write() { return scoped_write(*this); }
  scoped_read new_scoped_read() { return scoped_read(*this); }

  // Iterators
  // WARNING: iterators are not inherently thread-safe. Please acquire a
  // scoped_read or scoped_write before using them. See documentation & examples
  // for sample usage.

  iterator begin() { return &buffer_[0]; }
  iterator end() { return &buffer_[N]; }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_iterator cbegin() const { return &buffer_[0]; }
  const_iterator cend() const { return &buffer_[N]; }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  // Utilities

  void fill(const_reference val) {
    auto scoped_write = new_scoped_write();
    std::fill(begin(), end(), val);
  }
  void swap(cds_array& other) noexcept(std::is_nothrow_swappable_v<T>) {
    auto scoped_this = new_scoped_write();
    auto scoped_other = other.new_scoped_write();
    std::swap_ranges(begin(), end(), other.begin());
  }

  // Read-only element access

  const_reference at(const size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("element access out of range");
    }

    std::shared_lock read(mutex_);
    return buffer_[pos];
  }
  const_reference operator[](const size_type pos) const { return at(pos); }
  const_reference front() const {
    std::shared_lock read(mutex_);
    return buffer_[0];
  }
  const_reference back() const {
    std::shared_lock read(mutex_);
    return buffer_[N - 1];
  }

  // Size operations

  constexpr bool empty() const noexcept { return false; }
  constexpr size_type size() const noexcept { return N; }
  constexpr size_type max_size() const noexcept { return N; }

 private:
  T buffer_[N];
  mutable std::shared_mutex mutex_;
};
}  // namespace cds
