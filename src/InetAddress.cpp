#include "InetAddress.h"
#include <arpa/inet.h>

InetAddress::InetAddress()
{
    addr_.sin_family = AF_INET;  // 显式指定为IPv4
    // 其他成员（port、addr）可暂时置0，后续由accept填充
}

InetAddress::InetAddress(const std::string &ip, uint16_t port)
{
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

InetAddress::InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

std::string InetAddress::toIp() const
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return std::string(buf);
}

uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const
{
    return toIp() + ":" + std::to_string(toPort());
}

const sockaddr_in* InetAddress::getSockAddrInet() const
{
    return &addr_;
}

void InetAddress::setSockAddrInet(const struct sockaddr_in &addr)
{
    addr_ = addr;
}
