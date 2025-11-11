#include "Timestamp.h"
#include <time.h>
#include <iostream>

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {};
Timestamp::Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {};
Timestamp Timestamp::now()
{
    return Timestamp(time(NULL));
};
std::string Timestamp::toString() const
{
    char buf[128] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_);
    struct tm tm_time;               // 栈上的结构体，避免静态变量竞争
    localtime_r(&seconds, &tm_time); // 线程安全版本
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour,
             tm_time.tm_min,
             tm_time.tm_sec);
    return buf;
}

// int main()
// {
//     Timestamp ts = Timestamp::now();
//     std::cout << "Current Timestamp: " << ts.toString() << std::endl;
//     return 0;
// }