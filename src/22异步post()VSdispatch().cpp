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
        service.dispatch(boost::bind(func, i));//返回之前调用函数
        service.post(boost::bind(func, i + 1));//直接返回
    }
}
int main(int argc, char* argv[]) {
    service.post(run_dispatch_and_post);
    service.run();
}
/*偶数先输出，然后是奇数。这是因为我用dispatch()输出偶数，然后用post()输出奇数。dispatch()会在返回之前调用hanlder，
因为当前的线程调用了service.run()，而post()每次都立即返回了。*/