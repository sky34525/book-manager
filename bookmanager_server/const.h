//
// Created by kw on 25-6-5.
//

#ifndef CONST_H
#define CONST_H
#include <mysql/jdbc.h>
#include <iostream>
#include<version>
#include<memory>
#include<map>
#include<boost/asio.hpp>
#include<string>
#include<boost/beast.hpp>
#include<boost/asio/ip/tcp.hpp>
#include<boost/beast/http.hpp>
#include <unordered_map>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <ctime>
#include"Singleton.h"
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,  //Json解析错误
    RPCFailed = 1002,  //RPC请求错误
    VarifyExpired = 1003, //验证码过期
    VarifyCodeErr = 1004, //验证码错误
    UserExist = 1005,       //用户已经存在
    PasswdErr = 1006,    //密码错误
    EmailNotMatch = 1007,  //邮箱不匹配
    PasswdUpFailed = 1008,  //更新密码失败
    PasswdInvalid = 1009,   //密码更新失败
    TokenInvalid = 1010,   //Token失效
    UidInvalid = 1011,  //uid无效
};

#endif //CONST_H
