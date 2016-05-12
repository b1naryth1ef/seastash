#include "server.h"

stats_s* stats_new() {
  stats_s* stats = malloc(sizeof(stats_s));
  stats->total = 0;
  return stats;
}

void* stats_thread_pf(void* this) {
  stats_thread((stats_s*) this);
}

// Yes, technically this is not thread safe. But read only so *shrug*
void stats_thread(stats_s* this) {
  int last = 0;

  while (true) {
    // Copy the value
    int value = this->total;

    if (value > last) {
      LOG("MPS: %i", value - last);
      last = value;
    }

    sleep(1);
  }
}

bool server_listen(server_s* this, char* hoststring) {
  // Setup address information
  struct addrinfo hints;
  struct addrinfo* servinfo;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, hoststring, &hints, &servinfo)) {
    LOG("ERROR: failed to getaddrinfo");
    return false;
  }

  // Create socket
  this->socket = socket(servinfo->ai_family,
      servinfo->ai_socktype,
      servinfo->ai_protocol);

  if (this->socket <= 0) {
    LOG("ERROR: failed to create socket");
    freeaddrinfo(servinfo);
    return false;
  }

  // set SO_RESUSEADDR
  int optval = 1;
  setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  // Bind socket
  if (bind(this->socket, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
    LOG("ERROR: failed to bind socket");
    freeaddrinfo(servinfo);
    return false;
  }

  static int flags;
  if (-1 == (flags = fcntl(this->socket, F_GETFL, 0))) {
    flags = 0;
  }
  fcntl(this->socket, F_SETFL, flags | O_NONBLOCK);

  // TODO: error check
  listen(this->socket, SOMAXCONN);
  freeaddrinfo(servinfo);

  reactor_init(this->r);
  reactor_add(this->r, this->socket);
  return true;
}

void server_loop(server_s* this) {
  // Busy loop
  while (reactor_review(this->r) >= 0) {

  }
  LOG("quit");
  // TODO: error check
}

void server_on_data(server_s* this, int fd) {
  if (fd == this->socket) {
    int client = 0;
    unsigned int len = 0;
    while ((client = accept(fd, NULL, &len)) > 0) {
      reactor_add(this->r, client);
    }
  } else {
    char buff[8094];
    ssize_t len;
    if ((len = recv(fd, buff, 8094, 0)) > 0) {
      // Termiante buffer and strip control character
      buff[len - 1] = '\0';
      server_process_message(this, buff);
    }
  }
}

// Attempts to fully process a message through all steps
void server_process_message(server_s* this, char* buffer) {
  config_step_s* step;
  bool cont = false;
  int table_ref;

  // Run all the steps in order, encapsilating failures within the step
  for (int i = 0; this->config->steps[i]; i++) {
    step = this->config->steps[i];

    // Run all the filters in order, skipping the step if a single one fails to match
    for (int fi = 0; step->filters[fi]; fi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->filters[fi]);
      lua_pushstring(this->L, buffer);
      lua_call(this->L, 1, 1);

      if (!lua_isboolean(this->L, -1)) {
        LOG("WARNING: filter did not return a bool, skipping step");
        lua_pop(this->L, 1);
        break;
      }

      cont = lua_toboolean(this->L, -1);
      lua_pop(this->L, 1);
      if (!cont) {
        break;
      }
    }

    // If the filter doesnt match, move on to next step
    if (!cont) {
      continue;
    }

    // Run all the parsers in order
    for (int pi = 0; step->parsers[pi]; pi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->parsers[pi]);

      // For the first iteration, create a blank table. Subsequently we need
      // to grab the table from the registry.
      if (pi == 0) {
        lua_newtable(this->L);
      } else {
        lua_rawgeti(this->L, LUA_REGISTRYINDEX, table_ref);
      }

      lua_pushstring(this->L, buffer);
      lua_call(this->L, 2, 1);

      // If we didn't return a table, treat it as a complete falure of the step
      if (!lua_istable(this->L, -1)) {
        LOG("ERROR: parser did not return a table.");
        cont = false;
        lua_pop(this->L, 1);
        break;
      }

      // If we got a table we need to pop it so we can resort the stack
      table_ref = luaL_ref(this->L, LUA_REGISTRYINDEX);
    }

    // If a parser failed, move on to next step
    if (!cont) {
      continue;
    }

    // Run all the outputs in order
    for (int oi = 0; step->outputs[oi]; oi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->outputs[oi]);
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, table_ref);
      lua_call(this->L, 1, 0);
    }

    // Finally unref the final table
    luaL_unref(this->L, LUA_REGISTRYINDEX, table_ref);
  }

  this->stats->total += 1;
}
