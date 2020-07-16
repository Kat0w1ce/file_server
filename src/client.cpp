#include "client.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "util.h"
using namespace boost::asio;
using std::cout;
using std::endl;

client::client()
    : ios(), sock(std::make_shared<ip::tcp::socket>(ios)), blocksize(1024) {
    endpoint.port(9999);
    endpoint.address().from_string("127.0.0.1");
}

void client::send_cmd(const std::string& cmd) {
    sock->connect(endpoint);
    sock->send(buffer(cmd));
    // sleep(1);
}
std::pair<int, std::string> client::build_cmd(int argc, char const* argv[]) {
    std::string cmd;
    int op = 0;
    if (argc < 3) {
        return std::make_pair<int, std::string>(std::move(op), std::move(cmd));
    }

    std::string kind(argv[1]);
    cout << kind << endl;
    if (kind == std::string("-c")) {
        cmd += "0 ";
        std::string opl(argv[2]);
        if (argc == 4) {
            if (opl != std::string("cd")) {
                cmd.clear();
                return std::make_pair<int, std::string>(std::move(op),
                                                        std::move(cmd));
            } else {
                cmd += opl;
                cmd += ' ';
                cmd += argv[3];
                cout << cmd << endl;
            }
        } else {
            cmd += op;
        }
    } else if (kind == std::string("-g")) {
        cmd += "1 ";
        op = 1;
        std::string fn(argv[2]);
        cmd += fn;
    } else if (kind == std::string("-p")) {
        cmd += "2 ";
        op = 2;
        char tmp[100];
        realpath(argv[2], tmp);
        std::filesystem::path p(tmp);
        cout << p.filename().string() << endl;
        cmd += p.filename().string();
    } else if (kind == std::string("-gr")) {
        cmd += "3 ";
        op = 3;
        std::string fn(argv[2]);
        cmd += fn;
    } else {
        cout << "invalid operation" << endl;
        cout << "help" << endl;
    }
    std::string t(64 - cmd.size(), '\0');
    cmd += t;
    return std::make_pair<int, std::string>(std::move(op), std::move(cmd));
}
void client::get(const std::string filepath) { getfile(sock, filepath); }

client::~client() {
    if (sock->is_open()) {
        sock->close();
    }
}
// filename blocksize blocks leftsize
void client::send(const std::string filepath) { send_file(sock, filepath); }

void client::getdir(const std::string dirpath) { recvdir(sock, dirpath); }