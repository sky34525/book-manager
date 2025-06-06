//
// Created by kw on 25-6-5.
//

#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include "const.h"
#include <mysql/jdbc.h>

struct User {
    std::string username;
    std::string password;
    std::string role; // "admin" or "reader"
};

struct Book {
    int id;
    std::string title;
    std::string author;
    int stock;
};

struct Borrow {
    std::string username;
    int bookId;
    std::string borrowTime;
    std::string returnTime; // 为空表示未归还
};

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem :public Singleton<LogicSystem>{
    friend class Singleton<LogicSystem>;
public:
    bool HandleGet(const std::string&,std::shared_ptr<HttpConnection>);
    void RegGet(std::string,HttpHandler handler);
    void RegPost(std::string,HttpHandler);
    bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);
    // handler 声明
    void HandleBooks(std::shared_ptr<HttpConnection> con);
    void HandleAddBook(std::shared_ptr<HttpConnection> con);
    void HandleUpdateBook(std::shared_ptr<HttpConnection> con);
    void HandleDeleteBook(std::shared_ptr<HttpConnection> con);
    void HandleMyBooks(std::shared_ptr<HttpConnection> con);
    void HandleHistory(std::shared_ptr<HttpConnection> con);
    void HandleBorrow(std::shared_ptr<HttpConnection> con);
    void HandleReturn(std::shared_ptr<HttpConnection> con);
    void HandleLogin(std::shared_ptr<HttpConnection> con);
    void HandleRegister(std::shared_ptr<HttpConnection> con);
private:
    LogicSystem();
    std::map<std::string,HttpHandler> _post_handlers;
    std::map<std::string,HttpHandler>_get_handlers;
    // 数据存储
    std::vector<User> users_;
    std::vector<Book> books_;
    std::vector<Borrow> borrows_;
    int nextBookId_ = 1;
    std::unique_ptr<sql::Connection> db_;
};

#endif //LOGICSYSTEM_H
