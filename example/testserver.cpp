#include <mymuduo/TcpServer.h>
#include <mymuduo/Logger.h>
#include <string>
#include <functional>


class Echoserver {
public:
    Echoserver(EventLoop *loop, const InetAddress &listenAddr,std::string name)
        : loop_(loop),
          server_(loop, listenAddr, name) {
        server_.setConnectionCallback(
            std::bind(&Echoserver::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&Echoserver::onMessage, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
        server_.setThreadNum(4);
    }
    void start() { server_.start(); }
private:
    void onConnection(const TcpConnectionPtr &conn) {
        if (conn->connected()) {
            LOG_INFO("Connection from %s is UP",
                     conn->peerAddress().toIpPort().c_str());
        } else {
            LOG_INFO("Connection from %s is DOWN",
                     conn->peerAddress().toIpPort().c_str());
        }
    }

    void onMessage(const TcpConnectionPtr &conn, mymuduo::MyBuffer *buf,
                   Timestamp receiveTime) {
        std::string msg(buf->retrieveAllAsString());
        LOG_INFO("Received %ld bytes from %s at %s", msg.size(),
                 conn->peerAddress().toIpPort().c_str(),
                 receiveTime.toString().c_str());
        conn->send(msg);
        conn->shutdown();

    }
    EventLoop *loop_;
    TcpServer server_;
};

int main(){
    EventLoop loop;
    InetAddress listenAddr("127.0.0.1",8000);
    Echoserver server(&loop,listenAddr,"Echoserver");
    server.start();
    loop.loop();
    return 0;
}