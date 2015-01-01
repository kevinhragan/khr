#! /usr/bin/python
import socket
host = socket.gethostname()
print "host is ", host
ip = socket.gethostbyname(host)
print "ip is ", ip

