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

    int getRequest_count() const;

    void setRequest_count(int request_count);

    const std::string &getMsg() const;

    void setMsg(const std::string &msg);

    int getState() const;
    void connectionStateMachine();
    void addRequestCount();
    void enableRead();
    bool isKeepAlive();
    void closeConn();

    http_request_t *getHttp_request() const;

    void setHttp_request(http_request_t *http_request);

    void requestStart();
    void eraseHttpRequest();
private:

    evutil_socket_t connfd;
    int state;
    int deal_times;
    int is_readable;
    int request_count;
    bufferevent *bev;
    http_request_t *http_request;
    http_response response;
    http_response before_response;
    std::string msg;
};


#endif //WEBSERVER_CONNECTION_H
