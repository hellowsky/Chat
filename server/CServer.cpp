#include "CServer.h"
#include "HttpConnection.h"

// CServer 构造函数实现
// 初始化服务器对象。
// @param ioc boost::asio::io_context 对象引用，用于管理 I/O 操作。
// @param port 服务器监听的端口号。
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
    : _ioc(ioc),
      _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
      _socket(ioc)
{
}

// 启动服务器
// 开始监听客户端连接请求。
void CServer::Start()
{
    auto self = shared_from_this();
    _acceptor.async_accept(_socket, [self](beast::error_code ec) {
        try {
            // 出错，放弃这个链接，继续监听其他链接
            if (ec) {
                self->Start();
                return;
            }

            // 创建新链接，并且创建 HttpConnection 类管理这个链接
            std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

            // 继续监听
            self->Start();
        } catch (std::exception& e) {
            // 处理异常
            // 可以在这里记录异常信息
        }
    });
}