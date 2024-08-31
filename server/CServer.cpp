#include "CServer.h"
#include "HttpConnection.h"

// CServer ���캯��ʵ��
// ��ʼ������������
// @param ioc boost::asio::io_context �������ã����ڹ��� I/O ������
// @param port �����������Ķ˿ںš�
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
    : _ioc(ioc),
      _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
      _socket(ioc)
{
}

// ����������
// ��ʼ�����ͻ�����������
void CServer::Start()
{
    auto self = shared_from_this();
    _acceptor.async_accept(_socket, [self](beast::error_code ec) {
        try {
            // ��������������ӣ�����������������
            if (ec) {
                self->Start();
                return;
            }

            // ���������ӣ����Ҵ��� HttpConnection ������������
            std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

            // ��������
            self->Start();
        } catch (std::exception& e) {
            // �����쳣
            // �����������¼�쳣��Ϣ
        }
    });
}