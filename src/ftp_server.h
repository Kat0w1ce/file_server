#pragma once
#include <boost/asio.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <memory>
#include <string>
namespace ftpServer {
using pSocket = std::shared_ptr<boost::asio::ip::tcp::socket>;
const int blocksize = 1024;
class ftpServer {
   public:
    void start();
    int file_handler();

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
    void init();
};
}  // namespace ftpServer