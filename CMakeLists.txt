cmake_minimum_required(VERSION 2.6)
project(hashtable)

SET( CMAKE_CXX_FLAGS  "-pthread" )

SET(SRC_LIST Common.cpp Threads.cpp)
add_executable(hashtable ${SRC_LIST} main.cpp)
target_link_libraries(hashtable "-lrt")
