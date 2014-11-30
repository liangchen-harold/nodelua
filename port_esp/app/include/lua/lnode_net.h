/*
** $Id: lmem.h,v 1.31.1.1 2007/12/27 13:02:25 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#ifndef lnode_net_h
#define lnode_net_h


enum Protocol {
    TCP = 0,
    UDP,
};

extern ip_addr_t dummy_ip;

void lua_checkself(lua_State *L);

#endif
