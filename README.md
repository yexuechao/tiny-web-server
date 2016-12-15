这是一个tiny web server.

原设想是模仿lighttpd状态机,做一个web server..

涉及到的技术:
libevent:用于网络编程方面.
http_parser:用于解析http请求.
lighttpd:把状态机作为整个项目的框架.

最终尚未解决问题:
1.状态机没有完善.
2.http_response没有完善.
3.libevent:Master主进程监听僵死进程,想要同时派生新的进程.但是event_base会被fork到子进程,由于不能够free掉,有点麻烦..
