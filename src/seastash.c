#include "seastash.h"

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

  server.config = config_from_file("config.lua");
  if (!server.config) {
    return -1;
  }

  server.work = chmake(msg_s*, server.config->msg_buffer_len);
  server.mps = 0;
  server_listen(&server, NULL, 5555);

  LOG("INFO: Starting %i workers routines...", server.config->num_workers);
  // TODO: store worker structs somewhere
  for (int i = 0; i < server.config->num_workers; i++) {
    worker_s* w = worker_new(&server);
    go(worker_loop(w));
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

