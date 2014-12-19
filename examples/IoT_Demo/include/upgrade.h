#ifndef __UPGRADE_H__
#define __UPGRADE_H__

#define SPI_FLASH_SEC_SIZE      4096

#define USER_BIN1               0x00
#define USER_BIN2               0x01

#define UPGRADE_FLAG_IDLE       0x00
#define UPGRADE_FLAG_START      0x01
#define UPGRADE_FLAG_FINISH     0x02

typedef void (*upgrade_states_check_callback)(void * arg);

//#define UPGRADE_SSL_ENABLE

struct upgrade_server_info {
    uint8 ip[4];
    uint16 port;

    uint8 upgrade_flag;

    uint8 pre_version[8];
    uint8 upgrade_version[8];

    uint32 check_times;
    uint8 *url;

    upgrade_states_check_callback check_cb;
    struct espconn *pespconn;
};

#ifdef UPGRADE_SSL_ENABLE
bool system_upgrade_start_ssl(struct upgrade_server_info *server);
#else
bool system_upgrade_start(struct upgrade_server_info *server);
#endif
#endif
