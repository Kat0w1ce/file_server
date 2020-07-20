#include "ftp_server.h"

#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include "util.h"
extern fileLogger logger("a.log");
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

void ftpServer::init() {}

int ftpServer::file_handler() {
    io_service ios;

    while (true) {
        /* code */
        pSocket socket =
            std::make_shared<ip::tcp::socket>(ip::tcp::socket(ios));
        boost::asio::ip::tcp::no_delay option(true);
        socket->set_option(option);
        acceptor->accept(*socket);
        std::string cmd_buf(64, '\0');
        if (check(socket)) {
            cout << "correct" << endl;
            cmd_buf[0] = '1';
            socket->send(buffer(cmd_buf));
        } else {
            cmd_buf[0] = '0';
            socket->send(buffer(cmd_buf));
            socket->close();
            continue;
        }

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

            boost::asio::post(thread_pool,
                              std::bind(getfile, socket, fn,
                                        std::filesystem::current_path()));
        } else if (op == "3") {
            string fn;
            is >> fn;
            boost::asio::post(thread_pool, std::bind(senddir, socket, fn));
        } else if (op == "4") {
            string fn;
            is >> fn;

            boost::asio::post(thread_pool,
                              std::bind(recvdir, socket, std::string()));
        }
    }
    return 0;
}
bool ftpServer::check(pSocket socket) {
    string msg(64, '\0');
    string tmp;
    socket->read_some(buffer(msg));
    std::istringstream is(msg);
    std::string _usrname, _pwd;
    std::string ss;
    is >> _usrname >> _pwd >> ss;
    std::ifstream s("user.conf", std::ios::in);
    while (std::getline(s, tmp)) {
        /* code */
        std::istringstream iss(tmp);
        string usrname;
        string pwd;
        iss >> usrname >> pwd;
        if (_usrname == usrname && _pwd == pwd) {
            logger(Level::Info) << "User " << _usrname << " log in.";
            return true;
        }
    }
    return false;
}
}  // namespace ftpServer
