#pragma once
#include<string>
#include<boost/filesystem.hpp>
#include<iostream>
namespace ftpServer{
using std::string;
using std::cout;
using std::endl;
class fileOperator{
    using Path=boost::filesystem::path;
    public:
    fileOperator();
    inline const Path pwd(){
       std::cout<< _path.string()<<endl ;
       return _path;
    }
    void ls();
    bool cd(const string& _path);
    private:
    boost::filesystem::path _path;
};

    
   
}