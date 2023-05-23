#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
int main()
{
	asio::io_context service;
  	ip::udp::socket sock(service);
	sock.open(ip::udp::v4());
	ip::udp::endpoint receiver_ep(ip::address::from_string("127.0.0.1"),80);
	sock.send_to(asio::buffer("testing\n"),receiver_ep);//同步发送数据
	char buff[512]{};
	ip::udp::endpoint sender_ep{};
	sock.receive_from(asio::buffer(buff),sender_ep);//异步地从一个指定的端点获取数据并写入到给定的缓冲区。在读完所有数据或者错误出现之前，这个函数都是阻塞的
}