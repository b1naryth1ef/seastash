#pragma once

#include <libmill.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>

typedef enum {
  CLOSE,
  TASK
} msg_type_t;

typedef struct config_t {
  uint16_t num_workers;
  uint16_t msg_buffer_len;
} config_s;

typedef struct server_t {
  config_s* config;
  tcpsock socket;

  chan work;

  uint64_t mps;
} server_s;

typedef struct msg_t {
  msg_type_t type;
  char* buffer;
} msg_s;

void LOG(const char* format, ...) {
  va_list args;
  va_start(args, format);

  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  // time(&timer);
  // tm_info = localtime(&timer);
  // strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

  char fmt[128];
  sprintf(fmt, "%s\n", format);
  vprintf(fmt, args);
  va_end(args);
}

