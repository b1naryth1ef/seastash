#include "seastash.h"

coroutine void stats_loop(server_s* server) {
  while (true) {
    if (server->stats->mps > 0) {
      LOG("INFO: %i messages per second", server->stats->mps);
      server->stats->mps = 0;
    }
    msleep(now() + 1000);
  }
}

int main(int argc, char *argv[]) {
  server_s server;
  server.config = config_from_file("config.lua");
  server.stats = stats_new();

  // Make sure the config loaded OK
  if (!server.config) {
    return 1;
  }

  server.work = chmake(msg_s*, server.config->msg_buffer_len);

  // Start listening
  if (!server_listen(&server, NULL, 5555)) {
    LOG("ERROR: Failed to bind server.");
    return 1;
  }

  LOG("INFO: Starting %i workers", server.config->num_workers);

  // Start all the worker forks
  for (int i = 0; i < server.config->num_workers; i++) {
    worker_s* w = worker_new(&server);
    go(worker_loop(w));
  }

  go(stats_loop(&server));

  server_listen_loop(&server);
}

