#pragma once
#include <string>


class InetAddress;
class Socket {
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();
    int fd() const { return sockfd_; }
    void bandAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite();
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void keepAlive(bool on);

private:
    const int sockfd_;
};