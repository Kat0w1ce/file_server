#include<boost/asio.hpp>
#include<string>
#include <unistd.h>
class client {
 private:
//   boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint;
 public:
//   client(std::string ip,int port);
  client ();
  ~client(){};
  void run(const std::string filepath);
};


