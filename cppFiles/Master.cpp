//
// Created by yxc on 16-11-28.
//

#include <assert.h>
#include <cstring>
#include <zconf.h>
#include <wait.h>
#include "../hFiles/Master.h"
#include "../hFiles/Worker.h"

Master::Master(const string &ip, const int &port,const int &number_of_child)
        :listen_ip(ip),listen_port(port),number_of_child(number_of_child)
{}
bool Master::startListenfd() {
    evutil_socket_t listenfd;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    assert(listenfd>0);
    evutil_make_listen_socket_reuseable(listenfd);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    evutil_inet_pton(AF_INET,listen_ip.c_str(),&addr.sin_addr);
    addr.sin_port=htons(listen_port);
    std::cout<<listen_ip<<"  "<<listen_port<<std::endl;
    if(bind(listenfd,(struct sockaddr *)&addr,sizeof(addr))<0){
        std::cerr<<"bind wrong";
        return false;
    }
    if(listen(listenfd,10)<0){
        std::cerr<<"listen error";
        return false;
    }
//    std::cout<<"listening"<<std::endl;
    //nonblocking
    assert(evutil_make_socket_nonblocking(listenfd)==0);
    listen_fd=listenfd;
    return true;
}
bool Master::run() {
    if(!startListenfd()){
        return false;
    }

    int num_of_child=number_of_child;
    Worker worker;
    pid_t pid;
    while(num_of_child>0){
        switch (pid=fork()){
            case -1:
                std::cerr<<"fork error";
                return false;
            case 0:
                //子进程
                worker.run(listen_fd);
                return true;
            default:
                num_of_child--;
                break;
        }
    }
    event_base *base=event_base_new();
    //信号处理
    event *signal_SIGINT;
    event *signal_SIGTERM;
    event *signal_SIGCHLD;

    signal_SIGINT =evsignal_new(base, SIGINT, signalSIGINT, NULL);
    evsignal_add(signal_SIGINT,NULL);
    signal_SIGTERM =evsignal_new(base, SIGTERM, signalSIGTERM, signal_SIGTERM);
    evsignal_add(signal_SIGTERM,NULL);
    signal_SIGCHLD=evsignal_new(base,SIGCHLD,signalSIGCHLD,NULL);
    evsignal_add(signal_SIGCHLD,NULL);

    event_base_dispatch(base);
    event_free(signal_SIGTERM);
    event_free(signal_SIGCHLD);
    event_free(signal_SIGINT);
    event_base_free(base);
}

void Master::signalSIGINT(evutil_socket_t sig, short events, void *user_data) {
    event_base *base=(event_base *)user_data;
    struct timeval delay = { 2, 0 };
    std::cout<<"Caught an interrupt signal; exiting cleanly in two seconds."<<std::endl;
    event_base_loopexit(base,&delay);
}

void Master::signalSIGTERM(evutil_socket_t sig, short events, void *user_data) {
    event_base *base=(event_base *)user_data;
    std::cout<<"SIGTERM occur"<<std::endl;
    std::cout<<getpid()<<std::endl;
    kill(0,SIGTERM);
    evsignal_del((event *)user_data);
    event_base_loopexit(base,NULL);
}
void Master::signalSIGCHLD(evutil_socket_t sig, short events, void *user_data) {
    pid_t pid;
    int stat;
    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0){
        
        std::cout<<"child "<<pid<<" terminated"<<std::endl;
    }
    return ;
}
Master::~Master() {
    //释放资源
    if(-1==close(listen_fd)){
        std::cerr<<"close error";
    }
}