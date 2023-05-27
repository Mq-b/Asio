#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using namespace std::placeholders;

int main() {
	asio::io_context service;
	HANDLE file = CreateFile(TEXT("1.txt"), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	asio::windows::random_access_handle h(service, file);
	asio::streambuf buf;
	read_at(h, 4, buf, asio::transfer_exactly(6));//���ļ�ƫ��4��λ�ö�ȡ6���ֽ�
	std::istream in(&buf);
	std::string line;
	std::getline(in, line);
	std::cout << "first line: " << line << std::endl;
}