#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock, const boost::system::error_code& ec) {//连接成功后打印
	if (ec) return;
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}
void timeout_handler(const boost::system::error_code&) {
	fmt::print("笑死\n");
}
int main() {
	asio::io_service service;
	ip::tcp::socket sock{ service };
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	sock.async_connect(ep, [&](const auto& ec)
		{connect_handler(sock, ec); });
	asio::deadline_timer t(service, boost::posix_time::seconds(5));
	t.async_wait(timeout_handler);
	service.run();
}