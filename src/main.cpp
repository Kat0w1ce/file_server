#include "ftp_server.h"
using std::string;
int main(int argc, char const *argv[]) {
    ftpServer::ftpServer server("0", std::stoi(argv[1]));
    server.start();
    server.file_handler();
}
