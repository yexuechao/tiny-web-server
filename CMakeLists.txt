cmake_minimum_required(VERSION 3.5)
project(webServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Master.cpp Master.h Worker.cpp Worker.h Connection.cpp Connection.h ConnectionState.h http_parser.c http_parser.h HttpParser.cpp HttpParser.h HttpResponse.cpp HttpResponse.h)
link_libraries(libevent.a)
add_executable(webServer ${SOURCE_FILES})