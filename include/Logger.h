#pragma once
#include <string>
#include <stdarg.h>
#include <cstring>
#include "nocopyable.h"

class AsyncLogging;

enum LogLevel { INFO, ERROR, FATAL, DEBUG };

class Logger : nocopyable {
public:
    static Logger& instance();

    void setLogLevel(LogLevel level) { logLevel_ = level; }
    LogLevel logLevel() const { return logLevel_; }

    void setAsyncLogger(AsyncLogging* async) { async_ = async; }

    void log(const char* msg);
    void logf(LogLevel level, const char* fmt, ...);

private:
    Logger() : logLevel_(INFO), async_(nullptr) {}

private:
    LogLevel logLevel_;
    AsyncLogging* async_;
};


// ---- 宏 ----
#define LOG_INFO(fmt, ...) \
    do { \
        if (Logger::instance().logLevel() <= INFO) \
            Logger::instance().logf(INFO, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(fmt, ...) \
    do { \
        if (Logger::instance().logLevel() <= ERROR) \
            Logger::instance().logf(ERROR, fmt, ##__VA_ARGS__); \
    } while (0)

#define LOG_FATAL(fmt, ...) \
    do { \
        Logger::instance().logf(FATAL, fmt, ##__VA_ARGS__); \
        exit(1); \
    } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(fmt, ...) \
    do { \
        if (Logger::instance().logLevel() <= DEBUG) \
            Logger::instance().logf(DEBUG, fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define LOG_DEBUG(...)
#endif
