//
// Created by yxc on 16-11-28.
//

#ifndef WEBSERVER_WORKER_H
#define WEBSERVER_WORKER_H

#include <event2/event.h>
#include <map>
#include "Connection.h"
#include <boost/make_shared.hpp>
class Worker {
public:
    //信号处理
    //sigint
    Worker();
    void run(int listenfd);
    ~Worker();
private:
    static void signalSIGINT(evutil_socket_t sig, short events, void *user_data);
    static void do_accept(evutil_socket_t listenfd,short events,void *arg);
    static void doRead(bufferevent *bev,void *user_data);
    static void doError(bufferevent *bev,short events, void *user_data);
    static void doWrite(bufferevent *bev,void *user_data);
    static void connectionStateMachine(boost::shared_ptr<Connection> &conn);
    evutil_socket_t listen_fd;
    static std::map<evutil_socket_t,boost::shared_ptr<Connection> > conns;
    static int conns_num;
};


#endif //WEBSERVER_WORKER_H
