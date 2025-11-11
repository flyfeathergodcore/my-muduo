#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"
#include "InetAddress.h"

#include <functional>
#include <strings.h>


EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if (loop == nullptr) {
        LOG_FATAL("TcpServer::TcpServer() or TcpConnection() - EventLoop is null!");
    }
    return loop;
} 

TcpServer::TcpServer(EventLoop* loop, 
    const InetAddress &listenAddr, 
    const std::string& name, 
    Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(),
      messageCallback_(),
      nextConnId_(1),
      started_(0)
{   //新用户连接时使用tcpserver为acceptor设置的回调函数
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, 
            std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s", 
        name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str());

    InetAddress localAddr;
    sockaddr_in localaddr;
    ::bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if(::getsockname(sockfd, (sockaddr*)&localaddr, &addrlen)){
        LOG_ERROR("TcpServer::newConnection - getsockname error!");
    }
    localAddr.setSockAddrInet(localaddr);


    TcpConnectionPtr conn(new TcpConnection(
        ioLoop, connName, sockfd, localAddr, peerAddr));    
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::start() {
    if (started_.fetch_add(1) == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(
            std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s", 
        name_.c_str(), conn->name().c_str());
    size_t n = connections_.erase(conn->name());
    (void)n;
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
} 

TcpServer::~TcpServer() {
    for (auto& item : connections_) {
        TcpConnectionPtr conn = item.second;
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}
