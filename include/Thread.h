#pragma once
#include <thread>
#include <functional>
#include <atomic>
#include <string>
#include <memory>
#include "nocopyable.h"

class Thread : nocopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc func, const std::string &name = "");
    ~Thread();
    void start();
    void join();
    bool started() const { return started_; }
    const std::string &name() const { return name_; }
    pid_t tid() const { return tid_; }

private:
    bool started_;
    bool joined_;
    pid_t tid_;
    std::shared_ptr<std::thread> thread_;
    std::string name_;
    static std::atomic<int> numCreated_;
    ThreadFunc func_;
};
