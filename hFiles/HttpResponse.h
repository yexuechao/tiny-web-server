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
const int MAXSIZE=4096;
struct http_response{
    std::string version;
    int status;
    std::string status_description;
    std::map<std::string,std::string> header;
    std::string content;
    void clearThis(){
        version.clear();
        status=0;
        status_description.clear();
        header.clear();
        content.clear();
    }

};

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();
    //get
    void getMethodResponse();
    void successGetNoCgi();
    //post
    void postResponse();
    void getCgiResponse();
    //error
    void unimplemented();
    void notFound();
    void badRequest();
    void cannotExecute();
    //success
    void successHeader();
    void successFile(std::ifstream &in);
    void successLine();
    //other
    bool addIndex();
    int getResponse_error() const;
    void setResponse_error(int response_error);
    http_response &getResponse();
    http_request_t getHhp_context() const;
    void setHhp_context(http_request_t hhp_context);
private:
    http_response response;
    const static std::map<std::string,std::string> filetype;
    int response_error;
    http_request_t hhp_context;
private:
    void Close(int fd);
    void Dup2(int fd1,int fd2);
    HttpResponse(const HttpResponse&){}
    HttpResponse &operator =(const HttpResponse&){}
};


#endif //WEBSERVER_HTTPRESPONSE_H
