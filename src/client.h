#include <unistd.h>

#include <boost/asio.hpp>
#include <string>
class client {
   private:
    //   boost::asio::io_service io_service;
    boost::asio::io_service ios;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket sock;
    const int blocksize;

   public:
    //   client(std::string ip,int port);
    client();
    ~client();
    void get(const std::string filepath);
    std::pair<int, std::string> build_cmd(int argc, char const* argv[]);
    void send_cmd(const std::string& cmd);
    void send(const std::string filepath);
};
