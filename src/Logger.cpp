#include "Logger.h"
#include "AsyncLogging.h"
#include <stdio.h>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::log(const char* msg) {
    if (async_) {
        async_->append(msg, strlen(msg));
    } else {
        fputs(msg, stdout);
        fputc('\n', stdout);
    }
}

void Logger::logf(LogLevel level, const char* fmt, ...) {
    char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    log(buf);
}
