#include "worker.h"

worker_s* worker_new(server_s* server) {
  worker_s* w = malloc(sizeof(worker_s));
  w->L = lua_newthread(server->config->L);
  w->ctrl = chmake(int, 0);
  w->server = server;
}

void worker_free(worker_s* w) {
  lua_close(w->L);
  chclose(w->ctrl);
  free(w);
}

// Attempts to fully process a message through all steps
void worker_process_message(worker_s* this, msg_s* msg) {
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

coroutine void worker_loop(worker_s* this) {
  while (true) {
    choose {
      in(this->ctrl, int, i): {
        if (i == 0) {
          return;
        }
      }
      in(this->server->work, msg_s*, msg): {
        if (msg->type == CLOSE) {
          msg_free(msg);
          return;
        }

        worker_process_message(this, msg);
        this->server->stats->mps++;
        msg_free(msg);
      }
      end
    }
  }
}

