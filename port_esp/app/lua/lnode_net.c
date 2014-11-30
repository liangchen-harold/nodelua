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

#define loslib_c
#define LUA_LIB

#include "lua/lua.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua/lnode_net.h"

typedef struct {
    lua_State* L;
    int r;
//  uv_getaddrinfo_t handle;
} luv_dns_ref_t;

typedef struct {
    struct espconn *pConn;
} Socket;

ip_addr_t dummy_ip;

/* Utility for storing the callback in the dns_req token */
static luv_dns_ref_t* ICACHE_FLASH_ATTR luv_dns_store_callback(lua_State* L, int index) {
  luv_dns_ref_t* ref;

  ref = (luv_dns_ref_t*)os_malloc(sizeof(luv_dns_ref_t));
  ref->L = L;
  if (lua_isfunction(L, index)) {
    lua_pushvalue(L, index); /* Store the callback */
    ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  return ref;
}

static void ICACHE_FLASH_ATTR luv_dns_ref_cleanup(luv_dns_ref_t *ref)
{
    if (ref != NULL)
    {
        os_free(ref);
    }
}

static void ICACHE_FLASH_ATTR luv_dns_get_callback(luv_dns_ref_t *ref)
{
  lua_State *L = ref->L;
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);
  luaL_unref(L, LUA_REGISTRYINDEX, ref->r);
}

/******************************************************************************
 * FunctionName : on_dns_found
 * Description  : dns found callback
 * Parameters   : name -- pointer to the name that was looked up.
 *                ipaddr -- pointer to an ip_addr_t containing the IP address of
 *                the hostname, or NULL if the name could not be found (or on any
 *                other error).
 *                callback_arg -- a user-specified callback argument passed to
 *                dns_gethostbyname
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
on_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    luv_dns_ref_t *ref = (luv_dns_ref_t *)pespconn->reverse;

    if (ipaddr != NULL)
    {
        char buf[16];
        char *p = (char*)&(ipaddr->addr);
        os_sprintf(buf, IPSTR, p[0], p[1], p[2], p[3]);

        luv_dns_get_callback(ref);

        lua_pushnil(ref->L);
        lua_pushstring(ref->L, buf);
    }
    else
    {
        lua_pushstring(ref->L, "error!");
        lua_pushnil(ref->L);
    }
    lua_pcall(ref->L, 2, 0, 0);

    luv_dns_ref_cleanup(ref);
    os_free(pespconn);
}

static int ICACHE_FLASH_ATTR lnode_net_dns (lua_State *L)
{
    const char* host = luaL_checkstring(L, 1);
    luv_dns_ref_t *ref = luv_dns_store_callback(L, 2);

    struct espconn *pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
    pCon->type = ESPCONN_TCP;
    pCon->state = ESPCONN_NONE;
    pCon->reverse = ref;

    espconn_gethostbyname(pCon, host, &dummy_ip, on_dns_found);

    return 0;
}


static int ICACHE_FLASH_ATTR lnode_net_createConnection (lua_State *L)
{
    int protocol = luaL_checkinteger(L, 1);

    /* return Socket:new()
     *
     */
    //temp=Socket:new()
    lua_getglobal(L, "socket");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    lua_getglobal(L, "socket"); //Object is the first argument as 'self'
    lua_pushinteger(L, protocol);
    /* do the call (2 arguments, 1 result) */
    if (lua_pcall(L, 2, 1, 0) != 0)
    {
        luaL_error(L, "err2");
    }
    //return temp
    return 1;
}

/* ====================================================== */

static const luaL_Reg lnode_net_lib[] = {
  {"dns",                  lnode_net_dns},
  {"createConnection",      lnode_net_createConnection},
  {NULL, NULL}
};

LUALIB_API int luaopen_lnode_net (lua_State *L) {
    luaL_register(L, LUA_LNODE_NET_NAME, lnode_net_lib);
    lua_pushnumber(L, TCP);
    lua_setfield(L, -2, "TCP");
    lua_pushnumber(L, UDP);
    lua_setfield(L, -2, "UDP");

    luaopen_lnode_net_socket(L);

    return 1;
}

/* Testing code:
*/
