#include "server.h"

bool server_listen(server_s* this, char* addr_s, uint16_t port) {
  ipaddr addr = iplocal(addr_s, port, 0);
  this->socket = tcplisten(addr, 32);

  if (!this->socket) {
    return false;
  }

  return true;
}

tcpsock server_accept(server_s* this) {
  return tcpaccept(this->socket, -1);
}

