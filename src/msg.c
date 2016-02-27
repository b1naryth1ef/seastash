#include "msg.h"

msg_s* msg_new(msg_type_t type) {
  msg_s* msg = malloc(sizeof(msg_s));
  msg->type = type;

  return msg;
}

void msg_free(msg_s* this) {
  if (this->buffer) {
    free(this->buffer);
  }
  free(this);
}

