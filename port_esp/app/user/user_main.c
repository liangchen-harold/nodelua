/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
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


static void ICACHE_FLASH_ATTR user_on_http_response(int status, char *data)
{
	if (status == 200)
	{
		luainit(data);
		cloud_data_append("d736b312f168c3b2", "1", "", "");
	}
	else
	{
		cloud_data_append("d736b312f168c3b2", "0", "", "");
	}
}

void ICACHE_FLASH_ATTR fetch_code_from_cloud()
{
	char buf0[512], buf1[512];
	os_sprintf(buf1, CLOUD_FETCH_URL, miid, security);
	os_sprintf(buf0, HTTP_QUERY, buf1);

	http_get(CLOUD_HOST, buf0, user_on_http_response);
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

	__printf("SDK version:%d.%d.%d\n", SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_REVISION);

	SpiFlashOpResult succ = spi_flash_read(0x70000, buf, sizeof(buf));
	os_printf("load %s: %s", succ == SPI_FLASH_RESULT_OK? "succ" : "fail", (char*)buf);

	// char *argv[] = {"lua", "-e", (char*)buf};
	// for(i = 0; 1; i ++)
	// {
	// 	uint16_t val = system_adc_read();
	// 	__printf("adc=%d\n", val);
	// 	luamain(sizeof(argv)/sizeof(char*), argv);
	// }

	fetch_code_from_cloud();

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
