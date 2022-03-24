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

// TODO: Try to disable as many volatiles as we can
typedef struct {
    u8 unk_0;
    u8 _1;
    vu16 unk_2;
    vu8 unk_4;
    vu8 sio_mode;
    vu8 adapter_type;
    u8 _7[1];
    vs16 timer[MA_NUM_SIO_MODES];
    vu16 unk_12;
    vu16 unk_14;
    vu8 interval;
    u8 _17[3];
    u32 counter_null[MA_NUM_SIO_MODES];
    u8 _28[8];
    u32 counter_p2p[MA_NUM_SIO_MODES];
    u32 counter_timeout200msec[MA_NUM_SIO_MODES];
    u32 counter_adapter[MA_NUM_SIO_MODES];
    u32 unk_60;
    u32 unk_64;
    vu8 unk_68;
    vu8 unk_69;
    u16 unk_70;
    vu8 unk_72;
    vu8 unk_73;
    u8 _74[2];
    vu8 unk_76;
    vu8 unk_77;
    u8 _78[21];
    u8 sockets[NUM_SOCKETS];
    u8 _101[5];
    u8 ipaddr[4];
    u8 _110[94];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[6];
    u8 unk_212[1];
    u8 _213[267];
    u16 unk_480;
    u8 _482[2];
    void *unk_484;
    u8 _488[316];
    u8 unk_804[1];
    u8 _805[7];
    u16 unk_812;
    u8 _814[2];
    void *unk_816;
    u8 _820[4];
    u32 unk_824;
    u8 _828[1064];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
