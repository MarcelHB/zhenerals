// SPDX-License-Identifier: GPL-2.0

#include "ThreadPool.h"

namespace ZH {

ThreadPool::ThreadPool (uint16_t n) {
  barrier = std::make_unique<std::barrier<std::function<void()>>>(n, []() {});

  for (uint16_t i = 0; i < n; ++i) {
    threads.emplace_back(threadFn, this, i);
  }
}

void ThreadPool::join() {
  shutdown = true;
  taskCv.notify_all();
  for (auto& thread : threads) {
    thread.join();
  }
  launchCv.notify_all();
}

void ThreadPool::waitOnTasks() {
  std::unique_lock<std::mutex> lock {mutex};
  launchCv.wait(lock, [this] { return runningThreads == 0 || shutdown; });
}

void ThreadPool::threadFn(void *obj, uint16_t i) {
  ThreadPool *pool = reinterpret_cast<ThreadPool*>(obj);

  while (true) {
    {
      std::unique_lock<std::mutex> lock {pool->mutex};
      pool->taskCv.wait(lock, [pool] { return pool->isFreshBatch() || pool->shutdown; });
    }

    if (pool->shutdown) {
      break;
    }

    pool->nextFn(i);
    pool->barrier->arrive_and_wait();

    {
      std::unique_lock<std::mutex> lock {pool->mutex};
      pool->runningThreads -= 1;
    }
    pool->launchCv.notify_all();

    if (pool->shutdown) {
      break;
    }
  }
}

bool ThreadPool::isFreshBatch() const {
  return runningThreads == threads.size();
}

}
