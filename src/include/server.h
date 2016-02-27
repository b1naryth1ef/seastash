#pragma once

#include "global.h"
#include "config.h"

typedef struct server_t {
  config_s* config;
  tcpsock socket;

  chan work;

  uint64_t mps;
} server_s;

bool server_listen(server_s* this, char* addr_s, uint16_t port);
tcpsock server_accept(server_s* this);
