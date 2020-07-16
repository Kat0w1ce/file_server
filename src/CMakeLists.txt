cmake_minimum_required(VERSION 3.0)

project(ftp)
# add_library(boost_beast INTERFACE)

# target_include_directories(boost_beast
#                            SYSTEM
#                            PUBLIC
#                            "${CMAKE_CURRENT_LIST_DIR}/../beast/include")

find_package(Boost REQUIRED system thread regex filesystem log )
include_directories(${Boost_INCLUDE_DIRS})
aux_source_directory(. DIR_SRC)

add_executable(demo main.cpp  ftp_server.cpp)
add_executable(client  c.cpp client.cpp)
add_executable(test_fsop  test_fileop.cpp fsoperator.cpp)
# add_executable(test_logger  test_logger.cpp)
SET(CMAKE_CXX_FLAGS "-std=c++17")
set_property(TARGET demo PROPERTY CXX_STANDARD 17)
set_property(TARGET test_fsop PROPERTY CXX_STANDARD 17)
set_property(TARGET client PROPERTY CXX_STANDARD 17)
target_link_libraries(demo ${Boost_LIBRARIES})
target_link_libraries(client ${Boost_LIBRARIES})
target_link_libraries(test_fsop ${Boost_LIBRARIES})
# target_link_libraries(test_logger ${Boost_LIBRARIES})
# target_compile_definitions(server/demo )
# target_compile_definitions(client/client )