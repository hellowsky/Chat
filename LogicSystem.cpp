#include "LogicSystem.h"
#include"HttpConnection.h"


// ע�� GET ��������
void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
    // ���� GET ��������
    _get_handlers.insert({ url, handler });
}

// ע�� POST ��������
void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    // ���� GET ��������
    _post_handlers.insert({ url, handler });
     
}

// ���캯��
LogicSystem::LogicSystem()
{
    // ע��һ��ʾ�� GET ��������
    //���յ� /get_test �� GET ����ʱ������������ lambda ������
    //�� connection ����Ӧ����д�� "receive get_test req" �ַ���
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        // ������Ӧ��
        beast::ostream(connection->_response.body()) << "receive get_test reqsssssssss";
        int i = 0;
        for (auto& elem : connection->_get_params)
        {
            i++;
            beast::ostream(connection->_response.body()) << " param: " << i << " key is : " << elem.first;
            beast::ostream(connection->_response.body()) << " param: " << i << " value is : " << elem.second << std::endl;
        }
    });

    RegPost("/post_varifycode", [](std::shared_ptr<HttpConnection> connection) {
        auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
        std::cout<<"receive body is"<<body_str<<std::endl;
        connection->_response.set(http::field::content_type, "text/json");
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success)
        {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        }

        if (!src_root.isMember("email"))
        {
            //key������ 
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        } 
         //key����
         auto email = src_root["email"].asString();
         std::cout << "email is " << email << std::endl;
         root["error"] = 0;
         root["email"] = src_root["email"];
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
        });

}

// ���� GET ����
bool LogicSystem::HandleGet(std::string url, std::shared_ptr<HttpConnection> conn)
{
    // ���� GET ��������
    if (_get_handlers.find(url) == _get_handlers.end())
    {
        // ����Ҳ�����Ӧ�Ĵ��������򷵻� false
        return false;
    }

    // ִ�ж�Ӧ�� GET ��������
    _get_handlers[url](conn);

    // �ɹ���������
    return true;
}

bool LogicSystem::HandlePost(std::string url, std::shared_ptr<HttpConnection> conn)
{
    if (_post_handlers.find(url) == _post_handlers.end())
    {
        // ����Ҳ�����Ӧ�Ĵ��������򷵻� false
        return false;
    }
    _post_handlers[url](conn);
    return true;

}


