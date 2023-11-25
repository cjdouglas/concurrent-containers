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

/// @brief A thread-safe static array inspired by std::array.
/// @tparam T The type of object the array will hold.
/// @tparam N The number of elements the array will hold.
template <typename T, std::size_t N>
class cds_array {
  static_assert(N, "cds_array does not support empty arrays");

 public:
  // Type definitions

  /// @brief Template parameter T.
  using value_type = T;
  /// @brief Reference to T.
  using reference = T&;
  /// @brief Const reference to T.
  using const_reference = const T&;
  /// @brief Iterator type for T.
  using iterator = value_type*;
  /// @brief Const iterator type for T.
  using const_iterator = const value_type*;
  /// @brief Reverse iterator type for T.
  using reverse_iterator = std::reverse_iterator<iterator>;
  /// @brief Const reverse iterator type for T.
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  /// @brief cds_array size type.
  using size_type = std::size_t;
  /// @brief cds_array difference type.
  using difference_type = std::ptrdiff_t;

  /// @brief A convenience struct which acquires a write lock for the target
  /// array and exposes an interface for batch writes.
  struct scoped_write {
    /// @brief Construct a new scoped_write.
    /// @param arr The input cds_array to build the scoped_write object for.
    explicit scoped_write(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    scoped_write(const scoped_write&) = delete;
    scoped_write& operator=(const scoped_write&) = delete;
    scoped_write(scoped_write&&) = default;
    scoped_write& operator=(scoped_write&&) = default;

    /// @brief Returns a reference to the value at the specified position.
    /// Functionally equivalent to operator[].
    /// @param pos The specified position.
    /// @return A reference to the value at position pos.
    reference at(const size_type pos) {
      if (pos >= array_.size()) {
        throw std::out_of_range("element access out of range");
      }

      return array_.buffer_[pos];
    }

    /// @brief Returns a reference to the value at the specified position.
    /// Functionally equivalent to at().
    /// @param pos The specified position.
    /// @return A reference to the value at position pos.
    reference operator[](const size_type pos) { return at(pos); }

    /// @brief Returns a reference to the value at the front of the array.
    /// @return A reference to the value at the front of the array.
    reference front() { return at(0); }

    /// @brief Returns a reference to the value at the back of the array.
    /// @return A reference to the value at the back of the array.
    reference back() { return at(array_.size() - 1); }

   private:
    cds_array& array_;
    std::unique_lock<std::shared_mutex> lock_;
  };

  /// @brief A convenience struct which acquires a read lock for the target
  /// array and exposes an interface for batch reads.
  struct scoped_read {
    /// @brief Construct a new scoped_read.
    /// @param arr The input cds_array to build the scoped_read object for.
    explicit scoped_read(cds_array& arr) : array_(arr), lock_(arr.mutex_) {}
    scoped_read(const scoped_read&) = delete;
    scoped_read& operator=(const scoped_read&) = delete;
    scoped_read(scoped_read&&) = default;
    scoped_read& operator=(scoped_read&&) = default;

    /// @brief Returns a const_reference to the value at the specified position.
    /// Functionally equivalent to operator[].
    /// @param pos The specified position.
    /// @return A const_reference to the value at position pos.
    const_reference at(const size_type pos) const {
      if (pos >= array_.size()) {
        throw std::out_of_range("element access out of range");
      }

      return array_.buffer_[pos];
    }

    /// @brief Returns a const_reference to the value at the specified position.
    /// Functionally equivalent to at().
    /// @param pos The specified position.
    /// @return A const_reference to the value at position pos.
    const_reference operator[](const size_type pos) const { return at(pos); }

    /// @brief Returns a const_reference to the value at the front of the array.
    /// @return A reference to the value at the front of the array.
    const_reference front() const { return at(0); }

    /// @brief Returns a const_reference to the value at the back of the array.
    /// @return A reference to the value at the back of the array.
    const_reference back() const { return at(array_.size() - 1); }

   private:
    cds_array& array_;
    std::shared_lock<std::shared_mutex> lock_;
  };

  /// @brief Construct a cds_array from a list of values. Each value must be
  /// convertible to type T.
  /// @tparam ...Ts Variadic template type.
  /// @param ...ts Variadic argument used to initialize the cds_array.
  template <typename... Ts>
  cds_array(Ts... ts) : buffer_{ts...} {}

  /// @brief Returns a new scoped_write from this array for batch write
  /// operations.
  /// @return A new scoped_write instance for batch write operations.
  scoped_write new_scoped_write() { return scoped_write(*this); }

  /// @brief Returns a new scoped_read from this array for batch read
  /// operations.
  /// @return A new scoped_read instance for batch read operations.
  scoped_read new_scoped_read() { return scoped_read(*this); }

  /// @brief Returns an iterator pointing to the start of the array.
  /// @warning begin() is not thread-safe by itself. Please acquire a
  /// scoped_write to ensure thread safe iteration.
  /// @return An iterator pointing to the start of the array.
  iterator begin() { return &buffer_[0]; }

  /// @brief Returns an iterator pointing to the end of the array.
  /// @warning end() is not thread-safe by itself. Please acquire a
  /// scoped_write to ensure thread safe iteration.
  /// @return An iterator pointing to the end of the array.
  iterator end() { return &buffer_[N]; }

  /// @brief Returns an iterator pointing to the reverse start of the array.
  /// @warning rbegin() is not thread-safe by itself. Please acquire a
  /// scoped_write to ensure thread safe iteration.
  /// @return An iterator pointing to the reverse start of the array.
  reverse_iterator rbegin() { return reverse_iterator(end()); }

  /// @brief Returns an iterator pointing to the reverse end of the array.
  /// @warning rend() is not thread-safe by itself. Please acquire a
  /// scoped_write to ensure thread safe iteration.
  /// @return An iterator pointing to the reverse end of the array.
  reverse_iterator rend() { return reverse_iterator(begin()); }

  /// @brief Returns a const iterator pointing to the start of the array.
  /// @warning cbegin() is not thread-safe by itself. Please acquire a
  /// scoped_read to ensure thread safe iteration.
  /// @return A const iterator pointing to the start of the array.
  const_iterator cbegin() const { return &buffer_[0]; }

  /// @brief Returns a const iterator pointing to the end of the array.
  /// @warning cend() is not thread-safe by itself. Please acquire a
  /// scoped_read to ensure thread safe iteration.
  /// @return A const iterator pointing to the end of the array.
  const_iterator cend() const { return &buffer_[N]; }

  /// @brief Returns a const iterator pointing to the reverse start of the
  /// array.
  /// @warning crbegin() is not thread-safe by itself. Please acquire a
  /// scoped_read to ensure thread safe iteration.
  /// @return A const iterator pointing to the reverse start of the array.
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }

  /// @brief Returns a const iterator pointing to the reverse end of the
  /// array.
  /// @warning crend() is not thread-safe by itself. Please acquire a
  /// scoped_read to ensure thread safe iteration.
  /// @return A const iterator pointing to the reverse end of the array.
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  /// @brief Acquires a write lock and sets the value at position pos to value.
  /// @param pos The position in the array to update.
  /// @param value The value to update position pos to.
  void set(const size_type pos, const_reference value) {
    if (pos >= size()) {
      throw std::out_of_range("element access out of range");
    }

    std::unique_lock write(mutex_);
    buffer_[pos] = value;
  }

  /// @brief Acquires a write lock and fills the array with the specified value.
  /// @param val The value to fill the array with.
  void fill(const_reference val) {
    auto scoped_write = new_scoped_write();
    std::fill(begin(), end(), val);
  }

  /// @brief Acquires a write lock for the caller and target array, and swaps
  /// the contents.
  /// @param other The array to swap contents with.
  void swap(cds_array& other) noexcept(std::is_nothrow_swappable_v<T>) {
    auto scoped_this = new_scoped_write();
    auto scoped_other = other.new_scoped_write();
    std::swap_ranges(begin(), end(), other.begin());
  }

  /// @brief Acquires a read lock and returns a const_reference to the value at
  /// the specified position. Functionally equivalent to operator[].
  /// @param pos The specified position.
  /// @return A const_reference to the value at position pos.
  const_reference at(const size_type pos) const {
    if (pos >= size()) {
      throw std::out_of_range("element access out of range");
    }

    std::shared_lock read(mutex_);
    return buffer_[pos];
  }

  /// @brief Acquires a read lock and returns a const_reference to the value at
  /// the specified position. Functionally equivalent to at().
  /// @param pos The specified position.
  /// @return A const_reference to the value at position pos.
  const_reference operator[](const size_type pos) const { return at(pos); }

  /// @brief Acquires a read lock and returns a const_reference to the value at
  /// the front of the array.
  /// @return A reference to the value at the front of the array.
  const_reference front() const {
    std::shared_lock read(mutex_);
    return buffer_[0];
  }

  /// @brief Acquires a read lock and returns a const_reference to the value at
  /// the back of the array.
  /// @return A reference to the value at the back of the array.
  const_reference back() const {
    std::shared_lock read(mutex_);
    return buffer_[N - 1];
  }

  /// @brief Returns if the array is empty or not. Since cds_array does not
  /// support empty arrays, this always evaluates to false.
  /// @return Whether the array is empty or not.
  constexpr bool empty() const noexcept { return false; }

  /// @brief Returns the size of the array. This is equivalent to template
  /// parameter N.
  /// @return The size of the array.
  constexpr size_type size() const noexcept { return N; }

  /// @brief Returns the maximum size of the array. This is equivalent to
  /// size().
  /// @return The maximum size of the array.
  constexpr size_type max_size() const noexcept { return N; }

 private:
  T buffer_[N];
  mutable std::shared_mutex mutex_;
};
}  // namespace cds
