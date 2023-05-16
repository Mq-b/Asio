#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;

int main() {
	asio::io_service service;
	asio::serial_port sp( service);//在Linux端口名称是/dev/ttyS0
	try{
		sp.open("COM1");
	}
	catch (const std::exception&e){
		std::cout << e.what() << '\n';
		exit(0);
	}
	asio::serial_port::baud_rate rate(8009);
	sp.set_option(rate);

	char data[512]{};
	read(sp, asio::buffer(data));
	fmt::print("{}", data);
}
//由于没啥别的设置，也懒得搞虚拟机的，所以本程序暂时放下