#pragma once
#include <unistd.h>

#include <algorithm>
#include <boost/asio.hpp>
#include <memory>
#include <string>
class client {
   private:
    //   boost::asio::io_service io_service;
    boost::asio::io_service ios;
    boost::asio::ip::tcp::endpoint endpoint;
    std::shared_ptr<boost::asio::ip::tcp::socket> sock;
    const int blocksize;
    const std::string ip;
    const int port;
    const std::string username;
    const std::string pwd;

   public:
    client(std::string ip, int port, std::string _username, std::string _pwd);
    client() = delete;
    ~client();
    void get(const std::string filepath);
    void get_dir(const std::string dirpath);
    std::pair<int, std::string> build_cmd(int argc,
                                          const std::vector<std::string> argv);
    void send_cmd(const std::string& cmd);
    void send(const std::string filepath);
    void send_dir(const std::string dirpath);
    void run();
    bool login();
};
