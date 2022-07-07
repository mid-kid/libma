#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define MA_CONDITION_UNK_5 (1 << 5)
#define MA_CONDITION_UNK_6 (1 << 6)

#define MAAPIE_UNK_83 0x83
#define MAAPIE_UNK_84 0x84
#define MAAPIE_UNK_85 0x85
#define MAAPIE_UNK_87 0x87

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
#define STATUS_UNK_15 (1 << 15)
#define STATUS_UNK_16 (1 << 16)

#define MAPROT_REPLY 0x80

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

#define EEPROM_TELNO_SIZE 8
#define NUM_SOCKETS 2

enum tasks {
    TASK_NONE,
    TASK_INITLIBRARY,
    TASK_INITLIBRARY2,
    TASK_TELSERVER,
    TASK_TEL,
    TASK_RECEIVE,
    TASK_SDATA,
    TASK_GDATA,
    TASK_CONDITION,
    TASK_CONDITION2,
    TASK_OFFLINE,
    TASK_SMTP_CONNECT,
    TASK_SMTP_SENDER,
    TASK_SMTP_SEND,
    TASK_SMTP_QUIT,
    TASK_POP3_CONNECT,
    TASK_POP3_STAT,
    TASK_POP3_LIST,
    TASK_POP3_RETR,
    TASK_POP3_DELE,
    TASK_POP3_HEAD,
    TASK_POP3_QUIT,
    TASK_HTTP_GET,
    TASK_HTTP_POST,
    TASK_GETTEL,
    TASK_GETUSERID,
    TASK_GETMAILID,
    TASK_GETEEPROMDATA,
    TASK_EEPROM_READ,
    TASK_EEPROM_WRITE,
    TASK_STOP,
    TASK_TCP_CUT,
    TASK_TCP_CONNECT,
    TASK_TCP_DISCONNECT,
    TASK_TCP_SENDRECV,
    TASK_GETHOSTADDRESS,
    TASK_GETLOCALADDRESS,
};

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
    u8 *data;
} MA_BUF;

typedef struct {
    u32 cmd;
} PARAM_TCP_CUT;

typedef struct {
    u8 *pHardwareType;
} PARAM_INITLIBRARY;

typedef struct {
    u8 *unk_1;
    u8 *unk_2;
    u32 unk_3;
} PARAM_TCP_CONNECT;

typedef struct {
    u32 unk_1;
} PARAM_TCP_DISCONNECT;

typedef struct {
    u32 unk_1;
    u8 *unk_2;
    u32 unk_3;
    u8 *unk_4;
} PARAM_TCP_SENDRECV;

typedef struct {
    u8 *unk_1;
    char *unk_2;
} PARAM_GETHOSTADDRESS;

typedef struct {
    const char *pTelNo;
    const char *pUserID;
    const char *pPassword;
} PARAM_TELSERVER;

typedef struct {
    const char *pTelNo;
} PARAM_TEL;

typedef struct {
    u32 unk_1;
} PARAM_RECEIVE;

typedef struct {
    const u8 *pSendData;
    u32 sendSize;
} PARAM_SDATA;

typedef struct {
    u8 *pCondition;
    u32 unk_2;
} PARAM_CONDITION;

typedef struct {
    u32 unk_1;
} PARAM_OFFLINE;

typedef struct {
    const char *pMailAddress;
} PARAM_SMTP_CONNECT;

typedef struct {
    const char * const *pRecipients;
} PARAM_SMTP_SENDER;

typedef struct {
    const char *pSendData;
    u32 sendSize;
    u32 endFlag;
    u32 unk_4;
    u32 unk_5;
    u32 unk_6;
} PARAM_SMTP_SEND;

typedef struct {
    u32 unk_1;
} PARAM_SMTP_POP3_QUIT;

typedef struct {
    char *unk_1;
    char *unk_2;
    char *unk_3;
} PARAM_POP3_CONNECT;

typedef struct {
    u16 *pNum;
    u32 *pSize;
} PARAM_POP3_STAT;

typedef struct {
    u32 *pSize;
} PARAM_POP3_LIST;

typedef struct {
    u8 *pRecvData;
    u32 recvBufSize;
    u16 *pRecvSize;
    u32 _4;
    u32 _5;
    u32 _6;
    u32 _7;
    u32 unk_8;
} PARAM_POP3_RETR;

typedef struct {
    u32 task;
    char *dest;
} PARAM_GETEEPROMDATA;

typedef struct {
    u8 *pData;
} PARAM_EEPROM_READ;

typedef struct {
    const u8 *pData;
} PARAM_EEPROM_WRITE;

typedef struct {
    u32 task;
    u32 unk_2;
    u8 *pRecvData;
    u32 recvBufSize;
    u16 *pRecvSize;
    const u8 *pSendData;
    u32 sendSize;
    const char *pServerPath;
    u32 pServerPathLen;
    const char *unk_10;
    u32 unk_11;
    u32 headBufSize;
    u32 server_unk_1;
    u32 unk_14;
    char *pHeadBuf;
    u32 unk_16;
    u32 unk_17;
    const char *pUserID;
    const char *pPassword;
    u32 unk_20;
    u32 unk_21;
    u32 counter;
    u32 next_step;
} PARAM_HTTP_GETPOST;

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
    vu8 cmd_recv;
    u16 recv_checksum;
    vu8 send_footer[4];
    vu8 recv_footer[4];
    vu8 cmd_last;
    vu8 unk_81;
    u8 recv_rubbish_counter;
    u8 unk_83;
    u8 unk_84[4];
    u32 unk_88;
    vu8 unk_92;
    u8 _93[1];
    vu16 error_unk_94;
    vu8 unk_96;
    vu8 task;
    vu8 task_step;
    u8 sockets[NUM_SOCKETS];
    u8 unk_101;
    u8 task_error;
    u8 _103[1];
    u16 task_error_unk_2;
    u8 ipaddr[4];
    u8 _110[2];
    union {
        PARAM_TCP_CUT tcp_cut;
        PARAM_INITLIBRARY initlibrary;
        PARAM_TCP_CONNECT tcp_connect;
        PARAM_TCP_DISCONNECT tcp_disconnect;
        PARAM_TCP_SENDRECV tcp_sendrecv;
        PARAM_GETHOSTADDRESS gethostaddress;
        PARAM_TELSERVER telserver;
        PARAM_TEL tel;
        PARAM_RECEIVE receive;
        PARAM_SDATA sdata;
        PARAM_CONDITION condition;
        PARAM_OFFLINE offline;
        PARAM_SMTP_CONNECT smtp_connect;
        PARAM_SMTP_SENDER smtp_sender;
        PARAM_SMTP_SEND smtp_send;
        PARAM_SMTP_POP3_QUIT smtp_pop3_quit;
        PARAM_POP3_CONNECT pop3_connect;
        PARAM_POP3_STAT pop3_stat;
        PARAM_POP3_LIST pop3_list;
        PARAM_POP3_RETR pop3_retr;
        PARAM_HTTP_GETPOST http_getpost;
        PARAM_GETEEPROMDATA geteepromdata;
        PARAM_EEPROM_READ eeprom_read;
        PARAM_EEPROM_WRITE eeprom_write;
    } param;
    u8 sockets_used[NUM_SOCKETS];
    u8 local_address[4];
    u8 _210[2];
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
    u8 unk_828[4];
    u8 unk_832[4];
    char smtp_server[20];
    char pop3_server[20];
    u32 unk_876;
    char unk_880[14];
    u8 _894[255];
    u8 prevbuf[12];
    char eeprom_unk_1161[32];
    char eeprom_unk_1193[30];
    char eeprom_unk_1223[44];
    u8 eeprom_telno[EEPROM_TELNO_SIZE];
    char eeprom_unk_1275[16];
    u8 _1291[495];
    u16 prevbuf_size;
    char unk_1788[6];
    u16 unk_1794;
    u16 unk_1796;
    u8 unk_1798[2];
    u8 _1800[92];
} MA_VAR;

extern MA_VAR gMA;

#define MA_SetCondition(cond) \
{ \
    gMA.condition &= ~MA_CONDITION_MASK; \
    gMA.condition |= (cond) << MA_CONDITION_SHIFT; \
}

#define MA_Reset() \
{ \
    gMA.unk_92 = 0; \
    MA_ChangeSIOMode(MA_SIO_BYTE); \
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode]; \
    gMA.counter = 0; \
    gMA.intr_sio_mode = 0; \
    gMA.status &= ~STATUS_UNK_0; \
    gMA.status &= ~STATUS_UNK_9; \
    gMA.status &= ~STATUS_UNK_10; \
    gMA.status &= ~STATUS_UNK_13; \
    gMA.status &= ~STATUS_UNK_2; \
    gMA.condition &= ~MA_CONDITION_PTP_GET; \
    gMA.condition &= ~MA_CONDITION_CONNECT; \
    MA_SetCondition(MA_CONDITION_IDLE); \
    MAU_Socket_Clear(); \
    MA_SetCondition(MA_CONDITION_IDLE); \
}

#endif // _MA_VAR_H
