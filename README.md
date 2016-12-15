这是一个tiny web server.

原设想是模仿lighttpd状态机,做一个web server..

涉及到的技术:
libevent:用于网络编程方面.
http_parser:用于解析http请求.
lighttpd:把状态机作为整个项目的框架.

最终尚未解决问题:
1.状态机没有完善.
2.http_response没有完善.
3.libevent:Master主进程监听僵死进程,想要同时派生新的进程.但是event_base会被fork到子进程,由于不能够free掉,有点麻烦.
4.当然还有很多其他很复杂的问题,定时器,代理...I am just tiny..

进入项目部分:
这个项目模仿lighttpd,Master进程和Worker进程分离.进程池+epoll模式.

main.cpp
Master master("127.0.0.1",9000,1);
分别是服务器ip,端口号,派生Worker子进程数量.

Master只需要fork和监听信号.

Worker监听listenfd可读,还有connfd可读.

Connection.cpp:每一个连接就是一个Connection
ConnectionState.cpp:状态机种类
HttpParser.cpp:http_parser的文件
HttpResponse.cpp:http响应类
Worker.cpp:Master fork几个,就有几个Worker.cpp
http_parser.cpp:http解析类,用http_parser解析.

关于http_parser的用法,看下面两个github就很清楚了:
https://github.com/nodejs/http-parser
http_parser是什么.
https://github.com/bodokaiser/libuv-webserver
http_parser怎么用

关于libevent的话,下载源码后,看里面sample.
还有就是看文档.
还有看下面这个链接,入门飞快.
http://www.tuicool.com/articles/uYVzua(libevent使用例子从简单到复杂)

等了解多一点lighttpd和http后,在完善这个项目.




