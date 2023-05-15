#include<iostream>
#include<thread>
#include<functional>
#include<chrono>
#include<format>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;
using socket_ptr = boost::shared_ptr<ip::tcp::socket>;
using namespace std::placeholders;

asio::io_service service;
ip::tcp::endpoint ep{ ip::tcp::v4(),2001 };
ip::tcp::acceptor acc{ service,ep };//构造函数传入了ep，省去了调用bind成员函数的步骤
socket_ptr sock{ new ip::tcp::socket{service} };

void handle_accept(socket_ptr sock, const boost::system::error_code& err);
void start_accept(socket_ptr sock) {
	acc.async_accept(*sock, std::bind(handle_accept, sock, _1));//不堵塞 启动异步接受
	fmt::print("start_accept 表示这是异步的，并没有堵塞\n");
}
void handle_accept(socket_ptr sock, const boost::system::error_code& err) {//当接收到客户端连接时，handle_accept被调用
	if (err) return;
	sock->write_some(asio::buffer(std::format("{} server:笑死人了惹\n", std::chrono::system_clock::now())));

	//当完成使用后创建新的socket，再次调用start_accept()，创建等待另一个客户端连接的异步操作
	socket_ptr socket(new ip::tcp::socket(service));
	start_accept(socket);
}
int main() {
	start_accept(sock);
	fmt::print("main\n");
	service.run();
}