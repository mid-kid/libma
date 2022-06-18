#include "ma_api.h"
#include "libma.h"

#include <stddef.h>
#include "ma_bios.h"
#include "ma_var.h"
#include "ma_sub.h"

#define CASSETTE_INITIAL_CODE (ROM_BANK0 + 0xac)
#define CASSETTE_VERSION_NO (ROM_BANK0 + 0xbc)

//static void MA_SetApiError();
//static void ApiValisStatusCheck();
//static void MA_ApiPreExe();
//static void MakeEndLineBuffer();
//static void IsEndMultiLine();
//static void InitPrevBuf();
//static void ConcatPrevBuf();
//static void MATASK_Stop();
//static void MATASK_TCP_Cut();
static void MA_InitLibraryMain(u8 *pHardwareType, int task);
//static void MATASK_InitLibrary();
//static void MATASK_TCP_Connect();
//static void MATASK_TCP_Disconnect();
//static void MATASK_TCP_SendRecv();
//static void MATASK_GetHostAddress();
//static void EEPROMSumCheck();
//static void EEPROMRegistrationCheck();
//static void MATASK_TelServer();
//static void MATASK_Tel();
//static void MATASK_Receive();
//static void MATASK_P2P();
static void MA_ConditionMain(u8 *pCondition, int task);
//static void MATASK_Condition();
//static void MATASK_Offline();
//static void CheckSMTPResponse();
//static void MATASK_SMTP_Connect();
//static void MATASK_SMTP_Sender();
//static void MATASK_SMTP_Send();
//static void MATASK_SMTP_POP3_Quit();
//static void CheckPOP3Response();
//static void MATASK_POP3_Connect();
//static void MATASK_POP3_Stat();
//static void MATASK_POP3_List();
//static void MATASK_POP3_Retr();
//static void MATASK_POP3_Dele();
//static void MATASK_POP3_Head();
//static void ExtractServerName();
//static void MA_HTTP_GetPost();
//static void ConcatUserAgent();
//static void GetRequestType();
//static void CreateHttpRequestHeader();
//static void HttpGetNextStep();
//static void MATASK_HTTP_GetPost();
//static void CopyEEPROMString();
//static void CopyEEPROMData();
//static void MATASK_GetEEPROMData();
//static void MATASK_EEPROM_Read();
//static void MATASK_EEPROM_Write();
//static void ErrDetailHexConv();

void SetApiCallFlag(void)
{
    *(vu16 *)REG_IE &= ~SIO_INTR_FLAG & ~TIMER3_INTR_FLAG;
    gMA.status |= STATUS_UNK_5;
}

void ResetApiCallFlag(void)
{
    gMA.status &= ~STATUS_UNK_5;
    *(vu16 *)REG_IE |= SIO_INTR_FLAG | TIMER3_INTR_FLAG;
}

void MA_TaskSet(u8 unk_1, u8 unk_2)
{
    gMA.timer_unk_12 = gMA.timer[gMA.sio_mode];
    gMA.task = unk_1;
    gMA.task_unk_98 = unk_2;
    if (gMA.task == 0) gMA.condition &= ~MA_CONDITION_APIWAIT;
}

static void MA_SetApiError(u8 error, u16 unk_2)
{
    gMA.unk_96 = 0;
    gMA.unk_94 = unk_2;
    MA_SetError(error);
}

static int ApiValisStatusCheck(u8 unk_1)
{
    static int ret asm("ret.15");

    ret = TRUE;
    switch (unk_1) {
    case 0x3:
        if (gMA.unk_92 == 0) break;
        ret = FALSE;
        break;

    case 0x4:
        if (gMA.unk_92 == 0) break;
        ret = FALSE;
        break;

    case 0x5:
        if (gMA.unk_92 == 0) break;
        ret = FALSE;
        break;

    case 0x6:
        if (gMA.unk_92 == 7) break;
        if (gMA.unk_92 == 8) break;
        ret = FALSE;
        break;

    case 0x7:
        if (gMA.unk_92 == 7) break;
        if (gMA.unk_92 == 8) break;
        ret = FALSE;
        break;

    case 0xa:
        if (gMA.unk_92 == 3) break;
        if (gMA.unk_92 == 7) break;
        if (gMA.unk_92 == 8) break;
        if (gMA.unk_92 == 4) break;
        if (gMA.unk_92 == 5) break;
        ret = FALSE;
        break;

    case 0xc:
    case 0xd:
    case 0xe:
        if (gMA.unk_92 == 4) break;
        ret = FALSE;
        break;

    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
        if (gMA.unk_92 == 5) break;
        ret = FALSE;
        break;

    case 0x16:
    case 0x17:
        if (gMA.unk_92 == 3) break;
        if (gMA.unk_92 == 6) break;
        ret = FALSE;
        break;

    case 0x18:
    case 0x19:
    case 0x1a:
        if (gMA.unk_92 == 0) break;
        ret = FALSE;
        break;

    case 0x1c:
    case 0x1d:
        if (gMA.unk_92 == 0) break;
        ret = FALSE;
        break;

    case 0xb:
    case 0xf:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
        if (gMA.unk_92 == 3) break;
        ret = FALSE;
        break;
    }

    return ret;
}

static int MA_ApiPreExe(u8 unk_1)
{
    gMA.error = 0;
    gMA.unk_94 = 0;

    if (gMA.unk_88 != 0x4247414d) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return FALSE;
    }
    if (gMA.status & STATUS_UNK_7 || gMA.status & STATUS_UNK_8) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return FALSE;
    }
    if (gMA.condition & MA_CONDITION_ERROR) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return FALSE;
    }
    if (gMA.task != 0) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return FALSE;
    }
    if (!ApiValisStatusCheck(unk_1)) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return FALSE;
    }

    if (unk_1 != 0x16 && unk_1 != 0x17) {
        gMA.condition &= ~MA_CONDITION_UNK_6;
    }
    gMA.condition &= ~MA_CONDITION_ERROR;
    gMA.error = -1;
    gMA.unk_102 = 0;
    gMA.unk_104 = 0;
    gMA.unk_1794 = 0;
    gMA.unk_1796 = 0;
    gMA.condition |= MA_CONDITION_APIWAIT;
    return TRUE;
}

static void MakeEndLineBuffer(u8 *end, int size)
{
    gMA.unk_1788[5] = 0;
    if (size == 0) return;
    if (end == NULL) return;

    if (size >= 5) {
        MAU_memcpy(gMA.unk_1788, end, 5);
    } else {
        MAU_memcpy(gMA.unk_1788, &gMA.unk_1788[size], 5 - size);
        MAU_memcpy(&gMA.unk_1788[5 - size], end, size);
    }
}

static int IsEndMultiLine(void)
{
    static const char strEndMultiline[] asm("strEndMultiLine.25") = "\r\n.\r\n";

    if (MAU_strcmp(gMA.unk_1788, strEndMultiline) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static const char POP3_Quit[] asm(".LPOP3_Quit") = "QUIT\r\n";
static const char POP3_Helo[] asm(".LPOP3_Helo") = "HELO ";
static const char POP3_Newl[] asm(".LPOP3_Newl") = "\r\n";
static const char POP3_From[] asm(".LPOP3_From") = "MAIL FROM:<";
static const char POP3_From_Newl[] asm(".LPOP3_From_Newl") = ">\r\n";
static const char POP3_Rcpt[] asm(".LPOP3_Rcpt") = "RCPT TO:<";
static const char POP3_Data[] asm(".LPOP3_Data") = "DATA\r\n";
static const char POP3_User[] asm(".LPOP3_User") = "USER ";
static const char POP3_Pass[] asm(".LPOP3_Pass") = "PASS ";
static const char POP3_Stat[] asm(".LPOP3_Stat") = "STAT\r\n";
static const char POP3_List[] asm(".LPOP3_List") = "LIST ";
static const char POP3_Retr[] asm(".LPOP3_Retr") = "RETR ";
static const char POP3_Dele[] asm(".LPOP3_Dele") = "DELE ";
static const char POP3_Top[] asm(".LPOP3_Top") = "TOP ";
static const char POP3_Zero[] asm(".LPOP3_Zero") = " 0\r\n";

static void InitPrevBuf(void)
{
    gMA.prevbuf[0] = 0;
    gMA.prevbuf_size = 0;
    gMA.unk_101 = 0;
}

static void ConcatPrevBuf(u8 *data, u16 size)
{
    MAU_memcpy(&gMA.prevbuf[gMA.prevbuf_size], data, size);
    gMA.prevbuf_size += size;
}

void MA_End(void)
{
    int zero;

    if (gMA.unk_92) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return;
    }

    *(vu32 *)REG_TM3CNT = 0;
    *(vu16 *)REG_SIOCNT = 0;
    *(vu16 *)REG_IE &= ~SIO_INTR_FLAG & ~TIMER3_INTR_FLAG;
    zero = 0;
    CpuSet(&zero, &gMA, DMA_SRC_FIX | DMA_32BIT_BUS | (sizeof(gMA) / 4));
}

void MA_Stop(void)
{
    SetApiCallFlag();
    if (gMA.unk_88 != 0x4247414d) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return;
    }

    if (gMA.condition & MA_CONDITION_ERROR) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (gMA.task == TASK_UNK_1E ||
            (!gMA.unk_92 && !(gMA.condition & MA_CONDITION_APIWAIT))) {
        ResetApiCallFlag();
        return;
    }

    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;

    if (gMA.condition & MA_CONDITION_UNK_5 && gMA.cmd_cur == MACMD_TEL) {
        gMA.condition |= MA_CONDITION_APIWAIT;
        MA_TaskSet(TASK_UNK_1E, 0);
        MA_CancelRequest();
        ResetApiCallFlag();
        return;
    }

    gMA.condition |= MA_CONDITION_APIWAIT;
    MA_TaskSet(TASK_UNK_1E, 1);
    ResetApiCallFlag();
}

static void MATASK_Stop(void)
{
    switch (gMA.task_unk_98) {  // MAGIC
    case 0:
        gMA.task_unk_98++;

    case 1:
        MA_BiosStop();
        gMA.task_unk_98++;
        break;

    case 2:
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

        gMA.recv_cmd = 0;
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

#if 0

void MA_TCP_Cut(void)
{
    SetApiCallFlag();
    if (gMA.unk_88 != 0x4247414d) {  // MAGIC
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        return;
    }

    if (gMA.unk_92 == 3 && gMA.task == 0) {
        ResetApiCallFlag();
        return;
    }

    if (!(gMA.unk_92 == 6 || gMA.unk_92 == 4 || gMA.unk_92 == 5 ||
        (0x0a < gMA.task && gMA.task < 0x18 && gMA.condition & 1))) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (gMA.condition & MA_CONDITION_ERROR) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (gMA.task == TASK_UNK_1F || gMA.task == TASK_UNK_1E) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)(u32)gMA.cmd_cur;
    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
    if (!(gMA.condition & MA_CONDITION_UNK_5 || gMA.unk_92 == 3)) {
        gMA.condition |= MA_CONDITION_APIWAIT;
        MA_TaskSet(TASK_UNK_1F, 2);
    } else {
        gMA.condition |= MA_CONDITION_APIWAIT;
        MA_TaskSet(TASK_UNK_1F, 0);
    }
    ResetApiCallFlag();
}

#else
void MA_TCP_Cut(void);
asm("
.align 2
.thumb_func
.global MA_TCP_Cut
MA_TCP_Cut:
    push	{lr}
    bl	SetApiCallFlag
    ldr	r2, [pc, #20]
    ldr	r1, [r2, #88]
    ldr	r0, [pc, #20]
    cmp	r1, r0
    beq	MA_TCP_Cut+0x24
    mov	r0, #33
    mov	r1, #0
    bl	MA_SetApiError
    b	MA_TCP_Cut+0xf2
.align 2
    .word gMA
    .word 0x4247414d

    mov	r0, r2
    add	r0, #92
    ldrb	r0, [r0, #0]
    cmp	r0, #3
    bne	MA_TCP_Cut+0x38
    mov	r0, r2
    add	r0, #97
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    beq	MA_TCP_Cut+0x8c
    ldr	r1, [pc, #88]
    mov	r2, r1
    add	r2, #92
    ldrb	r0, [r2, #0]
    mov	r3, r1
    cmp	r0, #6
    beq	MA_TCP_Cut+0x6a
    ldrb	r0, [r2, #0]
    cmp	r0, #4
    beq	MA_TCP_Cut+0x6a
    ldrb	r0, [r2, #0]
    cmp	r0, #5
    beq	MA_TCP_Cut+0x6a
    add	r1, #97
    ldrb	r0, [r1, #0]
    cmp	r0, #10
    bls	MA_TCP_Cut+0x84
    ldrb	r0, [r1, #0]
    cmp	r0, #23
    bhi	MA_TCP_Cut+0x84
    ldrh	r1, [r3, #2]
    mov	r0, #1
    and	r0, r1
    cmp	r0, #0
    beq	MA_TCP_Cut+0x84
    ldrh	r1, [r3, #2]
    mov	r0, #2
    and	r0, r1
    cmp	r0, #0
    bne	MA_TCP_Cut+0x84
    mov	r1, r3
    add	r1, #97
    ldrb	r0, [r1, #0]
    cmp	r0, #31
    beq	MA_TCP_Cut+0x8c
    ldrb	r0, [r1, #0]
    cmp	r0, #30
    bne	MA_TCP_Cut+0x98
    mov	r0, #33
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_TCP_Cut+0xf2
.align 2
    .word gMA

    mov	r0, r3
    add	r0, #68
    ldrb	r0, [r0, #0]
    str	r0, [r3, #112]
    ldrh	r1, [r3, #2]
    ldr	r0, [pc, #52]
    and	r0, r1
    ldrh	r1, [r3, #2]
    strh	r0, [r3, #2]
    ldrh	r1, [r3, #2]
    mov	r0, #32
    and	r0, r1
    cmp	r0, #0
    bne	MA_TCP_Cut+0xdc
    mov	r0, r3
    add	r0, #92
    ldrb	r0, [r0, #0]
    cmp	r0, #3
    beq	MA_TCP_Cut+0xdc
    ldrh	r0, [r3, #2]
    mov	r1, #1
    orr	r0, r1
    ldrh	r1, [r3, #2]
    mov	r1, #0
    orr	r0, r1
    strh	r0, [r3, #2]
    mov	r0, #31
    mov	r1, #2
    bl	MA_TaskSet
    b	MA_TCP_Cut+0x8c
.align 2
    .word 0x0000fffb

    ldrh	r0, [r3, #2]
    mov	r1, #1
    ldrh	r2, [r3, #2]
    orr	r1, r0
    strh	r1, [r3, #2]
    mov	r0, #31
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r0}
    bx	r0
.size MA_TCP_Cut, .-MA_TCP_Cut
");
#endif

static void MATASK_TCP_Cut(void)
{
    int param = gMA.task_unk_98;

    switch (param) {
    case 0:
        gMA.task_unk_98++;

    case 1:
        if ((u32)gMA.unk_112 == 0x23 &&  // MAGIC
                gMA.recv_cmd != (MACMD_ERROR | MAPROT_REPLY)) {
            gMA.sockets[0] = gMA.buffer_unk_480.data[0];
            gMA.task_unk_98++;
        } else if (gMA.sockets_used[0] != TRUE) {
            gMA.task_unk_98 = 3;
            break;
        } else {
            gMA.task_unk_98++;
        }

    case 2:
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 3:
        gMA.unk_92 = param;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        gMA.recv_cmd = 0;
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_InitLibrary(u8 *pHardwareType)
{
    MA_InitLibraryMain(pHardwareType, TASK_UNK_01);
}


void MA_InitLibrary2(u8 *pHardwareType)
{
    MA_InitLibraryMain(pHardwareType, TASK_UNK_02);
}

void MA_InitLibraryMain(u8 *pHardwareType, int task)
{
    int zero;

    zero = 0;
    CpuSet(&zero, &gMA, DMA_SRC_FIX | DMA_32BIT_BUS | (sizeof(gMA) / 4));

    SetApiCallFlag();

    gMA.unk_88 = 0x4247414d;
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

    InitPrevBuf();
    MABIOS_Init();

    gMA.unk_101 = 0;
    gMA.unk_112 = pHardwareType;
    MA_TaskSet(task, 0);

    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_InitLibrary(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;

    case 2:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 3:
        *gMA.unk_112 = gMA.adapter_type + 0x78;  // MAGIC

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

        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_TCP_Connect(u8 *unk_1, u8 *unk_2, u16 unk_3)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_20)) {
        ResetApiCallFlag();
        return;
    }

    if (!MAU_Socket_FreeCheck()) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (MAU_Socket_GetNum() == 0) {
        MAU_memcpy(gMA.ipaddr, unk_2, 4);
    } else if (!MAU_Socket_IpAddrCheck(unk_2)) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = unk_1;
    gMA.unk_116 = (u32)unk_2;
    gMA.unk_120 = unk_3;
    MA_TaskSet(TASK_UNK_20, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_Connect(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x23:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_unk_98) {  // MAGIC
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPConnect(&gMA.buffer_unk_480, (u8 *)gMA.unk_116, gMA.unk_120);
        gMA.task_unk_98++;
        break;

    case 1:
        *gMA.unk_112 = *gMA.buffer_unk_480.data;
        MAU_Socket_Add(*gMA.buffer_unk_480.data);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        gMA.unk_92 = 3;  // MAGIC
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102,gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
    }
}

void MA_TCP_Disconnect(u8 unk_1)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_21)) {
        ResetApiCallFlag();
        return;
    }

    if (MAU_Socket_GetNum() == 0) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (!MAU_Socket_Search(unk_1)) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)(u32)unk_1;
    MA_TaskSet(TASK_UNK_21, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_Disconnect(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, (u32)gMA.unk_112);
        gMA.task_unk_98++;
        break;

    case 1:
        MAU_Socket_Delete((u32)gMA.unk_112);
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_TCP_SendRecv(u8 unk_1, int unk_2, u8 unk_3, int unk_4)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_22)) {
        ResetApiCallFlag();
        return;
    }

    if (MAU_Socket_GetNum() == 0) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    if (!MAU_Socket_Search(unk_1)) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    if (unk_3 >= 0xff) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)(u32)unk_1;
    gMA.unk_116 = unk_2;
    gMA.unk_120 = unk_3;
    gMA.unk_124 = unk_4;
    MA_TaskSet(TASK_UNK_22, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_SendRecv(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf0;
            MAU_Socket_Delete((u32)gMA.unk_112);
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_102 = 0x23;  // MAGIC
        gMA.unk_104 = 0;
        gMA.task_unk_98 = 0xf0;
        MAU_Socket_Delete((u32)gMA.unk_112);
    }

    switch (gMA.task_unk_98) {  // MAGIC
    case 0:
        MABIOS_Data(&gMA.buffer_unk_480, (u8 *)gMA.unk_116, gMA.unk_120, (u32)gMA.unk_112);
        gMA.task_unk_98++;
        break;

    case 1:
        MAU_memcpy((u8 *)gMA.unk_116, &gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        *(u8 *)gMA.unk_124 = gMA.buffer_unk_480.size - 1;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_GetHostAddress(u8 *unk_1, char *unk_2)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_23)) {
        ResetApiCallFlag();
        return;
    }

    if (MAU_strlen(unk_2) >= 0x100) {  // MAGIC
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = unk_1;
    gMA.unk_116 = (u32)unk_2;
    MA_TaskSet(TASK_UNK_23, 0);
    ResetApiCallFlag();
}

static void MATASK_GetHostAddress(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x28:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_unk_98) {  // MAGIC
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_DNSRequest(&gMA.buffer_unk_480, (char *)gMA.unk_116);
        gMA.task_unk_98++;
        break;

    case 1:
        MAU_memcpy(gMA.unk_112, gMA.buffer_unk_480.data, 4);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_GetLocalAddress(u8 *address)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_24)) {
        ResetApiCallFlag();
        return;
    }

    address[0] = gMA.local_address[0];
    address[1] = gMA.local_address[1];
    address[2] = gMA.local_address[2];
    address[3] = gMA.local_address[3];

    ResetApiCallFlag();
    gMA.condition &= ~MA_CONDITION_APIWAIT;
}

static int EEPROMSumCheck(u8 *data)
{
    static u16 tmp asm("tmp.89");
    static u16 sum asm("sum.90");
    static int i asm("i.91");

    sum = 0;
    for (i = 0; i < 0xbe; i++) {
        sum += data[i];
    }

    tmp = (data[0xbe] << 8) + data[0xbf];
    if (tmp == sum) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int EEPROMRegistrationCheck(u8 *data)
{
    if (data[0] == 'M' && data[1] == 'A') {
        return TRUE;
    } else {
        return FALSE;
    }
}

void MA_TelServer(const char *pTelNo, const char *pUserID, const char *pPassword)
{
    int len_telno, len_userid, len_password;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_03)) {
        ResetApiCallFlag();
        return;
    }

    len_telno = MAU_strlen(pTelNo);
    len_userid = MAU_strlen(pUserID);
    len_password = MAU_strlen(pPassword);
    if (len_telno > 20 || len_userid > 32 || len_password > 16 ||
            len_telno == 0 || len_userid == 0 || len_password == 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    if (!MAU_IsValidTelNoStr(pTelNo)) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)pTelNo;
    gMA.unk_116 = (u32)pUserID;
    gMA.unk_120 = (u32)pPassword;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_03, 0);

    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_TelServer(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch(gMA.unk_80) {
        case 0x11:
        case 0x13:
            break;

        case 0x12:
        case 0x17:
        case 0x18:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        case 0x19:
            gMA.unk_102 = 0x14;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;

        case 0x21:
            gMA.unk_102 = 0x22;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf9;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_unk_98++;
        break;

    case 3:
        if (gMA.buffer_unk_480.data[0] == 0xff) {
            gMA.unk_102 = 0x11;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;
        }
        if (gMA.unk_101 == 0) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
            gMA.task_unk_98++;
            break;
        }
        gMA.task_unk_98 = 6;
        break;

    case 4:
        if (!EEPROMRegistrationCheck(&gMA.unk_212[1])) {
            gMA.unk_102 = 0x25;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            return;
        }
        MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_unk_98++;
        break;

    case 5:
        MAU_memcpy(&gMA.eeprom_unk_1275[2], &gMA.buffer_unk_480.data[1], 0x40);
        if (!EEPROMSumCheck(gMA.prevbuf)) {
            gMA.unk_102 = 0x14;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            return;
        }
        gMA.unk_101 = 1;
        gMA.task_unk_98++;
        break;

    case 6:
        MAU_memcpy(gMA.unk_828, &gMA.prevbuf[4], 8);
        MAU_memcpy(gMA.smtp_server, gMA.eeprom_unk_1223, 0x2c);
        gMA.task_unk_98++;
        break;

    case 7:
        MABIOS_Tel(MA_GetCallTypeFromHarwareType(gMA.adapter_type), gMA.unk_112);
        gMA.task_unk_98++;
        break;

    case 8:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_PPPConnect(&gMA.buffer_unk_480, (char *)gMA.unk_116, (char *)gMA.unk_120, gMA.unk_828, gMA.unk_832);
        gMA.task_unk_98++;
        break;

    case 9:
        gMA.local_address[0] = gMA.buffer_unk_480.data[0];
        gMA.local_address[1] = gMA.buffer_unk_480.data[1];
        gMA.local_address[2] = gMA.buffer_unk_480.data[2];
        gMA.local_address[3] = gMA.buffer_unk_480.data[3];
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf9:
        MABIOS_Offline();
        gMA.task_unk_98++;
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_Tel(const char *pTelNo)
{
    int len;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_04)) {
        ResetApiCallFlag();
        return;
    }

    len = MAU_strlen(pTelNo);
    if (len > 20 || len == 0) {  // MAGIC
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)pTelNo;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_04, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_Tel(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x12:
        case 0x17:
        case 0x18:
        case 0x19:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_unk_98++;
        break;

    case 3:
        if (gMA.buffer_unk_480.data[0] == 0xff) {
            gMA.unk_102 = 0x11;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;
        }

        MABIOS_Tel(MA_GetCallTypeFromHarwareType(gMA.adapter_type), gMA.unk_112);
        gMA.task_unk_98++;
        break;

    case 4:
        gMA.status |= STATUS_UNK_9;
        gMA.unk_92 = 7;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_P2P_SEND << MA_CONDITION_SHIFT;
        gMA.unk_112 = NULL;
        gMA.buffer_unk_480.size = 0;
        InitPrevBuf();
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_94);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_Receive(void)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_05)) {
        ResetApiCallFlag();
        return;
    }

    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_05, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_Receive(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x14:
            if (gMA.unk_81 != 0) {
                MA_DefaultNegaResProc();
                gMA.task_unk_98 = 0xfa;
            }
            break;

        case 0x18:
        case 0x19:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        MABIOS_WaitCall();
        gMA.task_unk_98++;
        break;

    case 3:
        if (gMA.recv_cmd != (MACMD_WAITCALL | MAPROT_REPLY)) {
            gMA.task_unk_98--;
            break;
        }
        gMA.task_unk_98++;
        break;

    case 4:
        gMA.status |= STATUS_UNK_9;
        gMA.unk_92 = 8;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_P2P_RECV << MA_CONDITION_SHIFT;
        gMA.unk_112 = NULL;
        gMA.buffer_unk_480.size = 0;
        InitPrevBuf();
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_94);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_SData(const u8 *pSendData, u8 sendSize, u8 *pResult)
{
    SetApiCallFlag();
    *pResult = FALSE;
    if (!MA_ApiPreExe(TASK_UNK_06)) {
        ResetApiCallFlag();
        return;
    }

    if ((s8)(sendSize - 1) < 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    if (gMA.condition & MA_CONDITION_PTP_GET) {
        *pResult = TRUE;
        gMA.condition &= ~MA_CONDITION_APIWAIT;
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)pSendData;
    gMA.unk_116 = sendSize;
    gMA.status |= STATUS_UNK_13;
    ResetApiCallFlag();
}

void MA_GData(u8 *pRecvData, u8 *pRecvSize)
{
    int i;
    u8 size;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_07)) {
        ResetApiCallFlag();
        return;
    }

    if (!(gMA.condition & MA_CONDITION_PTP_GET)) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    MAU_memcpy(pRecvData, &gMA.prevbuf[1], gMA.prevbuf[0]);

    *pRecvSize = gMA.prevbuf[0];
    size = gMA.prevbuf[0] + 1;
    for (i = 0; i < gMA.prevbuf_size - size; i++) {
        gMA.prevbuf[i] = gMA.prevbuf[size + i];
    }

    gMA.prevbuf_size -= size;
    if (gMA.prevbuf_size != 0) {
        if ((s8)(gMA.prevbuf[0] - 1) < 0) {
            gMA.prevbuf[0] = 0x80;  // MAGIC
        }

        if (gMA.prevbuf_size >= gMA.prevbuf[0] + 1) {
            gMA.condition |= MA_CONDITION_PTP_GET;
        } else {
            gMA.condition &= ~MA_CONDITION_PTP_GET;
        }
    } else {
        gMA.condition &= ~MA_CONDITION_PTP_GET;
    }
    gMA.condition &= ~MA_CONDITION_APIWAIT;
    ResetApiCallFlag();
}

static void MATASK_P2P(void)
{
    if (gMA.status & STATUS_UNK_10) {
        gMA.status &= ~STATUS_UNK_10;
        gMA.status &= ~STATUS_UNK_13;
        gMA.condition &= ~MA_CONDITION_APIWAIT;
    }

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
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

            MA_SetApiError(MAAPIE_OFFLINE, 0);
            MA_TaskSet(TASK_UNK_00, 0);
            return;
        }
    }

    if (gMA.buffer_unk_480.size <= 1) return;

    ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);

    gMA.buffer_unk_480.data = NULL;
    gMA.buffer_unk_480.size = 0;
    if ((s8)(gMA.prevbuf[0] - 1) < 0) {
        gMA.prevbuf[0] = 0x80;  // MAGIC
    }

    if (gMA.prevbuf_size >= gMA.prevbuf[0] + 1) {
        gMA.condition |= MA_CONDITION_PTP_GET;
    }
}

void MA_Condition(u8 *pCondition)
{
    MA_ConditionMain(pCondition, TASK_UNK_08);
}

void MA_Condition2(u8 *pCondition)
{
    MA_ConditionMain(pCondition, TASK_UNK_09);
}

void MA_ConditionMain(u8 *pCondition, int task)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(task)) {
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = pCondition;
    if (!gMA.unk_92) {
        gMA.unk_116 = 1;
        *(vu32 *)REG_TM3CNT = 0;
        MA_TaskSet(task, 0);
    } else {
        gMA.unk_116 = 0;
        MA_TaskSet(task, 1);
    }

    ResetApiCallFlag();
    if (gMA.unk_92) return;

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_Condition(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x17:
            MA_DefaultNegaResProc();
            if (gMA.unk_116 == 1) {
                gMA.task_unk_98 = 0xfa;
            } else {
                gMA.task_unk_98 = 0xfc;
            }
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_unk_98++;
        break;

    case 2:
        *gMA.unk_112 = MA_ProcessCheckStatusResponse(gMA.buffer_unk_480.data[0]);
        if (0xef < gMA.unk_84[2]) {  // MAGIC
            *gMA.unk_112 |= 0x80;  // MAGIC
        }
        if (gMA.unk_116 == 1) {
            gMA.task_unk_98 = 3;
        } else {
            MA_TaskSet(TASK_UNK_00, 0);
        }
        break;

    case 3:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 4:
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

        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfc:
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_Offline(void)
{
    SetApiCallFlag();
    if (!gMA.unk_92 || !MA_ApiPreExe(TASK_UNK_0A)) {
        ResetApiCallFlag();
        return;
    }

    if (gMA.unk_92 == 7 || gMA.unk_92 == 8) {
        MA_TaskSet(TASK_UNK_0A, 1);
    } else if (gMA.unk_92 == 4 || gMA.unk_92 == 5) {
        MA_TaskSet(TASK_UNK_0A, 100);
    } else {
        MA_TaskSet(TASK_UNK_0A, 0);
    }
    ResetApiCallFlag();
}

static void MATASK_Offline(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.task_unk_98 = 0;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        if (gMA.unk_92 != 7 && gMA.unk_92 != 8) {
            MABIOS_PPPDisconnect();
        }
        gMA.task_unk_98++;
        break;

    case 1:
        MABIOS_Offline();
        gMA.task_unk_98++;
        break;

    case 2:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 3:
        gMA.condition &= ~MA_CONDITION_PTP_GET;
        gMA.status &= ~STATUS_UNK_9;

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

        MA_TaskSet(TASK_UNK_00, 0);
        MA_ChangeSIOMode(MA_SIO_BYTE);
        break;

    case 100:
        InitPrevBuf();
        MAU_strcpy(gMA.unk_880, POP3_Quit);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 101:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }
        gMA.unk_112 = (u8 *)gMA.counter_timeout200msec[gMA.sio_mode];
        gMA.task_unk_98++;
        break;

    case 102:
        if (--gMA.unk_112 != 0) break;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 103:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        gMA.task_unk_98 = 0;
        break;
    }
}

static int CheckSMTPResponse(char *response)
{
    if ((response[0] >= '0' && response[0] <= '9') &&
            (response[1] >= '0' && response[1] <= '9') &&
            (response[2] >= '0' && response[2] <= '9')) {
        return FALSE;
    } else {
        return TRUE;
    }
}

void MA_SMTP_Connect(const char *pMailAddress)
{
    int len;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_0B)) {
        ResetApiCallFlag();
        return;
    }

    len = MAU_strlen(pMailAddress);
    if (len > 30 || len == 0) {  // MAGIC
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    MA_GetSMTPServerName(gMA.unk_880);
    gMA.unk_112 = (u8 *)pMailAddress;
    MA_TaskSet(TASK_UNK_0B, 0);
    ResetApiCallFlag();
}

#if 0
#else
asm("
.lcomm cp1.149, 0x4
.lcomm cp2.150, 0x4
.lcomm smtpRes.151, 0x4

.align 2
.thumb_func
MATASK_SMTP_Connect:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r2, [pc, #32]
    mov	r0, r2
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_SMTP_Connect+0x82
    mov	r0, r2
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #35
    beq	MATASK_SMTP_Connect+0x52
    cmp	r0, #35
    bgt	MATASK_SMTP_Connect+0x2c
    cmp	r0, #21
    beq	MATASK_SMTP_Connect+0x36
    b	MATASK_SMTP_Connect+0x74
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_SMTP_Connect+0x82
    cmp	r0, #40
    beq	MATASK_SMTP_Connect+0x52
    b	MATASK_SMTP_Connect+0x74
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_SMTP_Connect+0x82
    ldr	r2, [pc, #28]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #241
    strb	r0, [r1, #0]
    b	MATASK_SMTP_Connect+0x82
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #84]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #241
    strb	r1, [r0, #0]
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    lsl	r1, r1, #16
    lsr	r6, r1, #16
    cmp	r6, #0
    beq	MATASK_SMTP_Connect+0xd4
    ldr	r3, [pc, #60]
    mov	r1, r3
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r3, #2]
    mov	r2, #0
    strh	r0, [r3, #2]
    ldrh	r0, [r3, #2]
    mov	r4, #128
    lsl	r4, r4, #1
    mov	r1, r4
    orr	r0, r1
    ldrh	r1, [r3, #2]
    orr	r0, r2
    strh	r0, [r3, #2]
    mov	r0, r3
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #48
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_SMTP_Connect+0x3d8
.align 2
    .word gMA

    ldr	r5, [pc, #28]
    mov	r7, r5
    add	r7, #98
    ldrb	r0, [r7, #0]
    cmp	r0, #3
    beq	MATASK_SMTP_Connect+0x18e
    cmp	r0, #3
    bgt	MATASK_SMTP_Connect+0xf8
    cmp	r0, #1
    beq	MATASK_SMTP_Connect+0x12c
    cmp	r0, #1
    bgt	MATASK_SMTP_Connect+0x158
    cmp	r0, #0
    beq	MATASK_SMTP_Connect+0x112
    b	MATASK_SMTP_Connect+0x446
.align 2
    .word gMA

    cmp	r0, #5
    bne	MATASK_SMTP_Connect+0xfe
    b	MATASK_SMTP_Connect+0x3b6
    cmp	r0, #5
    bge	MATASK_SMTP_Connect+0x104
    b	MATASK_SMTP_Connect+0x2e6
    cmp	r0, #240
    bne	MATASK_SMTP_Connect+0x10a
    b	MATASK_SMTP_Connect+0x3e2
    cmp	r0, #241
    bne	MATASK_SMTP_Connect+0x110
    b	MATASK_SMTP_Connect+0x402
    b	MATASK_SMTP_Connect+0x446
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    strh	r6, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r2, #220
    lsl	r2, r2, #2
    add	r1, r5, r2
    bl	MABIOS_DNSRequest
    b	MATASK_SMTP_Connect+0x3f8
    mov	r4, r5
    add	r4, #106
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r1, [r0, #0]
    mov	r0, r4
    mov	r2, #4
    bl	MAU_memcpy
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r5, r2
    strh	r6, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, r4
    mov	r2, #25
    bl	MABIOS_TCPConnect
    b	MATASK_SMTP_Connect+0x3f8
    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r5, r4
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    mov	r4, r5
    add	r4, #99
    strb	r0, [r4, #0]
    mov	r1, r5
    add	r1, #204
    mov	r0, #1
    strb	r0, [r1, #0]
    bl	InitPrevBuf
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    strh	r6, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldrb	r3, [r4, #0]
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_SMTP_Connect+0x3f8
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r4, #240
    lsl	r4, r4, #1
    add	r4, r4, r5
    mov	r8, r4
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r0, [pc, #228]
    add	r4, r5, r0
    ldr	r1, [pc, #228]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_Connect+0x1c2
    b	MATASK_SMTP_Connect+0x318
    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #208]
    str	r0, [r1, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Connect+0x1d2
    b	MATASK_SMTP_Connect+0x3a0
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #192]
    add	r0, r5, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #180]
    add	r0, r5, r1
    ldr	r4, [pc, #180]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r1, r5
    add	r1, #94
    ldrh	r0, [r1, #0]
    strh	r2, [r1, #0]
    ldrh	r0, [r1, #0]
    cmp	r0, #220
    bne	MATASK_SMTP_Connect+0x2d4
    mov	r1, #220
    lsl	r1, r1, #2
    add	r0, r5, r1
    ldr	r1, [pc, #156]
    bl	MAU_strcpy
    ldr	r2, [pc, #156]
    ldr	r4, [pc, #156]
    add	r0, r5, r4
    str	r0, [r2, #0]
    ldr	r1, [pc, #156]
    ldr	r0, [r5, #112]
    str	r0, [r1, #0]
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Connect+0x248
    cmp	r0, #64
    beq	MATASK_SMTP_Connect+0x248
    mov	r4, r2
    mov	r3, r1
    ldr	r2, [r4, #0]
    ldr	r1, [r3, #0]
    ldrb	r0, [r1, #0]
    strb	r0, [r2, #0]
    add	r1, #1
    str	r1, [r3, #0]
    add	r2, #1
    str	r2, [r4, #0]
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Connect+0x248
    cmp	r0, #64
    bne	MATASK_SMTP_Connect+0x22e
    ldr	r0, [pc, #100]
    ldr	r1, [r0, #0]
    mov	r0, #0
    strb	r0, [r1, #0]
    ldr	r4, [pc, #104]
    ldr	r1, [pc, #108]
    mov	r0, r4
    bl	MAU_strcat
    bl	InitPrevBuf
    ldr	r0, [pc, #100]
    add	r5, r4, r0
    mov	r0, #0
    strh	r0, [r5, #0]
    ldr	r1, [pc, #96]
    add	r0, r4, r1
    str	r0, [r5, #4]
    mov	r0, r4
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    ldr	r1, [pc, #80]
    add	r0, r4, r1
    ldrb	r3, [r0, #0]
    mov	r0, r5
    mov	r1, r4
    bl	MABIOS_Data
    ldr	r2, [pc, #72]
    add	r4, r4, r2
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_SMTP_Connect+0x446
.align 2
    .word 0x0000047d
    .word 0x000006fa
    .word smtpRes.151
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0
    .word strEndMultiLine.25+0x10
    .word cp1.149
    .word 0x00000375
    .word cp2.150
    .word gMA+0x370
    .word strEndMultiLine.25+0x18
    .word 0xfffffe70
    .word 0xfffffd64
    .word 0xfffffcf3
    .word 0xfffffcf2

    mov	r2, r5
    add	r2, #102
    mov	r0, #48
    strb	r0, [r2, #0]
    ldrh	r1, [r1, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    b	MATASK_SMTP_Connect+0x3ae
    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r5, r4
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r1, r1, r5
    mov	r8, r1
    ldrh	r1, [r1, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #44]
    add	r4, r5, r2
    ldr	r1, [pc, #44]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_Connect+0x33c
    mov	r2, r8
    strh	r6, [r2, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r2, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r8
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_SMTP_Connect+0x446
.align 2
    .word 0x0000047d
    .word 0x000006fa

    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #76]
    str	r0, [r1, #0]
    cmp	r0, #0
    bne	MATASK_SMTP_Connect+0x3a0
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #60]
    add	r0, r5, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #48]
    add	r0, r5, r1
    ldr	r4, [pc, #48]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r1, r5
    add	r1, #94
    ldrh	r0, [r1, #0]
    strh	r2, [r1, #0]
    ldrh	r0, [r1, #0]
    cmp	r0, #250
    beq	MATASK_SMTP_Connect+0x3f8
    mov	r2, r5
    add	r2, #102
    mov	r0, #48
    strb	r0, [r2, #0]
    ldrh	r1, [r1, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    b	MATASK_SMTP_Connect+0x3ae
.align 2
    .word smtpRes.151
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0

    mov	r1, r5
    add	r1, #102
    mov	r0, #48
    strb	r0, [r1, #0]
    mov	r0, r5
    add	r0, #104
    strh	r6, [r0, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #240
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Connect+0x446
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #4
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r2, #128
    lsl	r2, r2, #3
    mov	r0, r2
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_SMTP_Connect+0x446
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r5, r4
    strh	r6, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r7, #0]
    add	r0, #1
    ldrb	r1, [r7, #0]
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Connect+0x446
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    mov	r4, #0
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r1, r2
    orr	r0, r1
    ldrh	r1, [r5, #2]
    orr	r0, r4
    strh	r0, [r5, #2]
    mov	r0, r5
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r5
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r5
    add	r0, #204
    strb	r4, [r0, #0]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MATASK_SMTP_Connect, .-MATASK_SMTP_Connect
");
#endif

void MA_SMTP_Sender(const char * const pRecipients[])
{
    int i;
    int len;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_0C)) {
        ResetApiCallFlag();
        return;
    }

    if (pRecipients[0] == NULL || pRecipients[1] == NULL) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    len = MAU_strlen(pRecipients[0]);
    if (len > 30 || len == 0) {  // MAGIC
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    for (i = 1; pRecipients[i] != NULL; i++) {
        len = MAU_strlen(pRecipients[i]);
        if (len == 0 || len >= 0x80 || i > 0x100) {  // MAGIC
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }
    }

    gMA.unk_112 = (u8 *)pRecipients;
    MA_TaskSet(TASK_UNK_0C, 0);
    ResetApiCallFlag();
}

static void MATASK_SMTP_Sender(void)
{
    static int smtpRes asm("smtpRes.158");

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_SetApiError(MAAPIE_SMTP, 0);
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        MAU_strcpy(gMA.unk_880, POP3_From);
        MAU_strcat(gMA.unk_880, *(char **)gMA.unk_112);
        MAU_strcat(gMA.unk_880, POP3_From_Newl);
        gMA.unk_112 += 4;
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (smtpRes == FALSE) {
            gMA.unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.unk_94 == 250) {
                gMA.task_unk_98++;
                break;
            }

            gMA.unk_102 = 0x30;
            gMA.unk_104 = gMA.unk_94;
            gMA.task_unk_98 = 0xf0;
            break;
        }

        gMA.unk_102 = 0x30;
        gMA.unk_104 = 0;
        gMA.task_unk_98 = 0xf0;
        break;

    case 2:
        if (*(u32 *)gMA.unk_112 != 0) {
            MAU_strcpy(gMA.unk_880, POP3_Rcpt);
            MAU_strcat(gMA.unk_880, *(char **)gMA.unk_112);
            MAU_strcat(gMA.unk_880, POP3_From_Newl);
            gMA.unk_112 += 4;
            InitPrevBuf();
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
            gMA.task_unk_98++;
            break;
        }
        gMA.task_unk_98 = 4;
        break;

    case 3:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (smtpRes == FALSE) {
            gMA.unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.unk_94 == 250) {
                gMA.task_unk_98 = 2;
                break;
            }

            gMA.unk_102 = 0x30;
            gMA.unk_104 = gMA.unk_94;
            gMA.task_unk_98 = 0xf0;
            break;
        }

        gMA.unk_102 = 0x30;
        gMA.unk_104 = 0;
        gMA.task_unk_98 = 0xf0;
        break;

    case 4:
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.unk_96 = 1;
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

void MA_SMTP_Send(const char *pSendData, u16 sendSize, int endFlag)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_0D)) {
        ResetApiCallFlag();
        return;
    }

    if (sendSize == 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    if (gMA.unk_96 == 1) gMA.unk_124 = 0;
    gMA.unk_112 = (u8 *)pSendData;
    gMA.unk_116 = sendSize;
    gMA.unk_120 = endFlag;
    gMA.unk_124 += sendSize;
    MA_TaskSet(TASK_UNK_0D, 0);
    ResetApiCallFlag();
}

#if 0
#else
asm("
.lcomm smtpRes.165, 0x4

.align 2
.thumb_func
MATASK_SMTP_Send:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_SMTP_Send+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_SMTP_Send+0x3c
    mov	r2, r4
    add	r2, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r2, #0]
    mov	r0, r4
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_SMTP_Send+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_SMTP_Send+0x4e
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #241
    strb	r1, [r0, #0]
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    mov	r8, r1
    cmp	r1, #0
    beq	MATASK_SMTP_Send+0xa0
    ldr	r3, [pc, #56]
    mov	r1, r3
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r3, #2]
    mov	r2, #0
    strh	r0, [r3, #2]
    ldrh	r0, [r3, #2]
    mov	r4, #128
    lsl	r4, r4, #1
    mov	r1, r4
    orr	r0, r1
    ldrh	r1, [r3, #2]
    orr	r0, r2
    strh	r0, [r3, #2]
    mov	r0, r3
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #48
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_SMTP_Send+0x248
.align 2
    .word gMA

    ldr	r6, [pc, #32]
    mov	r7, r6
    add	r7, #98
    ldrb	r0, [r7, #0]
    mov	r4, r6
    cmp	r0, #50
    bne	MATASK_SMTP_Send+0xb0
    b	MATASK_SMTP_Send+0x252
    cmp	r0, #50
    bgt	MATASK_SMTP_Send+0xd4
    cmp	r0, #1
    beq	MATASK_SMTP_Send+0x11a
    cmp	r0, #1
    bgt	MATASK_SMTP_Send+0xc8
    cmp	r0, #0
    beq	MATASK_SMTP_Send+0xfa
    b	MATASK_SMTP_Send+0x42a
.align 2
    .word gMA

    cmp	r0, #2
    beq	MATASK_SMTP_Send+0x1a6
    cmp	r0, #3
    bne	MATASK_SMTP_Send+0xd2
    b	MATASK_SMTP_Send+0x248
    b	MATASK_SMTP_Send+0x42a
    cmp	r0, #101
    bne	MATASK_SMTP_Send+0xda
    b	MATASK_SMTP_Send+0x2e8
    cmp	r0, #101
    bgt	MATASK_SMTP_Send+0xec
    cmp	r0, #51
    bne	MATASK_SMTP_Send+0xe4
    b	MATASK_SMTP_Send+0x26c
    cmp	r0, #100
    bne	MATASK_SMTP_Send+0xea
    b	MATASK_SMTP_Send+0x28e
    b	MATASK_SMTP_Send+0x42a
    cmp	r0, #240
    bne	MATASK_SMTP_Send+0xf2
    b	MATASK_SMTP_Send+0x3c4
    cmp	r0, #241
    bne	MATASK_SMTP_Send+0xf8
    b	MATASK_SMTP_Send+0x3e6
    b	MATASK_SMTP_Send+0x42a
    mov	r1, r6
    add	r1, #96
    ldrb	r0, [r1, #0]
    cmp	r0, #1
    bne	MATASK_SMTP_Send+0x112
    ldrb	r0, [r1, #0]
    mov	r0, r8
    strb	r0, [r1, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #100
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Send+0x42a
    ldrb	r0, [r7, #0]
    add	r0, #1
    ldrb	r1, [r7, #0]
    strb	r0, [r7, #0]
    ldr	r0, [r4, #116]
    cmp	r0, #254
    bls	MATASK_SMTP_Send+0x158
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r4, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r4, #112]
    mov	r2, r4
    add	r2, #99
    ldrb	r3, [r2, #0]
    mov	r2, #254
    bl	MABIOS_Data
    ldr	r0, [r4, #112]
    add	r0, #254
    str	r0, [r4, #112]
    ldr	r0, [r4, #116]
    sub	r0, #254
    str	r0, [r4, #116]
    mov	r2, r4
    add	r2, #132
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    str	r0, [r2, #0]
    b	MATASK_SMTP_Send+0x19e
    bl	InitPrevBuf
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r4, r2
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r4, #112]
    ldr	r2, [r4, #116]
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r3, r4
    add	r3, #99
    ldrb	r3, [r3, #0]
    bl	MABIOS_Data
    ldr	r0, [r4, #120]
    cmp	r0, #1
    bne	MATASK_SMTP_Send+0x194
    mov	r2, r4
    add	r2, #132
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    add	r0, #1
    str	r0, [r2, #0]
    b	MATASK_SMTP_Send+0x19e
    mov	r1, r4
    add	r1, #132
    mov	r0, #3
    str	r0, [r1, #0]
    sub	r1, #34
    ldrb	r0, [r1, #0]
    mov	r0, #50
    strb	r0, [r1, #0]
    b	MATASK_SMTP_Send+0x42a
    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r6, r4
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r5, r6, r1
    ldrh	r1, [r5, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #108]
    add	r4, r6, r2
    ldr	r1, [pc, #108]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_Send+0x1d8
    b	MATASK_SMTP_Send+0x318
    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #88]
    str	r0, [r1, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Send+0x1e8
    b	MATASK_SMTP_Send+0x3ac
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #72]
    add	r0, r6, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #60]
    add	r0, r6, r1
    ldr	r4, [pc, #60]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r1, r6
    add	r1, #94
    ldrh	r0, [r1, #0]
    strh	r2, [r1, #0]
    ldrh	r0, [r1, #0]
    cmp	r0, #250
    bne	MATASK_SMTP_Send+0x21e
    b	MATASK_SMTP_Send+0x3dc
    mov	r2, r6
    add	r2, #102
    mov	r0, #48
    strb	r0, [r2, #0]
    ldrh	r1, [r1, #0]
    mov	r0, r6
    add	r0, #104
    b	MATASK_SMTP_Send+0x3ba
.align 2
    .word 0x0000047d
    .word 0x000006fa
    .word smtpRes.165
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0

    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_SMTP_Send+0x42a
    mov	r2, r6
    add	r2, #128
    ldrb	r0, [r6, #5]
    lsl	r0, r0, #2
    mov	r1, r6
    add	r1, #52
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    str	r0, [r2, #0]
    ldrb	r0, [r7, #0]
    add	r0, #1
    ldrb	r1, [r7, #0]
    strb	r0, [r7, #0]
    mov	r0, r4
    add	r0, #128
    ldr	r1, [r0, #0]
    sub	r1, #1
    str	r1, [r0, #0]
    mov	r0, #1
    neg	r0, r0
    cmp	r1, r0
    beq	MATASK_SMTP_Send+0x280
    b	MATASK_SMTP_Send+0x42a
    mov	r0, r4
    add	r0, #132
    ldr	r1, [r0, #0]
    sub	r0, #34
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    b	MATASK_SMTP_Send+0x42a
    mov	r2, #220
    lsl	r2, r2, #2
    add	r5, r6, r2
    ldr	r1, [pc, #76]
    mov	r0, r5
    bl	MAU_strcpy
    bl	InitPrevBuf
    mov	r0, #240
    lsl	r0, r0, #1
    add	r4, r6, r0
    mov	r1, r8
    strh	r1, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    mov	r0, r5
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r6
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, r5
    bl	MABIOS_Data
    ldrb	r0, [r7, #0]
    add	r0, #1
    ldrb	r1, [r7, #0]
    strb	r0, [r7, #0]
    mov	r1, r6
    add	r1, #132
    ldrb	r0, [r7, #0]
    str	r0, [r1, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #50
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Send+0x42a
.align 2
    .word strEndMultiLine.25+0x38

    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r6, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r4, #240
    lsl	r4, r4, #1
    add	r5, r6, r4
    ldrh	r1, [r5, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r0, [pc, #44]
    add	r4, r6, r0
    ldr	r1, [pc, #44]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_Send+0x33c
    mov	r2, r8
    strh	r2, [r5, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r5, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r5
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_SMTP_Send+0x42a
.align 2
    .word 0x0000047d
    .word 0x000006fa

    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #72]
    str	r0, [r1, #0]
    cmp	r0, #0
    bne	MATASK_SMTP_Send+0x3ac
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #56]
    add	r0, r6, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #44]
    add	r0, r6, r1
    ldr	r4, [pc, #44]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r3, r6
    add	r3, #94
    ldrh	r0, [r3, #0]
    strh	r2, [r3, #0]
    ldrh	r1, [r3, #0]
    mov	r0, #177
    lsl	r0, r0, #1
    cmp	r1, r0
    bne	MATASK_SMTP_Send+0x39c
    ldrb	r0, [r7, #0]
    mov	r0, #1
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Send+0x42a
.align 2
    .word smtpRes.165
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0

    mov	r1, r6
    add	r1, #102
    mov	r0, #48
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #0]
    mov	r0, r6
    add	r0, #104
    b	MATASK_SMTP_Send+0x3ba
    mov	r1, r6
    add	r1, #102
    mov	r0, #48
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    mov	r1, r8
    strh	r1, [r0, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #240
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Send+0x42a
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r6, r2
    mov	r4, r8
    strh	r4, [r0, #0]
    mov	r1, r6
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r7, #0]
    add	r0, #1
    ldrb	r1, [r7, #0]
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Send+0x42a
    mov	r1, r6
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r6, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r6, #2]
    mov	r4, #0
    strh	r0, [r6, #2]
    ldrh	r0, [r6, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r1, r2
    orr	r0, r1
    ldrh	r1, [r6, #2]
    orr	r0, r4
    strh	r0, [r6, #2]
    mov	r0, r6
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r6
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r6
    add	r0, #204
    strb	r4, [r0, #0]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MATASK_SMTP_Send, .-MATASK_SMTP_Send
");
#endif

void MA_POP3_Quit(void)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_15)) {
        ResetApiCallFlag();
        return;
    }

    MA_TaskSet(TASK_UNK_15, 0);
    ResetApiCallFlag();
}

void MA_SMTP_Quit(void)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_0E)) {
        ResetApiCallFlag();
        return;
    }

    MA_TaskSet(TASK_UNK_0E, 0);
    ResetApiCallFlag();
}

static void MATASK_SMTP_POP3_Quit(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        if (gMA.unk_92 == 4) {
            MA_SetApiError(MAAPIE_SMTP, 0);
        } else {
            MA_SetApiError(MAAPIE_POP3, 0);
        }
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        InitPrevBuf();
        MAU_strcpy(gMA.unk_880, POP3_Quit);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        gMA.unk_112 = (u8 *)gMA.counter_timeout200msec[gMA.sio_mode];
        gMA.task_unk_98++;
        break;

    case 2:
        gMA.unk_112--;
        if (gMA.unk_112 != 0) break;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 3:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

static int CheckPOP3Response(char *response)
{
    if (response[0] == '+' &&
            response[1] == 'O' &&
            response[2] == 'K') {
        return 0;
    }

    if (response[0] == '-' &&
            response[1] == 'E' &&
            response[2] == 'R' &&
            response[3] == 'R') {
        return 1;
    }

    return 2;
}

void MA_POP3_Connect(const char *pUserID, const char *pPassword)
{
    int len_userid, len_password;
    char *end;

    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_0F)) {
        ResetApiCallFlag();
        return;
    }

    len_userid = MAU_strlen(pUserID);
    if (len_userid > 30 || len_userid == 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    len_password = MAU_strlen(pPassword);
    if (len_password > 16 || len_password == 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = gMA.unk_880;
    MA_GetPOP3ServerName(gMA.unk_112);

    gMA.unk_116 = (u32)(gMA.unk_112 + MAU_strlen(gMA.unk_112) + 1);
    MAU_strcpy((char *)gMA.unk_116, POP3_User);
    MAU_strcat((char *)gMA.unk_116, pUserID);

    end = (char *)gMA.unk_116;
    while (*end != '\0' && *end != '@') end++;
    *end++ = '\r';
    *end++ = '\n';
    *end++ = '\0';

    gMA.unk_120 = (u32)end;
    MAU_strcpy((char *)gMA.unk_120, POP3_Pass);
    MAU_strcat((char *)gMA.unk_120, pPassword);
    MAU_strcat((char *)gMA.unk_120, POP3_Newl);

    MA_TaskSet(TASK_UNK_0F, 0);
    ResetApiCallFlag();
}

#if 0
#else
asm("
.lcomm pop3res.184, 0x4

.align 2
.thumb_func
MATASK_POP3_Connect:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r2, [pc, #32]
    mov	r0, r2
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_POP3_Connect+0x82
    mov	r0, r2
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #35
    beq	MATASK_POP3_Connect+0x52
    cmp	r0, #35
    bgt	MATASK_POP3_Connect+0x2c
    cmp	r0, #21
    beq	MATASK_POP3_Connect+0x36
    b	MATASK_POP3_Connect+0x74
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_Connect+0x82
    cmp	r0, #40
    beq	MATASK_POP3_Connect+0x52
    b	MATASK_POP3_Connect+0x74
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_POP3_Connect+0x82
    ldr	r2, [pc, #28]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #241
    strb	r0, [r1, #0]
    b	MATASK_POP3_Connect+0x82
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #84]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #241
    strb	r1, [r0, #0]
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    mov	r8, r1
    cmp	r1, #0
    beq	MATASK_POP3_Connect+0xd4
    ldr	r3, [pc, #56]
    mov	r1, r3
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r3, #2]
    mov	r2, #0
    strh	r0, [r3, #2]
    ldrh	r0, [r3, #2]
    mov	r4, #128
    lsl	r4, r4, #1
    mov	r1, r4
    orr	r0, r1
    ldrh	r1, [r3, #2]
    orr	r0, r2
    strh	r0, [r3, #2]
    mov	r0, r3
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_POP3_Connect+0x3b2
.align 2
    .word gMA

    ldr	r5, [pc, #28]
    mov	r6, r5
    add	r6, #98
    ldrb	r0, [r6, #0]
    cmp	r0, #4
    bne	MATASK_POP3_Connect+0xe2
    b	MATASK_POP3_Connect+0x24c
    cmp	r0, #4
    bgt	MATASK_POP3_Connect+0x102
    cmp	r0, #1
    beq	MATASK_POP3_Connect+0x134
    cmp	r0, #1
    bgt	MATASK_POP3_Connect+0xf8
    cmp	r0, #0
    beq	MATASK_POP3_Connect+0x11c
    b	MATASK_POP3_Connect+0x422
.align 2
    .word gMA

    cmp	r0, #2
    beq	MATASK_POP3_Connect+0x162
    cmp	r0, #3
    beq	MATASK_POP3_Connect+0x19a
    b	MATASK_POP3_Connect+0x422
    cmp	r0, #6
    bne	MATASK_POP3_Connect+0x108
    b	MATASK_POP3_Connect+0x390
    cmp	r0, #6
    bge	MATASK_POP3_Connect+0x10e
    b	MATASK_POP3_Connect+0x2fe
    cmp	r0, #240
    bne	MATASK_POP3_Connect+0x114
    b	MATASK_POP3_Connect+0x3bc
    cmp	r0, #241
    bne	MATASK_POP3_Connect+0x11a
    b	MATASK_POP3_Connect+0x3de
    b	MATASK_POP3_Connect+0x422
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r2, r8
    strh	r2, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r5, #112]
    bl	MABIOS_DNSRequest
    b	MATASK_POP3_Connect+0x3d4
    mov	r4, r5
    add	r4, #106
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r1, [r0, #0]
    mov	r0, r4
    mov	r2, #4
    bl	MAU_memcpy
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r5, r2
    mov	r1, r8
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, r4
    mov	r2, #110
    bl	MABIOS_TCPConnect
    b	MATASK_POP3_Connect+0x3d4
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    mov	r4, r5
    add	r4, #99
    strb	r0, [r4, #0]
    mov	r1, r5
    add	r1, #204
    mov	r0, #1
    strb	r0, [r1, #0]
    bl	InitPrevBuf
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r2, r8
    strh	r2, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldrb	r3, [r4, #0]
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_Connect+0x3d4
    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r5, r4
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r7, r5, r1
    ldrh	r1, [r7, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #36]
    add	r4, r5, r2
    ldr	r1, [pc, #36]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_Connect+0x1e4
    mov	r2, r8
    strh	r2, [r7, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r7, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r7
    b	MATASK_POP3_Connect+0x33e
.align 2
    .word 0x0000047d
    .word 0x000006fa

    mov	r0, r4
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #48]
    str	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_POP3_Connect+0x224
    bl	InitPrevBuf
    mov	r4, r8
    strh	r4, [r7, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r7, #4]
    ldr	r4, [r5, #116]
    mov	r0, r4
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r5
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r7
    mov	r1, r4
    bl	MABIOS_Data
    b	MATASK_POP3_Connect+0x3d4
.align 2
    .word pop3res.184

    cmp	r1, #1
    bne	MATASK_POP3_Connect+0x23a
    mov	r1, r5
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r5
    add	r0, #104
    mov	r1, r8
    strh	r1, [r0, #0]
    b	MATASK_POP3_Connect+0x388
    mov	r2, r5
    add	r2, #102
    mov	r1, #0
    mov	r0, #49
    strb	r0, [r2, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    b	MATASK_POP3_Connect+0x388
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r4, #240
    lsl	r4, r4, #1
    add	r7, r5, r4
    ldrh	r1, [r7, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r0, [pc, #36]
    add	r4, r5, r0
    ldr	r1, [pc, #36]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_Connect+0x298
    mov	r2, r8
    strh	r2, [r7, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r7, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r7
    b	MATASK_POP3_Connect+0x33e
.align 2
    .word 0x0000047d
    .word 0x000006fa

    mov	r0, r4
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #48]
    str	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_POP3_Connect+0x2d8
    bl	InitPrevBuf
    mov	r4, r8
    strh	r4, [r7, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r7, #4]
    ldr	r4, [r5, #120]
    mov	r0, r4
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r5
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r7
    mov	r1, r4
    bl	MABIOS_Data
    b	MATASK_POP3_Connect+0x3d4
.align 2
    .word pop3res.184

    cmp	r1, #1
    bne	MATASK_POP3_Connect+0x2ec
    mov	r1, r5
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    add	r1, #2
    mov	r0, #2
    strh	r0, [r1, #0]
    b	MATASK_POP3_Connect+0x388
    mov	r1, r5
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r5
    add	r0, #104
    mov	r1, r8
    strh	r1, [r0, #0]
    b	MATASK_POP3_Connect+0x388
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r5, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r5, r1
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #44]
    add	r7, r5, r2
    ldr	r1, [pc, #44]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    mov	r0, r7
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_Connect+0x350
    mov	r2, r8
    strh	r2, [r4, #0]
    mov	r0, r5
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_Connect+0x422
.align 2
    .word 0x0000047d
    .word 0x000006fa

    mov	r0, r7
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #24]
    str	r1, [r0, #0]
    cmp	r1, #0
    beq	MATASK_POP3_Connect+0x3d4
    cmp	r1, #1
    bne	MATASK_POP3_Connect+0x378
    mov	r1, r5
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    add	r1, #2
    mov	r0, #3
    strh	r0, [r1, #0]
    b	MATASK_POP3_Connect+0x388
.align 2
    .word pop3res.184

    mov	r1, r5
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r5
    add	r0, #104
    mov	r4, r8
    strh	r4, [r0, #0]
    ldrb	r0, [r6, #0]
    mov	r0, #240
    strb	r0, [r6, #0]
    b	MATASK_POP3_Connect+0x422
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #5
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r2, #160
    lsl	r2, r2, #3
    mov	r0, r2
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_Connect+0x422
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r5, r4
    mov	r1, r8
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r6, #0]
    add	r0, #1
    ldrb	r1, [r6, #0]
    strb	r0, [r6, #0]
    b	MATASK_POP3_Connect+0x422
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    mov	r4, #0
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r1, r2
    orr	r0, r1
    ldrh	r1, [r5, #2]
    orr	r0, r4
    strh	r0, [r5, #2]
    mov	r0, r5
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r5
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r5
    add	r0, #204
    strb	r4, [r0, #0]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MATASK_POP3_Connect, .-MATASK_POP3_Connect
");
#endif

void MA_POP3_Stat(u16 *pNum, u32 *pSize)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_10)) {
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)pNum;
    gMA.unk_116 = (u32)pSize;
    MA_TaskSet(TASK_UNK_10, 0);
    ResetApiCallFlag();
}

static void MATASK_POP3_Stat(void)
{
    static const char *cp asm("cp.191");
    static int pop3res asm("pop3res.192");

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_SetApiError(MAAPIE_POP3, 0);
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        MAU_strcpy(gMA.unk_880, POP3_Stat);
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            return;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            cp = &gMA.prevbuf[4];
            *(u16 *)gMA.unk_112 = MAU_atoi(cp);
            cp = MAU_FindPostBlank((char *)cp);
            *(u32 *)gMA.unk_116 = MAU_atoi(cp);
            gMA.task_unk_98++;
        } else if (pop3res == 1) {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
        } else {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
        }
        break;

    case 2:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

void MA_POP3_List(u16 mailNo, u32 *pSize)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_11)) {
        ResetApiCallFlag();
        return;
    }

    if (mailNo == 0) {
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u8 *)pSize;
    MAU_strcpy(gMA.unk_880, POP3_List);
    MAU_itoa(mailNo, &gMA.unk_880[MAU_strlen(gMA.unk_880)], 10);
    MAU_strcat(gMA.unk_880, POP3_Newl);
    MA_TaskSet(TASK_UNK_11, 0);

    ResetApiCallFlag();
}

static void MATASK_POP3_List(void)
{
    static const char *cp asm("cp.199");
    static int pop3res asm("pop3res.200");

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_SetApiError(MAAPIE_POP3, 0);
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);

        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            return;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            cp = MAU_FindPostBlank(&gMA.prevbuf[4]);
            *(u32 *)gMA.unk_112 = MAU_atoi(cp);
            gMA.task_unk_98 = 100;
        } else if (pop3res == 1) {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 4;
            gMA.task_unk_98 = 0xf0;
        } else {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
        }
        break;

    case 100:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

void MA_POP3_Retr(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_12)) {
        ResetApiCallFlag();
        return;
    }

    *pRecvSize = 0;
    gMA.unk_112 = pRecvData;
    gMA.unk_116 = recvBufSize;
    gMA.unk_120 = (u32)pRecvSize;

    if (gMA.condition & MA_CONDITION_BUFFER_FULL) {
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        if (recvBufSize == 0) {
            gMA.unk_112 = (u8 *)(u32)recvBufSize;
            gMA.unk_120 = recvBufSize;
            MA_TaskSet(TASK_UNK_12, 1);
        } else if (gMA.prevbuf_size != 0 && gMA.prevbuf_size <= recvBufSize) {
            MAU_memcpy(pRecvData, gMA.prevbuf, gMA.prevbuf_size);
            *pRecvSize = gMA.prevbuf_size + *pRecvSize;
            gMA.prevbuf_size = 0;

            gMA.status |= STATUS_UNK_15;
            MA_TaskSet(TASK_UNK_12, 1);
        } else {
            MAU_memcpy(pRecvData, gMA.prevbuf, recvBufSize);
            gMA.prevbuf_size -= recvBufSize;
            MAU_memcpy(gMA.prevbuf, &gMA.prevbuf[recvBufSize], gMA.prevbuf_size);
            *pRecvSize = recvBufSize;

            gMA.condition |= MA_CONDITION_BUFFER_FULL;
            ResetApiCallFlag();
            return;
        }
    } else {
        if (recvBufSize == 0) {
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }

        gMA.unk_140 = 0;
        InitPrevBuf();
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        MAU_strcpy(gMA.unk_880, POP3_Retr);
        MAU_itoa(mailNo, &gMA.unk_880[MAU_strlen(gMA.unk_880)], 10);
        MAU_strcat(gMA.unk_880, POP3_Newl);

        MA_TaskSet(TASK_UNK_12, 0);
    }

    ResetApiCallFlag();
}

#if 0
#else
asm("
.lcomm cp.207, 0x4
.lcomm dataLen.208, 0x4
.lcomm pop3res.209, 0x4

.align 2
.thumb_func
MATASK_POP3_Retr:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_POP3_Retr+0x52
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_POP3_Retr+0x40
    mov	r2, r4
    add	r2, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r2, #0]
    mov	r0, r4
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_POP3_Retr+0x52
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_Retr+0x52
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #241
    strb	r1, [r0, #0]
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    mov	r9, r1
    cmp	r1, #0
    beq	MATASK_POP3_Retr+0xa4
    ldr	r3, [pc, #56]
    mov	r1, r3
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r3, #2]
    mov	r2, #0
    strh	r0, [r3, #2]
    ldrh	r0, [r3, #2]
    mov	r4, #128
    lsl	r4, r4, #1
    mov	r1, r4
    orr	r0, r1
    ldrh	r1, [r3, #2]
    orr	r0, r2
    strh	r0, [r3, #2]
    mov	r0, r3
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_POP3_Retr+0x358
.align 2
    .word gMA

    ldr	r6, [pc, #32]
    mov	r0, #98
    add	r0, r0, r6
    mov	r8, r0
    ldrb	r0, [r0, #0]
    mov	r4, r0
    mov	r7, r6
    cmp	r4, #100
    bne	MATASK_POP3_Retr+0xb8
    b	MATASK_POP3_Retr+0x358
    cmp	r4, #100
    bgt	MATASK_POP3_Retr+0xcc
    cmp	r4, #0
    beq	MATASK_POP3_Retr+0xda
    cmp	r4, #1
    beq	MATASK_POP3_Retr+0x11a
    b	MATASK_POP3_Retr+0x3ca
.align 2
    .word gMA

    cmp	r4, #240
    bne	MATASK_POP3_Retr+0xd2
    b	MATASK_POP3_Retr+0x362
    cmp	r4, #241
    bne	MATASK_POP3_Retr+0xd8
    b	MATASK_POP3_Retr+0x386
    b	MATASK_POP3_Retr+0x3ca
    bl	InitPrevBuf
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    mov	r2, r9
    strh	r2, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    mov	r3, #220
    lsl	r3, r3, #2
    add	r5, r6, r3
    mov	r0, r5
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r6
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, r5
    bl	MABIOS_Data
    mov	r4, r8
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_POP3_Retr+0x3ca
    ldr	r2, [pc, #108]
    mov	r0, #240
    lsl	r0, r0, #1
    add	r0, r0, r6
    mov	sl, r0
    ldrh	r0, [r0, #0]
    sub	r3, r0, #1
    str	r3, [r2, #0]
    ldr	r0, [r6, #64]
    mov	r1, #128
    lsl	r1, r1, #8
    and	r0, r1
    cmp	r0, #0
    beq	MATASK_POP3_Retr+0x138
    b	MATASK_POP3_Retr+0x2cc
    ldr	r0, [r6, #112]
    cmp	r0, #0
    bne	MATASK_POP3_Retr+0x140
    b	MATASK_POP3_Retr+0x2d4
    cmp	r3, #1
    bgt	MATASK_POP3_Retr+0x146
    b	MATASK_POP3_Retr+0x2d4
    ldr	r1, [pc, #68]
    ldr	r0, [r1, #0]
    cmp	r0, #0
    bne	MATASK_POP3_Retr+0x216
    ldr	r7, [pc, #64]
    mov	r2, #242
    lsl	r2, r2, #1
    add	r5, r6, r2
    ldr	r0, [r5, #0]
    add	r0, #1
    mov	r1, r3
    bl	MAU_SearchCRLF
    mov	r1, r0
    str	r1, [r7, #0]
    cmp	r1, #0
    bne	MATASK_POP3_Retr+0x194
    ldr	r0, [r5, #0]
    add	r0, #1
    ldr	r3, [pc, #24]
    ldrh	r1, [r3, #0]
    bl	ConcatPrevBuf
    mov	r0, r9
    mov	r4, sl
    strh	r0, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, sl
    b	MATASK_POP3_Retr+0x342
.align 2
    .word dataLen.208
    .word gMA+0x8c
    .word cp.207

    ldr	r2, [r5, #0]
    add	r0, r2, #1
    sub	r1, #1
    sub	r1, r1, r2
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r1, [pc, #40]
    add	r0, r6, r1
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #36]
    str	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_POP3_Retr+0x1e0
    ldr	r2, [r7, #0]
    sub	r2, #1
    ldr	r1, [r5, #0]
    sub	r1, r2, r1
    ldr	r3, [pc, #24]
    ldr	r0, [r3, #0]
    sub	r0, r0, r1
    str	r0, [r3, #0]
    str	r2, [r5, #0]
    ldr	r0, [pc, #16]
    str	r4, [r0, #0]
    mov	r7, r6
    b	MATASK_POP3_Retr+0x216
.align 2
    .word 0x0000047d
    .word pop3res.209
    .word dataLen.208
    .word gMA+0x8c

    cmp	r1, #1
    bne	MATASK_POP3_Retr+0x1fc
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    add	r1, #2
    mov	r0, #4
    strh	r0, [r1, #0]
    mov	r1, r8
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_POP3_Retr+0x3ca
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    mov	r2, r9
    strh	r2, [r0, #0]
    mov	r3, r8
    ldrb	r0, [r3, #0]
    mov	r0, #240
    strb	r0, [r3, #0]
    b	MATASK_POP3_Retr+0x3ca
    ldr	r3, [r7, #116]
    ldr	r4, [pc, #56]
    ldr	r2, [r4, #0]
    cmp	r3, r2
    bcc	MATASK_POP3_Retr+0x25c
    ldr	r0, [r7, #112]
    mov	r3, #242
    lsl	r3, r3, #1
    add	r1, r7, r3
    ldr	r1, [r1, #0]
    add	r1, #1
    bl	MAU_memcpy
    ldr	r0, [r7, #112]
    ldr	r2, [r4, #0]
    add	r0, r0, r2
    str	r0, [r7, #112]
    ldr	r0, [r7, #116]
    sub	r0, r0, r2
    str	r0, [r7, #116]
    ldrh	r1, [r7, #2]
    ldr	r0, [pc, #20]
    and	r0, r1
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    ldr	r1, [r7, #120]
    ldrh	r0, [r1, #0]
    add	r0, r0, r2
    strh	r0, [r1, #0]
    b	MATASK_POP3_Retr+0x2d4
.align 2
    .word dataLen.208
    .word 0x0000fffb

    ldr	r0, [r7, #112]
    mov	r1, #242
    lsl	r1, r1, #1
    add	r4, r7, r1
    ldr	r1, [r4, #0]
    add	r1, #1
    mov	r2, r3
    bl	MAU_memcpy
    ldr	r2, [pc, #80]
    ldr	r1, [r2, #0]
    ldr	r0, [r7, #116]
    sub	r1, r1, r0
    ldr	r0, [pc, #76]
    add	r3, r7, r0
    mov	r6, #0
    mov	r5, #0
    strh	r1, [r3, #0]
    ldr	r1, [pc, #68]
    add	r0, r7, r1
    ldr	r2, [r7, #116]
    add	r2, #1
    ldr	r1, [r4, #0]
    add	r1, r1, r2
    ldrh	r2, [r3, #0]
    bl	MAU_memcpy
    ldr	r2, [r7, #120]
    ldr	r1, [r7, #116]
    ldrh	r0, [r2, #0]
    add	r0, r0, r1
    strh	r0, [r2, #0]
    str	r5, [r7, #116]
    ldrh	r0, [r7, #2]
    mov	r1, #4
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r6
    strh	r0, [r7, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r0, [r7, #2]
    mov	r1, #1
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r6
    strh	r0, [r7, #2]
    b	MATASK_POP3_Retr+0x3ca
.align 2
    .word dataLen.208
    .word 0x000006fa
    .word 0x0000047d

    ldr	r0, [r6, #64]
    ldr	r1, [pc, #40]
    and	r0, r1
    str	r0, [r6, #64]
    ldr	r0, [pc, #36]
    ldr	r1, [r0, #0]
    cmp	r1, #4
    ble	MATASK_POP3_Retr+0x304
    ldr	r4, [pc, #32]
    mov	r3, #242
    lsl	r3, r3, #1
    add	r2, r4, r3
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldrh	r1, [r0, #0]
    ldr	r0, [r2, #0]
    add	r0, r0, r1
    sub	r0, #5
    mov	r1, #5
    b	MATASK_POP3_Retr+0x314
.align 2
    .word 0xffff7fff
    .word dataLen.208
    .word gMA

    cmp	r1, #0
    beq	MATASK_POP3_Retr+0x330
    ldr	r4, [pc, #32]
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r4, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    bl	MakeEndLineBuffer
    bl	IsEndMultiLine
    cmp	r0, #1
    bne	MATASK_POP3_Retr+0x330
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #100
    strb	r1, [r0, #0]
    b	MATASK_POP3_Retr+0x3ca
.align 2
    .word gMA

    ldr	r0, [pc, #24]
    mov	r1, #0
    strh	r1, [r0, #0]
    ldr	r3, [pc, #24]
    add	r1, r0, r3
    str	r1, [r0, #4]
    ldr	r4, [pc, #20]
    add	r1, r0, r4
    ldrb	r3, [r1, #0]
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_Retr+0x3ca
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffe83

    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_Retr+0x3ca
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r6, r1
    mov	r2, r9
    strh	r2, [r0, #0]
    mov	r1, r6
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    mov	r3, r8
    ldrb	r0, [r3, #0]
    add	r0, #1
    ldrb	r1, [r3, #0]
    strb	r0, [r3, #0]
    b	MATASK_POP3_Retr+0x3ca
    mov	r1, r6
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r6, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r6, #2]
    mov	r4, #0
    strh	r0, [r6, #2]
    ldrh	r0, [r6, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r1, r2
    orr	r0, r1
    ldrh	r1, [r6, #2]
    orr	r0, r4
    strh	r0, [r6, #2]
    mov	r0, r6
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r6
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r6
    add	r0, #204
    strb	r4, [r0, #0]
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MATASK_POP3_Retr, .-MATASK_POP3_Retr
");
#endif

void MA_POP3_Dele(u16 mailNo)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_13)) {
        ResetApiCallFlag();
        return;
    }

    MAU_strcpy(gMA.unk_880, POP3_Dele);
    MAU_itoa(mailNo, &gMA.unk_880[MAU_strlen(gMA.unk_880)], 10);
    MAU_strcat(gMA.unk_880, POP3_Newl);

    MA_TaskSet(TASK_UNK_13, 0);
    ResetApiCallFlag();
}

static void MATASK_POP3_Dele(void)
{
    static int pop3res asm("pop3res.216");

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;  // MAGIC
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_SetApiError(MAAPIE_POP3, 0);
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            return;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            gMA.task_unk_98++;
        } else if (pop3res == 1) {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 4;
            gMA.task_unk_98 = 0xf0;
        } else {
            gMA.unk_102 = 0x31;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
        }
        break;

    case 2:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

void MA_POP3_Head(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_12)) {
        ResetApiCallFlag();
        return;
    }

    *pRecvSize = 0;
    gMA.unk_112 = pRecvData;
    gMA.unk_116 = recvBufSize;
    gMA.unk_120 = (u32)pRecvSize;

    if (gMA.condition & MA_CONDITION_BUFFER_FULL) {
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        if (recvBufSize == 0) {
            gMA.unk_112 = (u8 *)(u32)recvBufSize;
            gMA.unk_120 = recvBufSize;
            MA_TaskSet(TASK_UNK_12, 1);
        } else if (gMA.prevbuf_size != 0 && gMA.prevbuf_size <= recvBufSize) {
            MAU_memcpy(pRecvData, gMA.prevbuf, gMA.prevbuf_size);
            *pRecvSize = gMA.prevbuf_size + *pRecvSize;
            gMA.prevbuf_size = 0;

            gMA.status |= STATUS_UNK_15;
            MA_TaskSet(TASK_UNK_12, 1);
        } else {
            MAU_memcpy(pRecvData, gMA.prevbuf, recvBufSize);
            gMA.prevbuf_size -= recvBufSize;
            MAU_memcpy(gMA.prevbuf, &gMA.prevbuf[recvBufSize], gMA.prevbuf_size);
            *pRecvSize = recvBufSize;

            gMA.condition |= MA_CONDITION_BUFFER_FULL;
            ResetApiCallFlag();
            return;
        }
    } else {
        if (recvBufSize == 0) {
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }

        gMA.unk_140 = 0;
        InitPrevBuf();
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;

        MAU_strcpy(gMA.unk_880, POP3_Top);
        MAU_itoa(mailNo, &gMA.unk_880[MAU_strlen(gMA.unk_880)], 10);
        MAU_strcat(gMA.unk_880, POP3_Zero);

        MA_TaskSet(TASK_UNK_12, 0);
    }

    ResetApiCallFlag();
}

static void MATASK_POP3_Head(void)
{
    static const char *cp asm("cp.223");
    static int dataLen asm("dataLen.224");
    static int pop3res asm("pop3res.225");

    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x15:
            gMA.unk_102 = 0x24;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xf0;
            break;

        case 0x24:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xf1;
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        MA_SetApiError(MAAPIE_POP3, 0);
        MA_TaskSet(TASK_UNK_00, 0);
        return;
    }

    switch (gMA.task_unk_98) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_unk_98++;
        return;

    case 1:
        dataLen = gMA.buffer_unk_480.size - 1;
        if (!(gMA.status & STATUS_UNK_15)) {
            if (gMA.unk_112 != 0 && dataLen > 1) {
                if (gMA.unk_140 == 0) {
                    cp = MAU_SearchCRLF(&gMA.buffer_unk_480.data[1], dataLen);
                    if (cp == NULL) {
                        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], dataLen);
                        (&gMA.buffer_unk_480)->size = 0;
                        (&gMA.buffer_unk_480)->data = gMA.unk_212;
                        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
                        return;
                    }
                    ConcatPrevBuf(&gMA.buffer_unk_480.data[1], cp - (char *)&gMA.buffer_unk_480.data[1]);
                    pop3res = CheckPOP3Response(gMA.prevbuf);
                    if (pop3res == 0) {
                        dataLen -= cp - (char *)&gMA.buffer_unk_480.data[1];
                        gMA.buffer_unk_480.data = (char *)&cp[-1];
                        gMA.unk_140 = 1;
                    } else if (pop3res == 1) {
                        gMA.unk_102 = 0x31;
                        gMA.unk_104 = 4;
                        gMA.task_unk_98 = 0xf0;
                        return;
                    } else {
                        gMA.unk_102 = 0x31;
                        gMA.unk_104 = 0;
                        gMA.task_unk_98 = 0xf0;
                        return;
                    }
                }

                if (gMA.unk_116 >= dataLen) {
                    MAU_memcpy(gMA.unk_112, &gMA.buffer_unk_480.data[1], dataLen);
                    gMA.unk_112 += dataLen;
                    gMA.unk_116 -= dataLen;
                    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
                    *(u16 *)gMA.unk_120 += dataLen;
                } else {
                    MAU_memcpy(gMA.unk_112, &gMA.buffer_unk_480.data[1], gMA.unk_116);
                    gMA.prevbuf_size = dataLen - gMA.unk_116;
                    MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[gMA.unk_116 + 1], gMA.prevbuf_size);
                    *(u16 *)gMA.unk_120 += gMA.unk_116;
                    gMA.unk_116 = 0;
                    gMA.condition |= MA_CONDITION_BUFFER_FULL;
                    MA_TaskSet(TASK_UNK_00, 0);
                    gMA.condition |= MA_CONDITION_APIWAIT;
                    return;
                }
            }
        }
        else {
            gMA.status &= ~STATUS_UNK_15;
        }

        if (dataLen >= 5) {
            MakeEndLineBuffer(gMA.buffer_unk_480.data + gMA.buffer_unk_480.size - 5, 5);
            if (IsEndMultiLine() == TRUE) {
                gMA.task_unk_98 = 100;
                return;
            }
        } else {
            if (dataLen != 0) {
                MakeEndLineBuffer(&gMA.buffer_unk_480.data[1], dataLen);
                if (IsEndMultiLine() == TRUE) {
                    gMA.task_unk_98 = 100;
                    return;
                }
            }
        }
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
        return;

    case 100:
        MA_TaskSet(TASK_UNK_00, 0);
        return;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_unk_98++;
        return;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        return;
    }
}

#if 0

static char *ExtractServerName(char *unk_1, char *unk_2, u8 *unk_3, u8 *unk_4)
{
    static const char strDownload[] asm("strDownload.229") = "gameboy.datacenter.ne.jp/cgb/download";
    static const char strUpload[] asm("strUpload.230") = "gameboy.datacenter.ne.jp/cgb/upload";
    static const char strUtility[] asm("strUtility.231") = "gameboy.datacenter.ne.jp/cgb/utility";
    static const char strRanking[] asm("strRanking.232") = "gameboy.datacenter.ne.jp/cgb/ranking";
    static const char strHttp[] asm("strHttp.233") = "http://";

    static char *cp asm("cp.234");
    static char *tmpp asm("tmpp.235");
    static int len asm("len.236");

    if (MAU_strnicmp(unk_2, strHttp, sizeof(strHttp) - 1) == 0) {
        unk_2 += sizeof(strHttp) - 1;
    }

    cp = MAU_strchr(unk_2, '/');
    if (!cp) {
        if (MAU_strchr(unk_2, '\0') - unk_2 > 0xff) {
            *unk_1 = '\0';
            return NULL;
        }
        MAU_strcpy(unk_1, unk_2);
        *unk_3 = 0;
        return NULL;
    }

    len = cp - unk_2;
    if (len > 0xff) {
        *unk_1 = '\0';
        return NULL;
    }

    MAU_memcpy(unk_1, unk_2, len);
    unk_1[len] = '\0';

    if (MAU_strnicmp(unk_2, strDownload, sizeof(strDownload) - 1) == 0) {
        *unk_4 = 1;
        *unk_3 = 1;
        tmpp = MAU_strrchr(unk_2, '/') + 1;
        if (tmpp[0] < '0' || tmpp[0] > '9') {
            *unk_3 = 0;
            return cp;
        } else {
            return cp;
        }
    }

    if (MAU_strnicmp(unk_2, strUpload, sizeof(strUpload) - 1) == 0) {
        *unk_4 = 2;
        *unk_3 = 2;
        tmpp = MAU_strrchr(unk_2, '/') + 1;
        if (tmpp[0] < '0' || tmpp[0] > '9') {
            *unk_3 = 0;
            return cp;
        } else {
            return cp;
        }
    }

    if (MAU_strnicmp(unk_2, strUtility, sizeof(strUtility) - 1) == 0) {
        *unk_4 = 3;
        *unk_3 = 3;
        return cp;
    }

    if (MAU_strnicmp(unk_2, strRanking, sizeof(strRanking) - 1) == 0) {
        *unk_4 = 4;
        *unk_3 = 4;
        tmpp = MAU_strrchr(unk_2, '/') + 1;
        if (tmpp[0] < '0' || tmpp[0] > '9') {
            *unk_3 = 0;
            return cp;
        } else {
            return cp;
        }
    }

    return cp;
}

#else
asm("
.section .rodata
.align 2
.type strDownload.229, object
strDownload.229:
    .asciz \"gameboy.datacenter.ne.jp/cgb/download\"
.size strDownload.229, .-strDownload.229

.align 2
.type strUpload.230, object
strUpload.230:
    .asciz \"gameboy.datacenter.ne.jp/cgb/upload\"
.size strUpload.230, .-strUpload.230

.align 2
.type strUtility.231, object
strUtility.231:
    .asciz \"gameboy.datacenter.ne.jp/cgb/utility\"
.size strUtility.231, .-strUtility.231

.align 2
.type strRanking.232, object
strRanking.232:
    .asciz \"gameboy.datacenter.ne.jp/cgb/ranking\"
.size strRanking.232, .-strRanking.232

.align 2
.type strHttp.233, object
strHttp.233:
    .asciz \"http://\"
.size strHttp.233, .-strHttp.233
.section .text

.lcomm cp.234, 0x4
.lcomm tmpp.235, 0x4
.lcomm len.236, 0x4

.align 2
.thumb_func
ExtractServerName:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r6, r0
    mov	r4, r1
    mov	r7, r2
    mov	r9, r3
    ldr	r1, [pc, #52]
    mov	r0, r4
    mov	r2, #7
    bl	MAU_strnicmp
    cmp	r0, #0
    bne	ExtractServerName+0x20
    add	r4, #7
    mov	r0, r4
    mov	r1, #47
    bl	MAU_strchr
    mov	r5, r0
    ldr	r0, [pc, #32]
    str	r5, [r0, #0]
    cmp	r5, #0
    bne	ExtractServerName+0x5c
    mov	r0, r4
    mov	r1, #0
    bl	MAU_strchr
    sub	r0, r0, r4
    cmp	r0, #255
    ble	ExtractServerName+0x50
    strb	r5, [r6, #0]
    mov	r0, #0
    b	ExtractServerName+0x14e
.align 2
    .word strHttp.233
    .word cp.234

    mov	r0, r6
    mov	r1, r4
    bl	MAU_strcpy
    strb	r5, [r7, #0]
    b	ExtractServerName+0x14a
    ldr	r0, [pc, #16]
    mov	r8, r0
    sub	r2, r5, r4
    str	r2, [r0, #0]
    cmp	r2, #255
    ble	ExtractServerName+0x74
    mov	r0, #0
    strb	r0, [r6, #0]
    mov	r0, #0
    b	ExtractServerName+0x14e
.align 2
    .word len.236

    mov	r0, r6
    mov	r1, r4
    bl	MAU_memcpy
    mov	r1, r8
    ldr	r0, [r1, #0]
    add	r0, r6, r0
    mov	r6, #0
    strb	r6, [r0, #0]
    ldr	r1, [pc, #48]
    mov	r0, r4
    mov	r2, #37
    bl	MAU_strnicmp
    mov	r5, r0
    cmp	r5, #0
    bne	ExtractServerName+0xc0
    mov	r6, #1
    mov	r0, r9
    strb	r6, [r0, #0]
    mov	r0, r4
    mov	r1, #47
    bl	MAU_strrchr
    ldr	r2, [pc, #20]
    add	r1, r0, #1
    str	r1, [r2, #0]
    ldrb	r0, [r0, #1]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bls	ExtractServerName+0x148
    b	ExtractServerName+0x58
.align 2
    .word strDownload.229
    .word tmpp.235

    ldr	r1, [pc, #48]
    mov	r0, r4
    mov	r2, #35
    bl	MAU_strnicmp
    mov	r5, r0
    cmp	r5, #0
    bne	ExtractServerName+0xfc
    mov	r6, #2
    mov	r1, r9
    strb	r6, [r1, #0]
    mov	r0, r4
    mov	r1, #47
    bl	MAU_strrchr
    ldr	r2, [pc, #24]
    add	r1, r0, #1
    str	r1, [r2, #0]
    ldrb	r0, [r0, #1]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bls	ExtractServerName+0x148
    b	ExtractServerName+0x58
.align 2
    .word strUpload.230
    .word tmpp.235

    ldr	r1, [pc, #20]
    mov	r0, r4
    mov	r2, #36
    bl	MAU_strnicmp
    cmp	r0, #0
    bne	ExtractServerName+0x118
    mov	r0, #3
    mov	r1, r9
    strb	r0, [r1, #0]
    strb	r0, [r7, #0]
    b	ExtractServerName+0x14a
.align 2
    .word strUtility.231

    ldr	r1, [pc, #64]
    mov	r0, r4
    mov	r2, #36
    bl	MAU_strnicmp
    mov	r5, r0
    cmp	r5, #0
    bne	ExtractServerName+0x148
    mov	r6, #4
    mov	r0, r9
    strb	r6, [r0, #0]
    mov	r0, r4
    mov	r1, #47
    bl	MAU_strrchr
    ldr	r2, [pc, #40]
    add	r1, r0, #1
    str	r1, [r2, #0]
    ldrb	r0, [r0, #1]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bhi	ExtractServerName+0x58
    strb	r6, [r7, #0]
    ldr	r0, [pc, #24]
    ldr	r0, [r0, #0]
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r1}
    bx	r1
.align 2
    .word strRanking.232
    .word tmpp.235
    .word cp.234
.size ExtractServerName, .-ExtractServerName
");
#endif

static const char strHttpGet[] = "GET ";
static const char strHttpPost[] = "POST ";
static const char strHttpReqSuffix[] = " HTTP/1.0\r\n";
static const char strHttpContentLength[] = "Content-Length: 0\r\n";
static const char strHttpAuthenticate[] = "WWW-Authenticate: GB00 name=\"";
static const char strHttpAuthorization[] = "Authorization: GB00 name=\"";
static const char strHttpContentType[] = "Content-Type: application/x-cgb\r\n";
static const char strHttpGbStatus[] = "Gb-Status: ";
static const char strHttpGbAuthID[] = "Gb-Auth-ID: ";
static const char strHttpDate[] = "Date: ";
static const char strHttpLocation[] = "Location: ";
static const char strHttpUserAgent[] = "User-Agent: AGB-";
static const char strServerRoot[] = "/";
asm(".word 0x00000000\n");
asm(".section .text\n");

#if 0
#else
void MA_HTTP_Get(const char *pURL, char *pHeadBuf, u16 headBufSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword);
asm("
.align 2
.thumb_func
.global MA_HTTP_Get
MA_HTTP_Get:
    push	{r4, r5, r6, r7, lr}
    sub	sp, #28
    ldr	r5, [sp, #48]
    ldr	r6, [sp, #52]
    ldr	r7, [sp, #56]
    ldr	r4, [sp, #60]
    str	r4, [sp, #20]
    lsl	r2, r2, #16
    lsr	r2, r2, #16
    lsl	r5, r5, #16
    lsr	r5, r5, #16
    mov	r4, #0
    str	r4, [sp, #0]
    str	r3, [sp, #4]
    str	r5, [sp, #8]
    str	r6, [sp, #12]
    str	r7, [sp, #16]
    mov	r3, #22
    str	r3, [sp, #24]
    mov	r3, #0
    bl	MA_HTTP_GetPost
    add	sp, #28
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MA_HTTP_Get, .-MA_HTTP_Get
");
#endif

#if 0
#else
void MA_HTTP_Post(const char *pURL, char *pHeadBuf, u16 headBufSize, const u8 *pSendData, u16 sendSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword);
asm("
.align 2
.thumb_func
.global MA_HTTP_Post
MA_HTTP_Post:
    push	{r4, r5, r6, r7, lr}
    sub	sp, #28
    ldr	r4, [sp, #48]
    ldr	r6, [sp, #52]
    ldr	r5, [sp, #56]
    ldr	r7, [sp, #60]
    str	r7, [sp, #12]
    ldr	r7, [sp, #64]
    str	r7, [sp, #16]
    ldr	r7, [sp, #68]
    str	r7, [sp, #20]
    lsl	r2, r2, #16
    lsr	r2, r2, #16
    lsl	r4, r4, #16
    lsr	r4, r4, #16
    lsl	r5, r5, #16
    lsr	r5, r5, #16
    str	r4, [sp, #0]
    str	r6, [sp, #4]
    str	r5, [sp, #8]
    mov	r4, #23
    str	r4, [sp, #24]
    bl	MA_HTTP_GetPost
    add	sp, #28
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MA_HTTP_Post, .-MA_HTTP_Post
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MA_HTTP_GetPost:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    sub	sp, #20
    str	r0, [sp, #4]
    str	r1, [sp, #8]
    str	r3, [sp, #12]
    ldr	r1, [sp, #52]
    ldr	r7, [sp, #56]
    ldr	r3, [sp, #60]
    ldr	r0, [sp, #64]
    mov	r9, r0
    ldr	r0, [sp, #76]
    lsl	r2, r2, #16
    lsr	r2, r2, #16
    mov	sl, r2
    lsl	r1, r1, #16
    lsr	r5, r1, #16
    lsl	r3, r3, #16
    lsr	r6, r3, #16
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    mov	r8, r0
    mov	r1, r8
    str	r1, [sp, #16]
    bl	SetApiCallFlag
    mov	r0, r8
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_HTTP_GetPost+0x4a
    bl	ResetApiCallFlag
    b	MA_HTTP_GetPost+0x358
    ldr	r4, [pc, #84]
    mov	r2, r8
    str	r2, [r4, #112]
    str	r7, [r4, #120]
    str	r6, [r4, #124]
    mov	r0, r9
    ldr	r1, [pc, #76]
    str	r0, [r1, #0]
    mov	r0, r4
    add	r0, #132
    ldr	r2, [sp, #12]
    str	r2, [r0, #0]
    add	r0, #4
    str	r5, [r0, #0]
    add	r0, #44
    ldr	r1, [sp, #68]
    str	r1, [r0, #0]
    add	r0, #4
    ldr	r2, [sp, #72]
    str	r2, [r0, #0]
    sub	r0, #28
    mov	r1, sl
    str	r1, [r0, #0]
    add	r0, #12
    ldr	r2, [sp, #8]
    str	r2, [r0, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #4
    and	r0, r1
    lsl	r0, r0, #16
    lsr	r0, r0, #16
    mov	r5, r4
    cmp	r0, #0
    beq	MA_HTTP_GetPost+0x90
    b	MA_HTTP_GetPost+0x266
    cmp	r6, #0
    bne	MA_HTTP_GetPost+0xa8
    str	r6, [r4, #120]
    mov	r0, r9
    cmp	r0, #0
    beq	MA_HTTP_GetPost+0xb2
    strh	r6, [r0, #0]
    b	MA_HTTP_GetPost+0xb2
.align 2
    .word gMA
    .word gMA+0x80

    cmp	r7, #0
    bne	MA_HTTP_GetPost+0xae
    b	MA_HTTP_GetPost+0x2a4
    mov	r1, r9
    strh	r0, [r1, #0]
    mov	r0, r5
    add	r0, #156
    ldr	r0, [r0, #0]
    cmp	r0, #0
    beq	MA_HTTP_GetPost+0xca
    mov	r0, r5
    add	r0, #168
    ldr	r1, [r0, #0]
    cmp	r1, #0
    beq	MA_HTTP_GetPost+0xca
    mov	r0, #0
    strb	r0, [r1, #0]
    ldr	r0, [sp, #4]
    bl	MAU_strlen
    mov	r1, r0
    mov	r0, #128
    lsl	r0, r0, #3
    cmp	r1, r0
    ble	MA_HTTP_GetPost+0xdc
    b	MA_HTTP_GetPost+0x2a4
    cmp	r1, #0
    bne	MA_HTTP_GetPost+0xe2
    b	MA_HTTP_GetPost+0x2a4
    mov	r4, sp
    add	r4, #1
    mov	r0, #0
    strb	r0, [r4, #0]
    ldr	r5, [pc, #56]
    mov	r0, r5
    ldr	r1, [sp, #4]
    mov	r2, sp
    mov	r3, r4
    bl	ExtractServerName
    mov	r7, r0
    ldrb	r0, [r5, #0]
    cmp	r0, #0
    bne	MA_HTTP_GetPost+0x102
    b	MA_HTTP_GetPost+0x2a4
    ldr	r2, [pc, #36]
    add	r1, r5, r2
    mov	r0, sp
    ldrb	r0, [r0, #0]
    str	r0, [r1, #0]
    ldrb	r0, [r4, #0]
    cmp	r0, #0
    bne	MA_HTTP_GetPost+0x134
    ldr	r1, [pc, #24]
    add	r0, r5, r1
    ldr	r1, [pc, #24]
    str	r1, [r0, #0]
    add	r2, #24
    add	r0, r5, r2
    str	r1, [r0, #0]
    b	MA_HTTP_GetPost+0x150
.align 2
    .word gMA+0x370
    .word 0xfffffd30
    .word 0xfffffd44
    .word strServerRoot+0x4

    ldr	r0, [sp, #68]
    bl	MAU_strlen
    mov	r1, r0
    cmp	r1, #16
    ble	MA_HTTP_GetPost+0x142
    b	MA_HTTP_GetPost+0x2a4
    ldr	r0, [sp, #72]
    bl	MAU_strlen
    mov	r1, r0
    cmp	r1, #16
    ble	MA_HTTP_GetPost+0x150
    b	MA_HTTP_GetPost+0x2a4
    ldrb	r4, [r4, #0]
    ldr	r5, [pc, #80]
    cmp	r4, #1
    bne	MA_HTTP_GetPost+0x160
    ldr	r0, [r5, #112]
    cmp	r0, #23
    bne	MA_HTTP_GetPost+0x160
    b	MA_HTTP_GetPost+0x2a4
    cmp	r4, #2
    bne	MA_HTTP_GetPost+0x16c
    ldr	r0, [r5, #112]
    cmp	r0, #22
    bne	MA_HTTP_GetPost+0x16c
    b	MA_HTTP_GetPost+0x2a4
    cmp	r4, #3
    bne	MA_HTTP_GetPost+0x178
    ldr	r0, [r5, #112]
    cmp	r0, #23
    bne	MA_HTTP_GetPost+0x178
    b	MA_HTTP_GetPost+0x2a4
    cmp	r4, #4
    bne	MA_HTTP_GetPost+0x184
    ldr	r0, [r5, #112]
    cmp	r0, #22
    bne	MA_HTTP_GetPost+0x184
    b	MA_HTTP_GetPost+0x2a4
    mov	r1, #219
    lsl	r1, r1, #2
    add	r0, r5, r1
    ldr	r0, [r0, #0]
    mov	r6, #0
    cmp	r0, #0
    bne	MA_HTTP_GetPost+0x194
    mov	r6, #1
    cmp	r6, #1
    bne	MA_HTTP_GetPost+0x1b8
    cmp	r7, #0
    beq	MA_HTTP_GetPost+0x1a8
    mov	r0, r5
    add	r0, #140
    str	r7, [r0, #0]
    b	MA_HTTP_GetPost+0x1c0
.align 2
    .word gMA

    mov	r1, r5
    add	r1, #140
    ldr	r0, [pc, #4]
    str	r0, [r1, #0]
    b	MA_HTTP_GetPost+0x1c0
.align 2
    .word strServerRoot

    mov	r0, r5
    add	r0, #140
    ldr	r2, [sp, #4]
    str	r2, [r0, #0]
    mov	r4, r5
    add	r4, #140
    ldr	r0, [r4, #0]
    bl	MAU_strlen
    mov	r1, r5
    add	r1, #144
    str	r0, [r1, #0]
    mov	r2, r5
    add	r2, #148
    ldr	r1, [r4, #0]
    str	r1, [r2, #0]
    mov	r1, r5
    add	r1, #152
    str	r0, [r1, #0]
    mov	r0, #7
    str	r0, [r5, #116]
    bl	InitPrevBuf
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #92]
    and	r0, r1
    ldrh	r1, [r5, #2]
    mov	r3, #0
    mov	r2, #0
    strh	r0, [r5, #2]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #84]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r1, [pc, #80]
    add	r0, r5, r1
    strh	r2, [r0, #0]
    sub	r1, #2
    add	r0, r5, r1
    strh	r2, [r0, #0]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #72]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r1, [pc, #68]
    add	r0, r5, r1
    strb	r3, [r0, #0]
    mov	r0, r5
    add	r0, #164
    str	r2, [r0, #0]
    add	r0, #24
    str	r2, [r0, #0]
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #6
    strb	r0, [r1, #0]
    cmp	r6, #0
    bne	MA_HTTP_GetPost+0x25c
    mov	r0, r5
    add	r0, #106
    mov	r2, #219
    lsl	r2, r2, #2
    add	r1, r5, r2
    mov	r2, #4
    bl	MAU_memcpy
    ldr	r0, [sp, #16]
    mov	r1, #2
    bl	MA_TaskSet
    b	MA_HTTP_GetPost+0x354
.align 2
    .word 0x0000fffb
    .word 0xffff7fff
    .word 0x00000704
    .word 0xfffeffff
    .word 0x00000706

    ldr	r0, [sp, #16]
    mov	r1, #0
    bl	MA_TaskSet
    b	MA_HTTP_GetPost+0x354
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #44]
    and	r0, r1
    ldrh	r1, [r4, #2]
    mov	r1, #0
    mov	sl, r1
    strh	r0, [r4, #2]
    ldr	r0, [r4, #64]
    ldr	r1, [pc, #36]
    and	r0, r1
    str	r0, [r4, #64]
    cmp	r6, #0
    bne	MA_HTTP_GetPost+0x2a0
    str	r6, [r4, #120]
    mov	r2, r9
    cmp	r2, #0
    beq	MA_HTTP_GetPost+0x28a
    strh	r6, [r2, #0]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #8
    orr	r0, r1
    str	r0, [r4, #64]
    mov	r0, r8
    b	MA_HTTP_GetPost+0x2fc
.align 2
    .word 0x0000fffb
    .word 0xffff7fff

    cmp	r7, #0
    bne	MA_HTTP_GetPost+0x2b2
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_HTTP_GetPost+0x358
    mov	r1, sl
    mov	r0, r9
    strh	r1, [r0, #0]
    ldr	r2, [pc, #72]
    add	r5, r4, r2
    ldrh	r0, [r5, #0]
    cmp	r0, #0
    beq	MA_HTTP_GetPost+0x310
    cmp	r0, r6
    bhi	MA_HTTP_GetPost+0x310
    ldr	r0, [pc, #64]
    add	r1, r4, r0
    ldrh	r2, [r5, #0]
    mov	r0, r7
    bl	MAU_memcpy
    ldrh	r1, [r5, #0]
    add	r7, r7, r1
    sub	r0, r6, r1
    lsl	r0, r0, #16
    lsr	r6, r0, #16
    mov	r2, r9
    ldrh	r0, [r2, #0]
    add	r0, r0, r1
    strh	r0, [r2, #0]
    mov	r0, sl
    strh	r0, [r5, #0]
    str	r7, [r4, #120]
    str	r6, [r4, #124]
    ldr	r1, [pc, #28]
    str	r2, [r1, #0]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #8
    orr	r0, r1
    str	r0, [r4, #64]
    ldr	r0, [sp, #16]
    mov	r1, #110
    bl	MA_TaskSet
    b	MA_HTTP_GetPost+0x354
.align 2
    .word 0x000006fa
    .word 0x0000047d
    .word gMA+0x80

    ldr	r4, [pc, #52]
    mov	r0, r7
    mov	r1, r4
    mov	r2, r6
    bl	MAU_memcpy
    ldr	r2, [pc, #44]
    add	r5, r4, r2
    ldr	r0, [pc, #44]
    add	r2, r4, r0
    ldrh	r0, [r2, #0]
    sub	r0, r0, r6
    strh	r0, [r2, #0]
    add	r1, r6, r4
    ldrh	r2, [r2, #0]
    mov	r0, r4
    bl	MAU_memcpy
    mov	r1, r9
    strh	r6, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #4
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    bl	ResetApiCallFlag
    b	MA_HTTP_GetPost+0x358
.align 2
    .word gMA+0x47d
    .word 0xfffffb83
    .word 0x0000027d

    bl	ResetApiCallFlag
    add	sp, #20
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MA_HTTP_GetPost, .-MA_HTTP_GetPost
");
#endif

#define ConcatUserAgent_WriteAscii(dest, code) \
{ \
    if (code < 0x20 || code >= 0x7f) { \
        *dest = '0'; \
    } else { \
        *dest = code; \
    } \
}

asm(".section .rodata\n");

static void ConcatUserAgent(char *user_agent)
{
    static int tmpLen asm("tmpLen.249");
    static u8 *tmpp asm("tmpp.250");
    static u8 tmpNum asm("tmpNum.251");
    static const char hexChar[] asm("hexChar.252") = "0123456789ABCDEF";

    tmpLen = MAU_strlen(user_agent);
    tmpp = &user_agent[tmpLen];

    ConcatUserAgent_WriteAscii(tmpp++, *(char *)(CASSETTE_INITIAL_CODE + 0));
    ConcatUserAgent_WriteAscii(tmpp++, *(char *)(CASSETTE_INITIAL_CODE + 1));
    ConcatUserAgent_WriteAscii(tmpp++, *(char *)(CASSETTE_INITIAL_CODE + 2));
    ConcatUserAgent_WriteAscii(tmpp++, *(char *)(CASSETTE_INITIAL_CODE + 3));

    *tmpp++ = '-';
    tmpNum = *(u8 *)CASSETTE_VERSION_NO;
    *tmpp++ = hexChar[(tmpNum >> 4) & 0xf];
    *tmpp++ = hexChar[(tmpNum >> 0) & 0xf];

    *tmpp++ = '\r';
    *tmpp++ = '\n';
    *tmpp++ = '\r';
    *tmpp++ = '\n';
    *tmpp = '\0';
}

static const char strNewl[] asm(".LstrHttpNewl") = "\"\r\n";
asm("
.section .rodata
.align 2
    .asciz \"HTTP\"
.section .text
");

static int GetRequestType(void)
{
    static int ret asm("ret.256");

    ret = 0;
    switch ((u32)gMA.unk_112) {  // MAGIC
    case 0x16:
        switch (gMA.unk_160) {  // MAGIC
        case 1:
            switch (gMA.unk_164) {  // MAGIC
            case 0:
            case 1:
                break;

            case 2:
                ret = 1;
                break;
            }
            break;

        }
        break;

    case 0x17:
        switch (gMA.unk_160) {  // MAGIC
        case 0:
            ret = 1;
            break;

        case 2:
            switch (gMA.unk_164) {  // MAGIC
            case 0:
            case 1:
                break;

            case 2:
                ret = 1;
                break;
            }
            break;

        case 4:
            switch (gMA.unk_164) {  // MAGIC
            case 0:
                break;

            case 1:
            case 2:
                ret = 1;
                break;
            }
            break;
        }
    }
    return ret;
}

static void CreateHttpRequestHeader(void)
{
    static int tmpLen asm("tmpLen.260");
    static char bAddContentLength asm("bAddContentLength.261");
    static char bAddContentLengthZero asm("bAddContentLengthZero.262");
    static char bAddAuthorization asm("bAddAuthorization.263");
    static char bAddAuthID asm("bAddAuthID.264");

    bAddContentLength = FALSE;
    bAddContentLengthZero = FALSE;
    bAddAuthorization = FALSE;
    bAddAuthID = FALSE;

    // ALL MAGIC
    switch ((u32)gMA.unk_112) {
    case 0x16:
        switch (gMA.unk_160) {
        case 0:
            break;

        case 3:
            switch (gMA.unk_164) {
            case 1:
                bAddAuthorization = TRUE;
                break;

            case 2:
                break;
            }
            break;

        case 1:
            switch (gMA.unk_164) {
            case 0:
                break;

            case 1:
                bAddAuthorization = TRUE;
                break;

            case 2:
                bAddAuthID = TRUE;
                break;
            }
            break;
        }
        break;

    case 0x17:
        switch (gMA.unk_160) {
        case 0:
            bAddContentLength = TRUE;
            break;

        case 2:
            switch (gMA.unk_164) {
            case 0:
                break;

            case 1:
                bAddAuthorization = TRUE;
                break;

            case 2:
                bAddAuthID = TRUE;
                bAddContentLength = TRUE;
                break;
            }
            break;

        case 4:
            switch (gMA.unk_164) {
            case 0:
                break;

            case 1:
                bAddContentLength = TRUE;
                bAddAuthorization = TRUE;
                break;

            case 2:
                bAddAuthID = TRUE;
                break;
            }
            break;
        }
        break;
    }

    if (bAddContentLength == TRUE) {
        MAU_strcat(gMA.unk_880, strHttpContentType);
        MAU_strcat(gMA.unk_880, strHttpContentLength);
        tmpLen = MAU_strlen(gMA.unk_880);
        MAU_itoa(gMA.unk_136, &gMA.unk_880[tmpLen - 3], 10);
        MAU_strcat(gMA.unk_880, POP3_Newl);
    }

    if (bAddContentLengthZero == TRUE) {
        MAU_strcat(gMA.unk_880, strHttpContentType);
        MAU_strcat(gMA.unk_880, strHttpContentLength);
    }

    if (bAddAuthorization == TRUE) {
        MAU_strcat(gMA.unk_880, strHttpAuthorization);
        MAU_strcat(gMA.unk_880, gMA.unk_1798);
        MAU_strcat(gMA.unk_880, strNewl);
    }

    if (bAddAuthID == TRUE) {
        MAU_strcat(gMA.unk_880, strHttpGbAuthID);
        MAU_strcat(gMA.unk_880, gMA.unk_1798);
        MAU_strcat(gMA.unk_880, POP3_Newl);
    }

    MAU_strcat(gMA.unk_880, strHttpUserAgent);
    ConcatUserAgent(gMA.unk_880);
}

static int HttpGetNextStep(int unk_1)
{
    static int step asm("step.268");

    // ALL MAGIC
    switch (unk_1) {
    case 0:
        switch ((u32)gMA.unk_112) {
        case 0x16:
            switch (gMA.unk_160) {
            case 0:
                step = 8;
                break;

            case 1:
                step = 8;
                break;

            case 3:
                step = 8;
                break;
            }
            break;

        case 0x17:
            switch (gMA.unk_160) {
            case 0:
                step = 100;
                break;

            case 2:
                switch (gMA.unk_164) {
                case 0:
                    step = 8;
                    break;

                case 1:
                    step = 8;
                    break;

                case 2:
                    step = 100;
                    break;
                }
                break;

            case 4:
                switch (gMA.unk_164) {
                case 0:
                    step = 8;
                    break;

                case 1:
                    step = 100;
                    break;

                case 2:
                    step = 8;
                    break;
                }
                break;
            }
            break;
        }
        break;

    case 1:
        switch ((u32)gMA.unk_112) {
        case 0x16:
            switch (gMA.unk_160) {
            case 0:
                step = 0x6e;
                break;

            case 3:
                switch (gMA.unk_164) {
                case 0:
                case 1:
                    step = 0x6e;
                    break;
                }
                break;

            case 1:
                switch (gMA.unk_164) {
                case 0:
                case 1:
                    step = 0x6e;
                    break;

                case 2:
                    step = 0xff;
                    break;
                }
                break;
            }
            break;

        case 0x17:
            switch (gMA.unk_160) {
            case 0:
                step = 0x6e;
                break;

            case 2:
                switch (gMA.unk_164) {
                case 0:
                case 1:
                    step = 0x6e;
                    break;

                case 2:
                    step = 0xff;
                    break;
                }
                break;

            case 4:
                switch (gMA.unk_164) {
                case 0:
                case 1:
                    step = 0x6e;
                    break;

                case 2:
                    step = 0xff;
                    break;
                }
                break;
            }
            break;
        }
        break;

    case 2:
        switch ((u32)gMA.unk_112) {
        case 0x16:
            switch (gMA.unk_160) {
            case 0:
                step = 0xff;
                break;

            case 1:
                switch (gMA.unk_164) {
                case 1:
                case 2:
                    step = 2;
                    break;
                }
                break;

            case 3:
                switch (gMA.unk_164) {
                case 1:
                    step = 2;
                    break;

                case 2:
                    step = 0xff;
                    break;
                }
                break;
            }
            break;

        case 0x17:
            switch (gMA.unk_160) {
            case 0:
                step = 0xff;
                break;

            case 2:
                switch (gMA.unk_164) {
                case 1:
                case 2:
                    step = 2;
                    break;
                }
                break;

            case 4:
                switch (gMA.unk_164) {
                case 1:
                case 2:
                    step = 2;
                    break;
                }
                break;
            }
            break;
        }
        break;
    }
    return step;
}

#if 0
#else
asm("
.lcomm curCp.272, 0x4
.lcomm nextCp.273, 0x4
.lcomm lineCp.274, 0x4
.lcomm tmpLen.275, 0x4
.lcomm dataLen.276, 0x4
.lcomm headerLineLen.277, 0x4

.align 2
.thumb_func
MATASK_HTTP_GetPost:
    push	{r4, r5, r6, r7, lr}
    mov	r7, sl
    mov	r6, r9
    mov	r5, r8
    push	{r5, r6, r7}
    ldr	r2, [pc, #56]
    ldr	r1, [pc, #56]
    ldr	r0, [pc, #60]
    mov	r3, #0
    str	r3, [r0, #0]
    str	r3, [r1, #0]
    str	r3, [r2, #0]
    ldr	r2, [pc, #52]
    ldr	r1, [pc, #56]
    ldr	r0, [pc, #56]
    str	r3, [r0, #0]
    str	r3, [r1, #0]
    str	r3, [r2, #0]
    ldr	r1, [pc, #52]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_HTTP_GetPost+0xb2
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #35
    beq	MATASK_HTTP_GetPost+0x82
    cmp	r0, #35
    bgt	MATASK_HTTP_GetPost+0x60
    cmp	r0, #21
    beq	MATASK_HTTP_GetPost+0x6a
    b	MATASK_HTTP_GetPost+0xa4
.align 2
    .word curCp.272
    .word nextCp.273
    .word lineCp.274
    .word tmpLen.275
    .word dataLen.276
    .word headerLineLen.277
    .word gMA

    cmp	r0, #36
    beq	MATASK_HTTP_GetPost+0xb2
    cmp	r0, #40
    beq	MATASK_HTTP_GetPost+0x82
    b	MATASK_HTTP_GetPost+0xa4
    mov	r2, r1
    add	r2, #102
    mov	r0, #36
    strb	r0, [r2, #0]
    mov	r0, r1
    add	r0, #104
    strh	r3, [r0, #0]
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0xb2
    ldr	r2, [pc, #28]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #36
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #241
    strb	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0xb2
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #56]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #241
    strb	r1, [r0, #0]
    ldr	r7, [pc, #48]
    mov	r5, r7
    add	r5, #98
    ldrb	r0, [r5, #0]
    mov	r2, r0
    mov	r3, r7
    cmp	r2, #7
    bne	MATASK_HTTP_GetPost+0xc4
    b	MATASK_HTTP_GetPost+0x30c
    cmp	r2, #7
    bgt	MATASK_HTTP_GetPost+0xf6
    cmp	r2, #3
    bne	MATASK_HTTP_GetPost+0xce
    b	MATASK_HTTP_GetPost+0x1d4
    cmp	r2, #3
    bgt	MATASK_HTTP_GetPost+0xe8
    cmp	r2, #1
    beq	MATASK_HTTP_GetPost+0x166
    cmp	r2, #1
    bgt	MATASK_HTTP_GetPost+0x180
    cmp	r2, #0
    beq	MATASK_HTTP_GetPost+0x14a
    bl	MATASK_HTTP_GetPost+0xb22
.align 2
    .word gMA

    cmp	r2, #5
    bne	MATASK_HTTP_GetPost+0xee
    b	MATASK_HTTP_GetPost+0x250
    cmp	r2, #5
    ble	MATASK_HTTP_GetPost+0xf4
    b	MATASK_HTTP_GetPost+0x26e
    b	MATASK_HTTP_GetPost+0x1f2
    cmp	r2, #100
    bne	MATASK_HTTP_GetPost+0xfc
    b	MATASK_HTTP_GetPost+0x894
    cmp	r2, #100
    bgt	MATASK_HTTP_GetPost+0x11e
    cmp	r2, #50
    bne	MATASK_HTTP_GetPost+0x106
    b	MATASK_HTTP_GetPost+0x858
    cmp	r2, #50
    bgt	MATASK_HTTP_GetPost+0x114
    cmp	r2, #8
    bne	MATASK_HTTP_GetPost+0x110
    b	MATASK_HTTP_GetPost+0x37c
    bl	MATASK_HTTP_GetPost+0xb22
    cmp	r2, #51
    bne	MATASK_HTTP_GetPost+0x11a
    b	MATASK_HTTP_GetPost+0x872
    bl	MATASK_HTTP_GetPost+0xb22
    cmp	r2, #240
    bne	MATASK_HTTP_GetPost+0x126
    bl	MATASK_HTTP_GetPost+0xa60
    cmp	r2, #240
    bgt	MATASK_HTTP_GetPost+0x136
    cmp	r2, #110
    bne	MATASK_HTTP_GetPost+0x132
    bl	MATASK_HTTP_GetPost+0x920
    bl	MATASK_HTTP_GetPost+0xb22
    cmp	r2, #241
    bne	MATASK_HTTP_GetPost+0x13e
    bl	MATASK_HTTP_GetPost+0xa82
    cmp	r2, #255
    bne	MATASK_HTTP_GetPost+0x146
    bl	MATASK_HTTP_GetPost+0xace
    bl	MATASK_HTTP_GetPost+0xb22
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r7, r1
    strh	r2, [r0, #0]
    mov	r1, r7
    add	r1, #212
    str	r1, [r0, #4]
    mov	r2, #220
    lsl	r2, r2, #2
    add	r1, r7, r2
    bl	MABIOS_DNSRequest
    bl	MATASK_HTTP_GetPost+0xa78
    mov	r0, r7
    add	r0, #106
    mov	r3, #242
    lsl	r3, r3, #1
    add	r1, r7, r3
    ldr	r1, [r1, #0]
    mov	r2, #4
    bl	MAU_memcpy
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    ldr	r2, [pc, #36]
    mov	r0, #0
    strh	r0, [r2, #0]
    ldr	r4, [pc, #36]
    add	r0, r2, r4
    str	r0, [r2, #4]
    mov	r1, #198
    lsl	r1, r1, #1
    add	r0, r2, r1
    ldr	r0, [r0, #0]
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x1b4
    ldr	r3, [pc, #20]
    add	r1, r2, r3
    mov	r0, r2
    mov	r2, #80
    bl	MABIOS_TCPConnect
    b	MATASK_HTTP_GetPost+0x1c0
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffe8a

    ldr	r4, [pc, #20]
    add	r1, r2, r4
    mov	r0, r2
    mov	r2, #80
    bl	MABIOS_TCPConnect
    ldr	r0, [pc, #12]
    add	r0, #98
    ldrb	r1, [r0, #0]
    add	r1, #1
    b	MATASK_HTTP_GetPost+0x88e
.align 2
    .word 0xfffffe8a
    .word gMA

    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r7, r1
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    mov	r1, r7
    add	r1, #99
    strb	r0, [r1, #0]
    add	r1, #105
    mov	r0, #1
    strb	r0, [r1, #0]
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    ldr	r4, [pc, #36]
    mov	r0, #0
    strh	r0, [r4, #0]
    ldr	r2, [pc, #32]
    add	r0, r4, r2
    str	r0, [r4, #4]
    bl	GetRequestType
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x228
    ldr	r1, [pc, #24]
    ldr	r3, [pc, #24]
    add	r0, r4, r3
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r2, #4
    bl	MABIOS_Data
    b	MATASK_HTTP_GetPost+0x238
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word strHttpGet
    .word 0xfffffe83

    ldr	r1, [pc, #24]
    ldr	r2, [pc, #28]
    add	r0, r4, r2
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r2, #5
    bl	MABIOS_Data
    ldr	r1, [pc, #16]
    mov	r2, r1
    add	r2, #200
    add	r1, #98
    b	MATASK_HTTP_GetPost+0x2ea
.align 2
    .word strHttpPost
    .word 0xfffffe83
    .word gMA

    mov	r1, r7
    add	r1, #140
    mov	r0, r7
    add	r0, #148
    ldr	r0, [r0, #0]
    str	r0, [r1, #0]
    add	r1, #4
    mov	r0, r7
    add	r0, #152
    ldr	r0, [r0, #0]
    str	r0, [r1, #0]
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    ldr	r4, [pc, #64]
    mov	r0, #0
    strh	r0, [r4, #0]
    ldr	r3, [pc, #60]
    add	r0, r4, r3
    str	r0, [r4, #4]
    ldr	r0, [pc, #60]
    add	r6, r4, r0
    ldr	r2, [r6, #0]
    cmp	r2, #254
    bls	MATASK_HTTP_GetPost+0x2cc
    ldr	r1, [pc, #52]
    add	r5, r4, r1
    ldr	r1, [r5, #0]
    ldr	r2, [pc, #52]
    add	r0, r4, r2
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r2, #254
    bl	MABIOS_Data
    ldr	r0, [r5, #0]
    add	r0, #254
    str	r0, [r5, #0]
    ldr	r0, [r6, #0]
    sub	r0, #254
    str	r0, [r6, #0]
    ldr	r3, [pc, #28]
    add	r2, r4, r3
    ldr	r0, [pc, #28]
    add	r1, r4, r0
    ldrb	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0x2ee
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffeb0
    .word 0xfffffeac
    .word 0xfffffe83
    .word 0xfffffee8
    .word 0xfffffe82

    ldr	r1, [pc, #44]
    add	r0, r4, r1
    ldr	r1, [r0, #0]
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    ldr	r3, [pc, #40]
    add	r0, r4, r3
    ldrb	r3, [r0, #0]
    mov	r0, r4
    bl	MABIOS_Data
    ldr	r0, [pc, #32]
    add	r2, r4, r0
    ldr	r3, [pc, #32]
    add	r1, r4, r3
    ldrb	r0, [r1, #0]
    add	r0, #1
    str	r0, [r2, #0]
    ldrb	r0, [r1, #0]
    mov	r0, #50
    strb	r0, [r1, #0]
    bl	MATASK_HTTP_GetPost+0xb22
.align 2
    .word 0xfffffeac
    .word 0xfffffe83
    .word 0xfffffee8
    .word 0xfffffe82

    mov	r4, #220
    lsl	r4, r4, #2
    add	r6, r7, r4
    ldr	r1, [pc, #92]
    mov	r0, r6
    bl	MAU_strcpy
    bl	CreateHttpRequestHeader
    bl	InitPrevBuf
    mov	r0, r7
    add	r0, #172
    mov	r4, #0
    str	r4, [r0, #0]
    add	r0, #4
    str	r4, [r0, #0]
    add	r0, #16
    str	r4, [r0, #0]
    ldr	r1, [pc, #64]
    ldr	r0, [pc, #64]
    str	r4, [r0, #0]
    str	r4, [r1, #0]
    mov	r0, #0
    bl	HttpGetNextStep
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    mov	r0, #240
    lsl	r0, r0, #1
    add	r5, r7, r0
    strh	r4, [r5, #0]
    mov	r0, r7
    add	r0, #212
    str	r0, [r5, #4]
    mov	r0, r6
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r7
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r5
    mov	r1, r6
    bl	MABIOS_Data
    b	MATASK_HTTP_GetPost+0xb22
.align 2
    .word strHttpReqSuffix
    .word curCp.272
    .word nextCp.273

    mov	r0, r7
    add	r0, #172
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bne	MATASK_HTTP_GetPost+0x3b4
    mov	r0, r7
    add	r0, #192
    ldr	r1, [r0, #0]
    cmp	r1, #0
    beq	MATASK_HTTP_GetPost+0x3ca
    mov	r0, r7
    add	r0, #176
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	MATASK_HTTP_GetPost+0x3d8
    mov	r0, #1
    bl	HttpGetNextStep
    mov	r2, r7
    add	r2, #98
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    mov	r1, r7
    add	r1, #164
    ldr	r0, [r1, #0]
    add	r0, #1
    str	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0xb22
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    cmp	r1, #0
    beq	MATASK_HTTP_GetPost+0x412
    mov	r0, r7
    add	r0, #192
    ldr	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_HTTP_GetPost+0x3e0
    mov	r2, r7
    add	r2, #102
    mov	r0, #50
    strb	r0, [r2, #0]
    mov	r0, r7
    add	r0, #104
    strh	r1, [r0, #0]
    ldrb	r0, [r5, #0]
    mov	r0, #240
    strb	r0, [r5, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r0, r7
    add	r0, #176
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bne	MATASK_HTTP_GetPost+0x3f6
    mov	r0, r7
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #240
    strb	r1, [r0, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r2, r7
    add	r2, #102
    mov	r1, #0
    mov	r0, #50
    strb	r0, [r2, #0]
    mov	r0, r7
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r7
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r1, #240
    lsl	r1, r1, #1
    add	r2, r7, r1
    ldrh	r0, [r2, #0]
    cmp	r0, #1
    bne	MATASK_HTTP_GetPost+0x420
    b	MATASK_HTTP_GetPost+0x7e4
    ldr	r6, [pc, #52]
    sub	r0, #1
    str	r0, [r6, #0]
    ldr	r1, [pc, #52]
    mov	r3, #242
    lsl	r3, r3, #1
    add	r0, r7, r3
    ldr	r0, [r0, #0]
    add	r0, #1
    str	r0, [r1, #0]
    ldr	r3, [pc, #40]
    ldr	r4, [pc, #44]
    add	r5, r7, r4
    str	r5, [r3, #0]
    ldr	r0, [pc, #40]
    add	r4, r7, r0
    ldrh	r0, [r4, #0]
    mov	r8, r1
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x44c
    add	r0, r0, r5
    str	r0, [r3, #0]
    mov	r9, r3
    mov	sl, r6
    mov	r6, r7
    add	r7, #102
    b	MATASK_HTTP_GetPost+0x7c8
.align 2
    .word tmpLen.275
    .word curCp.272
    .word lineCp.274
    .word 0x0000047d
    .word 0x000006fa

    mov	r2, r8
    ldr	r0, [r2, #0]
    sub	r0, r1, r0
    mov	r3, sl
    ldr	r1, [r3, #0]
    sub	r1, r1, r0
    str	r1, [r3, #0]
    ldr	r4, [pc, #172]
    ldr	r0, [pc, #172]
    add	r1, r4, r0
    ldrh	r0, [r1, #0]
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x492
    ldr	r2, [pc, #168]
    add	r0, r4, r2
    mov	r3, r9
    str	r0, [r3, #0]
    mov	r0, #0
    strh	r0, [r1, #0]
    mov	r1, r9
    ldr	r0, [r1, #0]
    ldr	r1, [pc, #156]
    mov	r2, #4
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x590
    mov	r2, r9
    ldr	r3, [r2, #0]
    ldrb	r0, [r3, #9]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bhi	MATASK_HTTP_GetPost+0x578
    ldrb	r0, [r3, #10]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bhi	MATASK_HTTP_GetPost+0x578
    ldrb	r0, [r3, #11]
    sub	r0, #48
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #9
    bhi	MATASK_HTTP_GetPost+0x578
    ldrb	r0, [r3, #9]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldrb	r1, [r3, #10]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r0, [pc, #84]
    add	r2, r2, r0
    ldrb	r3, [r3, #11]
    add	r2, r2, r3
    ldr	r1, [pc, #80]
    add	r0, r4, r1
    strh	r2, [r0, #0]
    mov	r0, r4
    add	r0, #192
    mov	r3, #1
    str	r3, [r0, #0]
    sub	r0, #32
    ldr	r0, [r0, #0]
    sub	r0, #1
    cmp	r0, #3
    bhi	MATASK_HTTP_GetPost+0x54c
    mov	r0, r4
    add	r0, #164
    ldr	r3, [r0, #0]
    cmp	r3, #0
    bne	MATASK_HTTP_GetPost+0x54c
    lsl	r0, r2, #16
    lsr	r1, r0, #16
    ldr	r0, [pc, #48]
    cmp	r1, r0
    bne	MATASK_HTTP_GetPost+0x516
    b	MATASK_HTTP_GetPost+0x730
    mov	r0, #50
    strb	r0, [r7, #0]
    cmp	r1, #200
    bne	MATASK_HTTP_GetPost+0x544
    mov	r0, r4
    add	r0, #104
    strh	r3, [r0, #0]
    b	MATASK_HTTP_GetPost+0x584
.align 2
    .word gMA
    .word 0x000006fa
    .word 0x0000047d
    .word hexChar.252+0x18
    .word 0x0000ffd0
    .word 0x00000702
    .word 0x00000191

    mov	r0, r4
    add	r0, #104
    strh	r2, [r0, #0]
    b	MATASK_HTTP_GetPost+0x584
    ldr	r2, [pc, #28]
    ldr	r1, [pc, #32]
    add	r0, r2, r1
    ldrh	r1, [r0, #0]
    cmp	r1, #200
    bne	MATASK_HTTP_GetPost+0x55a
    b	MATASK_HTTP_GetPost+0x730
    mov	r0, #50
    strb	r0, [r7, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r2, #1
    ldr	r3, [pc, #12]
    str	r2, [r3, #0]
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word gMA
    .word 0x00000702
    .word gMA+0xb0

    mov	r0, #0
    mov	r1, #50
    strb	r1, [r7, #0]
    mov	r1, r6
    add	r1, #104
    strh	r0, [r1, #0]
    mov	r4, #1
    ldr	r0, [pc, #4]
    str	r4, [r0, #0]
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word gMA+0xb0

    mov	r0, r4
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x668
    mov	r1, r9
    ldr	r0, [r1, #0]
    ldr	r1, [pc, #84]
    mov	r2, #6
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x5b8
    mov	r0, r4
    add	r0, #188
    ldr	r0, [r0, #0]
    mov	r2, #1
    and	r0, r2
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x5d6
    mov	r3, r9
    ldr	r0, [r3, #0]
    ldr	r1, [pc, #56]
    mov	r2, #10
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x668
    mov	r0, r4
    add	r0, #188
    ldr	r0, [r0, #0]
    mov	r1, #2
    and	r0, r1
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x668
    ldr	r4, [pc, #36]
    ldr	r0, [r4, #0]
    ldr	r1, [pc, #24]
    mov	r2, #6
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x600
    mov	r1, r6
    add	r1, #188
    ldr	r0, [r1, #0]
    mov	r4, #1
    orr	r0, r4
    str	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0x61a
.align 2
    .word strHttpDate
    .word strHttpLocation
    .word lineCp.274

    ldr	r0, [r4, #0]
    ldr	r1, [pc, #88]
    mov	r2, #10
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x61a
    mov	r0, r6
    add	r0, #188
    ldr	r1, [r0, #0]
    mov	r2, #2
    orr	r1, r2
    str	r1, [r0, #0]
    ldr	r1, [pc, #68]
    mov	r5, r1
    add	r5, #156
    ldr	r2, [r5, #0]
    cmp	r2, #0
    bne	MATASK_HTTP_GetPost+0x628
    b	MATASK_HTTP_GetPost+0x730
    mov	r4, r1
    add	r4, #168
    ldr	r0, [r4, #0]
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x634
    b	MATASK_HTTP_GetPost+0x730
    mov	r3, r9
    ldr	r1, [r3, #0]
    bl	MAU_strcpy_size
    ldr	r1, [pc, #36]
    str	r0, [r1, #0]
    ldr	r1, [r5, #0]
    sub	r2, r1, r0
    str	r2, [r5, #0]
    ldr	r1, [r4, #0]
    add	r1, r1, r0
    str	r1, [r4, #0]
    sub	r0, r1, #1
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x730
    cmp	r2, #0
    beq	MATASK_HTTP_GetPost+0x730
    strb	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word strHttpLocation
    .word gMA
    .word headerLineLen.277

    ldr	r4, [pc, #40]
    ldr	r0, [r4, #0]
    ldr	r1, [pc, #40]
    mov	r2, #29
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x6a0
    ldr	r0, [r4, #0]
    add	r0, #29
    mov	r1, r6
    add	r1, #180
    ldr	r1, [r1, #0]
    mov	r2, r6
    add	r2, #184
    ldr	r2, [r2, #0]
    ldr	r4, [pc, #16]
    add	r3, r6, r4
    bl	MA_MakeAuthorizationCode
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word lineCp.274
    .word strHttpAuthenticate
    .word 0x00000706

    ldr	r0, [r4, #0]
    ldr	r1, [pc, #72]
    mov	r2, #11
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x718
    ldr	r5, [pc, #64]
    ldr	r3, [r4, #0]
    ldrb	r0, [r3, #11]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldrb	r1, [r3, #12]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r0, [pc, #40]
    add	r2, r2, r0
    ldrb	r3, [r3, #13]
    add	r1, r2, r3
    ldr	r2, [pc, #36]
    add	r0, r5, r2
    strh	r1, [r0, #0]
    lsl	r0, r1, #16
    lsr	r0, r0, #16
    cmp	r0, #101
    bne	MATASK_HTTP_GetPost+0x6fc
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #9
    orr	r0, r1
    str	r0, [r5, #64]
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word strHttpGbStatus
    .word gMA
    .word 0x0000ffd0
    .word 0x00000704

    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x730
    mov	r0, #51
    strb	r0, [r7, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r3, #1
    ldr	r4, [pc, #4]
    str	r3, [r4, #0]
    b	MATASK_HTTP_GetPost+0x730
.align 2
    .word gMA+0xb0

    ldr	r0, [r4, #0]
    ldr	r1, [pc, #60]
    mov	r2, #12
    bl	MAU_strncmp
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x730
    ldr	r1, [r4, #0]
    add	r1, #12
    ldr	r0, [pc, #48]
    bl	MAU_strcpy
    ldr	r0, [pc, #44]
    ldr	r2, [r0, #0]
    ldrb	r3, [r2, #0]
    mov	r1, r0
    cmp	r3, #13
    bne	MATASK_HTTP_GetPost+0x742
    ldrb	r0, [r2, #1]
    cmp	r0, #10
    beq	MATASK_HTTP_GetPost+0x746
    cmp	r3, #10
    bne	MATASK_HTTP_GetPost+0x7bc
    ldr	r0, [r1, #0]
    ldrb	r0, [r0, #0]
    cmp	r0, #13
    bne	MATASK_HTTP_GetPost+0x764
    mov	r2, sl
    ldr	r0, [r2, #0]
    sub	r0, #2
    str	r0, [r2, #0]
    b	MATASK_HTTP_GetPost+0x76c
.align 2
    .word strHttpGbAuthID
    .word gMA+0x706
    .word nextCp.273

    mov	r3, sl
    ldr	r0, [r3, #0]
    sub	r0, #1
    str	r0, [r3, #0]
    ldr	r3, [pc, #40]
    mov	r0, r3
    add	r0, #172
    mov	r4, #1
    str	r4, [r0, #0]
    mov	r0, sl
    ldr	r2, [r0, #0]
    cmp	r2, #0
    beq	MATASK_HTTP_GetPost+0x7e4
    ldr	r1, [r1, #0]
    ldrb	r0, [r1, #0]
    cmp	r0, #13
    bne	MATASK_HTTP_GetPost+0x79c
    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r3, r4
    ldr	r0, [r0, #0]
    add	r0, #1
    add	r1, #2
    bl	MAU_memcpy
    b	MATASK_HTTP_GetPost+0x7ac
.align 2
    .word gMA

    mov	r4, #242
    lsl	r4, r4, #1
    add	r0, r3, r4
    ldr	r0, [r0, #0]
    add	r0, #1
    add	r1, #1
    bl	MAU_memcpy
    mov	r1, sl
    ldr	r0, [r1, #0]
    add	r0, #1
    ldr	r2, [pc, #4]
    strh	r0, [r2, #0]
    b	MATASK_HTTP_GetPost+0x7e4
.align 2
    .word gMA+0x1e0

    ldr	r1, [pc, #112]
    str	r2, [r1, #0]
    ldr	r0, [pc, #112]
    mov	r3, r9
    str	r0, [r3, #0]
    mov	r8, r1
    mov	r4, r9
    ldr	r0, [r4, #0]
    mov	r2, r8
    ldr	r1, [r2, #0]
    mov	r3, sl
    ldr	r2, [r3, #0]
    bl	MAU_strncpy_CRLF_LF
    mov	r1, r0
    ldr	r0, [pc, #92]
    str	r1, [r0, #0]
    cmp	r1, #0
    beq	MATASK_HTTP_GetPost+0x7e4
    b	MATASK_HTTP_GetPost+0x46c
    ldr	r4, [pc, #84]
    mov	r6, r4
    add	r6, #172
    ldr	r0, [r6, #0]
    cmp	r0, #0
    bne	MATASK_HTTP_GetPost+0x812
    ldr	r5, [pc, #76]
    ldr	r2, [r5, #0]
    cmp	r2, #0
    beq	MATASK_HTTP_GetPost+0x80c
    ldr	r1, [pc, #72]
    add	r0, r4, r1
    ldr	r1, [pc, #48]
    ldr	r1, [r1, #0]
    bl	MAU_memcpy
    ldr	r1, [r5, #0]
    ldr	r2, [pc, #64]
    add	r0, r4, r2
    strh	r1, [r0, #0]
    ldr	r0, [r6, #0]
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x81c
    ldr	r0, [pc, #44]
    ldr	r0, [r0, #0]
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0x81c
    b	MATASK_HTTP_GetPost+0xb22
    ldr	r0, [pc, #44]
    mov	r1, #0
    strh	r1, [r0, #0]
    ldr	r3, [pc, #44]
    add	r1, r0, r3
    str	r1, [r0, #4]
    ldr	r4, [pc, #40]
    add	r1, r0, r4
    b	MATASK_HTTP_GetPost+0xa46
.align 2
    .word curCp.272
    .word gMA+0x47d
    .word nextCp.273
    .word gMA
    .word tmpLen.275
    .word 0x0000047d
    .word 0x000006fa
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffe83

    mov	r2, r7
    add	r2, #196
    ldrb	r0, [r7, #5]
    lsl	r0, r0, #2
    mov	r1, r7
    add	r1, #52
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    str	r0, [r2, #0]
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    mov	r0, r3
    add	r0, #196
    ldr	r1, [r0, #0]
    sub	r1, #1
    str	r1, [r0, #0]
    mov	r0, #1
    neg	r0, r0
    cmp	r1, r0
    beq	MATASK_HTTP_GetPost+0x886
    b	MATASK_HTTP_GetPost+0xb22
    mov	r0, r3
    add	r0, #200
    ldr	r1, [r0, #0]
    sub	r0, #102
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r0, #240
    lsl	r0, r0, #1
    add	r6, r7, r0
    mov	r4, #0
    strh	r4, [r6, #0]
    mov	r0, r7
    add	r0, #212
    str	r0, [r6, #4]
    mov	r1, #136
    add	r1, r1, r7
    mov	r8, r1
    ldr	r2, [r1, #0]
    cmp	r2, #254
    bls	MATASK_HTTP_GetPost+0x8d8
    mov	r4, r7
    add	r4, #132
    ldr	r1, [r4, #0]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r6
    mov	r2, #254
    bl	MABIOS_Data
    ldr	r0, [r4, #0]
    add	r0, #254
    str	r0, [r4, #0]
    mov	r2, r8
    ldr	r0, [r2, #0]
    sub	r0, #254
    str	r0, [r2, #0]
    mov	r1, r7
    add	r1, #200
    ldrb	r0, [r5, #0]
    b	MATASK_HTTP_GetPost+0x90c
    mov	r0, r7
    add	r0, #132
    ldr	r1, [r0, #0]
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    sub	r0, #33
    ldrb	r3, [r0, #0]
    mov	r0, r6
    bl	MABIOS_Data
    mov	r0, r7
    add	r0, #172
    str	r4, [r0, #0]
    add	r0, #4
    str	r4, [r0, #0]
    add	r0, #16
    str	r4, [r0, #0]
    ldr	r1, [pc, #28]
    ldr	r0, [pc, #28]
    str	r4, [r0, #0]
    str	r4, [r1, #0]
    bl	InitPrevBuf
    mov	r1, r7
    add	r1, #200
    mov	r0, #8
    str	r0, [r1, #0]
    ldrb	r0, [r5, #0]
    mov	r0, #50
    strb	r0, [r5, #0]
    b	MATASK_HTTP_GetPost+0xb22
.align 2
    .word curCp.272
    .word nextCp.273

    ldr	r6, [r7, #64]
    mov	r0, #128
    lsl	r0, r0, #8
    and	r6, r0
    cmp	r6, #0
    bne	MATASK_HTTP_GetPost+0x9f8
    ldr	r5, [r7, #120]
    cmp	r5, #0
    beq	MATASK_HTTP_GetPost+0xa00
    mov	r3, #240
    lsl	r3, r3, #1
    add	r1, r7, r3
    ldrh	r0, [r1, #0]
    cmp	r0, #1
    bls	MATASK_HTTP_GetPost+0xa00
    ldr	r4, [pc, #68]
    mov	r8, r4
    sub	r2, r0, #1
    str	r2, [r4, #0]
    ldr	r3, [r7, #124]
    cmp	r3, r2
    bcc	MATASK_HTTP_GetPost+0x98c
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r7, r1
    ldr	r1, [r0, #0]
    add	r1, #1
    mov	r0, r5
    bl	MAU_memcpy
    ldr	r0, [r7, #120]
    ldr	r2, [r4, #0]
    add	r0, r0, r2
    str	r0, [r7, #120]
    ldr	r0, [r7, #124]
    sub	r0, r0, r2
    str	r0, [r7, #124]
    ldrh	r1, [r7, #2]
    ldr	r0, [pc, #24]
    and	r0, r1
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    mov	r0, r7
    add	r0, #128
    ldr	r1, [r0, #0]
    ldrh	r0, [r1, #0]
    add	r0, r0, r2
    strh	r0, [r1, #0]
    b	MATASK_HTTP_GetPost+0xa00
.align 2
    .word dataLen.276
    .word 0x0000fffb

    mov	r2, #242
    lsl	r2, r2, #1
    add	r4, r7, r2
    ldr	r1, [r4, #0]
    add	r1, #1
    mov	r0, r5
    mov	r2, r3
    bl	MAU_memcpy
    mov	r3, r8
    ldr	r0, [r3, #0]
    ldr	r2, [r7, #124]
    sub	r0, r0, r2
    ldr	r1, [pc, #72]
    add	r3, r7, r1
    mov	r5, #0
    strh	r0, [r3, #0]
    ldr	r1, [pc, #68]
    add	r0, r7, r1
    add	r2, #1
    ldr	r1, [r4, #0]
    add	r1, r1, r2
    ldrh	r2, [r3, #0]
    bl	MAU_memcpy
    mov	r0, r7
    add	r0, #128
    ldr	r2, [r0, #0]
    ldr	r1, [r7, #124]
    ldrh	r0, [r2, #0]
    add	r0, r0, r1
    strh	r0, [r2, #0]
    str	r6, [r7, #124]
    ldrh	r0, [r7, #2]
    mov	r1, #4
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r5
    strh	r0, [r7, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r0, [r7, #2]
    mov	r1, #1
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r5
    strh	r0, [r7, #2]
    b	MATASK_HTTP_GetPost+0xb22
.align 2
    .word 0x000006fa
    .word 0x0000047d

    ldr	r0, [r7, #64]
    ldr	r1, [pc, #48]
    and	r0, r1
    str	r0, [r7, #64]
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    cmp	r1, #0
    beq	MATASK_HTTP_GetPost+0xa38
    ldr	r4, [pc, #28]
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #28]
    and	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    mov	r0, #2
    bl	HttpGetNextStep
    add	r4, #98
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_HTTP_GetPost+0xb22
.align 2
    .word 0xffff7fff
    .word gMA
    .word 0x0000fffb

    ldr	r0, [pc, #24]
    strh	r1, [r0, #0]
    ldr	r2, [pc, #24]
    add	r1, r0, r2
    str	r1, [r0, #4]
    ldr	r3, [pc, #24]
    add	r1, r0, r3
    ldrb	r3, [r1, #0]
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_HTTP_GetPost+0xb22
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffe83

    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r7, r4
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r7
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r1, r7
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r5, #0
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r7, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r7, #2]
    mov	r4, #0
    strh	r0, [r7, #2]
    ldrh	r0, [r7, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r1, r2
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r4
    strh	r0, [r7, #2]
    mov	r0, r7
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r7
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r7
    add	r0, #204
    strb	r4, [r0, #0]
    sub	r0, #16
    str	r5, [r0, #0]
    b	MATASK_HTTP_GetPost+0xb22
    mov	r1, r7
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r5, #0
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r0, [r7, #2]
    and	r2, r0
    ldrh	r0, [r7, #2]
    mov	r4, #0
    strh	r2, [r7, #2]
    ldrh	r0, [r7, #2]
    mov	r3, #128
    lsl	r3, r3, #1
    mov	r1, r3
    orr	r0, r1
    ldrh	r1, [r7, #2]
    orr	r0, r4
    strh	r0, [r7, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r7
    add	r0, #204
    strb	r4, [r0, #0]
    sub	r0, #16
    str	r5, [r0, #0]
    ldr	r0, [r7, #64]
    mov	r1, #128
    lsl	r1, r1, #9
    and	r0, r1
    cmp	r0, #0
    beq	MATASK_HTTP_GetPost+0xb22
    mov	r0, #51
    mov	r1, #101
    bl	MA_SetApiError
    ldr	r0, [r7, #64]
    ldr	r1, [pc, #16]
    and	r0, r1
    str	r0, [r7, #64]
    pop	{r3, r4, r5}
    mov	r8, r3
    mov	r9, r4
    mov	sl, r5
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffeffff
.size MATASK_HTTP_GetPost, .-MATASK_HTTP_GetPost
");
#endif

static void CopyEEPROMString(char *dest, char *src, int size)
{
    static int i asm("i.281");
    for (i = 0; i < size; i++) if ((*dest++ = src[i]) == '\0') break;
    if (i == size) *dest = '\0';
}

static void CopyEEPROMData(int unk_1, char *dest)
{
    switch (unk_1) {  // MAGIC
    case 0x18:
        MAU_DecodeEEPROMTelNo(gMA.eeprom_telno, dest);
        CopyEEPROMString(&dest[EEPROM_TELNO_SIZE * 2 + 1],
                gMA.eeprom_unk_1275, sizeof(gMA.eeprom_unk_1275));
        break;

    case 0x19:
        CopyEEPROMString(dest,
                gMA.eeprom_unk_1161, sizeof(gMA.eeprom_unk_1161));
        break;

    case 0x1a:
        CopyEEPROMString(dest,
                gMA.eeprom_unk_1193, sizeof(gMA.eeprom_unk_1193));
        break;
    }
}

static void MATASK_GetEEPROMData(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x18:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        case 0x19:
            gMA.unk_102 = 0x14;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
        gMA.task_unk_98++;
        break;

    case 3:
        if (!EEPROMRegistrationCheck(&gMA.unk_212[1])) {
            gMA.unk_102 = 0x25;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;
        }
        MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_unk_98++;
        break;

    case 4:
        MAU_memcpy(&gMA.eeprom_unk_1275[2], &gMA.buffer_unk_480.data[1], 0x40);
        if (!EEPROMSumCheck(gMA.prevbuf)) {
            gMA.unk_102 = 0x14;
            gMA.unk_104 = 0;
            gMA.task_unk_98 = 0xfa;
            break;
        }
        gMA.unk_101 = 1;
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 5:
        CopyEEPROMData((int)gMA.unk_112, (char *)gMA.unk_116);
        gMA.task_unk_98++;
        break;

    case 6:
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

        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_94);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_EEPROMRead(u8 *unk_1)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_1C)) {
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = unk_1;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1C, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_EEPROM_Read(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x18:
        case 0x19:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
        gMA.task_unk_98++;
        break;

    case 3:
        MAU_memcpy(gMA.unk_112, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_unk_98++;
        break;

    case 4:
        MAU_memcpy(&gMA.unk_112[0x80], &gMA.buffer_unk_480.data[1], 0x40);
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 5:
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

        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_EEPROMWrite(u8 *unk_1)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_1D)) {
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = unk_1;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1D, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_EEPROM_Write(void)
{
    if (gMA.recv_cmd == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.unk_80) {
        case 0x11:
            break;

        case 0x18:
        case 0x1a:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_unk_98 = 0xfb;
            break;
        }
    }

    switch (gMA.task_unk_98) {
    case 0:
        MABIOS_Start();
        gMA.task_unk_98++;
        break;

    case 1:
        gMA.task_unk_98++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Write(&gMA.buffer_unk_480, 0, gMA.unk_112, 0x80);
        gMA.task_unk_98++;
        break;

    case 3:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Write(&gMA.buffer_unk_480, 0x80, &gMA.unk_112[0x80], 0x40);
        gMA.task_unk_98++;
        break;

    case 4:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 5:
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

        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_unk_98++;
        break;

    case 0xfb:
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

        MA_SetApiError(gMA.unk_102, gMA.unk_104);
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

void MA_GetTel(MA_TELDATA *pTelData)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_18)) {
        ResetApiCallFlag();
        return;
    }

    MAU_memset(pTelData, 36, 0);  // MAGIC
    if (gMA.unk_101 == 1) {
        CopyEEPROMData(TASK_UNK_18, (char *)pTelData);
        ResetApiCallFlag();
        gMA.condition &= ~MA_CONDITION_APIWAIT;
        return;
    }

    gMA.unk_112 = (u8 *)TASK_UNK_18;
    gMA.unk_116 = (u32)pTelData;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1B, 0);
    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

void MA_GetUserID(char *pUserIDBuf)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_19)) {
        ResetApiCallFlag();
        return;
    }

    if (gMA.unk_101 == 1) {
        CopyEEPROMData(TASK_UNK_19, pUserIDBuf);
        ResetApiCallFlag();
        gMA.condition &= ~MA_CONDITION_APIWAIT;
        return;
    }

    MAU_memset(pUserIDBuf, 33, 0);  // MAGIC
    gMA.unk_112 = (u8 *)TASK_UNK_19;
    gMA.unk_116 = (u32)pUserIDBuf;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1B, 0);
    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

void MA_GetMailID(char *pBufPtr)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_1A)) {
        ResetApiCallFlag();
        return;
    }

    MAU_memset(pBufPtr, 31, 0);  // MAGIC
    if (gMA.unk_101 == 1) {
        CopyEEPROMData(TASK_UNK_1A, pBufPtr);
        ResetApiCallFlag();
        gMA.condition &= ~MA_CONDITION_APIWAIT;
        return;
    }

    gMA.unk_112 = (u8 *)TASK_UNK_1A;
    gMA.unk_116 = (u32)pBufPtr;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1B, 0);
    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

void MA_GetSMTPServerName(char *dest)
{
    CopyEEPROMString(dest, gMA.smtp_server, sizeof(gMA.smtp_server));
}

void MA_GetPOP3ServerName(char *dest)
{
    CopyEEPROMString(dest, gMA.pop3_server, sizeof(gMA.pop3_server));
}

extern void (*taskProcTable[])(void);
asm("
.section .rodata
.align 2
.type taskProcTable, object
taskProcTable:
    .word 0x00000000
    .word MATASK_InitLibrary
    .word MATASK_InitLibrary
    .word MATASK_TelServer
    .word MATASK_Tel
    .word MATASK_Receive
    .word 0x00000000
    .word 0x00000000
    .word MATASK_Condition
    .word MATASK_Condition
    .word MATASK_Offline
    .word MATASK_SMTP_Connect
    .word MATASK_SMTP_Sender
    .word MATASK_SMTP_Send
    .word MATASK_SMTP_POP3_Quit
    .word MATASK_POP3_Connect
    .word MATASK_POP3_Stat
    .word MATASK_POP3_List
    .word MATASK_POP3_Retr
    .word MATASK_POP3_Dele
    .word MATASK_POP3_Head
    .word MATASK_SMTP_POP3_Quit
    .word MATASK_HTTP_GetPost
    .word MATASK_HTTP_GetPost
    .word 0x00000000
    .word 0x00000000
    .word 0x00000000
    .word MATASK_GetEEPROMData
    .word MATASK_EEPROM_Read
    .word MATASK_EEPROM_Write
    .word MATASK_Stop
    .word MATASK_TCP_Cut
    .word MATASK_TCP_Connect
    .word MATASK_TCP_Disconnect
    .word MATASK_TCP_SendRecv
    .word MATASK_GetHostAddress
    .word 0x00000000
.size taskProcTable, .-taskProcTable
.section .text
");

void MAAPI_Main(void)
{
    if (gMA.task != TASK_UNK_01 &&
            gMA.task != TASK_UNK_02 &&
            gMA.task != TASK_UNK_03 &&
            gMA.task != TASK_UNK_04 &&
            gMA.task != TASK_UNK_08 &&
            gMA.task != TASK_UNK_09 &&
            gMA.task != TASK_UNK_05 &&
            gMA.task != TASK_UNK_1B &&
            gMA.task != TASK_UNK_1C &&
            gMA.task != TASK_UNK_1D &&
            gMA.task != TASK_UNK_1E &&
            !(gMA.status & STATUS_UNK_0)) {
        return;
    }

    if (gMA.status & STATUS_UNK_2 ||
            gMA.condition & MA_CONDITION_ERROR ||
            gMA.condition & MA_CONDITION_UNK_5) {
        return;
    }

    if (gMA.status & STATUS_UNK_9 && gMA.cmd_cur == MACMD_DATA) {
        MATASK_P2P();
    }

    if (gMA.task) taskProcTable[gMA.task]();
}

u16 MAAPI_GetConditionFlag(void)
{
    return gMA.condition;
}

static u16 ErrDetailHexConv(u16 err)
{
    u16 hex;

    hex = 0;
    while (err >= 1000) {
        err -= 1000;
        hex += 0x1000;
    }
    while (err >= 100) {
        err -= 100;
        hex += 0x100;
    }
    while (err >= 10) {
        err -= 10;
        hex += 0x10;
    }
    hex += err;

    return hex;
}

u8 MAAPI_ErrorCheck(u16 *pProtocolError)
{
    if (!(gMA.condition & MA_CONDITION_ERROR)) {
        *pProtocolError = 0;
        return 0;
    }

    switch (gMA.error) {
    case MAAPIE_UNK_83:
        gMA.error = MAAPIE_SYSTEM;
        gMA.unk_94 = 0;  // MAGIC
        break;

    case MAAPIE_UNK_84:
        gMA.error = MAAPIE_SYSTEM;
        gMA.unk_94 = 1;  // MAGIC
        break;

    case MAAPIE_UNK_87:
        gMA.error = MAAPIE_SYSTEM;
        gMA.unk_94 = 2;  // MAGIC
        break;

    case MAAPIE_UNK_85:
        gMA.error = MAAPIE_SYSTEM;
        gMA.unk_94 = 3;  // MAGIC
        break;
    }

    gMA.condition &= ~MA_CONDITION_ERROR;

    if (pProtocolError != NULL) {
        switch (gMA.error) {
        case MAAPIE_SYSTEM:
        case MAAPIE_SMTP:
        case MAAPIE_POP3:
        case MAAPIE_HTTP:
        case MAAPIE_GB_CENTER:
            *pProtocolError = ErrDetailHexConv(gMA.unk_94);
            break;

        default:
            *pProtocolError = 0;
            break;
        }
    }

    if (!(gMA.condition & MA_CONDITION_UNK_5)) {
        gMA.cmd_cur = 0;
        gMA.recv_cmd = 0;
    }
    return gMA.error;
}
