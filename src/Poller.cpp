#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop) {
}

bool Poller::hasChannel(Channel* channel) const {
    return channels_.find(channel->fd()) != channels_.end();
}

Poller::~Poller() {
    // 基类析构函数可以为空，但必须有实现
}
