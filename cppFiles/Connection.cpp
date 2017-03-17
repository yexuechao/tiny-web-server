//
// Created by yxc on 16-11-29.
//

#include <iostream>
#include "../hFiles/Connection.h"
#include "../hFiles/ConnectionState.h"
#include <event2/buffer.h>
queue<http_request_t> Connection::q_http_request=queue<http_request_t>();
Connection::Connection(evutil_socket_t fd,bufferevent *bev)
        :connfd(fd),
         bev(bev),
         state(CON_STATE_REQUEST_START)
{}

void Connection::setState(int state) {
    Connection::state = state;
}

bufferevent *Connection::getBev() const {
    return bev;
}

void Connection::setBev(bufferevent *bev) {
    Connection::bev = bev;
}

bool Connection::parseHttp(std::string buf,int nread) {
    bool nparsed=hp_current.httpRun(buf,nread);
    if(!nparsed){
        std::cerr<<"header error"<<std::endl;
        return false;
    }
    return true;
}

bool Connection::responsePrepare() {
    while(!q_http_request.empty()){
        hr_current.setHhp_context(q_http_request.front());
        q_http_request.pop();
        if (hr_current.getHhp_context().method!="GET" && hr_current.getHhp_context().method!="POST"){
            std::cout<<"501 false"<<std::endl;
            hr_current.unimplemented();
        }else if(hr_current.getHhp_context().method=="POST"){
            hr_current.getHhp_context().setCgi(1);
            hr_current.postResponse();
        }else if (hr_current.getHhp_context().method=="GET"){
            hr_current.getMethodResponse();
        }
        if(hr_current.getResponse_error()!=0){
            Connection::q_http_request=queue<http_request_t>();
            return false;
        }
        //入队列
        out_queue.push(hr_current.getResponse());
        hr_current.getResponse().clearThis();
    }
    return true;
}

void Connection::writeResponse() {
    //组织response
    while(!out_queue.empty()){
        http_response response=out_queue.front();
        out_queue.pop();
        std::string wr;
        //version
        wr.append(response.version+" ");
        //status
        char num[10];
        sprintf(num,"%d \0",response.status);
        wr.append(num);
        //description
        wr.append(response.status_description+"\r\n");
        //header
        std::map<std::string,std::string>::iterator itr=response.header.begin();
        for(;itr!=response.header.end();itr++){
            wr.append(itr->first+": ");
            wr.append(itr->second+"\r\n");
        }
        wr.append("\r\n");
        wr.append(response.content+"\r\n\r\n");
        bufferevent_write(bev,wr.c_str(),wr.length());
    }

    hp_current.erasehht();
}


bool Connection::isKeepAlive    () {
    return false;
}
void Connection::closeConn() {
    evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0 && out_queue.empty()) {
        bufferevent_free(bev);
    }
    return ;
}

Connection::~Connection() {}

int Connection::getState() const {
    return state;
}

const std::string &Connection::getMsg() const {
    return msg;
}

void Connection::setMsg(const std::string &msg) {
    Connection::msg = msg;
}

void Connection::inQueue(const http_request_t &hrt) {
    q_http_request.push(hrt);
}
