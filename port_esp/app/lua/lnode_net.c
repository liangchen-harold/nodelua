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
    struct espconn *pConn;
} Socket;

ip_addr_t dummy_ip;

/* Utility for storing the callback in the dns_req token */
static ref_t* ICACHE_FLASH_ATTR luv_dns_store_callback(lua_State* L, int index) {
  ref_t* ref;

  ref = (ref_t*)os_malloc(sizeof(ref_t));
  ref->L = L;
  if (lua_isfunction(L, index)) {
    lua_pushvalue(L, index); /* Store the callback */
    ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  return ref;
}

static void ICACHE_FLASH_ATTR luv_dns_ref_cleanup(ref_t *ref)
{
    if (ref != NULL)
    {
        os_free(ref);
    }
}

static void ICACHE_FLASH_ATTR luv_dns_get_callback(ref_t *ref)
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

    ref_t *ref = (ref_t *)pespconn->reverse;

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
    ref_t *ref = luv_dns_store_callback(L, 2);

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
    lua_getglobal(L, "Socket");
    lua_pushstring(L, "new");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    lua_getglobal(L, "Socket"); //Object is the first argument as 'self'
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

print(node.free()) collectgarbage() print(node.free())

host="api.gulumao.cn" a=net.createConnection(net.TCP)
a:on("data", function(data) local i = 0 while true do i = string.find(data, "\r\n\r\n", i+1)
if i == nil then break end local j=string.find(data, "\r\n", i+4) if j == nil then break end
local len=tonumber(string.sub(data, i+4, j-1), 16) local c=string.sub(data, j+2, j+2+len)
print(c) _G.product = loadstring(c) _G.product():config() end end)
a:on("connect", function(data)
    a:send("GET /api/module/code?miid=6&security=9eeec357d65a9ef4ef79e4473e96cb88 HTTP/1.1\r\nHost: " .. host .. "\r\n\r\n")
end) a:connect(host, 80)
a:connect() for i = 1,100 do print(i .. ":" .. node.free()) node.wdt() collectgarbage() end

node.setid("6", "9eeec357d65a9ef4ef79e4473e96cb88")
node.setid()

*/
