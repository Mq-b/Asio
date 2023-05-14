#include<iostream>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
#include<format>
using fmt::print;
namespace asio = boost::asio;

void my_task(int i) {
    std::cout<<std::format("This is my task {}.\n", i);
}

int main() {
    asio::thread_pool tp(4);

    /* 将函数投放到线程池 */
    for (int i = 0; i < 5; ++i) {
        asio::post(tp, [=] {my_task(i); });
    }

    /* 将函数投放到线程池 */
    for (int i = 0; i < 5; ++i) {
        asio::post(tp, [=] { std::cout<<std::format("This is lambda task {}.\n", i); });
    }

    /* 退出所有线程 */
    tp.join();
}