#pragma once

#include <libmill.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct server_t {
  tcpsock socket;
} server_s;

bool server_listen(server_s* this, char* addr_s, uint16_t port) {
  ipaddr addr = iplocal(addr_s, port, 0);
  this->socket = tcplisten(addr, 32);

  if (!this->socket) {
    return false;
  }

  return true;
}
