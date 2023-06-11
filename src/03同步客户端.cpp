#include<iostream>
#include<thread>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
using fmt::print;
namespace asio = boost::asio;
namespace ip = asio::ip;

void echo(ip::tcp::socket& sock) {
	char data[512]{};
	while (true) {
		print("clinet in: "); fgets(data, 512, stdin);
		write(sock, asio::buffer(data, strlen(data) + 1));
		size_t len = sock.read_some(asio::buffer(data));
		if (len != asio::error::eof)
			print("server out: {}", data);
		else
			break;
	}
}
int main() {
	asio::io_service service;
	ip::tcp::endpoint end_point(ip::address::from_string("127.0.0.1"), 2001);
	ip::tcp::socket socket{ service };
	socket.connect(end_point);
	print("connect ok!\n");
	echo(socket);
}