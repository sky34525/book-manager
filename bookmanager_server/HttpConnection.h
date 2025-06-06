//
// Created by kw on 25-6-5.
//

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include"const.h"
#include"LogicSystem.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>{
    friend class LogicSystem;
public:
    explicit HttpConnection(tcp::socket socket);
    void start();
    http::request<http::string_body> request_;
    http::response<http::string_body> response_;
    std::unordered_map<std::string, std::string> _get_params;
private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{32*1024};
    net::steady_timer deadline_{socket_.get_executor(),std::chrono::seconds(60)};
    std::string body_;
    void CheckDeadline();
    void WriteResponse();
    void HandleRequest();
    void PreParseGetParam();
};



#endif //HTTPCONNECTION_H
