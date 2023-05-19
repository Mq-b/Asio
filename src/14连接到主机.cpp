#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;
//nslookup www.baidu.com验证
int main() {
	asio::io_service service;
	ip::tcp::resolver resolver(service);
	ip::tcp::resolver::query query("www.baidu.com", "80");
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::endpoint ep = *iter;
	fmt::print("IP: {} Port: {} 协议类型：{}\n", ep.address().to_string(), ep.port(), ep.protocol().type());
}