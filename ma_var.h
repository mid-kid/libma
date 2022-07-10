#ifndef _MA_VAR_H
#define _MA_VAR_H

#include <AgbTypes.h>

#define MA_CONDITION_BIOS_BUSY (1 << 5)  // Communicating with the adapter
#define MA_CONDITION_TCPCLOSED (1 << 6)  // Remote closed the connection

#define MAAPIE_UNK_83 0x83
#define MAAPIE_UNK_84 0x84
#define MAAPIE_UNK_85 0x85
#define MAAPIE_UNK_87 0x87

#define STATUS_CONNECTED (1 << 0)  // Connected to the adapter
#define STATUS_SIO_START (1 << 1)  // Send next byte (IntrSio keeps sending until done)
#define STATUS_CONNTEST (1 << 2)  // If the command fails the adapter is disconnected
#define STATUS_SIO_RETRY (1 << 3)  // Retrying the last packet
#define STATUS_SIO_ERR_CHECKSUM (1 << 4)  // Checksum error encountered
#define STATUS_API_CALL (1 << 5)  // Pause interrupts
#define STATUS_SIO_RECV_DONE (1 << 6)  // Received a packet correctly
#define STATUS_INTR_TIMER (1 << 7)  // Pause api calls
#define STATUS_INTR_SIO (1 << 8)  // Pause api calls
#define STATUS_CONN_PTP (1 << 9)  // Connected with a p2p connection
#define STATUS_PTP_SEND_DONE (1 << 10)  // Triggered p2p data send
#define STATUS_BIOS_STOP (1 << 11)  // MA_BiosStop called
#define STATUS_CANCEL_REQUEST (1 << 12)  // MA_CancelRequest called
#define STATUS_PTP_SEND (1 << 13)  // Send p2p data in the next interrupt
#define STATUS_SIO_SEND_TIMEOUT (1 << 14)  // Timeout during MA_IntrSio_Send
#define STATUS_BUFFER_EMPTY (1 << 15)  // gMA.prevbuf has just been emptied
#define STATUS_GBCENTER_ERR_101 (1 << 16)  // Gb-Status http header set to 101

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
#define MACMD_TCPCLOSED 0x1f
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

#define EEPROM_USERID_OFFSET 12
#define EEPROM_USERID_SIZE 32
#define EEPROM_MAILID_OFFSET 44
#define EEPROM_MAILID_SIZE 30
#define EEPROM_SERVERCONF_OFFSET 74
#define EEPROM_SERVERCONF_SIZE 44
#define EEPROM_TELNO1_OFFSET 118
#define EEPROM_TELNO_SIZE 8
#define EEPROM_COMMENT1_OFFSET 126
#define EEPROM_COMMENT_SIZE 16

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

enum ma_intr_sio_modes {
    MA_INTR_SIO_IDLE,
    MA_INTR_SIO_SEND,
    MA_INTR_SIO_RECV,
    MA_INTR_SIO_WAIT,
};

typedef struct {
    vu16 state;
    vu16 size;
    vu16 readCnt;
    vu16 checkSum;
    vu8 *pRead;
    vu8 *pWrite;
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
    u8 *pSocket;
    u8 *pHost;
    u32 port;
} PARAM_TCP_CONNECT;

typedef struct {
    u32 socket;
} PARAM_TCP_DISCONNECT;

typedef struct {
    u32 socket;
    u8 *pSendData;
    u32 size;
    u8 *unk_4;
} PARAM_TCP_SENDRECV;

typedef struct {
    u8 *pHost;
    char *pServerName;
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
    const u8 *pSendData;
    u32 sendSize;
} PARAM_SDATA;

typedef struct {
    u8 *pCondition;
    u32 unk_2;
} PARAM_CONDITION;

typedef struct {
    u32 timeout;
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
    u32 totalSize;  // unused
    u32 timeout;
    u32 nextStep;
} PARAM_SMTP_SEND;

typedef struct {
    u32 timeout;
} PARAM_SMTP_POP3_QUIT;

typedef struct {
    char *pServerName;
    char *pUserID;
    char *pPassword;
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
    u32 _4[4];
    u32 unk_8;
} PARAM_POP3_RETR;

typedef struct {
    u32 task;
    void *pData;
} PARAM_GETEEPROMDATA;

typedef struct {
    u8 *pData;
} PARAM_EEPROM_READ;

typedef struct {
    const u8 *pData;
} PARAM_EEPROM_WRITE;

typedef struct {
    u32 task;
    u32 unk_2;  // unused
    u8 *pRecvData;
    u32 recvBufSize;
    u16 *pRecvSize;
    const u8 *pSendData;
    u32 sendSize;
    const char *pServerPath;
    u32 serverPathLen;
    const char *pServerPathBkp;
    u32 serverPathLenBkp;
    u32 headBufSize;
    u32 server_unk_1;
    u32 unk_14;
    char *pHeadBuf;
    u32 headEnd;
    u32 headError;
    const char *pUserID;
    const char *pPassword;
    u32 headFlags;
    u32 headFound;
    u32 counter;
    u32 nextStep;
} PARAM_HTTP_GETPOST;

typedef struct {
    vu8 error;
    vu16 condition;
    vu8 intrSioMode;
    vu8 sioMode;
    vu8 adapter_type;
    vu8 unk_7;
    vu16 timer[MA_NUM_SIO_MODES];
    vu16 timer_unk_12;
    vu16 retryCount;
    vu8 interval;
    u32 nullCounter[MA_NUM_SIO_MODES];
    u32 counter_timeout[MA_NUM_SIO_MODES];
    u32 P2PCounter[MA_NUM_SIO_MODES];
    u32 timeout200msecCounter[MA_NUM_SIO_MODES];
    u32 timeoutCounter[MA_NUM_SIO_MODES];
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
    vu16 errorDetail;
    vu8 unk_96;
    vu8 task;
    vu8 taskStep;
    u8 sockets[NUM_SOCKETS];
    u8 prevbufHasEEPROMData;
    u8 task_error;
    u16 task_error_unk_2;
    u8 ipAddr[4];
    union {
        PARAM_TCP_CUT tcp_cut;
        PARAM_INITLIBRARY initlibrary;
        PARAM_TCP_CONNECT tcp_connect;
        PARAM_TCP_DISCONNECT tcp_disconnect;
        PARAM_TCP_SENDRECV tcp_sendrecv;
        PARAM_GETHOSTADDRESS gethostaddress;
        PARAM_TELSERVER telserver;
        PARAM_TEL tel;
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
    MA_BUF *pRecvBuf;
    MA_IOBUF *iobuf_sio_tx;
    u8 unk_828[4];
    u8 unk_832[4];
    struct {
        char smtp[20];
        char pop3[20];
        u32 http;
    } serverConf;
    char unk_880[14];
    u8 _894[255];
    u8 prevbuf[192];
    u8 _1291[445];
    u16 prevbufSize;
    char unk_1788[6];
    u16 httpRes;
    u16 gbCenterRes;
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
    gMA.timer_unk_12 = gMA.timer[gMA.sioMode]; \
    gMA.counter = 0; \
    gMA.intrSioMode = MA_INTR_SIO_IDLE; \
    gMA.status &= ~STATUS_CONNECTED; \
    gMA.status &= ~STATUS_CONN_PTP; \
    gMA.status &= ~STATUS_PTP_SEND_DONE; \
    gMA.status &= ~STATUS_PTP_SEND; \
    gMA.status &= ~STATUS_CONNTEST; \
    gMA.condition &= ~MA_CONDITION_PTP_GET; \
    gMA.condition &= ~MA_CONDITION_CONNECT; \
    MA_SetCondition(MA_CONDITION_IDLE); \
    MAU_Socket_Clear(); \
    MA_SetCondition(MA_CONDITION_IDLE); \
}

#endif // _MA_VAR_H
