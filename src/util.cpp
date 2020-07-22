#include "util.h"

#include "logger.h"
int cnt_file(const std::filesystem::path& p) {
    auto ditor = directory_iterator(p);
    int cnt = 0;
    for (auto& p : std::filesystem::directory_iterator(p))
        cnt++;
    return cnt;
}
bool stoi(const string& s, int& rst) {
    rst = 0;
    if (s[0] == ' ') return false;
    for (auto j = 0; j != s.size(); j++) {
        if (s[j] == ' ') break;
        if (s[j] < '0' || s[j] > '9') return false;
        rst *= 10;
        rst += s[j] - '0';
    }
    return true;
}
void getfile(pSocket socket, const std::string& filepath,
             std::filesystem::path savepath) {
    boost::asio::ip::tcp::no_delay option(true);
    socket->set_option(option);
    std::string proto(128, '\0');
    socket->read_some(buffer(proto));
    cout << proto << endl;
    std::istringstream is(proto);
    string filename;
    int _blocksize, _blocks, _left;
    is >> filename >> _blocksize >> _blocks >> _left;
    std::filesystem::path p(filename);

    std::filesystem::path rst_path = savepath / p.filename().string();
    if (!filepath.empty()) rst_path = savepath / filepath;
    logger(Level::Info) << "start to receive " << rst_path.filename().string()
                        << " blocksize: " << _blocksize
                        << " blocks: " << _blocks
                        << " last block size: " << _left;
    std::ofstream out(rst_path.string(), std::iostream::out);
    if (!out.is_open()) {
        logger(Level::Error) << "open file error" << std::endl;
        return;
    }
    std::string exp("expect ");
    std::string buf(blocksize + 8, '\0');
    int index = 0;
    if (_blocks != 0) {
        while (index < _blocks) {
            /* code */
            if (!socket->is_open()) {
                logger(Level::Error) << "receive" << filepath << " failed at "
                                     << index << " of " << _blocks;
                return;
            }
            string cmd = exp + std::to_string(index);
            cmd.resize(32);
            // send "expect index"
            socket->send(buffer(cmd));
            usleep(50);

            socket->read_some(buffer(buf));
            auto str = buf.substr(0, 8);
            int i = 0;
            // int i = std::stoi(buf.substr(0, 8));
            // cout << str << ' ' << stoi(str, i) << endl;
            // sleep(1);
            if (stoi(str, i) && i == index) {
                out.write(&buf[8], blocksize);
                index++;
            } else {
                cout << "retry get " << index << endl;
            }
        }
    }

    std::string cmd = exp + "left";
    cmd.resize(32);
    socket->send(buffer(cmd));

    if (_left != 0 && socket->is_open()) {
        cout << "read left" << endl;
        socket->read_some(buffer(buf, _left));
        for (auto i = 0; i < _left; ++i)
            out << buf[i];
    }
    logger(Level::Info) << "receive " << filepath << "successfully" << endl;
    out.close();
    sleep(1);
}

void send_file(pSocket socket, const std::string& filepath) {
    char tmp[100];
    // cout << "socket option: " << socket->get_option() << endl;
    boost::asio::ip::tcp::no_delay option(true);
    socket->set_option(option);
    realpath(filepath.c_str(), tmp);
    cout << "send " << tmp << endl;
    std::ifstream s(tmp);
    if (!s.is_open()) {
        logger(Level::Error) << "can't open " << tmp << endl;
        return;
    }
    s.seekg(0, s.end);
    auto filesize = s.tellg();
    s.seekg(0, s.beg);
    // char* buf = new char[blocksize];
    std::string buf(blocksize, '\0');
    int blocks = filesize / blocksize;
    int left = filesize % blocksize;
    std::ostringstream os;
    os << filepath << ' ';

    os << blocksize << ' ' << blocks << ' ' << left << endl;
    std::string proto(os.str());
    for (int i = proto.size(); i <= 128; ++i)
        proto.push_back('\0');
    cout << proto << endl;
    socket->write_some(buffer(proto, 128));
    logger(Level::Info) << "start to send " << tmp
                        << " blocksize: " << blocksize << " blocs " << blocks
                        << " last block size " << left;
    int index = 0;
    std::string cmd_buf(32, ' ');
    string last_block;
    if (blocks != 0) {
        while (index < blocks) {
            if (!socket->is_open()) {
                logger(Level::Error) << "send" << filepath << " failed at "
                                     << index << " of " << blocks;
                return;
            }
            socket->read_some(buffer(cmd_buf));
            usleep(50);
            std::istringstream iss(cmd_buf);
            string tmp;
            int i;
            iss >> tmp >> i;
            string ss(std::to_string(i));
            if (i == index) {
                ss.resize(8 + blocksize, ' ');
                s.read(&ss[8], blocksize);
                socket->send(buffer(ss, 8 + blocksize));
                last_block = ss;
                index++;
            } else {
                usleep(500);
                cout << "retry send" << index << endl;
                socket->send(buffer(last_block));
            }
            usleep(50);
        }
    }
    socket->read_some(buffer(cmd_buf));
    cout << "cmd_buf " << cmd_buf << endl;
    if (left != 0) {
        s.read(const_cast<char*>(buf.c_str()), left);
        socket->send(buffer(buf, left));
    }
    s.close();
    logger(Level::Info) << "send " << filepath << " sucessfully";
    // delete[] buf;
    sleep(1);
}

void senddir(pSocket socket, const std::string& filepath) {
    char tmp[100];
    std::ostringstream os;
    realpath(filepath.c_str(), tmp);
    std::filesystem::path p(tmp);
    if (!std::filesystem::is_directory(p)) {
        logger(Level::Error) << tmp << ": Not a dir" << std::endl;
        return;
    }
    int cnt = cnt_file(p);
    // cout << "cnt: " << cnt << endl;
    os << p.filename().string() << ' ' << cnt;
    string proto(os.str());
    proto += string(64 - proto.size(), ' ');
    socket->send(buffer(proto));
    std::filesystem::directory_iterator ditor(p);
    const std::filesystem::directory_iterator dend;
    while (socket->is_open() && ditor != dend) {
        // cout << ditor->path().string();
        if (ditor->is_directory()) continue;
        if (ditor->is_symlink()) continue;
        send_file(socket, ditor->path().string());
        ++ditor;
    }
}

void recvdir(pSocket socket, const std::string& filename) {
    string proto(64, ' ');
    socket->read_some(buffer(proto));
    std::istringstream is(proto);
    string dirname;
    int cnt;
    is >> dirname >> cnt;
    // cout << dirname << ' ' << cnt << endl;
    string rst = filename;
    if (filename.empty()) rst = dirname;
    std::filesystem::create_directory(rst);
    auto sp = std::filesystem::current_path() / rst;
    for (int i = 0; i < cnt; i++) {
        getfile(socket, "", sp);
    }
    // std::filesystem::
}