/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <ets_sys.h>
#include <os_type.h>
#include <gpio.h>

#define loslib_c
#define LUA_LIB

#include "lua/lua.h"
#include "lua/portesp.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua/lnode_gpio.h"


static int ICACHE_FLASH_ATTR w1_reset (int pin)
{
    uint8_t r;
    uint8_t retries = 125;

    //Set GPIO pin to output mode
    PIN_FUNC_SELECT(pins[pin].mux, pins[pin].func);
    GPIO_DIS_OUTPUT(pin);
    PIN_PULLUP_EN(pins[pin].mux);

    // wait until the wire is high
    while (GPIO_INPUT_GET(pin) == 0 && retries > 0)
    {
        os_delay_us(2);
        retries --;
    }

    //Set GPIO pin to output mode
    PIN_FUNC_SELECT(pins[pin].mux, pins[pin].func);
    GPIO_OUTPUT_SET(pin, 0);
    os_delay_us(480);
    GPIO_DIS_OUTPUT(pin);
    PIN_PULLUP_EN(pins[pin].mux);
    os_delay_us(70);
    r = !GPIO_INPUT_GET(pin);
    os_delay_us(410);
    return r;
}

static void ICACHE_FLASH_ATTR w1_write_bit(int pin, uint8_t v)
{
    //Set GPIO pin to output mode
    PIN_FUNC_SELECT(pins[pin].mux, pins[pin].func);

	if (v & 1) {
		GPIO_OUTPUT_SET(pin, 0);
		os_delay_us(10);
		GPIO_OUTPUT_SET(pin, 1);
		os_delay_us(55);
	} else {
		GPIO_OUTPUT_SET(pin, 0);
		os_delay_us(65);
		GPIO_OUTPUT_SET(pin, 1);
		os_delay_us(5);
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static int ICACHE_FLASH_ATTR w1_read_bit(int pin)
{
	uint8_t r;

    //Set GPIO pin to output mode
    PIN_FUNC_SELECT(pins[pin].mux, pins[pin].func);

	GPIO_OUTPUT_SET(pin, 0);
	os_delay_us(3);
    GPIO_DIS_OUTPUT(pin);
    PIN_PULLUP_EN(pins[pin].mux);
	os_delay_us(10);
    r = GPIO_INPUT_GET(pin);
	os_delay_us(53);
	return r;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
static void ICACHE_FLASH_ATTR w1_write(int pin, uint8_t v, uint8_t power /* = 0 */) {
    uint8_t bitMask;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        w1_write_bit(pin, (bitMask & v)?1:0);
    }
    if ( !power) {
        GPIO_DIS_OUTPUT(pin);
        PIN_PULLUP_EN(pins[pin].mux);
    }
}

//
// Read a byte
//
static int ICACHE_FLASH_ATTR w1_read(int pin) {
    uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
        if ( w1_read_bit(pin)) r |= bitMask;
    }
    return r;
}

// Send addr to match all devices
static void ICACHE_FLASH_ATTR w1_skip(int pin) {
    w1_write(pin, 0xCC, 0);
}

static uint8_t ICACHE_FLASH_ATTR w1_crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
    uint8_t i;

	while (len--) {
		uint8_t inbyte = *addr++;
		for (i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}


static int ICACHE_FLASH_ATTR lnode_sensor_1820 (lua_State *L)
{
    int i;
    uint8_t buf[9];
    int pin = luaL_checkinteger(L, 1);
    w1_reset(pin);
    w1_skip(pin);
    w1_write(pin, 0x44, 0);
	os_delay_us(750000);
    int found = w1_reset(pin);
    w1_skip(pin);
    w1_write(pin, 0xBE, 0);
    for (i = 0; i < 9; i ++)
    {
        buf[i] = w1_read(pin);
    }
    uint8_t crc = w1_crc8(buf, 8);
    int tempi = (buf[0] + buf[1] * 256) * 625;
    //__printf("found=%d crc=0x%02X(r:0x%02X) t=%d.%04d\n", found, buf[8], crc, tempi/10000, tempi%10000);
    float temp = tempi/10000.f;

    if (crc == buf[8] && found)
    {
        lua_pushnil(L);
        lua_pushnumber(L, temp);
        return 2;
    }
    else if(found)
    {
        lua_pushstring(L, "Crc err");
        return 1;
    }
    lua_pushstring(L, "Not fount");
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_sensor_dht (lua_State *L, int model)
{
    #define BREAKTIME 20
    #define MAXTIMINGS 10000

    //Set GPIO4 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);

    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;
    //int bitidx = 0;
    //int bits[250];

    int data[6];

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    GPIO_OUTPUT_SET(4, 1);
    os_delay_us(250000);
    GPIO_OUTPUT_SET(4, 0);
    os_delay_us(20000);
    GPIO_OUTPUT_SET(4, 1);
    os_delay_us(40);
    GPIO_DIS_OUTPUT(4);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);


    // wait for pin to drop?
    while (GPIO_INPUT_GET(4) == 1 && i<100000)
    {
        os_delay_us(1);
        i++;
    }

    if(i==100000)
    {
        lua_pushstring(L, "Not found");
        return 1;
    }

    // read data!

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while ( GPIO_INPUT_GET(4) == laststate) {
            counter++;
            os_delay_us(1);
            if (counter == 1000)
                break;
        }
        laststate = GPIO_INPUT_GET(4);
        if (counter == 1000) break;

        //bits[bitidx++] = counter;

        if ((i>3) && (i%2 == 0)) {
            // shove each bit into the storage bytes
            data[j/8] <<= 1;
            if (counter > BREAKTIME)
                data[j/8] |= 1;
            j++;
        }
    }

/*
    for (i=3; i<bitidx; i+=2) {
        os_printf("bit %d: %d\n", i-3, bits[i]);
        os_printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > BREAKTIME);
    }
    os_printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);
*/
    float temp_p, hum_p;
    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            /* yay! checksum is valid */
            if (model == 22) //DHT22
            {
                hum_p = data[0] * 256 + data[1];
                hum_p /= 10;

                temp_p = (data[2] & 0x7F)* 256 + data[3];
                temp_p /= 10.0;
                if (data[2] & 0x80)
                    temp_p *= -1;
            }
            else //DHT11
            {
                hum_p = data[0];
                temp_p = data[2];
            }

            lua_pushnil(L);
            lua_pushnumber(L, temp_p);
            lua_pushnumber(L, hum_p);
            return 3;
        }
        else
        {
            lua_pushstring(L, "Crc err");
            return 1;
        }
    }

    lua_pushstring(L, "Len err");
    return 1;
}

static int ICACHE_FLASH_ATTR lnode_sensor_dht11 (lua_State *L)
{
    return lnode_sensor_dht(L, 11);
}

static int ICACHE_FLASH_ATTR lnode_sensor_dht22 (lua_State *L)
{
    return lnode_sensor_dht(L, 22);
}

static const luaL_Reg lnode_sensor_lib[] = {
  {"t1820",          lnode_sensor_1820},
  {"dht11",          lnode_sensor_dht11},
  {"dht22",          lnode_sensor_dht22},
  {NULL, NULL}
};

/* }====================================================== */


LUALIB_API int luaopen_lnode_sensor (lua_State *L) {
    luaL_register(L, LUA_LNODE_SENSOR_NAME, lnode_sensor_lib);
    return 1;
}
