#pragma once

#include "task_queue.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <thread>

namespace threadpool {
class Threadpool {
    using writelock = std::unique_lock<std::shared_mutex>;
    using readlock = std::shared_lock<std::shared_mutex>;

public:
    void init(int num);
    void spawn();
    void terminate();
    void cancel();

    bool isInitialized() const;
    bool isRunning() const;
    size_t size() const;


    template <class F, class... Args>
    auto async(F&& f, Args&&... args) const -> std::future<decltype(f(args...))> { 
        // decltype will tell you type and value category of an expression
        using returnT = decltype(f(args...));
        using futureT = std::future<returnT>;
        using taskT = std::packaged_task<returnT()>;

        {
            readlock lock(mtx_);
            if (hasStopped_ || isCancelled_) 
                throw std::runtime_error (
                    "Delegating task to a threadpool that" 
                    "has been terminated or cancelled!");
        }

        auto bindFunc = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<taskT>(std::move(bindFunc));
        futureT fut = task->get_future();
        tasks_.emplace([task]() -> void { (*task)(); });
        cond_.notify_one();
        return fut;
    }

private:
    std::atomic<bool> isInitialized_ {false};
    std::atomic<bool> hasStopped_ {false};
    std::atomic<bool> isCancelled_ {false};
    std::vector<std::thread> workers_;
    mutable std::shared_mutex mtx_;
    mutable task_queue::TaskQueue<std::function<void()>> tasks_;
    mutable std::once_flag once_;
    mutable std::condition_variable_any cond_;
    
    bool isRunningImpl() const;
};
}  // namespace threadpool