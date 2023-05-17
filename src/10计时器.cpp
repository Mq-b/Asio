#include<iostream>
#include<boost/asio.hpp>
namespace asio = boost::asio;
using namespace std::placeholders;
bool read = false;

void deadline_handler(const boost::system::error_code&) {
    std::cout << (read ? "read successfully" : "read failed") << std::endl;
}
void read_handler(const boost::system::error_code&) {
    read = true;
}
int main() {
    asio::io_service service;
    asio::ip::tcp::socket sock(service);
    char data[512]{};
    sock.async_read_some(asio::buffer(data), std::bind(read_handler, _1));
    asio::deadline_timer t(service, boost::posix_time::milliseconds(100));//等价于sleep
    t.async_wait(&deadline_handler);
    service.run();
}