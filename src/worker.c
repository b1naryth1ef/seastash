#include "worker.h"

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

worker_s* worker_new(server_s* server) {
  worker_s* w = malloc(sizeof(worker_s));
  w->L = lua_newthread(server->config->L);
  w->server = server;
}

void worker_free(worker_s* w) {
  lua_close(w->L);
  free(w);
}

void worker_process_message(worker_s* this, msg_s* msg) {
	config_step_s* step;

  bool cont = false;

	for (int i = 0; this->server->config->steps[i]; i++) {
		step = this->server->config->steps[i];

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

    // If the filter doesnt match, just move on
    if (!cont) {
      continue;
    }

    for (int pi = 0; step->parsers[pi]; pi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->parsers[pi]);

      // Push a new table on the stack if its the first iteration
      if (pi == 0) {
        lua_newtable(this->L);
      }

      lua_pushstring(this->L, msg->buffer);
      lua_call(this->L, 2, 1);

      if (!lua_istable(this->L, -1)) {
        LOG("ERROR: parser did not return a table.");
        cont = false;
        lua_pop(this->L, 1);
        break;
      }
    }

    if (!cont) {
      break;
    }

    // Now store the final version of the table in the global registry
    int result_table = luaL_ref(this->L, LUA_REGISTRYINDEX);

    for (int oi = 0; step->parsers[oi]; oi++) {
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, step->outputs[oi]);
      lua_rawgeti(this->L, LUA_REGISTRYINDEX, result_table);
      lua_call(this->L, 1, 0);
    }

    lua_debug_stack(this->L);
	}
}

coroutine void worker_loop(worker_s* this) {
  while (true) {
    msg_s* msg = chr(this->server->work, msg_s*);

    if (msg->type == CLOSE) {
      msg_free(msg);
      return;
    }

    worker_process_message(this, msg);
    this->server->mps++;
    msg_free(msg);
  }
}

