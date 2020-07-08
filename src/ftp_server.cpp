#include "ftp_server.h"
#include <iostream>
namespace ftpServer {

using namespace boost::asio;
using std::string;

using std::cout;
using std::endl;
ftpServer::ftpServer(const string ip, const int port)
    : localhost(ip),
      Port(port),
      endpoint(ip::tcp::v4(), port),
      acceptor(nullptr) {}

void ftpServer::start() {
  io_service ios;
  acceptor.reset(new ip::tcp::acceptor(ios, endpoint));
  //   std::make_shared<ip::tcp::acceptor>(ip::tcp::acceptor(ios, endpoint));
}

int ftpServer::run() {
  io_service ios;
  while (true) {
    /* code */
    ip::tcp::socket socket(ios);
    acceptor->accept(socket);
    cout << "ip: " << socket.remote_endpoint().address() << endl;
    // break;
    // cout<<socket.
    string msg;
    socket.read_some(buffer(msg));
    cout << msg << endl;
  }
  return 0;
}
}  // namespace ftpServer
