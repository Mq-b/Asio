#include<iostream>
#include<thread>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
using fmt::print;
namespace asio = boost::asio;
namespace ip = asio::ip;
using socket_ptr =  boost::shared_ptr<ip::tcp::socket>;

void client_session(socket_ptr sock) {
    static int i = 1;
    print("Connect {} ok!", i);
    while (true) {
        thread_local char data[512]{};
        size_t len = sock->read_some(asio::buffer(data));
        if (len > 0)
            write(*sock, asio::buffer(data, strlen(data) + 1));
    }
}
int main() {
    asio::thread_pool tp(10);
    asio::io_service service;
    ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // 设置端口为2001
    ip::tcp::acceptor acc(service, ep);//server端才会用到的
   
    while (true) {
        socket_ptr sock(new ip::tcp::socket(service));
        acc.accept(*sock);
        asio::post(tp, [=] {client_session(sock); });
    }
    tp.join();
}