#include "client.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace boost::asio;
using std::cout,std::endl;
client::client(){
  endpoint.port(9999);
  endpoint.address().from_string("127.0.0.1");
    
}
void client::run(){
    io_service io;
    std::string s("hello");
    std::ofstream out("b.png",std::iostream::out);
    if(!out.is_open()){
        std::cout<<"fl"<<std::endl;
        return;
    }
    // boost::beast::tcp_stream stream(io);
    ip::tcp::socket sock(io);
    // char buf[1024];
    std::string buf(1024,'\0');
    sock.connect(endpoint);

    while( true){
        cout<<'a'<<endl;
        auto cnt=sock.read_some(buffer(buf));
        cout<<cnt<<endl;
        if(cnt==1024)
            out<<buf;
        if(cnt!=1024){
            for(auto i=0;i<cnt;++i)
                out<<buf[i];
            break;
        }
    }
 
    sock.close();
    out.close();
}