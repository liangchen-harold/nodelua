/*
** $Id: lapi.h,v 2.2.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef portesp_h
#define portesp_h

#include <osapi.h>
#include <os_type.h>
#include <queue.h>
#include <stdio.h>
#include "portesp_math.h"

#define nodelua_luaProcTaskPrio        0
#define nodelua_luaProcTaskQueueLen    1
os_event_t    nodelua_luaProcTaskQueue[nodelua_luaProcTaskQueueLen];

#define min(a,b) ( (a)<(b)?(a):(b) )

char *__fgets(char *buf, int num, FILE *fd);
void __fputs(const char *str, FILE *fd);

#endif
