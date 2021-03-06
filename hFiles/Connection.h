//
// Created by yxc on 16-11-29.
//

#ifndef WEBSERVER_CONNECTION_H
#define WEBSERVER_CONNECTION_H


#include <event2/event.h>
#include <event2/bufferevent.h>
#include "HttpParser.h"
#include "HttpResponse.h"
#include <string>
#include <queue>

using namespace std;
class Connection {
public:
    Connection(evutil_socket_t fd,bufferevent *bev);
    void setState(int state);
    bufferevent *getBev() const;
    void setBev(bufferevent *bev);
    ~Connection();
    bool parseHttp(std::string buf,int nread);
    bool responsePrepare();
    void writeResponse();
    const std::string &getMsg() const;
    void setMsg(const std::string &msg);
    int getState() const;
    bool isKeepAlive();
    void closeConn();
    static void inQueue(const http_request_t & hrt);
private:
    evutil_socket_t connfd;
    int state;
    bufferevent *bev;
    HttpParser hp_current;
    HttpResponse hr_current;
    std::string msg;
    static queue<http_request_t> q_http_request;
    queue<http_response> out_queue;
private:
    Connection(const Connection &){}
    Connection &operator =(const Connection &){}
};


#endif //WEBSERVER_CONNECTION_H
