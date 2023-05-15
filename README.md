- [环境](#环境)
- [前言](#前言)
- [正文](#正文)
  - [线程池的基本使用`asio::thread_pool`](#线程池的基本使用asiothread_pool)
  - [利用`asio`实现一个简单的同步回声`TCP` `server`与`client`](#利用asio实现一个简单的同步回声tcp-server与client)
    - [`server`](#server)
    - [`client`](#client)
  - [完成一个异步`TCP` `server`和`client`](#完成一个异步tcp-server和client)
    - [`server`](#server-1)

# 环境

**IDE**：**`Microsoft Visual Studio Enterprise 2022 (64 位) - Current`**
版本 **17.5.4**。

**C++语言标准**：**`/std:c++latest`**。

**操作系统**：**`Windows11`**。

**包管理器**：使用 **`vcpkg`** 下载和管理 **`fmt`** **`boost.asio`** 的依赖。

**项目类型**：原生的 **`.sln`**。

**CPU**：**`i7-13700H`**

<br>

# 前言

本文只用来介绍个人学习`boost.asio`的一些代码和理解，不承担其他任何责任。

且希望各位在此之前已经学过**网络编程**，不管是`linux`下的`C`风格系统`api`，还是`windows`下的`C`风格系统`api`，或者其他的`C++`网络库，或者其他语言的，都可以。

总之不要什么都没学过，包括对`C++`也是同理，应当具备一定的知识。

本文不一定有非常强的连贯性，纯属是看本人学习路程，或者有空了说不定就学了哪个，应当注意。

本文会使用到的只有`fmt`和`boost`以及标准库，不负责环境的配置。

<br>

# 正文

## 线程池的基本使用`asio::thread_pool`
```cpp
#include<iostream>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
using fmt::print;
namespace asio = boost::asio;

void my_task(int i) {
    print("This is my task {}.\n", i);
}

int main() {
    asio::thread_pool tp(4);

    for (int i = 0; i < 5; ++i) {
        asio::post(tp, [=] {my_task(i); });
    }

    for (int i = 0; i < 5; ++i) {
        asio::post(tp, [=] { print("This is lambda task {}.\n", i); });
    }

    /* 退出所有线程 */
    tp.join();
}
```

运行结果：

    This is my task 1.
    This is my task 0.
    This is my task 2.
    This is my task 4.
    This is my task 3.
    This is lambda task 0.
    This is lambda task 1.
    This is lambda task 2.
    This is lambda task 3.
    This is lambda task 4.

其实这没啥好说的，每次运行结果必然不一样，但是如果你多运行几次，我相信你们会有一个疑问。

“为什么好像这个乱序看起来**带了点顺序？** 它不会从中间打断，每次都是一句彻底打印完毕，乱也只是这些整句乱序”。

这个问题很简单，实际上和使用`fmt::print`无关，你用`std::cout`也是一样的：**同步的 C++ 流保证是线程安全的（从多个线程输出的单独字符可能交错，但无数据竞争）**，参见[文档](https://zh.cppreference.com/w/cpp/io/ios_base/sync_with_stdio)。

`asio::thread_pool`线程池类参见[文档](https://think-async.com/Asio/asio-1.18.1/doc/asio/reference/thread_pool.html)。
`asio::thread_pool tp(4);`就是构造具有四个线程的线程池。

`asio::post`是提交要执行的函数对象，并且不能像用`std::thread`那样构造，传个函数对象再传参数，即`asio::post(tp,my_task,i)`是错误的，必须得用`std::bind boost::bind`或者直接`lambda`表达式包一下传入即可。

更多的写法使用后面再慢慢增加`demo`和讲解。

<br>

## 利用`asio`实现一个简单的同步回声`TCP` `server`与`client`

### `server`

```cpp
#include<iostream>
#include<thread>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
using fmt::print;
namespace asio = boost::asio;
namespace ip = asio::ip;
using socket_ptr =  boost::shared_ptr<ip::tcp::socket>;

void client_session(socket_ptr sock) {
    static int i = 1;
    print("Connect {} ok!", i);
    while (true) {
        thread_local char data[512]{};
        size_t len = sock->read_some(asio::buffer(data));
        if (len > 0)
            write(*sock, asio::buffer(data, strlen(data) + 1));
    }
}
int main() {
    asio::thread_pool tp(10);
    asio::io_service service;
    ip::tcp::endpoint ep(ip::tcp::v4(), 2001); // 设置端口为2001
    ip::tcp::acceptor acc(service, ep);//server端才会用到的
   
    while (true) {
        socket_ptr sock(new ip::tcp::socket(service));
        acc.accept(*sock);
        asio::post(tp, [=] {client_session(sock); });
    }
    tp.join();
}
```

创建一个简单的`TCP` `server`只需要用到

[`asio::io_service`](https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/reference/io_service.html) **调度器**。

`io_service`对象是 **`asio`** 框架中的调度器，所有异步io事件都是通过它来分发处理的（`io`对象的构造函数中都需要传入一个`io_service`对象）。

    asio::io_service io_service;
    asio::ip::tcp::socket socket(io_service);

[`ip::tcp::endpoint`](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/ip__tcp/endpoint.html) **`TCP` 终结点的类型**。

主要存储一些基本信息：`IP` 端口。以供`server`的`acceptor`使用，或者`client`的`socket`调用`connect()`成员函数的时候传入。

[`ip::tcp::acceptor`](https://www.boost.org/doc/libs/1_62_0/doc/html/boost_asio/reference/ip__tcp/acceptor.html) **`TCP` 接受器类型**。

需要`asio::io_service`和`ip::tcp::endpoint`（即网络信息）配合使用，构造对象。
然后调用了`accept`成员函数，需要传入`socket`对象。

[`ip::tcp::socket`](https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/reference/ip__tcp/socket.html) **`TCP`套接字类型**。

可以调用`read_some()`成员函数读取套接字数据，或者使用`write_some()`成员函数将数据写入套接字，或者用于调用公共函数 **`boost::asio::write()`**，需要将`socket`作为第一个参数传入。或者在`client`端直接调用`connect()`成员函数，需要传入`ip::tcp::endpoint`类型，也就是需要一些基本的网络信息，`IP`，端口。

至于[`asio::buffer`](https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/reference/buffer.html)缓冲区似乎也没什么更多的介绍。

`asio::buffer`可以从内置数组创建单个缓冲区，`std::vector`，`std::array`或`boost::array POD` 元素数组。这有助于通过自动确定缓冲区的大小来防止缓冲区溢出。上面的情况传不传第二个参数都可以。

### `client`

```cpp
#include<iostream>
#include<thread>
#include<fmt/core.h>
#include<fmt/ranges.h>
#include<boost/asio.hpp>
using fmt::print;
namespace asio = boost::asio;
namespace ip = asio::ip;

void echo(ip::tcp::socket& sock) {
    char data[512]{};
    while (true) {
        print("clinet in: "); fgets(data, 512, stdin);
        write(sock, asio::buffer(data, strlen(data) + 1));
        size_t len = sock.read_some(asio::buffer(data));
        if (len != asio::error::eof)
            print("server out: {}", data);
        else
            break;
    }
}
int main() {
	asio::io_service service;//调度器
	ip::tcp::endpoint end_point(ip::address::from_string("127.0.0.1"), 2001);//构造存储基本server信息的对象
	ip::tcp::socket socket{ service };//构造tcp套接字类型，传入调度器
	socket.connect(end_point);//调用connect成员函数，传入存有server信息的end_point对象连接server。
    print("connect ok!\n");//连接成功打印
    echo(socket);//调用函数传入tcp套接字对象
}
```

实际上我觉得`client`已经不需要讲解了，已然非常的清晰明了了。

<br>

## 完成一个异步`TCP` `server`和`client`

### `server`

```cpp
#include<iostream>
#include<thread>
#include<functional>
#include<chrono>
#include<format>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;
using socket_ptr = boost::shared_ptr<ip::tcp::socket>;
using namespace std::placeholders;

asio::io_service service;
ip::tcp::endpoint ep{ ip::tcp::v4(),2001 };
ip::tcp::acceptor acc{ service,ep };//构造函数传入了ep，省去了调用bind成员函数的步骤
socket_ptr sock{ new ip::tcp::socket{service} };

void handle_accept(socket_ptr sock, const boost::system::error_code& err);
void start_accept(socket_ptr sock) {
	acc.async_accept(*sock, std::bind(handle_accept, sock, _1));//不堵塞 启动异步接受
	fmt::print("start_accept 表示这是异步的，并没有堵塞\n");
}
void handle_accept(socket_ptr sock, const boost::system::error_code& err) {//当接收到客户端连接时，handle_accept被调用
	if (err) return;
	sock->write_some(asio::buffer(std::format("{} server:笑死人了惹\n", std::chrono::system_clock::now())));

	//当完成使用后创建新的socket，再次调用start_accept()，创建等待另一个客户端连接的异步操作
	socket_ptr socket(new ip::tcp::socket(service));
	start_accept(socket);
}
int main() {
	start_accept(sock);
	fmt::print("main\n");
	service.run();
}
```

使用`nc`作为`client`：

当没有`client`连接的时候，打印：

    start_accept 表示这是异步的，并没有堵塞
    main

当有三个`client`连接的时候，打印：

    start_accept 表示这是异步的，并没有堵塞
    main
    start_accept 表示这是异步的，并没有堵塞
    start_accept 表示这是异步的，并没有堵塞
    start_accept 表示这是异步的，并没有堵塞

三个`client`都打印了一句

    2023-05-15 08:40:46.6725041 server:笑死人了惹

这是`server`发送的。
