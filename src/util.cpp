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

    std::ostringstream idx_out;
    idx_out << "expect 0" << endl;
    std::string index_buf = idx_out.str();
    index_buf.append(32 - index_buf.size(), ' ');
    socket->send(buffer(index_buf, 32));
    std::string buf(blocksize, '\0');
    for (int i = 0; i < _blocks; ++i) {
        socket->read_some(buffer(index_buf));
        cout << "get: " << index_buf << endl;
        std::istringstream idx_in(index_buf);
        string tosend;
        // get index
        int index;
        idx_in >> tosend >> index;

        if (!socket->is_open()) {
            logger(Level::Error) << "receive" << filepath << " failed at " << i
                                 << " of " << _blocks;
            return;
        }
        if (index == i) {
            cout << "yes" << endl;

            auto cnt = socket->read_some(buffer(buf));
            out << buf;
            std::ostringstream idx_stream;
            idx_stream << "expect " << (i + 1) << endl;
            cout << "idx_out" << idx_stream.str() << endl;
            index_buf.clear();
            index_buf = idx_stream.str();
            index_buf.append(32 - index_buf.size(), ' ');
            cout << "index_buf " << index_buf << endl;

            if (i + 1 == _blocks) {
                std::string s("left 0");
                s.append(32 - s.size(), ' ');
                socket->send(buffer(s));
                break;
            }
            socket->send(buffer(index_buf));

        } else {
            cout << "retry" << endl;
            socket->send(buffer(index_buf));

            --i;
        }
    }
    if (_left != 0 && socket->is_open()) {
        cout << "read left" << endl;
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
    int index;
    std::string expect, idx_buf(32, ' ');
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
        // idx_buf.clear();
        idx_buf = std::string(32, '\0');
        // get expectation
        socket->read_some(buffer(idx_buf));
        std::istringstream idx_in(idx_buf);
        idx_in >> expect >> index;
        cout << "send: " << expect << ' ' << index << endl;

        if (index == i) {
            idx_buf.clear();
            idx_buf += "send ";
            idx_buf += std::to_string(i);
            idx_buf.append(32 - idx_buf.size(), ' ');
            socket->send(buffer(idx_buf));
            cout << "idx send " << idx_buf << endl;
            s.read(buf, blocksize);
            socket->send(buffer(buf, blocksize));
            socket->wait(boost::asio::socket_base::wait_type::wait_write);
        }
    }
    socket->read_some(buffer(idx_buf));
    if (left != 0) {
        s.read(buf, left);
        socket->send(buffer(buf, left));
        socket->wait(boost::asio::socket_base::wait_type::wait_write);
    }
    s.close();
    logger(Level::Info) << "send " << filepath << " sucessfully";
    delete[] buf;
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