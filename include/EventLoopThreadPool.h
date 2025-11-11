#pragma once
#include "nocopyable.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback cb = ThreadInitCallback());
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    EventLoop *getNextLoop();
    std::vector<EventLoop *> getAllLoops();

    const std::string &name() const { return name_; }


private:
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop *> loops_;
};