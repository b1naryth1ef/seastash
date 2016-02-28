#pragma once

#include "global.h"
#include "server.h"

typedef struct worker_t {
  server_s* server;
  lua_State *L;
  chan ctrl;
} worker_s;

worker_s* worker_new(server_s* server);
void worker_free(worker_s* w);
void worker_process_message(worker_s* this, msg_s* msg);
coroutine void worker_loop(worker_s* this);
