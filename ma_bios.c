#include "ma_bios.h"
#include "libma.h"

#include <stddef.h>
#include "ma_api.h"
#include "ma_var.h"
#include "ma_sub.h"

#define MAPROT_IDLE_SLAVE 0xd2
#define MAPROT_IDLE_MASTER 0x4b
#define MAPROT_MAGIC_1 0x99
#define MAPROT_MAGIC_2 0x66

#define MAPROT_REPLY 0x80

#define MAPROT_ERR_F0 0xf0
#define MAPROT_ERR_CHECKSUM 0xf1
#define MAPROT_ERR_F2 0xf2

#define MATYPE_PROT_MASK 0xf0
#define MATYPE_PROT_MASTER (MAPROT_REPLY | 0x0)
#define MATYPE_PROT_SLAVE (MAPROT_REPLY | 0x8)

#define MATYPE_GBC 0
#define MATYPE_GBA 1

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

#define MAPROT_HEADER_SIZE 6
#define MAPROT_FOOTER_SIZE 4

typedef struct {
    u16 magic;
    u8 cmd;
    u8 pad;
    u16 size;
} MAPROT_HEADER;

typedef struct {
    u8 checksum_hi;
    u8 checksum_lo;
    u8 device;
    u8 pad[3];
} MAPROT_FOOTER;

enum timeouts {
    TIMEOUT_02,
    TIMEOUT_10,
    TIMEOUT_30,
    TIMEOUT_90
};

static void SetInternalRecvBuffer(void);
static void MA_SetInterval(int index);
static void MA_SetTimeoutCount(int index);
static int MA_PreSend(void);
static void MA_InitIoBuffer(MA_IOBUF *buffer, vu8 *mem, u16 size, u16 state);
static void MA_StartSioTransmit(void);
static void MA_SetTransmitData(MA_IOBUF *buffer);
static int MA_IsSupportedHardware(u8 hardware);
static void MABIOS_Data2(MA_BUF *data_recv, u8 *data_send, u8 size);
static int MA_CreatePacket(u8 *packet, u8 cmd, u16 size);
static int MA_Create8BitPacket(u8 *packet, u8 cmd, u16 size);
static int MA_Create32BitPacket(u8 *packet, u8 cmd, u16 size);
static u16 MA_CalcCheckSum(u8 *data, u16 size);
static void MA_IntrTimer_SIOSend(void);
static void MA_IntrTimer_SIORecv(void);
static void MA_IntrTimer_SIOIdle(void);
static void MA_IntrTimer_SIOWaitTime(void);
static void ConvertNegaErrToApiErr(void);
static void MA_ProcessRecvPacket(u8 cmd);
static void MA_IntrSio_Send(void);
static void MA_IntrSio_Recv(u8 byte);

static u8 *tmppPacket;
static MAPROT_FOOTER *tmppPacketLast;
static u16 tmpPacketLen;
static int i;

static const s16 gTimerIntByteInter[] = {
    -4, -5, -7, -9, -10,
    -8, -10, -14, -18, -20
};

static const u32 gNullCounterByte[] = {
    4097, 3278, 2341, 1821, 1639,
    2048, 1639, 1170, 910, 819
};

static const u32 gP2PCounterByte[] = {
    513, 410, 293, 228, 205,
    256, 205, 146, 114, 102
};

static const u32 gTimeout90CounterByte[] = {
    368702, 294962, 210687, 163868, 147481,
    184351, 147481, 105343, 81934, 73740
};

static const u32 gTimeout30CounterByte[] = {
    122901, 98321, 70229, 54623, 49161,
    61450, 49160, 35114, 27311, 24580
};

static const u32 gTimeout10CounterByte[] = {
    40967, 32774, 23410, 18208, 16387,
    20483, 16387, 11705, 9104, 8193
};

static const u32 gTimeout02CounterByte[] = {
    8194, 6555, 4682, 3642, 3278,
    4097, 3277, 2341, 1821, 1639
};

static const u32 gTimeout200msecCounterByte[] = {
    820, 656, 469, 365, 328,
    410, 328, 234, 182, 164
};

static const u32 gTimeout250msecCounterByte[] = {
    1025, 820, 586, 456, 410,
    512, 410, 293, 228, 205
};

static const u32 gTimeout40msecCounterByte[] = {
    164, 131, 94, 73, 66,
    82, 65, 47, 36, 33
};

static const s16 gTimerIntWordInter[] = {
    -8, -7, -9, -9, -10,
    -16, -14, -18, -18, -20
};

static const u32 gNullCounterWord[] = {
    2049, 2341, 1821, 1639, 1366,
    1024, 1170, 910, 819, 683
};

static const u32 gP2PCounterWord[] = {
    257, 293, 228, 205, 171,
    128, 146, 114, 102, 85
};

static const u32 gTimeout90CounterWord[] = {
    184351, 210687, 163868, 147481, 122901,
    92175, 105343, 81934, 73740, 61450
};

static const u32 gTimeout30CounterWord[] = {
    61451, 70229, 54623, 49161, 40967,
    30725, 35114, 27311, 24580, 20483
};

static const u32 gTimeout10CounterWord[] = {
    20484, 23410, 18208, 16387, 13656,
    10242, 11705, 9104, 8193, 6828
};

static const u32 gTimeout02CounterWord[] = {
    4097, 4682, 3642, 3278, 2732,
    2048, 2341, 1821, 1639, 1366
};

static const u32 gTimeout200msecCounterWord[] = {
    410, 469, 365, 328, 274,
    205, 234, 182, 164, 137
};

static const u32 gTimeout250msecCounterWord[] = {
    513, 586, 456, 410, 342,
    256, 293, 228, 205, 171
};

static const u32 gTimeout40msecCounterWord[] = {
    82, 94, 73, 66, 55,
    41, 47, 36, 33, 27
};

static const u8 MaPacketData_PreStart[] = {
    0x4b, 0x4b, 0x4b, 0x4b
};

static const u8 MaPacketData_Start[] = {
    0x99, 0x66,
    0x10, 0x00, 0x00, 0x08,
    'N', 'I', 'N', 'T', 'E', 'N', 'D', 'O',
    0x02, 0x77,
    0x81, 0x00, 0x00, 0x00
};

static const u8 MaPacketData_NULL[] = {
    0x99, 0x66,
    0x0f, 0x00, 0x00, 0x00,
    0x00, 0x0f,
    0x81, 0x00, 0x00, 0x00
};

static const u8 MaPacketData_CheckStatus[] = {
    0x99, 0x66,
    0x17, 0x00, 0x00, 0x00,
    0x00, 0x17,
    0x81, 0x00, 0x00, 0x00
};

void MABIOS_Init(void)
{
    *(vu16 *)REG_IME = 0;

    *(vu32 *)REG_TM3CNT = 0;
    *(vu16 *)REG_RCNT = 0;
    *(vu16 *)REG_SIOCNT = SIO_8BIT_MODE;
    *(vu16 *)REG_SIOCNT |= SIO_IF_ENABLE | SIO_SCK_IN;
    *(vu16 *)REG_IF = SIO_INTR_FLAG | TIMER3_INTR_FLAG;
    *(vu16 *)REG_IE |= SIO_INTR_FLAG | TIMER3_INTR_FLAG;
    *(vu32 *)REG_TM3CNT = 0;

    gMA.condition = 0;
    gMA.error = -1;
    gMA.intr_sio_mode = 0;
    gMA.adapter_type = -1;

    MA_ChangeSIOMode(MA_SIO_BYTE);
    MA_SetInterval(0);

    gMA.counter = 0;
    gMA.timer_unk_12 = 0;
    gMA.unk_14 = 0;
    gMA.status = 0;
    gMA.iobuf_sio_tx = NULL;
    gMA.cmd_cur = 0;
    gMA.recv_cmd = 0;
    gMA.recv_checksum = 0;
    gMA.send_footer[0] = 0;
    gMA.send_footer[1] = 0;
    gMA.recv_footer[0] = 0;
    gMA.recv_footer[1] = 0;

    gMA.buffer_recv.size = sizeof(gMA.buffer_recv_data);
    gMA.buffer_recv.data = gMA.buffer_recv_data;
    gMA.buffer_unk_480.size = sizeof(gMA.unk_212);
    gMA.buffer_unk_480.data = gMA.unk_212;

    *(vu16 *)REG_IME = 1;
}

static void SetInternalRecvBuffer(void)
{
    gMA.buffer_recv.size = sizeof(gMA.buffer_recv_data);
    gMA.buffer_recv.data = gMA.buffer_recv_data;
    gMA.buffer_recv_ptr = &gMA.buffer_recv;
}

static void MA_SetInterval(int index)
{
    if (gMA.adapter_type == (MATYPE_PROT_SLAVE | MATYPE_PDC)) {
        index += 5;
    }

    gMA.interval = index;
    gMA.timer[MA_SIO_BYTE] = gTimerIntByteInter[index];
    gMA.timer[MA_SIO_WORD] = gTimerIntWordInter[index];
    gMA.counter_null[MA_SIO_BYTE] = gNullCounterByte[index];
    gMA.counter_null[MA_SIO_WORD] = gNullCounterWord[index];
    gMA.counter_p2p[MA_SIO_BYTE] = gP2PCounterByte[index];
    gMA.counter_p2p[MA_SIO_WORD] = gP2PCounterWord[index];
    gMA.counter_timeout200msec[MA_SIO_BYTE] = gTimeout200msecCounterByte[index];
    gMA.counter_timeout200msec[MA_SIO_WORD] = gTimeout200msecCounterWord[index];

    switch (gMA.adapter_type) {
    case MATYPE_PROT_SLAVE | MATYPE_CDMA:
        gMA.counter_adapter[MA_SIO_BYTE] = gTimeout250msecCounterByte[index];
        gMA.counter_adapter[MA_SIO_WORD] = gTimeout250msecCounterWord[index];
        break;

    case MATYPE_PROT_SLAVE | MATYPE_PDC:
        gMA.counter_adapter[MA_SIO_BYTE] = gTimeout200msecCounterByte[index];
        gMA.counter_adapter[MA_SIO_WORD] = gTimeout200msecCounterWord[index];
        break;

    case MATYPE_PROT_SLAVE | MATYPE_PHS_Pocket:
        gMA.counter_adapter[MA_SIO_BYTE] = gTimeout40msecCounterByte[index];
        gMA.counter_adapter[MA_SIO_WORD] = gTimeout40msecCounterWord[index];
        break;

    default:
        gMA.counter_adapter[MA_SIO_BYTE] = 0;
        gMA.counter_adapter[MA_SIO_WORD] = 0;
        break;
    }
}

static void MA_SetTimeoutCount(int index)
{
    static const u32 *const counterArrayByte[] = {
        gTimeout02CounterByte,
        gTimeout10CounterByte,
        gTimeout30CounterByte,
        gTimeout90CounterByte
    };

    static const u32 *const counterArrayWord[] = {
        gTimeout02CounterWord,
        gTimeout10CounterWord,
        gTimeout30CounterWord,
        gTimeout90CounterWord
    };

    gMA.counter_timeout[MA_SIO_BYTE] = counterArrayByte[index][gMA.interval];
    gMA.counter_timeout[MA_SIO_WORD] = counterArrayWord[index][gMA.interval];
}

int MA_GetStatus(void)
{
    if (gMA.status & STATUS_UNK_0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

u16 MA_GetCondition(void)
{
    return gMA.condition;
}

u8 MA_ErrorCheck(void)
{
    gMA.condition &= ~MA_CONDITION_ERROR;
    return gMA.error;
}

void MA_SetError(u8 error)
{
    if (error == MAAPIE_MA_NOT_FOUND) {
        gMA.unk_92 = 0;
        MA_ChangeSIOMode(MA_SIO_BYTE);
        gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
        gMA.counter = 0;
        gMA.intr_sio_mode = 0;
        gMA.status &= ~STATUS_UNK_0;
        gMA.status &= ~STATUS_UNK_9;
        gMA.status &= ~STATUS_UNK_10;
        gMA.status &= ~STATUS_UNK_13;
        gMA.status &= ~STATUS_UNK_2;
        gMA.condition &= ~MA_CONDITION_PTP_GET;
        gMA.condition &= ~MA_CONDITION_CONNECT;

        gMA.condition &= 0xff;
        gMA.condition = gMA.condition;
        MAU_Socket_Clear();
        gMA.condition &= 0xff;
        gMA.condition = gMA.condition;
    }

    gMA.error = error;
    gMA.intr_sio_mode = 0;
    gMA.iobuf_packet_send.state = 0;
    gMA.iobuf_packet_recv.state = 0;
    gMA.condition |= MA_CONDITION_ERROR;
    gMA.condition &= ~MA_CONDITION_UNK_5;
    gMA.task_unk_97 = TASK_UNK_97_00;
    gMA.condition &= ~MA_CONDITION_APIWAIT;
    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
    gMA.iobuf_sio_tx = NULL;
}

static int MA_PreSend(void)
{
    int flag;

    if (gMA.condition & MA_CONDITION_UNK_5) {
        MA_SetError(MAAPIE_CANNOT_EXECUTE);
        return FALSE;
    }

    flag = gMA.condition & MA_CONDITION_ERROR;
    if (flag) {
        MA_SetError(MAAPIE_CANNOT_EXECUTE);
        return FALSE;
    }

    gMA.condition &= ~MA_CONDITION_ERROR;
    gMA.error = -1;
    gMA.condition &= ~MA_CONDITION_UNK_6;
    gMA.unk_14 = flag;
    gMA.status &= ~STATUS_UNK_3;
    gMA.counter = flag;
    gMA.cmd_cur = 0;
    gMA.recv_cmd = 0;
    return TRUE;
}

static void MA_InitIoBuffer(MA_IOBUF *buffer, vu8 *mem, u16 size, u16 state)
{
    buffer->state = state;
    buffer->readptr = mem;
    buffer->writeptr = mem;
    buffer->size = size;
    buffer->readcnt = 0;
    buffer->checksum = 0;
}

static void MA_StartSioTransmit(void)
{
    static u32 wordData;

    if (!gMA.iobuf_sio_tx) return;

    while (*(vu16 *)REG_SIOCNT & SIO_START);

    if (gMA.sio_mode == MA_SIO_WORD) {
        wordData = gMA.iobuf_sio_tx->readptr[3] << 0 |
                   gMA.iobuf_sio_tx->readptr[2] << 8 |
                   gMA.iobuf_sio_tx->readptr[1] << 16 |
                   gMA.iobuf_sio_tx->readptr[0] << 24;
        *(vu32 *)REG_SIODATA32 = wordData;

        gMA.iobuf_sio_tx->readptr += 4;
        gMA.iobuf_sio_tx->readcnt += 4;
    } else {
        *(vu8 *)REG_SIODATA8 = gMA.iobuf_sio_tx->readptr[0];

        gMA.iobuf_sio_tx->readptr += 1;
        gMA.iobuf_sio_tx->readcnt += 1;
    }

    gMA.iobuf_sio_tx = NULL;
    gMA.status &= ~STATUS_UNK_1;
    *(vu16 *)REG_SIOCNT |= SIO_START;
}

static void MA_SetTransmitData(MA_IOBUF *buffer)
{
    gMA.iobuf_sio_tx = buffer;
}

void MA_ChangeSIOMode(u8 mode)
{
    gMA.sio_mode = mode;
    if (mode == MA_SIO_BYTE) {
        *(vu16 *)REG_SIOCNT &= ~SIO_MODE_MASK;
        //*(vu16 *)REG_SIOCNT |= SIO_8BIT_MODE;
    } else {
        *(vu16 *)REG_SIOCNT &= ~SIO_MODE_MASK;
        *(vu16 *)REG_SIOCNT |= SIO_32BIT_MODE;
    }
}

void MA_SetDataInterval(s16 interval_byte, s16 interval_word)
{
    gMA.timer[MA_SIO_BYTE] = interval_byte;
    gMA.timer[MA_SIO_WORD] = interval_word;
}

static int MA_IsSupportedHardware(u8 hardware)
{
    if ((hardware & MATYPE_PROT_MASK) == MAPROT_REPLY) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int MA_GetCallTypeFromHarwareType(u8 hardware)
{
    if (hardware == (MATYPE_PROT_SLAVE | MATYPE_PDC)) return 0;
    if (hardware == (MATYPE_PROT_SLAVE | MATYPE_CDMA)) return 2;
    if (hardware == (MATYPE_PROT_SLAVE | MATYPE_PHS_DoCoMo) ||
            hardware == (MATYPE_PROT_SLAVE | MATYPE_PHS_Pocket)) {
        return 1;
    }
    return 3;  // MAGIC
}

void MABIOS_Null(void)
{
    if (!(gMA.status & STATUS_UNK_0) || gMA.status & STATUS_UNK_2) return;

    SetInternalRecvBuffer();
    gMA.condition &= ~MA_CONDITION_UNK_5;
    gMA.cmd_cur = 0xf;  // MAGIC

    tmpPacketLen = sizeof(MaPacketData_NULL);
    if (gMA.sio_mode == MA_SIO_BYTE) tmpPacketLen -= 2;
    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_NULL, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    gMA.status |= STATUS_UNK_2;
    gMA.status |= STATUS_UNK_1;
    MA_SetTimeoutCount(TIMEOUT_30);
}

void MABIOS_Start(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_START;

    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_PreStart, 1, 1);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
    *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
        TMR_PRESCALER_1024CK | gMA.timer_unk_12;
}

void MABIOS_Start2(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    gMA.condition &= ~MA_CONDITION_UNK_5;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_START;

    if (gMA.sio_mode == MA_SIO_BYTE) {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start) - 2, 3);
    } else {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start), 3);
    }

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_02);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_End(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_END, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_END;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_Tel(u8 calltype, char *number)
{
    static int telNoLen;

    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_TEL;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = calltype;
    telNoLen = 0;
    while (*number != '\0') {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + telNoLen++] = *number++;
#else
        u8 *p = tmppPacket;
        int n = telNoLen + 1;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *number++;
        telNoLen = n;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TEL, telNoLen + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_90);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_Offline(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_OFFLINE, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_OFFLINE;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_WaitCall(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_WAITCALL, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_WAITCALL;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_Data(MA_BUF *data_recv, u8 *data_send, u8 size, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + i] = *data_send++;
#else
        u8 *p = tmppPacket;
        int n = i + 1;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *data_send++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

static void MABIOS_Data2(MA_BUF *data_recv, u8 *data_send, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = 0xff;
    tmppPacket[MAPROT_HEADER_SIZE + 1] = size;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[i + MAPROT_HEADER_SIZE + 2] = *data_send++;
#else
        u8 *p = tmppPacket;
        int n = i + 2;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *data_send++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 2);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_ReInit(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_REINIT, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_REINIT;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_CheckStatus(MA_BUF *data_recv)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHECKSTATUS, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_CHECKSTATUS;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    gMA.status |= STATUS_UNK_1;
    MA_SetTimeoutCount(TIMEOUT_30);
}

void MABIOS_CheckStatus2(MA_BUF *data_recv)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!(gMA.status & STATUS_UNK_0) || gMA.status & STATUS_UNK_2) return;

    gMA.buffer_recv_ptr = data_recv;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHECKSTATUS, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_CHECKSTATUS;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_10);
    gMA.status |= STATUS_UNK_2;
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_ChangeClock(u8 mode)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_CHANGECLOCK;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = mode;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHANGECLOCK, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_EEPROM_Read(MA_BUF *data_recv, u8 offset, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_EEPROM_READ;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = offset;
    tmppPacket[MAPROT_HEADER_SIZE + 1] = size;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_EEPROM_READ, 2);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_EEPROM_Write(MA_BUF *data_recv, u8 offset, u8 *data_send, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_EEPROM_WRITE;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = offset;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + i] = *data_send++;
#else
        u8 *p = tmppPacket;
        int n = i + 1;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *data_send++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_EEPROM_WRITE, size + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_PPPConnect(MA_BUF *data_recv, char *userid, char *password, u8 *dns1, u8 *dns2)
{
    static u8 *pData;
    static int dataLen;
    static int userIDLength;
    static int passwordLength;

    tmppPacket = gMA.buffer_packet_send;
    pData = tmppPacket + MAPROT_HEADER_SIZE;
    if (!MA_PreSend()) return;

    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_PPPCONNECT;
    gMA.buffer_recv_ptr = data_recv;

    userIDLength = MAU_strlen(userid);
    passwordLength = MAU_strlen(password);
    dataLen = userIDLength + passwordLength + 10;
    *pData++ = userIDLength;
    for (; userIDLength; userIDLength--) *pData++ = *userid++;
    *pData++ = passwordLength;
    for (; passwordLength; passwordLength--) *pData++ = *password++;
    for (i = 0; i < 4; i++) *pData++ = *dns1++;
    for (i = 0; i < 4; i++) *pData++ = *dns2++;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_PPPCONNECT, dataLen);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_PPPDisconnect(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_PPPDISCONNECT, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_PPPDISCONNECT;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_TCPConnect(MA_BUF *data_recv, u8 *ip, u16 port)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_TCPCONNECT;

    for (i = 0; i < 4; i++) {
        *(u8 *)(tmppPacket + i + MAPROT_HEADER_SIZE) = *ip++;
    }
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 0] = ((u8 *)&port)[1];
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 1] = ((u8 *)&port)[0];
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TCPCONNECT, 6);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.unk_83 = 0;  // MAGIC
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_TCPDisconnect(MA_BUF *data_recv, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_TCPDISCONNECT;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TCPDISCONNECT, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_UDPConnect(MA_BUF *data_recv, u8 *ip, u16 port)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_UDPCONNECT;

    for (i = 0; i < 4; i++) {
        *(u8 *)(tmppPacket + i + MAPROT_HEADER_SIZE) = *ip++;
    }
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 0] = ((u8 *)&port)[1];
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 1] = ((u8 *)&port)[0];
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_UDPCONNECT, 6);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_UDPDisconnect(MA_BUF *data_recv, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_UDPDISCONNECT;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_UDPDISCONNECT, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_DNSRequest(MA_BUF *data_recv, char *addr)
{
    static int serverNameLen;

    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_ptr = data_recv;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_DNSREQUEST;

    serverNameLen = 0;
    while (*addr != '\0') {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + serverNameLen++] = *addr++;
#else
        u8 *p = tmppPacket;
        int n = serverNameLen;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *addr++;
        serverNameLen = n + 1;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DNSREQUEST, serverNameLen);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_TestMode(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TESTMODE, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_TESTMODE;
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.intr_sio_mode = 1;  // MAGIC
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

static int MA_CreatePacket(u8 *packet, u8 cmd, u16 size)
{
    if (gMA.sio_mode == MA_SIO_WORD) {
        return MA_Create32BitPacket(packet, cmd, size);
    } else {
        return MA_Create8BitPacket(packet, cmd, size);
    }
}

static int MA_Create8BitPacket(u8 *packet, u8 cmd, u16 size)
{
    static u16 checkSum;

    MAPROT_HEADER *p = (MAPROT_HEADER *)packet;
    p->magic = 0x6699;
    p->cmd = cmd;
    p->pad = 0;
    p->size = (size & 0x00ff) << 8 | (size & 0xff00) >> 8;

    tmppPacketLast = (MAPROT_FOOTER *)(packet + MAPROT_HEADER_SIZE + size);
    checkSum = MA_CalcCheckSum(packet + 2, size + 4);
    tmppPacketLast->checksum_hi = checkSum >> 8;
    tmppPacketLast->checksum_lo = checkSum >> 0;
    tmppPacketLast->device = MATYPE_PROT_MASTER | MATYPE_GBA;
    tmppPacketLast->pad[0] = 0;

    return size + MAPROT_HEADER_SIZE + MAPROT_FOOTER_SIZE;
}

static int MA_Create32BitPacket(u8 *packet, u8 cmd, u16 size)
{
    static u8 *pPadding;
    static int paddingLength;
    static int amari;
    static u16 checkSum;

    MAPROT_HEADER *p = (MAPROT_HEADER *)packet;
    p->magic = 0x6699;
    p->cmd = cmd;
    p->pad = 0;
    p->size = (size & 0x00ff) << 8 | (size & 0xff00) >> 8;

    if (p->size == 0) {
        paddingLength = 0;
    } else {
        amari = size % 4;
        if (amari == 0) {
            paddingLength = amari;
        } else {
            paddingLength = 4 - amari;
        }

        pPadding = packet + MAPROT_HEADER_SIZE + size;
        for (i = 0; i < paddingLength; i++) *pPadding++ = 0;
    }

    tmppPacketLast = (MAPROT_FOOTER *)(packet + MAPROT_HEADER_SIZE + size + paddingLength);
    checkSum = MA_CalcCheckSum(packet + 2, size + 4);
    tmppPacketLast->checksum_hi = checkSum >> 8;
    tmppPacketLast->checksum_lo = checkSum >> 0;
    tmppPacketLast->device = MATYPE_PROT_MASTER | MATYPE_GBA;
    tmppPacketLast->pad[0] = 0;
    tmppPacketLast->pad[1] = 0;
    tmppPacketLast->pad[2] = 0;
    return size + paddingLength + MAPROT_HEADER_SIZE + MAPROT_FOOTER_SIZE + 2;
}

static u16 MA_CalcCheckSum(u8 *data, u16 size)
{
    static u16 sum;

    sum = 0;
    while (size != 0) {
        sum = sum + *data++;
        size--;
    }
    return sum;
}

void MA_CancelRequest(void)
{
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.status |= STATUS_UNK_12;
    gMA.timer_unk_12 = 0x5ff9;  // MAGIC
    gMA.intr_sio_mode = 3;  // MAGIC
}

void MA_BiosStop(void)
{
    gMA.condition |= MA_CONDITION_UNK_5;
    gMA.status |= STATUS_UNK_11;
    gMA.timer_unk_12 = 0;  // MAGIC
    gMA.intr_sio_mode = 3;  // MAGIC
    gMA.status &= ~STATUS_UNK_12;
}

void MA_SendRetry(void)
{
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.iobuf_packet_send.writeptr, gMA.iobuf_packet_send.size, 3);
}

void MA_RecvRetry(void)
{
    MA_InitIoBuffer(&gMA.iobuf_packet_recv, gMA.iobuf_packet_recv.writeptr, 0, 1);
    gMA.recv_garbage_counter = 0;
    gMA.status &= ~STATUS_UNK_3;
    gMA.intr_sio_mode = 2;
    gMA.iobuf_packet_send.state = 0;
    gMA.unk_80 = 0;
    gMA.unk_81 = 0;
}

static void MA_IntrTimer_SIOSend(void)
{
    switch (gMA.iobuf_packet_send.state) {  // MAGIC
    case 1:
        MA_SetTransmitData(&gMA.iobuf_packet_send);
        gMA.iobuf_packet_send.state = 2;
        gMA.timer_unk_12 = -1967;  // MAGIC
        break;

    case 2:
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start, sizeof(MaPacketData_Start) - 2, 3);
        gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
        MA_SetTransmitData(&gMA.iobuf_packet_send);
        break;

    case 3:
        MA_SetTransmitData(&gMA.iobuf_packet_send);
        break;
    }
}

static void MA_IntrTimer_SIORecv(void)
{
    switch (gMA.iobuf_packet_recv.state) {  // MAGIC
    case 0:
        break;

    case 1:
    case 2:
    case 3:
    case 4:
        if (gMA.sio_mode == MA_SIO_BYTE) {
            MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 1, 0);
        } else {
            MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 4, 0);
        }
        MA_SetTransmitData(&gMA.iobuf_footer);
        break;

    case 5:
        MA_SetTransmitData(&gMA.iobuf_footer);
        break;
    }
}

static void MA_IntrTimer_SIOIdle(void)
{
    if (gMA.task_unk_97 != TASK_UNK_97_00
            && gMA.task_unk_97 != TASK_UNK_97_06
            && gMA.task_unk_97 != TASK_UNK_97_07) return;  // MAGIC
    if (!(gMA.status & STATUS_UNK_0)) return;
    gMA.counter++;

    if (gMA.status & STATUS_UNK_9 &&
            (!(gMA.condition & STATUS_UNK_3) || gMA.status & STATUS_UNK_13)) {
        if (gMA.counter > gMA.counter_p2p[gMA.sio_mode]) {
            gMA.counter = 0;
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            if (gMA.status & STATUS_UNK_13) {
                MABIOS_Data2(&gMA.buffer_unk_480, gMA.unk_112, gMA.unk_112_size);
                gMA.unk_112 = NULL;
                gMA.unk_112_size = 0;
                gMA.status |= STATUS_UNK_10;
            } else {
                MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, 0xff);
            }
        }
    } else {
        if (gMA.counter > gMA.counter_null[gMA.sio_mode]) {
            gMA.counter = 0;
            (&gMA.buffer_recv)->size = 0;
            (&gMA.buffer_recv)->data = &gMA.unk_84;
            MABIOS_CheckStatus2(&gMA.buffer_recv);
        }
    }
}

static void MA_IntrTimer_SIOWaitTime(void)
{
    if (gMA.status & STATUS_UNK_12) {
        MABIOS_Start2();
    } else if (gMA.status & STATUS_UNK_11) {
        gMA.status &= ~STATUS_UNK_11;
        gMA.condition &= ~MA_CONDITION_UNK_5;
        gMA.intr_sio_mode = 0;

        if (gMA.status & STATUS_UNK_14) {
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.unk_92 = 0;
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
            gMA.counter = 0;
            gMA.intr_sio_mode = 0;
            gMA.status &= ~STATUS_UNK_0;
            gMA.status &= ~STATUS_UNK_9;
            gMA.status &= ~STATUS_UNK_10;
            gMA.status &= ~STATUS_UNK_13;
            gMA.status &= ~STATUS_UNK_2;
            gMA.condition &= ~MA_CONDITION_PTP_GET;
            gMA.condition &= ~MA_CONDITION_CONNECT;

            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;
            MAU_Socket_Clear();
            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;

            MA_TaskSet(0, 0);
            if (gMA.task_unk_97 != TASK_UNK_97_1E) {
                gMA.status &= ~STATUS_UNK_14;
                MA_SetError(MAAPIE_TIMEOUT);
                gMA.counter = 0;
                gMA.intr_sio_mode = 0;
            }
        }
    } else if (gMA.status & 8) {
        gMA.intr_sio_mode = 1;
        MA_SendRetry();
    } else {
        gMA.intr_sio_mode = 0;
        gMA.buffer_recv_ptr->size = gMA.iobuf_packet_recv.size;
        gMA.counter = 0;
        gMA.condition &= ~MA_CONDITION_UNK_5;
    }
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
}

int MA_ProcessCheckStatusResponse(u8 response)
{
    int ret = 0;

    switch (response) {  // MAGIC
    case 0xff:
        ret = MA_CONDITION_LOST;
        if (gMA.unk_92 != 0) {
            if (gMA.status & STATUS_UNK_2) {
                gMA.status = 0;
                MA_SetError(MAAPIE_OFFLINE);
            } else {
                gMA.status = 0;
            }
            gMA.unk_92 = 0;
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
            gMA.counter = 0;
            gMA.intr_sio_mode = 0;
            gMA.status &= ~STATUS_UNK_0;
            gMA.status &= ~STATUS_UNK_9;
            gMA.status &= ~STATUS_UNK_10;
            gMA.status &= ~STATUS_UNK_13;
            gMA.status &= ~STATUS_UNK_2;
            gMA.condition &= ~MA_CONDITION_PTP_GET;
            gMA.condition &= ~MA_CONDITION_CONNECT;

            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;
            MAU_Socket_Clear();
            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;
        }
        break;

    case 0:
    case 1:
        if (gMA.unk_92 != 0) {
            if (gMA.status & STATUS_UNK_2) {
                gMA.status = 0;
                MA_SetError(MAAPIE_OFFLINE);
            } else {
                gMA.status = ret;
            }
            gMA.unk_92 = 0;
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
            gMA.counter = 0;
            gMA.intr_sio_mode = 0;
            gMA.status &= ~STATUS_UNK_0;
            gMA.status &= ~STATUS_UNK_9;
            gMA.status &= ~STATUS_UNK_10;
            gMA.status &= ~STATUS_UNK_13;
            gMA.status &= ~STATUS_UNK_2;
            gMA.condition &= ~MA_CONDITION_PTP_GET;
            gMA.condition &= ~MA_CONDITION_CONNECT;

            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;
            MAU_Socket_Clear();
            gMA.condition &= 0xff;
            gMA.condition = gMA.condition;
        }

    case 5:
    case 4:
        switch(gMA.unk_92) {  // MAGIC
        case 3:
            ret = MA_CONDITION_PPP;
            break;
        case 4:
            ret = MA_CONDITION_SMTP;
            break;
        case 5:
            ret = MA_CONDITION_POP3;
            break;
        case 7:
            ret = MA_CONDITION_P2P_SEND;
            break;
        case 8:
            ret = MA_CONDITION_P2P_RECV;
            break;
        }
        break;
    }

    gMA.condition &= ~MA_CONDITION_MASK;
    gMA.condition |= ret << MA_CONDITION_SHIFT;
    gMA.intr_sio_mode = 0;
    gMA.iobuf_packet_send.state = 0;
    gMA.iobuf_packet_recv.state = 0;
    gMA.status &= ~STATUS_UNK_2;

    return ret;
}

static void ConvertNegaErrToApiErr(void)
{
    static const u8 errTable[] = {
        MAAPIE_SYSTEM,
        MAAPIE_CANNOT_EXECUTE_LOW,
        MAAPIE_ILLEGAL_PARAMETER_LOW,
        MAAPIE_CONNECT,
        MAAPIE_CONNECT
    };

    gMA.unk_102 = errTable[gMA.unk_81];
    gMA.unk_104 = 0;
}

void MA_DefaultNegaResProc(void)
{
    switch(gMA.unk_80) {
    case MACMD_TEL:
        switch(gMA.unk_81) {  // MAGIC
            case 0: gMA.unk_102 = MAAPIE_BUSY; break;
            case 1: gMA.unk_102 = MAAPIE_CONNECT; break;
            case 2: gMA.unk_102 = MAAPIE_ILLEGAL_PARAMETER_LOW; break;
            case 3: gMA.unk_102 = MAAPIE_CONNECT; break;
            case 4: gMA.unk_102 = MAAPIE_CONNECT; break;
        }
        gMA.unk_104 = 0;
        break;

    case MACMD_START:
    case MACMD_WAITCALL:
    case MACMD_DATA:
    case MACMD_CHECKSTATUS:
    case MACMD_CHANGECLOCK:
    case MACMD_EEPROM_READ:
    case MACMD_EEPROM_WRITE:
    case MACMD_PPPCONNECT:
    case MACMD_TCPCONNECT:
    case MACMD_DNSREQUEST:
        ConvertNegaErrToApiErr();
        break;
    }
}

static void MA_ProcessRecvPacket(u8 cmd)
{
    static u8 *pPacket;

    i = 1;
    pPacket = gMA.buffer_packet_send;
    if (gMA.status & STATUS_UNK_12) {
        gMA.status &= ~STATUS_UNK_12;
        MA_BiosStop();
        i = 0;
    } else {
        switch(cmd) {
        case MAPROT_REPLY | MACMD_START:
            MA_SetInterval(0);
            gMA.status |= STATUS_UNK_0;
            break;

        case MAPROT_REPLY | MACMD_END:
            gMA.status &= ~STATUS_UNK_2;
            gMA.status &= ~STATUS_UNK_9;
            gMA.status &= ~STATUS_UNK_10;
            gMA.status &= ~STATUS_UNK_13;
            gMA.condition &= ~MA_CONDITION_CONNECT;

            if (gMA.task_unk_97 != 2 && gMA.task_unk_97 != 9) {  // MAGIC
                gMA.intr_sio_mode = 3;  // MAGIC
                gMA.timer_unk_12 = -0x4003;  // MAGIC
                i = 0;
            }
            break;

        case MAPROT_REPLY | MACMD_TEL:
        case MAPROT_REPLY | MACMD_WAITCALL:
            gMA.condition |= MA_CONDITION_CONNECT;
            break;

        case MAPROT_REPLY | MACMD_OFFLINE:
            gMA.condition &= ~MA_CONDITION_CONNECT;
            break;

        case MAPROT_REPLY | MACMD_UNK_1F:
            gMA.buffer_recv_ptr->size = gMA.iobuf_packet_recv.size;
            gMA.condition |= MA_CONDITION_UNK_6;
            break;

        case MAPROT_REPLY | MACMD_REINIT:
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.intr_sio_mode = 3;  // MAGIC
            gMA.timer_unk_12 = -0x2002;  // MAGIC
            i = 0;
            break;

        case MAPROT_REPLY | MACMD_CHANGECLOCK:
            MA_ChangeSIOMode(pPacket[6]);
            gMA.intr_sio_mode = 3;  // MAGIC
            gMA.timer_unk_12 = -0x3d8;  // MAGIC
            i = 0;
            break;

        case MAPROT_REPLY | MACMD_CHECKSTATUS:
            MA_ProcessCheckStatusResponse(gMA.iobuf_packet_recv.writeptr[0]);
            break;

        case MAPROT_REPLY | MACMD_ERROR:
            gMA.unk_80 = gMA.iobuf_packet_recv.writeptr[0];
            gMA.unk_81 = gMA.iobuf_packet_recv.writeptr[1];

            if (gMA.unk_80 == MACMD_TCPCONNECT && ++gMA.unk_83 != 5) {
                gMA.timer_unk_12 = -0x4003;  // MAGIC
                gMA.status |= STATUS_UNK_3;
                gMA.intr_sio_mode = 3;  // MAGIC
                i = 0;
                *(vu32 *)REG_TM3CNT = 0;
                *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
                    TMR_PRESCALER_1024CK | gMA.timer_unk_12;
            }
            break;

        default:
            gMA.buffer_recv_ptr->size = gMA.iobuf_packet_recv.size;
            break;
        }
    }

    gMA.status &= ~STATUS_UNK_6;
    if (i == 1) {
        gMA.intr_sio_mode = 0;  // MAGIC
        gMA.condition &= ~MA_CONDITION_UNK_5;
    }
    gMA.counter = 0;
}

void MA_IntrTimer(void)
{
    static u8 saveSioMode;

    gMA.status |= STATUS_UNK_7;
    saveSioMode = gMA.intr_sio_mode;
    *(vu32 *)REG_TM3CNT = 0;

    if (!(gMA.status & STATUS_UNK_1) || gMA.status & STATUS_UNK_8 ||
            gMA.status & STATUS_UNK_5 || *(vu16 *)REG_SIOCNT & SIO_START) {
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        gMA.status &= ~STATUS_UNK_7;
        return;
    }

    MAAPI_Main();

    if (saveSioMode != gMA.intr_sio_mode) {
        gMA.status &= ~STATUS_UNK_7;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    if (gMA.status & STATUS_UNK_6) {
        MA_ProcessRecvPacket(gMA.recv_cmd);
        gMA.status &= ~STATUS_UNK_7;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    switch (gMA.intr_sio_mode) {  // MAGIC
    case 0:
        MA_IntrTimer_SIOIdle();
        break;

    case 3:
        MA_IntrTimer_SIOWaitTime();
        break;

    case 1:
        MA_IntrTimer_SIOSend();
        break;

    case 2:
        MA_IntrTimer_SIORecv();
        break;
    }
    MA_StartSioTransmit();

    *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
        TMR_PRESCALER_1024CK | gMA.timer_unk_12;
    gMA.status &= ~STATUS_UNK_7;
}

#define MA_IntrSio_Timeout() \
{ \
    if (++gMA.counter > gMA.counter_timeout[gMA.sio_mode]) { \
        if (!(gMA.status & STATUS_UNK_12)) { \
            MA_CancelRequest(); \
            gMA.status |= STATUS_UNK_14; \
        } else { \
            MA_BiosStop(); \
        } \
        return; \
    } \
}

#define MA_Bios_Error() \
{ \
    gMA.status &= ~STATUS_UNK_0; \
    gMA.condition &= ~MA_CONDITION_UNK_5; \
    gMA.status &= ~STATUS_UNK_2; \
}

#define MA_Bios_disconnect_inline() \
{ \
    gMA.intr_sio_mode = 0; \
    gMA.iobuf_packet_send.state = 0; \
    gMA.iobuf_packet_recv.state = 0; \
    MA_SetError(MAAPIE_MA_NOT_FOUND); \
    MA_Bios_Error(); \
    gMA.iobuf_packet_send.state = 0; \
    *(vu32 *)REG_TM3CNT = 0; \
}

void MA_Bios_disconnect(void)
{
    MA_Bios_disconnect_inline();
}

static void MA_IntrSio_Send(void)
{
    static int dataLeft;

    switch (gMA.iobuf_packet_send.state) {  // MAGIC
        default:
        case 1: return;
        case 2: return;
        case 3: break;
    }

    MA_IntrSio_Timeout();

    dataLeft = gMA.iobuf_packet_send.size - gMA.iobuf_packet_send.readcnt;
    if (gMA.sio_mode == MA_SIO_BYTE) {
        if (dataLeft < 2) {
            gMA.send_footer[1 - dataLeft] = *(vu8 *)(REG_SIODATA8);
        }
    } else {
        if (dataLeft == 0) {
            gMA.send_footer[0] = *(vu8 *)(REG_SIODATA32 + 3);
            gMA.send_footer[1] = *(vu8 *)(REG_SIODATA32 + 2);
            gMA.send_footer[2] = *(vu8 *)(REG_SIODATA32 + 1);
            gMA.send_footer[3] = *(vu8 *)(REG_SIODATA32 + 0);
        }
    }

    // Process only the packet footer (everything has been sent)
    if (gMA.iobuf_packet_send.size != gMA.iobuf_packet_send.readcnt) return;

    // Pull up behavior on SI (pin always high) means nothing is connected
    if (gMA.send_footer[0] == 0xff && gMA.send_footer[1] == 0xff) {
        MA_SetError(MAAPIE_MA_NOT_FOUND);
        MA_Bios_Error();
        return;
    }

    // Check if the adapter hardware is supported
    if (!MA_IsSupportedHardware(gMA.send_footer[0]) &&
            !(gMA.status & STATUS_UNK_12)) {
        if (gMA.status & STATUS_UNK_3) {
            if (--gMA.unk_14 == 0) {
                MA_Bios_disconnect_inline();
                return;
            }
        } else {
            gMA.unk_14 = 2;
        }

        gMA.unk_7 = gMA.send_footer[1];
        gMA.timer_unk_12 = -1967;  // MAGIC
        gMA.status |= STATUS_UNK_3;
        gMA.intr_sio_mode = 3;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    // Check if the adapter returned an unknown protocol error
    if (gMA.send_footer[1] == MAPROT_ERR_F2 &&
            !(gMA.status & STATUS_UNK_12)) {
        gMA.unk_7 = MAPROT_ERR_F2;

        if (gMA.status & STATUS_UNK_3) {
            if (--gMA.unk_14 == 0) {
                MA_SetError(MAAPIE_UNK_85);
                MA_Bios_Error();
                return;
            }
        } else {
            gMA.unk_14 = -2;
        }

        gMA.unk_7 = gMA.send_footer[1];
        gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
        gMA.status |= STATUS_UNK_3;
        gMA.intr_sio_mode = 3;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    // Check if the adapter supports the command and didn't fail the checksum
    if ((gMA.send_footer[1] == MAPROT_ERR_F0 ||
                gMA.send_footer[1] == MAPROT_ERR_CHECKSUM) &&
            !(gMA.status & STATUS_UNK_12)) {
        if (gMA.status & STATUS_UNK_3) {
            if (--gMA.unk_14 == 0) {
                if (gMA.send_footer[1] == MAPROT_ERR_F0) {
                    MA_SetError(MAAPIE_UNK_83);
                } else {
                    MA_SetError(MAAPIE_UNK_84);
                }
                MA_Bios_Error();
                return;
            }
        } else {
            gMA.unk_14 = 2;
        }

        gMA.unk_7 = gMA.send_footer[1];
        gMA.timer_unk_12 = -16387;  // MAGIC
        gMA.status |= STATUS_UNK_3;
        gMA.intr_sio_mode = 3;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    // Make sure the adapter received the correct command, or an error command
    if (!(gMA.send_footer[1] == gMA.cmd_cur + MAPROT_REPLY ||
                gMA.send_footer[1] == MACMD_ERROR + MAPROT_REPLY) &&
            !(gMA.status & STATUS_UNK_12)) {
        if (gMA.status & STATUS_UNK_3) {
            if (--gMA.unk_14 == 0) {
                MA_Bios_disconnect();
                return;
            }
        } else {
            gMA.unk_14 = 2;
        }

        gMA.unk_7 = gMA.send_footer[1];
        gMA.timer_unk_12 = -1967;  // MAGIC
        gMA.status |= STATUS_UNK_3;
        gMA.intr_sio_mode = 3;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
            TMR_PRESCALER_1024CK | gMA.timer_unk_12;
        return;
    }

    // Initialize the reception of the reply packet
    gMA.adapter_type = gMA.send_footer[0];
    if (gMA.sio_mode == MA_SIO_BYTE) {
        MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 1, 0);
    } else {
        MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 4, 0);
    }
    MA_InitIoBuffer(&gMA.iobuf_packet_recv, gMA.buffer_recv_ptr->data, 0, 1);
    gMA.recv_garbage_counter = 0;
    gMA.status &= ~STATUS_UNK_3;
    gMA.intr_sio_mode = 2;
    gMA.iobuf_packet_send.state = 0;
    gMA.counter = 0;
    gMA.unk_80 = 0;
    gMA.unk_81 = 0;
}

static void MA_IntrSio_Recv(u8 byte)
{
    static u8 recvByte;
    static int amari;

    if (gMA.sio_mode == MA_SIO_BYTE) {
        recvByte = *(vu8 *)REG_SIODATA8;
    } else {
        recvByte = *(vu8 *)(REG_SIODATA32 + 3 - byte);
    }

    switch(gMA.iobuf_packet_recv.state) {  // MAGIC
    case 1:
        // Wait for handshake
        switch (gMA.iobuf_packet_recv.readcnt) {
        case 0:
            // Handshake may only start from the first byte in a MA_SIO_WORD
            //   serial transaction.
            if (byte != 0) break;

            // If we time out, stop trying to receive
            MA_IntrSio_Timeout();

            // When the first magic byte is received, try to receive the second
            if (recvByte == MAPROT_MAGIC_1) {
                gMA.iobuf_packet_recv.readcnt++;
                break;
            }

            // Adapter is still busy, keep waiting
            if (recvByte == MAPROT_IDLE_SLAVE) break;

            // Allow up to 20 garbage bytes to be received
            if (++gMA.recv_garbage_counter <= 20) break;
            MA_SetError(MAAPIE_MA_NOT_FOUND);
            gMA.recv_garbage_counter = 0;
            break;

        case 1:
            // Try to receive the second magic byte
            if (recvByte == MAPROT_MAGIC_2) {
                gMA.iobuf_packet_recv.state = 2;
                gMA.iobuf_packet_recv.readcnt = 0;
            } else {
                MA_SetError(MAAPIE_MA_NOT_FOUND);
            }
            gMA.recv_garbage_counter = 0;
            break;
        }
        break;

    case 2:
        // Parse the header
        switch (gMA.iobuf_packet_recv.readcnt) {
        case 0:  // Command
            gMA.recv_cmd = recvByte;
            break;

        case 1:  // Unused
            break;

        case 2:  // Size (high byte, assume 0)
            ((u8 *)&gMA.iobuf_packet_recv.size)[3 - gMA.iobuf_packet_recv.readcnt] = 0;
            break;

        case 3:  // Size (low byte)
            ((u8 *)&gMA.iobuf_packet_recv.size)[3 - gMA.iobuf_packet_recv.readcnt] = recvByte;
            break;
        }

        // Read until we get 4 bytes
        gMA.iobuf_packet_recv.readcnt += 1;
        gMA.iobuf_packet_recv.checksum += recvByte;
        if (gMA.iobuf_packet_recv.readcnt != 4) break;

        // If the packet is empty, skip to the end
        if (gMA.iobuf_packet_recv.size == 0) {
            gMA.iobuf_packet_recv.state = 4;
            gMA.iobuf_packet_recv.readcnt = 0;
            break;
        }

        // When using MA_SIO_WORD mode, a multiple of 4 bytes should be read
        if (gMA.sio_mode == MA_SIO_BYTE) {
            gMA.iobuf_packet_recv.readcnt = gMA.iobuf_packet_recv.size;
        } else {
            amari = gMA.iobuf_packet_recv.size % 4;
            if (amari == 0) {
                gMA.iobuf_packet_recv.readcnt = gMA.iobuf_packet_recv.size;
            } else {
                gMA.iobuf_packet_recv.readcnt =
                    gMA.iobuf_packet_recv.size + (4 - amari);
            }
        }

        gMA.iobuf_packet_recv.state = 3;
        break;

    case 3:
        MA_IntrSio_Timeout();

        // Read the packet body
        *gMA.iobuf_packet_recv.readptr++ = recvByte;
        gMA.iobuf_packet_recv.readcnt--;
        gMA.iobuf_packet_recv.checksum += recvByte;
        if (gMA.iobuf_packet_recv.readcnt != 0) break;

        gMA.iobuf_packet_recv.state = 4;
        gMA.iobuf_packet_recv.readcnt = 0;
        break;

    case 4:
        // Read the checksum
        ((u8 *)&gMA.recv_checksum)[1 - gMA.iobuf_packet_recv.readcnt] = recvByte;
        gMA.iobuf_packet_recv.readcnt++;
        if (gMA.iobuf_packet_recv.readcnt != 2) break;

        // Initialize the recv_footer buffer
        MA_InitIoBuffer(&gMA.iobuf_footer, gMA.buffer_footer, sizeof(gMA.buffer_footer), 0);
        gMA.buffer_footer[0] = MATYPE_PROT_MASTER | MATYPE_GBA;
        if (gMA.recv_checksum != gMA.iobuf_packet_recv.checksum &&
                !(gMA.status & STATUS_UNK_12)) {
            gMA.buffer_footer[1] = MAPROT_ERR_CHECKSUM;
            gMA.status |= STATUS_UNK_4;
        } else {
            gMA.buffer_footer[1] = gMA.recv_cmd;
        }
        gMA.buffer_footer[3] = 0;
        gMA.buffer_footer[2] = 0;

        gMA.iobuf_packet_recv.state = 5;
        gMA.iobuf_packet_recv.readcnt = 0;
        break;

    case 5:
        gMA.recv_footer[gMA.iobuf_packet_recv.readcnt] = recvByte;
        gMA.iobuf_packet_recv.readcnt++;

        // Wait until enough bytes have been read depending on the mode
        if (!((gMA.sio_mode == MA_SIO_BYTE && gMA.iobuf_packet_recv.readcnt == 2) ||
              (gMA.sio_mode == MA_SIO_WORD && gMA.iobuf_packet_recv.readcnt == 4))) {
            break;
        }

        // Retry if the checksum failed
        if (gMA.status & STATUS_UNK_4) {
            gMA.status &= ~STATUS_UNK_4;
            MA_RecvRetry();
        } else {
            gMA.status |= STATUS_UNK_6;
            gMA.intr_sio_mode = 0;
        }
    }
}

void MA_IntrSerialIO(void)
{
    *(vu32 *)REG_TM3CNT &= ~TMR_IF_ENABLE;
    gMA.status |= STATUS_UNK_8;

    switch (gMA.intr_sio_mode) {  // MAGIC
    case 1:
        MA_IntrSio_Send();
        gMA.status |= STATUS_UNK_1;
        break;

    case 2:
        if (gMA.sio_mode == MA_SIO_BYTE) {
            MA_IntrSio_Recv(0);
        } else {
            for (i = 0; i < 4; i++) {
                if (gMA.condition & MA_CONDITION_ERROR) break;
                MA_IntrSio_Recv(i);
            }
        }
        break;
    }

    *(vu32 *)REG_TM3CNT |= TMR_IF_ENABLE;
    gMA.status |= STATUS_UNK_1;
    gMA.status &= ~STATUS_UNK_8;
}
