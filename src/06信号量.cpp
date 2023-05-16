#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;

void signal_handler(const boost::system::error_code& err, int signal) {
	switch (signal) {
	case SIGINT:fmt::print("SIGNINT\n");
		break;
	case SIGTERM:fmt::print("SIGTERM\n");
		break;
	default:
		break;
	}
}
int main() {
	asio::io_service service;
	asio::signal_set sig(service, SIGINT, SIGTERM);//可以使用构造函数传入需要处理的信号，也可以直接用add方法
	sig.async_wait(signal_handler);////设置触发函数
	service.run();
	fmt::print("End\n");
}