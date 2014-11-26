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
#include "driver/uart.h"
//#include "lua/lua.h"
//#include "lua/luaconf.h"
#include <math.h>

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

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
	os_printf("SDK version:%d.%d.%d\n", SDK_VERSION_MAJOR, SDK_VERSION_MINOR, SDK_VERSION_REVISION);
	//for(i = 0; 1; i ++)
	{
		uint16_t val = system_adc_read();
		os_printf("adc=%d\n", val);
	}
	int luamain (int argc, char **argv);
	char *argv[] = {"lua", "-e", "print('OK!')"};
	for(i = 0; 1; i ++)
	{
		luamain(3, argv);
	}
	//ceil(luaL_checknumber(NULL, 1));
	//double v1, v2, r;
	//sprintf(argv[0], "", cos(sin(v1)));

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
