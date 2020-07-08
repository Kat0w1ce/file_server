cmake_minimum_required(VERSION 3.0)

project(ftp)

find_package(Boost REQUIRED system thread regex)
include_directories(${Boost_INCLUDE_DIRS})
aux_source_directory(. DIR_SRC)

add_executable(demo main.cpp  ftp_server.cpp)

target_link_libraries(demo ${Boost_LIBRARIES})