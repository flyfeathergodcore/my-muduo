#pragma once
#include "Thread.h"
#include "nocopyable.h"

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread : nocopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();
    EventLoop* loop_;
    Thread thread_;
    bool exiting_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};