//
// Created by yxc on 16-12-3.
//

#ifndef WEBSERVER_HTTPPARSER_H
#define WEBSERVER_HTTPPARSER_H

#include <cstring>
#include <map>
#include <vector>
#include "../httpParser/http_parser.h"
using namespace std;
/**
 * Represents a single http header.
 */
//struct http_header_t{
//    string field;
//    string value;
//    size_t field_length;
//    size_t value_length;
//    ~http_header_t(){
//        cout<<"header free"<<endl;
//    }
//} ;

const int MAX_HTTP_HEADERS=20;
/**
 * Represents a http request with internal dependencies.
 *
 * - write request for sending the response
 * - reference to tcp socket as write stream
 * - instance of http_parser parser
 * - string of the http url
 * - string of the http method
 * - amount of total header lines
 * - http header array
 * - body content
 */
struct http_request_t{
    string url;
    string method;
    int header_lines;
    map<string,string> headers;
    string temp_field;
    string body;
    string version;
    string query_string;
    int cgi;
    http_request_t():header_lines(0),cgi(0){}
    void clearThis(){
        url.clear();
        method.clear();
        header_lines=0;
        headers.clear();
        temp_field.clear();
        body.clear();
        version.clear();
        query_string.clear();
        cgi=0;
    }

    void setCgi(int cgi) {
        http_request_t::cgi = cgi;
    }
};

class HttpParser {
public:
    HttpParser();
    bool httpRun(std::string buf,int nread);

/**
 * Executed at begin of message.
 */
    static int http_message_begin_cb(http_parser* parser);

/**
 * Executed when parsed the url.
 */
    static int http_url_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed on each header field.
 */
    static int http_header_field_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed on each header value.
 */
    static int http_header_value_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Executed when completed header parsing.
 */
    static int http_headers_complete_cb(http_parser* parser);

/**
 * Executed on body
 */
    static int http_body_cb(http_parser* parser, const char* chunk, size_t len);

/**
 * Is executed when request fully parsed.
 * User can read all request options from "&parser->data.request".
 */
    static int http_message_complete_cb(http_parser* parser);

    void erasehht();
    ~HttpParser();
    http_request_t *getHht() const;

private:
    http_parser_settings settings;
    http_parser parser;
    http_request_t *hht;
private:
    HttpParser(const HttpParser &){}
    HttpParser &operator =(const HttpParser&){}
};


#endif //WEBSERVER_HTTPPARSER_H
