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

static void ICACHE_FLASH_ATTR user_on_http_response(int status, char *data, void *arg)
{
	if (status == 200)
	{
		__printf("Loading, free mem=%d\n", system_get_free_heap_size());

		luainit(data);

		__printf("Loaded, free mem=%d\n", system_get_free_heap_size());

		cloud_data_append("d736b312f168c3b2", "1", "", "", NULL, NULL);
	}
	else
	{
		cloud_data_append("d736b312f168c3b2", "0", "", "", NULL, NULL);
	}
}

void ICACHE_FLASH_ATTR fetch_code_from_cloud()
{
	char buf0[512], buf1[512];
	os_sprintf(buf1, CLOUD_FETCH_URL, miid, security);
	os_sprintf(buf0, HTTP_QUERY, buf1);

	http_get(CLOUD_HOST, buf0, user_on_http_response, NULL);
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
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {
            //user_esp_platform_reset_mode();
        } else {
            os_timer_setfn(&check_sta_timer, (os_timer_func_t *)user_check_ip, NULL);
            os_timer_arm(&check_sta_timer, 100, 0);
        }
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
	uint32 buf[256];

    // rom use 74880 baut_rate, here reinitialize
    uart_init(BIT_RATE_115200, BIT_RATE_115200);

	__printf("SDK version:%d.%d.%d free mem=%d\n", SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_REVISION, system_get_free_heap_size());

	SpiFlashOpResult succ = spi_flash_read(0x70000, buf, sizeof(buf));
	os_printf("load %s: %s", succ == SPI_FLASH_RESULT_OK? "succ" : "fail", (char*)buf);

	// char *argv[] = {"lua", "-e", (char*)buf};
	// for(i = 0; 1; i ++)
	// {
	// 	uint16_t val = system_adc_read();
	// 	__printf("adc=%d\n", val);
	// 	luamain(sizeof(argv)/sizeof(char*), argv);
	// }

	os_timer_disarm(&check_sta_timer);
	os_timer_setfn(&check_sta_timer, (os_timer_func_t *)user_check_ip, 1);
	os_timer_arm(&check_sta_timer, 100, 0);

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
