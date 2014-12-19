
#ifndef lnode_gpio_h
#define lnode_gpio_h


struct Pin
{
    uint32_t mux;
    uint8_t func;
};
extern const struct Pin pins[];

#endif
