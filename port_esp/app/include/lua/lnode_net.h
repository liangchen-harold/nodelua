
#ifndef lnode_net_h
#define lnode_net_h


enum Protocol {
    TCP = 0,
    UDP,
};

typedef struct {
    lua_State* L;
    int r;
    void *arg;
} ref_t;

extern ip_addr_t dummy_ip;

void lua_checkself(lua_State *L);

#endif
