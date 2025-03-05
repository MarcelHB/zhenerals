#ifndef H_LOGGING
#define H_LOGGING

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>

#include "fmt/std.h"

namespace ZH {

enum class LogLevel {
  INFO,
  WARNING,
  ERROR
};

struct LogMessage {
  LogMessage (
      LogLevel level
    , std::string section
    , std::string message
  ) : level(level)
    , section(std::move(section))
    , message(std::move(message))
  { }
  LogLevel level;
  std::string section;
  std::string message;
};

using LogQueue = std::deque<LogMessage>;
extern std::mutex logMutex;
extern LogQueue logQueue;
extern std::condition_variable logSignal;

void enqueueLogMessage(LogMessage&& logMessage);

template<typename... ARGS>
void log(LogLevel level, std::string section, fmt::format_string<ARGS...> message, ARGS&& ...args) {
  auto formattedMessage = fmt::format(message, std::forward<ARGS>(args)...);
  enqueueLogMessage(LogMessage {level, std::move(section), std::move(formattedMessage)});
}

}

#define LOG_ZH(section, msg, ...)   log(ZH::LogLevel::INFO, (section), (msg), ##__VA_ARGS__);
#define WARN_ZH(section, msg, ...)  log(ZH::LogLevel::WARNING, (section), (msg), ##__VA_ARGS__);
#define ERROR_ZH(section, msg, ...) log(ZH::LogLevel::ERROR, (section), (msg), ##__VA_ARGS__);

#endif
