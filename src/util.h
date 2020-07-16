#pragma once
#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
using std::cout;
using std::endl;
using std::string;
using std::filesystem::directory_iterator;
const int blocksize = 1024;
int cnt_file(const std::filesystem::path& p) {
    auto ditor = directory_iterator(p);
    int cnt;
    const directory_iterator end;
    while (ditor != end) {
        ++ditor;
        cnt++;
    }
    return cnt;
}
using pSocket = std::shared_ptr<boost::asio::ip::tcp::socket>;
using boost::asio::buffer;
void getfile(pSocket socket, const std::string& filepath) {
    std::string proto(256, ' ');
    auto j = socket->read_some(buffer(proto));
    std::istringstream is(proto);
    string filename;
    int _blocksize, _blocks, _left;
    is >> filename >> _blocksize >> _blocks >> _left;
    std::filesystem::path p(filename);
    // std::filesystem::path fp(filepath);
    // auto r = p /= fp.filename().string();
    // cout << r.string() << endl;
    p /= filepath;
    cout << p.string() << endl;
    std::ofstream out(p.filename().string(), std::iostream::out);
    if (!out.is_open()) {
        std::cout << "open file error" << std::endl;
        return;
    }
    std::string buf(1024, '\0');
    for (int i = 0; i < _blocks; ++i) {
        auto cnt = socket->read_some(buffer(buf));
        out << buf;
    }
    socket->read_some(buffer(buf, _left));
    for (auto i = 0; i < _left; ++i) out << buf[i];
    socket->close();
    out.close();
}
void send_file(pSocket socket, const std::string& filepath) {
    char tmp[100];
    realpath(filepath.c_str(), tmp);
    std::ifstream s(tmp);
    if (!s.is_open()) {
        cout << "can't open " << tmp << endl;
        return;
    }
    s.seekg(0, s.end);
    auto filesize = s.tellg();
    s.seekg(0, s.beg);
    char* buf = new char[1024];
    int blocks = filesize / blocksize;
    int left = filesize % blocksize;
    std::string proto;
    std::ostringstream os;
    os << filepath << ' ';

    os << std::to_string(blocksize) << ' ' << std::to_string(blocks) << ' '
       << std::to_string(left) << endl;
    proto = os.str();
    proto += std::string(256 - proto.size(), ' ');
    socket->send(buffer(proto));
    cout << filepath << endl;
    for (int i = 0; i < blocks; i++) {
        s.read(buf, blocksize);
        socket->send(buffer(buf, blocksize));
    }
    s.read(buf, left);
    socket->send(buffer(buf, left));

    // socket->close();
    s.close();

    delete[] buf;
}
void senddir(pSocket socket, const std::string& filepath) {
    char tmp[100];
    std::ostringstream os;
    realpath(filepath.c_str(), tmp);
    std::filesystem::path p(tmp);
    if (!std::filesystem::is_directory(p)) {
        std::cout << tmp << ": Not a dir" << std::endl;
        return;
    }
    int cnt = cnt_file(p);
    os << p.filename().string() << ' ' << cnt;
    string proto(os.str());
    proto += string(64 - proto.size(), '\0');
    socket->send(buffer(proto));
    std::filesystem::directory_iterator ditor(p);
    const std::filesystem::directory_iterator dend;
    while (socket->is_open() && ditor != dend) {
        send_file(socket, ditor->path().filename().string());
        ++ditor;
    }
}

void recvdir(pSocket socket, const std::string& filename) {
    string proto(64, '\0');
    socket->read_some(buffer(proto));
    std::istringstream is(proto);
    string dirname;
    int cnt;
    is >> dirname >> cnt;
    cout << dirname << ' ' << cnt << endl;

    std::filesystem::create_directory(filename);
    // std::filesystem::
}