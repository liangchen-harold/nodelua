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
	os_timer_t *pCloud;
} Cloud;


void ICACHE_FLASH_ATTR cloud_on_data_cb(void *arg)
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

int ICACHE_FLASH_ATTR lnode_cloud_initialize(lua_State* L)
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

    lua_pushvalue(L, 2); //cloudid
    lua_setfield(L, 1, "id");

    Cloud *s = lua_newuserdata(L,sizeof(Cloud));
    lua_setfield(L, 1, "rawcloud");

    DEBUG_MSG("## timer:initialize udata=0x%08X\n", s);

    return 0;
}

int ICACHE_FLASH_ATTR lnode_cloud_gc(lua_State* L) {
    DEBUG_MSG("## __gc\n");

    lua_getfield(L, 1, "rawcloud");
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

static void ICACHE_FLASH_ATTR cloud_on_http_response(int status, char *data, void *arg)
{
    //DEBUG_MSG("resp 0x%08X\n", arg);
    ref_t *ref = (ref_t*)arg;
	if (ref != NULL)
	{
        lua_State *L = ref->L;
        //get the callback
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);

        /* do the call (0 arguments, 0 result) */
        if (lua_pcall(L, 0, 0, 0) != 0)
        {
            const char *msg = lua_tostring(L, -1);
            if (msg == NULL) msg = "err2";
            luaL_error(L, msg);
            lua_pop(L, 1);
        }

        //free
        luaL_unref(L, LUA_REGISTRYINDEX, ref->r);
        os_free(ref);
	}
}


static int ICACHE_FLASH_ATTR lnode_cloud_append (lua_State *L)
{
    lua_checkself(L);

    lua_getfield(L, 1, "id");
    const char *id = luaL_checkstring(L, -1);
    double v0 = luaL_checknumber(L, 2);

    char buf[32];
    os_sprintf(buf, "%d.%03d", (int)v0, (int)((v0-(int)v0)*1000));
    //DEBUG_MSG("cloud:%s\n", buf);

    ref_t *ref = NULL;
    if (lua_isfunction(L, 3))
    {
        ref = (ref_t*)os_malloc(sizeof(ref_t));
        ref->L = L;
        lua_pushvalue(L, 3); /* Store the callback */
        ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    cloud_data_append(id, buf, "", "", cloud_on_http_response, ref);

    // lua_getfield(L, 1, "rawcloud");
    // Cloud *s = (Cloud *)lua_touserdata(L, -1);
    // int delay = luaL_checkinteger(L, 3);
    //
    // ref_t *ref = (ref_t*)os_malloc(sizeof(ref_t)); //TODO: free!
    // ref->L = L;
    // lua_pushvalue(L, 2); /* Store the callback */
    // ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
    //
    // s->pCloud = (os_timer_t *)os_zalloc(sizeof(os_timer_t));
    //
    // os_timer_setfn(s->pCloud, timer_ontimer_cb, ref);
    // os_timer_arm(s->pCloud, delay, repeat);

    return 0;
}

static const luaL_Reg lnode_cloud_meta[] = {
    {"__gc", lnode_cloud_gc},
    { NULL, NULL }
};

static const luaL_Reg lnode_cloud_methods[] = {
    {"initialize", lnode_cloud_initialize},
    {"append", lnode_cloud_append},
    { NULL, NULL }
};

/* ====================================================== */

LUALIB_API int luaopen_lnode_cloud (lua_State *L) {
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
    luaL_register(L, NULL, lnode_cloud_methods); // register functions in the metatable
    //Emitter.meta.funcs <= lnode_emitter_meta
    lua_pushstring(L, "meta");
    lua_gettable(L, -2);
    luaL_register(L, NULL, lnode_cloud_meta); // register functions in the metatable
    lua_pop(L, 1);
    //_G.net.socket <= temp
    lua_setglobal(L, "Cloud");

    return 1;
}

/* Testing code:
*/
