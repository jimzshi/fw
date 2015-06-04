fw
==

zks framework.

- simfcgi:
A C++ friendly fcgi frame work. It's very easy to write a RESTful service using it. Setting up a WebServer(lighttpd/apache/nginx) + simfcgi service is within-hour work to provide RESTful interface through HTTP(S).
It's multi-threaded (adjustable) hence quite efficient. 
- simproto
A small C++ fw to start-up a WebSocket service in no time. As simfcgi, it's simple but efficient, using both ASIO and Google ProtoBuffer to maximize the performance.

