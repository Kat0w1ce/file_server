cmake_minimum_required(VERSION 3.0)

project(ftp)
# add_library(boost_beast INTERFACE)

# target_include_directories(boost_beast
#                            SYSTEM
#                            PUBLIC
#                            "${CMAKE_CURRENT_LIST_DIR}/../beast/include")

find_package(Boost REQUIRED system thread regex filesystem )
include_directories(${Boost_INCLUDE_DIRS})
aux_source_directory(. DIR_SRC)

add_executable(demo main.cpp  ftp_server.cpp)
add_executable(client  c.cpp client.cpp)
add_executable(test_fsop  test_fileop.cpp fsoperator.cpp)
set_property(TARGET demo PROPERTY CXX_STANDARD 14)
target_link_libraries(demo ${Boost_LIBRARIES})
target_link_libraries(test_fsop ${Boost_LIBRARIES})