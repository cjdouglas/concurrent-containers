#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <utility>

namespace cds {

/// @brief A thread-safe dynamic array inspired by std::vector.
/// @tparam T The type of object the vector will hold.
/// @tparam Allocator The allocator used to acquire/release memory, and
/// construct/destroy elements.
template <typename T, typename Allocator = std::allocator<T>>
class cds_vector {
 public:
  /// @brief Template parameter T.
  using value_type = T;
  /// @brief Reference to T.
  using reference = value_type&;
  /// @brief Const reference to T.
  using const_reference = const value_type&;
  /// @brief Template parameter Allocator.
  using allocator_type = Allocator;
  /// @brief Pointer type.
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  /// @brief Const pointer type.
  using const_pointer =
      typename std::allocator_traits<Allocator>::const_pointer;
  /// @brief Iterator type.
  using iterator = pointer;
  /// @brief Const iterator type.
  using const_iterator = const_pointer;
  /// @brief Reverse iterator type.
  using reverse_iterator = std::reverse_iterator<iterator>;
  /// @brief Const reverse iterator type.
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  /// @brief cds_vector size type.
  using size_type = std::size_t;
  /// @brief cds_vector difference type.
  using difference_type = std::ptrdiff_t;

  /// @brief Constructs an empty cds_vector with a default allocator.
  cds_vector() noexcept(noexcept(Allocator()))
      : start_(nullptr),
        end_(nullptr),
        end_of_storage_(nullptr),
        allocator_() {}

  /// @brief Constructs an empty cds_vector with the given allocator.
  /// @param alloc The allocator used for all memory allocations.
  explicit cds_vector(const Allocator& alloc) noexcept
      : start_(nullptr),
        end_(nullptr),
        end_of_storage_(nullptr),
        allocator_(alloc) {}

  /// @brief Constructs a cds_vector with count copies of elements with the
  /// given value.
  /// @param count The number of elements to allocate.
  /// @param value The value to set each element to.
  /// @param alloc The allocator to use for all memory allocations.
  cds_vector(const size_type count, const T& value,
             const Allocator& alloc = Allocator())
      : allocator_(alloc) {
    start_ = allocator_.allocate(count);
    end_ = start_ + count;
    end_of_storage_ = end_;
    try {
      std::fill(start_, end_, value);
    } catch (...) {
      allocator_.deallocate(start_, count);
      throw;
    }
  }

  /// @brief Constructs a cds_vector with count copies of default inserted
  /// elements.
  /// @param count The number of elements to allocate.
  /// @param alloc The allocator to use for all memory allocations.
  explicit cds_vector(const size_type count,
                      const Allocator& alloc = Allocator())
      : allocator_(alloc) {
    start_ = allocator_.allocate(count);
    end_of_storage_ = start_ + count;

    try {
      for (end_ = start_; end_ != end_of_storage_; ++end_) {
        allocator_.construct(end_);
      }
    } catch (...) {
      while (end_ != start_) {
        allocator_.destroy(end_--);
      }
      allocator_.deallocate(start_, count);
      throw;
    }
  }

  /// @brief Constructs a cds_vector with the contents of range [first, last).
  /// @tparam InputIt Input iterator type.
  /// @param first The start of the iterator range.
  /// @param last The end of the iterator range.
  /// @param alloc The allocator to use for all memory allocations.
  template <class InputIt>
  cds_vector(InputIt first, InputIt last, const Allocator& alloc = Allocator())
      : allocator_(alloc) {
    const size_type count = std::distance(first, last);
    start_ = allocator_.allocate(count);
    end_of_storage_ = start_ + count;
    try {
      end_ = std::uninitialized_copy(first, last, start_);
    } catch (...) {
      allocator_.deallocate(start_, count);
      throw;
    }
  }

  /// @brief Copy constructor. Copies the contents of other.
  /// @param other The source cds_vector to copy from.
  cds_vector(const cds_vector& other)
      : allocator_(
            std::allocator_traits<allocator_type>::
                select_on_container_copy_construction(other.allocator_)) {
    const size_type size = std::distance(other.start_, other.end_of_storage_);
    start_ = allocator_.allocate(size);
    end_of_storage_ = start_ + size;
    try {
      end_ = std::uninitialized_copy(other.start_, other.end_, start_);
    } catch (...) {
      allocator_.deallocate(start_, size);
      throw;
    }
  }

  /// @brief Copies the contents of other, using allocator alloc.
  /// @param other The source cds_vector to copy from.
  /// @param alloc The allocator to use for all memory allocations.
  cds_vector(const cds_vector& other, const Allocator& alloc)
      : allocator_(alloc) {
    const size_type size = std::distance(other.start_, other.end_of_storage_);
    start_ = allocator_.allocate(size);
    end_of_storage_ = start_ + size;
    try {
      end_ = std::uninitialized_copy(other.start_, other.end_, start_);
    } catch (...) {
      allocator_.deallocate(start_, size);
      throw;
    }
  }

  /// @brief Move constructor. Uses move semantics to move the contents of
  /// other.
  /// @param other The source cds_vector to move.
  cds_vector(cds_vector&& other) noexcept
      : allocator_(std::move(other.allocator_)),
        start_(std::exchange(other.start_, nullptr)),
        end_(std::exchange(other.end_, nullptr)),
        end_of_storage_(std::exchange(other.end_of_storage_, nullptr)) {}

  /// @brief Allocator-extended move constructor. Uses move semantics if alloc
  /// == other.allocator_. Otherwise, it uses an element-wise move (other is not
  /// guaranteed to be empty after this operation).
  /// @param other The source cds_vector to move.
  /// @param alloc The allocator to use for all memory allocations.
  cds_vector(cds_vector&& other, const Allocator& alloc) : allocator_(alloc) {
    if (allocator_ == other.allocator_) {
      start_ = std::exchange(other.start_, nullptr);
      end_ = std::exchange(other.end_, nullptr);
      end_of_storage_ = std::exchange(other.end_of_storage_, nullptr);
    } else {
      const size_type size = std::distance(other.start_, other.end_of_storage_);
      start_ = allocator_.allocate(size);
      end_of_storage_ = start_ + size;
      try {
        end_ = std::uninitialized_copy(std::make_move_iterator(other.start_),
                                       std::make_move_iterator(other.end_),
                                       start_);
      } catch (...) {
        allocator_.deallocate(start_, size);
        throw;
      }
    }
  }

  /// @brief Constructs a cds_vector from the contents of the initializer_list.
  /// @param init The initializer list to copy from.
  /// @param alloc The allocator to use for all memory allocations.
  cds_vector(std::initializer_list<T> init,
             const Allocator& alloc = Allocator())
      : allocator_(alloc) {
    start_ = allocator_.allocate(init.size());
    end_of_storage_ = start_ + init.size();
    try {
      end_ = std::uninitialized_copy(init.begin(), init.end(), start_);
    } catch (...) {
      allocator_.deallocate(start_, init.size());
      throw;
    }
  }

  /// @brief Destroys the cds_vector instance.
  ~cds_vector() { allocator_.deallocate(start_, end_of_storage_ - start_); }

  cds_vector& operator=(const cds_vector& other);
  cds_vector& operator=(cds_vector&& other);

  iterator begin() { return start_; }
  iterator end() { return end_; }

  const_reference operator[](const size_type pos) { return *(start_ + pos); }

  bool empty() const noexcept { return !(end_ - start_); }
  size_type size() const noexcept { return end_ - start_; }
  size_type capacity() const noexcept { return end_of_storage_ - start_; }

 private:
  pointer start_;
  pointer end_;
  pointer end_of_storage_;
  Allocator allocator_;
  mutable std::shared_mutex mutex_;
};
}  // namespace cds
