#include "client.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "logger.h"
#include "util.h"
using namespace boost::asio;
using std::cout;
using std::endl;
extern fileLogger logger("b.log");
client::client(std::string _ip, int _port, const std::string _username,
               std::string _pwd)
    : ios(),
      username(_username),
      pwd(_pwd),
      sock(std::make_shared<ip::tcp::socket>(ios)),
      blocksize(1024),
      ip(_ip),
      port(_port) {
    endpoint.port(_port);
    endpoint.address(boost::asio::ip::make_address_v4(_ip));
}

bool client::login() {
    cout << endpoint.address() << "  " << endpoint.port() << endl;
    sock->connect(endpoint);
    boost::asio::ip::tcp::no_delay option(true);
    sock->set_option(option);
    string msg(username);

    msg += ' ';
    msg += pwd;
    msg += string(256 - msg.size(), ' ');
    sock->send(buffer(msg));
    sock->read_some(buffer(msg));

    cout << msg[0] << endl;
    return msg[0] == '1';
}
void client::send_cmd(const std::string& cmd) {
    sock->send(buffer(cmd, 256));
    cout << "a" << endl;
    // sleep(1);
}

void client::run() {
    if (!login()) {
        cout << "username or password is incorrect" << endl;
        return;
    }
    cout << "login successfully" << endl;
    std::string line, tmp;
    std::getline(std::cin, line);
    std::istringstream is(line);
    std::vector<std::string> vs;
    while (is >> tmp) {
        /* code */
        vs.emplace_back(tmp);
    }
    auto rst = build_cmd(vs.size(), vs);
    send_cmd(rst.second);
    cout << "ggg" << '\n';
    if (rst.first == 1) get(vs[2]);
    if (rst.first == 2) send(vs[1]);
    if (rst.first == 3) get_dir(vs[2]);
    if (rst.first == 4) send_dir(vs[1]);
}
std::pair<int, std::string> client::build_cmd(
    int argc, const std::vector<std::string> argv) {
    std::string cmd;
    int op = 0;
    if (argc < 2) {
        return std::make_pair<int, std::string>(std::move(op), std::move(cmd));
    }

    if (argv[0] == std::string("-c")) {
        cmd += "0 ";
        std::string opl(argv[1]);
        if (argc == 3) {
            // if (opl != std::string("cd")) {
            //     cmd.clear();
            //     return std::make_pair<int, std::string>(std::move(op),
            //                                             std::move(cmd));
            // } else {
            //     cmd += opl;
            //     cmd += ' ';
            //     cmd += argv[2];
            //     cout << cmd << endl;
            // }
            // } else {
            //     cmd += op;
            // }
        }
    } else if (argv[0] == std::string("-g")) {
        cmd += "1 ";
        op = 1;
        std::string fn(argv[1]);
        cmd += fn;
    } else if (argv[0] == std::string("-p")) {
        cmd += "2 ";
        op = 2;
        char tmp[100];
        realpath(argv[1].c_str(), tmp);
        std::filesystem::path p(tmp);
        cmd += p.filename().string();
    } else if (argv[0] == std::string("-gr")) {
        cmd += "3 ";
        op = 3;
        std::string fn(argv[1]);
        cmd += fn;
    } else if (argv[0] == std::string("-pr")) {
        cmd += "4 ";
        op = 4;
        char tmp[100];
        realpath(argv[1].c_str(), tmp);
        std::filesystem::path p(tmp);
        cout << p.filename().string() << endl;
        cmd += p.filename().string();
    } else {
        cout << "invalid operation" << endl;
        cout << "help" << endl;
    }
    std::string t(256 - cmd.size(), '\0');
    cmd += t;
    return std::make_pair<int, std::string>(std::move(op), std::move(cmd));
}

void client::get(const std::string filepath) {
    getfile(sock, filepath, std::filesystem::current_path());
}

client::~client() {
    if (sock->is_open()) {
        sock->close();
    }
}
// filename blocksize blocks leftsize
void client::send(const std::string filepath) { send_file(sock, filepath); }

void client::get_dir(const std::string dirpath) { recvdir(sock, dirpath); }
void client::send_dir(const std::string dirpath) { senddir(sock, dirpath); }