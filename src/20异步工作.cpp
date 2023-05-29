#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include<functional>
#include<print>
#include<mutex>
using namespace boost::asio;
std::mutex m;

void func(int i) {
    std::lock_guard lc{ m };
    std::cout << std::this_thread::get_id() << ' ';
    std::print("func called��i= {}\n", i);
}
void worker_thread(io_service& service) {
    service.run();
}

int main(int argc, char* argv[]) {
    std::cout << "main ID: " << std::this_thread::get_id() << '\n';
    io_service service;
    for (int i = 0; i < 10; ++i)
        service.post(boost::bind(func, i));//io_context��֤���������ڵ�ǰ���ڵ��� run() ��run_one() ��poll() �� poll_one() ����Ա�������߳��е���
    boost::thread_group threads;//�̳߳ض���
    for (int i = 0; i < 3; ++i)
        threads.create_thread(std::bind(worker_thread, std::ref(service)));
    // �ȴ������̱߳�������
    boost::this_thread::sleep(boost::posix_time::millisec(500));
    threads.join_all();
}