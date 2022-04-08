#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define NUM_SOCKETS 2

#define CONDITION_UNK_0 (1 << 0)
#define CONDITION_UNK_1 (1 << 1)
#define CONDITION_UNK_2 (1 << 2)
#define CONDITION_UNK_3 (1 << 3)
#define CONDITION_UNK_4 (1 << 4)
#define CONDITION_UNK_5 (1 << 5)
#define CONDITION_UNK_6 (1 << 6)

#define STATUS_UNK_0 (1 << 0)
#define STATUS_UNK_1 (1 << 1)
#define STATUS_UNK_2 (1 << 2)
#define STATUS_UNK_3 (1 << 3)
#define STATUS_UNK_4 (1 << 4)
#define STATUS_UNK_5 (1 << 5)
#define STATUS_UNK_6 (1 << 6)
#define STATUS_UNK_7 (1 << 7)
#define STATUS_UNK_8 (1 << 8)
#define STATUS_UNK_9 (1 << 9)
#define STATUS_UNK_10 (1 << 10)
#define STATUS_UNK_11 (1 << 11)
#define STATUS_UNK_12 (1 << 12)
#define STATUS_UNK_13 (1 << 13)

enum ma_sio_modes {
    MA_SIO_BYTE,
    MA_SIO_WORD,
    MA_NUM_SIO_MODES,
};

typedef struct {
    vu16 unk_0;
    vu16 size;
    vu16 readcnt;
    vu16 writecnt;
    vu8 *readptr;
    vu8 *writeptr;
} MA_IOBUF;

typedef struct {
    u16 size;
    void *data;
} MA_BUF;

typedef struct {
    vu8 error;
    u8 _1[1];
    vu16 condition;
    vu8 unk_4;
    vu8 sio_mode;
    vu8 adapter_type;
    u8 _7[1];
    vs16 timer[MA_NUM_SIO_MODES];
    vu16 timer_unk_12;
    vu16 unk_14;
    vu8 interval;
    u8 _17[3];
    u32 counter_null[MA_NUM_SIO_MODES];
    u32 counter_timeout[MA_NUM_SIO_MODES];
    u32 counter_p2p[MA_NUM_SIO_MODES];
    u32 counter_timeout200msec[MA_NUM_SIO_MODES];
    u32 counter_adapter[MA_NUM_SIO_MODES];
    vu32 unk_60;
    vu32 status;
    vu8 cmd_cur;
    vu8 unk_69;
    u16 unk_70;
    vu8 unk_72;
    vu8 unk_73;
    u8 _74[2];
    vu8 unk_76;
    vu8 unk_77;
    u8 _78[2];
    vu8 unk_80;
    vu8 unk_81;
    u8 unk_82;
    u8 unk_83;
    u8 unk_84[4];
    u8 _88[4];
    vu8 unk_92;
    u8 _93[4];
    vu8 task_unk_97;
    vu8 task_unk_98;
    u8 sockets[NUM_SOCKETS];
    u8 _101[5];
    u8 ipaddr[4];
    u8 _110[2];
    u8 *unk_112;
    u32 unk_112_size;
    u8 _120[84];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[6];
    u8 unk_212[268];
    MA_BUF buffer_unk_480;
    MA_IOBUF iobuf_packet_send;
    MA_IOBUF iobuf_packet_recv;
    MA_IOBUF iobuf_recv;
    u8 buffer_packet_send[32];
    u8 _568[236];
    u8 buffer_recv_data[4];
    u8 _808[4];
    MA_BUF buffer_recv;
    MA_BUF *buffer_recv_ptr;
    MA_IOBUF *iobuf_sio_tx;
    u8 _828[1064];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
