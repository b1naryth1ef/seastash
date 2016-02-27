import sys
import time
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.connect(('localhost', 5555))

i = 1

if len(sys.argv) > 1:
    wait = float(sys.argv[1])
else:
    wait = 0

try:
    while True:
        sock.sendall("TEST %s!\r" % i)
        i += 1
        if wait: time.sleep(wait)
except KeyboardInterrupt:
    sock.close()
