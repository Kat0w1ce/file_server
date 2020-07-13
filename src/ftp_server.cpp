#include "ftp_server.h"
#include<unistd.h>
#include <fstream>
#include <iostream>
#include<boost/format.hpp>
namespace ftpServer {

using namespace boost::asio;
using std::string;

using std::cout;
using std::endl;
using std::ifstream;
const string respHead = "HTTP/1.1 200 OK\r\n\r\n";
ftpServer::ftpServer(const string ip, const int port)
    : localhost(ip),
      Port(port),
      endpoint(ip::tcp::v4(), port),
      acceptor(nullptr) {}

void ftpServer::start() {
  io_service ios;
  acceptor.reset(new ip::tcp::acceptor(ios, endpoint));
}

int ftpServer::run() {
  io_service ios;

  while (true) {
    /* code */
    ifstream s("hello.html");
    ip::tcp::socket socket(ios);
    acceptor->accept(socket);
    string request;
    socket.read_some(buffer(request));
    cout<<request<<endl;
    string msg(respHead);
    string resp;
    resp += msg;
    while (getline(s, msg)) {
      msg+="\r\n";
      resp += msg;
    }
    socket.send(buffer(resp));  
    cout << resp << endl;
    sleep(3);
    s.close();
  }
  return 0;
}
}  // namespace ftpServer
