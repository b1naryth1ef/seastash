#pragma once

#include "global.h"

/*
  A "step" in the pipeline. Each step contains up to 256 seperate filters,
  parsers and outputs. The arrays in this struct contain LUA_REGISTRYINDEX
  indexes that point to functions passed in the configuration.
*/
typedef struct config_step_t {
  int filters[256];
  int parsers[256];
  int outputs[256];
} config_step_s;

/*
  The base configuration struct. This is repsonsible for initially loading
  the lua state and then is cloned into seperate lua threads for worker
  execution.
*/
typedef struct config_t {
  uint16_t regi;
  uint16_t num_workers;
  uint16_t msg_buffer_len;

	lua_State* L;
  config_step_s* steps[256];
} config_s;

config_s* config_new();
config_s* config_from_file(const char* filename);
