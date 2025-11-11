#pragma once

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>

class InetAddress
{
public:
    InetAddress();

    explicit InetAddress(const std::string &ip, uint16_t port);

    explicit InetAddress(const struct sockaddr_in &addr);

    std::string toIp() const;

    uint16_t toPort() const;

    void setSockAddrInet(const struct sockaddr_in &addr);

    const sockaddr_in* getSockAddrInet() const;

    std::string toIpPort() const;
private:
    sockaddr_in addr_;
};