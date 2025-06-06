//
// Created by kw on 25-6-5.
//

#include "LogicSystem.h"
#include <json/json.h>
#include <ctime>
#include <algorithm>
#include "HttpConnection.h"
#include <mysql/jdbc.h>
#include <memory>
#include <iostream>

// 工具函数：获取当前时间字符串
static std::string nowTime() {
    time_t t = time(nullptr);
    char buf[32];
    // MySQL DATETIME 格式：YYYY-MM-DD HH:MM:SS
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buf;
}

LogicSystem::LogicSystem() {
    // 注册路由
    RegGet("/api/books", [this](std::shared_ptr<HttpConnection> con) { this->HandleBooks(con); });
    RegPost("/api/books/add", [this](std::shared_ptr<HttpConnection> con) { this->HandleAddBook(con); });
    RegPost("/api/books/update", [this](std::shared_ptr<HttpConnection> con) { this->HandleUpdateBook(con); });
    RegPost("/api/books/delete", [this](std::shared_ptr<HttpConnection> con) { this->HandleDeleteBook(con); });
    RegGet("/api/mybooks", [this](std::shared_ptr<HttpConnection> con) { this->HandleMyBooks(con); });
    RegGet("/api/history", [this](std::shared_ptr<HttpConnection> con) { this->HandleHistory(con); });
    RegPost("/api/borrow", [this](std::shared_ptr<HttpConnection> con) { this->HandleBorrow(con); });
    RegPost("/api/return", [this](std::shared_ptr<HttpConnection> con) { this->HandleReturn(con); });
    RegPost("/api/login", [this](std::shared_ptr<HttpConnection> con) { this->HandleLogin(con); });
    RegPost("/api/register", [this](std::shared_ptr<HttpConnection> con) { this->HandleRegister(con); });
    // 默认添加一个管理员
    users_.push_back({"admin", "admin", "admin"});

    try {
        // 获取驱动
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        db_ = std::unique_ptr<sql::Connection>(
            driver->connect("tcp://127.0.0.1:3308", "root", "123456")
        );
        db_->setSchema("book_manager");

        // 测试查询
        std::unique_ptr<sql::Statement> stmt(db_->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT VERSION()"));
        while (res->next()) {
            std::cout << "MySQL version: " << res->getString(1) << std::endl;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
    }
}

void LogicSystem::HandleBooks(std::shared_ptr<HttpConnection> con) {
    std::string q;
    if (con->_get_params.count("q")) q = con->_get_params["q"];
    Json::Value arr(Json::arrayValue);
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt;
        if (q.empty()) {
            pstmt.reset(db_->prepareStatement("SELECT id, title, author, stock FROM books"));
        } else {
            pstmt.reset(db_->prepareStatement("SELECT id, title, author, stock FROM books WHERE title LIKE ? OR author LIKE ?"));
            std::string pattern = "%" + q + "%";
            pstmt->setString(1, pattern);
            pstmt->setString(2, pattern);
        }
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            Json::Value obj;
            obj["id"] = res->getInt("id");
            obj["title"] = res->getString("title").c_str();
            obj["author"] = res->getString("author").c_str();
            obj["stock"] = res->getInt("stock");
            arr.append(obj);
        }
    } catch (sql::SQLException& e) {
        // 错误处理
    }
    Json::FastWriter writer;
    con->response_.set(http::field::content_type, "application/json; charset=utf-8");
    con->response_.body() = writer.write(arr);
}

void LogicSystem::HandleAddBook(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("INSERT INTO books(title, author, stock) VALUES (?, ?, ?)")
        );
        pstmt->setString(1, req.get("title", "").asString());
        pstmt->setString(2, req.get("author", "").asString());
        pstmt->setInt(3, req.get("stock", 1).asInt());
        pstmt->execute();
        con->response_.body() = "{\"success\":true}";
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleUpdateBook(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    int id = req.get("id", 0).asInt();
    std::string title = req.get("title", "").asString();
    std::string author = req.get("author", "").asString();
    int stock = req.get("stock", 1).asInt();
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("UPDATE books SET title=?, author=?, stock=? WHERE id=?")
        );
        pstmt->setString(1, title);
        pstmt->setString(2, author);
        pstmt->setInt(3, stock);
        pstmt->setInt(4, id);
        int affected = pstmt->executeUpdate();
        if (affected > 0) {
            con->response_.body() = "{\"success\":true}";
        } else {
            con->response_.result(http::status::not_found);
            con->response_.body() = "{\"error\":\"book not found\"}";
        }
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleDeleteBook(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    int id = req.get("id", 0).asInt();
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("DELETE FROM books WHERE id=?")
        );
        pstmt->setInt(1, id);
        int affected = pstmt->executeUpdate();
        if (affected > 0) {
            con->response_.body() = "{\"success\":true}";
        } else {
            con->response_.result(http::status::not_found);
            con->response_.body() = "{\"error\":\"book not found\"}";
        }
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleMyBooks(std::shared_ptr<HttpConnection> con) {
    std::string user;
    if (con->_get_params.count("user")) user = con->_get_params["user"];
    Json::Value arr(Json::arrayValue);
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement(
                "SELECT b.id, b.title, br.borrowTime "
                "FROM borrows br JOIN books b ON br.bookId = b.id "
                "WHERE br.username=? AND br.returnTime IS NULL"
            )
        );
        pstmt->setString(1, user);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            Json::Value obj;
            obj["bookId"] = res->getInt("id");
            obj["bookTitle"] = res->getString("title").c_str();
            obj["borrowTime"] = res->getString("borrowTime").c_str();
            arr.append(obj);
        }
    } catch (sql::SQLException& e) {
        // 错误处理
    }
    Json::FastWriter writer;
    con->response_.set(http::field::content_type, "application/json; charset=utf-8");
    con->response_.body() = writer.write(arr);
}

void LogicSystem::HandleHistory(std::shared_ptr<HttpConnection> con) {
    std::string user;
    if (con->_get_params.count("user")) user = con->_get_params["user"];
    Json::Value arr(Json::arrayValue);
    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement(
                "SELECT b.title, br.borrowTime, br.returnTime "
                "FROM borrows br JOIN books b ON br.bookId = b.id "
                "WHERE br.username=? AND br.returnTime IS NOT NULL"
            )
        );
        pstmt->setString(1, user);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        while (res->next()) {
            Json::Value obj;
            obj["bookTitle"] = res->getString("title").c_str();
            obj["borrowTime"] = res->getString("borrowTime").c_str();
            obj["returnTime"] = res->getString("returnTime").c_str();
            arr.append(obj);
        }
    } catch (sql::SQLException& e) {
        // 错误处理
    }
    Json::FastWriter writer;
    con->response_.set(http::field::content_type, "application/json; charset=utf-8");
    con->response_.body() = writer.write(arr);
}

void LogicSystem::HandleBorrow(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    int bookId = req.get("bookId", 0).asInt();
    std::string token = req.get("token", "").asString();
    // 这里假设前端传了用户名
    std::string username = req.get("username", "").asString();
    std::cout << "[借阅] username=" << username << ", bookId=" << bookId << std::endl;
    try {
        // 检查库存
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("SELECT stock FROM books WHERE id=?")
        );
        pstmt->setInt(1, bookId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (!res->next() || res->getInt("stock") <= 0) {
            con->response_.result(http::status::bad_request);
            con->response_.body() = "{\"error\":\"book not found or out of stock\"}";
            return;
        }

        // 检查当前借阅数
        pstmt.reset(db_->prepareStatement("SELECT COUNT(*) FROM borrows WHERE username=? AND returnTime IS NULL"));
        pstmt->setString(1, username);
        res.reset(pstmt->executeQuery());
        res->next();
        if (res->getInt(1) >= 5) {
            con->response_.result(http::status::bad_request);
            con->response_.body() = "{\"error\":\"已达最大借书数\"}";
            return;
        }

        // 借书：插入borrows，更新books
        pstmt.reset(db_->prepareStatement("INSERT INTO borrows(username, bookId, borrowTime) VALUES (?, ?, ?)"));
        pstmt->setString(1, username);
        pstmt->setInt(2, bookId);
        pstmt->setString(3, nowTime());
        pstmt->execute();

        pstmt.reset(db_->prepareStatement("UPDATE books SET stock=stock-1 WHERE id=?"));
        pstmt->setInt(1, bookId);
        pstmt->execute();

        con->response_.body() = "{\"success\":true}";
    } catch (sql::SQLException& e) {
        std::cerr << "[借阅SQL错误] " << e.what() << std::endl;
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleReturn(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    int bookId = req.get("bookId", 0).asInt();
    std::string username = req.get("user", "").asString();

    try {
        // 查找未归还的借阅记录
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("SELECT id FROM borrows WHERE username=? AND bookId=? AND returnTime IS NULL")
        );
        pstmt->setString(1, username);
        pstmt->setInt(2, bookId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (!res->next()) {
            con->response_.result(http::status::not_found);
            con->response_.body() = "{\"error\":\"borrow record not found\"}";
            return;
        }
        int borrowId = res->getInt("id");

        // 更新归还时间
        pstmt.reset(db_->prepareStatement("UPDATE borrows SET returnTime=? WHERE id=?"));
        pstmt->setString(1, nowTime());
        pstmt->setInt(2, borrowId);
        pstmt->execute();

        // 库存+1
        pstmt.reset(db_->prepareStatement("UPDATE books SET stock=stock+1 WHERE id=?"));
        pstmt->setInt(1, bookId);
        pstmt->execute();

        con->response_.body() = "{\"success\":true}";
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleLogin(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    std::string username = req.get("username", "").asString();
    std::string password = req.get("password", "").asString();

    try {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("SELECT role FROM users WHERE username=? AND password=?")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            Json::Value resp;
            resp["token"] = "mock-token";
            resp["username"] = username;
            resp["role"] = res->getString("role").c_str();
            Json::FastWriter writer;
            con->response_.body() = writer.write(resp);
        } else {
            con->response_.result(http::status::unauthorized);
            con->response_.body() = "{\"error\":\"invalid username or password\"}";
        }
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::HandleRegister(std::shared_ptr<HttpConnection> con) {
    Json::Value req;
    Json::Reader reader;
    if (!reader.parse(con->request_.body(), req)) {
        con->response_.result(http::status::bad_request);
        con->response_.body() = "{\"error\":\"json parse error\"}";
        return;
    }
    std::string username = req.get("username", "").asString();
    std::string password = req.get("password", "").asString();

    try {
        // 检查用户是否存在
        std::unique_ptr<sql::PreparedStatement> pstmt(
            db_->prepareStatement("SELECT COUNT(*) FROM users WHERE username=?")
        );
        pstmt->setString(1, username);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        res->next();
        if (res->getInt(1) > 0) {
            con->response_.result(http::status::bad_request);
            con->response_.body() = "{\"error\":\"user exists\"}";
            return;
        }
        // 插入新用户
        pstmt.reset(db_->prepareStatement("INSERT INTO users(username, password, role) VALUES (?, ?, 'reader')"));
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        pstmt->execute();
        con->response_.body() = "{\"success\":true}";
    } catch (sql::SQLException& e) {
        con->response_.result(http::status::internal_server_error);
        con->response_.body() = "{\"error\":\"db error\"}";
    }
}

void LogicSystem::RegGet(std::string path, HttpHandler handler) {
    _get_handlers[path] = handler;
}

void LogicSystem::RegPost(std::string path, HttpHandler handler) {
    _post_handlers[path] = handler;
}

bool LogicSystem::HandleGet(const std::string& path, std::shared_ptr<HttpConnection> con) {
    auto it = _get_handlers.find(path);
    if (it != _get_handlers.end()) {
        it->second(con);
        return true;
    }
    return false;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con) {
    auto it = _post_handlers.find(path);
    if (it != _post_handlers.end()) {
        it->second(con);
        return true;
    }
    return false;
}

void connect_mysql_example() {
    try {
        // 获取驱动
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        // 创建连接（替换用户名、密码、数据库名）
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://127.0.0.1:3308", "root", "123456")
        );
        con->setSchema("book_manager");

        // 测试查询
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT VERSION()"));
        while (res->next()) {
            std::cout << "MySQL version: " << res->getString(1) << std::endl;
        }
    } catch (sql::SQLException& e) {
        std::cerr << "MySQL error: " << e.what() << std::endl;
    }
}
