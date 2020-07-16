#include "ftp_server.h"

#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "util.h"

namespace ftpServer {

using namespace boost::asio;
using std::string;

using std::cout;
using std::endl;
using std::ifstream;

ftpServer::ftpServer(const string ip, const int port)
    : localhost(ip),
      Port(port),
      endpoint(ip::tcp::v4(), port),
      acceptor(nullptr),
      blocksize(1024),
      thread_pool(4) {}

void ftpServer::start() {
    io_service ios;
    acceptor.reset(new ip::tcp::acceptor(ios, endpoint));
    init();
}
void ftpServer::send_dir(const string dirname) {}
void ftpServer::parser() {}

void ftpServer::init() {
    boost::log::add_file_log("sample.log");
    boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                        boost::log::trivial::trace);
}

int ftpServer::file_handler() {
    io_service ios;

    while (true) {
        /* code */
        pSocket socket =
            std::make_shared<ip::tcp::socket>(ip::tcp::socket(ios));
        acceptor->accept(*socket);
        std::string cmd_buf(64, '\0');
        socket->read_some(buffer(cmd_buf));
        std::istringstream is(cmd_buf);
        string op;
        is >> op;
        if (op == "1") {
            string fn;
            is >> fn;
            // BOOST_LOG_TRIVIAL(fatal) << "sendfile";
            boost::asio::post(thread_pool, std::bind(send_file, socket, fn));
        } else if (op == "2") {
            string fn;
            is >> fn;
            cout << fn << endl;
            // BOOST_LOG_TRIVIAL(fatal) << "recvfile";
            boost::asio::post(thread_pool,
                              std::bind(getfile, socket, fn,
                                        std::filesystem::current_path()));
        } else if (op == "3") {
            string fn;
            is >> fn;
            // BOOST_LOG_TRIVIAL(fatal) << "recvfile";
            boost::asio::post(thread_pool, std::bind(senddir, socket, fn));
        }
    }
    return 0;
}
}  // namespace ftpServer
