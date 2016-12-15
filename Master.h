//
// Created by yxc on 16-11-28.
//

#ifndef WEBSERVER_MASTER_H
#define WEBSERVER_MASTER_H

#include <iostream>
#include <event2/event.h>
using std::string;
class Master {
public:
    Master(const string &ip,const int &port,const int &num_of_child);
    //注册信号函数,生成子进程,生成listenfd,然后交给子进程处理
    bool run();
    ~Master();

private:
    static void signalSIGINT(evutil_socket_t sig, short events, void *user_data);
    //更新父进程和子进程
//    void signalSighup(evutil_socket_t sig, short events, void *user_data);
    static void signalSIGTERM(evutil_socket_t sig, short events, void *user_data);
    static void signalSIGCHLD(evutil_socket_t sig, short events, void *user_data);
    bool startListenfd();
    string listen_ip;
    int listen_port;
    int number_of_child;
    evutil_socket_t listen_fd;
};


#endif //WEBSERVER_MASTER_H
