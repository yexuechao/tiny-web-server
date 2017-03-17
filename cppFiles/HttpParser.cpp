//
// Created by yxc on 16-12-3.
//

#include <cstdio>
#include <iostream>
#include "../hFiles/HttpParser.h"
#include "../hFiles/Connection.h"
HttpParser::HttpParser():hht(NULL) {
    //绑定回调函数
    settings.on_url = http_url_cb;
    settings.on_body = http_body_cb;
    settings.on_header_field = http_header_field_cb;
    settings.on_header_value = http_header_value_cb;
    settings.on_headers_complete = http_headers_complete_cb;
    settings.on_message_begin = http_message_begin_cb;
    settings.on_message_complete = http_message_complete_cb;
}

bool HttpParser::httpRun(std::string buf,int nread) {
    hht=new http_request_t;
    parser.data=hht;
    http_parser_init(&parser, HTTP_REQUEST);
    size_t nparsed = http_parser_execute(&parser, &settings,buf.c_str(), nread);
    if(parser.upgrade){
        //websocket
        //handle new protocol
    }else if(nparsed!=nread){
        std::cout<<"parse error"<<std::endl;
        return false;
    }
    return true;
}

/**
 * Initializes default values, counters.
 */
int HttpParser::http_message_begin_cb(http_parser* parser) {
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->header_lines = 0;
    return 0;
}

/**
 * Copies url string to http_request->url.
 */
int HttpParser::http_url_cb(http_parser* parser, const char* chunk, size_t len) {
    //url
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->url.assign(chunk,len);
    //version
    char *find_version=(char *)chunk+len+1;
    while(*find_version!='\r'){
        find_version++;
    }
    http_request->version.assign(chunk+len+1,find_version-chunk-len-1);
    return 0;
}

/**
 * Copy the header field name to the current header item.
 */
int HttpParser::http_header_field_cb(http_parser* parser, const char* chunk, size_t len) {
    //header field
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->temp_field.assign(chunk,len);
    return 0;
}

/**
 * Now copy its assigned value.
 */
int HttpParser::http_header_value_cb(http_parser* parser, const char* chunk, size_t len) {
    //header value
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->headers[http_request->temp_field].assign(chunk,len);
    ++http_request->header_lines;
    return 0;
}

/**
 * Extract the method name.
 */
int HttpParser::http_headers_complete_cb(http_parser* parser) {
    http_request_t* http_request = (http_request_t *) parser->data;
    //method
    const char* method = http_method_str((http_method) parser->method);
    http_request->method.assign(method);
    return 0;
}

/**
 * And copy the body content.
 */
int HttpParser::http_body_cb(http_parser* parser, const char* chunk, size_t len) {
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->body.assign(chunk,len);
    return 0;
}

/**
 * Last cb executed by http_parser.
 *
 * In our case just logs the whole request to stdou.
 */
int HttpParser::http_message_complete_cb(http_parser* parser) {
    //入队列
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request_t new_http_request=*http_request;
    Connection::inQueue(new_http_request);
    http_request->clearThis();
    /* lets send our short http hello world response and close the socket */
    return 0;
}

void HttpParser::erasehht() {
    if(hht!=NULL){
        delete hht;
        hht=NULL;
    }


}
HttpParser::~HttpParser() {}
http_request_t *HttpParser::getHht() const {
    return hht;
}
