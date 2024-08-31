#pragma once
#include"const.h"
// 定义 HttpConnection 类的前向声明
// 这使得在不需要包含 HttpConnection 完整定义的情况下就可以引用它
class HttpConnection;
// 定义处理 HTTP 请求的回调函数类型
// 这是一个标准函数对象类型，它接受一个 HttpConnection 的共享指针作为参数
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
// LogicSystem 类声明
class LogicSystem:public Singleton<LogicSystem>
{
	//声明为友元 否则无法调用私有的构造函数
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem() {}
	// 处理GET请求       用于处理传入的GET请求，并调用相应的处理器
	bool HandleGet(std::string url,std::shared_ptr<HttpConnection> conn);
	//处理POST请求
	bool HandlePost(std::string url, std::shared_ptr<HttpConnection> conn);

	// 注册GET请求处理器  用于注册处理特定URL路径的GET请求的处理器
	void RegGet(std::string url,HttpHandler handler);
	// 注册POST请求处理器
	void RegPost(std::string url, HttpHandler handler);
private:
	// 构造函数是私有的，因为这是一个单例类
	LogicSystem();
	//存储每个URL路径及其对应的处理器函数
	std::unordered_map<std::string, HttpHandler> _post_handlers;
	std::unordered_map<std::string, HttpHandler> _get_handlers;

};

