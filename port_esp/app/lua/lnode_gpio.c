/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <os_type.h>
#include <gpio.h>

#define loslib_c
#define LUA_LIB

#include "lua/lnode_gpio.h"
#include "lua/lua.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

enum Mode {
    INPUT = 0,
    OUTPUT,
    INT,
};
enum Value {
    LOW = 0,
    HIGH,
};

const struct Pin pins[] = {
    {PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0},
    {PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1}, //TXD
    {PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2},
    {PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3}, //RXD
    {PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4},
    {PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5},
    {0, 0},
    {0, 0},
    {0, 0},
    {PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9}, //DANGEROUS
    {PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10}, //DANGEROUS
    {0, 0},
    {PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12},
    {PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13},
    {PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14},
    {PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15},
};

static int ICACHE_FLASH_ATTR check_pin(int pin)
{
    if ((pin >= 0 && pin <= 5) || (pin == 9) || (pin == 10) || (pin >= 12 && pin <= 15))
    {
        return 1;
    }
    return 0;
}
static int ICACHE_FLASH_ATTR lnode_gpio_mode (lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);
    if (check_pin(pin))
    {
        PIN_FUNC_SELECT(pins[pin].mux, pins[pin].func);
    }
    else
    {
        return luaL_error(L, "invalid pin");
    }
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_gpio_write (lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);
    int val = luaL_checkinteger(L, 2);
    if (check_pin(pin))
    {
        GPIO_OUTPUT_SET(GPIO_ID_PIN(pin), val);
    }
    else
    {
        return luaL_error(L, "invalid pin");
    }
    return 0;
}

static int ICACHE_FLASH_ATTR lnode_gpio_read (lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);
    if (check_pin(pin))
    {
        lua_pushinteger(L, GPIO_INPUT_GET(GPIO_ID_PIN(pin)));
        return 1;
    }
    else
    {
        return luaL_error(L, "invalid pin");
    }
}

static const luaL_Reg lnode_gpio_lib[] = {
  {"mode",          lnode_gpio_mode},
  {"read",          lnode_gpio_read},
  {"write",         lnode_gpio_write},
  {NULL, NULL}
};

/* }====================================================== */



LUALIB_API int luaopen_lnode_gpio (lua_State *L) {
  luaL_register(L, LUA_LNODE_GPIO_NAME, lnode_gpio_lib);
  lua_pushnumber(L, HIGH);
  lua_setfield(L, -2, "HIGH");
  lua_pushnumber(L, LOW);
  lua_setfield(L, -2, "LOW");
  lua_pushnumber(L, INPUT);
  lua_setfield(L, -2, "INPUT");
  lua_pushnumber(L, OUTPUT);
  lua_setfield(L, -2, "OUTPUT");

  gpio_init();

  return 1;
}

/* Testing code:
for i = 1,10000 do print(i .. ":" .. node.free()) node.wdt() collectgarbage() end
for i = 1,10000 do node.wdt() collectgarbage() end

print(node.free()) collectgarbage() print(node.free())

t={{0,0}, {2,0}, {4,0}, {5,0}, {12,0}, {13,0}, {14,0}, {15,1}}
table.foreach(t, function(j, v) gpio.mode(v[1],gpio.OUTPUT) gpio.write(v[1], v[2]) end)

t={{0,0}, {2,0}, {4,0}, {5,0}, {12,0}, {13,0}, {14,0}, {15,1}}
for i = 1,10000 do print(i .. ":" .. node.free()) node.wdt() collectgarbage()
table.foreach(_G, function(j, v) gpio.mode(v[1],gpio.OUTPUT) gpio.write(1, i%2) end)
end
*/
