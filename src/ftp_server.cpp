#include "ftp_server.h"
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <boost/format.hpp>
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
      acceptor(nullptr),
      blocksize(1024) {}

void ftpServer::start() {
  io_service ios;
  acceptor.reset(new ip::tcp::acceptor(ios, endpoint));
}

int ftpServer::run() {
  io_service ios;

  while (true) {
    /* code */
    ifstream s("a.png",std::ios::ate|std::ios::binary);
   
    ip::tcp::socket socket(ios);
    acceptor->accept(socket);
    s.seekg(0,s.end);
    auto filesize=s.tellg();
    s.seekg(0,s.beg);
    char* buf=new char[1024];
    int blocks= filesize/blocksize;
    int left=filesize%blocksize;
    cout<<blocks<<" "<<left<<endl;;
    for(int i=0;i<blocks;i++){
      s.read(buf,blocksize);
      boost::asio::write(socket,buffer(buf,blocksize));
    }
    s.read(buf,left);
    
    socket.send(buffer(buf,left));
    
    cout<<"a"<<endl;
     sleep(3);
    socket.close();
    s.close();
    delete[] buf;
  }
  return 0;
}
}  // namespace ftpServer
