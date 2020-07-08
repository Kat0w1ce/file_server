#include <boost/asio.hpp>
#include <memory>
#include <string>
namespace ftpServer {
class ftpServer {
 public:
  void start();
  int run();

  ftpServer(const std::string ip, const int port);
  ~ftpServer() { acceptor.release(); }

 private:
  const std::string localhost;
  const int Port;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
  boost::asio::ip::tcp::endpoint endpoint;
};
}  // namespace ftpServer