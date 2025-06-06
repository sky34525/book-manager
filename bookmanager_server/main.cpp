#include"const.h"
#include"CServer.h"

int main() {
    unsigned short manager_port=18080;
    try {
        net::io_context ioc{1};
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc](boost::system::error_code ec,int signum) {
            if (ec) {
                std::cout<<"error"<<std::endl;
                return;
            }
            ioc.stop();
        });

        std::make_shared<CServer>(ioc,manager_port)->start();
        ioc.run();
    }
    catch (std::exception& e) {
        std::cerr<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }
}
