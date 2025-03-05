#ifndef H_LOGGER
#define H_LOGGER

#include <thread>

#include "Logging.h"

namespace ZH {

class Logger {
  public:
    Logger() = default;
    Logger(const Logger&) = delete;
    ~Logger();

    void start();
  private:
    bool shutdown = false;
    std::thread thread;

    static void writeLog(void*);
    void writeLogLines(LogQueue&& queue);
};

}

#endif
