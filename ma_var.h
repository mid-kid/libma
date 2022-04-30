#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define MA_CONDITION_UNK_5 (1 << 5)
#define MA_CONDITION_UNK_6 (1 << 6)

#define MAAPIE_UNK_83 0x83
#define MAAPIE_UNK_84 0x84
#define MAAPIE_UNK_85 0x85

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
#define STATUS_UNK_14 (1 << 14)

#define TASK_UNK_00 0x00
#define TASK_UNK_01 0x01
#define TASK_UNK_02 0x02
#define TASK_UNK_05 0x05
#define TASK_UNK_06 0x06
#define TASK_UNK_07 0x07
#define TASK_UNK_08 0x08
#define TASK_UNK_09 0x09
#define TASK_UNK_0E 0x0e
#define TASK_UNK_10 0x10
#define TASK_UNK_15 0x15
#define TASK_UNK_1C 0x1c
#define TASK_UNK_1E 0x1e
#define TASK_UNK_23 0x23

#define MACMD_START 0x10
#define MACMD_END 0x11
#define MACMD_TEL 0x12
#define MACMD_OFFLINE 0x13
#define MACMD_WAITCALL 0x14
#define MACMD_DATA 0x15
#define MACMD_REINIT 0x16
#define MACMD_CHECKSTATUS 0x17
#define MACMD_CHANGECLOCK 0x18
#define MACMD_EEPROM_READ 0x19
#define MACMD_EEPROM_WRITE 0x1a
#define MACMD_UNK_1F 0x1f
#define MACMD_PPPCONNECT 0x21
#define MACMD_PPPDISCONNECT 0x22
#define MACMD_TCPCONNECT 0x23
#define MACMD_TCPDISCONNECT 0x24
#define MACMD_UDPCONNECT 0x25
#define MACMD_UDPDISCONNECT 0x26
#define MACMD_DNSREQUEST 0x28
#define MACMD_TESTMODE 0x3f
#define MACMD_ERROR 0x6e

#define NUM_SOCKETS 2

enum ma_sio_modes {
    MA_SIO_BYTE,
    MA_SIO_WORD,
    MA_NUM_SIO_MODES,
};

typedef struct {
    vu16 state;
    vu16 size;
    vu16 readcnt;
    vu16 checksum;
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
    vu8 intr_sio_mode;
    vu8 sio_mode;
    vu8 adapter_type;
    vu8 unk_7;
    vu16 timer[MA_NUM_SIO_MODES];
    vu16 timer_unk_12;
    vu16 unk_14;
    vu8 interval;
    u8 _17[3];
    u32 counter_null[MA_NUM_SIO_MODES];
    u32 counter_timeout[MA_NUM_SIO_MODES];
    u32 counter_p2p[MA_NUM_SIO_MODES];
    u32 counter_timeout200msec[MA_NUM_SIO_MODES];
    u32 counter_adapter[MA_NUM_SIO_MODES];
    vu32 counter;
    vu32 status;
    vu8 cmd_cur;
    vu8 recv_cmd;
    u16 recv_checksum;
    vu8 send_footer[4];
    vu8 recv_footer[4];
    vu8 unk_80;
    vu8 unk_81;
    u8 recv_garbage_counter;
    u8 unk_83;
    u8 unk_84[4];
    u32 unk_88;
    vu8 unk_92;
    u8 _93[1];
    vu16 unk_94;
    vu8 unk_96;
    vu8 task;
    vu8 task_unk_98;
    u8 sockets[NUM_SOCKETS];
    u8 unk_101;
    u8 unk_102;
    u8 _103[1];
    u16 unk_104;
    u8 ipaddr[4];
    u8 _110[2];
    u8 *unk_112;
    u32 unk_116;
    u8 _120[84];
    u8 sockets_used[NUM_SOCKETS];
    u8 _206[6];
    u8 unk_212[268];
    MA_BUF buffer_unk_480;
    MA_IOBUF iobuf_packet_send;
    MA_IOBUF iobuf_packet_recv;
    MA_IOBUF iobuf_footer;
    u8 buffer_packet_send[32];
    u8 _568[236];
    u8 buffer_recv_data[4];
    u8 buffer_footer[4];
    MA_BUF buffer_recv;
    MA_BUF *buffer_recv_ptr;
    MA_IOBUF *iobuf_sio_tx;
    u8 _828[8];
    u8 smtp_server[20];
    u8 pop3_server[20];
    u8 _876[273];
    u8 prevbuf[1];
    u8 _1150[636];
    u16 prevbuf_size;
    u8 unk_1788[6];
    u16 unk_1794;
    u16 unk_1796;
    u8 _1798[94];
} MA_VAR;

extern MA_VAR gMA;

#endif // _MA_VAR_H
