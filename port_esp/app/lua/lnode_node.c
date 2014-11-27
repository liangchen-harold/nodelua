/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define loslib_c
#define LUA_LIB

#include "lua/lua.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"


static int ICACHE_FLASH_ATTR lnode_node_free (lua_State *L)
{
    lua_pushinteger(L, system_get_free_heap_size());
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_node_wdt (lua_State *L)
{
    ets_wdt_restore();
    return 0;
}

static const luaL_Reg lnode_node_lib[] = {
  {"free",          lnode_node_free},
  {"wdt",           lnode_node_wdt},
  {NULL, NULL}
};

/* }====================================================== */



LUALIB_API int luaopen_lnode_node (lua_State *L) {
  luaL_register(L, LUA_LNODE_NODE_NAME, lnode_node_lib);
  return 1;
}
