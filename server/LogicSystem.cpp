#include "LogicSystem.h"
#include"HttpConnection.h"


// 注册 GET 请求处理器
void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
    // 插入 GET 请求处理器
    _get_handlers.insert({ url, handler });
}

// 注册 POST 请求处理器
void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    // 插入 GET 请求处理器
    _post_handlers.insert({ url, handler });
     
}

// 构造函数
LogicSystem::LogicSystem()
{
    // 注册一个示例 GET 请求处理器
    //当收到 /get_test 的 GET 请求时，将会调用这个 lambda 函数，
    //向 connection 的响应体中写入 "receive get_test req" 字符串
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        // 设置响应体
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
            //key不存在 
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body()) << jsonstr;
            return true;
        } 
         //key存在
         auto email = src_root["email"].asString();
         std::cout << "email is " << email << std::endl;
         root["error"] = 0;
         root["email"] = src_root["email"];
         std::string jsonstr = root.toStyledString();
         beast::ostream(connection->_response.body()) << jsonstr;
         return true;
        });

}

// 处理 GET 请求
bool LogicSystem::HandleGet(std::string url, std::shared_ptr<HttpConnection> conn)
{
    // 查找 GET 请求处理器
    if (_get_handlers.find(url) == _get_handlers.end())
    {
        // 如果找不到对应的处理器，则返回 false
        return false;
    }

    // 执行对应的 GET 请求处理器
    _get_handlers[url](conn);

    // 成功处理请求
    return true;
}

bool LogicSystem::HandlePost(std::string url, std::shared_ptr<HttpConnection> conn)
{
    if (_post_handlers.find(url) == _post_handlers.end())
    {
        // 如果找不到对应的处理器，则返回 false
        return false;
    }
    _post_handlers[url](conn);
    return true;

}


