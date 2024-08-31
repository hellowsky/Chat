#pragma once
#include"const.h"
// HttpConnection ������
class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
	// ���� HttpConnection ���һ����Ԫ��
	friend class LogicSystem;
	// ���캯��������һ���ѽ������ӵ� TCP �׽�����Ϊ����
	HttpConnection(tcp::socket socket);
	// ��ʼ��������
	void Start();
private:
	// ��鳬ʱʱ���Ƿ񵽴�
	void CheckDeadline();
	// д�� HTTP ��Ӧ
	void WriteResponse();
	// ���� HTTP ����
	void HandleRequest();
	void PreParseGetParam();
	tcp::socket _socket;// �ѽ������ӵ� TCP �׽���
	beast::flat_buffer _buffer{8192}; //8KB �Ļ����������ڶ�ȡ����
	http::request<http::dynamic_body> _request; //HTTP �������
	http::response<http::dynamic_body> _response;//HTTP ��Ӧ��
	//_socket.get_executor(): ����һ��ִ����������ȷ����ʱ���������̻߳��̳߳ء�_socket 
	// Ӧ����һ�� net::socket ���͵ĳ�Ա���������ڻ�ȡִ����
	//��ʱ���ĳ�ʱʱ��Ϊ 60 �롣����ζ�ŴӶ�ʱ��������ʼ��ʱ�����û������������Ԥ��60 ���ʱ����������ʱ�¼���
	net::steady_timer _deadline{ _socket.get_executor(),std::chrono::seconds(60)};// ��ʱ��ʱ��������Ϊ 60 ��
	//net::steady_timer _deadline(io_context, std::chrono::seconds(60));
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

