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

static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg);
static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 errType);

/* ==socket====================================================== */

/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR tcpclient_discon_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;

    DEBUG_MSG("tcp client disconnect\n");
}

/**
  * @brief  Client received callback function.
  * @param  arg: contain the ip link information
  * @param  pdata: received data
  * @param  len: the lenght of received data
  * @retval None
  */
static void ICACHE_FLASH_ATTR tcpclient_recv_cb(void *arg, char *pdata, unsigned short len)
{
    struct espconn *pConn = (struct espconn *)arg;

    ref_t *ref = (ref_t *)pConn->reverse;
    lua_State *L = ref->L;
    //get the self
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);
    //luaL_unref(ref->L, LUA_REGISTRYINDEX, ref->r); //TODO: free

    lua_pushstring(L, "emit");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    //push self
    lua_pushvalue(L, -2);
    //push event
    lua_pushstring(L, "data");
    //push data
    lua_pushlstring(L, pdata, len);
    //lua_pushinteger(L, len);
    /* do the call (3 arguments, 0 result) */
    if (lua_pcall(L, 3, 0, 0) != 0)
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "err2";
        luaL_error(L, msg);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    // DEBUG_MSG("tcp client recv %d bytes\n", len);
}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;

    DEBUG_MSG("tcp client connect\n");

    espconn_regist_disconcb(pConn, tcpclient_discon_cb);
    espconn_regist_recvcb(pConn, tcpclient_recv_cb);////////
    //espconn_regist_sentcb(pConn, tcpclient_sent_cb);///////

    //call emit: "connect"
    ref_t *ref = (ref_t *)pConn->reverse;
    lua_State *L = ref->L;
    //get the self
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);
    //luaL_unref(ref->L, LUA_REGISTRYINDEX, ref->r); //TODO: free

    lua_pushstring(L, "emit");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    //push self
    lua_pushvalue(L, -2);
    //push event
    lua_pushstring(L, "connect");
    /* do the call (2 arguments, 0 result) */
    if (lua_pcall(L, 2, 0, 0) != 0)
    {
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "err2";
        luaL_error(L, msg);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 errType)
{
    struct espconn *pConn = (struct espconn *)arg;

    DEBUG_MSG("tcp client reconnect\n");
}

int ICACHE_FLASH_ATTR lnode_net_socket_initialize(lua_State* L)
{
    int protocol = luaL_checkinteger(L, 2);

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

    Socket *s = lua_newuserdata(L,sizeof(Socket));
    lua_setfield(L, 1, "rawsocket");

    DEBUG_MSG("## net_socket:initialize udata=0x%08X\n", s);

    s->pConn = (struct espconn *)os_zalloc(sizeof(struct espconn));
    if (protocol == TCP)
    {
        s->pConn->type = ESPCONN_TCP;
        s->pConn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

        s->pConn->proto.tcp->local_port = espconn_port();

        espconn_regist_connectcb(s->pConn, tcpclient_connect_cb);
        espconn_regist_reconcb(s->pConn, tcpclient_recon_cb);
    }
    else if (protocol == UDP)
    {
        s->pConn->type = ESPCONN_UDP;
        s->pConn->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
        s->pConn->proto.udp->local_port = espconn_port();
    }

    ref_t *ref = (ref_t*)os_malloc(sizeof(ref_t)); //TODO: free!
    ref->L = L;
    lua_pushvalue(L, 1); /* Store the self */
    ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
    s->pConn->reverse = ref;

    return 0;
}

int ICACHE_FLASH_ATTR lnode_net_socket_gc(lua_State* L) {
    DEBUG_MSG("## __gc\n");

    lua_getfield(L, 1, "rawsocket");
    Socket *s = (Socket *)lua_touserdata(L, -1);

    if (s->pConn->proto.tcp != NULL)
    {
        os_free(s->pConn->proto.tcp);
    }
    if (s->pConn->proto.udp != NULL)
    {
        os_free(s->pConn->proto.udp);
    }
    os_free(s->pConn);
    return 0;
}

static void ICACHE_FLASH_ATTR on_dns_found_for_connect(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    if (ipaddr != NULL)
    {
        char buf[16];
        char *p = (char*)&(ipaddr->addr);
        os_memcpy(pespconn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
        DEBUG_MSG("connection to "IPSTR"...\n", p[0], p[1], p[2], p[3]);

        espconn_connect(pespconn);
    }
}
static int ICACHE_FLASH_ATTR lnode_net_socket_connect (lua_State *L)
{
    lua_checkself(L);
    lua_getfield(L, 1, "rawsocket");
    Socket *s = (Socket *)lua_touserdata(L, -1);
    const char *host = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);

    s->pConn->proto.tcp->remote_port = port;

    if (1)
    {
        espconn_gethostbyname(s->pConn, host, &dummy_ip, on_dns_found_for_connect);
    }
    else
    {
        *((uint8 *) &(s->pConn->proto.tcp->remote_ip)) = 115;
        *((uint8 *) &(s->pConn->proto.tcp->remote_ip)+1) = 28;
        *((uint8 *) &(s->pConn->proto.tcp->remote_ip)+2) = 78;
        *((uint8 *) &(s->pConn->proto.tcp->remote_ip)+3) = 47;

        espconn_connect(s->pConn);
    }
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_net_socket_send (lua_State *L)
{
    lua_checkself(L);
    lua_getfield(L, 1, "rawsocket");
    Socket *s = (Socket *)lua_touserdata(L, -1);
    const char *content = luaL_checkstring(L, 2);

    espconn_sent(s->pConn, (char *)content, os_strlen(content));

    return 0;
}

static const luaL_Reg lnode_net_socket_meta[] = {
    {"__gc", lnode_net_socket_gc},
    { NULL, NULL }
};

static const luaL_Reg lnode_net_socket_methods[] = {
    {"initialize", lnode_net_socket_initialize},
    {"connect", lnode_net_socket_connect},
    {"send", lnode_net_socket_send},
    { NULL, NULL }
};

/* ====================================================== */

LUALIB_API int luaopen_lnode_net_socket (lua_State *L) {
    //temp=Emitter:extend()
    lua_getglobal(L, "Emitter");
    lua_pushstring(L, "extend");
    lua_gettable(L, -2);
    if (lua_isfunction(L, -1) != 1)
    {
        luaL_error(L, "err1");
    }
    lua_getglobal(L, "Emitter"); //Object is the first argument as 'self'
    /* do the call (1 arguments, 1 result) */
    if (lua_pcall(L, 1, 1, 0) != 0)
    {
        luaL_error(L, "err2");
    }
    //Emitter.funcs <= lnode_emitter_methods
    luaL_register(L, NULL, lnode_net_socket_methods); // register functions in the metatable
    //Emitter.meta.funcs <= lnode_emitter_meta
    lua_pushstring(L, "meta");
    lua_gettable(L, -2);
    luaL_register(L, NULL, lnode_net_socket_meta); // register functions in the metatable
    lua_pop(L, 1);
    //_G.net.socket <= temp
    lua_setglobal(L, "Socket");

    return 1;
}

/* Testing code:
*/
