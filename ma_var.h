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
    vu16 unk_0;
    vu16 unk_2;
    vu16 readcnt;
    vu16 writecnt;
    vu8 *readptr;
    vu8 *writeptr;
} MA_IOBUF;

typedef struct {
    vu8 error;
    u8 _1[1];
    vu16 condition;  // FLAGS
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
    u32 counter_timeout[MA_NUM_SIO_MODES];
    u32 counter_p2p[MA_NUM_SIO_MODES];
    u32 counter_timeout200msec[MA_NUM_SIO_MODES];
    u32 counter_adapter[MA_NUM_SIO_MODES];
    u32 unk_60;
    vu32 status;  // FLAGS
    vu8 unk_68;
    vu8 unk_69;
    u16 unk_70;
    vu8 unk_72;
    vu8 unk_73;
    u8 _74[2];
    vu8 unk_76;
    vu8 unk_77;
    u8 _78[14];
    vu8 unk_92;
    u8 _93[4];
    vu8 unk_97;
    u8 _98[1];
    u8 sockets[NUM_SOCKETS];
    u8 _101[5];
    u8 ipaddr[4];
    u8 _110[94];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[6];
    u8 unk_212[268];
    u16 unk_480;
    u8 _482[2];
    void *unk_484;
    vu16 unk_488;
    u8 _490[14];
    vu16 unk_504;
    u8 _506[298];
    u8 buffer_recv[4];
    u8 _808[4];
    u16 buffer_recv_size;
    u8 _814[2];
    void *buffer_recv_ptr;
    void *buffer_recv_unk;
    MA_IOBUF *iobuf_sio_tx;
    u8 _828[1064];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
