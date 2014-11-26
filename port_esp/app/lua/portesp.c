/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <errno.h>
#include <sys/time.h>
#include <math.h>

#include "lua/lua.h"

double ICACHE_FLASH_ATTR __strtod(const char* str, char** endptr)
{
    os_printf("__strtod: %s\n", str);
    return strtol(str, endptr);
}
char * ICACHE_FLASH_ATTR __fgets(const char *buf, int num, FILE *fd)
{

}
void ICACHE_FLASH_ATTR __fputs(const char *str, FILE *fd)
{
    os_printf("%s", str);
}
void ICACHE_FLASH_ATTR __tiny_fload_to_string(char *buf, const char *fmt, float val)
{
    os_sprintf(buf, "%d", (int)val);
    // char * dtoa(char *s, double n);
    //
    // dtoa(buf, val);
}






double ICACHE_FLASH_ATTR pow(double a, double b)
{
    return 0;
}

double ICACHE_FLASH_ATTR floor(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR ceil(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR sin(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR asin(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR sinh(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR cos(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR acos(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR cosh(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR atan(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR atan2(double a, double b)
{
    return 0;
}

double ICACHE_FLASH_ATTR tan(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR tanh(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR exp(double a)
{
    return 0;
}

double ICACHE_FLASH_ATTR fmod(double a, double b)
{
    return 0;
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
