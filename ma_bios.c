#include "ma_bios.h"
#include "libma.h"

#include <stdlib.h>
#include "ma_var.h"
#include "ma_sub.h"

#define MAPROT_REPLY 0x80

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
static void MA_InitIoBuffer(MA_IOBUF *buffer, u8 *mem, u16 size, u16 unk);
//static void MA_StartSioTransmit();
//static void MA_SetTransmitData();
//static void MA_IsSupportedHardware();
//static void MABIOS_Data2();
static int MA_CreatePacket(u8 *packet, u8 cmd, u16 size);
static int MA_Create8BitPacket(u8 *packet, u8 cmd, u16 size);
static int MA_Create32BitPacket(u8 *packet, u8 cmd, u16 size);
static u16 MA_CalcCheckSum(u8 *data, u16 size);
//static void MA_IntrTimer_SIOSend();
//static void MA_IntrTimer_SIORecv();
//static void MA_IntrTimer_SIOIdle();
//static void MA_IntrTimer_SIOWaitTime();
//static void ConvertNegaErrToApiErr();
//static void MA_ProcessRecvPacket();
//static void MA_IntrSio_Send();
//static void MA_IntrSio_Recv();

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
    gMA.unk_4 = 0;
    gMA.adapter_type = -1;

    MA_ChangeSIOMode(0);
    MA_SetInterval(0);

    gMA.unk_60 = 0;
    gMA.unk_12 = 0;
    gMA.unk_14 = 0;
    gMA.status = 0;
    gMA.iobuf_sio_tx = NULL;
    gMA.cmd_cur = 0;
    gMA.unk_69 = 0;
    gMA.unk_70 = 0;
    gMA.unk_72 = 0;
    gMA.unk_73 = 0;
    gMA.unk_76 = 0;
    gMA.unk_77 = 0;

    gMA.buffer_recv_size = sizeof(gMA.buffer_recv);
    gMA.buffer_recv_ptr = gMA.buffer_recv;
    gMA.unk_480 = sizeof(gMA.unk_212);
    gMA.unk_484 = gMA.unk_212;

    *(vu16 *)REG_IME = 1;
}

static void SetInternalRecvBuffer(void)
{
    gMA.buffer_recv_size = sizeof(gMA.buffer_recv);
    gMA.buffer_recv_ptr = gMA.buffer_recv;
    gMA.buffer_recv_unk = &gMA.buffer_recv_size;
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
    if (gMA.status & STATUS_UNK_0) return TRUE;
    return FALSE;
}

u16 MA_GetCondition(void)
{
    return gMA.condition;
}

u8 MA_ErrorCheck(void)
{
    gMA.condition &= ~CONDITION_UNK_1;
    return gMA.error;
}

void MA_SetError(u8 error)
{
    if (error == MAAPIE_MA_NOT_FOUND) {
        gMA.unk_92 = 0;
        MA_ChangeSIOMode(0);
        gMA.unk_12 = gMA.timer[gMA.sio_mode];
        gMA.unk_60 = 0;
        gMA.unk_4 = 0;
        gMA.status &= ~STATUS_UNK_0;
        gMA.status &= ~STATUS_UNK_9;
        gMA.status &= ~STATUS_UNK_10;
        gMA.status &= ~STATUS_UNK_13;
        gMA.status &= ~STATUS_UNK_2;
        gMA.condition &= ~CONDITION_UNK_3;
        gMA.condition &= ~CONDITION_UNK_4;
        gMA.condition &= 0xff;
        gMA.condition = gMA.condition;
        MAU_Socket_Clear();
        gMA.condition &= 0xff;
        gMA.condition = gMA.condition;
    }
    gMA.error = error;
    gMA.unk_4 = 0;
    gMA.iobuf_packet_send.unk_0 = 0;
    gMA.unk_504.unk_0 = 0;
    gMA.condition |= CONDITION_UNK_1;
    gMA.condition &= ~CONDITION_UNK_5;
    gMA.unk_97 = 0;
    gMA.condition &= ~CONDITION_UNK_0;
    gMA.condition &= ~CONDITION_UNK_2;
    gMA.iobuf_sio_tx = NULL;
}

static int MA_PreSend(void)
{
    int flag;

    if (gMA.condition & CONDITION_UNK_5) {
        MA_SetError(MAAPIE_CANNOT_EXECUTE);
        return FALSE;
    }

    flag = gMA.condition & CONDITION_UNK_1;
    if (flag) {
        MA_SetError(MAAPIE_CANNOT_EXECUTE);
        return FALSE;
    }

    gMA.condition &= ~CONDITION_UNK_1;
    gMA.error = -1;
    gMA.condition &= ~CONDITION_UNK_6;
    gMA.unk_14 = flag;
    gMA.status &= ~STATUS_UNK_3;
    gMA.unk_60 = flag;
    gMA.cmd_cur = 0;
    gMA.unk_69 = 0;
    return TRUE;
}

static void MA_InitIoBuffer(MA_IOBUF *buffer, u8 *mem, u16 size, u16 unk)
{
    buffer->unk_0 = unk;
    buffer->readptr = mem;
    buffer->writeptr = mem;
    buffer->size = size;
    buffer->readcnt = 0;
    buffer->writecnt = 0;
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
    if ((hardware & MATYPE_PROT_MASK) == MAPROT_REPLY) return TRUE;
    return FALSE;
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
    gMA.condition &= ~CONDITION_UNK_5;
    gMA.cmd_cur = 0xf;  // MAGIC

    tmpPacketLen = sizeof(MaPacketData_NULL);
    if (gMA.sio_mode == MA_SIO_BYTE) tmpPacketLen -= 2;
    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_NULL, tmpPacketLen, 3);

    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    gMA.status |= STATUS_UNK_2;
    gMA.status |= STATUS_UNK_1;
    MA_SetTimeoutCount(TIMEOUT_30);
}

void MABIOS_Start(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_START;

    MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_PreStart, 1, 1);

    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;

    *(vu32 *)REG_TM3CNT = TMR_ENABLE | TMR_IF_ENABLE |
        TMR_PRESCALER_1024CK | gMA.unk_12;
}

void MABIOS_Start2(void)
{
    *(vu32 *)REG_TM3CNT = 0;
    gMA.condition &= ~CONDITION_UNK_5;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_START;

    if (gMA.sio_mode == MA_SIO_BYTE) {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start) - 2, 3);
    } else {
        MA_InitIoBuffer(&gMA.iobuf_packet_send, (u8 *)MaPacketData_Start,
            sizeof(MaPacketData_Start), 3);
    }

    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
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
    gMA.condition |= CONDITION_UNK_5;
    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_Tel(u8 calltype, char *number)
{
    static int telNoLen;

    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    SetInternalRecvBuffer();
    gMA.condition |= CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_TEL;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = calltype;
    telNoLen = 0;
    while (*number != '\0') {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + telNoLen++] = *number++;
#else
        char *p = tmppPacket;
        int n = telNoLen + 1;
        *(char *)(p + n + MAPROT_HEADER_SIZE) = *number++;
        telNoLen = n;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_TEL, telNoLen + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
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
    gMA.condition |= CONDITION_UNK_5;
    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
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
    gMA.condition |= CONDITION_UNK_5;
    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

void MABIOS_Data(u8 *data_recv, u8 *data_send, u8 size, u8 socket)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_unk = data_recv;
    gMA.condition |= CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = socket;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[MAPROT_HEADER_SIZE + 1 + i] = *data_send++;
#else
        char *p = tmppPacket;
        int n = i + 1;
        *(char *)(p + n + MAPROT_HEADER_SIZE) = *data_send++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 1);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.unk_4 = 1;  // MAGIC
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

static void MABIOS_Data2(u8 *data_recv, u8 *data_send, u8 size)
{
    tmppPacket = gMA.buffer_packet_send;
    if (!MA_PreSend()) return;

    gMA.buffer_recv_unk = data_recv;
    gMA.condition |= CONDITION_UNK_5;
    gMA.cmd_cur = MACMD_DATA;

    tmppPacket[MAPROT_HEADER_SIZE + 0] = 0xff;
    tmppPacket[MAPROT_HEADER_SIZE + 1] = size;
    for (i = 0; i < size; i++) {
#if NONMATCHING
        tmppPacket[i + MAPROT_HEADER_SIZE + 2] = *data_send++;
#else
        char *p = tmppPacket;
        int n = i + 2;
        *(char *)(p + n + MAPROT_HEADER_SIZE) = *data_send++;
#endif
    }
    tmpPacketLen = MA_CreatePacket(tmppPacket, MACMD_DATA, size + 2);
    MA_InitIoBuffer(&gMA.iobuf_packet_send, gMA.buffer_packet_send, tmpPacketLen, 3);

    gMA.unk_4 = 1;
    gMA.unk_12 = gMA.timer[gMA.sio_mode];
    MA_SetTimeoutCount(TIMEOUT_30);
    gMA.status |= STATUS_UNK_1;
}

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_ReInit
MABIOS_ReInit:
    push	{r4, r5, r6, lr}
    ldr	r6, [pc, #112]
    ldr	r5, [pc, #112]
    str	r5, [r6, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_ReInit+0x6c
    bl	SetInternalRecvBuffer
    ldr	r4, [pc, #100]
    ldr	r0, [r6, #0]
    mov	r1, #22
    mov	r2, #0
    bl	MA_CreatePacket
    strh	r0, [r4, #0]
    mov	r0, r5
    sub	r0, #48
    ldrh	r2, [r4, #0]
    mov	r1, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #76]
    add	r4, r5, r0
    ldr	r2, [pc, #76]
    add	r1, r5, r2
    ldrb	r0, [r1, #0]
    mov	r0, #22
    strb	r0, [r1, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word tmpPacketLen
    .word 0xfffffde8
    .word 0xfffffe2c
    .word 0xfffffdf0
.size MABIOS_ReInit, .-MABIOS_ReInit
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_CheckStatus
MABIOS_CheckStatus:
    push	{r4, r5, r6, r7, lr}
    mov	r5, r0
    ldr	r7, [pc, #112]
    ldr	r6, [pc, #116]
    str	r6, [r7, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_CheckStatus+0x72
    ldr	r0, [pc, #108]
    add	r4, r6, r0
    mov	r1, #142
    lsl	r1, r1, #1
    add	r0, r6, r1
    str	r5, [r0, #0]
    ldr	r5, [pc, #100]
    ldr	r0, [r7, #0]
    mov	r1, #23
    mov	r2, #0
    bl	MA_CreatePacket
    strh	r0, [r5, #0]
    mov	r0, r6
    sub	r0, #48
    ldrh	r2, [r5, #0]
    mov	r1, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r2, [pc, #76]
    add	r1, r6, r2
    ldrb	r0, [r1, #0]
    mov	r0, #23
    strb	r0, [r1, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #48]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word tmpPacketLen
    .word 0xfffffe2c
    .word 0xfffffdf0
.size MABIOS_CheckStatus, .-MABIOS_CheckStatus
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_CheckStatus2
MABIOS_CheckStatus2:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r1, r0
    ldr	r0, [pc, #136]
    ldr	r6, [pc, #140]
    str	r6, [r0, #0]
    ldr	r0, [pc, #140]
    add	r5, r6, r0
    ldr	r0, [r5, #64]
    mov	r7, #1
    and	r0, r7
    cmp	r0, #0
    beq	MABIOS_CheckStatus2+0x8a
    ldr	r0, [r5, #64]
    mov	r2, #4
    mov	r8, r2
    and	r0, r2
    cmp	r0, #0
    bne	MABIOS_CheckStatus2+0x8a
    mov	r2, #142
    lsl	r2, r2, #1
    add	r0, r6, r2
    str	r1, [r0, #0]
    ldr	r4, [pc, #108]
    mov	r0, r6
    mov	r1, #23
    mov	r2, #0
    bl	MA_CreatePacket
    strh	r0, [r4, #0]
    mov	r0, r6
    sub	r0, #48
    ldrh	r2, [r4, #0]
    mov	r1, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #84]
    add	r1, r6, r0
    ldrb	r0, [r1, #0]
    mov	r0, #23
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #32
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    ldrb	r0, [r5, #4]
    strb	r7, [r5, #4]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #60]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    strh	r0, [r5, #12]
    mov	r0, #1
    bl	MA_SetTimeoutCount
    ldr	r0, [r5, #64]
    mov	r1, r8
    orr	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r5, #64]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word tmpPacketLen
    .word 0xfffffe2c
    .word 0xfffffdf0
.size MABIOS_CheckStatus2, .-MABIOS_CheckStatus2
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_ChangeClock
MABIOS_ChangeClock:
    push	{r4, r5, r6, r7, lr}
    lsl	r0, r0, #24
    lsr	r5, r0, #24
    ldr	r6, [pc, #116]
    ldr	r7, [pc, #116]
    str	r7, [r6, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_ChangeClock+0x74
    bl	SetInternalRecvBuffer
    ldr	r0, [pc, #104]
    add	r4, r7, r0
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldr	r2, [pc, #96]
    add	r1, r7, r2
    ldrb	r0, [r1, #0]
    mov	r0, #24
    strb	r0, [r1, #0]
    ldr	r0, [r6, #0]
    strb	r5, [r0, #6]
    ldr	r5, [pc, #84]
    ldr	r0, [r6, #0]
    mov	r1, #24
    mov	r2, #1
    bl	MA_CreatePacket
    strh	r0, [r5, #0]
    mov	r0, r7
    sub	r0, #48
    ldrh	r2, [r5, #0]
    mov	r1, r7
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r7, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word tmpPacketLen
    .word 0xfffffdf0
.size MABIOS_ChangeClock, .-MABIOS_ChangeClock
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_EEPROM_Read
MABIOS_EEPROM_Read:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r5, r0
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    mov	r8, r1
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r9, r2
    ldr	r7, [pc, #132]
    ldr	r6, [pc, #132]
    str	r6, [r7, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_EEPROM_Read+0x90
    ldr	r0, [pc, #124]
    add	r4, r6, r0
    mov	r1, #142
    lsl	r1, r1, #1
    add	r0, r6, r1
    str	r5, [r0, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldr	r2, [pc, #108]
    add	r1, r6, r2
    ldrb	r0, [r1, #0]
    mov	r0, #25
    strb	r0, [r1, #0]
    ldr	r0, [r7, #0]
    mov	r1, r8
    strb	r1, [r0, #6]
    ldr	r0, [r7, #0]
    mov	r2, r9
    strb	r2, [r0, #7]
    ldr	r5, [pc, #88]
    ldr	r0, [r7, #0]
    mov	r1, #25
    mov	r2, #2
    bl	MA_CreatePacket
    strh	r0, [r5, #0]
    mov	r0, r6
    sub	r0, #48
    ldrh	r2, [r5, #0]
    mov	r1, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #56]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word tmpPacketLen
    .word 0xfffffdf0
.size MABIOS_EEPROM_Read, .-MABIOS_EEPROM_Read
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_EEPROM_Write
MABIOS_EEPROM_Write:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r8, r0
    mov	r5, r2
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    mov	r9, r1
    lsl	r3, r3, #24
    lsr	r7, r3, #24
    ldr	r4, [pc, #176]
    ldr	r6, [pc, #176]
    str	r6, [r4, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_EEPROM_Write+0xbc
    ldr	r0, [pc, #168]
    add	r1, r6, r0
    mov	r2, #142
    lsl	r2, r2, #1
    add	r0, r6, r2
    mov	r2, r8
    str	r2, [r0, #0]
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    mov	r3, #0
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [pc, #148]
    add	r1, r6, r0
    ldrb	r0, [r1, #0]
    mov	r0, #26
    strb	r0, [r1, #0]
    ldr	r0, [r4, #0]
    mov	r1, r9
    strb	r1, [r0, #6]
    ldr	r0, [pc, #136]
    str	r3, [r0, #0]
    ldr	r2, [pc, #136]
    mov	r8, r2
    add	r2, r7, #1
    cmp	r3, r7
    bge	MABIOS_EEPROM_Write+0x76
    mov	r6, r4
    mov	r3, r0
    ldr	r1, [r6, #0]
    ldr	r0, [r3, #0]
    add	r0, r0, r1
    ldrb	r1, [r5, #0]
    strb	r1, [r0, #7]
    add	r5, #1
    ldr	r0, [r3, #0]
    add	r0, #1
    str	r0, [r3, #0]
    cmp	r0, r7
    blt	MABIOS_EEPROM_Write+0x60
    ldr	r0, [r4, #0]
    mov	r1, #26
    bl	MA_CreatePacket
    mov	r1, r8
    strh	r0, [r1, #0]
    ldr	r5, [pc, #92]
    mov	r1, r5
    add	r1, #48
    mov	r0, r8
    ldrh	r2, [r0, #0]
    mov	r0, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r1, [pc, #76]
    add	r4, r5, r1
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #68]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word i
    .word tmpPacketLen
    .word gMA+0x1e8
    .word 0xfffffe18
    .word 0xfffffe20
.size MABIOS_EEPROM_Write, .-MABIOS_EEPROM_Write
");
#endif

#if 0
#else
asm("
.lcomm pData.100, 0x4
.lcomm dataLen.101, 0x4
.lcomm userIDLength.102, 0x4
.lcomm passwordLength.103, 0x4

.align 2
.thumb_func
.global MABIOS_PPPConnect
MABIOS_PPPConnect:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    mov	sl, r0
    mov	r4, r1
    mov	r6, r2
    mov	r7, r3
    ldr	r0, [sp, #32]
    mov	r8, r0
    ldr	r0, [pc, #332]
    ldr	r5, [pc, #332]
    str	r5, [r0, #0]
    ldr	r1, [pc, #332]
    mov	r9, r1
    add	r0, r5, #6
    str	r0, [r1, #0]
    bl	MA_PreSend
    cmp	r0, #0
    bne	MABIOS_PPPConnect+0x2e
    b	MABIOS_PPPConnect+0x156
    ldr	r2, [pc, #320]
    add	r1, r5, r2
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [pc, #308]
    add	r1, r5, r0
    ldrb	r0, [r1, #0]
    mov	r0, #33
    strb	r0, [r1, #0]
    mov	r1, #142
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r2, sl
    str	r2, [r0, #0]
    mov	r0, r4
    bl	MAU_strlen
    ldr	r5, [pc, #288]
    str	r0, [r5, #0]
    mov	r0, r6
    bl	MAU_strlen
    ldr	r1, [pc, #280]
    str	r0, [r1, #0]
    ldr	r2, [pc, #280]
    ldr	r1, [r5, #0]
    add	r0, r1, r0
    add	r0, #10
    str	r0, [r2, #0]
    mov	r2, r9
    ldr	r0, [r2, #0]
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r2, #0]
    ldr	r0, [r5, #0]
    cmp	r0, #0
    beq	MABIOS_PPPConnect+0x98
    mov	r3, r9
    mov	r2, r5
    ldr	r1, [r3, #0]
    ldrb	r0, [r4, #0]
    strb	r0, [r1, #0]
    add	r4, #1
    add	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    sub	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #0
    bne	MABIOS_PPPConnect+0x82
    ldr	r3, [pc, #208]
    ldr	r0, [r3, #0]
    ldr	r2, [pc, #220]
    ldr	r1, [r2, #0]
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r3, #0]
    ldr	r0, [r2, #0]
    ldr	r5, [pc, #184]
    ldr	r4, [pc, #212]
    mov	sl, r4
    ldr	r4, [pc, #212]
    ldr	r1, [pc, #212]
    mov	r9, r1
    cmp	r0, #0
    beq	MABIOS_PPPConnect+0xce
    ldr	r1, [r3, #0]
    ldrb	r0, [r6, #0]
    strb	r0, [r1, #0]
    add	r6, #1
    add	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    sub	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #0
    bne	MABIOS_PPPConnect+0xb8
    mov	r0, #0
    str	r0, [r4, #0]
    ldr	r3, [pc, #152]
    mov	r2, r4
    ldr	r1, [r3, #0]
    ldrb	r0, [r7, #0]
    strb	r0, [r1, #0]
    add	r7, #1
    add	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #3
    ble	MABIOS_PPPConnect+0xd6
    mov	r0, #0
    str	r0, [r4, #0]
    ldr	r3, [pc, #120]
    mov	r2, r4
    ldr	r1, [r3, #0]
    mov	r4, r8
    ldrb	r0, [r4, #0]
    strb	r0, [r1, #0]
    mov	r0, #1
    add	r8, r0
    add	r1, #1
    str	r1, [r3, #0]
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #3
    ble	MABIOS_PPPConnect+0xf4
    ldr	r0, [r5, #0]
    mov	r1, sl
    ldrh	r2, [r1, #0]
    mov	r1, #33
    bl	MA_CreatePacket
    mov	r2, r9
    strh	r0, [r2, #0]
    ldr	r5, [pc, #108]
    mov	r1, r5
    add	r1, #48
    ldrh	r2, [r2, #0]
    mov	r0, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #96]
    add	r4, r5, r0
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #84]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word pData.100
    .word 0xfffffde8
    .word 0xfffffe2c
    .word userIDLength.102
    .word passwordLength.103
    .word dataLen.101
    .word i
    .word tmpPacketLen
    .word gMA+0x1e8
    .word 0xfffffe18
    .word 0xfffffe20
.size MABIOS_PPPConnect, .-MABIOS_PPPConnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_PPPDisconnect
MABIOS_PPPDisconnect:
    push	{r4, r5, r6, lr}
    ldr	r6, [pc, #112]
    ldr	r5, [pc, #112]
    str	r5, [r6, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_PPPDisconnect+0x6c
    bl	SetInternalRecvBuffer
    ldr	r4, [pc, #100]
    ldr	r0, [r6, #0]
    mov	r1, #34
    mov	r2, #0
    bl	MA_CreatePacket
    strh	r0, [r4, #0]
    mov	r0, r5
    sub	r0, #48
    ldrh	r2, [r4, #0]
    mov	r1, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #76]
    add	r4, r5, r0
    ldr	r2, [pc, #76]
    add	r1, r5, r2
    ldrb	r0, [r1, #0]
    mov	r0, #34
    strb	r0, [r1, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word tmpPacketLen
    .word 0xfffffde8
    .word 0xfffffe2c
    .word 0xfffffdf0
.size MABIOS_PPPDisconnect, .-MABIOS_PPPDisconnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_TCPConnect
MABIOS_TCPConnect:
    push	{r4, r5, r6, r7, lr}
    sub	sp, #4
    mov	r7, r0
    mov	r5, r1
    mov	r0, sp
    strh	r2, [r0, #0]
    ldr	r4, [pc, #176]
    ldr	r6, [pc, #180]
    str	r6, [r4, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_TCPConnect+0xb8
    ldr	r0, [pc, #172]
    add	r1, r6, r0
    mov	r2, #142
    lsl	r2, r2, #1
    add	r0, r6, r2
    str	r7, [r0, #0]
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    mov	r3, #0
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [pc, #152]
    add	r1, r6, r0
    ldrb	r0, [r1, #0]
    mov	r0, #35
    strb	r0, [r1, #0]
    ldr	r0, [pc, #144]
    str	r3, [r0, #0]
    ldr	r7, [pc, #144]
    mov	r3, r4
    mov	r2, r0
    ldr	r1, [r3, #0]
    ldr	r0, [r2, #0]
    add	r1, r1, r0
    ldrb	r0, [r5, #0]
    strb	r0, [r1, #6]
    add	r5, #1
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #3
    ble	MABIOS_TCPConnect+0x46
    ldr	r1, [r4, #0]
    mov	r0, sp
    ldrb	r0, [r0, #1]
    strb	r0, [r1, #10]
    ldr	r1, [r4, #0]
    mov	r0, sp
    ldrb	r0, [r0, #0]
    strb	r0, [r1, #11]
    ldr	r0, [r4, #0]
    mov	r1, #35
    mov	r2, #6
    bl	MA_CreatePacket
    strh	r0, [r7, #0]
    ldr	r6, [pc, #92]
    mov	r1, r6
    add	r1, #48
    ldrh	r2, [r7, #0]
    mov	r0, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r1, [pc, #80]
    add	r5, r6, r1
    ldrb	r0, [r5, #4]
    mov	r0, #1
    strb	r0, [r5, #4]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #72]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    mov	r4, #0
    strh	r0, [r5, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r1, [pc, #56]
    add	r0, r6, r1
    strb	r4, [r0, #0]
    ldr	r0, [r5, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r5, #64]
    add	sp, #4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word i
    .word tmpPacketLen
    .word gMA+0x1e8
    .word 0xfffffe18
    .word 0xfffffe20
    .word 0xfffffe6b
.size MABIOS_TCPConnect, .-MABIOS_TCPConnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_TCPDisconnect
MABIOS_TCPDisconnect:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r5, r0
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    mov	r8, r1
    ldr	r7, [pc, #124]
    ldr	r6, [pc, #124]
    str	r6, [r7, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_TCPDisconnect+0x82
    ldr	r0, [pc, #116]
    add	r4, r6, r0
    mov	r1, #142
    lsl	r1, r1, #1
    add	r0, r6, r1
    str	r5, [r0, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldr	r2, [pc, #100]
    add	r1, r6, r2
    ldrb	r0, [r1, #0]
    mov	r0, #36
    strb	r0, [r1, #0]
    ldr	r0, [r7, #0]
    mov	r1, r8
    strb	r1, [r0, #6]
    ldr	r5, [pc, #88]
    ldr	r0, [r7, #0]
    mov	r1, #36
    mov	r2, #1
    bl	MA_CreatePacket
    strh	r0, [r5, #0]
    mov	r0, r6
    sub	r0, #48
    ldrh	r2, [r5, #0]
    mov	r1, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word tmpPacketLen
    .word 0xfffffdf0
.size MABIOS_TCPDisconnect, .-MABIOS_TCPDisconnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_UDPConnect
MABIOS_UDPConnect:
    push	{r4, r5, r6, r7, lr}
    sub	sp, #4
    mov	r7, r0
    mov	r5, r1
    mov	r0, sp
    strh	r2, [r0, #0]
    ldr	r4, [pc, #168]
    ldr	r6, [pc, #172]
    str	r6, [r4, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_UDPConnect+0xb0
    ldr	r0, [pc, #164]
    add	r1, r6, r0
    mov	r2, #142
    lsl	r2, r2, #1
    add	r0, r6, r2
    str	r7, [r0, #0]
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    mov	r3, #0
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [pc, #144]
    add	r1, r6, r0
    ldrb	r0, [r1, #0]
    mov	r0, #37
    strb	r0, [r1, #0]
    ldr	r0, [pc, #136]
    str	r3, [r0, #0]
    ldr	r6, [pc, #136]
    mov	r3, r4
    mov	r2, r0
    ldr	r1, [r3, #0]
    ldr	r0, [r2, #0]
    add	r1, r1, r0
    ldrb	r0, [r5, #0]
    strb	r0, [r1, #6]
    add	r5, #1
    ldr	r0, [r2, #0]
    add	r0, #1
    str	r0, [r2, #0]
    cmp	r0, #3
    ble	MABIOS_UDPConnect+0x46
    ldr	r1, [r4, #0]
    mov	r0, sp
    ldrb	r0, [r0, #1]
    strb	r0, [r1, #10]
    ldr	r1, [r4, #0]
    mov	r0, sp
    ldrb	r0, [r0, #0]
    strb	r0, [r1, #11]
    ldr	r0, [r4, #0]
    mov	r1, #37
    mov	r2, #6
    bl	MA_CreatePacket
    strh	r0, [r6, #0]
    ldr	r5, [pc, #84]
    mov	r1, r5
    add	r1, #48
    ldrh	r2, [r6, #0]
    mov	r0, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r2, [pc, #72]
    add	r4, r5, r2
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    add	r2, #8
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    add	sp, #4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word i
    .word tmpPacketLen
    .word gMA+0x1e8
    .word 0xfffffe18
.size MABIOS_UDPConnect, .-MABIOS_UDPConnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_UDPDisconnect
MABIOS_UDPDisconnect:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r5, r0
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    mov	r8, r1
    ldr	r7, [pc, #124]
    ldr	r6, [pc, #124]
    str	r6, [r7, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_UDPDisconnect+0x82
    ldr	r0, [pc, #116]
    add	r4, r6, r0
    mov	r1, #142
    lsl	r1, r1, #1
    add	r0, r6, r1
    str	r5, [r0, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldr	r2, [pc, #100]
    add	r1, r6, r2
    ldrb	r0, [r1, #0]
    mov	r0, #38
    strb	r0, [r1, #0]
    ldr	r0, [r7, #0]
    mov	r1, r8
    strb	r1, [r0, #6]
    ldr	r5, [pc, #88]
    ldr	r0, [r7, #0]
    mov	r1, #38
    mov	r2, #1
    bl	MA_CreatePacket
    strh	r0, [r5, #0]
    mov	r0, r6
    sub	r0, #48
    ldrh	r2, [r5, #0]
    mov	r1, r6
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r6, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word tmpPacketLen
    .word 0xfffffdf0
.size MABIOS_UDPDisconnect, .-MABIOS_UDPDisconnect
");
#endif

#if 0
#else
asm("
.lcomm serverNameLen.122, 0x4

.align 2
.thumb_func
.global MABIOS_DNSRequest
MABIOS_DNSRequest:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r7, r0
    mov	r5, r1
    ldr	r4, [pc, #168]
    ldr	r6, [pc, #168]
    str	r6, [r4, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_DNSRequest+0xa8
    ldr	r0, [pc, #160]
    add	r1, r6, r0
    mov	r2, #142
    lsl	r2, r2, #1
    add	r0, r6, r2
    str	r7, [r0, #0]
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    mov	r3, #0
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [pc, #140]
    add	r1, r6, r0
    ldrb	r0, [r1, #0]
    mov	r0, #40
    strb	r0, [r1, #0]
    ldr	r0, [pc, #136]
    str	r3, [r0, #0]
    ldrb	r2, [r5, #0]
    mov	r7, r0
    ldr	r1, [pc, #132]
    mov	r8, r1
    cmp	r2, #0
    beq	MABIOS_DNSRequest+0x62
    mov	r6, r4
    mov	r3, r7
    ldr	r0, [r6, #0]
    ldr	r1, [r3, #0]
    add	r0, r0, r1
    strb	r2, [r0, #6]
    add	r5, #1
    add	r1, #1
    str	r1, [r3, #0]
    ldrb	r2, [r5, #0]
    cmp	r2, #0
    bne	MABIOS_DNSRequest+0x4e
    ldr	r0, [r4, #0]
    ldrh	r2, [r7, #0]
    mov	r1, #40
    bl	MA_CreatePacket
    mov	r2, r8
    strh	r0, [r2, #0]
    ldr	r5, [pc, #88]
    mov	r1, r5
    add	r1, #48
    ldrh	r2, [r2, #0]
    mov	r0, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #76]
    add	r4, r5, r0
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #68]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xfffffe2c
    .word serverNameLen.122
    .word tmpPacketLen
    .word gMA+0x1e8
    .word 0xfffffe18
    .word 0xfffffe20
.size MABIOS_DNSRequest, .-MABIOS_DNSRequest
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MABIOS_TestMode
MABIOS_TestMode:
    push	{r4, r5, r6, lr}
    ldr	r6, [pc, #112]
    ldr	r5, [pc, #112]
    str	r5, [r6, #0]
    bl	MA_PreSend
    cmp	r0, #0
    beq	MABIOS_TestMode+0x6c
    bl	SetInternalRecvBuffer
    ldr	r4, [pc, #100]
    ldr	r0, [r6, #0]
    mov	r1, #63
    mov	r2, #0
    bl	MA_CreatePacket
    strh	r0, [r4, #0]
    mov	r0, r5
    sub	r0, #48
    ldrh	r2, [r4, #0]
    mov	r1, r5
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #76]
    add	r4, r5, r0
    ldr	r2, [pc, #76]
    add	r1, r5, r2
    ldrb	r0, [r1, #0]
    mov	r0, #63
    strb	r0, [r1, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #32
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    ldrb	r0, [r4, #4]
    mov	r0, #1
    strb	r0, [r4, #4]
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    ldr	r2, [pc, #52]
    add	r1, r5, r2
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, #2
    bl	MA_SetTimeoutCount
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word tmppPacket
    .word gMA+0x218
    .word tmpPacketLen
    .word 0xfffffde8
    .word 0xfffffe2c
    .word 0xfffffdf0
.size MABIOS_TestMode, .-MABIOS_TestMode
");
#endif

static int MA_CreatePacket(u8 *packet, u8 cmd, u16 size)
{
    if (gMA.sio_mode == MA_SIO_WORD) {
        return MA_Create32BitPacket(packet, cmd, size);
    } else {
        return MA_Create8BitPacket(packet, cmd, size);
    }
}

#if 0  // STATIC
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
#else
asm("
.lcomm checkSum.132, 0x2

.align 2
.thumb_func
MA_Create8BitPacket:
    push	{r4, r5, r6, lr}
    mov	r4, r2
    lsl	r4, r4, #16
    lsr	r4, r4, #16
    mov	r6, #0
    ldr	r2, [pc, #76]
    strh	r2, [r0, #0]
    strb	r1, [r0, #2]
    strb	r6, [r0, #3]
    mov	r1, #255
    and	r1, r4
    lsl	r1, r1, #8
    lsr	r2, r4, #8
    orr	r1, r2
    strh	r1, [r0, #4]
    ldr	r5, [pc, #60]
    add	r1, r4, #6
    add	r1, r0, r1
    str	r1, [r5, #0]
    add	r0, #2
    add	r1, r4, #4
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	MA_CalcCheckSum
    ldr	r2, [pc, #44]
    strh	r0, [r2, #0]
    ldr	r1, [r5, #0]
    lsr	r0, r0, #8
    strb	r0, [r1, #0]
    ldr	r1, [r5, #0]
    ldrh	r0, [r2, #0]
    strb	r0, [r1, #1]
    ldr	r1, [r5, #0]
    mov	r0, #129
    strb	r0, [r1, #2]
    ldr	r0, [r5, #0]
    strb	r6, [r0, #3]
    add	r4, #10
    mov	r0, r4
    pop	{r4, r5, r6}
    pop	{r1}
    bx	r1
.align 2
    .word 0x00006699
    .word tmppPacketLast
    .word checkSum.132
.size MA_Create8BitPacket, .-MA_Create8BitPacket
");
#endif

#if 0  // STATIC
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
#else
asm("
.lcomm pPadding.136, 0x4
.lcomm paddingLength.137, 0x4
.lcomm amari.138, 0x4
.lcomm checkSum.139, 0x2

.align 2
.thumb_func
MA_Create32BitPacket:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    sub	sp, #4
    mov	r4, r0
    lsl	r2, r2, #16
    lsr	r5, r2, #16
    mov	r2, #0
    ldr	r0, [pc, #44]
    strh	r0, [r4, #0]
    strb	r1, [r4, #2]
    strb	r2, [r4, #3]
    mov	r0, #255
    and	r0, r5
    lsl	r0, r0, #8
    lsr	r1, r5, #8
    orr	r0, r1
    strh	r0, [r4, #4]
    mov	r2, r0
    cmp	r2, #0
    bne	MA_Create32BitPacket+0x50
    ldr	r0, [pc, #24]
    str	r2, [r0, #0]
    add	r0, r5, #6
    mov	r8, r0
    ldr	r6, [pc, #20]
    add	r1, r4, #2
    mov	r9, r1
    add	r2, r5, #4
    str	r2, [sp, #0]
    b	MA_Create32BitPacket+0xb6
.align 2
    .word 0x00006699
    .word paddingLength.137
    .word tmppPacketLast

    ldr	r0, [pc, #16]
    mov	r2, #3
    and	r2, r5
    str	r2, [r0, #0]
    cmp	r2, #0
    bne	MA_Create32BitPacket+0x6c
    ldr	r0, [pc, #8]
    str	r2, [r0, #0]
    b	MA_Create32BitPacket+0x74
.align 2
    .word amari.138
    .word paddingLength.137

    ldr	r1, [pc, #156]
    mov	r0, #4
    sub	r0, r0, r2
    str	r0, [r1, #0]
    ldr	r3, [pc, #152]
    add	r1, r5, #6
    add	r0, r4, r1
    str	r0, [r3, #0]
    ldr	r6, [pc, #148]
    mov	sl, r6
    mov	r2, #0
    str	r2, [r6, #0]
    ldr	r7, [pc, #132]
    ldr	r0, [r7, #0]
    mov	r8, r1
    ldr	r6, [pc, #140]
    add	r1, r4, #2
    mov	r9, r1
    add	r7, r5, #4
    str	r7, [sp, #0]
    cmp	r2, r0
    bge	MA_Create32BitPacket+0xb6
    mov	r0, #0
    mov	ip, r0
    mov	r2, sl
    ldr	r0, [r3, #0]
    mov	r1, ip
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r3, #0]
    ldr	r1, [r2, #0]
    add	r1, #1
    str	r1, [r2, #0]
    ldr	r7, [pc, #92]
    ldr	r0, [r7, #0]
    cmp	r1, r0
    blt	MA_Create32BitPacket+0x9e
    mov	r1, r8
    add	r0, r4, r1
    ldr	r2, [pc, #80]
    ldr	r1, [r2, #0]
    add	r0, r0, r1
    str	r0, [r6, #0]
    ldr	r7, [sp, #0]
    lsl	r1, r7, #16
    lsr	r1, r1, #16
    mov	r0, r9
    bl	MA_CalcCheckSum
    ldr	r3, [pc, #76]
    strh	r0, [r3, #0]
    ldr	r1, [r6, #0]
    lsr	r0, r0, #8
    mov	r2, #0
    strb	r0, [r1, #0]
    ldr	r1, [r6, #0]
    ldrh	r0, [r3, #0]
    strb	r0, [r1, #1]
    ldr	r1, [r6, #0]
    mov	r0, #129
    strb	r0, [r1, #2]
    ldr	r0, [r6, #0]
    strb	r2, [r0, #3]
    ldr	r0, [r6, #0]
    strb	r2, [r0, #4]
    ldr	r0, [r6, #0]
    strb	r2, [r0, #5]
    ldr	r1, [pc, #24]
    ldr	r0, [r1, #0]
    add	r0, r5, r0
    add	r0, #12
    add	sp, #4
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r1}
    bx	r1
.align 2
    .word paddingLength.137
    .word pPadding.136
    .word i
    .word tmppPacketLast
    .word checkSum.139
.size MA_Create32BitPacket, .-MA_Create32BitPacket
");
#endif

#if 0  // STATIC
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
#else
asm("
.lcomm sum.143, 0x2

.align 2
.thumb_func
MA_CalcCheckSum:
    push	{r4, r5, r6, lr}
    mov	r4, r0
    lsl	r1, r1, #16
    lsr	r2, r1, #16
    ldr	r1, [pc, #40]
    mov	r0, #0
    strh	r0, [r1, #0]
    mov	r5, r1
    cmp	r2, #0
    beq	MA_CalcCheckSum+0x2c
    mov	r6, r5
    mov	r1, #0
    ldrb	r0, [r4, #0]
    add	r3, r1, r0
    mov	r1, r3
    add	r4, #1
    sub	r0, r2, #1
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    cmp	r2, #0
    bne	MA_CalcCheckSum+0x18
    strh	r3, [r6, #0]
    ldrh	r0, [r5, #0]
    pop	{r4, r5, r6}
    pop	{r1}
    bx	r1
.align 2
    .word sum.143
.size MA_CalcCheckSum, .-MA_CalcCheckSum
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_CancelRequest
MA_CancelRequest:
    ldr	r1, [pc, #32]
    ldrh	r2, [r1, #2]
    mov	r0, #32
    ldrh	r3, [r1, #2]
    orr	r0, r2
    strh	r0, [r1, #2]
    ldr	r0, [r1, #64]
    mov	r2, #128
    lsl	r2, r2, #5
    orr	r0, r2
    str	r0, [r1, #64]
    ldrh	r0, [r1, #12]
    ldr	r0, [pc, #12]
    strh	r0, [r1, #12]
    ldrb	r0, [r1, #4]
    mov	r0, #3
    strb	r0, [r1, #4]
    bx	lr
.align 2
    .word gMA
    .word 0x00005ff9
.size MA_CancelRequest, .-MA_CancelRequest
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_BiosStop
MA_BiosStop:
    ldr	r2, [pc, #40]
    ldrh	r1, [r2, #2]
    mov	r0, #32
    ldrh	r3, [r2, #2]
    mov	r3, #0
    orr	r0, r1
    strh	r0, [r2, #2]
    ldr	r0, [r2, #64]
    mov	r1, #128
    lsl	r1, r1, #4
    orr	r0, r1
    str	r0, [r2, #64]
    ldrh	r0, [r2, #12]
    strh	r3, [r2, #12]
    ldrb	r0, [r2, #4]
    mov	r0, #3
    strb	r0, [r2, #4]
    ldr	r0, [r2, #64]
    ldr	r1, [pc, #8]
    and	r0, r1
    str	r0, [r2, #64]
    bx	lr
.align 2
    .word gMA
    .word 0xffffefff
.size MA_BiosStop, .-MA_BiosStop
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_SendRetry
MA_SendRetry:
    push	{lr}
    ldr	r0, [pc, #16]
    ldr	r1, [r0, #12]
    ldrh	r2, [r0, #2]
    mov	r3, #3
    bl	MA_InitIoBuffer
    pop	{r0}
    bx	r0
.align 2
    .word gMA+0x1e8
.size MA_SendRetry, .-MA_SendRetry
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_RecvRetry
MA_RecvRetry:
    push	{r4, r5, lr}
    ldr	r4, [pc, #72]
    ldr	r0, [pc, #72]
    add	r5, r4, r0
    ldr	r1, [r4, #12]
    mov	r0, r4
    mov	r2, #0
    mov	r3, #1
    bl	MA_InitIoBuffer
    ldr	r1, [pc, #60]
    add	r0, r4, r1
    mov	r2, #0
    strb	r2, [r0, #0]
    ldr	r0, [r5, #64]
    mov	r1, #9
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrb	r0, [r5, #4]
    mov	r0, #2
    strb	r0, [r5, #4]
    mov	r1, r4
    sub	r1, #16
    ldrh	r0, [r1, #0]
    mov	r0, #0
    strh	r2, [r1, #0]
    ldr	r1, [pc, #32]
    add	r2, r4, r1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    ldr	r1, [pc, #28]
    add	r2, r4, r1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA+0x1f8
    .word 0xfffffe08
    .word 0xfffffe5a
    .word 0xfffffe58
    .word 0xfffffe59
.size MA_RecvRetry, .-MA_RecvRetry
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MA_IntrTimer_SIOSend:
    push	{r4, r5, lr}
    ldr	r4, [pc, #24]
    mov	r0, #244
    lsl	r0, r0, #1
    add	r5, r4, r0
    ldrh	r0, [r5, #0]
    cmp	r0, #2
    beq	MA_IntrTimer_SIOSend+0x40
    cmp	r0, #2
    bgt	MA_IntrTimer_SIOSend+0x20
    cmp	r0, #1
    beq	MA_IntrTimer_SIOSend+0x26
    b	MA_IntrTimer_SIOSend+0x6e
.align 2
    .word gMA

    cmp	r0, #3
    beq	MA_IntrTimer_SIOSend+0x68
    b	MA_IntrTimer_SIOSend+0x6e
    mov	r0, r5
    bl	MA_SetTransmitData
    ldrh	r0, [r5, #0]
    mov	r0, #2
    strh	r0, [r5, #0]
    ldrh	r0, [r4, #12]
    ldr	r0, [pc, #4]
    strh	r0, [r4, #12]
    b	MA_IntrTimer_SIOSend+0x6e
.align 2
    .word 0x0000f851

    ldr	r1, [pc, #32]
    mov	r0, r5
    mov	r2, #18
    mov	r3, #3
    bl	MA_InitIoBuffer
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #1
    mov	r1, r4
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r4, #12]
    strh	r0, [r4, #12]
    mov	r0, r5
    bl	MA_SetTransmitData
    b	MA_IntrTimer_SIOSend+0x6e
.align 2
    .word MaPacketData_Start

    mov	r0, r5
    bl	MA_SetTransmitData
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.size MA_IntrTimer_SIOSend, .-MA_IntrTimer_SIOSend
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MA_IntrTimer_SIORecv:
    push	{lr}
    ldr	r1, [pc, #20]
    mov	r2, #252
    lsl	r2, r2, #1
    add	r0, r1, r2
    ldrh	r0, [r0, #0]
    cmp	r0, #4
    bgt	MA_IntrTimer_SIORecv+0x1c
    cmp	r0, #1
    bge	MA_IntrTimer_SIORecv+0x22
    b	MA_IntrTimer_SIORecv+0x6a
.align 2
    .word gMA

    cmp	r0, #5
    beq	MA_IntrTimer_SIORecv+0x60
    b	MA_IntrTimer_SIORecv+0x6a
    ldrb	r0, [r1, #5]
    cmp	r0, #0
    bne	MA_IntrTimer_SIORecv+0x40
    mov	r2, #130
    lsl	r2, r2, #2
    add	r0, r1, r2
    ldr	r1, [pc, #12]
    mov	r2, #1
    mov	r3, #0
    bl	MA_InitIoBuffer
    b	MA_IntrTimer_SIORecv+0x50
.align 2
    .word MaPacketData_PreStart

    mov	r2, #130
    lsl	r2, r2, #2
    add	r0, r1, r2
    ldr	r1, [pc, #16]
    mov	r2, #4
    mov	r3, #0
    bl	MA_InitIoBuffer
    ldr	r0, [pc, #8]
    bl	MA_SetTransmitData
    b	MA_IntrTimer_SIORecv+0x6a
.align 2
    .word MaPacketData_PreStart
    .word gMA+0x208

    mov	r2, #130
    lsl	r2, r2, #2
    add	r0, r1, r2
    bl	MA_SetTransmitData
    pop	{r0}
    bx	r0
.size MA_IntrTimer_SIORecv, .-MA_IntrTimer_SIORecv
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MA_IntrTimer_SIOIdle:
    push	{r4, r5, lr}
    ldr	r1, [pc, #152]
    mov	r2, r1
    add	r2, #97
    ldrb	r0, [r2, #0]
    mov	r4, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0x1c
    ldrb	r0, [r2, #0]
    cmp	r0, #6
    beq	MA_IntrTimer_SIOIdle+0x1c
    ldrb	r0, [r2, #0]
    cmp	r0, #7
    bne	MA_IntrTimer_SIOIdle+0xd6
    ldr	r0, [r4, #64]
    mov	r1, #1
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0xd6
    ldr	r0, [r4, #60]
    add	r0, #1
    str	r0, [r4, #60]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #2
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0xae
    ldrh	r1, [r4, #2]
    mov	r0, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0x4e
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #6
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0xae
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #2
    mov	r1, r4
    add	r1, #36
    add	r0, r0, r1
    ldr	r1, [r4, #60]
    ldr	r0, [r0, #0]
    cmp	r1, r0
    bls	MA_IntrTimer_SIOIdle+0xd6
    mov	r5, #0
    str	r5, [r4, #60]
    mov	r0, #240
    lsl	r0, r0, #1
    add	r3, r4, r0
    strh	r5, [r3, #0]
    mov	r0, r4
    add	r0, #212
    str	r0, [r3, #4]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #6
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer_SIOIdle+0xa0
    ldr	r1, [r4, #112]
    ldr	r2, [r4, #116]
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r3
    bl	MABIOS_Data2
    str	r5, [r4, #112]
    str	r5, [r4, #116]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #3
    orr	r0, r1
    str	r0, [r4, #64]
    b	MA_IntrTimer_SIOIdle+0xd6
.align 2
    .word gMA

    mov	r0, r3
    mov	r1, #0
    mov	r2, #0
    mov	r3, #255
    bl	MABIOS_Data
    b	MA_IntrTimer_SIOIdle+0xd6
    ldrb	r0, [r4, #5]
    lsl	r0, r0, #2
    mov	r1, r4
    add	r1, #20
    add	r0, r0, r1
    ldr	r1, [r4, #60]
    ldr	r0, [r0, #0]
    cmp	r1, r0
    bls	MA_IntrTimer_SIOIdle+0xd6
    mov	r1, #0
    str	r1, [r4, #60]
    mov	r2, #203
    lsl	r2, r2, #2
    add	r0, r4, r2
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #84
    str	r1, [r0, #4]
    bl	MABIOS_CheckStatus2
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.size MA_IntrTimer_SIOIdle, .-MA_IntrTimer_SIOIdle
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MA_IntrTimer_SIOWaitTime:
    push	{r4, r5, r6, r7, lr}
    ldr	r5, [pc, #20]
    ldr	r6, [r5, #64]
    mov	r0, #128
    lsl	r0, r0, #5
    and	r6, r0
    cmp	r6, #0
    beq	MA_IntrTimer_SIOWaitTime+0x1c
    bl	MABIOS_Start2
    b	MA_IntrTimer_SIOWaitTime+0x160
.align 2
    .word gMA

    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #4
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrTimer_SIOWaitTime+0x2a
    b	MA_IntrTimer_SIOWaitTime+0x128
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #216]
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #212]
    and	r0, r1
    ldrh	r1, [r5, #2]
    mov	r7, #0
    strh	r0, [r5, #2]
    ldrb	r0, [r5, #4]
    strb	r7, [r5, #4]
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #7
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrTimer_SIOWaitTime+0x50
    b	MA_IntrTimer_SIOWaitTime+0x160
    mov	r0, #0
    bl	MA_ChangeSIOMode
    mov	r0, r5
    add	r0, #92
    ldrb	r1, [r0, #0]
    strb	r7, [r0, #0]
    mov	r0, #0
    bl	MA_ChangeSIOMode
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    mov	r1, r5
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    strh	r0, [r5, #12]
    str	r6, [r5, #60]
    ldrb	r0, [r5, #4]
    strb	r7, [r5, #4]
    ldr	r0, [r5, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #136]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #132]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #128]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #112]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #108]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r4, #255
    mov	r0, r4
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    bl	MAU_Socket_Clear
    ldrh	r0, [r5, #2]
    and	r4, r0
    ldrh	r0, [r5, #2]
    strh	r4, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r5
    add	r0, #97
    ldrb	r0, [r0, #0]
    cmp	r0, #30
    beq	MA_IntrTimer_SIOWaitTime+0x160
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #48]
    and	r0, r1
    str	r0, [r5, #64]
    mov	r0, #38
    bl	MA_SetError
    str	r6, [r5, #60]
    ldrb	r0, [r5, #4]
    strb	r7, [r5, #4]
    b	MA_IntrTimer_SIOWaitTime+0x160
.align 2
    .word 0xfffff7ff
    .word 0x0000ffdf
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
    .word 0xffffbfff

    ldr	r2, [r5, #64]
    mov	r0, #8
    and	r2, r0
    cmp	r2, #0
    beq	MA_IntrTimer_SIOWaitTime+0x13e
    ldrb	r0, [r5, #4]
    mov	r0, #1
    strb	r0, [r5, #4]
    bl	MA_SendRetry
    b	MA_IntrTimer_SIOWaitTime+0x160
    ldrb	r0, [r5, #4]
    strb	r2, [r5, #4]
    mov	r1, #205
    lsl	r1, r1, #2
    add	r0, r5, r1
    ldr	r1, [r0, #0]
    mov	r3, #253
    lsl	r3, r3, #1
    add	r0, r5, r3
    ldrh	r0, [r0, #0]
    strh	r0, [r1, #0]
    str	r2, [r5, #60]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #28]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldr	r2, [pc, #24]
    ldrb	r0, [r2, #5]
    lsl	r0, r0, #1
    mov	r1, r2
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r2, #12]
    strh	r0, [r2, #12]
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0000ffdf
    .word gMA
.size MA_IntrTimer_SIOWaitTime, .-MA_IntrTimer_SIOWaitTime
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_ProcessCheckStatusResponse
MA_ProcessCheckStatusResponse:
    push	{r4, r5, r6, lr}
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    mov	r6, #0
    cmp	r0, #5
    bgt	MA_ProcessCheckStatusResponse+0x48
    cmp	r0, #4
    blt	MA_ProcessCheckStatusResponse+0x12
    b	MA_ProcessCheckStatusResponse+0x1b4
    cmp	r0, #1
    ble	MA_ProcessCheckStatusResponse+0x18
    b	MA_ProcessCheckStatusResponse+0x212
    cmp	r0, #0
    bge	MA_ProcessCheckStatusResponse+0x1e
    b	MA_ProcessCheckStatusResponse+0x212
    ldr	r2, [pc, #36]
    mov	r0, r2
    add	r0, #92
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    bne	MA_ProcessCheckStatusResponse+0x2c
    b	MA_ProcessCheckStatusResponse+0x1b4
    mov	r3, r2
    ldr	r0, [r3, #64]
    mov	r1, #4
    and	r0, r1
    cmp	r0, #0
    beq	MA_ProcessCheckStatusResponse+0x124
    mov	r0, #0
    str	r0, [r3, #64]
    mov	r0, #35
    bl	MA_SetError
    b	MA_ProcessCheckStatusResponse+0x126
.align 2
    .word gMA

    cmp	r0, #255
    beq	MA_ProcessCheckStatusResponse+0x4e
    b	MA_ProcessCheckStatusResponse+0x212
    mov	r6, #7
    ldr	r2, [pc, #32]
    mov	r0, r2
    add	r0, #92
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    bne	MA_ProcessCheckStatusResponse+0x5e
    b	MA_ProcessCheckStatusResponse+0x212
    ldr	r1, [r2, #64]
    mov	r0, #4
    and	r1, r0
    cmp	r1, #0
    beq	MA_ProcessCheckStatusResponse+0x78
    mov	r0, #0
    str	r0, [r2, #64]
    mov	r0, #35
    bl	MA_SetError
    b	MA_ProcessCheckStatusResponse+0x7a
.align 2
    .word gMA

    str	r1, [r2, #64]
    ldr	r5, [pc, #144]
    mov	r0, r5
    add	r0, #92
    ldrb	r1, [r0, #0]
    mov	r4, #0
    strb	r4, [r0, #0]
    mov	r0, #0
    bl	MA_ChangeSIOMode
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    mov	r1, r5
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    mov	r1, #0
    strh	r0, [r5, #12]
    str	r4, [r5, #60]
    ldrb	r0, [r5, #4]
    strb	r1, [r5, #4]
    ldr	r0, [r5, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #92]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #88]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #84]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #72]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #64]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r4, #255
    mov	r0, r4
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    bl	MAU_Socket_Clear
    ldrh	r0, [r5, #2]
    and	r4, r0
    ldrh	r0, [r5, #2]
    strh	r4, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    b	MA_ProcessCheckStatusResponse+0x212
.align 2
    .word gMA
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    str	r6, [r2, #64]
    ldr	r5, [pc, #164]
    mov	r0, r5
    add	r0, #92
    ldrb	r1, [r0, #0]
    mov	r4, #0
    strb	r4, [r0, #0]
    mov	r0, #0
    bl	MA_ChangeSIOMode
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    mov	r1, r5
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    mov	r1, #0
    strh	r0, [r5, #12]
    str	r4, [r5, #60]
    ldrb	r0, [r5, #4]
    strb	r1, [r5, #4]
    ldr	r0, [r5, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #112]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #108]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #104]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #92]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #84]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r4, #255
    mov	r0, r4
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    bl	MAU_Socket_Clear
    ldrh	r0, [r5, #2]
    and	r4, r0
    ldrh	r0, [r5, #2]
    strh	r4, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldr	r0, [pc, #20]
    add	r0, #92
    ldrb	r0, [r0, #0]
    sub	r0, #3
    cmp	r0, #5
    bhi	MA_ProcessCheckStatusResponse+0x212
    lsl	r0, r0, #2
    ldr	r1, [pc, #32]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word gMA
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
    .word .L_MA_ProcessCheckStatusResponse.0x1e8
.L_MA_ProcessCheckStatusResponse.0x1e8:
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x18
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x1c
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x20
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x2a
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x24
    .word .L_MA_ProcessCheckStatusResponse.0x1e8+0x28

    mov	r6, #1
    b	MA_ProcessCheckStatusResponse+0x212
    mov	r6, #4
    b	MA_ProcessCheckStatusResponse+0x212
    mov	r6, #5
    b	MA_ProcessCheckStatusResponse+0x212
    mov	r6, #2
    b	MA_ProcessCheckStatusResponse+0x212
    mov	r6, #3
    ldr	r4, [pc, #68]
    ldrh	r1, [r4, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r4, #2]
    mov	r2, #0
    mov	r3, #0
    strh	r0, [r4, #2]
    lsl	r0, r6, #8
    ldrh	r1, [r4, #2]
    orr	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    ldrb	r0, [r4, #4]
    strb	r2, [r4, #4]
    mov	r1, #244
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldrh	r1, [r0, #0]
    strh	r3, [r0, #0]
    mov	r1, #252
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldrh	r1, [r0, #0]
    strh	r3, [r0, #0]
    ldr	r0, [r4, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r4, #64]
    mov	r0, r6
    pop	{r4, r5, r6}
    pop	{r1}
    bx	r1
.align 2
    .word gMA
.size MA_ProcessCheckStatusResponse, .-MA_ProcessCheckStatusResponse
");
#endif

#if 0
#else
asm("
.section .rodata
.align 2
.type errTable.174, object
errTable.174:
    .word 0x13171615
    .word 0x00000013
.section .text

.align 2
.thumb_func
ConvertNegaErrToApiErr:
    ldr	r2, [pc, #28]
    ldr	r1, [pc, #32]
    mov	r0, r2
    add	r0, #81
    ldrb	r0, [r0, #0]
    add	r0, r0, r1
    ldrb	r0, [r0, #0]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    bx	lr
.align 2
    .word gMA
    .word errTable.174
.size ConvertNegaErrToApiErr, .-ConvertNegaErrToApiErr
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_DefaultNegaResProc
MA_DefaultNegaResProc:
    push	{lr}
    ldr	r1, [pc, #24]
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    sub	r0, #16
    mov	r2, r1
    cmp	r0, #24
    bhi	MA_DefaultNegaResProc+0xda
    lsl	r0, r0, #2
    ldr	r1, [pc, #8]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word gMA
    .word .L_MA_DefaultNegaResProc.0x24
.L_MA_DefaultNegaResProc.0x24:
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0x64
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb2
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb6
    .word .L_MA_DefaultNegaResProc.0x24+0xb2

    mov	r0, r2
    add	r0, #81
    ldrb	r0, [r0, #0]
    cmp	r0, #4
    bhi	MA_DefaultNegaResProc+0xcc
    lsl	r0, r0, #2
    ldr	r1, [pc, #4]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word .L_MA_DefaultNegaResProc.0xa0
.L_MA_DefaultNegaResProc.0xa0:
    .word .L_MA_DefaultNegaResProc.0xa0+0x14
    .word .L_MA_DefaultNegaResProc.0xa0+0x24
    .word .L_MA_DefaultNegaResProc.0xa0+0x1c
    .word .L_MA_DefaultNegaResProc.0xa0+0x24
    .word .L_MA_DefaultNegaResProc.0xa0+0x24

    mov	r1, r2
    add	r1, #102
    mov	r0, #18
    b	MA_DefaultNegaResProc+0xca
    mov	r1, r2
    add	r1, #102
    mov	r0, #23
    b	MA_DefaultNegaResProc+0xca
    mov	r1, r2
    add	r1, #102
    mov	r0, #19
    strb	r0, [r1, #0]
    mov	r1, r2
    add	r1, #104
    mov	r0, #0
    strh	r0, [r1, #0]
    b	MA_DefaultNegaResProc+0xda
    bl	ConvertNegaErrToApiErr
    pop	{r0}
    bx	r0
.size MA_DefaultNegaResProc, .-MA_DefaultNegaResProc
");
#endif

#if 0
#else
asm("
.lcomm pPacket.181, 0x4

.align 2
.thumb_func
MA_ProcessRecvPacket:
    push	{r4, r5, lr}
    lsl	r0, r0, #24
    lsr	r4, r0, #24
    ldr	r5, [pc, #44]
    mov	r0, #1
    str	r0, [r5, #0]
    ldr	r2, [pc, #40]
    ldr	r0, [pc, #44]
    str	r0, [r2, #0]
    ldr	r1, [pc, #44]
    add	r3, r0, r1
    ldr	r0, [r3, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    beq	MA_ProcessRecvPacket+0x48
    ldr	r0, [r3, #64]
    ldr	r1, [pc, #28]
    and	r0, r1
    str	r0, [r3, #64]
    bl	MA_BiosStop
    mov	r0, #0
    str	r0, [r5, #0]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word i
    .word pPacket.181
    .word gMA+0x218
    .word 0xfffffde8
    .word 0xffffefff

    mov	r0, r4
    sub	r0, #144
    cmp	r0, #94
    bls	MA_ProcessRecvPacket+0x52
    b	MA_ProcessRecvPacket+0x394
    lsl	r0, r0, #2
    ldr	r1, [pc, #4]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word .L_MA_ProcessRecvPacket.0x60
.L_MA_ProcessRecvPacket.0x60:
    .word .L_MA_ProcessRecvPacket.0x60+0x17c
    .word .L_MA_ProcessRecvPacket.0x60+0x194
    .word .L_MA_ProcessRecvPacket.0x60+0x208
    .word .L_MA_ProcessRecvPacket.0x60+0x21c
    .word .L_MA_ProcessRecvPacket.0x60+0x208
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x258
    .word .L_MA_ProcessRecvPacket.0x60+0x2a4
    .word .L_MA_ProcessRecvPacket.0x60+0x278
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x234
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x334
    .word .L_MA_ProcessRecvPacket.0x60+0x2bc

    mov	r0, #0
    bl	MA_SetInterval
    ldr	r0, [pc, #12]
    ldr	r1, [r0, #64]
    mov	r2, #1
    orr	r1, r2
    str	r1, [r0, #64]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA

    ldr	r2, [pc, #84]
    ldr	r0, [r2, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r2, #64]
    ldr	r0, [r2, #64]
    ldr	r1, [pc, #76]
    and	r0, r1
    str	r0, [r2, #64]
    ldr	r0, [r2, #64]
    ldr	r1, [pc, #72]
    and	r0, r1
    str	r0, [r2, #64]
    ldr	r0, [r2, #64]
    ldr	r1, [pc, #68]
    and	r0, r1
    str	r0, [r2, #64]
    ldrh	r1, [r2, #2]
    ldr	r0, [pc, #64]
    and	r0, r1
    ldrh	r1, [r2, #2]
    strh	r0, [r2, #2]
    mov	r1, r2
    add	r1, #97
    ldrb	r0, [r1, #0]
    cmp	r0, #2
    bne	MA_ProcessRecvPacket+0x22e
    b	MA_ProcessRecvPacket+0x3a8
    ldrb	r0, [r1, #0]
    cmp	r0, #9
    bne	MA_ProcessRecvPacket+0x236
    b	MA_ProcessRecvPacket+0x3a8
    ldrb	r0, [r2, #4]
    mov	r0, #3
    strb	r0, [r2, #4]
    ldrh	r0, [r2, #12]
    ldr	r0, [pc, #32]
    strh	r0, [r2, #12]
    ldr	r1, [pc, #32]
    mov	r0, #0
    str	r0, [r1, #0]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000ffef
    .word 0x0000bffd
    .word i

    ldr	r1, [pc, #12]
    ldrh	r2, [r1, #2]
    mov	r0, #16
    ldrh	r3, [r1, #2]
    orr	r0, r2
    strh	r0, [r1, #2]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA

    ldr	r1, [pc, #12]
    ldrh	r2, [r1, #2]
    ldr	r0, [pc, #12]
    and	r0, r2
    ldrh	r2, [r1, #2]
    strh	r0, [r1, #2]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA
    .word 0x0000ffef

    ldr	r3, [pc, #28]
    mov	r2, #205
    lsl	r2, r2, #2
    add	r0, r3, r2
    ldr	r1, [r0, #0]
    mov	r2, #253
    lsl	r2, r2, #1
    add	r0, r3, r2
    ldrh	r0, [r0, #0]
    strh	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #64
    ldrh	r2, [r3, #2]
    orr	r0, r1
    strh	r0, [r3, #2]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA

    mov	r0, #0
    bl	MA_ChangeSIOMode
    ldr	r1, [pc, #16]
    ldrb	r0, [r1, #4]
    mov	r2, #0
    mov	r0, #3
    strb	r0, [r1, #4]
    ldrh	r0, [r1, #12]
    ldr	r0, [pc, #8]
    b	MA_ProcessRecvPacket+0x2ee
.align 2
    .word gMA
    .word 0x0000dffe

    ldr	r0, [r2, #0]
    ldrb	r0, [r0, #6]
    bl	MA_ChangeSIOMode
    ldr	r1, [pc, #20]
    ldrb	r0, [r1, #4]
    mov	r2, #0
    mov	r0, #3
    strb	r0, [r1, #4]
    ldrh	r0, [r1, #12]
    ldr	r0, [pc, #12]
    strh	r0, [r1, #12]
    ldr	r0, [pc, #12]
    str	r2, [r0, #0]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA
    .word 0x0000fc28
    .word i

    ldr	r0, [pc, #16]
    mov	r3, #129
    lsl	r3, r3, #2
    add	r0, r0, r3
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    bl	MA_ProcessCheckStatusResponse
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA

    ldr	r0, [pc, #100]
    mov	ip, r0
    mov	r0, #129
    lsl	r0, r0, #2
    add	r0, ip
    ldr	r1, [r0, #0]
    ldrb	r2, [r1, #0]
    mov	r0, ip
    add	r0, #80
    ldrb	r3, [r0, #0]
    mov	r4, #0
    strb	r2, [r0, #0]
    ldrb	r1, [r1, #1]
    mov	r3, ip
    add	r3, #81
    ldrb	r2, [r3, #0]
    strb	r1, [r3, #0]
    ldrb	r0, [r0, #0]
    cmp	r0, #35
    bne	MA_ProcessRecvPacket+0x3a8
    mov	r1, ip
    add	r1, #83
    ldrb	r0, [r1, #0]
    add	r0, #1
    strb	r0, [r1, #0]
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #5
    beq	MA_ProcessRecvPacket+0x3a8
    mov	r1, ip
    ldrh	r0, [r1, #12]
    ldr	r0, [pc, #44]
    strh	r0, [r1, #12]
    ldr	r0, [r1, #64]
    mov	r1, #8
    orr	r0, r1
    mov	r2, ip
    str	r0, [r2, #64]
    ldrb	r0, [r2, #4]
    mov	r0, #3
    strb	r0, [r2, #4]
    ldr	r0, [pc, #28]
    str	r4, [r0, #0]
    ldr	r2, [pc, #28]
    str	r4, [r2, #0]
    mov	r3, ip
    ldrh	r1, [r3, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r2, #0]
    b	MA_ProcessRecvPacket+0x3a8
.align 2
    .word gMA
    .word 0x0000bffd
    .word i
    .word 0x0400010c

    ldr	r2, [pc, #64]
    mov	r1, #205
    lsl	r1, r1, #2
    add	r0, r2, r1
    ldr	r1, [r0, #0]
    mov	r3, #253
    lsl	r3, r3, #1
    add	r0, r2, r3
    ldrh	r0, [r0, #0]
    strh	r0, [r1, #0]
    ldr	r2, [pc, #44]
    ldr	r0, [r2, #64]
    mov	r1, #65
    neg	r1, r1
    and	r0, r1
    str	r0, [r2, #64]
    ldr	r0, [pc, #36]
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bne	MA_ProcessRecvPacket+0x3cc
    ldrb	r0, [r2, #4]
    mov	r0, #0
    strb	r0, [r2, #4]
    ldrh	r1, [r2, #2]
    ldr	r0, [pc, #24]
    and	r0, r1
    ldrh	r1, [r2, #2]
    strh	r0, [r2, #2]
    mov	r0, #0
    str	r0, [r2, #60]
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word i
    .word 0x0000ffdf
.size MA_ProcessRecvPacket, .-MA_ProcessRecvPacket
");
#endif

#if 0
#else
asm("
.lcomm saveSioMode.185, 0x1

.align 2
.thumb_func
.global MA_IntrTimer
MA_IntrTimer:
    push	{r4, r5, r6, lr}
    ldr	r4, [pc, #88]
    ldr	r0, [r4, #64]
    mov	r2, #128
    orr	r0, r2
    str	r0, [r4, #64]
    ldr	r6, [pc, #80]
    ldrb	r0, [r4, #4]
    strb	r0, [r6, #0]
    ldr	r5, [pc, #80]
    mov	r0, #0
    str	r0, [r5, #0]
    ldr	r0, [r4, #64]
    mov	r1, #2
    and	r0, r1
    mov	r3, r4
    cmp	r0, #0
    beq	MA_IntrTimer+0x44
    ldr	r0, [r4, #64]
    add	r1, #254
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrTimer+0x44
    ldr	r0, [r4, #64]
    mov	r1, #32
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrTimer+0x44
    ldr	r0, [pc, #44]
    ldrh	r1, [r0, #0]
    mov	r0, r2
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer+0x6c
    ldr	r2, [pc, #28]
    ldrh	r1, [r3, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r2, #0]
    ldr	r0, [r3, #64]
    mov	r1, #129
    neg	r1, r1
    and	r0, r1
    str	r0, [r3, #64]
    b	MA_IntrTimer+0xee
.align 2
    .word gMA
    .word saveSioMode.185
    .word 0x0400010c
    .word 0x04000128

    bl	MAAPI_Main
    ldrb	r1, [r4, #4]
    ldrb	r0, [r6, #0]
    cmp	r0, r1
    bne	MA_IntrTimer+0x8c
    ldr	r0, [r4, #64]
    mov	r1, #64
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrTimer+0xa2
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    bl	MA_ProcessRecvPacket
    ldr	r0, [r4, #64]
    mov	r1, #129
    neg	r1, r1
    and	r0, r1
    str	r0, [r4, #64]
    ldrh	r1, [r4, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r5, #0]
    b	MA_IntrTimer+0xee
    ldrb	r0, [r4, #4]
    cmp	r0, #1
    beq	MA_IntrTimer+0xc8
    cmp	r0, #1
    bgt	MA_IntrTimer+0xb2
    cmp	r0, #0
    beq	MA_IntrTimer+0xbc
    b	MA_IntrTimer+0xd2
    cmp	r0, #2
    beq	MA_IntrTimer+0xce
    cmp	r0, #3
    beq	MA_IntrTimer+0xc2
    b	MA_IntrTimer+0xd2
    bl	MA_IntrTimer_SIOIdle
    b	MA_IntrTimer+0xd2
    bl	MA_IntrTimer_SIOWaitTime
    b	MA_IntrTimer+0xd2
    bl	MA_IntrTimer_SIOSend
    b	MA_IntrTimer+0xd2
    bl	MA_IntrTimer_SIORecv
    bl	MA_StartSioTransmit
    ldr	r3, [pc, #28]
    ldr	r2, [pc, #28]
    ldrh	r1, [r2, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r3, #0]
    ldr	r0, [r2, #64]
    mov	r1, #129
    neg	r1, r1
    and	r0, r1
    str	r0, [r2, #64]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0400010c
    .word gMA
.size MA_IntrTimer, .-MA_IntrTimer
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_Bios_disconnect
MA_Bios_disconnect:
    push	{r4, r5, r6, lr}
    ldr	r6, [pc, #76]
    ldrb	r0, [r6, #4]
    mov	r4, #0
    strb	r4, [r6, #4]
    mov	r0, #244
    lsl	r0, r0, #1
    add	r5, r6, r0
    ldrh	r0, [r5, #0]
    strh	r4, [r5, #0]
    mov	r1, #252
    lsl	r1, r1, #1
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    strh	r4, [r0, #0]
    mov	r0, #16
    bl	MA_SetError
    ldr	r0, [r6, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r6, #64]
    ldrh	r1, [r6, #2]
    ldr	r0, [pc, #32]
    and	r0, r1
    ldrh	r1, [r6, #2]
    strh	r0, [r6, #2]
    ldr	r0, [r6, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r6, #64]
    ldrh	r0, [r5, #0]
    strh	r4, [r5, #0]
    ldr	r0, [pc, #16]
    str	r4, [r0, #0]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word 0x0000ffdf
    .word 0x0400010c
.size MA_Bios_disconnect, .-MA_Bios_disconnect
");
#endif

#if 0
#else
asm("
.lcomm dataLeft.192, 0x4

.align 2
.thumb_func
MA_IntrSio_Send:
    push	{r4, r5, r6, lr}
    ldr	r1, [pc, #80]
    mov	r2, #244
    lsl	r2, r2, #1
    add	r0, r1, r2
    ldrh	r0, [r0, #0]
    mov	r5, r1
    cmp	r0, #1
    bge	MA_IntrSio_Send+0x14
    b	MA_IntrSio_Send+0x3f6
    cmp	r0, #2
    bgt	MA_IntrSio_Send+0x1a
    b	MA_IntrSio_Send+0x3f6
    cmp	r0, #3
    beq	MA_IntrSio_Send+0x20
    b	MA_IntrSio_Send+0x3f6
    ldr	r0, [r5, #60]
    add	r0, #1
    str	r0, [r5, #60]
    ldr	r2, [r5, #60]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #2
    add	r1, #28
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    cmp	r2, r0
    bls	MA_IntrSio_Send+0x5e
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x58
    bl	MA_CancelRequest
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #7
    orr	r0, r1
    str	r0, [r5, #64]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word gMA

    bl	MA_BiosStop
    b	MA_IntrSio_Send+0x3f6
    ldr	r2, [pc, #48]
    mov	r3, #245
    lsl	r3, r3, #1
    add	r0, r5, r3
    ldrh	r1, [r0, #0]
    add	r3, #2
    add	r0, r5, r3
    ldrh	r0, [r0, #0]
    sub	r1, r1, r0
    str	r1, [r2, #0]
    ldrb	r0, [r5, #5]
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x98
    cmp	r1, #1
    bgt	MA_IntrSio_Send+0xc8
    mov	r0, #1
    sub	r0, r0, r1
    mov	r1, r5
    add	r1, #72
    add	r0, r0, r1
    ldr	r1, [pc, #12]
    ldrb	r1, [r1, #0]
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    b	MA_IntrSio_Send+0xc8
.align 2
    .word dataLeft.192
    .word 0x0400012a

    cmp	r1, #0
    bne	MA_IntrSio_Send+0xc8
    ldr	r0, [pc, #88]
    ldrb	r1, [r0, #0]
    mov	r0, r5
    add	r0, #72
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    ldr	r0, [pc, #80]
    ldrb	r0, [r0, #0]
    mov	r2, r5
    add	r2, #73
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    ldr	r0, [pc, #72]
    ldrb	r0, [r0, #0]
    add	r2, #1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    ldr	r0, [pc, #68]
    ldrb	r0, [r0, #0]
    add	r2, #1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    mov	r0, #245
    lsl	r0, r0, #1
    add	r1, r5, r0
    mov	r2, #246
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldrh	r1, [r1, #0]
    ldrh	r0, [r0, #0]
    cmp	r1, r0
    beq	MA_IntrSio_Send+0xde
    b	MA_IntrSio_Send+0x3f6
    mov	r0, r5
    add	r0, #72
    ldrb	r0, [r0, #0]
    cmp	r0, #255
    bne	MA_IntrSio_Send+0x108
    mov	r0, r5
    add	r0, #73
    ldrb	r0, [r0, #0]
    cmp	r0, #255
    bne	MA_IntrSio_Send+0x108
    mov	r0, #16
    b	MA_IntrSio_Send+0x20a
    lsl	r0, r0, #0
    lsl	r3, r4, #4
    lsl	r0, r0, #16
    lsl	r2, r4, #4
    lsl	r0, r0, #16
    lsl	r1, r4, #4
    lsl	r0, r0, #16
    lsl	r0, r4, #4
    lsl	r0, r0, #16
    mov	r0, r5
    add	r0, #72
    ldrb	r0, [r0, #0]
    bl	MA_IsSupportedHardware
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x1d0
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x1d0
    ldr	r0, [r5, #64]
    mov	r1, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrSio_Send+0x18c
    ldrh	r0, [r5, #14]
    sub	r0, #1
    ldrh	r1, [r5, #14]
    strh	r0, [r5, #14]
    ldrh	r0, [r5, #14]
    mov	r6, r0
    cmp	r6, #0
    bne	MA_IntrSio_Send+0x192
    ldrb	r0, [r5, #4]
    mov	r0, #0
    strb	r0, [r5, #4]
    mov	r3, #244
    lsl	r3, r3, #1
    add	r4, r5, r3
    ldrh	r0, [r4, #0]
    strh	r6, [r4, #0]
    mov	r1, #252
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    strh	r6, [r0, #0]
    mov	r0, #16
    bl	MA_SetError
    ldr	r0, [r5, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #24]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r0, [r4, #0]
    strh	r6, [r4, #0]
    ldr	r0, [pc, #8]
    str	r6, [r0, #0]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word 0x0000ffdf
    .word 0x0400010c

    ldrh	r0, [r5, #14]
    mov	r0, #2
    strh	r0, [r5, #14]
    ldr	r2, [pc, #48]
    mov	r0, r2
    add	r0, #73
    ldrb	r0, [r0, #0]
    ldrb	r1, [r2, #7]
    mov	r4, #0
    strb	r0, [r2, #7]
    ldrh	r0, [r2, #12]
    ldr	r0, [pc, #36]
    strh	r0, [r2, #12]
    ldr	r0, [r2, #64]
    mov	r1, #8
    orr	r0, r1
    str	r0, [r2, #64]
    ldrb	r0, [r2, #4]
    mov	r0, #3
    strb	r0, [r2, #4]
    ldr	r3, [pc, #20]
    str	r4, [r3, #0]
    ldrh	r1, [r2, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r3, #0]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word gMA
    .word 0x0000f851
    .word 0x0400010c

    ldr	r1, [pc, #92]
    mov	r0, r1
    add	r0, #73
    ldrb	r0, [r0, #0]
    mov	r2, r0
    mov	r5, r1
    cmp	r2, #242
    bne	MA_IntrSio_Send+0x260
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x260
    ldrb	r0, [r5, #7]
    strb	r2, [r5, #7]
    ldr	r0, [r5, #64]
    mov	r1, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrSio_Send+0x238
    ldrh	r0, [r5, #14]
    sub	r0, #1
    ldrh	r1, [r5, #14]
    strh	r0, [r5, #14]
    ldrh	r0, [r5, #14]
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x23e
    mov	r0, #133
    bl	MA_SetError
    ldr	r0, [r5, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #24]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word gMA
    .word 0x0000ffdf

    ldrh	r0, [r5, #14]
    ldr	r0, [pc, #32]
    strh	r0, [r5, #14]
    mov	r0, r5
    add	r0, #73
    ldrb	r0, [r0, #0]
    ldrb	r1, [r5, #7]
    mov	r3, #0
    strb	r0, [r5, #7]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #1
    mov	r1, r5
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r5, #12]
    b	MA_IntrSio_Send+0x348
.align 2
    .word 0x0000fffe

    mov	r2, r5
    add	r2, #73
    ldrb	r0, [r2, #0]
    cmp	r0, #240
    beq	MA_IntrSio_Send+0x270
    ldrb	r0, [r2, #0]
    cmp	r0, #241
    bne	MA_IntrSio_Send+0x2f0
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x2f0
    ldr	r0, [r5, #64]
    mov	r1, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrSio_Send+0x2d4
    ldrh	r0, [r5, #14]
    sub	r0, #1
    ldrh	r1, [r5, #14]
    strh	r0, [r5, #14]
    ldrh	r0, [r5, #14]
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x2da
    ldrb	r0, [r2, #0]
    cmp	r0, #240
    bne	MA_IntrSio_Send+0x2a2
    mov	r0, #131
    bl	MA_SetError
    b	MA_IntrSio_Send+0x2a8
    mov	r0, #132
    bl	MA_SetError
    ldr	r2, [pc, #32]
    ldr	r0, [r2, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r2, #64]
    ldrh	r1, [r2, #2]
    ldr	r0, [pc, #24]
    and	r0, r1
    ldrh	r1, [r2, #2]
    strh	r0, [r2, #2]
    ldr	r0, [r2, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r2, #64]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word gMA
    .word 0x0000ffdf

    ldrh	r0, [r5, #14]
    mov	r0, #2
    strh	r0, [r5, #14]
    mov	r0, r5
    add	r0, #73
    ldrb	r0, [r0, #0]
    ldrb	r1, [r5, #7]
    mov	r3, #0
    strb	r0, [r5, #7]
    ldrh	r0, [r5, #12]
    ldr	r0, [pc, #0]
    b	MA_IntrSio_Send+0x348
.align 2
    .word 0x0000bffd

    mov	r2, r5
    add	r2, #73
    ldrb	r1, [r2, #0]
    mov	r0, r5
    add	r0, #68
    ldrb	r0, [r0, #0]
    add	r0, #128
    cmp	r1, r0
    beq	MA_IntrSio_Send+0x370
    ldrb	r0, [r2, #0]
    cmp	r0, #238
    beq	MA_IntrSio_Send+0x370
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x370
    ldr	r0, [r5, #64]
    mov	r1, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrSio_Send+0x332
    ldrh	r0, [r5, #14]
    sub	r0, #1
    ldrh	r1, [r5, #14]
    strh	r0, [r5, #14]
    ldrh	r0, [r5, #14]
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x338
    bl	MA_Bios_disconnect
    b	MA_IntrSio_Send+0x3f6
    ldrh	r0, [r5, #14]
    mov	r0, #2
    strh	r0, [r5, #14]
    mov	r0, r5
    add	r0, #73
    ldrb	r0, [r0, #0]
    ldrb	r1, [r5, #7]
    mov	r3, #0
    strb	r0, [r5, #7]
    ldrh	r0, [r5, #12]
    ldr	r0, [pc, #32]
    strh	r0, [r5, #12]
    ldr	r0, [r5, #64]
    mov	r1, #8
    orr	r0, r1
    str	r0, [r5, #64]
    ldrb	r0, [r5, #4]
    mov	r0, #3
    strb	r0, [r5, #4]
    ldr	r2, [pc, #16]
    str	r3, [r2, #0]
    ldrh	r1, [r5, #12]
    mov	r0, #195
    lsl	r0, r0, #16
    orr	r0, r1
    str	r0, [r2, #0]
    b	MA_IntrSio_Send+0x3f6
.align 2
    .word 0x0000f851
    .word 0x0400010c

    mov	r0, r5
    add	r0, #72
    ldrb	r0, [r0, #0]
    ldrb	r1, [r5, #6]
    strb	r0, [r5, #6]
    ldrb	r0, [r5, #5]
    cmp	r0, #0
    bne	MA_IntrSio_Send+0x398
    mov	r2, #130
    lsl	r2, r2, #2
    add	r0, r5, r2
    ldr	r1, [pc, #12]
    mov	r2, #1
    mov	r3, #0
    bl	MA_InitIoBuffer
    b	MA_IntrSio_Send+0x3a8
.align 2
    .word MaPacketData_PreStart

    mov	r3, #130
    lsl	r3, r3, #2
    add	r0, r5, r3
    ldr	r1, [pc, #92]
    mov	r2, #4
    mov	r3, #0
    bl	MA_InitIoBuffer
    ldr	r4, [pc, #84]
    ldr	r0, [pc, #88]
    add	r5, r4, r0
    mov	r1, #158
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldr	r0, [r0, #0]
    ldr	r1, [r0, #4]
    mov	r0, r4
    mov	r2, #0
    mov	r3, #1
    bl	MA_InitIoBuffer
    ldr	r2, [pc, #68]
    add	r0, r4, r2
    mov	r2, #0
    strb	r2, [r0, #0]
    ldr	r0, [r5, #64]
    mov	r1, #9
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrb	r0, [r5, #4]
    mov	r0, #2
    strb	r0, [r5, #4]
    mov	r1, r4
    sub	r1, #16
    ldrh	r0, [r1, #0]
    mov	r0, #0
    strh	r2, [r1, #0]
    str	r2, [r5, #60]
    ldr	r3, [pc, #36]
    add	r2, r4, r3
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    ldr	r1, [pc, #32]
    add	r2, r4, r1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word MaPacketData_PreStart
    .word gMA+0x1f8
    .word 0xfffffe08
    .word 0xfffffe5a
    .word 0xfffffe58
    .word 0xfffffe59
.size MA_IntrSio_Send, .-MA_IntrSio_Send
");
#endif

#if 0
#else
asm("
.lcomm recvByte.196, 0x1
.lcomm amari.197, 0x4

.align 2
.thumb_func
MA_IntrSio_Recv:
    push	{r4, r5, r6, lr}
    lsl	r0, r0, #24
    lsr	r6, r0, #24
    ldr	r1, [pc, #16]
    ldrb	r0, [r1, #5]
    mov	r5, r1
    cmp	r0, #0
    bne	MA_IntrSio_Recv+0x24
    ldr	r0, [pc, #8]
    ldr	r1, [pc, #12]
    b	MA_IntrSio_Recv+0x2a
.align 2
    .word gMA
    .word recvByte.196
    .word 0x0400012a

    ldr	r0, [pc, #36]
    ldr	r1, [pc, #40]
    sub	r1, r1, r6
    ldrb	r1, [r1, #0]
    strb	r1, [r0, #0]
    mov	r4, r0
    mov	r1, #252
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldrh	r0, [r0, #0]
    sub	r0, #1
    cmp	r0, #4
    bls	MA_IntrSio_Recv+0x40
    b	MA_IntrSio_Recv+0x372
    lsl	r0, r0, #2
    ldr	r1, [pc, #16]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word recvByte.196
    .word 0x04000123
    .word .L_MA_IntrSio_Recv.0x58
.L_MA_IntrSio_Recv.0x58:
    .word .L_MA_IntrSio_Recv.0x58+0x14
    .word .L_MA_IntrSio_Recv.0x58+0xb8
    .word .L_MA_IntrSio_Recv.0x58+0x184
    .word .L_MA_IntrSio_Recv.0x58+0x208
    .word .L_MA_IntrSio_Recv.0x58+0x2c0

    mov	r2, #254
    lsl	r2, r2, #1
    add	r3, r5, r2
    ldrh	r0, [r3, #0]
    cmp	r0, #0
    beq	MA_IntrSio_Recv+0x7e
    cmp	r0, #1
    beq	MA_IntrSio_Recv+0xe0
    b	MA_IntrSio_Recv+0x372
    cmp	r6, #0
    beq	MA_IntrSio_Recv+0x84
    b	MA_IntrSio_Recv+0x372
    ldr	r0, [r5, #60]
    add	r0, #1
    str	r0, [r5, #60]
    ldr	r2, [r5, #60]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #2
    mov	r1, r5
    add	r1, #28
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    cmp	r2, r0
    bls	MA_IntrSio_Recv+0xac
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Recv+0xaa
    b	MA_IntrSio_Recv+0x200
    b	MA_IntrSio_Recv+0x210
    ldrb	r0, [r4, #0]
    cmp	r0, #153
    bne	MA_IntrSio_Recv+0xbc
    ldrh	r0, [r3, #0]
    add	r0, #1
    ldrh	r1, [r3, #0]
    strh	r0, [r3, #0]
    b	MA_IntrSio_Recv+0x372
    cmp	r0, #210
    bne	MA_IntrSio_Recv+0xc2
    b	MA_IntrSio_Recv+0x372
    mov	r4, r5
    add	r4, #82
    ldrb	r0, [r4, #0]
    add	r0, #1
    strb	r0, [r4, #0]
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #20
    bhi	MA_IntrSio_Recv+0xd6
    b	MA_IntrSio_Recv+0x372
    mov	r0, #16
    bl	MA_SetError
    strb	r6, [r4, #0]
    b	MA_IntrSio_Recv+0x372
    ldrb	r0, [r4, #0]
    cmp	r0, #102
    bne	MA_IntrSio_Recv+0xfa
    mov	r0, #252
    lsl	r0, r0, #1
    add	r1, r5, r0
    ldrh	r0, [r1, #0]
    mov	r2, #0
    mov	r0, #2
    strh	r0, [r1, #0]
    ldrh	r0, [r3, #0]
    strh	r2, [r3, #0]
    b	MA_IntrSio_Recv+0x100
    mov	r0, #16
    bl	MA_SetError
    ldr	r0, [pc, #8]
    add	r0, #82
    mov	r1, #0
    strb	r1, [r0, #0]
    b	MA_IntrSio_Recv+0x372
.align 2
    .word gMA

    mov	r1, #254
    lsl	r1, r1, #1
    add	r2, r5, r1
    ldrh	r0, [r2, #0]
    mov	r1, r0
    cmp	r1, #1
    beq	MA_IntrSio_Recv+0x160
    cmp	r1, #1
    bgt	MA_IntrSio_Recv+0x128
    cmp	r1, #0
    beq	MA_IntrSio_Recv+0x132
    b	MA_IntrSio_Recv+0x160
    cmp	r1, #2
    beq	MA_IntrSio_Recv+0x13e
    cmp	r1, #3
    beq	MA_IntrSio_Recv+0x150
    b	MA_IntrSio_Recv+0x160
    ldrb	r0, [r4, #0]
    mov	r2, r5
    add	r2, #69
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    b	MA_IntrSio_Recv+0x160
    ldrh	r1, [r2, #0]
    mov	r0, #3
    sub	r0, r0, r1
    mov	r2, #253
    lsl	r2, r2, #1
    add	r1, r5, r2
    add	r0, r0, r1
    mov	r1, #0
    b	MA_IntrSio_Recv+0x15e
    ldrh	r0, [r2, #0]
    sub	r0, r1, r0
    mov	r3, #253
    lsl	r3, r3, #1
    add	r1, r5, r3
    add	r0, r0, r1
    ldrb	r1, [r4, #0]
    strb	r1, [r0, #0]
    mov	r0, #254
    lsl	r0, r0, #1
    add	r3, r5, r0
    ldrh	r0, [r3, #0]
    add	r0, #1
    ldrh	r1, [r3, #0]
    strh	r0, [r3, #0]
    mov	r2, #255
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldrb	r2, [r4, #0]
    ldrh	r0, [r1, #0]
    add	r0, r0, r2
    ldrh	r2, [r1, #0]
    strh	r0, [r1, #0]
    ldrh	r0, [r3, #0]
    mov	r6, r0
    cmp	r6, #4
    beq	MA_IntrSio_Recv+0x188
    b	MA_IntrSio_Recv+0x372
    mov	r0, #253
    lsl	r0, r0, #1
    add	r4, r5, r0
    ldrh	r0, [r4, #0]
    mov	r2, r0
    cmp	r2, #0
    bne	MA_IntrSio_Recv+0x1a2
    mov	r1, #252
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    strh	r6, [r0, #0]
    b	MA_IntrSio_Recv+0x25a
    ldrb	r0, [r5, #5]
    cmp	r0, #0
    beq	MA_IntrSio_Recv+0x1b6
    ldr	r1, [pc, #20]
    ldrh	r0, [r4, #0]
    mov	r2, #3
    and	r2, r0
    str	r2, [r1, #0]
    cmp	r2, #0
    bne	MA_IntrSio_Recv+0x1c4
    ldrh	r0, [r4, #0]
    ldrh	r1, [r3, #0]
    strh	r0, [r3, #0]
    b	MA_IntrSio_Recv+0x1ce
.align 2
    .word amari.197

    sub	r0, r2, #4
    ldrh	r1, [r4, #0]
    sub	r1, r1, r0
    ldrh	r0, [r3, #0]
    strh	r1, [r3, #0]
    mov	r2, #252
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldrh	r1, [r0, #0]
    mov	r1, #3
    strh	r1, [r0, #0]
    b	MA_IntrSio_Recv+0x372
    ldr	r0, [r5, #60]
    add	r0, #1
    str	r0, [r5, #60]
    ldr	r2, [r5, #60]
    ldrb	r0, [r5, #5]
    lsl	r0, r0, #2
    mov	r1, r5
    add	r1, #28
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    cmp	r2, r0
    bls	MA_IntrSio_Recv+0x216
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Recv+0x210
    bl	MA_CancelRequest
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #7
    orr	r0, r1
    str	r0, [r5, #64]
    b	MA_IntrSio_Recv+0x372
    bl	MA_BiosStop
    b	MA_IntrSio_Recv+0x372
    mov	r3, #128
    lsl	r3, r3, #2
    add	r2, r5, r3
    ldr	r0, [r2, #0]
    ldrb	r1, [r4, #0]
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r2, #0]
    mov	r0, #254
    lsl	r0, r0, #1
    add	r3, r5, r0
    ldrh	r0, [r3, #0]
    sub	r0, #1
    ldrh	r1, [r3, #0]
    strh	r0, [r3, #0]
    mov	r2, #255
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldrb	r2, [r4, #0]
    ldrh	r0, [r1, #0]
    add	r0, r0, r2
    ldrh	r2, [r1, #0]
    strh	r0, [r1, #0]
    ldrh	r0, [r3, #0]
    mov	r2, r0
    cmp	r2, #0
    beq	MA_IntrSio_Recv+0x24e
    b	MA_IntrSio_Recv+0x372
    mov	r0, #252
    lsl	r0, r0, #1
    add	r1, r5, r0
    ldrh	r0, [r1, #0]
    mov	r0, #4
    strh	r0, [r1, #0]
    ldrh	r0, [r3, #0]
    strh	r2, [r3, #0]
    b	MA_IntrSio_Recv+0x372
    mov	r1, #254
    lsl	r1, r1, #1
    add	r2, r5, r1
    ldrh	r1, [r2, #0]
    mov	r0, #1
    sub	r0, r0, r1
    mov	r6, r5
    add	r6, #70
    add	r0, r0, r6
    ldrb	r1, [r4, #0]
    strb	r1, [r0, #0]
    ldrh	r0, [r2, #0]
    add	r0, #1
    ldrh	r1, [r2, #0]
    strh	r0, [r2, #0]
    ldrh	r0, [r2, #0]
    cmp	r0, #2
    bne	MA_IntrSio_Recv+0x372
    mov	r2, #130
    lsl	r2, r2, #2
    add	r0, r5, r2
    mov	r3, #202
    lsl	r3, r3, #2
    add	r4, r5, r3
    mov	r1, r4
    mov	r2, #4
    mov	r3, #0
    bl	MA_InitIoBuffer
    mov	r0, #129
    strb	r0, [r4, #0]
    mov	r1, #255
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    ldrh	r0, [r6, #0]
    cmp	r0, r1
    beq	MA_IntrSio_Recv+0x2d0
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #5
    and	r0, r1
    cmp	r0, #0
    bne	MA_IntrSio_Recv+0x2d0
    ldr	r2, [pc, #16]
    add	r1, r5, r2
    mov	r0, #241
    strb	r0, [r1, #0]
    ldr	r0, [r5, #64]
    mov	r1, #16
    orr	r0, r1
    str	r0, [r5, #64]
    b	MA_IntrSio_Recv+0x2e0
    lsl	r0, r0, #0
    lsl	r1, r5, #12
    lsl	r0, r0, #0
    ldr	r2, [pc, #52]
    mov	r0, r2
    add	r0, #69
    ldrb	r1, [r0, #0]
    ldr	r3, [pc, #48]
    add	r0, r2, r3
    strb	r1, [r0, #0]
    mov	r5, r2
    ldr	r1, [pc, #44]
    add	r0, r5, r1
    mov	r2, #0
    strb	r2, [r0, #0]
    ldr	r3, [pc, #40]
    add	r0, r5, r3
    strb	r2, [r0, #0]
    mov	r0, #252
    lsl	r0, r0, #1
    add	r1, r5, r0
    ldrh	r0, [r1, #0]
    mov	r0, #5
    strh	r0, [r1, #0]
    mov	r1, #254
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    strh	r2, [r0, #0]
    b	MA_IntrSio_Recv+0x372
.align 2
    .word gMA
    .word 0x00000329
    .word 0x0000032b
    .word 0x0000032a

    mov	r2, #254
    lsl	r2, r2, #1
    add	r3, r5, r2
    ldrh	r0, [r3, #0]
    mov	r1, r5
    add	r1, #76
    add	r0, r0, r1
    ldrb	r1, [r4, #0]
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    ldrh	r0, [r3, #0]
    add	r0, #1
    ldrh	r1, [r3, #0]
    strh	r0, [r3, #0]
    ldrb	r0, [r5, #5]
    cmp	r0, #0
    bne	MA_IntrSio_Recv+0x340
    ldrh	r0, [r3, #0]
    cmp	r0, #2
    beq	MA_IntrSio_Recv+0x34c
    ldrb	r0, [r5, #5]
    cmp	r0, #1
    bne	MA_IntrSio_Recv+0x372
    ldrh	r0, [r3, #0]
    cmp	r0, #4
    bne	MA_IntrSio_Recv+0x372
    ldr	r2, [r5, #64]
    mov	r0, #16
    and	r2, r0
    cmp	r2, #0
    beq	MA_IntrSio_Recv+0x366
    ldr	r0, [r5, #64]
    mov	r1, #17
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    bl	MA_RecvRetry
    b	MA_IntrSio_Recv+0x372
    ldr	r0, [r5, #64]
    mov	r1, #64
    orr	r0, r1
    str	r0, [r5, #64]
    ldrb	r0, [r5, #4]
    strb	r2, [r5, #4]
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.size MA_IntrSio_Recv, .-MA_IntrSio_Recv
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_IntrSerialIO
MA_IntrSerialIO:
    push	{r4, lr}
    ldr	r2, [pc, #36]
    ldr	r0, [r2, #0]
    ldr	r1, [pc, #36]
    and	r0, r1
    str	r0, [r2, #0]
    ldr	r4, [pc, #32]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #1
    orr	r0, r1
    str	r0, [r4, #64]
    ldrb	r0, [r4, #4]
    mov	r1, r0
    cmp	r1, #1
    beq	MA_IntrSerialIO+0x34
    cmp	r1, #2
    beq	MA_IntrSerialIO+0x42
    b	MA_IntrSerialIO+0x7c
.align 2
    .word 0x0400010c
    .word 0xffbfffff
    .word gMA

    bl	MA_IntrSio_Send
    ldr	r0, [r4, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r4, #64]
    b	MA_IntrSerialIO+0x7c
    ldrb	r0, [r4, #5]
    cmp	r0, #0
    bne	MA_IntrSerialIO+0x50
    mov	r0, #0
    bl	MA_IntrSio_Recv
    b	MA_IntrSerialIO+0x7c
    ldr	r2, [pc, #76]
    mov	r0, #0
    str	r0, [r2, #0]
    ldrh	r0, [r4, #2]
    and	r1, r0
    cmp	r1, #0
    bne	MA_IntrSerialIO+0x7c
    mov	r4, r2
    ldrb	r0, [r4, #0]
    bl	MA_IntrSio_Recv
    ldr	r0, [r4, #0]
    add	r0, #1
    str	r0, [r4, #0]
    cmp	r0, #3
    bgt	MA_IntrSerialIO+0x7c
    ldr	r0, [pc, #48]
    ldrh	r1, [r0, #2]
    mov	r0, #2
    and	r0, r1
    cmp	r0, #0
    beq	MA_IntrSerialIO+0x60
    ldr	r2, [pc, #40]
    ldr	r0, [r2, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    orr	r0, r1
    str	r0, [r2, #0]
    ldr	r2, [pc, #24]
    ldr	r0, [r2, #64]
    mov	r1, #2
    orr	r0, r1
    str	r0, [r2, #64]
    ldr	r0, [r2, #64]
    ldr	r1, [pc, #20]
    and	r0, r1
    str	r0, [r2, #64]
    pop	{r4}
    pop	{r0}
    bx	r0
.align 2
    .word i
    .word gMA
    .word 0x0400010c
    .word 0xfffffeff
.size MA_IntrSerialIO, .-MA_IntrSerialIO
");
#endif
