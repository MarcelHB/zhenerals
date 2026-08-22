// SPDX-License-Identifier: GPL-2.0

#ifndef H_THREAD_POOL
#define H_THREAD_POOL

#include <barrier>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

namespace ZH {

class ThreadPool {
  private:
    std::list<std::thread> threads;

    std::mutex mutex;
    std::unique_ptr<std::barrier<std::function<void()>>> barrier;
    std::condition_variable taskCv;
    std::condition_variable launchCv;
    uint16_t runningThreads = 0;

    bool shutdown = false;

    std::function<void(uint16_t)> nextFn = [](uint16_t) {};
  public:
    ThreadPool (uint16_t n);
    ThreadPool (ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    void join();

    template<class F, class... Args>
    void kickAll(F && f, Args&&... args) {
      std::unique_lock<std::mutex> lock {mutex};
      if (runningThreads == 0) {
        nextFn = [&](uint16_t i) {
          f(i, args...);
        };
        runningThreads = threads.size();
        taskCv.notify_all();
      }
    }

    bool isFreshBatch() const;
    void waitOnTasks();
  private:
    static void threadFn(void *obj, uint16_t i);
};

}

#endif
