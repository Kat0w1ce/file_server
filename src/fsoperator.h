#pragma once
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
namespace ftpServer {
using std::cout;
using std::endl;
using std::string;
class fileOperator {
    using Path = boost::filesystem::path;

   public:
    fileOperator();
    inline const Path pwd() {
        std::cout << _path.string() << endl;
        return _path;
    }
    void ls();
    bool cd(const string& _path);

   private:
    boost::filesystem::path _path;
};

}  // namespace ftpServer