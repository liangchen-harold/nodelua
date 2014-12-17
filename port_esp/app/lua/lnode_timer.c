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
#include "lua/lnode_net.h"


typedef struct {
    os_timer_t *pTimer;
} Timer;


void ICACHE_FLASH_ATTR timer_ontimer_cb(void *arg)
{
    ref_t *ref = (ref_t *)arg;
    lua_State *L = ref->L;
    //get the callback
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);
    //luaL_unref(ref->L, LUA_REGISTRYINDEX, ref->r); //TODO: free

    /* do the call (0 arguments, 0 result) */
    if (lua_pcall(L, 0, 0, 0) != 0)
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "err2";
        luaL_error(L, msg);
        lua_pop(L, 1);
    }
    // DEBUG_MSG("tcp client recv %d bytes\n", len);
}

int ICACHE_FLASH_ATTR lnode_timer_initialize(lua_State* L)
{
    //Emitter.initialize(self)
    lua_getglobal(L, "Emitter");
    lua_pushstring(L, "initialize");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    lua_pushvalue(L, 1); //self
    /* do the call (1 arguments, 0 result) */
    if (lua_pcall(L, 1, 0, 0) != 0)
    {
        luaL_error(L, "err2");
    }

    Timer *s = lua_newuserdata(L,sizeof(Timer));
    lua_setfield(L, 1, "rawtimer");

    DEBUG_MSG("## timer:initialize udata=0x%08X\n", s);

    return 0;
}

int ICACHE_FLASH_ATTR lnode_timer_gc(lua_State* L) {
    DEBUG_MSG("## __gc\n");

    lua_getfield(L, 1, "rawtimer");
    // Socket *s = (Socket *)lua_touserdata(L, -1);
    //
    // if (s->pConn->proto.tcp != NULL)
    // {
    //     os_free(s->pConn->proto.tcp);
    // }
    // if (s->pConn->proto.udp != NULL)
    // {
    //     os_free(s->pConn->proto.udp);
    // }
    // os_free(s->pConn);
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_timer_do_setTimeout (lua_State *L, bool repeat)
{
    lua_checkself(L);
    lua_getfield(L, 1, "rawtimer");
    Timer *s = (Timer *)lua_touserdata(L, -1);
    int delay = luaL_checkinteger(L, 3);

    ref_t *ref = (ref_t*)os_malloc(sizeof(ref_t)); //TODO: free!
    ref->L = L;
    lua_pushvalue(L, 2); /* Store the callback */
    ref->r = luaL_ref(L, LUA_REGISTRYINDEX);

    s->pTimer = (os_timer_t *)os_zalloc(sizeof(os_timer_t));

    os_timer_setfn(s->pTimer, timer_ontimer_cb, ref);
    os_timer_arm(s->pTimer, delay, repeat);

    return 0;
}
static int ICACHE_FLASH_ATTR lnode_timer_setTimeout (lua_State *L)
{
    lnode_timer_do_setTimeout(L, false);
}
static int ICACHE_FLASH_ATTR lnode_timer_setInterval (lua_State *L)
{
    lnode_timer_do_setTimeout(L, true);
}

static const luaL_Reg lnode_timer_meta[] = {
    {"__gc", lnode_timer_gc},
    { NULL, NULL }
};

static const luaL_Reg lnode_timer_methods[] = {
    {"initialize", lnode_timer_initialize},
    {"setTimeout", lnode_timer_setTimeout},
    {"setInterval", lnode_timer_setInterval},
    { NULL, NULL }
};

/* ====================================================== */

LUALIB_API int luaopen_lnode_timer (lua_State *L) {
    //temp=Emitter:extend()
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
    luaL_register(L, NULL, lnode_timer_methods); // register functions in the metatable
    //Emitter.meta.funcs <= lnode_emitter_meta
    lua_pushstring(L, "meta");
    lua_gettable(L, -2);
    luaL_register(L, NULL, lnode_timer_meta); // register functions in the metatable
    lua_pop(L, 1);
    //_G.net.socket <= temp
    lua_setglobal(L, "Timer");

    return 1;
}

/* Testing code:
*/
