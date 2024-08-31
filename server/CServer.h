#pragma once

#include "const.h"

// CServer �ඨ��
// CServer �ฺ����������˵�����ͨ�š�
// �̳��� std::enable_shared_from_this ��֧������ָ�빲��
class CServer : public std::enable_shared_from_this<CServer>
{
public: 
    // ���캯��
    // ��ʼ������������
    // @param ioc boost::asio::io_context �������ã����ڹ��� I/O ������
    // @param port �����������Ķ˿ںš�
    CServer(boost::asio::io_context& ioc, unsigned short& port);

    // ����������
    // ��ʼ�����ͻ�����������
    void Start();

private:
    // TCP ������
    // ���ڽ��տͻ�����������
    tcp::acceptor _acceptor;

    // I/O ������
    // ���ڹ��� I/O ������
    net::io_context& _ioc;

    // TCP �׽���
    // ������ͻ���ͨ�š�
    tcp::socket _socket;
};