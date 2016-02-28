#pragma once

#define _GNU_SOURCE

#include <libmill.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

int ftruncate(int fildes, off_t length);

static void LOG(const char* format, ...) {
  va_list args;
  va_start(args, format);

  time_t timer;
  char buffer[26];
  struct tm* tm_info;

  time(&timer);
  tm_info = localtime(&timer);
  strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

  char fmt[128];
  sprintf(fmt, "[%s] %s\n", buffer, format);
  vprintf(fmt, args);
  va_end(args);
}

static bool lua_table_get(lua_State* L, char* key) {
  lua_pushnil(L);

  while (lua_next(L, -2)) {
    // Skip over non-string keys
    if (!lua_isstring(L, -2)) {
      continue;
    }

    if (strcmp(lua_tostring(L, -2), key) == 0) {
      return true;
    }

    lua_pop(L, 1);
  }

  lua_pop(L, 1);
  return false;
}

static void lua_debug_stack(lua_State* l) {
  int i;
  int top = lua_gettop(l);

  printf("total in stack %d\n",top);

  for (i = 1; i <= top; i++) {
    int t = lua_type(l, i);
    printf("  ");
    switch (t) {
      case LUA_TSTRING:
        printf("string: '%s'\n", lua_tostring(l, i));
        break;
      case LUA_TBOOLEAN:
        printf("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
        break;
      case LUA_TNUMBER:
        printf("number: %g\n", lua_tonumber(l, i));
        break;
      default:
        printf("%s\n", lua_typename(l, t));
        break;
    }
  }

  printf("\n");
}
