#pragma once

#include "global.h"
#include "config.h"
#include "libreact.h"

typedef struct {
  uint64_t total;
  uint64_t mps;
} stats_s;

stats_s* stats_new();
void stats_thread(stats_s* this);
void* stats_thread_pf(void* this);

typedef struct {
  config_s* config;
  int socket;
  struct Reactor* r;

  lua_State *L;
  stats_s* stats;
  bool running;
} server_s;

bool server_listen(server_s* this, char* hoststring);
void server_loop(server_s* this);
void server_on_data(server_s* this, int fd);
void server_process_message(server_s* this, char* buffer);
