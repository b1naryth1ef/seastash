#!/usr/bin/env python
import sys
import time
import socket
import thread

if len(sys.argv) < 3:
    print "Usage: ./test.py <clients> <mps>"
    sys.exit(1)

clients = int(sys.argv[1])
mps = int(sys.argv[2])

MPS = 0

def client():
    global MPS
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 5555))

    i = 0
    while True:
        i += 1
        sock.sendall('<13>1 2016-02-28T07:15:40.091451-06:00 eos andrei - - [timeQuality tzKnown="1" isSynced="0"] THIS IS A TEST #%i\n' % i)
        if mps: time.sleep(1.0 / mps)
        MPS += 1

for _ in range(clients):
    print "Spawning thread..."
    thread.start_new_thread(client, ())

while True:
    time.sleep(1)
    print "MPS: %s" % MPS
    MPS = 0

