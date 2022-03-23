#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define NUM_SOCKETS 2

typedef struct {
    u8 _0[99];
    u8 sockets[NUM_SOCKETS];
    u8 _101[5];
    u8 ipaddr[4];
    u8 _110[94];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[1686];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
