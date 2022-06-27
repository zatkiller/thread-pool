#pragma once

#include <mutex>
#include <queue>
#include <shared_mutex>

namespace task_queue {
template <typename T> class TaskQueue : protected std::queue<T> {
public:
  using writelock = std::unique_lock<std::shared_mutex>;
  using readlock = std::shared_lock<std::shared_mutex>;
  TaskQueue() = default;
  ~TaskQueue() {}
  TaskQueue(const TaskQueue &) = delete;            // Delete copy constructor
  TaskQueue(TaskQueue &&) = delete;                 // Delete move constructor
  TaskQueue &operator=(const TaskQueue &) = delete; // Delete copy assign op
  TaskQueue &operator=(TaskQueue &&) = delete;      // Delete move assign op
};

} // namespace task_queue