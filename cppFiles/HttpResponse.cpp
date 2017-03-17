//
// Created by yxc on 16-12-3.
//

#include <sys/stat.h>
#include <fstream>
#include <zconf.h>
#include <wait.h>
#include <iostream>
#include "../hFiles/HttpResponse.h"

std::map<std::string,std::string> filetype={
        { "txt", "text/plain" },
        { "c", "text/plain" },
        { "h", "text/plain" },
        { "html", "text/html" },
        { "htm", "text/htm" },
        { "css", "text/css" },
        { "gif", "image/gif" },
        { "jpg", "image/jpeg" },
        { "jpeg", "image/jpeg" },
        { "png", "image/png" },
        { "pdf", "application/pdf" },
        { "ps", "application/postsript" }
};

static const std::string &contentType(string path) {
    //find final .
    std::string url(path);
    size_t found=url.find_last_of('.');
    if(found==-1 || url.substr(found+1).find('/')!=std::string::npos){
        //no extension
        return "application/misc";
    }
    std::string extension=url.substr(found+1);
    return filetype[extension];
}

HttpResponse::HttpResponse()
        :response_error(0){}

HttpResponse::~HttpResponse() {}

void HttpResponse::unimplemented() {
    //501
    response.status=501;
    response.version="HTTP/1.1";
    response.status_description="Method Not Implemented";
    response.content=
            "<HTML><HEAD><TITLE>Method Not Implemented\r\n"
            "</TITLE></HEAD>\r\n"
            "<BODY><P>HTTP request method not supported.\r\n"
            "</BODY></HTML>\r\n";
    response.header["Content-Type"]="text/html";
    return ;
}

void HttpResponse::getMethodResponse() {
    std::string query_string(hhp_context.url);
    size_t found=query_string.find_first_of('?',0);
    if(found!=std::string::npos){
        hhp_context.url=query_string.substr(0,found);
        hhp_context.query_string=query_string.substr(found+1,std::string::npos);
        hhp_context.cgi=1;
    }
    if(!addIndex()){
        notFound();
        return ;
    }
    if(hhp_context.cgi!=1){
        successGetNoCgi();
    }else{
        std::cout<<"get cgi"<<std::endl;
        getCgiResponse();
    }
}

void HttpResponse::successGetNoCgi() {
    std::ifstream in(hhp_context.url);
    if(!in.is_open()){
        std::cout<<"get no found"<<std::endl;
        notFound();
        return ;
    }
    successHeader();
    successFile(in);
    return ;
}

bool HttpResponse::addIndex() {
    struct stat st;
    std::string path=target;
    if(hhp_context.cgi!=1){
        if(hhp_context.url.at(hhp_context.url.length()-1)=='/'){
            path=path+hhp_context.url+"index.html";
        }else{
            path=path+textSource+hhp_context.url;
        }
        hhp_context.url=path;
    }else{
        path=path+hhp_context.url;
        hhp_context.url=path;
    }
    if (stat(path.c_str(), &st) == -1) {
        std::cout<<"add index return false"<<std::endl;
        return false;
    } else {
        if ((st.st_mode & S_IFMT) == S_IFDIR){
            path=path+"/index.html";
            hhp_context.url=path;
        }
//        if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
//            hhp_context->cgi=1;
//        }
        return true;
    }
}

void HttpResponse::notFound() {
    //404
    response.status=404;
    response.version=hhp_context.version;
    response.status_description="NOT FOUND";
    response.header["Content-Type"]="text/html";
    response.content=
            "<HTML><TITLE>Not Found</TITLE>\r\n"
            "<BODY><P>The server could not fulfill\r\n"
            "your request because the resource specified\r\n"
            "is unavailable or nonexistent.\r\n"
            "</BODY></HTML>\r\n";
    return ;
}

void HttpResponse::successHeader() {
    successLine();
    response.header["Content-Type"]=contentType(hhp_context.url);
    response.header["Connection"]="close";
}

void HttpResponse::successFile(std::ifstream &in) {
    in>>response.content;
    return ;
}

void HttpResponse::postResponse() {
    if(!addIndex()){
        std::cout<<"add index and not found"<<std::endl;
        notFound();
        return ;
    }
    map<string,string>::iterator itr=hhp_context.headers.find("Content-Length");
    if(itr==hhp_context.headers.end()){
        //bad request
        badRequest();
    }
    int cgi_output[2];
    int cgi_input[2];
    if (pipe(cgi_output) < 0) {
        cannotExecute();
        return;
    }
    if (pipe(cgi_input) < 0) {
        cannotExecute();
        return;
    }
    pid_t pid;
    if ( (pid = fork()) < 0 ) {
        cannotExecute();
        return;
    }
    successLine();
    if(pid==0){
        //close pipe
        Close(cgi_input[1]);
        Close(cgi_output[0]);
        Dup2(cgi_input[0],STDIN_FILENO);
        Dup2(cgi_output[1],STDOUT_FILENO);
        std::string length="CONTENT-LENGTH=";
        length=length+itr->second;
        putenv((char *) length.c_str());
        execl((hhp_context.url).c_str(), NULL);
        return;
    }else{
        Close(cgi_input[0]);
        Close(cgi_output[1]);
        write(cgi_input[1],hhp_context.body.c_str(),stoi(itr->second));
        char content[MAXSIZE+1];
        ssize_t n=-1;
        n=read(cgi_output[0],content,MAXSIZE);
        content[n]='\0';
        response.content.assign(content);
        Close(cgi_output[0]);
        Close(cgi_input[1]);
        waitpid(pid,NULL, 0);
    }

}

void HttpResponse::successLine() {
    response.version=hhp_context.version;
    response.status=200;
    response.status_description="OK";
    return ;
}

void HttpResponse::cannotExecute() {
    response.status=500;
    response.version=hhp_context.version;
    response.status_description="Server error";
    response.header["Content-Type"]="text/html";
    response.content=
            "<h1>Error prohibited CGI execution</h1>\r\n";
}

void HttpResponse::badRequest() {
    response.status=400;
    response.version=hhp_context.version;
    response.status_description="BAD REQUEST";
    response.header["Content-Type"]="text/html";
    response.content=
            "<h1>Your browser sent a bad request,"
            "such as a POST without a Content-Length.</h1>\r\n";
    return;
}

void HttpResponse::getCgiResponse() {
    int cgi_output[2];
    int cgi_input[2];
    if (pipe(cgi_output) < 0) {
        cannotExecute();
        return;
    }
    if (pipe(cgi_input) < 0) {
        cannotExecute();
        return;
    }
    pid_t pid;
    if ( (pid = fork()) < 0 ) {
        cannotExecute();
        return;
    }
    successLine();

    if(pid==0){
        close(cgi_output[0]);
        close(cgi_input[1]);
        dup2(cgi_output[1], STDOUT_FILENO);
        dup2(cgi_input[0], STDIN_FILENO);
        std::string query_string="QUERY_STRING=";
        query_string=query_string+hhp_context.query_string;
        putenv((char *) query_string.c_str());
        execl(hhp_context.url.c_str(), NULL);
        return ;
    }else{
        close(cgi_output[1]);
        close(cgi_input[0]);
        char content[MAXSIZE+1];
        ssize_t n=-1;
        n=read(cgi_output[0],content,MAXSIZE);
        content[n]='\0';
        response.content.assign(content);
        Close(cgi_output[0]);
        Close(cgi_input[1]);
        waitpid(pid,NULL, 0);
    }
}

void HttpResponse::Close(int fd) {
    if(-1==close(fd)){
        std::cerr<<"close error"<<std::endl;
        response_error=1;
    }
}

void HttpResponse::Dup2(int fd1, int fd2) {
    if(-1==dup2(fd1,fd2)){
        std::cerr<<"dup2 error"<<std::endl;
        response_error=1;
    }
}

int HttpResponse::getResponse_error() const {
    return response_error;
}

void HttpResponse::setResponse_error(int response_error) {
    HttpResponse::response_error = response_error;
}



http_response &HttpResponse::getResponse(){
    return response;
}

http_request_t HttpResponse::getHhp_context() const {
    return hhp_context;
}

void HttpResponse::setHhp_context(http_request_t hhp_context) {
    HttpResponse::hhp_context = hhp_context;
}
