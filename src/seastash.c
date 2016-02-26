#include "seastash.h"

config_s* config_new() {
  config_s* cfg = malloc(sizeof(config_s));
  cfg->num_workers = 2;
  cfg->msg_buffer_len = 128;
  return cfg;
}

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

coroutine void socket_loop(server_s* this, tcpsock conn) {
  char buf[16384];

  while (true) {
    size_t nbytes = tcprecvuntil(conn, buf, sizeof(buf), "\r" , 1, -1);

    if (errno) {
      if (errno == ENOBUFS) {
        LOG("ERROR: Ran out of buffer space reading from socket %p", conn);
      } else if (errno == ENOMEM) {
        LOG("ERROR: Ran out of memory reading from socket %p", conn);
      }

      goto cleanup;
    }

    msg_s* msg = msg_new(TASK);

    // Create a buffer for the new string, minus control charcter
    msg->buffer = malloc((nbytes - 1) * sizeof(char));
    strncpy(msg->buffer, buf, nbytes - 1);
    msg->buffer[nbytes - 1] = '\0';
    chs(this->work, msg_s*, msg);
  }

  cleanup:
    LOG("Closing socket %p", conn);
    tcpclose(conn);
}

coroutine void worker_loop(server_s* this) {
  while (true) {
    msg_s* msg = chr(this->work, msg_s*);

    if (msg->type == CLOSE) {
      msg_free(msg);
      return;
    }

    // LOG("I would process %i bytes: `%s`", strlen(msg->buffer), msg->buffer);
    this->mps++;
    msg_free(msg);
  }
}

coroutine void stats_loop(server_s* this) {
  while (true) {
    msleep(now() + 1000);
    if (this->mps) {
      LOG("Messages processed last second: %i", this->mps);
      this->mps = 0;
    }
  }
}

int main(int argc, char *argv[]) {
  server_s server;
  server.config = config_new();
  server.work = chmake(msg_s*, server.config->msg_buffer_len);
  server.mps = 0;
  server_listen(&server, NULL, 5555);

  for (int i = 0; i < server.config->num_workers; i++) {
    go(worker_loop(&server));
  }

  go(stats_loop(&server));

  while (true) {
    tcpsock as = server_accept(&server);

    if (!as) {
      continue;
    }

    go(socket_loop(&server, as));
  }
}

