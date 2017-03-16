cmake_minimum_required(VERSION 3.5)
project(webServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        cppFiles/Master.cpp
        hFiles/Master.h
        cppFiles/Worker.cpp
        hFiles/Worker.h
        cppFiles/Connection.cpp
        hFiles/Connection.h
        hFiles/ConnectionState.h
        httpParser/http_parser.c
        httpParser/http_parser.h
        cppFiles/HttpParser.cpp
        hFiles/HttpParser.h
        cppFiles/HttpResponse.cpp
        hFiles/HttpResponse.h)

link_libraries(libevent.a)
add_executable(webServer ${SOURCE_FILES})