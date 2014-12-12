/*
** $Id: lualib.h,v 1.36.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua standard libraries
** See Copyright Notice in lua.h
*/


#ifndef lualib_h
#define lualib_h

#include "lua.h"


/* Key to file-handle type */
#define LUA_FILEHANDLE		"FILE*"


#define LUA_LNODE_NODE_NAME	"node"
LUALIB_API int (luaopen_lnode_node) (lua_State *L);

LUALIB_API int (luaopen_lnode_emitter) (lua_State *L);

#define LUA_LNODE_GPIO_NAME	"gpio"
LUALIB_API int (luaopen_lnode_gpio) (lua_State *L);

#define LUA_LNODE_WIFI_NAME	"wifi"
LUALIB_API int (luaopen_lnode_wifi) (lua_State *L);

#define LUA_LNODE_NET_NAME	"net"
LUALIB_API int (luaopen_lnode_net) (lua_State *L);
LUALIB_API int (luaopen_lnode_net_socket) (lua_State *L);

LUALIB_API int luaopen_lnode_timer (lua_State *L);
LUALIB_API int luaopen_lnode_cloud (lua_State *L);

#define LUA_LNODE_SENSOR_NAME	"sensor"
LUALIB_API int (luaopen_lnode_sensor) (lua_State *L);

#define LUA_COLIBNAME	"coroutine"
LUALIB_API int (luaopen_base) (lua_State *L);

#define LUA_TABLIBNAME	"table"
LUALIB_API int (luaopen_table) (lua_State *L);

#define LUA_IOLIBNAME	"io"
LUALIB_API int (luaopen_io) (lua_State *L);

#define LUA_OSLIBNAME	"os"
LUALIB_API int (luaopen_os) (lua_State *L);

#define LUA_STRLIBNAME	"string"
LUALIB_API int (luaopen_string) (lua_State *L);

#define LUA_MATHLIBNAME	"math"
LUALIB_API int (luaopen_math) (lua_State *L);

#define LUA_DBLIBNAME	"debug"
LUALIB_API int (luaopen_debug) (lua_State *L);

#define LUA_LOADLIBNAME	"package"
LUALIB_API int (luaopen_package) (lua_State *L);


/* open all previous libraries */
LUALIB_API void (luaL_openlibs) (lua_State *L);



#ifndef lua_assert
#define lua_assert(x)	((void)0)
#endif


#endif
