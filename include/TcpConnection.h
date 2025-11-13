#pragma once
#include "nocopyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "Timestamp.h"
#include "MyBuffer.h"
#include "Contentbase.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

    void send(const std::string& Buffer);
    void send(const void* data, size_t len);
    void send(mymuduo::MyBuffer* buf);
    void send(const mymuduo::MyBuffer& buf);

    void shutdown();
    void connectEstablished();
    void connectDestroyed();

    void setState(int s) { state_ = s; }

    void setContext(std::unique_ptr<Contentbase> content) { content_ = std::move(content); }
    Contentbase* getContext() const { return content_.get(); }

private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void shutdownInLoop();
    void sendInLoop(const void* data, size_t len);

    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
    
    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    std::atomic<int> state_;
    std::atomic<bool> reading_;
    std::unique_ptr<Contentbase> content_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;

    mymuduo::MyBuffer inputBuffer_;
    mymuduo::MyBuffer outputBuffer_;
};