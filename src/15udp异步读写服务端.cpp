#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;

asio::io_context service;
ip::udp::socket sock(service);
asio::ip::udp::endpoint sender_ep;
char buff[512]{};

void on_read(const boost::system::error_code & err, std::size_t read_bytes) {
  std::cout << "read " << read_bytes <<" "<<buff<< std::endl;
  sock.async_receive_from(asio::buffer(buff), sender_ep, on_read);///异步读取，循环调用当前函数
}
int main(){
    ip::udp::endpoint ep(ip::udp::v4(),80);
    sock.open(ep.protocol());//指定打开的协议类型
    sock.set_option(asio::ip::udp::socket::reuse_address(true));
    sock.bind(ep);
    sock.async_receive_from(asio::buffer(buff),sender_ep,on_read);
    service.run();
}