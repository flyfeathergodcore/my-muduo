#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include "MyBuffer.h"

class AsyncLogging {
public:
    explicit AsyncLogging(const std::string& file);

    void start();
    void stop();

    // 供 Logger 调用：写入日志消息
    void append(const char* msg, size_t len);

private:
    void threadFunc();

private:
    using Buffer = mymuduo::MyBuffer;

    std::string file_;
    std::atomic<bool> running_;
    std::thread thread_;

    std::mutex mutex_;
    std::condition_variable cond_;

    Buffer* currentBuffer_;  
    Buffer* nextBuffer_;  
    std::vector<Buffer*> buffersToWrite_;
};
