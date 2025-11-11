#pragma once
#include "nocopyable.h"
#include <functional>
#include "Timestamp.h"
#include <memory>

class EventLoop;

class Channel:nocopyable{
public:
    using EventCallback=std::function<void()>;
    using ReadEventCallback=std::function<void(Timestamp)>;
    
    Channel(EventLoop* loop,int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(const ReadEventCallback& cb){readCallback_= std::move(cb);}
    void setWriteCallback(const EventCallback& cb){writeCallback_=std::move(cb);}
    void setCloseCallback(const EventCallback& cb){closeCallback_=std::move(cb);}
    void setErrorCallback(const EventCallback& cb){errorCallback_=std::move(cb);}
    void set_revents(int revt){revents_=revt;}
    void set_index(int index){index_=index;}    


    int index()const{return index_;}
    void tie(const std::shared_ptr<void>&);
    int fd()const{return fd_;}
    int events()const{return events_;}
    EventLoop* ownerLoop(){return loop_;}
    

    void enableReading(){events_|=kReadEvent;update();}
    void disableReading(){events_&=~kReadEvent;update();}
    void enableWriting(){events_|=kWriteEvent;update();}
    void disableWriting(){events_&=~kWriteEvent;update();}
    void disableAll(){events_=kNoneEvent;update();}

    bool isWriting()const{return events_&kWriteEvent;}
    bool isReading()const{return events_&kReadEvent;}
    bool isNoneEvent()const{return events_==kNoneEvent;}

    
    void remove();

private:
    void handleEventWithGuard(Timestamp receiveTime);
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    bool tied_;
    std::weak_ptr<void> tie_;
    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;

    EventCallback errorCallback_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
};