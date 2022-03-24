#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define NUM_SOCKETS 2

#define MATYPE_PROT_MASTER 0x80
#define MATYPE_PROT_SLAVE 0x88

enum ma_sio_modes {
    MA_SIO_BYTE,
    MA_SIO_WORD,
    MA_NUM_SIO_MODES,
};

typedef struct {
    u8 _0[5];
    vu8 sio_mode;
    vu8 adapter_type;
    u8 _7[1];
    vs16 timer[MA_NUM_SIO_MODES];
    u8 _12[4];
    vu8 interval;
    u8 _17[3];
    vu32 counter_null[MA_NUM_SIO_MODES];
    u8 _28[8];
    vu32 counter_p2p[MA_NUM_SIO_MODES];
    vu32 counter_timeout200msec[MA_NUM_SIO_MODES];
    vu32 counter_adapter[MA_NUM_SIO_MODES];
    u8 _60[39];
    u8 sockets[NUM_SOCKETS];
    u8 _101[5];
    u8 ipaddr[4];
    u8 _110[94];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[1686];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
