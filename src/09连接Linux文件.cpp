#include<iostream>
#include<boost/asio.hpp>
#include<unistd.h>
namespace asio = boost::asio;

int main() {
	asio::io_service service;
	int h = open("test.txt", O_RDWR | O_CREAT, 0);
	asio::posix::stream_descriptor sh(service);
	sh.assign(h);

	asio::write(sh, asio::buffer("笑死人了惹\n"));
}