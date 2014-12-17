/*
** $Id: linit.c,v 1.14.1.1 2007/12/27 13:02:25 roberto Exp $
** Initialization of libraries for lua.c
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

#include "lua/lua.h"

#include "lua/lualib.h"
#include "lua/lauxlib.h"


static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  //{LUA_LOADLIBNAME, luaopen_package},
  //{LUA_TABLIBNAME, luaopen_table},
  //{LUA_IOLIBNAME, luaopen_io},
  //{LUA_OSLIBNAME, luaopen_os},
  //{LUA_STRLIBNAME, luaopen_string},
  //{LUA_MATHLIBNAME, luaopen_math},
  //{LUA_DBLIBNAME, luaopen_debug},
  {LUA_LNODE_NODE_NAME, luaopen_lnode_node},
  {"", luaopen_lnode_emitter},
  {"", luaopen_lnode_timer},
  {"", luaopen_lnode_cloud},
  {LUA_LNODE_SENSOR_NAME, luaopen_lnode_sensor},
  {LUA_LNODE_GPIO_NAME, luaopen_lnode_gpio},
  {LUA_LNODE_WIFI_NAME, luaopen_lnode_wifi},
  //{LUA_LNODE_NET_NAME, luaopen_lnode_net},
  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}
