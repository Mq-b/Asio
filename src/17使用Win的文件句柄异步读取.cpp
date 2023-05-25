#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using namespace std::chrono_literals;
using namespace std::placeholders;

void handler(const boost::system::error_code&, size_t bytes, asio::streambuf& buf) {
	std::cout << "读取\n";
	std::istream in(&buf);
	std::string line;
	std::getline(in, line);
	std::cout << "first line: " << line << std::endl;
}
int main() {
	asio::streambuf buf;
	asio::io_context service;
	HANDLE h = CreateFile(TEXT("1.txt"), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	asio::windows::stream_handle h2(service, h);
	asio::async_read(h2, buf, asio::transfer_exactly(256), std::bind(handler, _1, _2, std::ref(buf)));//读取前256个字符，即第二个参数完成处理方法
	service.run();
}