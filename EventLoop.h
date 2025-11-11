#pragma once
#include "nocopyable.h"
#include <functional>
#include <vector>
#include <atomic>
#include "Timestamp.h"
#include <memory>
#include <mutex>
#include "CurrentThread.h"

class Channel;
class Poller;

class EventLoop : nocopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    void loop(); // 事件循环
    void quit(); // 退出事件循环
    Timestamp pollReturnTime() const { return pollReturnTime_; }
    void updateChannel(Channel *channel); // 更新channel
    void removeChannel(Channel *channel); // 移除channel
    void runInLoop(Functor cb);           // 在当前loop中执行cb
    void queueInLoop(Functor cb);         // 把cb放入队列，唤醒loop所在的线程，执行cb
    void wakeup();                        // 唤醒loop所在的线程
    bool hasChannel(Channel *channel);    // 判断当前loop是否管理channel
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();            // wakeupFd_可读时的回调函数
    void doPendingFunctors();     // 执行回调操作

    using ChannelList = std::vector<Channel *>;
    std::atomic<bool> looping_;
    std::atomic<bool> quit_;
    std::atomic<bool> callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
    const pid_t threadId_;                     // 记录当前loop所在线程的id
    Timestamp pollReturnTime_;                 // 上次poller返回发生事件的时间点
    std::unique_ptr<Poller> poller_;
    int wakeupFd_; // eventfd用于唤醒loop所在的线程

    std::unique_ptr<Channel> wakeupChannel_; // 用于监听wakeupFd_的channel
    ChannelList activeChannels_;             // 当前loop所管理的所有channel的集合
    Channel *currentActiveChannel_;          // 当前正在处理的channel
    std::vector<Functor> pendingFunctors_;   // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                       // 保护pendingFunctors_的线程安全操作
};
