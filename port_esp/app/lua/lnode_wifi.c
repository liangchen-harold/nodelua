/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <os_type.h>
#include <ets_sys.h>
#include <mem.h>
#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>

#define loslib_c
#define LUA_LIB

#include "lua/lua.h"
#include "lua/lnode_net.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

enum Mode {
    STATION = 0,
    SOFTAP,
    STATIONAP,
};

//for connect callback
void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
    ref_t *ref = (ref_t *)arg;
    lua_State *L = ref->L;
    os_timer_t *pTimer = (os_timer_t*)ref->arg;


    struct ip_info ipconfig;

    os_timer_disarm(pTimer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
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
        os_free(ref->arg);
        os_free(ref);
    }
	else
	{
        /* if there are wrong while connecting to some AP, then reset mode */
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {
            //free
            luaL_unref(L, LUA_REGISTRYINDEX, ref->r);
            os_free(ref->arg);
            os_free(ref);
            __printf("wifi failed!\n");
        }
        else
        {
            os_timer_setfn(pTimer, (os_timer_func_t *)wifi_check_ip, ref);
            os_timer_arm(pTimer, 100, 0);
        }
    }
}

static int ICACHE_FLASH_ATTR lnode_wifi_setmode (lua_State *L)
{
    int mode = luaL_checkinteger(L, 1);
    switch(mode)
    {
        case STATION:
            wifi_set_opmode(STATION_MODE);
            break;
        case SOFTAP:
            wifi_set_opmode(SOFTAP_MODE);
            break;
        case STATIONAP:
            wifi_set_opmode(STATIONAP_MODE);
            break;
    }
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_config (lua_State *L)
{
    const char* ssid = luaL_checkstring(L, 1);
    const char* password = luaL_checkstring(L, 2);

    struct station_config config;
    os_memset(config.ssid, 0, sizeof(config.ssid));
    os_memcpy(config.ssid, ssid, min(sizeof(config.ssid)-1, os_strlen(ssid)));
    os_memset(config.password, 0, sizeof(config.password));
    os_memcpy(config.password, password, min(sizeof(config.password)-1, os_strlen(password)));
    config.bssid_set = 0;

    lua_pushboolean(L, wifi_station_set_config(&config));
    wifi_station_set_auto_connect(1);
    wifi_station_connect();
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_getconfig (lua_State *L)
{
    struct station_config config;
    wifi_station_get_config(&config);

    lua_pushstring(L, config.ssid);
    lua_pushstring(L, config.password);
    return 2;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_getip (lua_State *L)
{
    struct ip_info ipconfig;
    wifi_get_ip_info(STATION_IF, &ipconfig);
    char buf[16];
    char *p = (char*)&(ipconfig.ip.addr);
    os_sprintf(buf, IPSTR, p[0], p[1], p[2], p[3]);
    lua_pushstring(L, buf);
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_connect (lua_State *L)
{
    if (lua_isfunction(L, 1))
    {
        ref_t *ref = (ref_t*)os_malloc(sizeof(ref_t));
        ref->L = L;
        lua_pushvalue(L, 1); /* Store the callback */
        ref->r = luaL_ref(L, LUA_REGISTRYINDEX);

        os_timer_t *pTimer = (os_timer_t *)os_zalloc(sizeof(os_timer_t));
        ref->arg = pTimer;

        os_timer_setfn(pTimer, wifi_check_ip, ref);
        os_timer_arm(pTimer, 100, 0);
    }

    lua_pushboolean(L, wifi_station_connect());
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_disconnect (lua_State *L)
{
    lua_pushboolean(L, wifi_station_disconnect());
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_dsleep (lua_State *L)
{
    int timeus = luaL_checkinteger(L, 1);
    system_deep_sleep(timeus);
    //never return
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_wifi_sta_autoconnect (lua_State *L)
{
    lua_pushboolean(L, wifi_station_set_auto_connect(luaL_checkinteger(L, 1)));
    return 1;
}

static const luaL_Reg lnode_wifi_lib[] = {
  {"setmode",          lnode_wifi_setmode},
  {"config",           lnode_wifi_sta_config},
  {"getip",            lnode_wifi_sta_getip},
  {"getconfig",        lnode_wifi_sta_getconfig},
  {"connect",          lnode_wifi_sta_connect},
  {"disconnect",       lnode_wifi_sta_disconnect},
  {"autoconnect",      lnode_wifi_sta_autoconnect},
  {"dsleep",           lnode_wifi_sta_dsleep},
  {NULL, NULL}
};

/* }====================================================== */



LUALIB_API int luaopen_lnode_wifi (lua_State *L) {
  luaL_register(L, LUA_LNODE_WIFI_NAME, lnode_wifi_lib);
  lua_pushnumber(L, STATION);
  lua_setfield(L, -2, "STATION");
  lua_pushnumber(L, SOFTAP);
  lua_setfield(L, -2, "SOFTAP");
  lua_pushnumber(L, STATIONAP);
  lua_setfield(L, -2, "STATIONAP");

  return 1;
}

/* Testing code:

wifi.setmode(wifi.STATION)
print(wifi.config("yuyi", "yuyiapp2014"))
print(wifi.getconfig())
print(wifi.connect())
print(wifi.getip())
print(wifi.disconnect())

for i = 1,100 do
net.dns("gulumao.cn", function(err, ip) print("done!!!" .. ip) end) print(node.free()) collectgarbage()
end

print(wifi.autoconnect(0))

*/
