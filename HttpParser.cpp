//
// Created by yxc on 16-12-3.
//

#include <cstdio>
#include <iostream>
#include "HttpParser.h"

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
//    std::cout<<"111"<<std::endl;
    hht=new http_request_t;
    parser.data=hht;
    http_parser_init(&parser, HTTP_REQUEST);
    size_t nparsed = http_parser_execute(&parser, &settings,buf.c_str(), nread);
    //an error should be caught

    if(parser.upgrade){
        //websocket
        //handle new protocol
    }else if(nparsed!=nread){
        std::cout<<"parse error"<<std::endl;
        return false;
    }
//    std::cout<<"parse true"<<std::endl;
    return true;
}

/**
 * Initializes default values, counters.
 */
int HttpParser::http_message_begin_cb(http_parser* parser) {
//    std::cout<<"222"<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->header_lines = 0;
//    http_request->headers.reserve(20);
    return 0;
}

/**
 * Copies url string to http_request->url.
 */
int HttpParser::http_url_cb(http_parser* parser, const char* chunk, size_t len) {
//    std::cout<<"333"<<std::endl;
//    std::cout<<chunk<<std::endl;

    http_request_t* http_request = (http_request_t *) parser->data;
//        std::cout<<"url: "<<http_request->url<<endl;
//    std::cout<<"method: "<<http_request->method<<endl;
//    std::cout<<"header_lines="<<http_request->header_lines<<endl;

//    http_request->url = new char[len+1];
    http_request->url.assign(chunk,len);
//    strncpy(http_request->url, chunk, len);
//    http_request->url[len]='\0';
    //version
//    std::cout<<len<<std::endl;
//    std::cout<<chunk+len+1<<std::endl;
//    std::cout<<http_request->url<<std::endl;
    char *find_version=(char *)chunk+len+1;
    while(*find_version!='\r'){
        find_version++;
    }
    http_request->version.assign(chunk+len+1,find_version-chunk-len-1);
//    std::cout<<http_request->version<<std::endl;
    //找第一个换行
//    std::cout<<http_request->url<<std::endl;
    return 0;
}

/**
 * Copy the header field name to the current header item.
 */
int HttpParser::http_header_field_cb(http_parser* parser, const char* chunk, size_t len) {
//    std::cout<<"444"<<std::endl;
//    std::cout<<chunk<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->temp_field.assign(chunk,len);
//    http_header_t *header = &http_request->headers[http_request->header_lines];
//    header->field_length = len;
//    header->field.assign(chunk,len);
//    strncpy(header->field, chunk, len);
//    header->field[len]='\0';
//    std::cout<<header->field<<std::endl;
    return 0;
}

/**
 * Now copy its assigned value.
 */
int HttpParser::http_header_value_cb(http_parser* parser, const char* chunk, size_t len) {
//    std::cout<<"555"<<std::endl;
//    std::cout<<chunk<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;
    http_request->headers[http_request->temp_field].assign(chunk,len);
//    http_header_t *header = &http_request->headers[http_request->header_lines];

//    header->value_length = len;
//    header->value = new char[len+1];
//    std::cout<<header->value_length<<std::endl;
//    strncpy(header->value, chunk, len);
//    header->value.assign(chunk,len);
//    header->value[len]='\0';
//    std::cout<<header->value<<std::endl;
    ++http_request->header_lines;
    return 0;
}

/**
 * Extract the method name.
 */
int HttpParser::http_headers_complete_cb(http_parser* parser) {
//    std::cout<<"666"<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;

    const char* method = http_method_str((http_method) parser->method);

//    http_request->method = new char[strlen(method)+1];

//    strncpy(http_request->method, method, strlen(method));
    http_request->method.assign(method);
//    http_request->method[strlen(method)]='\0';
//    std::cout<<strlen(method)<<std::endl;
//    std::cout<<http_request->method<<std::endl;
//    std::cout<<http_request->header_lines<<std::endl;
    return 0;
}

/**
 * And copy the body content.
 */
int HttpParser::http_body_cb(http_parser* parser, const char* chunk, size_t len) {
//    std::cout<<"777"<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;

//    http_request->body = new char[len+1];
//    strncpy((char *) http_request->body, chunk, len);
//    ((char *)http_request->body)[len]='\0';
    http_request->body.assign(chunk,len);
//    http_request->body = chunk;
//    std::cout<<chunk<<std::endl;
    return 0;
}

/**
 * Last cb executed by http_parser.
 *
 * In our case just logs the whole request to stdou.
 */
int HttpParser::http_message_complete_cb(http_parser* parser) {
//    std::cout<<"888"<<std::endl;
    http_request_t* http_request = (http_request_t *) parser->data;

    /* now print the ordered http http_request to console */
//    std::cout<<"url: "<<http_request->url<<endl;
//    std::cout<<"method: "<<http_request->method<<endl;
//    std::cout<<"header_lines="<<http_request->header_lines<<endl;
//    std::cout<<http_request->headers[http_request->temp_field]<<endl;
//    std::cout<<"body: "<<http_request->body<<endl;
//    printf("\r\n");

    /* lets send our short http hello world response and close the socket */
    return 0;
}

void HttpParser::erasehht() {
    delete hht;
//    hht= nullptr;
}
HttpParser::~HttpParser() {
}

http_request_t *HttpParser::getHht() const {
    return hht;
}
