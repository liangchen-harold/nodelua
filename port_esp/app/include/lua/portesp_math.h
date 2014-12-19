/*
** $Id: lapi.h,v 2.2.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef portesp_math_h
#define portesp_math_h

double __strtod(const char* str, char** endptr);
void __tiny_fload_to_string(char *buf, const char *fmt, float val);

#endif
