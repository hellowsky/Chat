#include "HttpConnection.h"
#include "LogicSystem.h"
HttpConnection::HttpConnection(tcp::socket socket): _socket(std::move(socket)){}

// ��ʼ���� HTTP ����
void HttpConnection::Start()
{
	// ��ȡ��ǰ����Ĺ���ָ��
	auto self(shared_from_this());
	// �첽��ȡ HTTP ����
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, 
		std::size_t bytes_transferred) {
			try
			{
				// ����д�����
				if (ec)
				{ 
					std::cout<<"http read err is "<<ec.what()<<std::endl;
					return;
				}
				// ���Դ�����ֽ���
				boost::ignore_unused(bytes_transferred);
				// ��������
				self->HandleRequest();
				// ��鳬ʱʱ��
				self->CheckDeadline();

			}
			catch (const std::exception& exp)
			{
				std::cout<<"exception is "<<exp.what()<<std::endl;
			}
		});
}

//��ʮ���Ƶ�charתΪ16����
unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}
//16����תΪʮ���Ƶ�char
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
	//����һ�����ַ��� strTemp ���ڴ洢�����Ľ��
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//�ж��Ƿ�������ֺ���ĸ����
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //Ϊ���ַ�
			strTemp += "+";
		else
		{
			//�����ַ���Ҫ��ǰ��%���Ҹ���λ�͵���λ�ֱ�תΪ16����
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
		//��ԭ+Ϊ��
		if (str[i] == '+') strTemp += ' ';
		//����%������������ַ���16����תΪchar��ƴ��
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
	// ��ȡ URI  
	auto uri = _request.target();
	// ���Ҳ�ѯ�ַ����Ŀ�ʼλ�ã��� '?' ��λ�ã�  
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
			key = UrlDecode(pair.substr(0, eq_pos)); // ������ url_decode ����������URL����  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// �������һ�������ԣ����û�� & �ָ�����  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}

// ��������������������Ӧ
void HttpConnection::HandleRequest()
{
	//���ð汾
	_response.version(_request.version());
	// ���������ӻ״̬ ���������ڷ�������Ӧ��ر��� 
	_response.keep_alive(false); 
	// ������󷽷�Ϊ GET
	if (_request.method() == http::verb::get)
	{
		//����get
		PreParseGetParam();

		// ���Դ��� GET ����
		bool success = LogicSystem::GetInstance()->HandleGet(_get_url, shared_from_this());
		if (!success)
		{
			// �������ʧ�ܣ��򷵻ء�δ�ҵ���״̬
			_response.result(http::status::not_found);
			// ������Ӧ��������
			_response.set(http::field::content_type, "text/plain");
			// ������Ӧ��
			beast::ostream(_response.body()) << "url not found\r\n";
			// д����Ӧ
			WriteResponse();
			return;
		}
		// �������ɹ����򷵻ء�OK��״̬
		_response.result(http::status::ok);
		// ���÷������ֶ� set��������������Ӧͷ�е��ֶ�     http::field::server ��һ��Ԥ�����ö��ֵ����ʾ Server ��Ӧͷ�ֶ� 
		//"GateServer" ��һ���ַ�������ʾ���������������
		_response.set(http::field::server, "GateServer");
		// д����Ӧ
		WriteResponse();
		return;
	}


	if (_request.method() == http::verb::post)
	{
		//����get
		// ���Դ��� GET ����
		bool success = LogicSystem::GetInstance()->HandlePost(_request.target(), shared_from_this());
		if (!success)
		{
			// �������ʧ�ܣ��򷵻ء�δ�ҵ���״̬
			_response.result(http::status::not_found);
			// ������Ӧ��������
			_response.set(http::field::content_type, "text/plain");
			// ������Ӧ��
			beast::ostream(_response.body()) << "url not found\r\n";
			// д����Ӧ
			WriteResponse();
			return;
		}
		// �������ɹ����򷵻ء�OK��״̬
		_response.result(http::status::ok);
		// ���÷������ֶ� set��������������Ӧͷ�е��ֶ�     http::field::server ��һ��Ԥ�����ö��ֵ����ʾ Server ��Ӧͷ�ֶ� 
		//"GateServer" ��һ���ַ�������ʾ���������������
		_response.set(http::field::server, "GateServer");
		// д����Ӧ
		WriteResponse();
		return;
	}
}


// д�� HTTP ��Ӧ
void HttpConnection::WriteResponse()
{
	// ��ȡ��ǰ����Ĺ���ָ��
	auto self(shared_from_this());
	// ������Ӧ�峤��
	_response.content_length(_response.body().size());
	// �첽д�� HTTP ��Ӧ
	http::async_write(_socket, _response, [self](beast::error_code ec, std::size_t bytes_transferred) {
		// �ر��׽��ֵķ��Ͷ� ��ζ�Ų����������ݴӷ��������͵��ͻ�
		self->_socket.shutdown(tcp::socket::shutdown_send, ec);
		// ȡ����ʱ��ʱ�� һ����Ӧ�����ͣ���û�б�Ҫ�ٱ�����ʱ����
		self->_deadline.cancel();
		});
}
// ��鳬ʱʱ���Ƿ񵽴�
void HttpConnection::CheckDeadline()
{
	// ��ȡ��ǰ����Ĺ���ָ��
	auto self(shared_from_this());
	// �첽�ȴ���ʱʱ�� ������ecֻ���ڶ�ʱ������ʱ����һ��   
	_deadline.async_wait([self](beast::error_code ec){
		if (!ec)
		{
			// ���û�д�����ر��׽���
			self->_socket.close(ec);
		}
		});
}

