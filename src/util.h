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
const int blocksize = 4096;

using pSocket = std::shared_ptr<boost::asio::ip::tcp::socket>;
using boost::asio::buffer;
int cnt_file(const std::filesystem::path& p);

void getfile(pSocket socket, const std::string& filepath,
             std::filesystem::path savepath);
void send_file(pSocket socket, const std::string& filepath);
void senddir(pSocket socket, const std::string& filepath);

void recvdir(pSocket socket, const std::string& filename);