#!/usr/bin/env python
import sys
import time
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 5555))

def send(i):
    sock.sendall('<13>1 2016-02-28T07:15:40.091451-06:00 eos andrei - - [timeQuality tzKnown="1" isSynced="0"] THIS IS A TEST #%i\n' % i)

i = 0
while True:
    i += 1
    if (i % 100) == 0:
        print 'send'
    time.sleep(.001)
    send(i)
