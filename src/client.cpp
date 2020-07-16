#include "client.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
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
    std::string proto(256, ' ');
    auto j = sock->read_some(buffer(proto));
    std::istringstream is(proto);
    std::string filename;
    int _blocksize, _blocks, _left;
    is >> filename >> _blocksize >> _blocks >> _left;
    std::ofstream out(filepath, std::iostream::out);
    if (!out.is_open()) {
        std::cout << "open file error" << std::endl;
        return;
    }
    std::string buf(1024, '\0');
    for (int i = 0; i < _blocks; ++i) {
        auto cnt = sock->read_some(buffer(buf));
        out << buf;
    }
    sock->read_some(buffer(buf, _left));
    for (auto i = 0; i < _left; ++i) out << buf[i];

    out.close();
}

client::~client() {
    if (sock->is_open()) {
        sock->close();
    }
}
// filename blocksize blocks leftsize
void client::send(const std::string filepath) {
    std::ifstream s(filepath);
    s.seekg(0, s.end);
    auto filesize = s.tellg();
    s.seekg(0, s.beg);
    char* buf = new char[1024];
    std::string proto;
    std::ostringstream os;
    os << filepath << ' ';

    int blocks = filesize / blocksize;
    int left = filesize % blocksize;
    os << std::to_string(blocksize) << ' ' << std::to_string(blocks) << ' '
       << std::to_string(left) << endl;
    proto = os.str();
    proto += std::string(256 - proto.size(), ' ');
    sock->send(buffer(proto));
    // sleep(1);
    for (int i = 0; i < blocks; i++) {
        s.read(buf, blocksize);
        sock->send(buffer(buf, blocksize));
    }
    s.read(buf, left);
    sock->send(buffer(buf, left));

    s.close();
    // BOOST_LOG_TRIVIAL(fatal) << "sendfile";
    delete[] buf;
}