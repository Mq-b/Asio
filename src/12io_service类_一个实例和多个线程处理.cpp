#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
#include<boost/thread.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock) {//连接成功后打印
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}
void timeout_handler(const boost::system::error_code&){//这个形参必须需要 
	fmt::print("笑死\n");
}
void run_service(asio::io_service& service)
{
	service.run();//会执行两次connect_handler
}
int main() {
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	asio::io_service service;
	ip::tcp::socket sock1(service);
	ip::tcp::socket sock2(service);
	sock1.async_connect(ep, std::bind(connect_handler, std::ref(sock1)));
	sock2.async_connect(ep, std::bind(connect_handler, std::ref(sock2)));
	
	asio::deadline_timer t(service, boost::posix_time::seconds(5));//会创建一个线程
	t.async_wait(timeout_handler);//异步计时器，等待一定时间后调用，在线程中执行
	
	boost::thread t2(run_service, std::ref(service));//线程将运行两个connect_handler函数
	t2.join();
}