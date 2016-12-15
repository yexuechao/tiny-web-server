//
// Created by yxc on 16-12-3.
//

#include <sys/stat.h>
#include <fstream>
#include <zconf.h>
#include <wait.h>
#include <iostream>
#include "HttpResponse.h"
#include "HttpParser.h"

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

//static const struct filetype {
//    const std::string &extension;
//    const std::string &content_type;
//} file_type[] = {
//        { "txt", "text/plain" },
//        { "c", "text/plain" },
//        { "h", "text/plain" },
//        { "html", "text/html" },
//        { "htm", "text/htm" },
//        { "css", "text/css" },
//        { "gif", "image/gif" },
//        { "jpg", "image/jpeg" },
//        { "jpeg", "image/jpeg" },
//        { "png", "image/png" },
//        { "pdf", "application/pdf" },
//        { "ps", "application/postsript" },
//        { "", ""},
//};


static const std::string &contentType(const char *path) {
    //find final .
    std::string url(path);
    size_t found=url.find_last_of('.');
    if(found==-1 || url.substr(found+1).find('/')!=std::string::npos){
        //no extension
        return "application/misc";
    }
    std::string extension=url.substr(found+1);
    return filetype[extension];
//    const filetype *alltype;
//    for(alltype=&file_type[0];!alltype->extension.empty();++alltype){
//        if(alltype->extension==extension){
//            return alltype->content_type;
//        }
//    }
}

HttpResponse::HttpResponse(http_request_t *http_request,http_response &response)
        :cgi(0),http_request(http_request),response(response),response_error(0)
{}

HttpResponse::~HttpResponse() {

}

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
    std::string query_string(http_request->url);
    size_t found=query_string.find_first_of('?',0);
    if(found!=std::string::npos){
        //cgi
        http_request->query_string=query_string.substr(0,found+1);
        http_request->url=(char *)query_string.substr(found,std::string::npos).c_str();
    }
    //+index.html if is a dir or /
    if(!addIndex()){
//        std::cout<<"add index and not found"<<std::endl;
        notFound();
        return ;
    }
    //cgi or not?
    if(cgi!=1){
//        std::cout<<"get no cgi"<<std::endl;
        successGetNoCgi();
    }else{
        std::cout<<"get cgi"<<std::endl;
        getCgiResponse();
    }
}

void HttpResponse::successGetNoCgi() {
    //find file
//    std::cout<<http_request->url<<std::endl;
    std::ifstream in(http_request->url);
    if(!in.is_open()){
        //not found
        std::cout<<"get no found"<<std::endl;
        notFound();
        return ;
    }
    successHeader();
    successFile(in);
    return ;
}

bool HttpResponse::addIndex() {
    //url
//    std::cout<<"add index"<<std::endl;
    struct stat st;
    std::string path="/home/yxc/workplace/testWeb";
    path=path+http_request->url;
//    std::cout<<http_request->url<<std::endl;
    if(path.at(path.length()-1)=='/'){
        path=path+"index.html";
        delete[] http_request->url;
        http_request->url=NULL;
        http_request->url=new char[path.length()+1];
        strncpy(http_request->url,path.c_str(),path.length());
        http_request->url[path.length()]='\0';
    }

    if (stat(path.c_str(), &st) == -1) {
        std::cout<<"add index return false"<<std::endl;
        return false;
    } else {
        if ((st.st_mode & S_IFMT) == S_IFDIR){
            path=path+"/index.html";
            delete []http_request->url;
            http_request->url=NULL;
            http_request->url=new char[path.length()+1];
            strncpy(http_request->url,path.c_str(),path.length());
            http_request->url[path.length()]='\0';
        }
        if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH)){
            cgi=1;
        }
//        std::cout<<http_request->url<<std::endl;
        return true;
    }

}

void HttpResponse::notFound() {
    //404
    response.status=404;
    response.version=http_request->version;
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
//    std::cout<<"success header"<<std::endl;
    successLine();
    response.header["Content-Type"]=contentType(http_request->url);
}

void HttpResponse::successFile(std::ifstream &in) {
//    std::cout<<"success file"<<std::endl;
    in>>response.content;
    return ;
}

void HttpResponse::postResponse() {
    //bad request
    if(!addIndex()){
        std::cout<<"add index and not found"<<std::endl;
        notFound();
        return ;
    }
    int i=0;
    for(i=0;i<http_request->header_lines;i++){
        if(strcasecmp(http_request->headers[i].field, "Content-Length") == 0){
            break;
        }
    }
    if(i==http_request->header_lines){
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
        length=length+http_request->headers[i].value;
        putenv((char *) length.c_str());
        execl(http_request->url, NULL);
        return;
    }else{
        Close(cgi_input[0]);
        Close(cgi_output[1]);
        write(cgi_input[1],http_request->method,atoi(http_request->headers[i].value));
        //...read
        std::string content;
        while (read(cgi_output[0],(char *)content.c_str(),4096) > 0){
            response.content.append(content);
        }
        Close(cgi_output[0]);
        Close(cgi_input[1]);
        waitpid(pid,NULL, 0);
    }

}

void HttpResponse::successLine() {
//    std::cout<<"success line"<<std::endl;
    response.version=http_request->version;
    response.status=200;
    response.status_description="OK";
    return ;
}

void HttpResponse::cannotExecute() {
    response.status=500;
    response.version=http_request->version;
    response.status_description="Server error";
    response.header["Content-Type"]="text/html";
    response.content=
            "<h1>Error prohibited CGI execution</h1>\r\n";
}

void HttpResponse::badRequest() {
    response.status=400;
    response.version=http_request->version;
    response.status_description="BAD REQUEST";
    response.header["Content-Type"]="text/html";
    response.content=
            "<h1>Your browser sent a bad request,"
            "such as a POST without a Content-Length.</h1>\r\n";
    return;
}

void HttpResponse::getCgiResponse() {
    int cgi_pipe[2];
    if (pipe(cgi_pipe) < 0) {
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
        Close(cgi_pipe[0]);
        Dup2(cgi_pipe[1],STDOUT_FILENO);
        std::string query_string="QUERY_STRING=";
        query_string=query_string+http_request->query_string;
        putenv((char *) query_string.c_str());
        execl(http_request->url, NULL);
        return ;
    }else{
        Close(cgi_pipe[1]);
        std::string content;
        while (read(cgi_pipe[0],(char *)content.c_str(),4096) > 0){
            response.content.append(content);
        }
        Close(cgi_pipe[0]);
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

int HttpResponse::getCgi() const {
    return cgi;
}

void HttpResponse::setCgi(int cgi) {
    HttpResponse::cgi = cgi;
}

int HttpResponse::getResponse_error() const {
    return response_error;
}

void HttpResponse::setResponse_error(int response_error) {
    HttpResponse::response_error = response_error;
}
