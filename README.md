#前言
这是一个tiny web server.

原设想是模仿lighttpd状态机,做一个web server..

涉及到的技术:</br>
libevent:用于网络编程方面.</br>
http_parser:用于解析http请求.</br>
lighttpd:把状态机作为整个项目的框架.
#未解决问题
最终尚未解决问题:</br>
1.状态机没有完善.</br>
2.http_response没有完善.</br>
3.libevent:Master主进程监听僵死进程,想要同时派生新的进程.但是event_base会被fork到子进程,由于不能够free掉,有点麻烦.</br>
4.当然还有很多其他很复杂的问题,定时器,代理...I am just tiny..

#进入项目部分:

这个项目模仿lighttpd,Master进程和Worker进程分离.进程池+epoll模式.</br>

main.cpp</br>
Master master("127.0.0.1",9000,1);</br>
分别是服务器ip,端口号,派生Worker子进程数量.</br>
Master只需要fork和监听信号.</br>
Worker监听listenfd可读,还有connfd可读.
##各文件作用
Connection.cpp:每一个连接就是一个Connection</br>
ConnectionState.cpp:状态机种类</br>
HttpParser.cpp:http_parser的文件</br>
HttpResponse.cpp:http响应类</br>
Worker.cpp:Master fork几个,就有几个Worker.cpp</br>
http_parser.cpp:http解析类,用http_parser解析.</br>

关于http_parser的用法,看下面两个github就很清楚了:</br>
http_parser是什么:https://github.com/nodejs/http-parser</br>
http_parser怎么用:https://github.com/bodokaiser/libuv-webserver

关于libevent的话,下载源码后,看里面sample.</br>
还有就是看文档.</br>
还有看下面这个链接,入门飞快.</br>
http://www.tuicool.com/articles/uYVzua(libevent使用例子从简单到复杂)

等了解多一点lighttpd和http后,在完善这个项目.
