#include "client.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <string>
using namespace boost::asio;
using std::cout;
using std::endl;

client::client() : ios(), sock(ios), blocksize(1024) {
    endpoint.port(9999);
    endpoint.address().from_string("127.0.0.1");
}

void client::send_cmd(const std::string& cmd) {
    sock.connect(endpoint);
    sock.send(buffer(cmd));
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
        // char tmp[100];
        // realpath(argv[2], tmp);
        // boost::filesystem::path p(sdstring(tmp));
        std::string fn(argv[2]);
        cmd += fn;
    } else if (kind == std::string("-p")) {
        cmd += "2 ";
        op = 2;
        char tmp[100];
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
void client::get(const std::string filepath) {
    // io_service io;
    std::ofstream out(filepath, std::iostream::out);
    if (!out.is_open()) {
        std::cout << "open file error" << std::endl;
        return;
    }
    std::string buf(1024, '\0');

    while (true) {
        auto cnt = sock.read_some(buffer(buf));
        if (cnt == 1024) out << buf;
        if (cnt != 1024) {
            for (auto i = 0; i < cnt; ++i) out << buf[i];
            break;
        }
    }
    sock.close();
    out.close();
}

void client::send(const std::string filepath) {
    std::ifstream s(filepath);
    s.seekg(0, s.end);
    auto filesize = s.tellg();
    s.seekg(0, s.beg);
    char* buf = new char[1024];
    int blocks = filesize / blocksize;
    int left = filesize % blocksize;
    cout << blocks << " " << left << endl;

    for (int i = 0; i < blocks; i++) {
        s.read(buf, blocksize);
        sock.send(buffer(buf, blocksize));
    }
    s.read(buf, left);
    sock.send(buffer(buf, left));

    sleep(3);
    sock.close();
    s.close();
    // BOOST_LOG_TRIVIAL(fatal) << "sendfile";
    delete[] buf;
}