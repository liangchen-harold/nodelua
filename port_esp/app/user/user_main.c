/******************************************************************************
 * Copyright 2014 Nodelua.org (chenliang)
 *
*******************************************************************************/
#include "ets_sys.h"
#include <os_type.h>
#include <osapi.h>
#include "user_interface.h"
#include <espconn.h>
#include <mem.h>
#include "driver/uart.h"
#include "lua/lua.h"
#include "cloud/data.h"
#include "lua/luaconf.h"
#include <math.h>


#include "cloud/data.h"

#include "version.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#ifdef SERVER_SSL_ENABLE
#include "ssl/cert.h"
#include "ssl/private_key.h"
#else
#ifdef CLIENT_SSL_ENABLE
unsigned char *default_certificate;
unsigned int default_certificate_len = 0;
unsigned char *default_private_key;
unsigned int default_private_key_len = 0;
#endif
#endif

LOCAL os_timer_t check_sta_timer;
void ICACHE_FLASH_ATTR user_check_ip(uint8 reset_flag);

static void ICACHE_FLASH_ATTR user_on_http_response(int status, char *data, void *arg)
{
	if (status == 200)
	{
		DEBUG_MSG("Loading, free mem=%d\n", system_get_free_heap_size());

		luainit(data);

		DEBUG_MSG("Loaded, free mem=%d\n", system_get_free_heap_size());
	}
	else
	{
		//Retry after 10 seconds
		os_timer_disarm(&check_sta_timer);
		os_timer_setfn(&check_sta_timer, (os_timer_func_t *)user_check_ip, 1);
		os_timer_arm(&check_sta_timer, 10000, 0);
	}
}

void ICACHE_FLASH_ATTR fetch_code_from_cloud()
{
	char buf0[512], buf1[512];

	if ( user_params_get(buf0, sizeof(buf0)) )
	{
		buf0[sizeof(buf0) - 1] = 0;
		char *p = (char*)os_strstr(buf0, ";");
		if (p != NULL && p != buf0)
		{
			p[0] = 0;
			char *q = (char*)os_strstr(p + 1, ";");
			if (q != NULL && q != p + 1)
			{
				q[0] = 0;
				_miid = (char*)os_malloc(os_strlen(buf0) + 1);
				_security = (char*)os_malloc(os_strlen(p + 1) + 1);
				os_strcpy(_miid, buf0);
				os_strcpy(_security, p + 1);

				__printf("Node:%s\n", _miid);

				os_sprintf(buf1, CLOUD_FETCH_URL, _miid, _security);
				os_sprintf(buf0, HTTP_QUERY, buf1);

				http_get(CLOUD_HOST, buf0, user_on_http_response, NULL);
				return;
			}
		}
	}

	__printf("Please set node ID!\n");
	luainit("");
}

void ICACHE_FLASH_ATTR user_check_ip(uint8 reset_flag)
{
    struct ip_info ipconfig;

    os_timer_disarm(&check_sta_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
	{
		fetch_code_from_cloud();
    }
	else
	{
        /* if there are wrong while connecting to some AP, then reset mode */
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
		{
			//TODO: reconfig wifi!!!
            //user_esp_platform_reset_mode();
        } else {
            os_timer_setfn(&check_sta_timer, (os_timer_func_t *)user_check_ip, NULL);
            os_timer_arm(&check_sta_timer, 100, 0);
        }
    }
}

bool ICACHE_FLASH_ATTR user_params_get(char *valbuf, int buflen)
{
	SpiFlashOpResult succ = spi_flash_read(0x10000-(SPI_FLASH_SEC_SIZE), (uint32*)valbuf, buflen);
	if (succ == SPI_FLASH_RESULT_OK)
	{
		DEBUG_MSG("user_params_get: (0x%02X) %s\n", valbuf[0], valbuf);
		return true;
	}
	return false;
}

void ICACHE_FLASH_ATTR user_params_set(char *valbuf, int buflen)
{
	spi_flash_erase_sector(0x10000 / (SPI_FLASH_SEC_SIZE) - 1);
	SpiFlashOpResult succ = spi_flash_write(0x10000-(SPI_FLASH_SEC_SIZE), (uint32 *)valbuf, buflen);
	if (succ == SPI_FLASH_RESULT_OK)
	{
		DEBUG_MSG("user_params_set: (len=%d) %s\n", buflen, valbuf);
		os_memset(valbuf, 0x00, buflen);
		user_params_get(valbuf, buflen);
	}
	else
	{
		__printf("failed to set user params! %d\n", succ);
	}
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_init(void)
{
	int i;

    // rom use 74880 baut_rate, here reinitialize
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

	uart_tx_one_char('\r');
	__printf("NodeLua %s (With liblua %s) Copyright (C) 2014 NodeLua.org free mem=%d\n", NODELUA_RELEASE NODELUA_RELEASE_LOC, LUA_RELEASE, system_get_free_heap_size());

	// char *argv[] = {"lua", "-e", (char*)buf};
	// for(i = 0; 1; i ++)
	// {
	// 	uint16_t val = system_adc_read();
	// 	__printf("adc=%d\n", val);
	// 	luamain(sizeof(argv)/sizeof(char*), argv);
	// }

    struct station_config config;
    wifi_station_get_config(&config);
	if (wifi_get_opmode() == STATION_MODE && os_strlen(config.ssid) > 0)
	{
		os_timer_disarm(&check_sta_timer);
		os_timer_setfn(&check_sta_timer, (os_timer_func_t *)user_check_ip, 1);
		os_timer_arm(&check_sta_timer, 100, 0);
	}
	else
	{
		__printf("Please set wifi parameters to connect to an exist AP!\n");
		luainit("");
	}

	return;

// #if ESP_PLATFORM
//     user_esp_platform_init();
// #endif
//
//     user_devicefind_init();
// #ifdef SERVER_SSL_ENABLE
//     user_webserver_init(SERVER_SSL_PORT);
// #else
//     user_webserver_init(SERVER_PORT);
// #endif
}
