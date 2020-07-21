#include "util.h"

#include "logger.h"
int cnt_file(const std::filesystem::path& p) {
    auto ditor = directory_iterator(p);
    int cnt = 0;
    for (auto& p : std::filesystem::directory_iterator(p))
        cnt++;
    return cnt;
}

void getfile(pSocket socket, const std::string& filepath,
             std::filesystem::path savepath) {
    boost::asio::ip::tcp::no_delay option(true);
    socket->set_option(option);
    std::string proto(128, '\0');
    // boost::asio::read(*socket, buffer(proto, 64));
    socket->wait(boost::asio::socket_base::wait_type::wait_read);
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

    // std::string index_buf = "expect 0";
    const std::string exp = "expect ";
    std::string buf(blocksize + 8, '\0');
    for (int i = 0; i < _blocks; ++i) {
        std::string cmd = "expect ";
        if (!socket->is_open()) {
            logger(Level::Error) << "receive" << filepath << " failed at " << i
                                 << " of " << _blocks;
            return;
        }
        std::string index_buf = exp + std::to_string(i);
        cout << index_buf << endl;
        index_buf.append(32 - index_buf.size(), ' ');
        socket->send(buffer(index_buf, 32));
        socket->wait(boost::asio::socket_base::wait_type::wait_read);
        usleep(50);
        socket->read_some(buffer(buf));
        auto idx = buf.substr(0, 8);
        auto cnt = std::stoi(idx);
        if (cnt == i) {
            cout << "yes " << cnt << endl;
            auto data = buf.substr(8, blocksize);
            out << data;
            if (i + 1 == _blocks) {
                // cmd += std::to_string(-1);
                cout << "read left" << endl;
                break;
            } else {
                cmd += std::to_string(i + 1);
            }
            cmd.append(32 - cmd.size(), ' ');
            socket->send(buffer(cmd));
        } else {
            cout << "retry " << idx << endl;
            cmd += std::to_string(i);
            cmd.append(32 - cmd.size(), ' ');
            socket->send(buffer(cmd));
            i--;
        }
    }
    if (_left != 0 && socket->is_open()) {
        socket->wait(boost::asio::socket_base::wait_type::wait_read);
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
    string get(32, '0');
    // string buf(blocksize, '\0');
    char* buf = new char[blocksize];
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

    socket->wait(boost::asio::socket_base::wait_type::wait_write);
    logger(Level::Info) << "start to send " << tmp
                        << " blocksize: " << blocksize << " blocs " << blocks
                        << " last block size " << left;

    for (int i = 0; socket->is_open() && i < blocks; i++) {
        if (!socket->is_open()) {
            logger(Level::Error)
                << "send" << filepath << " failed at " << i << " of " << blocks;
            return;
        }
        socket->read_some(buffer(get));
        std::istringstream index(get);
        string tmp, idxs;
        int idx;
        index >> tmp >> idx;

        if (idx == i) {
            s.read(buf, blocksize);
            cout << tmp << " " << idx << endl;
        } else if (idx == -1) {
            cout << "left" << endl;
            break;
        } else {
            --i;
        }
        std::string ss;
        idxs = std::to_string(idx);
        idxs.append(8 - idxs.size(), ' ');
        ss = idxs + string(buf);
        socket->send(buffer(ss, 8 + blocksize));
        socket->wait(boost::asio::socket_base::wait_type::wait_write);
    }
    // socket->read_some(buffer(get));
    cout << "left " << get << endl;
    if (left != 0) {
        s.read(buf, left);
        socket->send(buffer(buf, left));
        socket->wait(boost::asio::socket_base::wait_type::wait_write);
    }
    s.close();
    logger(Level::Info) << "send " << filepath << " sucessfully";
    sleep(1);
    delete[] buf;
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
    socket->wait(boost::asio::socket_base::wait_type::wait_write);
    std::filesystem::directory_iterator ditor(p);
    const std::filesystem::directory_iterator dend;
    while (socket->is_open() && ditor != dend) {
        // cout << ditor->path().string();
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