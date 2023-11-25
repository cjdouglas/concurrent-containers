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

  // An iterator that locks the container for reading & writing.
  struct write_iterator {
    explicit write_iterator(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    write_iterator(const write_iterator&) = delete;
    write_iterator& operator=(const write_iterator&) = delete;
    write_iterator(write_iterator&&) = default;
    write_iterator& operator=(write_iterator&&) = default;

    void operator++() { ++i_; }
    void operator--() { --i_; }
    reference operator*() { return array_.buffer_[i_]; }

    iterator begin() { return &array_.buffer_[0]; }
    iterator end() { return &array_.buffer_[N]; }

   private:
    size_type i_ = 0;
    cds_array& array_;
    std::unique_lock<std::shared_mutex> lock_;
  };

  // An iterator that locks the container for reading.
  struct read_iterator {
    explicit read_iterator(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    read_iterator(const read_iterator&) = delete;
    read_iterator& operator=(const read_iterator&) = delete;
    read_iterator(read_iterator&&) = default;
    read_iterator& operator=(read_iterator&&) = default;

    void operator++() { ++i_; }
    void operator--() { --i_; }
    const_reference operator*() { return array_.buffer_[i_]; }

    iterator begin() { return &array_.buffer_[0]; }
    iterator end() { return &array_.buffer_[N]; }

   private:
    size_type i_ = 0;
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

  // Iterators

  write_iterator new_write_iterator() { return write_iterator(*this); }

  read_iterator new_read_iterator() { return read_iterator(*this); }

  // Utilities

  void fill(const_reference val) {
    write_iterator iter(*this);
    std::fill(iter.begin(), iter.end(), val);
  }
  void swap(cds_array& other) noexcept(std::is_nothrow_swappable_v<T>) {
    write_iterator iter_this(*this);
    write_iterator iter_other(other);
    std::swap_ranges(iter_this.begin(), iter_this.end(), iter_other.begin());
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