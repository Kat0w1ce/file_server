#include "ftp_server.h"
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <boost/format.hpp>
#include <memory>
namespace ftpServer {

using namespace boost::asio;
using std::string;

using std::cout;
using std::endl;
using std::ifstream;


ftpServer::ftpServer(const string ip, const int port): localhost(ip),
      Port(port),
      endpoint(ip::tcp::v4(), port),
      acceptor(nullptr),
      blocksize(1024),
      thread_pool(4) {}

void ftpServer::start() {
  io_service ios;
  acceptor.reset(new ip::tcp::acceptor(ios, endpoint));
}

void sendfile(pSocket socket, const std::string& filepath){
    ifstream s(filepath);
    s.seekg(0,s.end);
    auto filesize=s.tellg();
    s.seekg(0,s.beg);
    char* buf=new char[1024];
    int blocks= filesize/blocksize;
    int left=filesize%blocksize;
    cout<<blocks<<" "<<left<<endl;;
    for(int i=0;i<blocks;i++){
      s.read(buf,blocksize);
      socket->send(buffer(buf,blocksize));
    }
    s.read(buf,left);  
    socket->send(buffer(buf,left));
    
    cout<<"a"<<endl;
     sleep(3);
    socket->close();
    s.close();
    delete[] buf;
}
int ftpServer::file_handler(const std::string& filepath) {
  io_service ios;

  while (true) {
    /* code */
    pSocket socket=std::make_shared<ip::tcp::socket>(ip::tcp::socket(ios));
    acceptor->accept(*socket);
    boost::asio::post(thread_pool,std::bind(sendfile,socket,filepath));
    // thread_pool.get_executor().post(std::bind(sendfile,socket,filepath),);
  }
  return 0;
}
}  // namespace ftpServer
