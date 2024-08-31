#pragma once
#include"const.h"
// ���� HttpConnection ���ǰ������
// ��ʹ���ڲ���Ҫ���� HttpConnection �������������¾Ϳ���������
class HttpConnection;
// ���崦�� HTTP ����Ļص���������
// ����һ����׼�����������ͣ�������һ�� HttpConnection �Ĺ���ָ����Ϊ����
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
// LogicSystem ������
class LogicSystem:public Singleton<LogicSystem>
{
	//����Ϊ��Ԫ �����޷�����˽�еĹ��캯��
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {}
	// ����GET����       ���ڴ������GET���󣬲�������Ӧ�Ĵ�����
	bool HandleGet(std::string url,std::shared_ptr<HttpConnection> conn);
	//����POST����
	bool HandlePost(std::string url, std::shared_ptr<HttpConnection> conn);

	// ע��GET��������  ����ע�ᴦ���ض�URL·����GET����Ĵ�����
	void RegGet(std::string url,HttpHandler handler);
	// ע��POST��������
	void RegPost(std::string url, HttpHandler handler);
private:
	// ���캯����˽�еģ���Ϊ����һ��������
	LogicSystem();
	//�洢ÿ��URL·�������Ӧ�Ĵ���������
	std::unordered_map<std::string, HttpHandler> _post_handlers;
	std::unordered_map<std::string, HttpHandler> _get_handlers;

};

