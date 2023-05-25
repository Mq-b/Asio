#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using namespace std::placeholders;

typedef asio::buffers_iterator<asio::streambuf::const_buffers_type> iterator;
std::pair<iterator, bool> match_punct(iterator begin, iterator end) {
    while (begin != end) {
        if (*begin == '\n')
            return { begin, true };//如果是符号，那么read结束（根据第二个参数）
        begin++;
    }
    std::cout << "可以结束read" << std::endl;
    return { end, false };
}
void on_read(const boost::system::error_code&, size_t,asio::streambuf& buf) {
    std::cout << "读取" << std::endl;
    std::istream in(&buf);
    std::string line;
    std::getline(in, line);
    std::cout << "first line: " << line << std::endl;
}
int main() {
    asio::streambuf buf;
    asio::io_context service;
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 2001);
    ip::tcp::socket sock{ service };
    sock.connect(ep);
    asio::async_read_until(sock, buf, match_punct, std::bind(on_read, _1, _2, std::ref(buf)));
    service.run();
}