#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include "Timestamp.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

const int kNew = -1;
const int kAdd = 1;
const int kDel = 2;


EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_FATAL("EPollPoller::EPollPoller - epoll_create1 error:%d \n", errno);
    }
}
EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}

void EPollPoller::updateChannel(Channel* channel){
    const int index = channel->index();
    LOG_INFO("EPollPoller::updateChannel - fd=%d events=%d index=%d \n",
             channel->fd(), channel->events(), index);
    if (index == kNew || index == kDel) {
        if (index == kNew) {
            channels_[channel->fd()] = channel;
        }
        channel->set_index(kAdd);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDel);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::update(int option, Channel* channel) {
    epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollfd_, option, fd, &event) < 0) {
        if (option == EPOLL_CTL_DEL) {
            LOG_ERROR("EPollPoller::update - EPOLL_CTL_DEL error:%d \n", errno);
        } else {
            LOG_FATAL("EPollPoller::update - EPOLL_CTL_ADD/EPOLL_CTL_MOD error:%d \n", errno);
        }
    }

}

void EPollPoller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    LOG_INFO("EPollPoller::removeChannel - fd=%d \n", fd);
    channels_.erase(fd);
    if (channel->index() == kAdd) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    LOG_INFO("EPollPoller::poll - start polling ... \n");
    LOG_INFO("function=%s channels=%lu \n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_,
                                 &*events_.begin(),
                                 static_cast<int>(events_.size()),
                                 timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_INFO("EPollPoller::poll - %d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_INFO("EPollPoller::poll - nothing happened \n");
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_ERROR("EPollPoller::poll - epoll_wait error:%d \n", errno);
        }
    }
    return now;
}


void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const{
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}