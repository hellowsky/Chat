#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CServer.h"

int main()
{
    try
    {
        // ���÷������˿�
        unsigned short port = static_cast<unsigned short>(8080);

        // ���� io_context ʵ��
        net::io_context ioc{ 1 };

        // �����źż������ڼ����ж��ź�
        //����һ�� signal_set ���� signals�����󶨵��� io_context ���� ioc �ϣ�
        //��ע���������źţ�SIGINT �� SIGTERM��
        //SIGINT ͨ�������û����� Ctrl + C �����ġ�
        //SIGTERM ͨ�����ɲ���ϵͳ�򸸽��̷��͵���ֹ�ź�
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

        // �첽�ȴ��ź�
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {
            if (error) {
                // �������������ֱ�ӷ���
                return;
            }
            // ����յ��ж��źţ���ֹͣ io_context
            ioc.stop();
            });

        // ����������������
        std::make_shared<CServer>(ioc, port)->Start();
        std::cout<<"Gate Server listen on port" << port << std::endl;

        // ���� io_context����ʼ�����¼�
        ioc.run();
    }
    catch (const std::exception& e)
    {
        // �����쳣�����������Ϣ
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}