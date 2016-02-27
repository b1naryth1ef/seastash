#pragma once

#include "global.h"
#include "config.h"
#include "msg.h"

typedef struct {
  uint64_t total;
  uint64_t mps;
} stats_s;

stats_s* stats_new();

typedef struct {
  config_s* config;
  tcpsock socket;

  chan work;

  stats_s* stats;
} server_s;

bool server_listen(server_s* this, char* addr_s, uint16_t port);
tcpsock server_accept(server_s* this);
void server_listen_loop(server_s* this);
coroutine void server_loop(server_s* this, tcpsock conn);
