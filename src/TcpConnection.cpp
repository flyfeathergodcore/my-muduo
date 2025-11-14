#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpServer.h"

#include <functional>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <strings.h>
#include <netinet/tcp.h> 



TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(CheckLoopNotNull(loop)),
      name_(name),
      state_(kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024 * 1024)  // 64MB
{
    LOG_INFO("TcpConnection::TcpConnection() - new connection [%s] from %s to %s",
              name_.c_str(),
              peerAddr_.toIpPort().c_str(),
              localAddr_.toIpPort().c_str());

    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    socket_->keepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::~TcpConnection() - connection [%s] destructed", name_.c_str());
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
        }
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_ERROR("TcpConnection::handleRead() - error on fd %d", channel_->fd());
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(),&savedErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite() - error on fd %d", channel_->fd());
        }
    } else {
        LOG_ERROR("TcpConnection::handleWrite() - fd %d is not writing", channel_->fd());
    }
}

void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose() - connection [%s] is closing", name_.c_str());
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    if (closeCallback_) {
        closeCallback_(guardThis);
    }
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
        LOG_ERROR("TcpConnection::handleError() - getsockopt error on fd %d", channel_->fd());
    } else {
        err = optval;
        LOG_ERROR("TcpConnection::handleError() - SO_ERROR = %d on fd %d", optval, channel_->fd());
    }
}

void TcpConnection::send(const std::string& Buffer){
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(Buffer.c_str(), Buffer.size());
        } else {
             loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,
                this,
                Buffer.c_str(),
                Buffer.size()
            ));
        }
    }
}

void TcpConnection::send(const void* data, size_t len) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(data, len);
        } else {
             loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,
                this,
                data,
                len
            ));
        }
    }
}

void TcpConnection::send(const mymuduo::MyBuffer& buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            // 直接发送 buf 中的数据
            sendInLoop(buf.peek(), buf.readableBytes());
            // 注意：因为 buf 是 const 引用，我们不能在这里修改它（如调用 retrieveAll）
            // 清空缓冲区的责任交给调用方，或者在发送后由调用方自行处理
        } else {
            // 同样，需要拷贝一份数据到 lambda 中，以保证线程安全
            std::string data(buf.peek(), buf.readableBytes());
            loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,
                this,
                data.c_str(),
                data.size()
            ));
        }
    }
}

void TcpConnection::send(mymuduo::MyBuffer* buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
             std::string data(buf->peek(), buf->readableBytes());
             loop_->runInLoop(std::bind(
                &TcpConnection::sendInLoop,
                this,
                data.c_str(),
                data.size()
            ));
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        LOG_ERROR("TcpConnection::sendInLoop() - disconnected, give up writing");
        return;
    }
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                //一次性写完，不会再给channel注册写事件，所以直接调用写完成回调
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop() - error on fd %d", channel_->fd());
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((const char*)data + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    TcpConnectionPtr guardThis(shared_from_this());
    if (connectionCallback_) {
        connectionCallback_(guardThis);
    }
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();

        TcpConnectionPtr guardThis(shared_from_this());
        if (connectionCallback_) {
            connectionCallback_(guardThis);
        }
    }
    channel_->remove();
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

