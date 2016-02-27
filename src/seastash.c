#include "seastash.h"

int main(int argc, char *argv[]) {
  server_s server;
  server.config = config_from_file("config.lua");

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

  // Setup the shared memory region for stats...
  int segment = stats_share(server.stats);
  server.stats = stats_share_get(segment);

  LOG("INFO: Starting %i workers", server.config->num_workers);

  // Start all the worker forks
  for (int i = 0; i < server.config->num_workers; i++) {
    worker_s* w = worker_new(&server);

    pid_t pid = mfork();
    if (pid < 0) {
      LOG("ERROR: Failed to fork worker");
      return 1;
    }

    // Parent process
    if (pid > 0) {
      continue;
    }

    // Spawn the coroutine that handles message processing
    go(worker_loop(w, stats_share_get(segment)));

    // And start listening for connections
    server_listen_loop(&server);
  }

  LOG("INFO: Started!");
  while (true) {
    // TODO: eventually we need shared memory and stats/logging here
    msleep(now() + 1000);
    if (server.stats->mps > 0) {
      LOG("%i/LPS", server.stats->mps);
      server.stats->mps = 0;
    }
  }
}

