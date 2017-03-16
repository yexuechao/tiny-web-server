//
// Created by yxc on 16-12-3.
//

#ifndef WEBSERVER_HTTPRESPONSE_H
#define WEBSERVER_HTTPRESPONSE_H


#include "HttpParser.h"
#include <string>
#include <map>
const string target="/home/yxc/workplace/webServer/source";
const string textSource="/textSource";
struct http_response{
    std::string version;
    int status;
    std::string status_description;
    std::map<std::string,std::string> header;
    std::string content;
};

class HttpResponse {
public:
    HttpResponse();
    void unimplemented();
    ~HttpResponse();
    void postResponse();
    void getMethodResponse();
    void notFound();
    void badRequest();
    void successHeader();
    void successFile(std::ifstream &in);
    bool addIndex();
    int getCgi() const;
    void setCgi(int cgi);
    void successGetNoCgi();
    void cannotExecute();

    int getResponse_error() const;

    void setResponse_error(int response_error);

    void successLine();
    void getCgiResponse();
    //HTTP/1.1 200 OK
    //Date:
    //Server:
    //Last-Modified:
    //ETag:
    //Accept-Ranges:
    //Content-Length:
    //Connection:
    //Content-Type:
    void setHhp(const HttpParser &hhp);
    //内容
private:
    int cgi;
    HttpParser hhp;
    http_response response;
public:
    const http_response &getResponse() const;

private:
    static std::map<std::string,std::string> filetype;
    void Close(int fd);
    void Dup2(int fd1,int fd2);
    int response_error;
    http_request_t *hhp_context;
};


#endif //WEBSERVER_HTTPRESPONSE_H
