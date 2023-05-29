#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include<mutex>
#include<functional>
#include<print>
using namespace boost::asio;

io_service service;
std::mutex m;

void func(int i) {
    std::lock_guard lc{ m };
    std::cout << "func called, i= " << i << "/" << boost::this_thread::get_id() << std::endl;
}
void worker_thread() {
    service.run();
}
int main(int argc, char* argv[]){
    io_service::strand strand_one(service), strand_two(service);//让异步方法被顺序调用
    for (int i = 0; i < 5; ++i)
        service.post(strand_one.wrap(boost::bind(func, i)));
    for (int i = 5; i < 10; ++i)
        service.post(strand_two.wrap(boost::bind(func, i)));
    boost::thread_group threads;
    for (int i = 0; i < 3; ++i)
        threads.create_thread(worker_thread);
    // 等待所有线程被创建完
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    threads.join_all();
}
//我们保证前面的5个线程和后面的5个线程是顺序执行