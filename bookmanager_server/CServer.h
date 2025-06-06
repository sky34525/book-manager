//
// Created by kw on 25-6-5.
//

#ifndef CSERVER_H
#define CSERVER_H
#include<const.h>

class CServer :public std::enable_shared_from_this<CServer>{
public:
    CServer(boost::asio::io_context&,unsigned short&);
    void start();
private:
    tcp::acceptor _acceptor;
    net::io_context& _ioc;
    tcp::socket _socket;
};



#endif //CSERVER_H
