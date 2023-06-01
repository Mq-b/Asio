#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include<print>

using namespace boost::asio;
io_service service;
void func(int i) {
    std::print("func called, i= {}\n", i);
}
void run_dispatch_and_post() {
    for (int i = 0; i < 10; i += 2) {
        service.dispatch(boost::bind(func, i));//����֮ǰ���ú���
        service.post(boost::bind(func, i + 1));//ֱ�ӷ���
    }
}
int main(int argc, char* argv[]) {
    service.post(run_dispatch_and_post);
    service.run();
}
/*ż���������Ȼ����������������Ϊ����dispatch()���ż����Ȼ����post()���������dispatch()���ڷ���֮ǰ����hanlder��
��Ϊ��ǰ���̵߳�����service.run()����post()ÿ�ζ����������ˡ�*/