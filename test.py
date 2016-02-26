import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.connect(('localhost', 5555))

i = 1

try:
    while True:
        sock.sendall("THIS IS A TEST LOG LINE %s!\r" % i)
        i += 1
except KeyboardInterrupt:
    sock.close()
