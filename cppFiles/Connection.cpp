//
// Created by yxc on 16-11-29.
//

#include <iostream>
#include "../hFiles/Connection.h"
#include "../hFiles/ConnectionState.h"
#include <event2/buffer.h>

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
    hr_current.setHhp(hp_current);

    if (hp_current.getHht()->method!="GET" && hp_current.getHht()->method!="POST")
    {

        std::cout<<"501 false"<<std::endl;
        hr_current.unimplemented();//don't have this method
    }
    if(hp_current.getHht()->method=="POST"){
        std::cout<<"post method"<<std::endl;
        hr_current.setCgi(1);
        hr_current.postResponse();
    }
    if (hp_current.getHht()->method=="GET") {
        hr_current.getMethodResponse();
    }

    if(hr_current.getResponse_error()!=0){
        return false;
    }
    return true;
}

void Connection::writeResponse() {
    //组织response
    http_response response=hr_current.getResponse();
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
    hp_current.erasehht();
}


bool Connection::isKeepAlive() {
    http_request_t *hht=hp_current.getHht();
    int i=0;
    for(i=0;i<hht->header_lines;i++){
    }

    if(i==hht->header_lines){
        return false;
    }else{
        return true;
    }
}




void Connection::closeConn() {
    evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
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
