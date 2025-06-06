//
// Created by kw on 25-6-5.
//

#include "CServer.h"
#include"HttpConnection.h"
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):
_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc)
{}

void CServer::start() {
    auto self = shared_from_this();
    _acceptor.async_accept(_socket, [self](boost::system::error_code ec) {
        try {
            if (ec) {
                self->start();
                return;
            }
            std::make_shared<HttpConnection>(std::move(self->_socket))->start();
            self->start();
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            self->start();
        }
    });
}