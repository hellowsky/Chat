#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CServer.h"

int main()
{
    try
    {
        // 设置服务器端口
        unsigned short port = static_cast<unsigned short>(8080);

        // 创建 io_context 实例
        net::io_context ioc{ 1 };

        // 创建信号集，用于监听中断信号
        //创建一个 signal_set 对象 signals，它绑定到了 io_context 对象 ioc 上，
        //并注册了两个信号：SIGINT 和 SIGTERM。
        //SIGINT 通常是由用户按下 Ctrl + C 触发的。
        //SIGTERM 通常是由操作系统或父进程发送的终止信号
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

        // 异步等待信号
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
            if (error) {
                // 如果发生错误，则直接返回
                return;
            }
            // 如果收到中断信号，则停止 io_context
            ioc.stop();
            });

        // 创建并启动服务器
        std::make_shared<CServer>(ioc, port)->Start();
        std::cout<<"Gate Server listen on port" << port << std::endl;

        // 运行 io_context，开始监听事件
        ioc.run();
    }
    catch (const std::exception& e)
    {
        // 捕获异常并输出错误信息
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}