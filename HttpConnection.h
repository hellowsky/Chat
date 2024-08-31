#pragma once
#include"const.h"
// HttpConnection 类声明
class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
	// 声明 HttpConnection 类的一个友元类
	friend class LogicSystem;
	// 构造函数，接受一个已建立连接的 TCP 套接字作为参数
	HttpConnection(tcp::socket socket);
	// 开始处理请求
	void Start();
private:
	// 检查超时时间是否到达
	void CheckDeadline();
	// 写入 HTTP 响应
	void WriteResponse();
	// 处理 HTTP 请求
	void HandleRequest();
	void PreParseGetParam();
	tcp::socket _socket;// 已建立连接的 TCP 套接字
	beast::flat_buffer _buffer{8192}; //8KB 的缓冲区，用于读取数据
	http::request<http::dynamic_body> _request; //HTTP 请求对象
	http::response<http::dynamic_body> _response;//HTTP 响应对
	//_socket.get_executor(): 这是一个执行器，用于确定定时器所属的线程或线程池。_socket 
	// 应该是一个 net::socket 类型的成员变量，用于获取执行器
	//定时器的超时时间为 60 秒。这意味着从定时器创建开始计时，如果没有其他操作干预，60 秒后定时器将触发超时事件。
	net::steady_timer _deadline{ _socket.get_executor(),std::chrono::seconds(60)};// 超时定时器，设置为 60 秒
	//net::steady_timer _deadline(io_context, std::chrono::seconds(60));
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
};

