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

enum Protocol {
    TCP = 0,
    UDP,
};

typedef struct {
    lua_State* L;
    int r;
//  uv_getaddrinfo_t handle;
} luv_dns_ref_t;

typedef struct {
    struct espconn *pConn;
} Socket;

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

ip_addr_t dummy_ip;
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

static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg);
static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 errType);

static int ICACHE_FLASH_ATTR lnode_net_createConnection (lua_State *L)
{
    int protocol = luaL_checkinteger(L, 1);

    /* return Socket.new()
     *
     */
    Socket *s = lua_newuserdata(L,sizeof(Socket));
    luaL_getmetatable(L, "Socket");
    lua_setmetatable(L, -2);

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
    __printf("At: 0x%08X\n", s);

    return 1;
}

/* ==socket====================================================== */

/**
  * @brief  Tcp client disconnect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR tcpclient_discon_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;

    __printf("tcp client disconnect\n");
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

    __printf("tcp client recv %d bytes\n", len);
    // __printf("\n\ntcp client recv: \n\n");
    // __fputs(pdata, 0);
}

/**
  * @brief  Tcp client connect success callback function.
  * @param  arg: contain the ip link information
  * @retval None
  */
static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg)
{
    struct espconn *pConn = (struct espconn *)arg;

    __printf("tcp client connect\n");

    espconn_regist_disconcb(pConn, tcpclient_discon_cb);
    espconn_regist_recvcb(pConn, tcpclient_recv_cb);////////
    //espconn_regist_sentcb(pConn, tcpclient_sent_cb);///////
}

static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 errType)
{
    struct espconn *pConn = (struct espconn *)arg;

    __printf("tcp client reconnect\n");
}

int ICACHE_FLASH_ATTR lnode_net_socket_gc(lua_State* L) {
    __printf("## __gc\n");

    Socket *s = (Socket *)luaL_checkudata(L, 1, "Socket");
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
// 只能通过createConnection创建socket！
// int ICACHE_FLASH_ATTR lnode_net_socket_new(lua_State* L) {
//     __printf("## new\n");
//
//     void *p = lua_newuserdata(L,sizeof(Socket));
//     luaL_getmetatable(L, "Socket");
//     lua_setmetatable(L, -2);
//     return 1;
// }

static void ICACHE_FLASH_ATTR on_dns_found_for_connect(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    if (ipaddr != NULL)
    {
        char buf[16];
        char *p = (char*)&(ipaddr->addr);
        os_memcpy(pespconn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
        __printf("connection to "IPSTR"...\n", p[0], p[1], p[2], p[3]);

        espconn_connect(pespconn);
    }
}
static int ICACHE_FLASH_ATTR lnode_net_socket_connect (lua_State *L)
{
    Socket *s = (Socket *)luaL_checkudata(L, 1, "Socket");
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
    Socket *s = (Socket *)luaL_checkudata(L, 1, "Socket");
    const char *content = luaL_checkstring(L, 2);

    espconn_sent(s->pConn, (char *)content, os_strlen(content));

    return 0;
}

static const luaL_Reg lnode_net_socket_methods[] = {
    {"connect", lnode_net_socket_connect},
    {"send", lnode_net_socket_send},
    { NULL, NULL }
};

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

    // lua_createtable(L, 0, 1);
    //
    // lua_pushstring(L, "connect");
    // lua_pushcclosure(L, lnode_net_socket_connect, 0);
    // lua_settable(L, -3);  /* 3rd element from the stack top */
    //
    // lua_setfield(L, -2, "socket");
    luaL_newmetatable(L, "Socket"); //leaves new metatable on the stack
    lua_pushvalue(L, -1); // there are two 'copies' of the metatable on the stack
    lua_setfield(L, -2, "__index"); // pop one of those copies and assign it to
                                    // __index field od the 1st metatable
    // lua_pushcfunction(L, lnode_net_socket_new);
    // lua_setfield(L,-2, "new");
    lua_pushcfunction(L, lnode_net_socket_gc);
    lua_setfield(L,-2, "__gc");

    luaL_register(L, NULL, lnode_net_socket_methods); // register functions in the metatable
    //luaL_register(L, "classname", lnode_net_socket_meta);

    return 1;
}

/* Testing code:
*/
