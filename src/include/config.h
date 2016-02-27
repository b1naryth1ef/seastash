#pragma once

#include "global.h"

typedef struct config_step_t {
  int filters[256];
  int parsers[256];
  int outputs[256];
} config_step_s;

typedef struct config_t {
  uint16_t regi;
  uint16_t num_workers;
  uint16_t msg_buffer_len;

	lua_State* L;
  config_step_s* steps[256];
} config_s;

config_s* config_new();
config_s* config_from_file(const char* filename);
