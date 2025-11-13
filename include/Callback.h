#pragma once

#include <functional>
#include <memory>

namespace mymuduo
{
    class MyBuffer;
}

class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, mymuduo::MyBuffer*, Timestamp)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, size_t)>;