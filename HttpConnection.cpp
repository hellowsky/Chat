#include "HttpConnection.h"
#include "LogicSystem.h"
HttpConnection::HttpConnection(tcp::socket socket): _socket(std::move(socket)){}

// 开始处理 HTTP 请求
void HttpConnection::Start()
{
	// 获取当前对象的共享指针
	auto self(shared_from_this());
	// 异步读取 HTTP 请求
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, 
		std::size_t bytes_transferred) {
			try
			{
				// 如果有错误发生
				if (ec)
				{ 
					std::cout<<"http read err is "<<ec.what()<<std::endl;
					return;
				}
				// 忽略传输的字节数
				boost::ignore_unused(bytes_transferred);
				// 处理请求
				self->HandleRequest();
				// 检查超时时间
				self->CheckDeadline();

			}
			catch (const std::exception& exp)
			{
				std::cout<<"exception is "<<exp.what()<<std::endl;
			}
		});
}

//将十进制的char转为16进制
unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}
//16进制转为十进制的char
unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	//创建一个空字符串 strTemp 用于存储编码后的结果
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//判断是否仅有数字和字母构成
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //为空字符
			strTemp += "+";
		else
		{
			//其他字符需要提前加%并且高四位和低四位分别转为16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//还原+为空
		if (str[i] == '+') strTemp += ' ';
		//遇到%将后面的两个字符从16进制转为char再拼接
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}
//https://www.example.com/path/to/resource?param1=value1&param2=value2#section1
//https://www.example.com/search?q=query%20with%20spaces%20and%20special%20characters%2C%20like%20%26%2339%3B%20and%20%25
void HttpConnection::PreParseGetParam() {
	// 提取 URI  
	auto uri = _request.target();
	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		_get_url = uri;
		return;
	}

	_get_url = uri.substr(0, query_pos);
	std::string query_string = uri.substr(query_pos + 1);
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}

// 处理解析后的请求并生成响应
void HttpConnection::HandleRequest()
{
	//设置版本
	_response.version(_request.version());
	// 不保持连接活动状态 服务器将在发送完响应后关闭连 
	_response.keep_alive(false); 
	// 如果请求方法为 GET
	if (_request.method() == http::verb::get)
	{
		//处理get
		PreParseGetParam();

		// 尝试处理 GET 请求
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
		if (!success)
		{
			// 如果处理失败，则返回“未找到”状态
			_response.result(http::status::not_found);
			// 设置响应内容类型
			_response.set(http::field::content_type, "text/plain");
			// 设置响应体
			beast::ostream(_response.body()) << "url not found\r\n";
			// 写入响应
			WriteResponse();
			return;
		}
		// 如果处理成功，则返回“OK”状态
		_response.result(http::status::ok);
		// 设置服务器字段 set方法用于设置响应头中的字段     http::field::server 是一个预定义的枚举值，表示 Server 响应头字段 
		//"GateServer" 是一个字符串，表示服务器软件的名称
		_response.set(http::field::server, "GateServer");
		// 写入响应
		WriteResponse();
		return;
	}


	if (_request.method() == http::verb::post)
	{
		//处理get
		// 尝试处理 GET 请求
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
		if (!success)
		{
			// 如果处理失败，则返回“未找到”状态
			_response.result(http::status::not_found);
			// 设置响应内容类型
			_response.set(http::field::content_type, "text/plain");
			// 设置响应体
			beast::ostream(_response.body()) << "url not found\r\n";
			// 写入响应
			WriteResponse();
			return;
		}
		// 如果处理成功，则返回“OK”状态
		_response.result(http::status::ok);
		// 设置服务器字段 set方法用于设置响应头中的字段     http::field::server 是一个预定义的枚举值，表示 Server 响应头字段 
		//"GateServer" 是一个字符串，表示服务器软件的名称
		_response.set(http::field::server, "GateServer");
		// 写入响应
		WriteResponse();
		return;
	}
}


// 写入 HTTP 响应
void HttpConnection::WriteResponse()
{
	// 获取当前对象的共享指针
	auto self(shared_from_this());
	// 设置响应体长度
	_response.content_length(_response.body().size());
	// 异步写入 HTTP 响应
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) {
		// 关闭套接字的发送端 意味着不会再有数据从服务器发送到客户
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);
		// 取消超时定时器 一旦响应被发送，就没有必要再保留定时器了
		self->_deadline.cancel();
		});
}
// 检查超时时间是否到达
void HttpConnection::CheckDeadline()
{
	// 获取当前对象的共享指针
	auto self(shared_from_this());
	// 异步等待超时时间 错误码ec只会在定时器到期时生成一次   
	_deadline.async_wait([self](beast::error_code ec){
		if (!ec)
		{
			// 如果没有错误，则关闭套接字
			self->_socket.close(ec);
		}
		});
}

