#include "seastash.h"

server_s server;

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

void on_data_bound(struct Reactor* reactor, int fd) {
  server_on_data(&server, fd);
}

int main(int argc, char *argv[]) {
  if (!signal_bind_all()) {
    LOG("ERROR: Failed to bind signal handlers");
    return 1;
  }

  struct Reactor r = {.on_data = &on_data_bound, .maxfd = 1024};
  server.running = true;
  server.config = config_from_file("config.lua");
  server.stats = stats_new();
  server.r = &r;
  server.L = server.config->L;

  // Make sure the config loaded OK
  if (!server.config) {
    return 1;
  }

  // Start listening
  if (!server_listen(&server, "5555")) {
    LOG("ERROR: Failed to bind server.");
    return 1;
  }

  // If we're in debug mode, start the stats thread
  if (server.config->debug) {
    pthread_t tid;
    pthread_create(&tid, NULL, &stats_thread_pf, (void*)server.stats);
  }

  LOG("INFO: Started server");
  server_loop(&server);
}

