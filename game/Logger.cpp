#include <iostream>

#include "Logger.h"

namespace ZH {

Logger::~Logger() {
  shutdown = true;
  logSignal.notify_one();
  thread.join();
}

void Logger::start() {
  thread = std::thread(Logger::writeLog, this);
}

void Logger::writeLog(void *obj) {
  auto logger = reinterpret_cast<Logger*>(obj);

  while (true) {
    std::unique_lock<std::mutex> lock{logMutex};
    logSignal.wait(lock, [logger]{ return logger->shutdown || !logQueue.empty(); });

    LogQueue queue;
    queue.swap(logQueue);
    lock.unlock();

    logger->writeLogLines(std::move(queue));
    if (logger->shutdown) {
      return;
    }
  }
}

void Logger::writeLogLines(LogQueue&& queue) {
  while (!queue.empty()) {
    auto& message = queue.front();

    if (message.level == LogLevel::ERROR) {
      fmt::print(std::cerr, "!! [{}] {}\n", message.section, message.message);
    } else {
      fmt::print(std::cout, "[{}] {}\n", message.section, message.message);
    }

    queue.pop_front();
  }
}

}
