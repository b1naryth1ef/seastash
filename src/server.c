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

coroutine void server_read_loop(server_s* this, tcpsock conn) {
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

    // Strip control character
    buf[nbytes - 1] = '\0';
    server_process_message(this, buf);
  }

  cleanup:
    LOG("Closing socket %p", conn);
    tcpclose(conn);
}

// Attempts to fully process a message through all steps
void server_process_message(server_s* this, msg_s* msg) {
  config_step_s* step;
  bool cont = false;
  int table_ref;

  // Run all the steps in order, encapsilating failures within the step
  for (int i = 0; this->server->config->steps[i]; i++) {
    step = this->server->config->steps[i];

    // Run all the filters in order, skipping the step if a single one fails to match
    for (int fi = 0; step->filters[fi]; fi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->filters[fi]);
      lua_pushstring(this->L, msg->buffer);
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

      lua_pushstring(this->L, msg->buffer);
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
}
