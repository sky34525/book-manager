//
// Created by kw on 25-6-5.
//


#include "HttpConnection.h"
//char 转为16进制
char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

char FromHex(unsigned char x)
{
    char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp ;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //判断是否仅有数字和字母构成
        if (isalnum(static_cast<unsigned char>(str[i])) ||
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
            strTemp += ToHex(static_cast<unsigned char>(str[i]) >> 4);
            strTemp += ToHex(static_cast<unsigned char>(str[i]) & 0x0F);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //还原+为空
        if (str[i] == '+') strTemp += ' ';
        //遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex(static_cast<unsigned char>(str[++i]));
            unsigned char low = FromHex(static_cast<unsigned char>(str[++i]));
            strTemp += static_cast<char>(high * 16 + low);
        }
        else strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam()
{auto uri = request_.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        body_= uri;
        return;
    }
    body_= uri.substr(0, query_pos);
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
HttpConnection::HttpConnection(tcp::socket socket):socket_(std::move(socket)){}

void HttpConnection::start() {
    auto self = shared_from_this();
    http::async_read(socket_,buffer_,request_,[self](beast::error_code ec, std::size_t bytes_transferred) {
        try {
            if (ec) {
                std::cout<<"http read error"<<ec.what()<<std::endl;
                return;
            }
            self->HandleRequest();
            self->CheckDeadline();
        }
        catch (const std::exception& e) {
            std::cout<<e.what()<<std::endl;
        }
    });
}

void HttpConnection::HandleRequest() {
    try {
        std::cout << "HandleRequest called, method=" << request_.method_string() << ", target=" << request_.target() << std::endl;
        response_.version(request_.version());
        response_.keep_alive(request_.keep_alive());
        // 解析 path（去掉 query 部分）
        std::string path = std::string(request_.target());
        size_t pos = path.find('?');
        if (pos != std::string::npos) path = path.substr(0, pos);

        if (request_.method() == http::verb::options) {
            response_.result(http::status::ok);
            response_.set(http::field::access_control_allow_origin, "*");
            response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
            response_.set(http::field::access_control_allow_headers, "Content-Type");
            response_.set(http::field::access_control_max_age, "86400");
            response_.body() = "";
            WriteResponse();
            return;
        }

        if (request_.method() == http::verb::get) {
            PreParseGetParam();
            bool success = LogicSystem::GetInstance()->HandleGet(path, shared_from_this());
            if (!success) {
                response_.result(http::status::not_found);
                response_.set(http::field::content_type, "application/json; charset=utf-8");
                response_.body() = "url not found\r\n";
                WriteResponse();
                return;
            }
            response_.result(http::status::ok);
            response_.set(http::field::content_type, "application/json");
            response_.set(http::field::server, "bookmanager_server");
            response_.set(http::field::access_control_allow_origin, "*");
            WriteResponse();
            return;
        }
        if (request_.method() == http::verb::post) {
            bool success = LogicSystem::GetInstance()->HandlePost(path, shared_from_this());
            if (!success) {
                response_.result(http::status::not_found);
                response_.set(http::field::content_type, "application/json");
                response_.body() = "url not found\n";
                WriteResponse();
                return;
            }
            response_.result(http::status::ok);
            response_.set(http::field::content_type, "application/json");
            response_.set(http::field::server, "bookmanager_server");
            response_.set(http::field::access_control_allow_origin, "*");
            WriteResponse();
            return;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception in HandleRequest: " << e.what() << std::endl;
    }
}

void HttpConnection::WriteResponse()
{
    // 统一加 CORS 头
    response_.set(http::field::access_control_allow_origin, "*");
    response_.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    response_.set(http::field::access_control_allow_headers, "Content-Type");

    auto self=shared_from_this();
    response_.content_length(response_.body().size());
    http::async_write(socket_,response_,[self](beast::error_code ec,std::size_t)
    {
        self->socket_.shutdown(tcp::socket::shutdown_send,ec);
        self->deadline_.cancel();
    });
}

void HttpConnection::CheckDeadline()
{
    auto self=shared_from_this();

    deadline_.async_wait([self](beast::error_code ec)
    {
        if (!ec)
        {
            self->socket_.close(ec);
        }
    });
}

