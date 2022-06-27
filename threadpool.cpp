#include "threadpool.h"

#include <iostream>

namespace threadpool {
    void Threadpool::init(int num) {
        std::call_once(once_, [this, num]() {
            writelock lock(mtx_);
            hasStopped_.store(false);
            isCancelled_.store(false);
            workers_.reserve(num);
            for (int i = 0; i < num; ++i) {
                workers_.emplace_back(std::bind(&Threadpool::spawn, this));
            }
            isInitialized_.store(true);
        });
    }

    void Threadpool::spawn() {
        for (;;) {
            bool pop = false;
            std::function<void()> task;
            {
                writelock lock(mtx_);
                cond_.wait(lock, [this, &pop, &task] {
                    pop = tasks_.pop(task);
                    return isCancelled_.load() || hasStopped_.load() || pop;
                });
            }
            if (isCancelled_.load() || (hasStopped_.load() && !pop)) {
                return;
            }
            task();
        }
    }

    void Threadpool::terminate() {
        {
            writelock lock(mtx_);
            if (!isRunningImpl())
                return;

            hasStopped_.store(true);
        }
        cond_.notify_all();
        for (auto &worker: workers_)
            worker.join();
    }

    void Threadpool::cancel() {
        {
            writelock lock(mtx_);;
            if (!isRunningImpl())
                return;
                
            isCancelled_.store(true);
        }

        tasks_.clear();
        cond_.notify_all();

        for (auto &worker: workers_)
            worker.join();
    }

    bool Threadpool::isInitialized() const {
        readlock lock(mtx_);
        return isInitialized_.load();
    }

    bool Threadpool::isRunningImpl() const {
        return isInitialized_.load() && !(hasStopped_.load()) && !(isCancelled_.load());
    }

    bool Threadpool::isRunning() const {
        readlock lock(mtx_);
        return isRunningImpl();
    }

    size_t Threadpool::size() const {
        readlock lock(mtx_);
        return workers_.size();
    }
} // namespace threadpool