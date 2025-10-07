// SPDX-License-Identifier: GPL-2.0

#include "Logging.h"

namespace ZH {

std::mutex logMutex;
LogQueue logQueue;
std::condition_variable logSignal;

void enqueueLogMessage(LogMessage&& logMessage) {
  std::lock_guard<std::mutex> lock {logMutex};
  logQueue.push_back(std::move(logMessage));
  logSignal.notify_all();
}

}
