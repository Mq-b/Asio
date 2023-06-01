#include<thread>
#include<functional>
#include<print>
#include<iostream>
#include <boost/asio.hpp>
using namespace std::literals;
using namespace boost::asio;

io_service service;
void dispatched_func_1() {
    std::cout << "dispatched 1" << std::endl;
}
void dispatched_func_2() {
    std::cout << "dispatched 2" << std::endl;
}
void test(std::function<void()> func) {
    std::cout << "test" << std::endl;
    service.dispatch(dispatched_func_1);//这里用post也一样
    func();//最后调用
}
void service_run() {
    service.run();
}
int main(int argc, char* argv[]) {
    test(service.wrap(dispatched_func_2));
    std::jthread th(service_run);
}