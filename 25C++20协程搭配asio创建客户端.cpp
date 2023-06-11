#include <boost/asio.hpp>
#include<fmt/core.h>
#include"task.h"
using namespace boost::asio;

co_context::coroutine<void> echo(ip::tcp::socket& sock, ip::tcp::endpoint&end_point) {
	co_context::Input input{[&] {sock.connect(end_point); } };
	co_await input;
	fmt::println("connect ok!");
	char data[512]{};
	while (true) {
		fmt::print("clinet in: "); fgets(data, 512, stdin);
		write(sock, buffer(data, strlen(data) + 1));
		size_t len = sock.read_some(buffer(data));
		if (len != error::eof)
			fmt::print("server out: {}", data);
		else
			break;
	}
}

int main() {
	io_service service;
	ip::tcp::endpoint end_point(ip::address::from_string("127.0.0.1"), 2001);
	ip::tcp::socket socket{ service };
	auto result = echo(socket,end_point);
	std::cout << "main\n";

	result.promise().future.wait();
	std::cout << "end\n";
}