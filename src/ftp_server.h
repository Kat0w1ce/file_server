#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>

#include "logger.h"

namespace ftpServer {
using pSocket = std::shared_ptr<boost::asio::ip::tcp::socket>;
const int blocksize = 1024;
class ftpServer {
   public:
    void start();
    int file_handler();
    void send_dir(const std::string dirname);
    ftpServer(const std::string ip, const int port);
    ~ftpServer() { acceptor.release(); }
    void run();
    void parser();

   private:
    const std::string localhost;
    const int Port;
    const size_t blocksize;
    boost::asio::thread_pool thread_pool;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::asio::ip::tcp::endpoint endpoint;
    // fileLogger logger;
    void init();
};
}  // namespace ftpServer