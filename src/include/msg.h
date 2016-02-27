#pragma once

#include "global.h"

typedef enum {
  CLOSE,
  TASK
} msg_type_t;

typedef struct msg_t {
  msg_type_t type;
  char* buffer;
} msg_s;

msg_s* msg_new(msg_type_t type);
void msg_free(msg_s* this);
