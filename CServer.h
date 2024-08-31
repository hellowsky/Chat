#pragma once

#include "const.h"

// CServer 类定义
// CServer 类负责处理服务器端的网络通信。
// 继承自 std::enable_shared_from_this 以支持智能指针共享。
class CServer : public std::enable_shared_from_this<CServer>
{
public: 
    // 构造函数
    // 初始化服务器对象。
    // @param ioc boost::asio::io_context 对象引用，用于管理 I/O 操作。
    // @param port 服务器监听的端口号。
    CServer(boost::asio::io_context& ioc, unsigned short& port);

    // 启动服务器
    // 开始监听客户端连接请求。
    void Start();

private:
    // TCP 接收器
    // 用于接收客户端连接请求。
    tcp::acceptor _acceptor;

    // I/O 上下文
    // 用于管理 I/O 操作。
    net::io_context& _ioc;

    // TCP 套接字
    // 用于与客户端通信。
    tcp::socket _socket;
};