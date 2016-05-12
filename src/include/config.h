#pragma once

#include "global.h"

#define MAX_FILTERS 1024
#define MAX_STEPS 1024

/*
  A "step" in the pipeline. Each step contains up to 256 seperate filters,
  parsers and outputs. The arrays in this struct contain LUA_REGISTRYINDEX
  indexes that point to functions passed in the configuration.
*/
typedef struct config_step_t {
  int filters[MAX_FILTERS];
  int parsers[MAX_FILTERS];
  int outputs[MAX_FILTERS];
} config_step_s;

/*
  The base configuration struct. This is repsonsible for initially loading
  the lua state and then is cloned into seperate lua threads for worker
  execution.
*/
typedef struct config_t {
  // Debug mode
  bool debug;

  // Network delimiter
  char network_delim[1];

	lua_State* L;
  config_step_s* steps[MAX_STEPS];
} config_s;

config_s* config_new();
void config_free(config_s* this);
config_s* config_from_file(const char* filename);
