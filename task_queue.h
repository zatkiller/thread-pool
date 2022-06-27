#pragma once

#include <mutex>
#include <queue>
#include <shared_mutex>

namespace task_queue {
template <typename T> 
class TaskQueue : protected std::queue<T> {
    using writelock = std::unique_lock<std::shared_mutex>;
    using readlock = std::shared_lock<std::shared_mutex>;

    TaskQueue() = default;
    ~TaskQueue() { clear(); }
    TaskQueue(const TaskQueue&) = delete;            // Delete copy constructor
    TaskQueue(TaskQueue&&) = delete;                 // Delete move constructor
    TaskQueue& operator=(const TaskQueue&) = delete; // Delete copy assign op
    TaskQueue& operator=(TaskQueue&&) = delete;      // Delete move assign op

    bool empty() const {
        readlock lock(mtx_);
        return std::queue<T>::empty();
    };

    size_t size() const {
        readlock lock(mtx_);
        return std::queue<T>::size();
    };

    void clear() {
        writelock lock(mtx_);
        while (!std::queue<T>::empty())
            std::queue<T>::pop();
    }

    void push(const T &obj) {
        writelock lock(mtx_);
        std::queue<T>::push(obj);
    }

    template <typename... Args> 
    void emplace(Args &&... args) {
        writelock lock(mtx_);
        std::queue<T>::emplace(std::forward<Args>(args)...); //perfect forwarding
    }

    // More like a try_pop
    bool pop(T& holder) {
        writelock lock(mtx_);
        if (std::queue<T>::empty()) {
            return false;
        }

        holder = std::move(std::queue<T>::front());
        std::queue<T>::pop();
        return true;
    }

private:
    mutable std::shared_mutex mtx_; // Mutable Can be changed in const function
};// namespace task_queue

} // namespace task_queue