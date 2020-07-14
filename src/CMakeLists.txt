cmake_minimum_required(VERSION 3.0)

project(ftp)
# add_library(boost_beast INTERFACE)

# target_include_directories(boost_beast
#                            SYSTEM
#                            PUBLIC
#                            "${CMAKE_CURRENT_LIST_DIR}/../beast/include")

find_package(Boost REQUIRED system thread regex )
include_directories(${Boost_INCLUDE_DIRS})
aux_source_directory(. DIR_SRC)

add_executable(demo main.cpp  ftp_server.cpp)
add_executable(client  c.cpp client.cpp)
set_property(TARGET demo PROPERTY CXX_STANDARD 11)
target_link_libraries(demo ${Boost_LIBRARIES})