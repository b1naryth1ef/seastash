#pragma once

#include "global.h"
#include "server.h"

typedef enum {
  CLOSE,
  TASK
} msg_type_t;


typedef struct msg_t {
  msg_type_t type;
  char* buffer;
} msg_s;

typedef struct worker_t {
  server_s* server;
  lua_State *L;
} worker_s;

msg_s* msg_new(msg_type_t type);
void msg_free(msg_s* this);
worker_s* worker_new(server_s* server);
void worker_free(worker_s* w);
void worker_process_message(worker_s* this, msg_s* msg);
coroutine void worker_loop(worker_s* this);
