/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <os_type.h>
#include <ets_sys.h>
#include <ip_addr.h>
#include <mem.h>
#include <osapi.h>
#include <espconn.h>
#include "cloud/data.h"

#define loslib_c
#define LUA_LIB

#include "lua/lua.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

typedef struct {

} Emitter;

int ICACHE_FLASH_ATTR lnode_emitter_gc(lua_State* L) {
    DEBUG_MSG("## emitter:__gc\n");

    //Emitter *s = (Emitter *)luaL_checkudata(L, 1, "Emitter.meta");
    return 0;
}

int ICACHE_FLASH_ATTR lnode_emitter_initialize(lua_State* L) {

    lua_newtable(L);
    lua_setfield(L, 1, "cb");

    DEBUG_MSG("## emitter:initialize\n");

    // luaL_getmetatable(L, "Emitter.meta");
    // lua_setmetatable(L, -2);
    // return 1;
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_emitter_on (lua_State *L)
{
    lua_checkself(L);

    lua_getfield(L, 1, "cb");
    lua_pushvalue(L, 2); //event name as the second params of :on
    lua_pushvalue(L, 3); //event callback as the third params of :on
    lua_settable(L, -3);

    return 0;
}

static int ICACHE_FLASH_ATTR lnode_emitter_emit (lua_State *L)
{
    lua_checkself(L);
    int varargs = lua_gettop(L);

    lua_getfield(L, 1, "cb");
    lua_pushvalue(L, 2); //event name as the second params of :on
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err3");
    }
    // :emit("event", ...)
    int i;
    for (i = 3; i <= varargs; i ++)
    {
        lua_pushvalue(L, i);
    }
    /* do the call (varargs-2 arguments, 0 result) */
    if (lua_pcall(L, varargs-2, 0, 0) != 0)
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "err4";
        luaL_error(L, msg);
        lua_pop(L, 1);
    }

    return 0;
}

static const luaL_Reg lnode_emitter_meta[] = {
    {"__gc", lnode_emitter_gc},
    { NULL, NULL }
};

static const luaL_Reg lnode_emitter_methods[] = {
    {"initialize", lnode_emitter_initialize},
    {"on", lnode_emitter_on},
    {"emit", lnode_emitter_emit},
    { NULL, NULL }
};

/* ====================================================== */

LUALIB_API int luaopen_lnode_emitter (lua_State *L) {
    //temp=Object:extend()
    lua_getglobal(L, "Object");
    lua_pushstring(L, "extend");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    lua_getglobal(L, "Object"); //Object is the first argument as 'self'
    /* do the call (1 arguments, 1 result) */
    if (lua_pcall(L, 1, 1, 0) != 0)
    {
        luaL_error(L, "err2");
    }
    //Emitter.funcs <= lnode_emitter_methods
    luaL_register(L, NULL, lnode_emitter_methods); // register functions in the metatable
    //Emitter.meta.funcs <= lnode_emitter_meta
    lua_pushstring(L, "meta");
    lua_gettable(L, -2);
    luaL_register(L, NULL, lnode_emitter_meta); // register functions in the metatable
    lua_pop(L, 1);
    //_G.Emitter <= temp
    lua_setglobal(L, "Emitter");


    // luaL_newmetatable(L, "Emitter.meta"); //leaves new metatable on the stack
    // lua_pushvalue(L, -1); // there are two 'copies' of the metatable on the stack
    // lua_setfield(L, -2, "__index"); // pop one of those copies and assign it to
    //                                 // __index field od the 1st metatable
    // stackDump(L);
    // lua_pushcfunction(L, lnode_emitter_new);
    // lua_setfield(L,-2, "new");
    // lua_pushcfunction(L, lnode_emitter_gc);
    // lua_setfield(L,-2, "__gc");
    //
    // stackDump(L);
    // luaL_register(L, NULL, lnode_emitter_methods); // register functions in the metatable
    // luaL_register(L, "Emitter", lnode_emitter_meta);
    //
    // stackDump(L);
    return 1;
}

/* Testing code:

table.foreach(Emitter, function(i, v) print(i .. ":") print(v) end)
table.foreach(Emitter.meta, function(i, v) print(i .. ":") print(v) end)
e=Emitter:new()
e:on("recv")
e=nil collectgarbage()


*/
