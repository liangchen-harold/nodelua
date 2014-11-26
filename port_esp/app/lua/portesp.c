/*
** $Id: loslib.c,v 1.19.1.3 2008/01/18 16:38:18 roberto Exp $
** Standard Operating System library
** See Copyright Notice in lua.h
*/


#include <errno.h>
#include <sys/time.h>

void __fputs(const char *str, int fd)
{
    os_printf("%s", str);
}

double pow(double a, double b)
{
    return 0;
}

double floor(double a)
{
    return 0;
}

double ceil(double a)
{
    return 0;
}

double sin(double a)
{
    return 0;
}

double asin(double a)
{
    return 0;
}

double sinh(double a)
{
    return 0;
}

double cos(double a)
{
    return 0;
}

double acos(double a)
{
    return 0;
}

double cosh(double a)
{
    return 0;
}

double atan(double a)
{
    return 0;
}

double atan2(double a, double b)
{
    return 0;
}

double tan(double a)
{
    return 0;
}

double tanh(double a)
{
    return 0;
}

double log(double a)
{
    return 0;
}

double log10(double a)
{
    return 0;
}

double sqrt(double a)
{
    return 0;
}

double exp(double a)
{
    return 0;
}

double fmod(double a, double b)
{
    return 0;
}

int _rename_r(const char *filename, const char *to)
{
    return -1;
}

int _unlink_r(const char *filename)
{
    return -1;
}

void _exit(int status)
{
    while(1);
}

int _open_r(const char *filename)
{
    return -1;
}

int _close_r(int fd)
{
    return -1;
}

int _read_r(int fd)
{
    return -1;
}

int _write_r(int fd)
{
    return -1;
}

int _lseek_r(int fd)
{
    return -1;
}

int _fstat_r(int fd)
{
    return -1;
}

long _times_r()
{
    return 0;
}

int _gettimeofday_r(struct timeval *tv, struct timezone *tz)
{
    return -1;
}

int _getpid_r()
{
    return -1;
}

int _kill_r(int pid)
{
    return -1;
}

char *getenv(const char *key)
{
    return 0;
}

int _sbrk_r(int size)
{
}
