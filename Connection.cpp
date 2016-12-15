//
// Created by yxc on 16-11-29.
//

#include <iostream>
#include "Connection.h"
#include "ConnectionState.h"
#include "HttpParser.h"
#include <event2/buffer.h>
Connection::Connection(evutil_socket_t fd,bufferevent *bev)
        :connfd(fd),
         bev(bev),
         state(CON_STATE_REQUEST_START),
         deal_times(0),
         is_readable(0),
         request_count(0),
         msg()
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
    HttpParser hp;
    bool nparsed=hp.httpRun(http_request,buf,nread);
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
    HttpResponse hr(http_request,response);
    if (strcasecmp(http_request->method, "GET") && strcasecmp(http_request->method, "POST"))
    {
        //501
        std::cout<<"501 false"<<std::endl;
        hr.unimplemented();//don't have this method
    }
    if(strcasecmp(http_request->method, "POST") == 0){
        //post cgi
        std::cout<<"post method"<<std::endl;
        hr.setCgi(1);
        hr.postResponse();
    }
    if (strcasecmp(http_request->method, "GET") == 0) {
        std::cout<<"get method"<<std::endl;
        hr.getMethodResponse();
    }

    if(hr.getResponse_error()!=0){
        return false;
    }
    return true;
//    std::cout<<"response is :"<<std::endl<<std::endl;
//    std::cout<<"url is: "<<http_request->url<<std::endl;
//    std::cout<<"status is :"<<response.status<<std::endl;
//    std::cout<<"description is :"<<response.status_description<<std::endl;
//    std::cout<<"version is :"<<response.version<<std::endl;
//    std::map<std::string,std::string>::iterator itr=response.header.begin();
//    for(;itr!=response.header.end();itr++){
//        std::cout<<itr->first<<": "<<itr->second<<std::endl;
//    }
//    std::cout<<"content is :"<<std::endl<<response.content<<std::endl;
}

void Connection::writeResponse() {
    //组织response
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
//    evbuffer * input=bufferevent_get_input(bev);
//    evbuffer * output=bufferevent_get_output(bev);
//    std::cout<<evbuffer_get_length(input)<<std::endl;
//    std::cout<<evbuffer_get_length(output)<<std::endl;
    //重置http_request response

    response=before_response;
    msg="";
//    eraseHttpRequest();
}

void Connection::connectionStateMachine() {}

bool Connection::isKeepAlive() {
    //http
//    std::cout<<"is keep alive ?"<<std::endl;
    int i=0;
    for(i=0;i<http_request->header_lines;i++){
        if(strcasecmp(http_request->headers[i].field,"Connection")==0
           && strcasecmp(http_request->headers[i].value,"keep-alive")==0){
            break;
        }
    }

    if(i==http_request->header_lines){
        eraseHttpRequest();
//        std::cout<<"no keep alive"<<std::endl;
        return false;
    }else{
        eraseHttpRequest();
        std::cout<<"keep-alive"<<std::endl;
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

void Connection::requestStart() {
//    std::cout<<"request start"<<std::endl;
    http_request=new http_request_t;
    http_request->parser.data=http_request;
}

void Connection::eraseHttpRequest() {
//    std::cout<<"erase http_request"<<std::endl;
    if(http_request->method!=NULL){
        delete[] http_request->method;
        http_request->method=NULL;
    }

    if(http_request->url!=NULL){
        delete[] http_request->url;
        http_request->url=NULL;
    }
//    std::cout<<"erase http_request 2"<<std::endl;

    if(http_request->body!=NULL){
        delete[] http_request->body;
        http_request->body=NULL;
    }
//    std::cout<<"erase http_request 3"<<std::endl;
    //header

    for(int i=0;i<http_request->header_lines;i++){
        delete[] http_request->headers[i].field;
        http_request->headers[i].field=NULL;
        delete[] http_request->headers[i].value;
        http_request->headers[i].value=NULL;
    }
    delete http_request;
    http_request=NULL;
//    std::cout<<"erase http_request finish"<<std::endl;
    return ;
}
Connection::~Connection() {
//    std::cout<<"connection xigou"<<std::endl;
}

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

http_request_t *Connection::getHttp_request() const {
    return http_request;
}

void Connection::setHttp_request(http_request_t *http_request) {
    Connection::http_request = http_request;
}
