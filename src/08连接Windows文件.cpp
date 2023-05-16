#include<iostream>
#include<boost/asio.hpp>
namespace asio = boost::asio;

int main() {
	HANDLE h = CreateFile(TEXT("1.txt"), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	asio::io_service service;
	asio::windows::stream_handle sh(service, h);
	sh.write_some(asio::buffer("笑死人了惹\n"));
}
//CreateFile的第六个参数非常重要：epol支持llinux的普通文件描述符，但是windows的普通文件句柄是同步的，iocp不支持，需要加上那个参数设置为异步的