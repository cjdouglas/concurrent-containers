#pragma once

#include <mutex>
#include <shared_mutex>

namespace cds {

/// @brief CRTP base lock strategy, defines two functions for acquiring
/// exclusive & shared locks.
/// @tparam Derived The inheriting class, providing the implementations of the
/// lock acquisition functions.
template <typename Derived>
class LockStrategy {
 public:
  /// @brief Acquires an exclusive lock, providing exclusive access to the
  /// acquired mutex.
  /// @return A unique_lock holding a shared_mutex from the Derived strategy.
  std::unique_lock<std::shared_mutex> acquire_exclusive_lock() const {
    return static_cast<const Derived*>(this)->acquire_exclusive_lock_impl();
  }

  /// @brief Acquires a shared lock, providing shared access to acquired mutex.
  /// @return A shared_lock holding a shared_mutex from the Derived strategy.
  std::shared_lock<std::shared_mutex> acquire_shared_lock() const {
    return static_cast<const Derived*>(this)->acquire_shared_lock_impl();
  }

 private:
  LockStrategy() {}
  friend Derived;
};

/// @brief The default lock strategy applied to containers. Allows for shared
/// reads and exclusive writes.
class DefaultLockStrategy : public LockStrategy<DefaultLockStrategy> {
 public:
  /// @brief Exclusive lock implementation for the default lock strategy.
  std::unique_lock<std::shared_mutex> acquire_exclusive_lock_impl() const {
    return std::unique_lock<std::shared_mutex>(mutex_);
  }

  /// @brief Shared lock implementation for the default lock strategy.
  std::shared_lock<std::shared_mutex> acquire_shared_lock_impl() const {
    return std::shared_lock<std::shared_mutex>(mutex_);
  }

 private:
  mutable std::shared_mutex mutex_;
};
}  // namespace cds
