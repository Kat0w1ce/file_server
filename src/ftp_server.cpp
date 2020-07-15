#include "ftp_server.h"

#include <unistd.h>

#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
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

void ftpServer::parser() {}
void getfile(pSocket socket, const std::string& filepath) {
    std::ofstream out(filepath, std::iostream::out);
    if (!out.is_open()) {
        std::cout << "open file error" << std::endl;
        return;
    }
    std::string buf(1024, '\0');

    while (true) {
        auto cnt = socket->read_some(buffer(buf));
        if (cnt == 1024) out << buf;
        if (cnt != 1024) {
            for (auto i = 0; i < cnt; ++i) out << buf[i];
            break;
        }
    }
    socket->close();
    out.close();
}
void ftpServer::init() {
    boost::log::add_file_log("sample.log");
    boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                        boost::log::trivial::trace);
}
void sendfile(pSocket socket, const std::string& filepath) {
    char tmp[100];
    realpath(filepath.c_str(), tmp);
    ifstream s(tmp);
    if (!s.is_open()) {
        cout << "can't open " << tmp << endl;
        return;
    }
    s.seekg(0, s.end);
    auto filesize = s.tellg();
    s.seekg(0, s.beg);
    char* buf = new char[1024];
    int blocks = filesize / blocksize;
    int left = filesize % blocksize;
    cout << blocks << " " << left << endl;
    ;
    for (int i = 0; i < blocks; i++) {
        s.read(buf, blocksize);
        socket->send(buffer(buf, blocksize));
    }
    s.read(buf, left);
    socket->send(buffer(buf, left));

    sleep(3);
    socket->close();
    s.close();
    BOOST_LOG_TRIVIAL(fatal) << "sendfile";
    delete[] buf;
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
            boost::asio::post(thread_pool, std::bind(sendfile, socket, fn));
        } else if (op == "2") {
            string fn;
            is >> fn;
            cout << fn << endl;
            fn = string("copy_") + fn;
            boost::asio::post(thread_pool, std::bind(getfile, socket, fn));
        }
    }
    return 0;
}
}  // namespace ftpServer
