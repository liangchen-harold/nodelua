/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <errno.h>
#include <sys/time.h>
#include <math.h>

#include "ets_sys.h"
#include "driver/uart.h"

#define lua_c
#include "lua/lua.h"

void ICACHE_FLASH_ATTR __printf(const char *fmt, ...)
{
	static char buf[256];
    va_list argptr;
    va_start(argptr,fmt);
	ets_vsprintf(buf, fmt, argptr);
    va_end(argptr);
	uart0_tx_buffer(buf, os_strlen(buf));
	//os_printf("%s", buf);
}

static char readline_line[LUA_MAXINPUT];
char * ICACHE_FLASH_ATTR __fgets(char *buf, int num, FILE *fd)
{
    int i;
    if (readline_line != NULL)
    {
        //__printf("__fgets: %s\n", readline_line);
        char *p = readline_line;
        char *k = buf;
        for (i = 0; i < num-2; i ++)
        {
            *k = *p;
            if (*p == '\r' || *p == 0)
            {
                break;
            }
            k ++;
            p ++;
        }
        *(k) = '\r';
        *(k+1) = 0;
    }
    return buf;
}
void ICACHE_FLASH_ATTR __fputs(const char *str, FILE *fd)
{
	uart0_tx_buffer(str, os_strlen(str));
	//os_printf("%s", str);
}

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */
void ICACHE_FLASH_ATTR
nodelua_recvLine(char *line)
{
    int len = min(strlen(line), LUA_MAXINPUT-1);
    os_memcpy(readline_line, line, len);
    readline_line[len] = 0;
    system_os_post(nodelua_luaProcTaskPrio, 0, 0);
}

int ICACHE_FLASH_ATTR _unlink_r(const char *filename)
{
    return -1;
}

void ICACHE_FLASH_ATTR _exit(int status)
{
    while(1);
}

int ICACHE_FLASH_ATTR _open_r(const char *filename)
{
    return -1;
}

int ICACHE_FLASH_ATTR _close_r(int fd)
{
    return -1;
}

int ICACHE_FLASH_ATTR _read_r(int fd)
{
    return -1;
}

int ICACHE_FLASH_ATTR _write_r(int fd)
{
    return -1;
}

int ICACHE_FLASH_ATTR _lseek_r(int fd)
{
    return -1;
}

int ICACHE_FLASH_ATTR _fstat_r(int fd)
{
    return -1;
}

long ICACHE_FLASH_ATTR _times_r()
{
    return 0;
}

int ICACHE_FLASH_ATTR _gettimeofday_r(struct timeval *tv, struct timezone *tz)
{
    return -1;
}

int ICACHE_FLASH_ATTR _getpid_r()
{
    return -1;
}

int ICACHE_FLASH_ATTR _kill_r(int pid)
{
    return -1;
}

char *ICACHE_FLASH_ATTR getenv(const char *key)
{
    return 0;
}

int ICACHE_FLASH_ATTR _sbrk_r(int size)
{
}
