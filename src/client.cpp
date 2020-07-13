#include "client.h"
#include <string>
#include<boost/beast.hpp>
using namespace boost::asio;

client::client(){
  endpoint.port(9999);
  endpoint.address().from_string("127.0.0.1");
    
}
void client::run(){
    io_context io;
    std::string s("hello");
    boost::beast::tcp_stream stream(io);
    ip::tcp::endpoint endpoint;
    endpoint.address().from_string("127.0.0.1");
    endpoint.port(9999);
    stream.connect(endpoint);
    stream.write_some(buffer(s));
    // socket.send(buffer(s));
}