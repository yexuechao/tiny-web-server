//
// Created by yxc on 16-11-29.
//

#include <iostream>
#include "Connection.h"
#include "ConnectionState.h"
#include <event2/buffer.h>
#include <zconf.h>

Connection::Connection(evutil_socket_t fd,bufferevent *bev)
        :connfd(fd),
         bev(bev),
         state(CON_STATE_REQUEST_START),
         deal_times(0),
         is_readable(0),
         request_count(0)
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
        //handle error ,close the connection
        //exit loop
        std::cerr<<"header error"<<std::endl;
//        bufferevent_free(bev);
        return false;
    }
    return true;
}

bool Connection::responsePrepare() {
    //get or post
    hr_current.setHhp(hp_current);

    if (hp_current.getHht()->method!="GET" && hp_current.getHht()->method!="POST")
    {
        //strcasecmp(http_request->method, "GET") && strcasecmp(http_request->method, "POST")
        //501
        std::cout<<"501 false"<<std::endl;
        hr_current.unimplemented();//don't have this method
    }
    if(hp_current.getHht()->method=="POST"){
        //post cgi
        //strcasecmp(http_request->method, "POST") == 0
        std::cout<<"post method"<<std::endl;
        hr_current.setCgi(1);
        hr_current.postResponse();
    }
    if (hp_current.getHht()->method=="GET") {
        //strcasecmp(http_request->method, "GET") == 0
//        std::cout<<"get method"<<std::endl;
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
    //if has body
    wr.append("\r\n");
    wr.append(response.content+"\r\n\r\n");
    //write to client
//    std::cout<<wr<<std::endl;
//    std::cout<<wr.length()<<std::endl;
//    evutil_socket_t fd=bufferevent_getfd(bev);
//    write(fd,wr.c_str(),wr.length());
    bufferevent_write(bev,wr.c_str(),wr.length());
    hp_current.erasehht();
//    evbuffer * input=bufferevent_get_input(bev);
//    evbuffer * output=bufferevent_get_output(bev);
//    std::cout<<evbuffer_get_length(input)<<std::endl;
//    std::cout<<evbuffer_get_length(output)<<std::endl;
    //重置http_request response
//    eraseHttpRequest();
}


bool Connection::isKeepAlive() {
    //http
//    std::cout<<"is keep alive ?"<<std::endl;
    http_request_t *hht=hp_current.getHht();
    int i=0;
    for(i=0;i<hht->header_lines;i++){
//        if(hht->headers[i].field=="Connection" && hht->headers[i].value=="keep-alive"){
//            break;
//        }
    }

    if(i==hht->header_lines){
//        std::cout<<"no keep alive"<<std::endl;
        return false;
    }else{
//        std::cout<<"keep-alive"<<std::endl;
        return true;
    }
}



void Connection::addRequestCount() {
    request_count++;
}

void Connection::closeConn() {
    evbuffer *output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
//        printf("flushed answer\n");
        bufferevent_free(bev);
    }
    return ;
}

Connection::~Connection() {}

int Connection::getState() const {
    return state;
}

int Connection::getRequest_count() const {
    return request_count;
}

void Connection::setRequest_count(int request_count) {
    Connection::request_count = request_count;
}

const std::string &Connection::getMsg() const {
    return msg;
}

void Connection::setMsg(const std::string &msg) {
    Connection::msg = msg;
}
