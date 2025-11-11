#pragma once
#include <vector>
#include <unordered_map>
#include "nocopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

class Poller : nocopyable
{
public:
    using ChannelList = std::vector<Channel *>;
    Poller(EventLoop *loop);
    virtual ~Poller();

    //保留接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    //判断channel是否在poller中
    bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *loop);
protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_;
};