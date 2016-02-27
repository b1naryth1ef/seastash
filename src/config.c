#include "config.h"

// Load a table-array of function pointers into array
void store_fptr_array(int* array, lua_State* L) {
	lua_pushnil(L);

	int i = 0;
	while (lua_next(L, -2)) {
		array[i] = luaL_ref(L, LUA_REGISTRYINDEX);
		i++;
	}

	lua_pop(L, 1);
	lua_pop(L, 1);
}

config_s* config_new() {
	config_s* cfg = malloc(sizeof(config_s));
	cfg->num_workers = 2;
	cfg->msg_buffer_len = 128;
	cfg->L = luaL_newstate();
	luaL_openlibs(cfg->L);
	return cfg;
}

config_step_s* config_step_from_table(lua_State* L) {
	config_step_s* step = malloc(sizeof(config_step_s));

	if (lua_table_get(L, "filters")) {
		if (lua_istable(L, -1)) {
			store_fptr_array(step->filters, L);
		} else {
			LOG("WARNING: invalid filters table");
		}
	}

	if (lua_table_get(L, "parsers")) {
		if (lua_istable(L, -1)) {
			store_fptr_array(step->parsers, L);
		} else {
			LOG("WARNING: invalid parsers table");
		}
	}

	if (lua_table_get(L, "outputs")) {
		if (lua_istable(L, -1)) {
			store_fptr_array(step->outputs, L);
		} else {
			LOG("WARNING: invalid outputs table");
		}
	}

  return step;
}

config_s* config_from_file(const char* filename) {
	config_s* cfg = config_new();

	// Attempt to load config file
	if (luaL_loadfile(cfg->L, filename) || lua_pcall(cfg->L, 0, 0, 0)) {
		LOG("ERROR: cannot run config file: %s", lua_tostring(cfg->L, -1));
		return NULL;
	}

	lua_getglobal(cfg->L, "config");
	if (!lua_istable(cfg->L, -1)) {
		LOG("ERROR: config variable must be table");
		return NULL;
	}

	// Now we query variables out of the table
	if (lua_table_get(cfg->L, "num_workers")) {
		cfg->num_workers = (uint16_t) lua_tonumber(cfg->L, -1);
		lua_pop(cfg->L, 1);
		lua_pop(cfg->L, 1);
	}

	if (lua_table_get(cfg->L, "msg_buffer_len")) {
		cfg->msg_buffer_len = (uint16_t) lua_tonumber(cfg->L, -1);
		lua_pop(cfg->L, 1);
		lua_pop(cfg->L, 1);
	}

	// If we have steps load those
	if (lua_table_get(cfg->L, "steps")) {
		lua_pushnil(cfg->L);

		int stepi = 0;
		while (lua_next(cfg->L, -2)) {
			if (lua_istable(cfg->L, -1)) {
				cfg->steps[stepi] = config_step_from_table(cfg->L);
				stepi++;
			} else {
				LOG("WARNING: skipping invalid step");
			}
			lua_pop(cfg->L, 1);
		}

		lua_pop(cfg->L, 1);
		lua_pop(cfg->L, 1);
	}

	return cfg;
}

