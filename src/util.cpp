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
    std::string proto(128, ' ');
    boost::asio::read(*socket, buffer(proto, 64));
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

    std::string buf(blocksize, '\0');
    for (int i = 0; i < _blocks; ++i) {
        if (!socket->is_open()) {
            logger(Level::Error) << "receive" << filepath << " failed at " << i
                                 << " of " << _blocks;
            return;
        }
        auto cnt = socket->read_some(buffer(buf));
        out << buf;
    }
    if (_left != 0 && socket->is_open()) {
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
    socket->write_some(buffer(proto, 128));
    logger(Level::Info) << "start to send " << tmp
                        << " blocksize: " << blocksize << " blocs " << blocks
                        << " last block size " << left;
    for (int i = 0; socket->is_open() && i < blocks; i++) {
        s.read(buf, blocksize);
        if (!socket->is_open()) {
            logger(Level::Error)
                << "send" << filepath << " failed at " << i << " of " << blocks;
            return;
        }
        socket->send(buffer(buf, blocksize));
    }
    if (left != 0) {
        s.read(buf, left);
        socket->send(buffer(buf, left));
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