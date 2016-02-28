#include "seastash.h"

server_s server;
worker_s* workers[4096];

coroutine void stats_loop(server_s* server) {
  while (true) {
    if (server->stats->mps > 0) {
      LOG("INFO: %i messages per second", server->stats->mps);
      server->stats->mps = 0;
    }
    msleep(now() + 1000);
  }
}

void signal_handle(int signo) {
  if (signo == SIGINT || signo == SIGTERM) {
    LOG("INFO: Server shutting down from signal");
    // TODO: this segfaults things, would be nice to properly cleanup
    //for (int i = 0; workers[i]; i++) {
    //  chs(workers[i]->ctrl, int, 0);
    //}
    server.running = false;
  } else if (signo == SIGUSR1) {
    // TODO: reload workers
  }
}

bool signal_bind_all() {
  if (signal(SIGUSR1, signal_handle) == SIG_ERR) {
    return false;
  }

  if (signal(SIGINT, signal_handle) == SIG_ERR) {
    return false;
  }

  if (signal(SIGTERM, signal_handle) == SIG_ERR) {
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  if (!signal_bind_all()) {
    LOG("ERROR: Failed to bind signal handlers");
    return 1;
  }

  server.running = true;
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
    workers[i] = worker_new(&server);
    go(worker_loop(workers[i]));
  }

  go(stats_loop(&server));

  server_listen_loop(&server);
}

