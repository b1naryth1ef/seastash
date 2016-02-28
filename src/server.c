#include "server.h"

stats_s* stats_new() {
  stats_s* stats = malloc(sizeof(stats_s));
  stats->total = 0;
  stats->mps = 0;
  return stats;
}

bool server_listen(server_s* this, char* addr_s, uint16_t port) {
  ipaddr addr = iplocal(addr_s, port, 0);
  this->socket = tcplisten(addr, 32);

  if (!this->socket) {
    return false;
  }

  return true;
}

void server_listen_loop(server_s* this) {
  LOG("Starting listen... %p", this->socket);
  while (this->running) {
    tcpsock as = tcpaccept(this->socket, now() + 100);

    if (!as) {
      continue;
    }

    LOG("Accepted connection %p", as);
    go(server_loop(this, as));
  }
}

coroutine void server_loop(server_s* this, tcpsock conn) {
  char buf[16384];

  while (this->running) {
    size_t nbytes = tcprecvuntil(conn, buf, sizeof(buf), this->config->network_delim, 1, now() + 100);

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
