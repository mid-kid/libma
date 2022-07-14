#include "ma_bios.h"

#include <stddef.h>
#include "ma_api.h"
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
    TIMEOUT_90,
};

static void SetInternalRecvBuffer(void);
static void MA_SetInterval(int index);
static void MA_SetTimeoutCount(int index);
static int MA_PreSend(void);
static void MA_InitIoBuffer(MA_IOBUF *buffer, vu8 *mem, u16 size, u16 state);
static void MA_StartSioTransmit(void);
static void MA_SetTransmitData(MA_IOBUF *buffer);
static int MA_IsSupportedHardware(u8 hardware);
static void MABIOS_Data2(MA_BUF *pRecvBuf, const u8 *pSendData, u8 size);
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

static const u16 gTimerIntByteInter[] = {
    TIMER_COUNTER_MS(0.20), TIMER_COUNTER_MS(0.30), TIMER_COUNTER_MS(0.40), TIMER_COUNTER_MS(0.50), TIMER_COUNTER_MS(0.60),
    TIMER_COUNTER_MS(0.45), TIMER_COUNTER_MS(0.55), TIMER_COUNTER_MS(0.85), TIMER_COUNTER_MS(1.05), TIMER_COUNTER_MS(1.20),
    //-TIMER_MS(0.25), -TIMER_MS(0.35), -TIMER_MS(0.45), -TIMER_MS(0.55), -TIMER_MS(0.65),
    //-TIMER_MS(0.50), -TIMER_MS(0.65), -TIMER_MS(0.90), -TIMER_MS(1.10), -TIMER_MS(1.25),
    //-4, -5, -7, -9, -10,
    //-8, -10, -14, -18, -20,
};

static const u32 gNullCounterByte[] = {
    TIMER_MS(1000) / 4 + 1, TIMER_MS(1000) / 5 + 1, TIMER_MS(1000) / 7 + 1, TIMER_MS(1000) / 9 + 1, TIMER_MS(1000) / 10 + 1,
    TIMER_MS(1000) / 8 + 0, TIMER_MS(1000) / 10 + 1, TIMER_MS(1000) / 14 + 0, TIMER_MS(1000) / 18 + 0, TIMER_MS(1000) / 20 + 0,
    //4097, 3278, 2341, 1821, 1639,
    //2048, 1639, 1170, 910, 819,
};

static const u32 gP2PCounterByte[] = {
    TIMER_MS(125) / 4 + 1, TIMER_MS(125) / 5 + 1, TIMER_MS(125) / 7 + 1, TIMER_MS(125) / 9 + 1, TIMER_MS(125) / 10 + 1,
    TIMER_MS(125) / 8 + 0, TIMER_MS(125) / 10 + 1, TIMER_MS(125) / 14 + 0, TIMER_MS(125) / 18 + 1, TIMER_MS(125) / 20 + 0,
    //513, 410, 293, 228, 205,
    //256, 205, 146, 114, 102,
};

static const u32 gTimeout90CounterByte[] = {
    TIMER_MS(90000) / 4 + 1, TIMER_MS(90000) / 5 + 2, TIMER_MS(90000) / 7 + 1, TIMER_MS(90000) / 9 + 1, TIMER_MS(90000) / 10 + 1,
    TIMER_MS(90000) / 8 + 1, TIMER_MS(90000) / 10 + 1, TIMER_MS(90000) / 14 + 0, TIMER_MS(90000) / 18 + 1, TIMER_MS(90000) / 20 + 0,
    //368702, 294962, 210687, 163868, 147481,
    //184351, 147481, 105343, 81934, 73740,
};

static const u32 gTimeout30CounterByte[] = {
    TIMER_MS(30000) / 4 + 1, TIMER_MS(30000) / 5 + 1, TIMER_MS(30000) / 7 + 1, TIMER_MS(30000) / 9 + 1, TIMER_MS(30000) / 10 + 1,
    TIMER_MS(30000) / 8 + 0, TIMER_MS(30000) / 10 + 0, TIMER_MS(30000) / 14 + 0, TIMER_MS(30000) / 18 + 0, TIMER_MS(30000) / 20 + 0,
    //122901, 98321, 70229, 54623, 49161,
    //61450, 49160, 35114, 27311, 24580,
};

static const u32 gTimeout10CounterByte[] = {
    TIMER_MS(10000) / 4 + 1, TIMER_MS(10000) / 5 + 1, TIMER_MS(10000) / 7 + 1, TIMER_MS(10000) / 9 + 1, TIMER_MS(10000) / 10 + 1,
    TIMER_MS(10000) / 8 + 0, TIMER_MS(10000) / 10 + 1, TIMER_MS(10000) / 14 + 1, TIMER_MS(10000) / 18 + 1, TIMER_MS(10000) / 20 + 0,
    //40967, 32774, 23410, 18208, 16387,
    //20483, 16387, 11705, 9104, 8193,
};

static const u32 gTimeout02CounterByte[] = {
    TIMER_MS(2000) / 4 + 1, TIMER_MS(2000) / 5 + 1, TIMER_MS(2000) / 7 + 1, TIMER_MS(2000) / 9 + 1, TIMER_MS(2000) / 10 + 1,
    TIMER_MS(2000) / 8 + 1, TIMER_MS(2000) / 10 + 0, TIMER_MS(2000) / 14 + 1, TIMER_MS(2000) / 18 + 1, TIMER_MS(2000) / 20 + 1,
    //8194, 6555, 4682, 3642, 3278,
    //4097, 3277, 2341, 1821, 1639,
};

static const u32 gTimeout200msecCounterByte[] = {
    TIMER_MS(200) / 4 + 1, TIMER_MS(200) / 5 + 1, TIMER_MS(200) / 7 + 1, TIMER_MS(200) / 9 + 1, TIMER_MS(200) / 10 + 1,
    TIMER_MS(200) / 8 + 1, TIMER_MS(200) / 10 + 1, TIMER_MS(200) / 14 + 0, TIMER_MS(200) / 18 + 0, TIMER_MS(200) / 20 + 1,
    //820, 656, 469, 365, 328,
    //410, 328, 234, 182, 164,
};

static const u32 gTimeout250msecCounterByte[] = {
    TIMER_MS(250) / 4 + 1, TIMER_MS(250) / 5 + 1, TIMER_MS(250) / 7 + 1, TIMER_MS(250) / 9 + 1, TIMER_MS(250) / 10 + 1,
    TIMER_MS(250) / 8 + 0, TIMER_MS(250) / 10 + 1, TIMER_MS(250) / 14 + 1, TIMER_MS(250) / 18 + 1, TIMER_MS(250) / 20 + 1,
    //1025, 820, 586, 456, 410,
    //512, 410, 293, 228, 205,
};

static const u32 gTimeout40msecCounterByte[] = {
    TIMER_MS(40) / 4 + 1, TIMER_MS(40) / 5 + 0, TIMER_MS(40) / 7 + 1, TIMER_MS(40) / 9 + 1, TIMER_MS(40) / 10 + 1,
    TIMER_MS(40) / 8 + 1, TIMER_MS(40) / 10 + 0, TIMER_MS(40) / 14 + 1, TIMER_MS(40) / 18 + 0, TIMER_MS(40) / 20 + 1,
    //164, 131, 94, 73, 66,
    //82, 65, 47, 36, 33,
};

static const u16 gTimerIntWordInter[] = {
    TIMER_COUNTER_MS(0.45), TIMER_COUNTER_MS(0.40), TIMER_COUNTER_MS(0.50), TIMER_COUNTER_MS(0.50), TIMER_COUNTER_MS(0.60),
    TIMER_COUNTER_MS(0.95), TIMER_COUNTER_MS(0.85), TIMER_COUNTER_MS(1.05), TIMER_COUNTER_MS(1.05), TIMER_COUNTER_MS(1.20),
    //-TIMER_MS(0.50), -TIMER_MS(0.45), -TIMER_MS(0.55), -TIMER_MS(0.55), -TIMER_MS(0.65),
    //-TIMER_MS(1.00), -TIMER_MS(0.90), -TIMER_MS(1.10), -TIMER_MS(1.10), -TIMER_MS(1.25),
    //-8, -7, -9, -9, -10,
    //-16, -14, -18, -18, -20,
};

static const u32 gNullCounterWord[] = {
    TIMER_MS(1000) / 8 + 1, TIMER_MS(1000) / 7 + 1, TIMER_MS(1000) / 9 + 1, TIMER_MS(1000) / 10 + 1, TIMER_MS(1000) / 12 + 1,
    TIMER_MS(1000) / 16 + 0, TIMER_MS(1000) / 14 + 0, TIMER_MS(1000) / 18 + 0, TIMER_MS(1000) / 20 + 0, TIMER_MS(1000) / 24 + 1,
    //2049, 2341, 1821, 1639, 1366,
    //1024, 1170, 910, 819, 683,
};

static const u32 gP2PCounterWord[] = {
    TIMER_MS(125) / 8 + 1, TIMER_MS(125) / 7 + 1, TIMER_MS(125) / 9 + 1, TIMER_MS(125) / 10 + 1, TIMER_MS(125) / 12 + 1,
    TIMER_MS(125) / 16 + 0, TIMER_MS(125) / 14 + 0, TIMER_MS(125) / 18 + 1, TIMER_MS(125) / 20 + 0, TIMER_MS(125) / 24 + 0,
    //257, 293, 228, 205, 171,
    //128, 146, 114, 102, 85,
};

static const u32 gTimeout90CounterWord[] = {
    TIMER_MS(90000) / 8 + 1, TIMER_MS(90000) / 7 + 1, TIMER_MS(90000) / 9 + 1, TIMER_MS(90000) / 10 + 1, TIMER_MS(90000) / 12 + 1,
    TIMER_MS(90000) / 16 + 0, TIMER_MS(90000) / 14 + 0, TIMER_MS(90000) / 18 + 1, TIMER_MS(90000) / 20 + 0, TIMER_MS(90000) / 24 + 0,
    //184351, 210687, 163868, 147481, 122901,
    //92175, 105343, 81934, 73740, 61450,
};

static const u32 gTimeout30CounterWord[] = {
    TIMER_MS(30000) / 8 + 1, TIMER_MS(30000) / 7 + 1, TIMER_MS(30000) / 9 + 1, TIMER_MS(30000) / 10 + 1, TIMER_MS(30000) / 12 + 1,
    TIMER_MS(30000) / 16 + 0, TIMER_MS(30000) / 14 + 0, TIMER_MS(30000) / 18 + 0, TIMER_MS(30000) / 20 + 0, TIMER_MS(30000) / 24 + 0,
    //61451, 70229, 54623, 49161, 40967,
    //30725, 35114, 27311, 24580, 20483,
};

static const u32 gTimeout10CounterWord[] = {
    TIMER_MS(10000) / 8 + 1, TIMER_MS(10000) / 7 + 1, TIMER_MS(10000) / 9 + 1, TIMER_MS(10000) / 10 + 1, TIMER_MS(10000) / 12 + 1,
    TIMER_MS(10000) / 16 + 1, TIMER_MS(10000) / 14 + 1, TIMER_MS(10000) / 18 + 1, TIMER_MS(10000) / 20 + 0, TIMER_MS(10000) / 24 + 1,
    //20484, 23410, 18208, 16387, 13656,
    //10242, 11705, 9104, 8193, 6828,
};

static const u32 gTimeout02CounterWord[] = {
    TIMER_MS(2000) / 8 + 1, TIMER_MS(2000) / 7 + 1, TIMER_MS(2000) / 9 + 1, TIMER_MS(2000) / 10 + 1, TIMER_MS(2000) / 12 + 1,
    TIMER_MS(2000) / 16 + 0, TIMER_MS(2000) / 14 + 1, TIMER_MS(2000) / 18 + 1, TIMER_MS(2000) / 20 + 1, TIMER_MS(2000) / 24 + 1,
    //4097, 4682, 3642, 3278, 2732,
    //2048, 2341, 1821, 1639, 1366,
};

static const u32 gTimeout200msecCounterWord[] = {
    TIMER_MS(200) / 8 + 1, TIMER_MS(200) / 7 + 1, TIMER_MS(200) / 9 + 1, TIMER_MS(200) / 10 + 1, TIMER_MS(200) / 12 + 1,
    TIMER_MS(200) / 16 + 1, TIMER_MS(200) / 14 + 0, TIMER_MS(200) / 18 + 0, TIMER_MS(200) / 20 + 1, TIMER_MS(200) / 24 + 1,
    //410, 469, 365, 328, 274,
    //205, 234, 182, 164, 137,
};

static const u32 gTimeout250msecCounterWord[] = {
    TIMER_MS(250) / 8 + 1, TIMER_MS(250) / 7 + 1, TIMER_MS(250) / 9 + 1, TIMER_MS(250) / 10 + 1, TIMER_MS(250) / 12 + 1,
    TIMER_MS(250) / 16 + 0, TIMER_MS(250) / 14 + 1, TIMER_MS(250) / 18 + 1, TIMER_MS(250) / 20 + 1, TIMER_MS(250) / 24 + 1,
    //513, 586, 456, 410, 342,
    //256, 293, 228, 205, 171,
};

static const u32 gTimeout40msecCounterWord[] = {
    TIMER_MS(40) / 8 + 1, TIMER_MS(40) / 7 + 1, TIMER_MS(40) / 9 + 1, TIMER_MS(40) / 10 + 1, TIMER_MS(40) / 12 + 1,
    TIMER_MS(40) / 16 + 1, TIMER_MS(40) / 14 + 1, TIMER_MS(40) / 18 + 0, TIMER_MS(40) / 20 + 1, TIMER_MS(40) / 24 + 0,
    //82, 94, 73, 66, 55,
    //41, 47, 36, 33, 27,
};

static const u8 MaPacketData_PreStart[] = {
    0x4b, 0x4b, 0x4b, 0x4b,
};

static const u8 MaPacketData_Start[] = {
    0x99, 0x66,
    0x10, 0x00, 0x00, 0x08,
    'N', 'I', 'N', 'T', 'E', 'N', 'D', 'O',
    0x02, 0x77,
    0x81, 0x00, 0x00, 0x00,
};

static const u8 MaPacketData_NULL[] = {
    0x99, 0x66,
    0x0f, 0x00, 0x00, 0x00,
    0x00, 0x0f,
    0x81, 0x00, 0x00, 0x00,
};

static const u8 MaPacketData_CheckStatus[] = {
    0x99, 0x66,
    0x17, 0x00, 0x00, 0x00,
    0x00, 0x17,
    0x81, 0x00, 0x00, 0x00,
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
    gMA.intrSioMode = MA_INTR_SIO_IDLE;
    gMA.hardwareType = -1;

    MA_ChangeSIOMode(MA_SIO_BYTE);
    MA_SetInterval(0);

    gMA.counter = 0;
    gMA.timerInterval = 0;
    gMA.retryCount = 0;
    gMA.status = 0;
    gMA.iobuf_sio_tx = NULL;
    gMA.cmd_cur = 0;
    gMA.cmd_recv = 0;
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
    gMA.pRecvBuf = &gMA.buffer_recv;
}

static void MA_SetInterval(int index)
{
    if (gMA.hardwareType == (MATYPE_PROT_SLAVE | MATYPE_PDC)) {
        index += 5;
    }

    gMA.interval = index;
    gMA.timerDataInterval[MA_SIO_BYTE] = gTimerIntByteInter[index];
    gMA.timerDataInterval[MA_SIO_WORD] = gTimerIntWordInter[index];
    gMA.nullCounter[MA_SIO_BYTE] = gNullCounterByte[index];
    gMA.nullCounter[MA_SIO_WORD] = gNullCounterWord[index];
    gMA.P2PCounter[MA_SIO_BYTE] = gP2PCounterByte[index];
    gMA.P2PCounter[MA_SIO_WORD] = gP2PCounterWord[index];
    gMA.timeout200msecCounter[MA_SIO_BYTE] = gTimeout200msecCounterByte[index];
    gMA.timeout200msecCounter[MA_SIO_WORD] = gTimeout200msecCounterWord[index];

    switch (gMA.hardwareType) {
    case MATYPE_PROT_SLAVE | MATYPE_CDMA:
        gMA.timeoutCounter[MA_SIO_BYTE] = gTimeout250msecCounterByte[index];
        gMA.timeoutCounter[MA_SIO_WORD] = gTimeout250msecCounterWord[index];
        break;

    case MATYPE_PROT_SLAVE | MATYPE_PDC:
        gMA.timeoutCounter[MA_SIO_BYTE] = gTimeout200msecCounterByte[index];
        gMA.timeoutCounter[MA_SIO_WORD] = gTimeout200msecCounterWord[index];
        break;

    case MATYPE_PROT_SLAVE | MATYPE_PHS_Pocket:
        gMA.timeoutCounter[MA_SIO_BYTE] = gTimeout40msecCounterByte[index];
        gMA.timeoutCounter[MA_SIO_WORD] = gTimeout40msecCounterWord[index];
        break;

    default:
        gMA.timeoutCounter[MA_SIO_BYTE] = 0;
        gMA.timeoutCounter[MA_SIO_WORD] = 0;
        break;
    }
}

static void MA_SetTimeoutCount(int index)
{
    static const u32 * const counterArrayByte[] = {
        gTimeout02CounterByte,
        gTimeout10CounterByte,
        gTimeout30CounterByte,
        gTimeout90CounterByte,
    };

    static const u32 * const counterArrayWord[] = {
        gTimeout02CounterWord,
        gTimeout10CounterWord,
        gTimeout30CounterWord,
        gTimeout90CounterWord,
    };

    gMA.counter_timeout[MA_SIO_BYTE] = counterArrayByte[index][gMA.interval];
    gMA.counter_timeout[MA_SIO_WORD] = counterArrayWord[index][gMA.interval];
}

int MA_GetStatus(void)
{
    if (gMA.status & STATUS_CONNECTED) {
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
        MA_Reset();
    }

    gMA.error = error;
    gMA.intrSioMode = MA_INTR_SIO_IDLE;
    gMA.iobuf_packet_send.state = 0;
    gMA.iobuf_packet_recv.state = 0;
    gMA.condition |= MA_CONDITION_ERROR;
    gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
    gMA.task = TASK_NONE;
    gMA.condition &= ~MA_CONDITION_APIWAIT;
    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
    gMA.iobuf_sio_tx = NULL;
}

static int MA_PreSend(void)
{
    int flag;

    if (gMA.condition & MA_CONDITION_BIOS_BUSY) {
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
    gMA.condition &= ~MA_CONDITION_TCPCLOSED;
    gMA.retryCount = flag;
    gMA.status &= ~STATUS_SIO_RETRY;
    gMA.counter = flag;
    gMA.cmd_cur = 0;
    gMA.cmd_recv = 0;
    return TRUE;
}

static void MA_InitIoBuffer(MA_IOBUF *buffer, vu8 *mem, u16 size, u16 state)
{
    buffer->state = state;
    buffer->pRead = mem;
    buffer->pWrite = mem;
    buffer->size = size;
    buffer->readCnt = 0;
    buffer->checkSum = 0;
}

static void MA_StartSioTransmit(void)
{
    static u32 wordData;

    if (!gMA.iobuf_sio_tx) return;

    while (*(vu16 *)REG_SIOCNT & SIO_START) {}

    if (gMA.sioMode == MA_SIO_WORD) {
        wordData = gMA.iobuf_sio_tx->pRead[3] << 0
                   | gMA.iobuf_sio_tx->pRead[2] << 8
                   | gMA.iobuf_sio_tx->pRead[1] << 16
                   | gMA.iobuf_sio_tx->pRead[0] << 24;
        *(vu32 *)REG_SIODATA32 = wordData;

        gMA.iobuf_sio_tx->pRead += 4;
        gMA.iobuf_sio_tx->readCnt += 4;
    } else {
        *(vu8 *)REG_SIODATA8 = gMA.iobuf_sio_tx->pRead[0];

        gMA.iobuf_sio_tx->pRead += 1;
        gMA.iobuf_sio_tx->readCnt += 1;
    }

    gMA.iobuf_sio_tx = NULL;
    gMA.status &= ~STATUS_SIO_START;
    *(vu16 *)REG_SIOCNT |= SIO_START;
}

static void MA_SetTransmitData(MA_IOBUF *buffer)
{
    gMA.iobuf_sio_tx = buffer;
}

void MA_ChangeSIOMode(u8 mode)
{
    gMA.sioMode = mode;
    if (mode == MA_SIO_BYTE) {
        *(vu16 *)REG_SIOCNT &= ~SIO_MODE_MASK;
        //*(vu16 *)REG_SIOCNT |= SIO_8BIT_MODE;
    } else {
        *(vu16 *)REG_SIOCNT &= ~SIO_MODE_MASK;
        *(vu16 *)REG_SIOCNT |= SIO_32BIT_MODE;
    }
}

void MA_SetDataInterval(u16 interval_byte, u16 interval_word)
{
    gMA.timerDataInterval[MA_SIO_BYTE] = interval_byte;
    gMA.timerDataInterval[MA_SIO_WORD] = interval_word;
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
    if (hardware == (MATYPE_PROT_SLAVE | MATYPE_PHS_DoCoMo)
        || hardware == (MATYPE_PROT_SLAVE | MATYPE_PHS_Pocket)) {
        return 1;
    }
    return 3;  // MAGIC
}

void MABIOS_Null(void)
{
    if (!(gMA.status & STATUS_CONNECTED) || gMA.status & STATUS_CONNTEST) return;

    SetInternalRecvBuffer();
    gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = 0xf;  // MAGIC

    tmpPacketLen = sizeof(MaPacketData_NULL);
    if (gMA.sioMode == MA_SIO_BYTE) tmpPacketLen -= 2;
    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_NULL,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    gMA.status |= STATUS_CONNTEST;
    gMA.status |= STATUS_SIO_START;
    MA_SetTimeoutCount(TIMEOUT_30);
}

void MABIOS_Start(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_START;

    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_PreStart, 1, 1);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
    *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
}

void MABIOS_Start2(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_START;

    if (gMA.sioMode == MA_SIO_BYTE) {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start) - 2, 3);
    } else {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start), 3);
    }

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_02);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_End(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_END, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_END;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_Tel(u8 calltype, const char *number)
{
    static int telNoLen;

    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
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
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_90);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_Offline(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_OFFLINE, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_OFFLINE;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_WaitCall(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_WAITCALL, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_WAITCALL;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_Data(MA_BUF *pRecvBuf, const u8 *pSendData, u8 size, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + i] = *pSendData++;
#else
        u8 *p = tmppPacket;
        int n = i + 1;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *pSendData++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

static void MABIOS_Data2(MA_BUF *pRecvBuf, const u8 *pSendData, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = 0xff;
    tmppPacket[MAPROT_HEADER_SIZE + 1] = size;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 2 + i] = *pSendData++;
#else
        u8 *p = tmppPacket;
        int n = i + 2;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *pSendData++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 2);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_ReInit(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_REINIT, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_REINIT;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_CheckStatus(MA_BUF *pRecvBuf)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHECKSTATUS, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_CHECKSTATUS;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    gMA.status |= STATUS_SIO_START;
    MA_SetTimeoutCount(TIMEOUT_30);
}

void MABIOS_CheckStatus2(MA_BUF *pRecvBuf)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!(gMA.status & STATUS_CONNECTED) || gMA.status & STATUS_CONNTEST) return;

    gMA.pRecvBuf = pRecvBuf;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHECKSTATUS, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_CHECKSTATUS;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_10);
    gMA.status |= STATUS_CONNTEST;
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_ChangeClock(u8 mode)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_CHANGECLOCK;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = mode;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_CHANGECLOCK, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_EEPROM_Read(MA_BUF *pRecvBuf, u8 offset, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_EEPROM_READ;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = offset;
    tmppPacket[MAPROT_HEADER_SIZE + 1] = size;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_EEPROM_READ, 2);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_EEPROM_Write(MA_BUF *pRecvBuf, u8 offset, const u8 *data_send,
    u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
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
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_PPPConnect(MA_BUF *pRecvBuf, const char *userid,
    const char *password, u8 *dns1, u8 *dns2)
{
    static u8 *pData;
    static int dataLen;
    static int userIDLength;
    static int passwordLength;

    tmppPacket = gMA.buffer_packet_send;
    pData = tmppPacket + MAPROT_HEADER_SIZE;
    if (!MA_PreSend()) return;

    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_PPPCONNECT;
    gMA.pRecvBuf = pRecvBuf;

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
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_PPPDisconnect(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_PPPDISCONNECT, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_PPPDISCONNECT;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_TCPConnect(MA_BUF *pRecvBuf, u8 *ip, u16 port)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_TCPCONNECT;

    for (i = 0; i < 4; i++) {
        *(u8 *)(tmppPacket + i + MAPROT_HEADER_SIZE) = *ip++;
    }
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 0] = ((u8 *)&port)[1];
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 1] = ((u8 *)&port)[0];
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TCPCONNECT, 6);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.unk_83 = 0;  // MAGIC
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_TCPDisconnect(MA_BUF *pRecvBuf, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_TCPDISCONNECT;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TCPDISCONNECT, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_UDPConnect(MA_BUF *pRecvBuf, u8 *ip, u16 port)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_UDPCONNECT;

    for (i = 0; i < 4; i++) {
        *(u8 *)(tmppPacket + i + MAPROT_HEADER_SIZE) = *ip++;
    }
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 0] = ((u8 *)&port)[1];
    tmppPacket[MAPROT_HEADER_SIZE + 4 + 1] = ((u8 *)&port)[0];
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_UDPCONNECT, 6);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_UDPDisconnect(MA_BUF *pRecvBuf, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_UDPDISCONNECT;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_UDPDISCONNECT, 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_DNSRequest(MA_BUF *pRecvBuf, char *pServerName)
{
    static int serverNameLen;

    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.pRecvBuf = pRecvBuf;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.cmd_cur = MACMD_DNSREQUEST;

    serverNameLen = 0;
    while (*pServerName != '\0') {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + serverNameLen++] = *pServerName++;
#else
        u8 *p = tmppPacket;
        int n = serverNameLen;
        *(u8 *)(p + n + MAPROT_HEADER_SIZE) = *pServerName++;
        serverNameLen = n + 1;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DNSREQUEST, serverNameLen);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

void MABIOS_TestMode(void)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TESTMODE, 0);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send,
        tmpPacketLen, 3);

    gMA.cmd_cur = MACMD_TESTMODE;
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.intrSioMode = MA_INTR_SIO_SEND;
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_SIO_START;
}

static int MA_CreatePacket(u8 *packet, u8 cmd, u16 size)
{
    if (gMA.sioMode == MA_SIO_WORD) {
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

    tmppPacketLast =
        (MAPROT_FOOTER *)(packet + MAPROT_HEADER_SIZE + size + paddingLength);
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
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.status |= STATUS_CANCEL_REQUEST;
    gMA.timerInterval = TIMER_COUNTER_MS(2500);
    gMA.intrSioMode = MA_INTR_SIO_WAIT;
}

void MA_BiosStop(void)
{
    gMA.condition |= MA_CONDITION_BIOS_BUSY;
    gMA.status |= STATUS_BIOS_STOP;
    gMA.timerInterval = 0;  // MAGIC
    gMA.intrSioMode = MA_INTR_SIO_WAIT;
    gMA.status &= ~STATUS_CANCEL_REQUEST;
}

void MA_SendRetry(void)
{
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.iobuf_packet_send.pWrite,
        gMA.iobuf_packet_send.size, 3);
}

void MA_RecvRetry(void)
{
    MA_InitIoBuffer(&gMA.iobuf_packet_recv, gMA.iobuf_packet_recv.pWrite, 0, 1);
    gMA.recv_rubbish_counter = 0;
    gMA.status &= ~STATUS_SIO_RETRY;
    gMA.intrSioMode = MA_INTR_SIO_RECV;
    gMA.iobuf_packet_send.state = 0;
    gMA.cmd_last = 0;
    gMA.unk_81 = 0;
}

static void MA_IntrTimer_SIOSend(void)
{
    switch (gMA.iobuf_packet_send.state) {  // MAGIC
    case 1:
        MA_SetTransmitData(&gMA.iobuf_packet_send);
        gMA.iobuf_packet_send.state = 2;
        gMA.timerInterval = TIMER_COUNTER_MS(120);
        break;

    case 2:
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start) - 2, 3);
        gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
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
        if (gMA.sioMode == MA_SIO_BYTE) {
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
#define param gMA.param.sdata
    if (gMA.task != TASK_NONE
        && gMA.task != TASK_SDATA
        && gMA.task != TASK_GDATA) {
        return;
    }

    if (!(gMA.status & STATUS_CONNECTED)) return;
    gMA.counter++;

    if (gMA.status & STATUS_CONN_PTP
        && (!(gMA.condition & STATUS_SIO_RETRY) || gMA.status & STATUS_PTP_SEND)) {
        if (gMA.counter > gMA.P2PCounter[gMA.sioMode]) {
            gMA.counter = 0;
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            if (gMA.status & STATUS_PTP_SEND) {
                MABIOS_Data2(&gMA.buffer_unk_480, param.pSendData,
                    param.sendSize);
                param.pSendData = NULL;
                param.sendSize = 0;
                gMA.status |= STATUS_PTP_SEND_DONE;
            } else {
                MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, 0xff);
            }
        }
    } else {
        if (gMA.counter > gMA.nullCounter[gMA.sioMode]) {
            gMA.counter = 0;
            (&gMA.buffer_recv)->size = 0;
            (&gMA.buffer_recv)->data = gMA.unk_84;
            MABIOS_CheckStatus2(&gMA.buffer_recv);
        }
    }
#undef param
}

static void MA_IntrTimer_SIOWaitTime(void)
{
    if (gMA.status & STATUS_CANCEL_REQUEST) {
        MABIOS_Start2();
    } else if (gMA.status & STATUS_BIOS_STOP) {
        gMA.status &= ~STATUS_BIOS_STOP;
        gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
        gMA.intrSioMode = MA_INTR_SIO_IDLE;

        if (gMA.status & STATUS_SIO_SEND_TIMEOUT) {
            MA_ChangeSIOMode(MA_SIO_BYTE);
            MA_Reset();
            MA_TaskSet(TASK_NONE, 0);
            if (gMA.task != TASK_STOP) {
                gMA.status &= ~STATUS_SIO_SEND_TIMEOUT;
                MA_SetError(MAAPIE_TIMEOUT);
                gMA.counter = 0;
                gMA.intrSioMode = MA_INTR_SIO_IDLE;
            }
        }
    } else if (gMA.status & 8) {
        gMA.intrSioMode = MA_INTR_SIO_SEND;
        MA_SendRetry();
    } else {
        gMA.intrSioMode = MA_INTR_SIO_IDLE;
        gMA.pRecvBuf->size = gMA.iobuf_packet_recv.size;
        gMA.counter = 0;
        gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
    }
    gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
}

int MA_ProcessCheckStatusResponse(u8 response)
{
    int ret = 0;

    switch (response) {  // MAGIC
    case 0xff:
        ret = MA_CONDITION_LOST;
        if (gMA.unk_92 != 0) {
            if (gMA.status & STATUS_CONNTEST) {
                gMA.status = 0;
                MA_SetError(MAAPIE_OFFLINE);
            } else {
                gMA.status = 0;
            }
            MA_Reset();
        }
        break;

    case 0:
    case 1:
        if (gMA.unk_92 != 0) {
            if (gMA.status & STATUS_CONNTEST) {
                gMA.status = 0;
                MA_SetError(MAAPIE_OFFLINE);
            } else {
                gMA.status = 0;
            }
            MA_Reset();
        }

    case 5:
    case 4:
        switch (gMA.unk_92) {  // MAGIC
        case 3: ret = MA_CONDITION_PPP; break;
        case 4: ret = MA_CONDITION_SMTP; break;
        case 5: ret = MA_CONDITION_POP3; break;
        case 7: ret = MA_CONDITION_P2P_SEND; break;
        case 8: ret = MA_CONDITION_P2P_RECV; break;
        }
        break;
    }

    MA_SetCondition(ret);
    gMA.intrSioMode = MA_INTR_SIO_IDLE;
    gMA.iobuf_packet_send.state = 0;
    gMA.iobuf_packet_recv.state = 0;
    gMA.status &= ~STATUS_CONNTEST;

    return ret;
}

static void ConvertNegaErrToApiErr(void)
{
    static const u8 errTable[] = {
        MAAPIE_SYSTEM,
        MAAPIE_CANNOT_EXECUTE_LOW,
        MAAPIE_ILLEGAL_PARAMETER_LOW,
        MAAPIE_CONNECT,
        MAAPIE_CONNECT,
    };

    gMA.taskError = errTable[gMA.unk_81];
    gMA.taskErrorDetail = 0;
}

void MA_DefaultNegaResProc(void)
{
    switch (gMA.cmd_last) {
    case MACMD_TEL:
        switch (gMA.unk_81) {  // MAGIC
        case 0: gMA.taskError = MAAPIE_BUSY; break;
        case 1: gMA.taskError = MAAPIE_CONNECT; break;
        case 2: gMA.taskError = MAAPIE_ILLEGAL_PARAMETER_LOW; break;
        case 3: gMA.taskError = MAAPIE_CONNECT; break;
        case 4: gMA.taskError = MAAPIE_CONNECT; break;
        }
        gMA.taskErrorDetail = 0;
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
    if (gMA.status & STATUS_CANCEL_REQUEST) {
        gMA.status &= ~STATUS_CANCEL_REQUEST;
        MA_BiosStop();
        i = 0;
    } else {
        switch (cmd) {
        case MAPROT_REPLY | MACMD_START:
            MA_SetInterval(0);
            gMA.status |= STATUS_CONNECTED;
            break;

        case MAPROT_REPLY | MACMD_END:
            gMA.status &= ~STATUS_CONNTEST;
            gMA.status &= ~STATUS_CONN_PTP;
            gMA.status &= ~STATUS_PTP_SEND_DONE;
            gMA.status &= ~STATUS_PTP_SEND;
            gMA.condition &= ~MA_CONDITION_CONNECT;

            if (gMA.task != TASK_INITLIBRARY2 && gMA.task != TASK_CONDITION2) {
                gMA.intrSioMode = MA_INTR_SIO_WAIT;
                gMA.timerInterval = TIMER_COUNTER_MS(1000);
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

        case MAPROT_REPLY | MACMD_TCPCLOSED:
            gMA.pRecvBuf->size = gMA.iobuf_packet_recv.size;
            gMA.condition |= MA_CONDITION_TCPCLOSED;
            break;

        case MAPROT_REPLY | MACMD_REINIT:
            MA_ChangeSIOMode(MA_SIO_BYTE);
            gMA.intrSioMode = MA_INTR_SIO_WAIT;
            gMA.timerInterval = TIMER_COUNTER_MS(500);
            i = 0;
            break;

        case MAPROT_REPLY | MACMD_CHANGECLOCK:
            MA_ChangeSIOMode(pPacket[6]);
            gMA.intrSioMode = MA_INTR_SIO_WAIT;
            gMA.timerInterval = TIMER_COUNTER_MS(60);
            i = 0;
            break;

        case MAPROT_REPLY | MACMD_CHECKSTATUS:
            MA_ProcessCheckStatusResponse(gMA.iobuf_packet_recv.pWrite[0]);
            break;

        case MAPROT_REPLY | MACMD_ERROR:
            gMA.cmd_last = gMA.iobuf_packet_recv.pWrite[0];
            gMA.unk_81 = gMA.iobuf_packet_recv.pWrite[1];

            if (gMA.cmd_last == MACMD_TCPCONNECT && ++gMA.unk_83 != 5) {
                gMA.timerInterval = TIMER_COUNTER_MS(1000);
                gMA.status |= STATUS_SIO_RETRY;
                gMA.intrSioMode = MA_INTR_SIO_WAIT;
                i = 0;
                *(vu32 *)REG_TM3CNT = 0;
                *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
            }
            break;

        default:
            gMA.pRecvBuf->size = gMA.iobuf_packet_recv.size;
            break;
        }
    }

    gMA.status &= ~STATUS_SIO_RECV_DONE;
    if (i == 1) {
        gMA.intrSioMode = MA_INTR_SIO_IDLE;
        gMA.condition &= ~MA_CONDITION_BIOS_BUSY;
    }
    gMA.counter = 0;
}

void MA_IntrTimer(void)
{
    static u8 saveSioMode;

    gMA.status |= STATUS_INTR_TIMER;
    saveSioMode = gMA.intrSioMode;
    *(vu32 *)REG_TM3CNT = 0;

    if (!(gMA.status & STATUS_SIO_START) || gMA.status & STATUS_INTR_SIO
        || gMA.status & STATUS_API_CALL || *(vu16 *)REG_SIOCNT & SIO_START) {
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        gMA.status &= ~STATUS_INTR_TIMER;
        return;
    }

    MAAPI_Main();

    if (saveSioMode != gMA.intrSioMode) {
        gMA.status &= ~STATUS_INTR_TIMER;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    if (gMA.status & STATUS_SIO_RECV_DONE) {
        MA_ProcessRecvPacket(gMA.cmd_recv);
        gMA.status &= ~STATUS_INTR_TIMER;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    switch (gMA.intrSioMode) {
    case MA_INTR_SIO_IDLE: MA_IntrTimer_SIOIdle(); break;
    case MA_INTR_SIO_WAIT: MA_IntrTimer_SIOWaitTime(); break;
    case MA_INTR_SIO_SEND: MA_IntrTimer_SIOSend(); break;
    case MA_INTR_SIO_RECV: MA_IntrTimer_SIORecv(); break;
    }
    MA_StartSioTransmit();

    *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
    gMA.status &= ~STATUS_INTR_TIMER;
}

#define MA_IntrSio_Timeout() \
{ \
    if (++gMA.counter > gMA.counter_timeout[gMA.sioMode]) { \
        if (!(gMA.status & STATUS_CANCEL_REQUEST)) { \
            MA_CancelRequest(); \
            gMA.status |= STATUS_SIO_SEND_TIMEOUT; \
        } else { \
            MA_BiosStop(); \
        } \
        return; \
    } \
}

#define MA_Bios_Error() \
{ \
    gMA.status &= ~STATUS_CONNECTED; \
    gMA.condition &= ~MA_CONDITION_BIOS_BUSY; \
    gMA.status &= ~STATUS_CONNTEST; \
}

#define MA_Bios_disconnect_inline() \
{ \
    gMA.intrSioMode = MA_INTR_SIO_IDLE; \
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

    dataLeft = gMA.iobuf_packet_send.size - gMA.iobuf_packet_send.readCnt;
    if (gMA.sioMode == MA_SIO_BYTE) {
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
    if (gMA.iobuf_packet_send.size != gMA.iobuf_packet_send.readCnt) return;

    // Pull up behavior on SI (pin always high) means nothing is connected
    if (gMA.send_footer[0] == 0xff && gMA.send_footer[1] == 0xff) {
        MA_SetError(MAAPIE_MA_NOT_FOUND);
        MA_Bios_Error();
        return;
    }

    // Check if the adapter hardware is supported
    if (!MA_IsSupportedHardware(gMA.send_footer[0])
        && !(gMA.status & STATUS_CANCEL_REQUEST)) {
        if (gMA.status & STATUS_SIO_RETRY) {
            if (--gMA.retryCount == 0) {
                MA_Bios_disconnect_inline();
                return;
            }
        } else {
            gMA.retryCount = 2;
        }

        gMA.footerError = gMA.send_footer[1];
        gMA.timerInterval = TIMER_COUNTER_MS(120);
        gMA.status |= STATUS_SIO_RETRY;
        gMA.intrSioMode = MA_INTR_SIO_WAIT;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    // Check if the adapter returned an unknown protocol error
    if (gMA.send_footer[1] == MAPROT_ERR_F2 && !(gMA.status & STATUS_CANCEL_REQUEST)) {
        gMA.footerError = MAPROT_ERR_F2;

        if (gMA.status & STATUS_SIO_RETRY) {
            if (--gMA.retryCount == 0) {
                MA_SetError(MAAPIE_UNK_85);
                MA_Bios_Error();
                return;
            }
        } else {
            gMA.retryCount = -2;
        }

        gMA.footerError = gMA.send_footer[1];
        gMA.timerInterval = gMA.timerDataInterval[gMA.sioMode];
        gMA.status |= STATUS_SIO_RETRY;
        gMA.intrSioMode = MA_INTR_SIO_WAIT;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    // Check if the adapter supports the command and didn't fail the checksum
    if ((gMA.send_footer[1] == MAPROT_ERR_F0
            || gMA.send_footer[1] == MAPROT_ERR_CHECKSUM)
        && !(gMA.status & STATUS_CANCEL_REQUEST)) {
        if (gMA.status & STATUS_SIO_RETRY) {
            if (--gMA.retryCount == 0) {
                if (gMA.send_footer[1] == MAPROT_ERR_F0) {
                    MA_SetError(MAAPIE_UNK_83);
                } else {
                    MA_SetError(MAAPIE_UNK_84);
                }
                MA_Bios_Error();
                return;
            }
        } else {
            gMA.retryCount = 2;
        }

        gMA.footerError = gMA.send_footer[1];
        gMA.timerInterval = TIMER_COUNTER_MS(1000);
        gMA.status |= STATUS_SIO_RETRY;
        gMA.intrSioMode = MA_INTR_SIO_WAIT;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    // Make sure the adapter received the correct command, or an error command
    if (!(gMA.send_footer[1] == gMA.cmd_cur + MAPROT_REPLY
            || gMA.send_footer[1] == MACMD_ERROR + MAPROT_REPLY)
        && !(gMA.status & STATUS_CANCEL_REQUEST)) {
        if (gMA.status & STATUS_SIO_RETRY) {
            if (--gMA.retryCount == 0) {
                MA_Bios_disconnect();
                return;
            }
        } else {
            gMA.retryCount = 2;
        }

        gMA.footerError = gMA.send_footer[1];
        gMA.timerInterval = TIMER_COUNTER_MS(120);
        gMA.status |= STATUS_SIO_RETRY;
        gMA.intrSioMode = MA_INTR_SIO_WAIT;
        *(vu32 *)REG_TM3CNT = 0;
        *(vu32 *)REG_TM3CNT = TIMER_FLAGS | gMA.timerInterval;
        return;
    }

    // Initialize the reception of the reply packet
    gMA.hardwareType = gMA.send_footer[0];
    if (gMA.sioMode == MA_SIO_BYTE) {
        MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 1, 0);
    } else {
        MA_InitIoBuffer(&gMA.iobuf_footer, (u8 *)MaPacketData_PreStart, 4, 0);
    }
    MA_InitIoBuffer(&gMA.iobuf_packet_recv, gMA.pRecvBuf->data, 0, 1);
    gMA.recv_rubbish_counter = 0;
    gMA.status &= ~STATUS_SIO_RETRY;
    gMA.intrSioMode = MA_INTR_SIO_RECV;
    gMA.iobuf_packet_send.state = 0;
    gMA.counter = 0;
    gMA.cmd_last = 0;
    gMA.unk_81 = 0;
}

static void MA_IntrSio_Recv(u8 byte)
{
    static u8 recvByte;
    static int amari;

    if (gMA.sioMode == MA_SIO_BYTE) {
        recvByte = ((vu8 *)REG_SIODATA8)[0];
    } else {
        recvByte = ((vu8 *)REG_SIODATA32)[3 - byte];
    }

    switch (gMA.iobuf_packet_recv.state) {  // MAGIC
    case 1:
        // Wait for handshake
        switch (gMA.iobuf_packet_recv.readCnt) {
        case 0:
            // Handshake may only start from the first byte in a MA_SIO_WORD
            //   serial transaction.
            if (byte != 0) break;

            // If we time out, stop trying to receive
            MA_IntrSio_Timeout();

            // When the first magic byte is received, try to receive the second
            if (recvByte == MAPROT_MAGIC_1) {
                gMA.iobuf_packet_recv.readCnt++;
                break;
            }

            // Adapter is still busy, keep waiting
            if (recvByte == MAPROT_IDLE_SLAVE) break;

            // Allow up to 20 rubbish bytes to be received
            if (++gMA.recv_rubbish_counter <= 20) break;
            MA_SetError(MAAPIE_MA_NOT_FOUND);
            gMA.recv_rubbish_counter = 0;
            break;

        case 1:
            // Try to receive the second magic byte
            if (recvByte == MAPROT_MAGIC_2) {
                gMA.iobuf_packet_recv.state = 2;
                gMA.iobuf_packet_recv.readCnt = 0;
            } else {
                MA_SetError(MAAPIE_MA_NOT_FOUND);
            }
            gMA.recv_rubbish_counter = 0;
            break;
        }
        break;

    case 2:
        // Parse the header
        switch (gMA.iobuf_packet_recv.readCnt) {
        case 0:  // Command
            gMA.cmd_recv = recvByte;
            break;

        case 1:  // Unused
            break;

        case 2:  // Size (high byte, assume 0)
            ((u8 *)&gMA.iobuf_packet_recv.size)[3 - gMA.iobuf_packet_recv.readCnt] =
                0;
            break;

        case 3:  // Size (low byte)
            ((u8 *)&gMA.iobuf_packet_recv.size)[3 - gMA.iobuf_packet_recv.readCnt] =
                recvByte;
            break;
        }

        // Read until we get 4 bytes
        gMA.iobuf_packet_recv.readCnt += 1;
        gMA.iobuf_packet_recv.checkSum += recvByte;
        if (gMA.iobuf_packet_recv.readCnt != 4) break;

        // If the packet is empty, skip to the end
        if (gMA.iobuf_packet_recv.size == 0) {
            gMA.iobuf_packet_recv.state = 4;
            gMA.iobuf_packet_recv.readCnt = 0;
            break;
        }

        // When using MA_SIO_WORD mode, a multiple of 4 bytes should be read
        if (gMA.sioMode == MA_SIO_BYTE) {
            gMA.iobuf_packet_recv.readCnt = gMA.iobuf_packet_recv.size;
        } else {
            amari = gMA.iobuf_packet_recv.size % 4;
            if (amari == 0) {
                gMA.iobuf_packet_recv.readCnt = gMA.iobuf_packet_recv.size;
            } else {
                gMA.iobuf_packet_recv.readCnt = gMA.iobuf_packet_recv.size
                                                + (4 - amari);
            }
        }

        gMA.iobuf_packet_recv.state = 3;
        break;

    case 3:
        MA_IntrSio_Timeout();

        // Read the packet body
        *gMA.iobuf_packet_recv.pRead++ = recvByte;
        gMA.iobuf_packet_recv.readCnt--;
        gMA.iobuf_packet_recv.checkSum += recvByte;
        if (gMA.iobuf_packet_recv.readCnt != 0) break;

        gMA.iobuf_packet_recv.state = 4;
        gMA.iobuf_packet_recv.readCnt = 0;
        break;

    case 4:
        // Read the checkSum
        ((u8 *)&gMA.recv_checksum)[1 - gMA.iobuf_packet_recv.readCnt] = recvByte;
        gMA.iobuf_packet_recv.readCnt++;
        if (gMA.iobuf_packet_recv.readCnt != 2) break;

        // Initialize the recv_footer buffer
        MA_InitIoBuffer(&gMA.iobuf_footer, gMA.buffer_footer,
            sizeof(gMA.buffer_footer), 0);
        gMA.buffer_footer[0] = MATYPE_PROT_MASTER | MATYPE_GBA;
        if (gMA.recv_checksum != gMA.iobuf_packet_recv.checkSum
            && !(gMA.status & STATUS_CANCEL_REQUEST)) {
            gMA.buffer_footer[1] = MAPROT_ERR_CHECKSUM;
            gMA.status |= STATUS_SIO_ERR_CHECKSUM;
        } else {
            gMA.buffer_footer[1] = gMA.cmd_recv;
        }
        gMA.buffer_footer[3] = 0;
        gMA.buffer_footer[2] = 0;

        gMA.iobuf_packet_recv.state = 5;
        gMA.iobuf_packet_recv.readCnt = 0;
        break;

    case 5:
        gMA.recv_footer[gMA.iobuf_packet_recv.readCnt] = recvByte;
        gMA.iobuf_packet_recv.readCnt++;

        // Wait until enough bytes have been read depending on the mode
        if ((gMA.sioMode == MA_SIO_BYTE && gMA.iobuf_packet_recv.readCnt == 2)
            || (gMA.sioMode == MA_SIO_WORD
                && gMA.iobuf_packet_recv.readCnt == 4)) {
            // Retry if the checkSum failed
            if (gMA.status & STATUS_SIO_ERR_CHECKSUM) {
                gMA.status &= ~STATUS_SIO_ERR_CHECKSUM;
                MA_RecvRetry();
            } else {
                gMA.status |= STATUS_SIO_RECV_DONE;
                gMA.intrSioMode = MA_INTR_SIO_IDLE;
            }
        }
        break;
    }
}

void MA_IntrSerialIO(void)
{
    *(vu32 *)REG_TM3CNT &= ~TMR_IF_ENABLE;
    gMA.status |= STATUS_INTR_SIO;

    switch (gMA.intrSioMode) {
    case MA_INTR_SIO_SEND:
        MA_IntrSio_Send();
        gMA.status |= STATUS_SIO_START;
        break;

    case MA_INTR_SIO_RECV:
        if (gMA.sioMode == MA_SIO_BYTE) {
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
    gMA.status |= STATUS_SIO_START;
    gMA.status &= ~STATUS_INTR_SIO;
}
