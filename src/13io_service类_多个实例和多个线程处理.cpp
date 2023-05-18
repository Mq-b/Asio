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
void timeout_handler(const boost::system::error_code&) {//这个形参必须需要 
	fmt::print("笑死\n");//必然在第二个server的connect_handler运行之前执行完毕
}
void run_service(asio::io_service* service,int idx)
{
	service[idx].run();//计时器和第一个connect_handler共享了一个service[0]，那么必然延时和执行也会共享，所以会等待到五秒结束打印笑死
}
int main() {
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	asio::io_service service[2];
	ip::tcp::socket sock1(service[0]);
	ip::tcp::socket sock2(service[1]);
	sock1.async_connect(ep, std::bind(connect_handler, std::ref(sock1)));
	sock2.async_connect(ep, std::bind(connect_handler, std::ref(sock2)));

	asio::deadline_timer t(service[0], boost::posix_time::seconds(5));//第二个线程运行connect_handler必然晚于这个计时器任务
	t.async_wait(timeout_handler);

	for (int i = 0; i < 2; ++i) {
		boost::thread t(run_service, service, i);
		t.join();
	}
}