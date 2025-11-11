#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include <memory>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(const ThreadInitCallback cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);

        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.emplace_back(t);
        loops_.push_back(t->startLoop());
    }

    if (numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    std::vector<EventLoop *> allLoops;
    if (loops_.empty())
    {
        allLoops.push_back(baseLoop_);
    }
    else
    {
        allLoops = loops_;
    }
    return allLoops;
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }

    return loop;
}