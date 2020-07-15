#include "ftp_server.h"
using std::string;
int main() {
    ftpServer::ftpServer server("0", 9999);
    server.start();
    server.file_handler();
}
