- [环境](#环境)
- [前言](#前言)
- [正文](#正文)
	- [线程池的基本使用`asio::thread_pool`](#线程池的基本使用asiothread_pool)
	- [利用`asio`实现一个简单的同步回声`TCP` `server`与`client`](#利用asio实现一个简单的同步回声tcp-server与client)
		- [`server`](#server)
		- [`client`](#client)
	- [完成一个异步`TCP` `server`和`client`](#完成一个异步tcp-server和client)
		- [`server`](#server-1)
		- [`client`](#client-1)
	- [`asio`信号量](#asio信号量)
	- [端口设置与读写](#端口设置与读写)
	- [使用`asio::io_service`连接到系统文件](#使用asioio_service连接到系统文件)
		- [`windows`](#windows)
		- [`posix`](#posix)
	- [计时器](#计时器)
	- [`asio::io_service`类](#asioio_service类)
		- [一个io\_service实例和一个处理线程的单线程例子](#一个io_service实例和一个处理线程的单线程例子)
		- [一个`io_service`实例和多个处理线程的多线程例子](#一个io_service实例和多个处理线程的多线程例子)
		- [多个`io_service`实例和多个处理线程的多线程例子](#多个io_service实例和多个处理线程的多线程例子)
		- [总结](#总结)
	- [IP地址](#ip地址)
	- [端点](#端点)
	- [`UDP`同步`client`与异步`service`](#udp同步client与异步service)
		- [`service`](#service)
		- [`client`](#client-2)
	- [套接字缓冲区](#套接字缓冲区)
	- [缓冲区封装函数](#缓冲区封装函数)
	- [`connect`方法](#connect方法)
	- [`read/write`方法](#readwrite方法)

# 环境

**IDE**：**`Microsoft Visual Studio Enterprise 2022 (64 位) - Current`**
版本 **17.5.4**。

**编译器**：**`MSVC2022`** & **`gcc11.3`**

**C++语言标准**：**`/std:c++latest`**。

**操作系统**：**`Windows11`** & **`Ubuntu22.04（wsl）`**。

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

### `client`

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock, const boost::system::error_code& ec) {//连接成功后打印
	if (ec) return;
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}

int main() {
	asio::io_service service;
	ip::tcp::socket sock{ service };
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	sock.async_connect(ep, [&](const auto& ec)
		{connect_handler(sock,ec); });
	fmt::print("main\n");
	service.run();//只要还有待处理的异步操作，servece.run()循环就会一直运行
}
```

和上面的`server`一起运行，打印：

    main
    client: 2023-05-15 11:26:38.7514194 server:笑死人了惹

事实上`client`的逻辑十分简单。和同步并无多大区别。

只要还有待处理的异步操作，`servece.run()`循环就会一直运行。在上述例子中，只执行了一个这样的操作，就是`socket`的`async_connect`。在这之后，`service.run()`就退出了。

<br>

## `asio`信号量

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;

void signal_handler(const boost::system::error_code& err, int signal) {
	switch (signal) {
	case SIGINT:fmt::print("SIGNINT\n");
		break;
	case SIGTERM:fmt::print("SIGTERM\n");
		break;
	default:
		break;
	}
}
int main() {
	asio::io_service service;
	asio::signal_set sig(service, SIGINT, SIGTERM);//可以使用构造函数传入需要处理的信号，也可以直接用add方法
	sig.async_wait(signal_handler);////设置触发函数
	service.run();//没有收到信号就会一直堵塞循环运行
	fmt::print("End\n");
}
```

运行之后在控制台输入`CTRL+C`打印：

    SIGNINT
    End


[**`asio::signal_set`**](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/signal_set.html)

<br>

## 端口设置与读写

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;

int main() {
	asio::io_service service;
	asio::serial_port sp( service);//在Linux端口名称是/dev/ttyS0
	try{
		sp.open("COM1");
	}
	catch (const std::exception&e){
		std::cout << e.what() << '\n';
		exit(0);
	}
	asio::serial_port::baud_rate rate(8009);
	sp.set_option(rate);

	char data[512]{};
	read(sp, asio::buffer(data));
	fmt::print("{}", data);
}
```

实际上本程序由于我没有外部设备，也懒得整什么虚拟串口，所以并未调试运行。

<br>

## 使用`asio::io_service`连接到系统文件

### `windows`

```cpp
#include<iostream>
#include<boost/asio.hpp>
namespace asio = boost::asio;

int main() {
	HANDLE h = CreateFile(TEXT("1.txt"), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	asio::io_service service;
	asio::windows::stream_handle sh(service, h);
	sh.write_some(asio::buffer("笑死人了惹\n"));
}
```

`CreateFile`的第六个参数非常重要：`epol`支持`linux`的普通文件描述符，但是`windows`的普通文件句柄是**同步**的，`iocp`不支持，需要加上那个参数设置为异步的。

<br>

### `posix`

```cpp
#include<iostream>
#include<boost/asio.hpp>
#include<unistd.h>
namespace asio = boost::asio;

int main() {
	asio::io_service service;
	int h = open("test.txt", O_RDWR | O_CREAT, 0);
	asio::posix::stream_descriptor sh(service);
	sh.assign(h);

	asio::write(sh, asio::buffer("笑死人了惹\n"));
}
```

两个程序并无太大区别，但是我特意在`asio`的接口上展现了一点不同：
    `windows`写入数据用的是成员函数`write_some()`

`linux`是公共函数`asio::write`；

并且windows绑定文件资源句柄是用构造函数，Linux是用的`assign()`成员函数。

这些都是无所谓的，只是为了展示可以，你大可随意。

`asio`上真的不同的无非是 [**`asio::windows::stream_handle`**](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/windows__stream_handle.html) 和 [**`asio::posix::stream_descriptor`**](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_asio/reference/posix__stream_descriptor.html)，必须是固定平台使用。

<br>

## 计时器

```cpp
#include<iostream>
#include<boost/asio.hpp>
namespace asio = boost::asio;
using namespace std::placeholders;
bool read = false;

void deadline_handler(const boost::system::error_code&) {
    std::cout << (read ? "read successfully" : "read failed") << std::endl;
}
void read_handler(const boost::system::error_code&) {
    read = true;
}
int main() {
    asio::io_service service;
    asio::ip::tcp::socket sock(service);
    char data[512]{};
    sock.async_read_some(asio::buffer(data), std::bind(read_handler, _1));
    asio::deadline_timer t(service, boost::posix_time::milliseconds(100));
    t.async_wait(&deadline_handler);
    service.run();
}
```

运行结果：

    read successfully

`asio::deadline_timer`会创建一个线程进行计时，`t.async_wait`等待到达时间后调用函数在线程中执行。

<br>

## `asio::io_service`类

**配合`04异步服务端`运行**。

### 一个io_service实例和一个处理线程的单线程例子

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock, const boost::system::error_code& ec) {//连接成功后打印
	if (ec) return;
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}
void timeout_handler(const boost::system::error_code&) {
	fmt::print("笑死\n");
}
int main() {
	asio::io_service service;
	ip::tcp::socket sock{ service };
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	sock.async_connect(ep, [&](const auto& ec)
		{connect_handler(sock, ec); });
	asio::deadline_timer t(service, boost::posix_time::seconds(5));
	t.async_wait(timeout_handler);
	service.run();
}
```

运行结果：

    client: 2023-05-18 01:39:23.9009556 server:笑死人了惹

    笑死

这实际上和04的异步`client`差不多，就不过多介绍了

<br>

### 一个`io_service`实例和多个处理线程的多线程例子

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
#include<boost/thread.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock) {//连接成功后打印
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}
void timeout_handler(const boost::system::error_code&){//这个形参必须需要 
	fmt::print("笑死\n");
}
void run_service(asio::io_service& service)
{
	service.run();//会执行两次connect_handler
}
int main() {
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	asio::io_service service;
	ip::tcp::socket sock1(service);
	ip::tcp::socket sock2(service);
	sock1.async_connect(ep, std::bind(connect_handler, std::ref(sock1)));
	sock2.async_connect(ep, std::bind(connect_handler, std::ref(sock2)));
	
	asio::deadline_timer t(service, boost::posix_time::seconds(5));//会创建一个线程
	t.async_wait(timeout_handler);//异步计时器，等待一定时间后调用，在线程中执行
	
	boost::thread t2(run_service, std::ref(service));//线程将运行两个connect_handler函数
	t2.join();
}
```

运行结果：

    client: 2023-05-18 01:42:54.6338016 server:笑死人了惹

    client: 2023-05-18 01:42:54.6343521 server:笑死人了惹

    笑死

<br>

### 多个`io_service`实例和多个处理线程的多线程例子

```cpp
#include<iostream>
#include<fmt/core.h>
#include<boost/asio.hpp>
#include<boost/thread.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;

void connect_handler(ip::tcp::socket& sock) {//连接成功后打印
	static char data[1024]{};
	sock.read_some(asio::buffer(data));
	fmt::print("client: {}\n", data);
}
void timeout_handler(const boost::system::error_code&) {//这个形参必须需要 
	fmt::print("笑死\n");//必然在第二个server的connect_handler运行之前执行完毕
}
void run_service(asio::io_service* service,int idx)
{
	service[idx].run();//计时器和第一个connect_handler共享了一个service[0]，那么必然延时和执行也会共享，所以会等待到五秒结束打印笑死
}
int main() {
	ip::tcp::endpoint ep{ ip::address::from_string("127.0.0.1"),2001 };
	asio::io_service service[2];
	ip::tcp::socket sock1(service[0]);
	ip::tcp::socket sock2(service[1]);
	sock1.async_connect(ep, std::bind(connect_handler, std::ref(sock1)));
	sock2.async_connect(ep, std::bind(connect_handler, std::ref(sock2)));

	asio::deadline_timer t(service[0], boost::posix_time::seconds(5));//第二个线程运行connect_handler必然晚于这个计时器任务
	t.async_wait(timeout_handler);

	for (int i = 0; i < 2; ++i) {
		boost::thread t(run_service, service, i);
		t.join();
	}
}
```

运行结果：

    client: 2023-05-18 03:20:55.0917810 server:笑死人了惹

    笑死
    client: 2023-05-18 03:20:55.0926932 server:笑死人了惹

<br>

### 总结

需要一直记住的是如果没有其他需要监控的操作，**`.run()`就会结束**，就像下面的代码片段：

```cpp
io_service service; 
tcp::socket sock(service); 
sock.async_connect( ep, connect_handler); 
service.run();
```

在上面的例子中，只要`sock`建立了一个连接，`connect_handler`就会被调用，然后接着`service.run()`就会完成执行。

如果你想要`service.run()`接着执行，你需要分配更多的工作给它。这里有两个方式来完成这个目标。在`connect_handler`中启动另外一个异步操作来分配更多的工作。

<br>

## IP地址

对于`IP`地址的处理，`Boost.Asio`提供了`ip::address` , `ip::address_v4`和`ip::address_v6`类。 它们提供了相当多的函数。下面列出了最重要的几个：

`ip::address(v4_or_v6_address)`:这个函数把一个v4或者v6的地址转换成`ip::address`。

`ip::address:from_string(str)`：这个函数根据一个`IPv4`地址（用.隔开的）或者一个`IPv6`地址（十六进制表示）创建一个地址。

`ip::address::to_string()` ：这个函数返回这个地址的字符串。

`ip::address_v4::broadcast([addr, mask])`:这个函数创建了一个广播地址 `ip::address_v4::any()`：这个函数返回一个能表示任意地址的地址。

`ip::address_v4::loopback(), ip_address_v6::loopback()`：这个函数返回环路地址（为v4/v6协议）

`ip::host_name()`：这个函数用`string`数据类型返回当前的主机名。

大多数情况你会选择用`ip::address::from_string`：

```cpp
ip::address addr = ip::address::from_string("127.0.0.1");
```

如果你想通过一个主机名进行连接，下面的代码片段是无用的：

```cpp
// 抛出异常
ip::address addr = ip::address::from_string("www.yahoo.com");
```

<br>

## 端点
端点是使用某个端口连接到的一个地址。不同类型的socket有它自己的endpoint类，比如ip::tcp::endpoint、`ip::udp::endpoint`和`ip::icmp::endpoint`

如果想连接到本机的80端口，你可以这样做：


```cpp
ip::tcp::endpoint ep( ip::address::from_string("127.0.0.1"), 80);
```

有三种方式来让你建立一个端点：

`endpoint()`：这是默认构造函数，某些时候可以用来创建UDP/ICMP socket。

`endpoint(protocol, port)`：这个方法通常用来创建可以接受新连接的服务器端socket。

`endpoint(addr, port)`:这个方法创建了一个连接到某个地址和端口的端点。

例子如下：

```cpp
ip::tcp::endpoint ep1;
ip::tcp::endpoint ep2(ip::tcp::v4(), 80);
ip::tcp::endpoint ep3( ip::address::from_string("127.0.0.1), 80);
```

如果你想连接到一个主机（不是IP地址），你需要这样做：

```cpp
asio::io_service service;
ip::tcp::resolver resolver(service);
ip::tcp::resolver::query query("www.baidu.com", "80");
ip::tcp::resolver::iterator iter = resolver.resolve(query);
ip::tcp::endpoint ep = *iter;
fmt::print("IP: {} Port: {} 协议类型：{}\n", ep.address().to_string(), ep.port(), ep.protocol().type());
```

可以使用`nslookup`命令验证结果：

	PS C:\Users\A1387> nslookup www.baidu.com
	Server:  UnKnown
	Address:  192.168.43.1

	Non-authoritative answer:
	Name:    www.a.shifen.com
	Addresses:  36.152.44.96
	          36.152.44.95
	Aliases:  www.baidu.com

## `UDP`同步`client`与异步`service`

### `service`

```cpp
#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;

asio::io_context service;
ip::udp::socket sock(service);
asio::ip::udp::endpoint sender_ep;
char buff[512]{};

void on_read(const boost::system::error_code & err, std::size_t read_bytes) {
  std::cout << "read " << read_bytes <<" "<<buff<< std::endl;
  sock.async_receive_from(asio::buffer(buff), sender_ep, on_read);///异步读取，循环调用当前函数
}
int main(){
    ip::udp::endpoint ep(ip::udp::v4(),80);
    sock.open(ep.protocol());//指定打开的协议类型
    sock.set_option(asio::ip::udp::socket::reuse_address(true));//设置套接字属性 用于允许套接字绑定到已在使用的地址
    sock.bind(ep);
    sock.async_receive_from(asio::buffer(buff),sender_ep,on_read);
    service.run();
}
```

### `client`

```cpp
#include <iostream>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
int main()
{
	asio::io_context service;
  	ip::udp::socket sock(service);
	sock.open(ip::udp::v4());
	ip::udp::endpoint receiver_ep(ip::address::from_string("127.0.0.1"),80);
	sock.send_to(asio::buffer("testing\n"),receiver_ep);//同步发送数据
	char buff[512]{};
	ip::udp::endpoint sender_ep{};
	sock.receive_from(asio::buffer(buff),sender_ep);//异步地从一个指定的端点获取数据并写入到给定的缓冲区。在读完所有数据或者错误出现之前，这个函数都是阻塞的
}
```

配合着运行结果是：

	read 9 testing

值得一提的是`client`的最后一句`receive_from`成员函数调用，会一直阻塞。因为`service`压根就不会发送数据，它是接收数据的。

另外你可以注意到了，我们`UDP`用到了很多的**空`asio::ip::udp::endpoint`** 对象，设计如此，不用太过在意。

<br>

## 套接字缓冲区

当从一个套接字读写内容时，你需要一个缓冲区，用来保存读取和写入的数据。缓冲区内存的有效时间必须比`I/O`操作的时间要长；你需要保证它们在`I/O`操作结束之前不被释放。 对于同步操作来说，这很容易；当然，这个缓冲区在`receive`和`send`时都存在。

```cpp
char buff[512];
...
sock.receive(buffer(buff));
strcpy(buff, "ok\n");
sock.send(buffer(buff));
```

但是在**异步操作时就没这么简单了**，看下面的代码片段：

```cpp
// 非常差劲的代码 ...
void on_read(const boost::system::error_code & err, std::size_t read_bytes)
{ ... }
void func() {
    char buff[512];
    sock.async_receive(buffer(buff), on_read);
}
```
在我们调用`async_receive()`之后，`buff`就已经超出有效范围，它的内存当然会被释放。当我们开始从套接字接收一些数据时，我们会把它们拷贝到一片已经不属于我们的内存中；它可能会被释放，或者被其他代码重新开辟来存入其他的数据，结果就是：内存冲突。

对于上面的问题有几个解决方案：

* 使用全局缓冲区
* 创建一个缓冲区，然后在操作结束时释放它
* 使用一个集合对象管理这些套接字和其他的数据，比如缓冲区数组

第一个方法显然不是很好，因为我们都知道全局变量非常不好。此外，如果两个实例使用同一个缓冲区怎么办？

下面是第二种方式的实现：

```cpp
void on_read(char * ptr, const boost::system::error_code & err, std::size_t read_bytes) {                        
    delete[] ptr;
}
....
char * buff = new char[512];
sock.async_receive(buffer(buff, 512), boost::bind(on_read,buff,_1,_2))

```

这种方式挺好的。如果你想要缓冲区在操作结束后自动超出范围，使用共享指针

```cpp
struct shared_buffer {
    boost::shared_array<char> buff;
    int size;
    shared_buffer(size_t size) : buff(new char[size]), size(size) {
    }
    mutable_buffers_1 asio_buff() const {
        return buffer(buff.get(), size);
    }
};


// 当on_read超出范围时, boost::bind对象被释放了,
// 同时也会释放共享指针
void on_read(shared_buffer, const boost::system::error_code & err, std::size_t read_bytes) {}
sock.async_receive(buff.asio_buff(), boost::bind(on_read,buff,_1,_2));
```

<br>

## 缓冲区封装函数

纵观所有代码，你会发现：无论什么时候，当我们需要对一个`buffer`进行读写操作时，代码会把实际的缓冲区对象封装在一个`buffer()`方法中，然后再把它传递给方法调用：

```cpp
char buff[512];
sock.async_receive(buffer(buff), on_read);
```

基本上我们都会把缓冲区包含在一个类中以便`Boost.Asio`的方法能遍历这个缓冲区，比方说，使用下面的代码：

```cpp
sock.async_receive(some_buffer, on_read);
```

实例`some_buffer`需要满足一些需求，叫做**`ConstBufferSequence`**或者**`MutableBufferSequence`**（你可以在`Boost.Asio`的文档中查看它们）。创建你自己的类去处理这些需求的细节是非常复杂的，但是`Boost.Asio`已经提供了一些类用来处理这些需求。所以你不用直接访问这些缓冲区，而可以使用`buffer()`方法。

自信地讲，你可以把下面列出来的类型都包装到一个buffer()方法中：

	一个char[] const 数组
	一个字节大小的void *指针
	一个std::string类型的字符串
	一个POD const数组（POD代表纯数据，这意味着构造器和释放器不做任何操作）
	一个pod数据的std::vector
	一个包含pod数据的boost::array
	一个包含pod数据的std::array
下面的代码都是有效的：

```cpp
struct pod_sample { int i; long l; char c; };
...
char b1[512];
void * b2 = new char[512];
std::string b3; b3.resize(128);
pod_sample b4[16];
std::vector<pod_sample> b5; b5.resize(16);
boost::array<pod_sample,16> b6;
std::array<pod_sample,16> b7;
sock.async_send(buffer(b1), on_read);
sock.async_send(buffer(b2,512), on_read);
sock.async_send(buffer(b3), on_read);
sock.async_send(buffer(b4), on_read);
sock.async_send(buffer(b5), on_read);
sock.async_send(buffer(b6), on_read);
sock.async_send(buffer(b7), on_read);
```

总的来说就是：与其创建你自己的类来处理`ConstBufferSequence`或者`MutableBufferSequence`的需求，不如创建一个能在你需要的时候保留缓冲区，然后返回一个`mutable_buffers_1`实例的类，而我们早在`shared_buffer`类中就这样做了。

## `connect`方法

这些方法把套接字连接到一个端点。

`connect(socket, begin [, end] [, condition])`：这个方法遍历队列中从start到end的端点来尝试同步连接。begin迭代器是调用`socket_type::resolver::query`的返回结果（你可能需要回顾一下端点这个章节）。特别提示end迭代器是可选的；你可以忽略它。你还可以提供一个condition的方法给每次连接尝试之后调用。用法是Iterator connect_condition(const `boost::system::error_code & err,Iterator next)`;。你可以选择返回一个不是next的迭代器，这样你就可以跳过一些端点。

`async_connect(socket, begin [, end] [, condition], handler)`：这个方法异步地调用连接方法，在结束时，它会调用完成处理方法。用法是`void handler(constboost::system::error_code & err, Iterator iterator)`;。传递给处理方法的第二个参数是连接成功端点的迭代器（或者end迭代器）。

```cpp
int main(){
    asio::io_context service;
    ip::tcp::resolver resolver(service);
    ip::tcp::resolver::iterator iter = resolver.resolve(ip::tcp::resolver::query("www.baidu.com","80"));
    ip::tcp::socket sock(service);
    connect(sock, iter);
    char buf[512]{};
    read(sock,asio::buffer(buf));//读不到东西，会堵塞在这里
    std::cout<<buf<<'\n';
}
```

<br>

## `read/write`方法
这些方法对一个流进行读写操作（可以是套接字，或者其他表现得像流的类）：

* `async_read(stream, buffer [, completion] ,handler)`：这个方法异步地从一个流读取。结束时其处理方法被调用。处理方法的格式是：
  
  `void handler(const boost::system::error_code & err, size_t bytes);`。

  你可以选择指定一个完成处理方法。完成处理方法会在每个read操作调用成功之后调用，然后告诉`Boost.Asio async_read`操作是否完成（如果没有完成，它会继续读取）。它的格式是：

  `size_t completion(const boost::system::error_code& err, size_t bytes_transfered)` 。

  当这个完成处理方法返回0时，我们认为`read`操作完成；如果它返回一个非0值，它表示了下一个`async_read_some`操作需要从流中读取的字节数。接下来会有一个例子来详细展示这些。

* `async_write(stream, buffer [, completion], handler)`：这个方法异步地向一个流写入数据。参数的意义和async_read是一样的。
  
* `read(stream, buffer [, completion])`：这个方法同步地从一个流中读取数据。参数的意义和async_read是一样的。
  
* `write(stream, buffer [, completion])`: 这个方法同步地向一个流写入数据。参数的意义和async_read是一样的。

```cpp
async_read(stream, stream_buffer [, completion], handler)
async_write(strean, stream_buffer [, completion], handler)
write(stream, stream_buffer [, completion])//方括号代表可选，write和read可以只需要两个参数
read(stream, stream_buffer [, completion])
```

首先，要注意第一个参数变成了流，**而不单是`socket`**。这个参数包含了`socket`**但不仅仅是`socket`**。比如，你可以用一个`Windows`的**文件句柄**来替代`socket`。 当下面情况出现时，所有`read`和`write`操作都会结束：

* 可用的缓冲区满了（当读取时）或者所有的缓冲区已经被写入（当写入时）
* 完成处理方法返回0（如果你提供了这么一个方法）
* 错误发生时

下面的代码会异步地从一个socket中间读取数据直到读取到’\n’：

```cpp
io_service service;
ip::tcp::socket sock(service);
char buff[512];
size_t up_to_enter(const boost::system::error_code &, size_t bytes) {
    for ( size_t i = 0; i < bytes; ++i)
        if ( buff[i] == '\n') 
            return 0;
    return 1; 
 }
void on_read(const boost::system::error_code &, size_t) {}
...
async_read(sock, buffer(buff), up_to_enter, on_read);
```

这里的`up_to_endter()`是**完成处理函数**，它会等待，**`handler()`** 完成，即`on_read()`函数运行完被调用。

我们和`windows`的句柄搭配使用做了一个更好的例子

```cpp
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
```

运行结果：

	读取
	first line: 笑死人了惹

这个例子相比于上一个，更加关注的是 **`async_read()`**的第三个参数，而不是第二个，这里是直接用了一个内建的仿函数。