//
// Created by yxc on 16-11-28.
//

#include <iostream>
#include <csignal>
#include <zconf.h>
#include <cstring>
#include "../hFiles/HttpParser.h"
#include <memory>
#include "../hFiles/Worker.h"
#include "../hFiles/ConnectionState.h"
#include <event2/buffer.h>

std::map<evutil_socket_t,std::shared_ptr<Connection>>Worker::conns=std::map<evutil_socket_t,std::shared_ptr<Connection>>();
Worker::Worker()
{

}
void Worker::run(int listenfd){
    listen_fd=listenfd;
    event_base *base=event_base_new();
//    std::cout<<&base<<std::endl;
//    添加信号事件
    event *signal_SIGINT;
    if((signal_SIGINT =evsignal_new(base, SIGINT, signalSIGINT, NULL))==NULL){
        std::cerr<<" signal_SIGINT error";
        return;
    }
    if(evsignal_add(signal_SIGINT,NULL)<0){
        std::cerr<<"signal add error";
        return ;
    }
    //添加listen_fd
    event *listen_event;
    if(!(listen_event=event_new(base,listen_fd,EV_READ|EV_PERSIST,doAccept,base))){
        std::cerr<<"listen_event error";
        return ;
    }
    if(event_add(listen_event,NULL)<0){
        std::cerr<<"listen add error";
        return ;
    }
    event_base_dispatch(base);
    event_free(signal_SIGINT);
    event_free(listen_event);
    event_base_free(base);
}

void Worker::doAccept(evutil_socket_t listenfd, short events, void *user_data) {
    event_base *base=(event_base *)user_data;
    evutil_socket_t connfd=-1;
    sockaddr_in clien_addr;
    socklen_t clien_len;
    connfd=accept(listenfd,(sockaddr *)&clien_addr,&clien_len);
    cout<<"new conn="<<connfd<<endl;
    if(-1==connfd){
        switch(errno){
//非阻塞accept当前没有连接请求
            case EAGAIN:
#if EWOULDBLOCK!=EAGAIN
                caes EWOULDBLOCK:
#endif
            case ECONNABORTED://连接被中断
                break;
            case EMFILE://文件描述符太多.
                std::cerr<<"fd out ";
                break;
            default:
                //record
            std::cout<<"error pid="<<getpid()<<std::endl;
                break;
        }
        return;
    }
    bufferevent *bev=bufferevent_socket_new(base,connfd,BEV_OPT_CLOSE_ON_FREE);
    conns[connfd]=std::make_shared<Connection>(connfd,bev);
    if(conns[connfd]== nullptr){
        conns[connfd]->setState(CON_STATE_CLOSE);
    }
    connectionStateMachine(conns[connfd]);
    return;
}
void Worker::doRead(bufferevent *bev, void *user_data) {
    evutil_socket_t connfd = bufferevent_getfd(bev);
    conns[connfd]->setState(CON_STATE_READ);
    connectionStateMachine(conns[connfd]);
}

void Worker::doWrite(bufferevent *bev, void *user_data) {
//    std::cout<<"write finish"<<std::endl;
    evutil_socket_t connfd=bufferevent_getfd(bev);
    conns[connfd]->setState(CON_STATE_RESPONSE_END);
    connectionStateMachine(conns[connfd]);

}
void Worker::doError(bufferevent *bev, short events, void *user_data) {
    evutil_socket_t connfd=bufferevent_getfd(bev);
//    printf("fd=%u\n",connfd);
    if(events & BEV_EVENT_TIMEOUT){
        std::cerr<<"timeout";
    }else if(events &BEV_EVENT_EOF){
        std::cerr<<"connection close\n";
    }else if(events &BEV_EVENT_ERROR){
        std::cerr<<"some other error";
    }
    conns[connfd]->setState(CON_STATE_CLOSE);
    connectionStateMachine(conns[connfd]);
}

void Worker::signalSIGINT(evutil_socket_t sig, short events, void *user_data) {
    event_base *base=(event_base *)user_data;
    struct timeval delay = { 2, 0 };
    std::cout<<"Caught an interrupt signal; exiting cleanly in two seconds."<<std::endl;
    event_base_loopexit(base,&delay);
}


void Worker::connectionStateMachine(std::shared_ptr<Connection>& conn) {
    int done=0;
    while (done == 0) {
        size_t ostate = conn->getState();
        //根据当前状态机的状态进行相应的处理和状态转换。
        switch (conn->getState()) {
            case CON_STATE_REQUEST_START:{
                bufferevent_setcb(conn->getBev(), doRead,doWrite, doError, NULL);
                bufferevent_enable(conn->getBev(),EV_READ|EV_PERSIST);
            }
                break;

            case CON_STATE_READ:{
                char buf[MAXLINE+1];
                int n=-1;
                n=bufferevent_read(conn->getBev(),buf, MAXLINE);
                buf[n]='\0';
                conn->setMsg(buf);
                conn->setState(CON_STATE_REQUEST_END);
            }
                break;

            case CON_STATE_REQUEST_END:{
                if(conn->parseHttp(conn->getMsg(),conn->getMsg().length())){
                    conn->setState(CON_STATE_HANDLE_REQUEST);
                }else{
                    conn->setState(CON_STATE_ERROR);
                }

            }
                break;
            case CON_STATE_HANDLE_REQUEST:
                //do something
                conn->setState(CON_STATE_RESPONSE_START);
                break;
            case CON_STATE_RESPONSE_START:{
                //do something
                if(conn->responsePrepare()){

                    conn->setState(CON_STATE_WRITE);
                }else{
                    conn->setState(CON_STATE_ERROR);
                }

            }
                break;
            case CON_STATE_WRITE: {
                conn->writeResponse();
            }
                break;
            case CON_STATE_RESPONSE_END:{
                if(conn->isKeepAlive()){
                    conn->setState(CON_STATE_CLOSE);
                }else{
                    conn->setState(CON_STATE_CLOSE);
                }
            }
                break;
            case CON_STATE_CONNECT:
                break;
            case CON_STATE_CLOSE:{
                conn->closeConn();
                evutil_socket_t connfd=bufferevent_getfd(conn->getBev());
                if(conns[connfd]!= nullptr){
                    conns.erase(connfd);
                }

            }
                break;
            case CON_STATE_READ_POST:
                break;
            case CON_STATE_ERROR:
                conn->setState(CON_STATE_CLOSE);
                break;
            default:
                break;
        }
        if (done == -1) {
            done = 0;
        }
        else if (ostate == conn->getState()) {
            done = 1;
        }
    }
    return ;
}

Worker::~Worker() {}