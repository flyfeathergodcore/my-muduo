#pragma once

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "nocopyable.h"
#include "EventLoopThreadPool.h"
#include "Callback.h"
#include "TcpConnection.h"
#include "MyBuffer.h"

#include <memory>
#include <functional>
#include <atomic>
#include <map>

EventLoop* CheckLoopNotNull(EventLoop* loop);

class TcpServer:nocopyable
{ 
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,
        kReusePort,
    };

    TcpServer(EventLoop* loop, const InetAddress &listenAddr, const std::string& name, Option option = kNoReusePort);
    ~TcpServer();

    void start();

    void setThreadInitCallback(const ThreadInitCallback& cb){threadInitCallback_ = cb;}

    void setConnectionCallback(const ConnectionCallback& cb){connectionCallback_ = cb;}

    void setMessageCallback(const MessageCallback& cb){messageCallback_ = cb;}

    void setWriteCompleteCallback(const WriteCompleteCallback& cb){writeCompleteCallback_ = cb;}

    void setThreadNum(int numThreads);
private:
    using ConnectionMap = std::map<std::string, std::shared_ptr<TcpConnection>>;

    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const std::shared_ptr<TcpConnection>& conn);
    void removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn);

    EventLoop* loop_;
    const std::string name_;
    const std::string ipPort_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    std::atomic<int> started_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;

    int nextConnId_;
    ConnectionMap connections_;
};

