#include "ma_api.h"
#include "libma.h"

#include <stddef.h>
#include "ma_bios.h"
#include "ma_var.h"
#include "ma_sub.h"

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

static void MA_SetApiError(u8 unk_1, u16 unk_2)
{
    gMA.unk_96 = 0;
    gMA.unk_94 = unk_2;
    MA_SetError(unk_1);
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

static void MakeEndLineBuffer(u8 *unk_1, int size)
{
    gMA.unk_1788[5] = 0;
    if (size == 0) return;
    if (unk_1 == NULL) return;

    if (size >= 5) {
        MAU_memcpy(gMA.unk_1788, unk_1, 5);
    } else {
        MAU_memcpy(gMA.unk_1788, gMA.unk_1788 + size, 5 - size);
        MAU_memcpy(&gMA.unk_1788[5 - size], unk_1, size);
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

asm("
.section .rodata
.align 2
    .asciz \"QUIT\\r\\n\"
.align 2
    .asciz \"HELO \"
.align 2
    .asciz \"\\r\\n\"
.align 2
    .asciz \"MAIL FROM:<\"
.align 2
    .asciz \">\\r\\n\"
.align 2
    .asciz \"RCPT TO:<\"
.align 2
    .asciz \"DATA\\r\\n\"
.align 2
    .asciz \"USER \"
.align 2
    .asciz \"PASS \"
.align 2
    .asciz \"STAT\\r\\n\"
.align 2
    .asciz \"LIST \"
.align 2
    .asciz \"RETR \"
.align 2
    .asciz \"DELE \"
.align 2
    .asciz \"TOP \"
.align 2
    .asciz \" 0\\r\\n\"
.align 2
.section .text
");

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_TCP_Cut:
    push	{r4, lr}
    ldr	r3, [pc, #28]
    mov	r0, #98
    add	r0, r0, r3
    mov	ip, r0
    ldrb	r0, [r0, #0]
    mov	r2, r0
    mov	r4, r3
    cmp	r2, #1
    beq	MATASK_TCP_Cut+0x3a
    cmp	r2, #1
    bgt	MATASK_TCP_Cut+0x24
    cmp	r2, #0
    beq	MATASK_TCP_Cut+0x2e
    b	MATASK_TCP_Cut+0xec
.align 2
    .word gMA

    cmp	r2, #2
    beq	MATASK_TCP_Cut+0x88
    cmp	r2, #3
    beq	MATASK_TCP_Cut+0xb0
    b	MATASK_TCP_Cut+0xec
    mov	r1, ip
    ldrb	r0, [r1, #0]
    add	r0, #1
    ldrb	r1, [r1, #0]
    mov	r2, ip
    strb	r0, [r2, #0]
    ldr	r0, [r4, #112]
    cmp	r0, #35
    bne	MATASK_TCP_Cut+0x66
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    beq	MATASK_TCP_Cut+0x66
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    mov	r1, r4
    add	r1, #99
    strb	r0, [r1, #0]
    sub	r1, #1
    ldrb	r0, [r1, #0]
    add	r0, #1
    ldrb	r2, [r1, #0]
    strb	r0, [r1, #0]
    b	MATASK_TCP_Cut+0x88
    mov	r0, r4
    add	r0, #204
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    beq	MATASK_TCP_Cut+0x7c
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #3
    strb	r1, [r0, #0]
    b	MATASK_TCP_Cut+0xec
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    add	r1, #1
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    ldr	r4, [pc, #24]
    ldr	r2, [pc, #28]
    add	r0, r4, r2
    ldrb	r1, [r0, #0]
    mov	r0, r4
    bl	MABIOS_TCPDisconnect
    ldr	r0, [pc, #20]
    add	r2, r4, r0
    ldrb	r0, [r2, #0]
    add	r0, #1
    ldrb	r1, [r2, #0]
    strb	r0, [r2, #0]
    b	MATASK_TCP_Cut+0xec
.align 2
    .word gMA+0x1e0
    .word 0xfffffe83
    .word 0xfffffe82

    mov	r0, r3
    add	r0, #92
    ldrb	r1, [r0, #0]
    strb	r2, [r0, #0]
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
    sub	r0, #135
    ldrb	r1, [r0, #0]
    strb	r2, [r0, #0]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4}
    pop	{r0}
    bx	r0
.size MATASK_TCP_Cut, .-MATASK_TCP_Cut
");
#endif

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_InitLibrary:
    push	{r4, r5, lr}
    ldr	r4, [pc, #60]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_InitLibrary+0x26
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #17
    beq	MATASK_InitLibrary+0x26
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #24]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #2
    beq	MATASK_InitLibrary+0x66
    cmp	r0, #2
    bgt	MATASK_InitLibrary+0x44
    cmp	r0, #0
    beq	MATASK_InitLibrary+0x58
    cmp	r0, #1
    beq	MATASK_InitLibrary+0x5e
    b	MATASK_InitLibrary+0x1e6
.align 2
    .word gMA

    cmp	r0, #250
    beq	MATASK_InitLibrary+0x134
    cmp	r0, #250
    bgt	MATASK_InitLibrary+0x52
    cmp	r0, #3
    beq	MATASK_InitLibrary+0x7c
    b	MATASK_InitLibrary+0x1e6
    cmp	r0, #251
    beq	MATASK_InitLibrary+0x142
    b	MATASK_InitLibrary+0x1e6
    bl	MABIOS_Start
    b	MATASK_InitLibrary+0x138
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    bl	MABIOS_End
    ldr	r0, [pc, #12]
    add	r0, #98
    ldrb	r1, [r0, #0]
    add	r1, #1
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    b	MATASK_InitLibrary+0x1e6
.align 2
    .word gMA

    ldr	r1, [r5, #112]
    ldrb	r0, [r5, #6]
    add	r0, #120
    strb	r0, [r1, #0]
    mov	r0, #0
    bl	MA_ChangeSIOMode
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
    ldr	r1, [pc, #96]
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
    ldr	r0, [pc, #68]
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
    b	MATASK_InitLibrary+0x1e6
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_InitLibrary+0x1e6
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
    ldr	r1, [pc, #116]
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
    ldr	r0, [pc, #88]
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
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_InitLibrary, .-MATASK_InitLibrary
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_TCP_Connect
MA_TCP_Connect:
    push	{r4, r5, r6, lr}
    mov	r6, r0
    mov	r4, r1
    lsl	r2, r2, #16
    lsr	r5, r2, #16
    bl	SetApiCallFlag
    mov	r0, #32
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_TCP_Connect+0x1e
    bl	ResetApiCallFlag
    b	MA_TCP_Connect+0x70
    bl	MAU_Socket_FreeCheck
    cmp	r0, #0
    bne	MA_TCP_Connect+0x2a
    mov	r0, #33
    b	MA_TCP_Connect+0x50
    bl	MAU_Socket_GetNum
    cmp	r0, #0
    bne	MA_TCP_Connect+0x44
    ldr	r0, [pc, #12]
    mov	r1, r4
    mov	r2, #4
    bl	MAU_memcpy
    b	MA_TCP_Connect+0x5c
.align 2
    .word gMA+0x6a

    mov	r0, r4
    bl	MAU_Socket_IpAddrCheck
    cmp	r0, #0
    bne	MA_TCP_Connect+0x5c
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_TCP_Connect+0x70
    ldr	r0, [pc, #24]
    str	r6, [r0, #112]
    str	r4, [r0, #116]
    str	r5, [r0, #120]
    mov	r0, #32
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
.size MA_TCP_Connect, .-MA_TCP_Connect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_TCP_Connect:
    push	{r4, lr}
    ldr	r2, [pc, #48]
    mov	r0, r2
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_TCP_Connect+0x3c
    mov	r0, r2
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #35
    bne	MATASK_TCP_Connect+0x38
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
    b	MATASK_TCP_Connect+0x3c
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r3, [pc, #20]
    mov	r4, r3
    add	r4, #98
    ldrb	r0, [r4, #0]
    mov	r1, r0
    cmp	r1, #1
    beq	MATASK_TCP_Connect+0x82
    cmp	r1, #1
    bgt	MATASK_TCP_Connect+0x58
    cmp	r1, #0
    beq	MATASK_TCP_Connect+0x5e
    b	MATASK_TCP_Connect+0xdc
.align 2
    .word gMA

    cmp	r1, #240
    beq	MATASK_TCP_Connect+0xa2
    b	MATASK_TCP_Connect+0xdc
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r3, r2
    strh	r1, [r0, #0]
    mov	r1, r3
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r3, #116]
    ldr	r2, [r3, #120]
    lsl	r2, r2, #16
    lsr	r2, r2, #16
    bl	MABIOS_TCPConnect
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_TCP_Connect+0xdc
    ldr	r2, [r3, #112]
    mov	r0, #242
    lsl	r0, r0, #1
    add	r1, r3, r0
    ldr	r0, [r1, #0]
    ldrb	r0, [r0, #0]
    strb	r0, [r2, #0]
    ldr	r0, [r1, #0]
    ldrb	r0, [r0, #0]
    bl	MAU_Socket_Add
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_TCP_Connect+0xdc
    mov	r1, r3
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r3, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r3, #2]
    strh	r0, [r3, #2]
    ldrh	r1, [r3, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r0, r2
    ldrh	r2, [r3, #2]
    orr	r0, r1
    strh	r0, [r3, #2]
    mov	r0, r3
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r3
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4}
    pop	{r0}
    bx	r0
.size MATASK_TCP_Connect, .-MATASK_TCP_Connect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_TCP_Disconnect
MA_TCP_Disconnect:
    push	{r4, r5, lr}
    lsl	r0, r0, #24
    lsr	r4, r0, #24
    mov	r5, r4
    bl	SetApiCallFlag
    mov	r0, #33
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_TCP_Disconnect+0x1c
    bl	ResetApiCallFlag
    b	MA_TCP_Disconnect+0x50
    bl	MAU_Socket_GetNum
    cmp	r0, #0
    bne	MA_TCP_Disconnect+0x28
    mov	r0, #33
    b	MA_TCP_Disconnect+0x34
    mov	r0, r4
    bl	MAU_Socket_Search
    cmp	r0, #0
    bne	MA_TCP_Disconnect+0x40
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_TCP_Disconnect+0x50
    ldr	r0, [pc, #20]
    str	r5, [r0, #112]
    mov	r0, #33
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
.size MA_TCP_Disconnect, .-MA_TCP_Disconnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_TCP_Disconnect:
    push	{r4, r5, lr}
    ldr	r1, [pc, #44]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_TCP_Disconnect+0x1c
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #36
    beq	MATASK_TCP_Disconnect+0x1c
    bl	MA_DefaultNegaResProc
    ldr	r4, [pc, #16]
    mov	r5, r4
    add	r5, #98
    ldrb	r0, [r5, #0]
    cmp	r0, #0
    beq	MATASK_TCP_Disconnect+0x34
    cmp	r0, #1
    beq	MATASK_TCP_Disconnect+0x4e
    b	MATASK_TCP_Disconnect+0x90
.align 2
    .word gMA

    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r4, r1
    ldr	r1, [r4, #112]
    lsl	r1, r1, #24
    lsr	r1, r1, #24
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r5, #0]
    add	r0, #1
    ldrb	r1, [r5, #0]
    strb	r0, [r5, #0]
    b	MATASK_TCP_Disconnect+0x90
    ldr	r0, [r4, #112]
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    bl	MAU_Socket_Delete
    mov	r1, r4
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r4, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r4, #2]
    mov	r2, #0
    strh	r0, [r4, #2]
    ldrh	r0, [r4, #2]
    mov	r3, #128
    lsl	r3, r3, #1
    mov	r1, r3
    orr	r0, r1
    ldrh	r1, [r4, #2]
    orr	r0, r2
    strh	r0, [r4, #2]
    mov	r0, r4
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.size MATASK_TCP_Disconnect, .-MATASK_TCP_Disconnect
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
.global MA_TCP_SendRecv
MA_TCP_SendRecv:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r8, r1
    mov	r9, r3
    lsl	r0, r0, #24
    lsr	r4, r0, #24
    mov	r7, r4
    lsl	r2, r2, #24
    lsr	r5, r2, #24
    mov	r6, r5
    bl	SetApiCallFlag
    mov	r0, #34
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_TCP_SendRecv+0x2c
    bl	ResetApiCallFlag
    b	MA_TCP_SendRecv+0x6e
    bl	MAU_Socket_GetNum
    cmp	r0, #0
    bne	MA_TCP_SendRecv+0x38
    mov	r0, #33
    b	MA_TCP_SendRecv+0x48
    mov	r0, r4
    bl	MAU_Socket_Search
    cmp	r0, #0
    beq	MA_TCP_SendRecv+0x46
    cmp	r5, #254
    bls	MA_TCP_SendRecv+0x54
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_TCP_SendRecv+0x6e
    ldr	r0, [pc, #36]
    str	r7, [r0, #112]
    mov	r1, r8
    str	r1, [r0, #116]
    str	r6, [r0, #120]
    mov	r1, r9
    str	r1, [r0, #124]
    mov	r0, #34
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
.size MA_TCP_SendRecv, .-MA_TCP_SendRecv
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_TCP_SendRecv:
    push	{r4, r5, lr}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_TCP_SendRecv+0x3c
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_TCP_SendRecv+0x38
    bl	MA_DefaultNegaResProc
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    ldr	r0, [r4, #112]
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    bl	MAU_Socket_Delete
    b	MATASK_TCP_SendRecv+0x3c
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    bl	MA_GetCondition
    mov	r1, #64
    and	r1, r0
    cmp	r1, #0
    beq	MATASK_TCP_SendRecv+0x6e
    ldr	r2, [pc, #56]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #35
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    ldr	r0, [r2, #112]
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    bl	MAU_Socket_Delete
    ldr	r5, [pc, #20]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #1
    beq	MATASK_TCP_SendRecv+0xb0
    cmp	r0, #1
    bgt	MATASK_TCP_SendRecv+0x88
    cmp	r0, #0
    beq	MATASK_TCP_SendRecv+0x8e
    b	MATASK_TCP_SendRecv+0xf2
.align 2
    .word gMA

    cmp	r0, #240
    beq	MATASK_TCP_SendRecv+0xda
    b	MATASK_TCP_SendRecv+0xf2
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r1, [r5, #116]
    ldr	r2, [r5, #120]
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    ldr	r3, [r5, #112]
    lsl	r3, r3, #24
    lsr	r3, r3, #24
    bl	MABIOS_Data
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_TCP_SendRecv+0xf2
    ldr	r0, [r5, #116]
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    sub	r2, #4
    add	r4, r5, r2
    ldrh	r2, [r4, #0]
    sub	r2, #1
    bl	MAU_memcpy
    ldr	r1, [r5, #124]
    ldrb	r0, [r4, #0]
    sub	r0, #1
    strb	r0, [r1, #0]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_TCP_SendRecv+0xf2
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
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.size MATASK_TCP_SendRecv, .-MATASK_TCP_SendRecv
");
#endif

void MA_GetHostAddress(u8 *param_1, char *param_2)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_23)) {
        ResetApiCallFlag();
        return;
    }

    if (MAU_strlen(param_2) >= 0x100) {  // MAGIC
        MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = param_1;
    gMA.unk_116 = (u32)param_2;
    MA_TaskSet(TASK_UNK_23, 0);
    ResetApiCallFlag();
}

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_GetHostAddress:
    push	{r4, lr}
    ldr	r2, [pc, #48]
    mov	r0, r2
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_GetHostAddress+0x3c
    mov	r0, r2
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #40
    bne	MATASK_GetHostAddress+0x38
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
    b	MATASK_GetHostAddress+0x3c
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r2, [pc, #20]
    mov	r4, r2
    add	r4, #98
    ldrb	r0, [r4, #0]
    mov	r1, r0
    cmp	r1, #1
    beq	MATASK_GetHostAddress+0x7c
    cmp	r1, #1
    bgt	MATASK_GetHostAddress+0x58
    cmp	r1, #0
    beq	MATASK_GetHostAddress+0x5e
    b	MATASK_GetHostAddress+0xae
.align 2
    .word gMA
    cmp	r1, #240
    beq	MATASK_GetHostAddress+0x96
    b	MATASK_GetHostAddress+0xae
    mov	r3, #240
    lsl	r3, r3, #1
    add	r0, r2, r3
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r2, #116]
    bl	MABIOS_DNSRequest
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_GetHostAddress+0xae
    ldr	r0, [r2, #112]
    mov	r3, #242
    lsl	r3, r3, #1
    add	r1, r2, r3
    ldr	r1, [r1, #0]
    mov	r2, #4
    bl	MAU_memcpy
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_GetHostAddress+0xae
    mov	r0, r2
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r2
    add	r1, #104
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4}
    pop	{r0}
    bx	r0
.size MATASK_GetHostAddress, .-MATASK_GetHostAddress
");
#endif

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

#if 0
#else
asm("
.lcomm tmp.89, 0x2
.lcomm sum.90, 0x2
.lcomm i.91, 0x4

.align 2
.thumb_func
EEPROMSumCheck:
    push	{r4, r5, r6, r7, lr}
    mov	r5, r0
    ldr	r1, [pc, #68]
    mov	r0, #0
    strh	r0, [r1, #0]
    ldr	r2, [pc, #68]
    mov	r0, #0
    str	r0, [r2, #0]
    mov	r6, r1
    ldr	r7, [pc, #64]
    mov	r4, r6
    mov	r3, r2
    ldr	r0, [r3, #0]
    add	r2, r5, r0
    ldrh	r1, [r4, #0]
    ldrb	r2, [r2, #0]
    add	r1, r1, r2
    strh	r1, [r4, #0]
    add	r0, #1
    str	r0, [r3, #0]
    cmp	r0, #189
    ble	EEPROMSumCheck+0x18
    mov	r0, r5
    add	r0, #190
    ldrb	r0, [r0, #0]
    lsl	r0, r0, #8
    mov	r1, r5
    add	r1, #191
    ldrb	r1, [r1, #0]
    add	r1, r1, r0
    strh	r1, [r7, #0]
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    ldrh	r6, [r6, #0]
    cmp	r1, r6
    beq	EEPROMSumCheck+0x58
    mov	r0, #0
    b	EEPROMSumCheck+0x5a
.align 2
    .word sum.90
    .word i.91
    .word tmp.89

    mov	r0, #1
    pop	{r4, r5, r6, r7}
    pop	{r1}
    bx	r1
.size EEPROMSumCheck, .-EEPROMSumCheck
");
#endif

static int EEPROMRegistrationCheck(u8 *data)
{
    if (data[0] == 'M' && data[1] == 'A') {
        return TRUE;
    } else {
        return FALSE;
    }
}

#if 0
#else
void MA_TelServer(const char *pTelNo, const char *pUserID, const char *pPassword);
asm("
.align 2
.thumb_func
.global MA_TelServer
MA_TelServer:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    mov	r5, r0
    mov	r7, r1
    mov	r8, r2
    bl	SetApiCallFlag
    mov	r0, #3
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_TelServer+0x20
    bl	ResetApiCallFlag
    b	MA_TelServer+0xa8
    mov	r0, r5
    bl	MAU_strlen
    mov	r6, r0
    mov	r0, r7
    bl	MAU_strlen
    mov	r4, r0
    mov	r0, r8
    bl	MAU_strlen
    cmp	r6, #20
    bgt	MA_TelServer+0x58
    cmp	r4, #32
    bgt	MA_TelServer+0x58
    cmp	r0, #16
    bgt	MA_TelServer+0x58
    cmp	r6, #0
    beq	MA_TelServer+0x58
    cmp	r4, #0
    beq	MA_TelServer+0x58
    cmp	r0, #0
    beq	MA_TelServer+0x58
    mov	r0, r5
    bl	MAU_IsValidTelNoStr
    cmp	r0, #0
    bne	MA_TelServer+0x66
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_TelServer+0xa8
    ldr	r4, [pc, #76]
    str	r5, [r4, #112]
    str	r7, [r4, #116]
    mov	r0, r8
    str	r0, [r4, #120]
    ldr	r5, [pc, #68]
    mov	r0, #0
    str	r0, [r5, #0]
    mov	r0, #3
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r1, [r4, #2]
    mov	r0, #1
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    bl	ResetApiCallFlag
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    and	r0, r1
    cmp	r0, #0
    beq	MA_TelServer+0xa4
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #16
    and	r0, r1
    cmp	r0, #0
    bne	MA_TelServer+0xa8
    bl	MAAPI_Main
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word 0x0400010c
.size MA_TelServer, .-MA_TelServer
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_TelServer:
    push	{r4, r5, r6, r7, lr}
    sub	sp, #4
    ldr	r1, [pc, #32]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_TelServer+0xde
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    sub	r0, #17
    cmp	r0, #16
    bhi	MATASK_TelServer+0xd0
    lsl	r0, r0, #2
    ldr	r1, [pc, #12]
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    mov	pc, r0
.align 2
    .word gMA
    .word .L_MATASK_TelServer.0x30
.L_MATASK_TelServer.0x30:
    .word .L_MATASK_TelServer.0x30+0xae
    .word .L_MATASK_TelServer.0x30+0x44
    .word .L_MATASK_TelServer.0x30+0xae
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0x44
    .word .L_MATASK_TelServer.0x30+0x44
    .word .L_MATASK_TelServer.0x30+0x58
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0xa0
    .word .L_MATASK_TelServer.0x30+0x7c

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #8]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_TelServer+0xdc
.align 2
    .word gMA

    ldr	r2, [pc, #28]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #20
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #250
    strb	r0, [r1, #0]
    b	MATASK_TelServer+0xde
.align 2
    .word gMA

    ldr	r2, [pc, #28]
    mov	r3, r2
    add	r3, #102
    mov	r1, #0
    mov	r0, #34
    strb	r0, [r3, #0]
    mov	r0, r2
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r2
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #249
    strb	r0, [r1, #0]
    b	MATASK_TelServer+0xde
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #44]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #36]
    mov	r6, r5
    add	r6, #98
    ldrb	r0, [r6, #0]
    cmp	r0, #6
    bne	MATASK_TelServer+0xec
    b	MATASK_TelServer+0x242
    cmp	r0, #6
    bgt	MATASK_TelServer+0x128
    cmp	r0, #2
    beq	MATASK_TelServer+0x170
    cmp	r0, #2
    bgt	MATASK_TelServer+0x108
    cmp	r0, #0
    beq	MATASK_TelServer+0x15a
    cmp	r0, #1
    beq	MATASK_TelServer+0x160
    b	MATASK_TelServer+0x3ae
.align 2
    .word gMA

    cmp	r0, #4
    beq	MATASK_TelServer+0x1b0
    cmp	r0, #4
    bgt	MATASK_TelServer+0x1f8
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    cmp	r0, #255
    bne	MATASK_TelServer+0x186
    mov	r2, r5
    add	r2, #102
    mov	r1, #0
    mov	r0, #17
    b	MATASK_TelServer+0x220
    cmp	r0, #9
    bne	MATASK_TelServer+0x12e
    b	MATASK_TelServer+0x2a6
    cmp	r0, #9
    bgt	MATASK_TelServer+0x140
    cmp	r0, #7
    bne	MATASK_TelServer+0x138
    b	MATASK_TelServer+0x26c
    cmp	r0, #8
    bne	MATASK_TelServer+0x13e
    b	MATASK_TelServer+0x27e
    b	MATASK_TelServer+0x3ae
    cmp	r0, #250
    bne	MATASK_TelServer+0x146
    b	MATASK_TelServer+0x2fc
    cmp	r0, #250
    bgt	MATASK_TelServer+0x152
    cmp	r0, #249
    bne	MATASK_TelServer+0x150
    b	MATASK_TelServer+0x2f6
    b	MATASK_TelServer+0x3ae
    cmp	r0, #251
    bne	MATASK_TelServer+0x158
    b	MATASK_TelServer+0x30a
    b	MATASK_TelServer+0x3ae
    bl	MABIOS_Start
    b	MATASK_TelServer+0x300
    ldrb	r0, [r6, #0]
    add	r0, #1
    ldrb	r1, [r6, #0]
    strb	r0, [r6, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_TelServer+0x3ae
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r5, r2
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #84
    str	r1, [r0, #4]
    bl	MABIOS_CheckStatus
    b	MATASK_TelServer+0x300
    mov	r0, r5
    add	r0, #101
    ldrb	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_TelServer+0x1a8
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r5, r4
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #0
    mov	r2, #128
    bl	MABIOS_EEPROM_Read
    b	MATASK_TelServer+0x300
    ldrb	r0, [r6, #0]
    mov	r0, #6
    strb	r0, [r6, #0]
    b	MATASK_TelServer+0x3ae
    mov	r0, r5
    add	r0, #213
    bl	EEPROMRegistrationCheck
    mov	r1, r0
    cmp	r1, #0
    bne	MATASK_TelServer+0x1c6
    mov	r2, r5
    add	r2, #102
    mov	r0, #37
    b	MATASK_TelServer+0x220
    ldr	r7, [pc, #44]
    add	r0, r5, r7
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #128
    bl	MAU_memcpy
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r5, r4
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #128
    mov	r2, #64
    bl	MABIOS_EEPROM_Read
    b	MATASK_TelServer+0x300
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    ldr	r7, [pc, #52]
    add	r0, r5, r7
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #64
    bl	MAU_memcpy
    ldr	r4, [pc, #36]
    add	r0, r5, r4
    bl	EEPROMSumCheck
    mov	r1, r0
    cmp	r1, #0
    bne	MATASK_TelServer+0x238
    mov	r2, r5
    add	r2, #102
    mov	r0, #20
    strb	r0, [r2, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    ldrb	r0, [r6, #0]
    mov	r0, #250
    strb	r0, [r6, #0]
    b	MATASK_TelServer+0x3ae
    lsl	r5, r7, #19
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    mov	r1, r5
    add	r1, #101
    mov	r0, #1
    strb	r0, [r1, #0]
    b	MATASK_TelServer+0x300
    mov	r7, #207
    lsl	r7, r7, #2
    add	r0, r5, r7
    ldr	r2, [pc, #24]
    add	r1, r5, r2
    mov	r2, #8
    bl	MAU_memcpy
    mov	r4, #209
    lsl	r4, r4, #2
    add	r0, r5, r4
    ldr	r7, [pc, #12]
    add	r1, r5, r7
    mov	r2, #44
    bl	MAU_memcpy
    b	MATASK_TelServer+0x300
    lsl	r1, r0, #18
    lsl	r0, r0, #0
    lsl	r7, r0, #19
    lsl	r0, r0, #0
    ldrb	r0, [r5, #6]
    bl	MA_GetCallTypeFromHarwareType
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    ldr	r1, [r5, #112]
    bl	MABIOS_Tel
    b	MATASK_TelServer+0x300
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r1, [r5, #116]
    ldr	r2, [r5, #120]
    mov	r4, #207
    lsl	r4, r4, #2
    add	r3, r5, r4
    mov	r7, #208
    lsl	r7, r7, #2
    add	r4, r5, r7
    str	r4, [sp, #0]
    bl	MABIOS_PPPConnect
    b	MATASK_TelServer+0x300
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r1, [r0, #0]
    ldrb	r0, [r1, #0]
    mov	r2, r5
    add	r2, #206
    strb	r0, [r2, #0]
    ldrb	r0, [r1, #1]
    add	r2, #1
    strb	r0, [r2, #0]
    ldrb	r2, [r1, #2]
    mov	r0, r5
    add	r0, #208
    strb	r2, [r0, #0]
    ldrb	r0, [r1, #3]
    mov	r1, r5
    add	r1, #209
    strb	r0, [r1, #0]
    sub	r1, #117
    ldrb	r0, [r1, #0]
    mov	r0, #3
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r2, #128
    lsl	r2, r2, #1
    mov	r0, r2
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_TelServer+0x3ae
    bl	MABIOS_Offline
    b	MATASK_TelServer+0x300
    bl	MABIOS_End
    ldrb	r0, [r6, #0]
    add	r0, #1
    ldrb	r1, [r6, #0]
    strb	r0, [r6, #0]
    b	MATASK_TelServer+0x3ae
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
    ldr	r1, [pc, #120]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #116]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #112]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #96]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #92]
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
    add	sp, #4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_TelServer, .-MATASK_TelServer
");
#endif

#if 0
#else
void MA_Tel(const char *pTelNo);
asm("
.align 2
.thumb_func
.global MA_Tel
MA_Tel:
    push	{r4, lr}
    mov	r4, r0
    bl	SetApiCallFlag
    mov	r0, #4
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_Tel+0x18
    bl	ResetApiCallFlag
    b	MA_Tel+0x66
    mov	r0, r4
    bl	MAU_strlen
    cmp	r0, #20
    bgt	MA_Tel+0x26
    cmp	r0, #0
    bne	MA_Tel+0x34
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_Tel+0x66
    ldr	r0, [pc, #52]
    str	r4, [r0, #112]
    ldr	r4, [pc, #52]
    mov	r0, #0
    str	r0, [r4, #0]
    mov	r0, #4
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    ldr	r0, [r4, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    and	r0, r1
    cmp	r0, #0
    beq	MA_Tel+0x62
    ldr	r0, [r4, #0]
    mov	r1, #128
    lsl	r1, r1, #16
    and	r0, r1
    cmp	r0, #0
    bne	MA_Tel+0x66
    bl	MAAPI_Main
    pop	{r4}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word 0x0400010c
.size MA_Tel, .-MA_Tel
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_Tel:
    push	{r4, r5, lr}
    ldr	r1, [pc, #32]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_Tel+0x52
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #18
    beq	MATASK_Tel+0x30
    cmp	r0, #18
    bgt	MATASK_Tel+0x28
    cmp	r0, #17
    beq	MATASK_Tel+0x52
    b	MATASK_Tel+0x44
.align 2
    .word gMA

    cmp	r0, #25
    bgt	MATASK_Tel+0x44
    cmp	r0, #23
    blt	MATASK_Tel+0x44
    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #8]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_Tel+0x50
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #36]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #3
    beq	MATASK_Tel+0xb4
    cmp	r0, #3
    bgt	MATASK_Tel+0x74
    cmp	r0, #1
    beq	MATASK_Tel+0x8e
    cmp	r0, #1
    bgt	MATASK_Tel+0x9e
    cmp	r0, #0
    beq	MATASK_Tel+0x88
    b	MATASK_Tel+0x1de
.align 2
    .word gMA

    cmp	r0, #250
    beq	MATASK_Tel+0x12c
    cmp	r0, #250
    bgt	MATASK_Tel+0x82
    cmp	r0, #4
    beq	MATASK_Tel+0xec
    b	MATASK_Tel+0x1de
    cmp	r0, #251
    beq	MATASK_Tel+0x13a
    b	MATASK_Tel+0x1de
    bl	MABIOS_Start
    b	MATASK_Tel+0x130
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_Tel+0x1de
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #84
    str	r1, [r0, #4]
    bl	MABIOS_CheckStatus
    b	MATASK_Tel+0x130
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    cmp	r0, #255
    bne	MATASK_Tel+0xda
    mov	r2, r5
    add	r2, #102
    mov	r1, #0
    mov	r0, #17
    strb	r0, [r2, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    ldrb	r0, [r4, #0]
    mov	r0, #250
    strb	r0, [r4, #0]
    b	MATASK_Tel+0x1de
    ldrb	r0, [r5, #6]
    bl	MA_GetCallTypeFromHarwareType
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    ldr	r1, [r5, #112]
    bl	MABIOS_Tel
    b	MATASK_Tel+0x130
    ldr	r0, [r5, #64]
    mov	r2, #128
    lsl	r2, r2, #2
    orr	r0, r2
    str	r0, [r5, #64]
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r3, #0
    mov	r0, #7
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    orr	r2, r0
    strh	r2, [r5, #2]
    str	r3, [r5, #112]
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    strh	r3, [r0, #0]
    bl	InitPrevBuf
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_Tel+0x1de
    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_Tel+0x1de
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
    ldr	r1, [pc, #116]
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
    ldr	r0, [pc, #88]
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
    mov	r0, r5
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r5
    add	r1, #94
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_Tel, .-MATASK_Tel
");
#endif

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_Receive:
    push	{r4, r5, lr}
    ldr	r4, [pc, #32]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_Receive+0x64
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #20
    beq	MATASK_Receive+0x3e
    cmp	r0, #20
    bgt	MATASK_Receive+0x28
    cmp	r0, #17
    beq	MATASK_Receive+0x64
    b	MATASK_Receive+0x56
.align 2
    .word gMA

    cmp	r0, #25
    bgt	MATASK_Receive+0x56
    cmp	r0, #24
    blt	MATASK_Receive+0x56
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_Receive+0x62
    mov	r0, r4
    add	r0, #81
    ldrb	r0, [r0, #0]
    cmp	r0, #0
    beq	MATASK_Receive+0x64
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_Receive+0x62
    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #40]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #3
    beq	MATASK_Receive+0xb8
    cmp	r0, #3
    bgt	MATASK_Receive+0x88
    cmp	r0, #1
    beq	MATASK_Receive+0xa2
    cmp	r0, #1
    bgt	MATASK_Receive+0xb2
    cmp	r0, #0
    beq	MATASK_Receive+0x9c
    b	MATASK_Receive+0x1c0
.align 2
    .word gMA

    cmp	r0, #250
    beq	MATASK_Receive+0x10e
    cmp	r0, #250
    bgt	MATASK_Receive+0x96
    cmp	r0, #4
    beq	MATASK_Receive+0xc8
    b	MATASK_Receive+0x1c0
    cmp	r0, #251
    beq	MATASK_Receive+0x11c
    b	MATASK_Receive+0x1c0
    bl	MABIOS_Start
    b	MATASK_Receive+0x112
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_Receive+0x1c0
    bl	MABIOS_WaitCall
    b	MATASK_Receive+0x112
    mov	r0, r5
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #148
    beq	MATASK_Receive+0x112
    ldrb	r0, [r4, #0]
    sub	r0, #1
    b	MATASK_Receive+0x116
    ldr	r0, [r5, #64]
    mov	r1, #128
    lsl	r1, r1, #2
    orr	r0, r1
    str	r0, [r5, #64]
    mov	r1, r5
    add	r1, #92
    ldrb	r0, [r1, #0]
    mov	r2, #0
    mov	r0, #8
    strb	r0, [r1, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    mov	r3, #192
    lsl	r3, r3, #2
    mov	r0, r3
    ldrh	r3, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    str	r2, [r5, #112]
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    strh	r2, [r0, #0]
    bl	InitPrevBuf
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_Receive+0x1c0
    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_Receive+0x1c0
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
    ldr	r1, [pc, #116]
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
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #96]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #88]
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
    mov	r0, r5
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r5
    add	r1, #94
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_Receive, .-MATASK_Receive
");
#endif

#if 0
#else
void MA_SData(const u8 *pSendData, u8 sendSize, u8 *pResult);
asm("
.align 2
.thumb_func
.global MA_SData
MA_SData:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r0
    mov	r5, r2
    lsl	r1, r1, #24
    lsr	r4, r1, #24
    mov	r6, r4
    bl	SetApiCallFlag
    mov	r0, #0
    strb	r0, [r5, #0]
    mov	r0, #6
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_SData+0x24
    bl	ResetApiCallFlag
    b	MA_SData+0x76
    sub	r0, r4, #1
    lsl	r0, r0, #24
    cmp	r0, #0
    bge	MA_SData+0x3a
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_SData+0x76
    ldr	r2, [pc, #32]
    ldrh	r1, [r2, #2]
    mov	r0, #8
    and	r0, r1
    cmp	r0, #0
    beq	MA_SData+0x64
    mov	r0, #1
    strb	r0, [r5, #0]
    ldrh	r1, [r2, #2]
    ldr	r0, [pc, #16]
    and	r0, r1
    ldrh	r1, [r2, #2]
    strh	r0, [r2, #2]
    bl	ResetApiCallFlag
    b	MA_SData+0x76
.align 2
    .word gMA
    .word 0x0000fffe

    str	r7, [r2, #112]
    str	r6, [r2, #116]
    ldr	r0, [r2, #64]
    mov	r1, #128
    lsl	r1, r1, #6
    orr	r0, r1
    str	r0, [r2, #64]
    bl	ResetApiCallFlag
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MA_SData, .-MA_SData
");
#endif

#if 0
#else
void MA_GData(u8 *pRecvData, u8 *pRecvSize);
asm("
.align 2
.thumb_func
.global MA_GData
MA_GData:
    push	{r4, r5, r6, r7, lr}
    mov	r4, r0
    mov	r7, r1
    bl	SetApiCallFlag
    mov	r0, #7
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_GData+0x1a
    bl	ResetApiCallFlag
    b	MA_GData+0xe0
    ldr	r6, [pc, #24]
    ldrh	r1, [r6, #2]
    mov	r0, #8
    and	r0, r1
    cmp	r0, #0
    bne	MA_GData+0x38
    mov	r0, #33
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_GData+0xe0
.align 2
    .word gMA

    ldr	r0, [pc, #124]
    add	r1, r6, r0
    sub	r0, #1
    add	r5, r6, r0
    ldrb	r2, [r5, #0]
    mov	r0, r4
    bl	MAU_memcpy
    ldrb	r0, [r5, #0]
    strb	r0, [r7, #0]
    ldrb	r0, [r5, #0]
    add	r0, #1
    lsl	r0, r0, #24
    lsr	r4, r0, #24
    mov	r3, #0
    ldr	r0, [pc, #100]
    add	r1, r6, r0
    ldrh	r0, [r1, #0]
    sub	r0, r0, r4
    cmp	r3, r0
    bge	MA_GData+0x78
    mov	r6, r1
    add	r2, r4, r5
    add	r1, r3, r5
    ldrb	r0, [r2, #0]
    strb	r0, [r1, #0]
    add	r2, #1
    add	r3, #1
    ldrh	r0, [r6, #0]
    sub	r0, r0, r4
    cmp	r3, r0
    blt	MA_GData+0x66
    ldr	r1, [pc, #68]
    ldr	r0, [pc, #64]
    add	r5, r1, r0
    ldrh	r0, [r5, #0]
    sub	r0, r0, r4
    strh	r0, [r5, #0]
    lsl	r0, r0, #16
    mov	r2, r1
    cmp	r0, #0
    beq	MA_GData+0xc8
    ldr	r0, [pc, #52]
    add	r3, r2, r0
    ldrb	r0, [r3, #0]
    sub	r0, #1
    lsl	r0, r0, #24
    cmp	r0, #0
    bge	MA_GData+0x9e
    mov	r0, #128
    strb	r0, [r3, #0]
    ldrh	r1, [r5, #0]
    ldrb	r0, [r3, #0]
    add	r0, #1
    cmp	r1, r0
    blt	MA_GData+0xc8
    ldrh	r0, [r2, #2]
    mov	r1, #8
    orr	r0, r1
    ldrh	r1, [r2, #2]
    mov	r1, #0
    orr	r0, r1
    strh	r0, [r2, #2]
    b	MA_GData+0xd2
.align 2
    .word 0x0000047e
    .word 0x000006fa
    .word gMA
    .word 0x0000047d

    ldrh	r0, [r2, #2]
    ldr	r1, [pc, #28]
    and	r1, r0
    ldrh	r0, [r2, #2]
    strh	r1, [r2, #2]
    ldrh	r0, [r2, #2]
    ldr	r1, [pc, #20]
    and	r1, r0
    ldrh	r0, [r2, #2]
    strh	r1, [r2, #2]
    bl	ResetApiCallFlag
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0000fff7
    .word 0x0000fffe
.size MA_GData, .-MA_GData
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_P2P:
    push	{r4, r5, r6, r7, lr}
    ldr	r2, [pc, #224]
    ldr	r0, [r2, #64]
    mov	r1, #128
    lsl	r1, r1, #3
    and	r0, r1
    mov	r5, r2
    cmp	r0, #0
    beq	MATASK_P2P+0x2c
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #208]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #204]
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #200]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    mov	r0, r5
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_P2P+0x100
    mov	r0, r5
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_P2P+0x100
    mov	r0, #0
    bl	MA_ChangeSIOMode
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
    ldr	r1, [pc, #120]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #100]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #96]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #88]
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
    mov	r0, #35
    mov	r1, #0
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_P2P+0x154
.align 2
    .word gMA
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fffe
    .word 0xfffffdff
    .word 0x0000fff7
    .word 0x0000ffef

    mov	r6, r5
    mov	r0, #240
    lsl	r0, r0, #1
    add	r7, r6, r0
    ldrh	r1, [r7, #0]
    cmp	r1, #1
    bls	MATASK_P2P+0x154
    add	r0, #4
    add	r4, r6, r0
    ldr	r0, [r4, #0]
    add	r0, #1
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    mov	r0, #0
    str	r0, [r4, #0]
    strh	r0, [r7, #0]
    ldr	r1, [pc, #52]
    add	r4, r6, r1
    ldrb	r0, [r4, #0]
    sub	r0, #1
    lsl	r0, r0, #24
    cmp	r0, #0
    bge	MATASK_P2P+0x138
    mov	r0, #128
    strb	r0, [r4, #0]
    ldr	r1, [pc, #36]
    add	r0, r5, r1
    ldrh	r1, [r0, #0]
    ldrb	r0, [r4, #0]
    add	r0, #1
    cmp	r1, r0
    blt	MATASK_P2P+0x154
    ldrh	r0, [r5, #2]
    mov	r1, #8
    orr	r0, r1
    ldrh	r1, [r5, #2]
    mov	r1, #0
    orr	r0, r1
    strh	r0, [r5, #2]
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0000047d
    .word 0x000006fa
.size MATASK_P2P, .-MATASK_P2P
");
#endif

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_Condition:
    push	{r4, r5, lr}
    ldr	r4, [pc, #44]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_Condition+0x4c
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #17
    beq	MATASK_Condition+0x4c
    cmp	r0, #23
    bne	MATASK_Condition+0x3e
    bl	MA_DefaultNegaResProc
    ldr	r0, [r4, #116]
    cmp	r0, #1
    bne	MATASK_Condition+0x34
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_Condition+0x4a
.align 2
    .word gMA

    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #252
    b	MATASK_Condition+0x4a
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #3
    bne	MATASK_Condition+0x5a
    b	MATASK_Condition+0x184
    cmp	r0, #3
    bgt	MATASK_Condition+0x70
    cmp	r0, #1
    beq	MATASK_Condition+0x94
    cmp	r0, #1
    bgt	MATASK_Condition+0xaa
    cmp	r0, #0
    beq	MATASK_Condition+0x8e
    b	MATASK_Condition+0x264
.align 2
    .word gMA

    cmp	r0, #250
    bne	MATASK_Condition+0x76
    b	MATASK_Condition+0x184
    cmp	r0, #250
    bgt	MATASK_Condition+0x80
    cmp	r0, #4
    beq	MATASK_Condition+0xe0
    b	MATASK_Condition+0x264
    cmp	r0, #251
    bne	MATASK_Condition+0x86
    b	MATASK_Condition+0x192
    cmp	r0, #252
    bne	MATASK_Condition+0x8c
    b	MATASK_Condition+0x24c
    b	MATASK_Condition+0x264
    bl	MABIOS_Start
    b	MATASK_Condition+0x188
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #84
    str	r1, [r0, #4]
    bl	MABIOS_CheckStatus
    b	MATASK_Condition+0x188
    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r5, r1
    ldr	r0, [r0, #0]
    ldrb	r0, [r0, #0]
    bl	MA_ProcessCheckStatusResponse
    ldr	r1, [r5, #112]
    strb	r0, [r1, #0]
    mov	r0, r5
    add	r0, #86
    ldrb	r0, [r0, #0]
    cmp	r0, #239
    bls	MATASK_Condition+0xd0
    ldr	r0, [r5, #112]
    ldrb	r1, [r0, #0]
    mov	r2, #128
    orr	r1, r2
    strb	r1, [r0, #0]
    ldr	r0, [r5, #116]
    cmp	r0, #1
    beq	MATASK_Condition+0xd8
    b	MATASK_Condition+0x22e
    ldrb	r0, [r4, #0]
    mov	r0, #3
    strb	r0, [r4, #0]
    b	MATASK_Condition+0x264
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
    ldr	r1, [pc, #88]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #84]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    ldr	r1, [pc, #80]
    and	r0, r1
    str	r0, [r5, #64]
    ldr	r0, [r5, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r5, #64]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #68]
    and	r0, r1
    ldrh	r1, [r5, #2]
    strh	r0, [r5, #2]
    ldrh	r1, [r5, #2]
    ldr	r0, [pc, #60]
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
    b	MATASK_Condition+0x22e
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_Condition+0x264
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
    ldr	r0, [pc, #88]
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
    b	MATASK_Condition+0x264
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

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
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.size MATASK_Condition, .-MATASK_Condition
");
#endif

#if 0
#else
void MA_Offline(void);
asm("
.align 2
.thumb_func
.global MA_Offline
MA_Offline:
    push	{r4, lr}
    bl	SetApiCallFlag
    ldr	r0, [pc, #28]
    mov	r4, r0
    add	r4, #92
    ldrb	r0, [r4, #0]
    cmp	r0, #0
    beq	MA_Offline+0x1c
    mov	r0, #10
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_Offline+0x28
    bl	ResetApiCallFlag
    b	MA_Offline+0x60
.align 2
    .word gMA

    ldrb	r0, [r4, #0]
    cmp	r0, #7
    beq	MA_Offline+0x34
    ldrb	r0, [r4, #0]
    cmp	r0, #8
    bne	MA_Offline+0x3e
    mov	r0, #10
    mov	r1, #1
    bl	MA_TaskSet
    b	MA_Offline+0x5c
    ldrb	r0, [r4, #0]
    cmp	r0, #4
    beq	MA_Offline+0x4a
    ldrb	r0, [r4, #0]
    cmp	r0, #5
    bne	MA_Offline+0x54
    mov	r0, #10
    mov	r1, #100
    bl	MA_TaskSet
    b	MA_Offline+0x5c
    mov	r0, #10
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4}
    pop	{r0}
    bx	r0
.size MA_Offline, .-MA_Offline
");
#endif

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_Offline:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r1, [pc, #60]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    mov	r3, r1
    cmp	r0, #238
    bne	MATASK_Offline+0x26
    mov	r0, r3
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_Offline+0x26
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #0
    strb	r0, [r1, #0]
    mov	r7, r3
    mov	r6, r7
    add	r6, #98
    ldrb	r0, [r6, #0]
    cmp	r0, #3
    beq	MATASK_Offline+0x9c
    cmp	r0, #3
    bgt	MATASK_Offline+0x48
    cmp	r0, #1
    beq	MATASK_Offline+0x90
    cmp	r0, #1
    bgt	MATASK_Offline+0x96
    cmp	r0, #0
    beq	MATASK_Offline+0x68
    b	MATASK_Offline+0x25c
.align 2
    .word gMA

    cmp	r0, #101
    bne	MATASK_Offline+0x4e
    b	MATASK_Offline+0x1a4
    cmp	r0, #101
    bgt	MATASK_Offline+0x5a
    cmp	r0, #100
    bne	MATASK_Offline+0x58
    b	MATASK_Offline+0x160
    b	MATASK_Offline+0x25c
    cmp	r0, #102
    bne	MATASK_Offline+0x60
    b	MATASK_Offline+0x204
    cmp	r0, #103
    bne	MATASK_Offline+0x66
    b	MATASK_Offline+0x228
    b	MATASK_Offline+0x25c
    mov	r1, r7
    add	r1, #92
    ldrb	r0, [r1, #0]
    cmp	r0, #7
    beq	MATASK_Offline+0x7c
    ldrb	r0, [r1, #0]
    cmp	r0, #8
    beq	MATASK_Offline+0x7c
    bl	MABIOS_PPPDisconnect
    ldr	r0, [pc, #12]
    add	r0, #98
    ldrb	r1, [r0, #0]
    add	r1, #1
    ldrb	r2, [r0, #0]
    strb	r1, [r0, #0]
    b	MATASK_Offline+0x25c
.align 2
    .word gMA

    bl	MABIOS_Offline
    b	MATASK_Offline+0x21e
    bl	MABIOS_End
    b	MATASK_Offline+0x21e
    ldrh	r1, [r7, #2]
    ldr	r4, [pc, #172]
    mov	r0, r4
    and	r0, r1
    ldrh	r1, [r7, #2]
    mov	r5, #0
    mov	r1, #0
    mov	r8, r1
    strh	r0, [r7, #2]
    ldr	r0, [r7, #64]
    ldr	r6, [pc, #156]
    and	r0, r6
    str	r0, [r7, #64]
    mov	r0, r7
    add	r0, #92
    ldrb	r1, [r0, #0]
    strb	r5, [r0, #0]
    mov	r0, #0
    bl	MA_ChangeSIOMode
    ldrb	r0, [r7, #5]
    lsl	r0, r0, #1
    mov	r1, r7
    add	r1, #8
    add	r0, r0, r1
    ldrh	r0, [r0, #0]
    ldrh	r1, [r7, #12]
    strh	r0, [r7, #12]
    mov	r2, r8
    str	r2, [r7, #60]
    ldrb	r0, [r7, #4]
    strb	r5, [r7, #4]
    ldr	r0, [r7, #64]
    mov	r1, #2
    neg	r1, r1
    and	r0, r1
    str	r0, [r7, #64]
    ldr	r0, [r7, #64]
    and	r0, r6
    str	r0, [r7, #64]
    ldr	r0, [r7, #64]
    ldr	r1, [pc, #100]
    and	r0, r1
    str	r0, [r7, #64]
    ldr	r0, [r7, #64]
    ldr	r1, [pc, #96]
    and	r0, r1
    str	r0, [r7, #64]
    ldr	r0, [r7, #64]
    mov	r1, #5
    neg	r1, r1
    and	r0, r1
    str	r0, [r7, #64]
    ldrh	r0, [r7, #2]
    and	r4, r0
    ldrh	r0, [r7, #2]
    strh	r4, [r7, #2]
    ldrh	r1, [r7, #2]
    ldr	r0, [pc, #72]
    and	r0, r1
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    ldrh	r1, [r7, #2]
    mov	r4, #255
    mov	r0, r4
    and	r0, r1
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    ldrh	r0, [r7, #2]
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    bl	MAU_Socket_Clear
    ldrh	r0, [r7, #2]
    and	r4, r0
    ldrh	r0, [r7, #2]
    strh	r4, [r7, #2]
    ldrh	r0, [r7, #2]
    ldrh	r1, [r7, #2]
    strh	r0, [r7, #2]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, #0
    bl	MA_ChangeSIOMode
    b	MATASK_Offline+0x25c
.align 2
    .word 0x0000fff7
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000ffef

    bl	InitPrevBuf
    mov	r4, #220
    lsl	r4, r4, #2
    add	r5, r7, r4
    ldr	r1, [pc, #52]
    mov	r0, r5
    bl	MAU_strcpy
    mov	r0, #240
    lsl	r0, r0, #1
    add	r4, r7, r0
    mov	r0, #0
    strh	r0, [r4, #0]
    mov	r0, r7
    add	r0, #212
    str	r0, [r4, #4]
    mov	r0, r5
    bl	MAU_strlen
    mov	r2, r0
    lsl	r2, r2, #24
    lsr	r2, r2, #24
    mov	r0, r7
    add	r0, #99
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, r5
    bl	MABIOS_Data
    b	MATASK_Offline+0x21e
.align 2
    .word strEndMultiLine.25+0x8

    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r7, r1
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r2, #240
    lsl	r2, r2, #1
    add	r4, r7, r2
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r1, [pc, #40]
    add	r0, r7, r1
    ldr	r2, [pc, #40]
    add	r1, r7, r2
    ldrh	r1, [r1, #0]
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_Offline+0x1f4
    strh	r0, [r4, #0]
    mov	r0, r7
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_Offline+0x25c
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    ldrb	r0, [r7, #5]
    lsl	r0, r0, #2
    mov	r1, r7
    add	r1, #44
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    str	r0, [r7, #112]
    b	MATASK_Offline+0x21e
    ldr	r0, [r7, #112]
    sub	r0, #1
    str	r0, [r7, #112]
    cmp	r0, #0
    bne	MATASK_Offline+0x25c
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r7, r4
    mov	r1, r7
    add	r1, #99
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    ldrb	r0, [r6, #0]
    add	r0, #1
    ldrb	r1, [r6, #0]
    strb	r0, [r6, #0]
    b	MATASK_Offline+0x25c
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
    ldrb	r0, [r6, #0]
    strb	r2, [r6, #0]
    pop	{r3}
    mov	r8, r3
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.size MATASK_Offline, .-MATASK_Offline
");
#endif

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

#if 0
#else
void MA_SMTP_Connect(const char *pMailAddress);
asm("
.align 2
.thumb_func
.global MA_SMTP_Connect
MA_SMTP_Connect:
    push	{r4, r5, lr}
    mov	r5, r0
    bl	SetApiCallFlag
    mov	r0, #11
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_SMTP_Connect+0x18
    bl	ResetApiCallFlag
    b	MA_SMTP_Connect+0x4e
    mov	r0, r5
    bl	MAU_strlen
    cmp	r0, #30
    bgt	MA_SMTP_Connect+0x26
    cmp	r0, #0
    bne	MA_SMTP_Connect+0x34
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_SMTP_Connect+0x4e
    ldr	r4, [pc, #28]
    mov	r0, r4
    bl	MA_GetSMTPServerName
    ldr	r0, [pc, #24]
    add	r4, r4, r0
    str	r5, [r4, #112]
    mov	r0, #11
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA+0x370
    .word 0xfffffc90
.size MA_SMTP_Connect, .-MA_SMTP_Connect
");
#endif

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
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
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

#if 0
#else
void MA_SMTP_Sender(const char * const pRecipients[]);
asm("
.align 2
.thumb_func
.global MA_SMTP_Sender
MA_SMTP_Sender:
    push	{r4, r5, r6, lr}
    mov	r6, r0
    bl	SetApiCallFlag
    mov	r0, #12
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_SMTP_Sender+0x18
    bl	ResetApiCallFlag
    b	MA_SMTP_Sender+0x7a
    ldr	r1, [r6, #0]
    cmp	r1, #0
    beq	MA_SMTP_Sender+0x52
    ldr	r0, [r6, #4]
    cmp	r0, #0
    beq	MA_SMTP_Sender+0x52
    mov	r0, r1
    bl	MAU_strlen
    cmp	r0, #30
    bgt	MA_SMTP_Sender+0x52
    cmp	r0, #0
    beq	MA_SMTP_Sender+0x52
    mov	r5, #1
    ldr	r0, [r6, #4]
    cmp	r0, #0
    beq	MA_SMTP_Sender+0x6a
    add	r4, r6, #4
    ldr	r0, [r4, #0]
    bl	MAU_strlen
    cmp	r0, #0
    beq	MA_SMTP_Sender+0x52
    cmp	r0, #127
    bgt	MA_SMTP_Sender+0x52
    mov	r0, #128
    lsl	r0, r0, #1
    cmp	r5, r0
    ble	MA_SMTP_Sender+0x60
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_SMTP_Sender+0x7a
    add	r4, #4
    add	r5, #1
    ldr	r0, [r4, #0]
    cmp	r0, #0
    bne	MA_SMTP_Sender+0x3c
    ldr	r0, [pc, #20]
    str	r6, [r0, #112]
    mov	r0, #12
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
.size MA_SMTP_Sender, .-MA_SMTP_Sender
");
#endif

#if 0
#else
asm("
.lcomm smtpRes.158, 0x4

.align 2
.thumb_func
MATASK_SMTP_Sender:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_SMTP_Sender+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_SMTP_Sender+0x3c
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
    b	MATASK_SMTP_Sender+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_SMTP_Sender+0x4e
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
    beq	MATASK_SMTP_Sender+0xa8
    ldr	r3, [pc, #64]
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
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_SMTP_Sender+0x3aa
.align 2
    .word gMA

    ldr	r6, [pc, #32]
    mov	r7, r6
    add	r7, #98
    ldrb	r0, [r7, #0]
    cmp	r0, #3
    bne	MATASK_SMTP_Sender+0xb6
    b	MATASK_SMTP_Sender+0x258
    cmp	r0, #3
    bgt	MATASK_SMTP_Sender+0xd0
    cmp	r0, #1
    beq	MATASK_SMTP_Sender+0x148
    cmp	r0, #1
    ble	MATASK_SMTP_Sender+0xc4
    b	MATASK_SMTP_Sender+0x1e8
    cmp	r0, #0
    beq	MATASK_SMTP_Sender+0xea
    b	MATASK_SMTP_Sender+0x3aa
.align 2
    .word gMA

    cmp	r0, #240
    bne	MATASK_SMTP_Sender+0xd6
    b	MATASK_SMTP_Sender+0x344
    cmp	r0, #240
    bgt	MATASK_SMTP_Sender+0xe2
    cmp	r0, #4
    bne	MATASK_SMTP_Sender+0xe0
    b	MATASK_SMTP_Sender+0x330
    b	MATASK_SMTP_Sender+0x3aa
    cmp	r0, #241
    bne	MATASK_SMTP_Sender+0xe8
    b	MATASK_SMTP_Sender+0x366
    b	MATASK_SMTP_Sender+0x3aa
    mov	r0, #220
    lsl	r0, r0, #2
    add	r5, r6, r0
    ldr	r1, [pc, #76]
    mov	r0, r5
    bl	MAU_strcpy
    ldr	r0, [r6, #112]
    ldr	r1, [r0, #0]
    mov	r0, r5
    bl	MAU_strcat
    ldr	r1, [pc, #64]
    mov	r0, r5
    bl	MAU_strcat
    ldr	r0, [r6, #112]
    add	r0, #4
    str	r0, [r6, #112]
    bl	InitPrevBuf
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    mov	r2, r8
    strh	r2, [r4, #0]
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
    b	MATASK_SMTP_Sender+0x35c
.align 2
    .word strEndMultiLine.25+0x1c
    .word strEndMultiLine.25+0x28

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
    ldr	r2, [pc, #104]
    add	r4, r6, r2
    ldr	r1, [pc, #104]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r4
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_Sender+0x17a
    b	MATASK_SMTP_Sender+0x288
    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #84]
    str	r0, [r1, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Sender+0x18a
    b	MATASK_SMTP_Sender+0x318
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #68]
    add	r0, r6, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #56]
    add	r0, r6, r1
    ldr	r4, [pc, #56]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r1, r6
    add	r1, #94
    ldrh	r0, [r1, #0]
    strh	r2, [r1, #0]
    ldrh	r0, [r1, #0]
    cmp	r0, #250
    bne	MATASK_SMTP_Sender+0x1c0
    b	MATASK_SMTP_Sender+0x35c
    mov	r2, r6
    add	r2, #102
    mov	r0, #48
    strb	r0, [r2, #0]
    ldrh	r1, [r1, #0]
    mov	r0, r6
    add	r0, #104
    b	MATASK_SMTP_Sender+0x326
.align 2
    .word 0x0000047d
    .word 0x000006fa
    .word smtpRes.158
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0

    ldr	r0, [r6, #112]
    ldr	r0, [r0, #0]
    cmp	r0, #0
    beq	MATASK_SMTP_Sender+0x250
    mov	r2, #220
    lsl	r2, r2, #2
    add	r5, r6, r2
    ldr	r1, [pc, #80]
    mov	r0, r5
    bl	MAU_strcpy
    ldr	r0, [r6, #112]
    ldr	r1, [r0, #0]
    mov	r0, r5
    bl	MAU_strcat
    ldr	r1, [pc, #64]
    mov	r0, r5
    bl	MAU_strcat
    ldr	r0, [r6, #112]
    add	r0, #4
    str	r0, [r6, #112]
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
    b	MATASK_SMTP_Sender+0x35c
.align 2
    .word strEndMultiLine.25+0x2c
    .word strEndMultiLine.25+0x28

    ldrb	r0, [r7, #0]
    mov	r0, #4
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Sender+0x3aa
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
    bne	MATASK_SMTP_Sender+0x2ac
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
    b	MATASK_SMTP_Sender+0x3aa
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    mov	r0, r4
    bl	CheckSMTPResponse
    ldr	r1, [pc, #68]
    str	r0, [r1, #0]
    cmp	r0, #0
    bne	MATASK_SMTP_Sender+0x318
    ldrb	r0, [r4, #0]
    sub	r0, #48
    mov	r1, #100
    mov	r2, r0
    mul	r2, r1
    ldr	r4, [pc, #52]
    add	r0, r6, r4
    ldrb	r1, [r0, #0]
    sub	r1, #48
    lsl	r0, r1, #2
    add	r0, r0, r1
    lsl	r0, r0, #1
    add	r2, r2, r0
    ldr	r1, [pc, #40]
    add	r0, r6, r1
    ldr	r4, [pc, #40]
    add	r2, r2, r4
    ldrb	r0, [r0, #0]
    add	r2, r2, r0
    mov	r1, r6
    add	r1, #94
    ldrh	r0, [r1, #0]
    strh	r2, [r1, #0]
    ldrh	r0, [r1, #0]
    cmp	r0, #250
    bne	MATASK_SMTP_Sender+0x308
    ldrb	r0, [r7, #0]
    mov	r0, #2
    strb	r0, [r7, #0]
    b	MATASK_SMTP_Sender+0x3aa
.align 2
    .word smtpRes.158
    .word 0x0000047e
    .word 0x0000047f
    .word 0x0000ffd0

    mov	r2, r6
    add	r2, #102
    mov	r0, #48
    strb	r0, [r2, #0]
    ldrh	r1, [r1, #0]
    mov	r0, r6
    add	r0, #104
    b	MATASK_SMTP_Sender+0x326
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
    b	MATASK_SMTP_Sender+0x3aa
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    mov	r0, r6
    add	r0, #96
    ldrb	r1, [r0, #0]
    mov	r1, #1
    strb	r1, [r0, #0]
    b	MATASK_SMTP_Sender+0x3aa
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
    b	MATASK_SMTP_Sender+0x3aa
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
.size MATASK_SMTP_Sender, .-MATASK_SMTP_Sender
");
#endif

#if 0
#else
void MA_SMTP_Send(const char *pSendData, u16 sendSize, int endFlag);
asm("
.align 2
.thumb_func
.global MA_SMTP_Send
MA_SMTP_Send:
    push	{r4, r5, r6, r7, lr}
    mov	r6, r0
    mov	r7, r2
    lsl	r1, r1, #16
    lsr	r4, r1, #16
    mov	r5, r4
    bl	SetApiCallFlag
    mov	r0, #13
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_SMTP_Send+0x20
    bl	ResetApiCallFlag
    b	MA_SMTP_Send+0x5a
    cmp	r4, #0
    bne	MA_SMTP_Send+0x32
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_SMTP_Send+0x5a
    ldr	r1, [pc, #44]
    mov	r0, r1
    add	r0, #96
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	MA_SMTP_Send+0x42
    mov	r0, #0
    str	r0, [r1, #124]
    str	r6, [r1, #112]
    str	r5, [r1, #116]
    str	r7, [r1, #120]
    ldr	r0, [r1, #124]
    add	r0, r0, r5
    str	r0, [r1, #124]
    mov	r0, #13
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
.size MA_SMTP_Send, .-MA_SMTP_Send
");
#endif

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
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_SMTP_POP3_Quit:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_SMTP_POP3_Quit+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_SMTP_POP3_Quit+0x3c
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
    b	MATASK_SMTP_POP3_Quit+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_SMTP_POP3_Quit+0x4e
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
    beq	MATASK_SMTP_POP3_Quit+0xb0
    ldr	r0, [pc, #16]
    add	r0, #92
    ldrb	r0, [r0, #0]
    cmp	r0, #4
    bne	MATASK_SMTP_POP3_Quit+0x78
    mov	r0, #48
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_SMTP_POP3_Quit+0x80
.align 2
    .word gMA

    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    ldr	r3, [pc, #40]
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
    b	MATASK_SMTP_POP3_Quit+0x1c8
.align 2
    .word gMA

    ldr	r6, [pc, #24]
    mov	r7, r6
    add	r7, #98
    ldrb	r0, [r7, #0]
    mov	r2, r0
    cmp	r2, #2
    beq	MATASK_SMTP_POP3_Quit+0x18c
    cmp	r2, #2
    bgt	MATASK_SMTP_POP3_Quit+0xd0
    cmp	r2, #0
    beq	MATASK_SMTP_POP3_Quit+0xe8
    cmp	r2, #1
    beq	MATASK_SMTP_POP3_Quit+0x12c
    b	MATASK_SMTP_POP3_Quit+0x240
.align 2
    .word gMA

    cmp	r2, #240
    bne	MATASK_SMTP_POP3_Quit+0xd6
    b	MATASK_SMTP_POP3_Quit+0x1da
    cmp	r2, #240
    bgt	MATASK_SMTP_POP3_Quit+0xe0
    cmp	r2, #3
    beq	MATASK_SMTP_POP3_Quit+0x1a2
    b	MATASK_SMTP_POP3_Quit+0x240
    cmp	r2, #241
    bne	MATASK_SMTP_POP3_Quit+0xe6
    b	MATASK_SMTP_POP3_Quit+0x1fc
    b	MATASK_SMTP_POP3_Quit+0x240
    bl	InitPrevBuf
    mov	r0, #220
    lsl	r0, r0, #2
    add	r5, r6, r0
    ldr	r1, [pc, #52]
    mov	r0, r5
    bl	MAU_strcpy
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    mov	r2, r8
    strh	r2, [r4, #0]
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
    b	MATASK_SMTP_POP3_Quit+0x1f2
.align 2
    .word strEndMultiLine.25+0x8

    mov	r3, #242
    lsl	r3, r3, #1
    add	r0, r6, r3
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #40]
    add	r0, r6, r2
    ldr	r3, [pc, #40]
    add	r1, r6, r3
    ldrh	r1, [r1, #0]
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_SMTP_POP3_Quit+0x17c
    mov	r0, r8
    strh	r0, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_SMTP_POP3_Quit+0x240
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    ldrb	r0, [r6, #5]
    lsl	r0, r0, #2
    mov	r1, r6
    add	r1, #44
    add	r0, r0, r1
    ldr	r0, [r0, #0]
    str	r0, [r6, #112]
    b	MATASK_SMTP_POP3_Quit+0x1f2
    ldr	r0, [r6, #112]
    sub	r0, #1
    str	r0, [r6, #112]
    cmp	r0, #0
    bne	MATASK_SMTP_POP3_Quit+0x240
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r6, r1
    mov	r1, r6
    add	r1, #99
    b	MATASK_SMTP_POP3_Quit+0x1ec
    mov	r0, r6
    add	r0, #92
    ldrb	r1, [r0, #0]
    strb	r2, [r0, #0]
    ldrh	r1, [r6, #2]
    mov	r0, #255
    and	r0, r1
    ldrh	r1, [r6, #2]
    mov	r2, #0
    strh	r0, [r6, #2]
    ldrh	r0, [r6, #2]
    mov	r3, #128
    lsl	r3, r3, #1
    mov	r1, r3
    orr	r0, r1
    ldrh	r1, [r6, #2]
    orr	r0, r2
    strh	r0, [r6, #2]
    mov	r0, r6
    add	r0, #99
    strb	r2, [r0, #0]
    add	r0, #105
    strb	r2, [r0, #0]
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_SMTP_POP3_Quit+0x240
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r6, r4
    mov	r1, r8
    strh	r1, [r0, #0]
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
    b	MATASK_SMTP_POP3_Quit+0x240
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
.size MATASK_SMTP_POP3_Quit, .-MATASK_SMTP_POP3_Quit
");
#endif

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

#if 0
#else
void MA_POP3_Connect(const char *pUserID, const char *pPassword);
asm("
.align 2
.thumb_func
.global MA_POP3_Connect
MA_POP3_Connect:
    push	{r4, r5, r6, lr}
    mov	r5, r0
    mov	r6, r1
    bl	SetApiCallFlag
    mov	r0, #15
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_POP3_Connect+0x1a
    bl	ResetApiCallFlag
    b	MA_POP3_Connect+0xc2
    mov	r0, r5
    bl	MAU_strlen
    cmp	r0, #30
    bgt	MA_POP3_Connect+0x36
    cmp	r0, #0
    beq	MA_POP3_Connect+0x36
    mov	r0, r6
    bl	MAU_strlen
    cmp	r0, #16
    bgt	MA_POP3_Connect+0x36
    cmp	r0, #0
    bne	MA_POP3_Connect+0x44
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_POP3_Connect+0xc2
    ldr	r4, [pc, #44]
    mov	r1, #220
    lsl	r1, r1, #2
    add	r0, r4, r1
    str	r0, [r4, #112]
    bl	MA_GetPOP3ServerName
    ldr	r0, [r4, #112]
    bl	MAU_strlen
    mov	r1, r0
    ldr	r0, [r4, #112]
    add	r0, r0, r1
    add	r0, #1
    str	r0, [r4, #116]
    ldr	r1, [pc, #20]
    bl	MAU_strcpy
    ldr	r0, [r4, #116]
    mov	r1, r5
    bl	MAU_strcat
    ldr	r2, [r4, #116]
    b	MA_POP3_Connect+0x7e
.align 2
    .word gMA
    .word strEndMultiLine.25+0x40

    add	r2, #1
    ldrb	r0, [r2, #0]
    cmp	r0, #0
    beq	MA_POP3_Connect+0x88
    cmp	r0, #64
    bne	MA_POP3_Connect+0x7c
    mov	r0, #13
    strb	r0, [r2, #0]
    add	r2, #1
    mov	r0, #10
    strb	r0, [r2, #0]
    add	r2, #1
    mov	r0, #0
    strb	r0, [r2, #0]
    add	r2, #1
    ldr	r4, [pc, #44]
    str	r2, [r4, #120]
    ldr	r1, [pc, #44]
    mov	r0, r2
    bl	MAU_strcpy
    ldr	r0, [r4, #120]
    mov	r1, r6
    bl	MAU_strcat
    ldr	r0, [r4, #120]
    ldr	r1, [pc, #28]
    bl	MAU_strcat
    mov	r0, #15
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word strEndMultiLine.25+0x48
    .word strEndMultiLine.25+0x18
.size MA_POP3_Connect, .-MA_POP3_Connect
");
#endif

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
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
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
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
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
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
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

#if 0
#else
asm("
.lcomm cp.191, 0x4
.lcomm pop3res.192, 0x4

.align 2
.thumb_func
MATASK_POP3_Stat:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_POP3_Stat+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_POP3_Stat+0x3c
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
    b	MATASK_POP3_Stat+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_Stat+0x4e
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
    lsr	r7, r1, #16
    cmp	r7, #0
    beq	MATASK_POP3_Stat+0xa0
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
    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_POP3_Stat+0x1e0
.align 2
    .word gMA

    ldr	r6, [pc, #28]
    mov	r0, #98
    add	r0, r0, r6
    mov	r8, r0
    ldrb	r0, [r0, #0]
    cmp	r0, #2
    bne	MATASK_POP3_Stat+0xb0
    b	MATASK_POP3_Stat+0x1e0
    cmp	r0, #2
    bgt	MATASK_POP3_Stat+0xc4
    cmp	r0, #0
    beq	MATASK_POP3_Stat+0xd2
    cmp	r0, #1
    beq	MATASK_POP3_Stat+0x114
    b	MATASK_POP3_Stat+0x252
.align 2
    .word gMA

    cmp	r0, #240
    bne	MATASK_POP3_Stat+0xca
    b	MATASK_POP3_Stat+0x1ea
    cmp	r0, #241
    bne	MATASK_POP3_Stat+0xd0
    b	MATASK_POP3_Stat+0x20e
    b	MATASK_POP3_Stat+0x252
    mov	r1, #220
    lsl	r1, r1, #2
    add	r5, r6, r1
    ldr	r1, [pc, #52]
    mov	r0, r5
    bl	MAU_strcpy
    bl	InitPrevBuf
    mov	r2, #240
    lsl	r2, r2, #1
    add	r4, r6, r2
    strh	r7, [r4, #0]
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
    b	MATASK_POP3_Stat+0x194
.align 2
    .word strEndMultiLine.25+0x50

    mov	r1, #242
    lsl	r1, r1, #1
    add	r0, r6, r1
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r2, #240
    lsl	r2, r2, #1
    add	r4, r6, r2
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r0, [pc, #40]
    add	r5, r6, r0
    ldr	r1, [pc, #40]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r5
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_Stat+0x164
    strh	r7, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_Stat+0x252
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    mov	r0, r5
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #48]
    str	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_POP3_Stat+0x1ac
    ldr	r4, [pc, #44]
    ldr	r2, [pc, #48]
    add	r0, r6, r2
    str	r0, [r4, #0]
    bl	MAU_atoi
    ldr	r1, [r6, #112]
    strh	r0, [r1, #0]
    ldr	r0, [r4, #0]
    bl	MAU_FindPostBlank
    str	r0, [r4, #0]
    bl	MAU_atoi
    ldr	r1, [r6, #116]
    str	r0, [r1, #0]
    mov	r4, r8
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_POP3_Stat+0x252
.align 2
    .word pop3res.192
    .word cp.191
    .word 0x00000481

    cmp	r1, #1
    bne	MATASK_POP3_Stat+0x1c8
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    strh	r7, [r0, #0]
    mov	r1, r8
    ldrb	r0, [r1, #0]
    mov	r0, #240
    strb	r0, [r1, #0]
    b	MATASK_POP3_Stat+0x252
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    strh	r7, [r0, #0]
    mov	r2, r8
    ldrb	r0, [r2, #0]
    mov	r0, #240
    strb	r0, [r2, #0]
    b	MATASK_POP3_Stat+0x252
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_Stat+0x252
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r6, r4
    strh	r7, [r0, #0]
    mov	r1, r6
    add	r1, #212
    str	r1, [r0, #4]
    sub	r1, #113
    ldrb	r1, [r1, #0]
    bl	MABIOS_TCPDisconnect
    mov	r1, r8
    ldrb	r0, [r1, #0]
    add	r0, #1
    ldrb	r1, [r1, #0]
    mov	r2, r8
    strb	r0, [r2, #0]
    b	MATASK_POP3_Stat+0x252
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
.size MATASK_POP3_Stat, .-MATASK_POP3_Stat
");
#endif

#if 0
#else
void MA_POP3_List(u16 mailNo, u32 *pSize);
asm("
.align 2
.thumb_func
.global MA_POP3_List
MA_POP3_List:
    push	{r4, r5, r6, lr}
    mov	r5, r1
    lsl	r0, r0, #16
    lsr	r4, r0, #16
    mov	r6, r4
    bl	SetApiCallFlag
    mov	r0, #17
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_POP3_List+0x1e
    bl	ResetApiCallFlag
    b	MA_POP3_List+0x68
    cmp	r4, #0
    bne	MA_POP3_List+0x30
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_POP3_List+0x68
    ldr	r4, [pc, #60]
    str	r5, [r4, #112]
    mov	r0, #220
    lsl	r0, r0, #2
    add	r4, r4, r0
    ldr	r1, [pc, #56]
    mov	r0, r4
    bl	MAU_strcpy
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    add	r1, r1, r4
    mov	r0, r6
    mov	r2, #10
    bl	MAU_itoa
    ldr	r1, [pc, #32]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, #17
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5, r6}
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word strEndMultiLine.25+0x58
    .word strEndMultiLine.25+0x18
.size MA_POP3_List, .-MA_POP3_List
");
#endif

#if 0
#else
asm("
.lcomm cp.199, 0x4
.lcomm pop3res.200, 0x4

.align 2
.thumb_func
MATASK_POP3_List:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_POP3_List+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_POP3_List+0x3c
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
    b	MATASK_POP3_List+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_List+0x4e
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
    lsr	r5, r1, #16
    cmp	r5, #0
    beq	MATASK_POP3_List+0xa0
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
    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_POP3_List+0x1b6
.align 2
    .word gMA

    ldr	r6, [pc, #24]
    mov	r7, r6
    add	r7, #98
    ldrb	r0, [r7, #0]
    cmp	r0, #100
    bne	MATASK_POP3_List+0xae
    b	MATASK_POP3_List+0x1b6
    cmp	r0, #100
    bgt	MATASK_POP3_List+0xc0
    cmp	r0, #0
    beq	MATASK_POP3_List+0xce
    cmp	r0, #1
    beq	MATASK_POP3_List+0x102
    b	MATASK_POP3_List+0x224
.align 2
    .word gMA

    cmp	r0, #240
    bne	MATASK_POP3_List+0xc6
    b	MATASK_POP3_List+0x1c0
    cmp	r0, #241
    bne	MATASK_POP3_List+0xcc
    b	MATASK_POP3_List+0x1e0
    b	MATASK_POP3_List+0x224
    bl	InitPrevBuf
    mov	r0, #240
    lsl	r0, r0, #1
    add	r4, r6, r0
    strh	r5, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    mov	r1, #220
    lsl	r1, r1, #2
    add	r5, r6, r1
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
    b	MATASK_POP3_List+0x1d6
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r6, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #44]
    add	r2, r2, r6
    mov	r8, r2
    ldr	r1, [pc, #40]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r8
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_List+0x154
    strh	r5, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_List+0x224
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    mov	r0, r8
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #32]
    str	r1, [r0, #0]
    cmp	r1, #0
    bne	MATASK_POP3_List+0x18c
    ldr	r4, [pc, #28]
    ldr	r2, [pc, #32]
    add	r0, r6, r2
    bl	MAU_FindPostBlank
    str	r0, [r4, #0]
    bl	MAU_atoi
    ldr	r1, [r6, #112]
    str	r0, [r1, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #100
    strb	r0, [r7, #0]
    b	MATASK_POP3_List+0x224
.align 2
    .word pop3res.200
    .word cp.199
    .word 0x00000481

    cmp	r1, #1
    bne	MATASK_POP3_List+0x1a0
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    add	r1, #2
    mov	r0, #4
    strh	r0, [r1, #0]
    b	MATASK_POP3_List+0x1ae
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    strh	r5, [r0, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #240
    strb	r0, [r7, #0]
    b	MATASK_POP3_List+0x224
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_List+0x224
    mov	r4, #240
    lsl	r4, r4, #1
    add	r0, r6, r4
    strh	r5, [r0, #0]
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
    b	MATASK_POP3_List+0x224
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
.size MATASK_POP3_List, .-MATASK_POP3_List
");
#endif

#if 0
#else
void MA_POP3_Retr(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize);
asm("
.align 2
.thumb_func
.global MA_POP3_Retr
MA_POP3_Retr:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r9, r1
    mov	r8, r3
    lsl	r0, r0, #16
    lsr	r6, r0, #16
    lsl	r2, r2, #16
    lsr	r5, r2, #16
    mov	r7, r5
    bl	SetApiCallFlag
    mov	r0, #18
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_POP3_Retr+0x2a
    bl	ResetApiCallFlag
    b	MA_POP3_Retr+0x15e
    mov	r0, #0
    mov	r1, r8
    strh	r0, [r1, #0]
    ldr	r4, [pc, #48]
    mov	r0, r9
    str	r0, [r4, #112]
    str	r5, [r4, #116]
    str	r1, [r4, #120]
    ldrh	r1, [r4, #2]
    mov	r0, #4
    and	r0, r1
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    cmp	r1, #0
    beq	MA_POP3_Retr+0xfc
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #28]
    and	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    ldr	r0, [r4, #64]
    ldr	r1, [pc, #20]
    and	r0, r1
    str	r0, [r4, #64]
    cmp	r5, #0
    bne	MA_POP3_Retr+0x70
    str	r5, [r4, #112]
    str	r5, [r4, #120]
    b	MA_POP3_Retr+0xa4
.align 2
    .word gMA
    .word 0x0000fffb
    .word 0xffff7fff

    ldr	r1, [pc, #60]
    add	r6, r4, r1
    ldrh	r0, [r6, #0]
    cmp	r0, #0
    beq	MA_POP3_Retr+0xb8
    cmp	r0, r5
    bhi	MA_POP3_Retr+0xb8
    ldr	r0, [pc, #52]
    add	r1, r4, r0
    ldrh	r2, [r6, #0]
    mov	r0, r9
    bl	MAU_memcpy
    mov	r1, r8
    ldrh	r0, [r1, #0]
    ldrh	r1, [r6, #0]
    add	r0, r0, r1
    mov	r1, r8
    strh	r0, [r1, #0]
    mov	r0, #0
    strh	r0, [r6, #0]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #8
    orr	r0, r1
    str	r0, [r4, #64]
    mov	r0, #18
    mov	r1, #1
    bl	MA_TaskSet
    b	MA_POP3_Retr+0x15a
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    ldr	r4, [pc, #52]
    mov	r0, r9
    mov	r1, r4
    mov	r2, r7
    bl	MAU_memcpy
    ldr	r0, [pc, #44]
    add	r5, r4, r0
    ldr	r1, [pc, #44]
    add	r2, r4, r1
    ldrh	r0, [r2, #0]
    sub	r0, r0, r7
    strh	r0, [r2, #0]
    add	r1, r7, r4
    ldrh	r2, [r2, #0]
    mov	r0, r4
    bl	MAU_memcpy
    mov	r0, r8
    strh	r7, [r0, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #4
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    bl	ResetApiCallFlag
    b	MA_POP3_Retr+0x15e
.align 2
    .word gMA+0x47d
    .word 0xfffffb83
    .word 0x0000027d

    cmp	r7, #0
    bne	MA_POP3_Retr+0x10e
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_POP3_Retr+0x15e
    mov	r0, r4
    add	r0, #140
    str	r1, [r0, #0]
    bl	InitPrevBuf
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #80]
    and	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    ldr	r0, [r4, #64]
    ldr	r1, [pc, #72]
    and	r0, r1
    str	r0, [r4, #64]
    mov	r1, #220
    lsl	r1, r1, #2
    add	r4, r4, r1
    ldr	r1, [pc, #64]
    mov	r0, r4
    bl	MAU_strcpy
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    add	r1, r1, r4
    mov	r0, r6
    mov	r2, #10
    bl	MAU_itoa
    ldr	r1, [pc, #44]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, #18
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0000fffb
    .word 0xffff7fff
    .word strEndMultiLine.25+0x60
    .word strEndMultiLine.25+0x18
.size MA_POP3_Retr, .-MA_POP3_Retr
");
#endif

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

#if 0
#else
void MA_POP3_Dele(u16 mailNo);
asm("
.align 2
.thumb_func
.global MA_POP3_Dele
MA_POP3_Dele:
    push	{r4, r5, lr}
    lsl	r0, r0, #16
    lsr	r5, r0, #16
    bl	SetApiCallFlag
    mov	r0, #19
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_POP3_Dele+0x1a
    bl	ResetApiCallFlag
    b	MA_POP3_Dele+0x4a
    ldr	r4, [pc, #52]
    ldr	r1, [pc, #52]
    mov	r0, r4
    bl	MAU_strcpy
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    add	r1, r1, r4
    mov	r0, r5
    mov	r2, #10
    bl	MAU_itoa
    ldr	r1, [pc, #32]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, #19
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA+0x370
    .word strEndMultiLine.25+0x68
    .word strEndMultiLine.25+0x18
.size MA_POP3_Dele, .-MA_POP3_Dele
");
#endif

#if 0
#else
asm("
.lcomm pop3res.216, 0x4

.align 2
.thumb_func
MATASK_POP3_Dele:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r8
    push	{r7}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_POP3_Dele+0x4e
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_POP3_Dele+0x3c
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
    b	MATASK_POP3_Dele+0x4e
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_Dele+0x4e
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
    lsr	r5, r1, #16
    cmp	r5, #0
    beq	MATASK_POP3_Dele+0xa0
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
    mov	r0, #49
    mov	r1, #0
    bl	MA_SetApiError
    b	MATASK_POP3_Dele+0x18e
.align 2
    .word gMA

    ldr	r6, [pc, #24]
    mov	r7, r6
    add	r7, #98
    ldrb	r0, [r7, #0]
    cmp	r0, #2
    beq	MATASK_POP3_Dele+0x18e
    cmp	r0, #2
    bgt	MATASK_POP3_Dele+0xc0
    cmp	r0, #0
    beq	MATASK_POP3_Dele+0xca
    cmp	r0, #1
    beq	MATASK_POP3_Dele+0xfe
    b	MATASK_POP3_Dele+0x1fc
.align 2
    .word gMA

    cmp	r0, #240
    beq	MATASK_POP3_Dele+0x198
    cmp	r0, #241
    beq	MATASK_POP3_Dele+0x1b8
    b	MATASK_POP3_Dele+0x1fc
    bl	InitPrevBuf
    mov	r0, #240
    lsl	r0, r0, #1
    add	r4, r6, r0
    strh	r5, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    mov	r1, #220
    lsl	r1, r1, #2
    add	r5, r6, r1
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
    b	MATASK_POP3_Dele+0x1ae
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r6, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    mov	r1, #240
    lsl	r1, r1, #1
    add	r4, r6, r1
    ldrh	r1, [r4, #0]
    sub	r1, #1
    lsl	r1, r1, #16
    lsr	r1, r1, #16
    bl	ConcatPrevBuf
    ldr	r2, [pc, #44]
    add	r2, r2, r6
    mov	r8, r2
    ldr	r1, [pc, #40]
    add	r0, r6, r1
    ldrh	r1, [r0, #0]
    mov	r0, r8
    bl	MAU_CheckCRLF
    cmp	r0, #0
    bne	MATASK_POP3_Dele+0x150
    strh	r5, [r4, #0]
    mov	r0, r6
    add	r0, #212
    str	r0, [r4, #4]
    sub	r0, #113
    ldrb	r3, [r0, #0]
    mov	r0, r4
    mov	r1, #0
    mov	r2, #0
    bl	MABIOS_Data
    b	MATASK_POP3_Dele+0x1fc
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    mov	r0, r8
    bl	CheckPOP3Response
    mov	r1, r0
    ldr	r0, [pc, #24]
    str	r1, [r0, #0]
    cmp	r1, #0
    beq	MATASK_POP3_Dele+0x1ae
    cmp	r1, #1
    bne	MATASK_POP3_Dele+0x178
    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    add	r1, #2
    mov	r0, #4
    strh	r0, [r1, #0]
    b	MATASK_POP3_Dele+0x186
.align 2
    .word pop3res.216

    mov	r1, r6
    add	r1, #102
    mov	r0, #49
    strb	r0, [r1, #0]
    mov	r0, r6
    add	r0, #104
    strh	r5, [r0, #0]
    ldrb	r0, [r7, #0]
    mov	r0, #240
    strb	r0, [r7, #0]
    b	MATASK_POP3_Dele+0x1fc
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_Dele+0x1fc
    mov	r2, #240
    lsl	r2, r2, #1
    add	r0, r6, r2
    strh	r5, [r0, #0]
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
    b	MATASK_POP3_Dele+0x1fc
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
.size MATASK_POP3_Dele, .-MATASK_POP3_Dele
");
#endif

#if 0
#else
void MA_POP3_Head(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize);
asm("
.align 2
.thumb_func
.global MA_POP3_Head
MA_POP3_Head:
    push	{r4, r5, r6, r7, lr}
    mov	r7, r9
    mov	r6, r8
    push	{r6, r7}
    mov	r9, r1
    mov	r8, r3
    lsl	r0, r0, #16
    lsr	r6, r0, #16
    lsl	r2, r2, #16
    lsr	r5, r2, #16
    mov	r7, r5
    bl	SetApiCallFlag
    mov	r0, #18
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_POP3_Head+0x2a
    bl	ResetApiCallFlag
    b	MA_POP3_Head+0x156
    mov	r0, #0
    mov	r1, r8
    strh	r0, [r1, #0]
    ldr	r4, [pc, #48]
    mov	r0, r9
    str	r0, [r4, #112]
    str	r5, [r4, #116]
    str	r1, [r4, #120]
    ldrh	r1, [r4, #2]
    mov	r0, #4
    and	r0, r1
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    cmp	r1, #0
    beq	MA_POP3_Head+0xfc
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #28]
    and	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    ldr	r0, [r4, #64]
    ldr	r1, [pc, #20]
    and	r0, r1
    str	r0, [r4, #64]
    cmp	r5, #0
    bne	MA_POP3_Head+0x70
    str	r5, [r4, #112]
    str	r5, [r4, #120]
    b	MA_POP3_Head+0xa4
.align 2
    .word gMA
    .word 0x0000fffb
    .word 0xffff7fff

    ldr	r1, [pc, #60]
    add	r6, r4, r1
    ldrh	r0, [r6, #0]
    cmp	r0, #0
    beq	MA_POP3_Head+0xb8
    cmp	r0, r5
    bhi	MA_POP3_Head+0xb8
    ldr	r0, [pc, #52]
    add	r1, r4, r0
    ldrh	r2, [r6, #0]
    mov	r0, r9
    bl	MAU_memcpy
    mov	r1, r8
    ldrh	r0, [r1, #0]
    ldrh	r1, [r6, #0]
    add	r0, r0, r1
    mov	r1, r8
    strh	r0, [r1, #0]
    mov	r0, #0
    strh	r0, [r6, #0]
    ldr	r0, [r4, #64]
    mov	r1, #128
    lsl	r1, r1, #8
    orr	r0, r1
    str	r0, [r4, #64]
    mov	r0, #18
    mov	r1, #1
    bl	MA_TaskSet
    b	MA_POP3_Head+0x152
    lsl	r0, r0, #0
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    ldr	r4, [pc, #52]
    mov	r0, r9
    mov	r1, r4
    mov	r2, r7
    bl	MAU_memcpy
    ldr	r0, [pc, #44]
    add	r5, r4, r0
    ldr	r1, [pc, #44]
    add	r2, r4, r1
    ldrh	r0, [r2, #0]
    sub	r0, r0, r7
    strh	r0, [r2, #0]
    add	r1, r7, r4
    ldrh	r2, [r2, #0]
    mov	r0, r4
    bl	MAU_memcpy
    mov	r0, r8
    strh	r7, [r0, #0]
    ldrh	r1, [r5, #2]
    mov	r0, #4
    ldrh	r2, [r5, #2]
    orr	r0, r1
    strh	r0, [r5, #2]
    bl	ResetApiCallFlag
    b	MA_POP3_Head+0x156
.align 2
    .word gMA+0x47d
    .word 0xfffffb83
    .word 0x0000027d

    cmp	r7, #0
    bne	MA_POP3_Head+0x10e
    mov	r0, #32
    mov	r1, #0
    bl	MA_SetApiError
    bl	ResetApiCallFlag
    b	MA_POP3_Head+0x156
    mov	r0, r4
    add	r0, #140
    str	r1, [r0, #0]
    bl	InitPrevBuf
    ldrh	r1, [r4, #2]
    ldr	r0, [pc, #72]
    and	r0, r1
    ldrh	r1, [r4, #2]
    strh	r0, [r4, #2]
    mov	r1, #220
    lsl	r1, r1, #2
    add	r4, r4, r1
    ldr	r1, [pc, #60]
    mov	r0, r4
    bl	MAU_strcpy
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    add	r1, r1, r4
    mov	r0, r6
    mov	r2, #10
    bl	MAU_itoa
    ldr	r1, [pc, #40]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, #18
    mov	r1, #0
    bl	MA_TaskSet
    bl	ResetApiCallFlag
    pop	{r3, r4}
    mov	r8, r3
    mov	r9, r4
    pop	{r4, r5, r6, r7}
    pop	{r0}
    bx	r0
.align 2
    .word 0x0000fffb
    .word strEndMultiLine.25+0x70
    .word strEndMultiLine.25+0x78
.size MA_POP3_Head, .-MA_POP3_Head
");
#endif

#if 0
#else
asm("
.lcomm cp.223, 0x4
.lcomm dataLen.224, 0x4
.lcomm pop3res.225, 0x4

.align 2
.thumb_func
MATASK_POP3_Head:
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
    bne	MATASK_POP3_Head+0x52
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MATASK_POP3_Head+0x40
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
    b	MATASK_POP3_Head+0x52
.align 2
    .word gMA

    cmp	r0, #36
    beq	MATASK_POP3_Head+0x52
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
    beq	MATASK_POP3_Head+0xa4
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
    b	MATASK_POP3_Head+0x358
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
    bne	MATASK_POP3_Head+0xb8
    b	MATASK_POP3_Head+0x358
    cmp	r4, #100
    bgt	MATASK_POP3_Head+0xcc
    cmp	r4, #0
    beq	MATASK_POP3_Head+0xda
    cmp	r4, #1
    beq	MATASK_POP3_Head+0x11a
    b	MATASK_POP3_Head+0x3ca
.align 2
    .word gMA

    cmp	r4, #240
    bne	MATASK_POP3_Head+0xd2
    b	MATASK_POP3_Head+0x362
    cmp	r4, #241
    bne	MATASK_POP3_Head+0xd8
    b	MATASK_POP3_Head+0x386
    b	MATASK_POP3_Head+0x3ca
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
    b	MATASK_POP3_Head+0x3ca
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
    beq	MATASK_POP3_Head+0x138
    b	MATASK_POP3_Head+0x2cc
    ldr	r0, [r6, #112]
    cmp	r0, #0
    bne	MATASK_POP3_Head+0x140
    b	MATASK_POP3_Head+0x2d4
    cmp	r3, #1
    bgt	MATASK_POP3_Head+0x146
    b	MATASK_POP3_Head+0x2d4
    ldr	r1, [pc, #68]
    ldr	r0, [r1, #0]
    cmp	r0, #0
    bne	MATASK_POP3_Head+0x216
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
    bne	MATASK_POP3_Head+0x194
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
    b	MATASK_POP3_Head+0x342
.align 2
    .word dataLen.224
    .word gMA+0x8c
    .word cp.223

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
    bne	MATASK_POP3_Head+0x1e0
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
    b	MATASK_POP3_Head+0x216
.align 2
    .word 0x0000047d
    .word pop3res.225
    .word dataLen.224
    .word gMA+0x8c

    cmp	r1, #1
    bne	MATASK_POP3_Head+0x1fc
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
    b	MATASK_POP3_Head+0x3ca
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
    b	MATASK_POP3_Head+0x3ca
    ldr	r3, [r7, #116]
    ldr	r4, [pc, #56]
    ldr	r2, [r4, #0]
    cmp	r3, r2
    bcc	MATASK_POP3_Head+0x25c
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
    b	MATASK_POP3_Head+0x2d4
.align 2
    .word dataLen.224
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
    b	MATASK_POP3_Head+0x3ca
.align 2
    .word dataLen.224
    .word 0x000006fa
    .word 0x0000047d

    ldr	r0, [r6, #64]
    ldr	r1, [pc, #40]
    and	r0, r1
    str	r0, [r6, #64]
    ldr	r0, [pc, #36]
    ldr	r1, [r0, #0]
    cmp	r1, #4
    ble	MATASK_POP3_Head+0x304
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
    b	MATASK_POP3_Head+0x314
.align 2
    .word 0xffff7fff
    .word dataLen.224
    .word gMA

    cmp	r1, #0
    beq	MATASK_POP3_Head+0x330
    ldr	r4, [pc, #32]
    mov	r2, #242
    lsl	r2, r2, #1
    add	r0, r4, r2
    ldr	r0, [r0, #0]
    add	r0, #1
    bl	MakeEndLineBuffer
    bl	IsEndMultiLine
    cmp	r0, #1
    bne	MATASK_POP3_Head+0x330
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #100
    strb	r1, [r0, #0]
    b	MATASK_POP3_Head+0x3ca
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
    b	MATASK_POP3_Head+0x3ca
.align 2
    .word gMA+0x1e0
    .word 0xfffffef4
    .word 0xfffffe83

    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    b	MATASK_POP3_Head+0x3ca
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
    b	MATASK_POP3_Head+0x3ca
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
.size MATASK_POP3_Head, .-MATASK_POP3_Head
");
#endif

#if 0
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

#if 0
#else
asm("
.lcomm tmpLen.249, 0x4
.lcomm tmpp.250, 0x4
.lcomm tmpNum.251, 0x1

.section .rodata
    .word 0x00000000
.align 2
.type hexChar.252, object
hexChar.252:
    .asciz \"0123456789ABCDEF\"
.size hexChar.252, .-hexChar.252
.align 2
    .asciz \"\\\"\\r\\n\"
.align 2
    .asciz \"HTTP\"
.section .text

.align 2
.thumb_func
ConcatUserAgent:
    push	{r4, r5, lr}
    mov	r4, r0
    bl	MAU_strlen
    ldr	r1, [pc, #28]
    str	r0, [r1, #0]
    ldr	r2, [pc, #28]
    add	r4, r4, r0
    str	r4, [r2, #0]
    ldr	r0, [pc, #28]
    ldrb	r1, [r0, #0]
    mov	r0, r1
    sub	r0, #32
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #94
    bls	ConcatUserAgent+0x34
    mov	r0, #48
    strb	r0, [r4, #0]
    b	ConcatUserAgent+0x36
.align 2
    .word tmpLen.249
    .word tmpp.250
    .word 0x080000ac

    strb	r1, [r4, #0]
    add	r0, r4, #1
    str	r0, [r2, #0]
    ldr	r0, [pc, #32]
    ldrb	r2, [r0, #0]
    mov	r0, r2
    sub	r0, #32
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #94
    bls	ConcatUserAgent+0x64
    ldr	r0, [pc, #20]
    ldr	r1, [r0, #0]
    mov	r2, #48
    strb	r2, [r1, #0]
    add	r1, #1
    str	r1, [r0, #0]
    mov	r5, r0
    b	ConcatUserAgent+0x70
.align 2
    .word 0x080000ad
    .word tmpp.250

    ldr	r1, [pc, #32]
    ldr	r0, [r1, #0]
    strb	r2, [r0, #0]
    add	r0, #1
    str	r0, [r1, #0]
    mov	r5, r1
    ldr	r0, [pc, #24]
    ldrb	r1, [r0, #0]
    mov	r0, r1
    sub	r0, #32
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #94
    bls	ConcatUserAgent+0x90
    ldr	r0, [r5, #0]
    mov	r1, #48
    b	ConcatUserAgent+0x92
.align 2
    .word tmpp.250
    .word 0x080000ae

    ldr	r0, [r5, #0]
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r5, #0]
    ldr	r0, [pc, #20]
    ldrb	r1, [r0, #0]
    mov	r0, r1
    sub	r0, #32
    lsl	r0, r0, #24
    lsr	r0, r0, #24
    cmp	r0, #94
    bls	ConcatUserAgent+0xb4
    ldr	r0, [r5, #0]
    mov	r1, #48
    b	ConcatUserAgent+0xb6
.align 2
    .word 0x080000af

    ldr	r0, [r5, #0]
    strb	r1, [r0, #0]
    add	r0, #1
    str	r0, [r5, #0]
    ldr	r1, [r5, #0]
    mov	r0, #45
    strb	r0, [r1, #0]
    add	r1, #1
    str	r1, [r5, #0]
    ldr	r2, [pc, #76]
    ldr	r0, [pc, #76]
    ldrb	r0, [r0, #0]
    strb	r0, [r2, #0]
    ldr	r4, [pc, #76]
    lsl	r0, r0, #24
    lsr	r0, r0, #28
    add	r0, r0, r4
    ldrb	r0, [r0, #0]
    strb	r0, [r1, #0]
    add	r3, r1, #1
    str	r3, [r5, #0]
    ldrb	r2, [r2, #0]
    mov	r0, #15
    and	r0, r2
    add	r0, r0, r4
    ldrb	r0, [r0, #0]
    strb	r0, [r1, #1]
    add	r0, r3, #1
    str	r0, [r5, #0]
    mov	r4, #13
    strb	r4, [r3, #1]
    add	r1, r0, #1
    str	r1, [r5, #0]
    mov	r2, #10
    strb	r2, [r0, #1]
    add	r0, r1, #1
    str	r0, [r5, #0]
    strb	r4, [r1, #1]
    add	r1, r0, #1
    str	r1, [r5, #0]
    strb	r2, [r0, #1]
    add	r0, r1, #1
    str	r0, [r5, #0]
    mov	r0, #0
    strb	r0, [r1, #1]
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word tmpNum.251
    .word 0x080000bc
    .word hexChar.252
.size ConcatUserAgent, .-ConcatUserAgent
");
#endif

#if 0
#else
asm("
.lcomm ret.256, 0x4

.align 2
.thumb_func
GetRequestType:
    ldr	r1, [pc, #16]
    mov	r0, #0
    str	r0, [r1, #0]
    ldr	r2, [pc, #16]
    ldr	r0, [r2, #112]
    cmp	r0, #22
    beq	GetRequestType+0x1c
    cmp	r0, #23
    beq	GetRequestType+0x36
    b	GetRequestType+0x72
.align 2
    .word ret.256
    .word gMA

    mov	r0, r2
    add	r0, #160
    ldr	r3, [r0, #0]
    cmp	r3, #1
    bne	GetRequestType+0x72
    add	r0, #4
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bls	GetRequestType+0x72
    cmp	r0, #2
    bne	GetRequestType+0x72
    str	r3, [r1, #0]
    b	GetRequestType+0x72
    mov	r0, r2
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #2
    beq	GetRequestType+0x50
    cmp	r0, #2
    bhi	GetRequestType+0x4a
    cmp	r0, #0
    beq	GetRequestType+0x6e
    b	GetRequestType+0x72
    cmp	r0, #4
    beq	GetRequestType+0x60
    b	GetRequestType+0x72
    mov	r0, r2
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bls	GetRequestType+0x72
    cmp	r0, #2
    bne	GetRequestType+0x72
    b	GetRequestType+0x6e
    mov	r0, r2
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #0
    beq	GetRequestType+0x72
    cmp	r0, #2
    bhi	GetRequestType+0x72
    mov	r0, #1
    str	r0, [r1, #0]
    ldr	r0, [r1, #0]
    bx	lr
.size GetRequestType, .-GetRequestType
");
#endif

#if 0
#else
asm("
.lcomm tmpLen.260, 0x4
.lcomm bAddContentLength.261, 0x1
.lcomm bAddContentLengthZero.262, 0x1
.lcomm bAddAuthorization.263, 0x1
.lcomm bAddAuthID.264, 0x1

.align 2
.thumb_func
CreateHttpRequestHeader:
    push	{r4, r5, lr}
    ldr	r2, [pc, #32]
    mov	r1, #0
    strb	r1, [r2, #0]
    ldr	r0, [pc, #28]
    strb	r1, [r0, #0]
    ldr	r4, [pc, #28]
    strb	r1, [r4, #0]
    ldr	r5, [pc, #28]
    strb	r1, [r5, #0]
    ldr	r1, [pc, #28]
    ldr	r0, [r1, #112]
    cmp	r0, #22
    beq	CreateHttpRequestHeader+0x38
    cmp	r0, #23
    beq	CreateHttpRequestHeader+0x72
    b	CreateHttpRequestHeader+0xc8
.align 2
    .word bAddContentLength.261
    .word bAddContentLengthZero.262
    .word bAddAuthorization.263
    .word bAddAuthID.264
    .word gMA

    mov	r0, r1
    add	r0, #160
    ldr	r3, [r0, #0]
    cmp	r3, #1
    beq	CreateHttpRequestHeader+0x56
    cmp	r3, #1
    bcc	CreateHttpRequestHeader+0xc8
    cmp	r3, #3
    bne	CreateHttpRequestHeader+0xc8
    add	r0, #4
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bne	CreateHttpRequestHeader+0xc8
    strb	r0, [r4, #0]
    b	CreateHttpRequestHeader+0xc8
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	CreateHttpRequestHeader+0x6a
    cmp	r0, #1
    bcc	CreateHttpRequestHeader+0xc8
    cmp	r0, #2
    beq	CreateHttpRequestHeader+0x6e
    b	CreateHttpRequestHeader+0xc8
    strb	r3, [r4, #0]
    b	CreateHttpRequestHeader+0xc8
    strb	r3, [r5, #0]
    b	CreateHttpRequestHeader+0xc8
    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #2
    beq	CreateHttpRequestHeader+0x92
    cmp	r0, #2
    bhi	CreateHttpRequestHeader+0x86
    cmp	r0, #0
    beq	CreateHttpRequestHeader+0x8c
    b	CreateHttpRequestHeader+0xc8
    cmp	r0, #4
    beq	CreateHttpRequestHeader+0xac
    b	CreateHttpRequestHeader+0xc8
    mov	r0, #1
    strb	r0, [r2, #0]
    b	CreateHttpRequestHeader+0xc8
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	CreateHttpRequestHeader+0x52
    cmp	r0, #1
    bcc	CreateHttpRequestHeader+0xc8
    cmp	r0, #2
    bne	CreateHttpRequestHeader+0xc8
    mov	r0, #1
    strb	r0, [r5, #0]
    strb	r0, [r2, #0]
    b	CreateHttpRequestHeader+0xc8
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	CreateHttpRequestHeader+0xc0
    cmp	r0, #1
    bcc	CreateHttpRequestHeader+0xc8
    cmp	r0, #2
    beq	CreateHttpRequestHeader+0xc4
    b	CreateHttpRequestHeader+0xc8
    strb	r0, [r2, #0]
    b	CreateHttpRequestHeader+0x52
    mov	r0, #1
    strb	r0, [r5, #0]
    ldrb	r0, [r2, #0]
    cmp	r0, #1
    bne	CreateHttpRequestHeader+0x104
    ldr	r4, [pc, #172]
    ldr	r1, [pc, #172]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r1, [pc, #168]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, r4
    bl	MAU_strlen
    mov	r1, r0
    ldr	r0, [pc, #156]
    str	r1, [r0, #0]
    ldr	r2, [pc, #156]
    add	r0, r4, r2
    ldr	r0, [r0, #0]
    sub	r2, r4, #3
    add	r1, r1, r2
    mov	r2, #10
    bl	MAU_itoa
    ldr	r1, [pc, #144]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r0, [pc, #140]
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	CreateHttpRequestHeader+0x11e
    ldr	r4, [pc, #108]
    ldr	r1, [pc, #112]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r1, [pc, #108]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r0, [pc, #120]
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	CreateHttpRequestHeader+0x142
    ldr	r4, [pc, #84]
    ldr	r1, [pc, #112]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r0, [pc, #108]
    add	r1, r4, r0
    mov	r0, r4
    bl	MAU_strcat
    ldr	r1, [pc, #104]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r0, [pc, #100]
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	CreateHttpRequestHeader+0x166
    ldr	r4, [pc, #48]
    ldr	r1, [pc, #92]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r2, [pc, #72]
    add	r1, r4, r2
    mov	r0, r4
    bl	MAU_strcat
    ldr	r1, [pc, #48]
    mov	r0, r4
    bl	MAU_strcat
    ldr	r4, [pc, #20]
    ldr	r1, [pc, #68]
    mov	r0, r4
    bl	MAU_strcat
    mov	r0, r4
    bl	ConcatUserAgent
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word gMA+0x370
    .word strHttpContentType
    .word strHttpContentLength
    .word tmpLen.260
    .word 0xfffffd18
    .word strEndMultiLine.25+0x18
    .word bAddContentLengthZero.262
    .word bAddAuthorization.263
    .word strHttpAuthorization
    .word 0x00000396
    .word hexChar.252+0x14
    .word bAddAuthID.264
    .word strHttpGbAuthID
    .word strHttpUserAgent
.size CreateHttpRequestHeader, .-CreateHttpRequestHeader
");
#endif

#if 0
#else
asm("
.lcomm step.268, 0x4

.align 2
.thumb_func
HttpGetNextStep:
    mov	r2, r0
    cmp	r2, #1
    beq	HttpGetNextStep+0xa0
    cmp	r2, #1
    bgt	HttpGetNextStep+0x10
    cmp	r2, #0
    beq	HttpGetNextStep+0x18
    b	HttpGetNextStep+0x1a2
    cmp	r2, #2
    bne	HttpGetNextStep+0x16
    b	HttpGetNextStep+0x120
    b	HttpGetNextStep+0x1a2
    ldr	r1, [pc, #12]
    ldr	r0, [r1, #112]
    cmp	r0, #22
    beq	HttpGetNextStep+0x2c
    cmp	r0, #23
    beq	HttpGetNextStep+0x42
    b	HttpGetNextStep+0x1a2
.align 2
    .word gMA

    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0x94
    cmp	r0, #1
    bcc	HttpGetNextStep+0x94
    cmp	r0, #3
    beq	HttpGetNextStep+0x40
    b	HttpGetNextStep+0x1a2
    b	HttpGetNextStep+0x94
    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #2
    beq	HttpGetNextStep+0x5c
    cmp	r0, #2
    bhi	HttpGetNextStep+0x56
    cmp	r0, #0
    beq	HttpGetNextStep+0x86
    b	HttpGetNextStep+0x1a2
    cmp	r0, #4
    beq	HttpGetNextStep+0x72
    b	HttpGetNextStep+0x1a2
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0x94
    cmp	r0, #1
    bcc	HttpGetNextStep+0x94
    cmp	r0, #2
    beq	HttpGetNextStep+0x70
    b	HttpGetNextStep+0x1a2
    b	HttpGetNextStep+0x86
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0x86
    cmp	r0, #1
    bcc	HttpGetNextStep+0x94
    cmp	r0, #2
    beq	HttpGetNextStep+0x94
    b	HttpGetNextStep+0x1a2
    ldr	r1, [pc, #8]
    mov	r0, #100
    str	r0, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    ldr	r1, [pc, #4]
    mov	r0, #8
    str	r0, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    ldr	r1, [pc, #12]
    ldr	r0, [r1, #112]
    cmp	r0, #22
    beq	HttpGetNextStep+0xb4
    cmp	r0, #23
    beq	HttpGetNextStep+0xec
    b	HttpGetNextStep+0x1a2
.align 2
    .word gMA

    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0xdc
    cmp	r0, #1
    bcc	HttpGetNextStep+0x114
    cmp	r0, #3
    bne	HttpGetNextStep+0x1a2
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bhi	HttpGetNextStep+0x1a2
    mov	r0, #110
    ldr	r1, [pc, #4]
    str	r0, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bls	HttpGetNextStep+0x114
    cmp	r0, #2
    bne	HttpGetNextStep+0x1a2
    b	HttpGetNextStep+0x16c
    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #2
    beq	HttpGetNextStep+0xdc
    cmp	r0, #2
    bhi	HttpGetNextStep+0x100
    cmp	r0, #0
    beq	HttpGetNextStep+0x114
    b	HttpGetNextStep+0x1a2
    cmp	r0, #4
    bne	HttpGetNextStep+0x1a2
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    bls	HttpGetNextStep+0x114
    cmp	r0, #2
    beq	HttpGetNextStep+0x16c
    b	HttpGetNextStep+0x1a2
    ldr	r1, [pc, #4]
    mov	r0, #110
    str	r0, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    ldr	r1, [pc, #12]
    ldr	r0, [r1, #112]
    cmp	r0, #22
    beq	HttpGetNextStep+0x134
    cmp	r0, #23
    beq	HttpGetNextStep+0x152
    b	HttpGetNextStep+0x1a2
.align 2
    .word gMA

    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0x190
    cmp	r0, #1
    bcc	HttpGetNextStep+0x16c
    cmp	r0, #3
    bne	HttpGetNextStep+0x1a2
    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #1
    beq	HttpGetNextStep+0x19e
    b	HttpGetNextStep+0xe6
    mov	r0, r1
    add	r0, #160
    ldr	r0, [r0, #0]
    cmp	r0, #2
    beq	HttpGetNextStep+0x178
    cmp	r0, #2
    bhi	HttpGetNextStep+0x166
    cmp	r0, #0
    beq	HttpGetNextStep+0x16c
    b	HttpGetNextStep+0x1a2
    cmp	r0, #4
    beq	HttpGetNextStep+0x190
    b	HttpGetNextStep+0x1a2
    ldr	r1, [pc, #4]
    mov	r0, #255
    str	r0, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #2
    bhi	HttpGetNextStep+0x1a2
    cmp	r0, #1
    bcc	HttpGetNextStep+0x1a2
    ldr	r1, [pc, #4]
    str	r2, [r1, #0]
    b	HttpGetNextStep+0x1a2
.align 2
    .word step.268

    mov	r0, r1
    add	r0, #164
    ldr	r0, [r0, #0]
    cmp	r0, #2
    bhi	HttpGetNextStep+0x1a2
    cmp	r0, #1
    bcc	HttpGetNextStep+0x1a2
    ldr	r0, [pc, #8]
    str	r2, [r0, #0]
    ldr	r1, [pc, #4]
    ldr	r0, [r1, #0]
    bx	lr
.align 2
    .word step.268
.size HttpGetNextStep, .-HttpGetNextStep
");
#endif

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
    lsl	r2, r7, #27
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_GetEEPROMData:
    push	{r4, r5, lr}
    ldr	r4, [pc, #32]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_GetEEPROMData+0x66
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #24
    beq	MATASK_GetEEPROMData+0x2e
    cmp	r0, #24
    bgt	MATASK_GetEEPROMData+0x28
    cmp	r0, #17
    beq	MATASK_GetEEPROMData+0x66
    b	MATASK_GetEEPROMData+0x58
.align 2
    .word gMA

    cmp	r0, #25
    beq	MATASK_GetEEPROMData+0x3c
    b	MATASK_GetEEPROMData+0x58
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_GetEEPROMData+0x64
    mov	r2, r4
    add	r2, #102
    mov	r1, #0
    mov	r0, #20
    strb	r0, [r2, #0]
    mov	r0, r4
    add	r0, #104
    strh	r1, [r0, #0]
    mov	r1, r4
    add	r1, #98
    ldrb	r0, [r1, #0]
    mov	r0, #250
    strb	r0, [r1, #0]
    b	MATASK_GetEEPROMData+0x66
    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #36]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #4
    beq	MATASK_GetEEPROMData+0x120
    cmp	r0, #4
    bgt	MATASK_GetEEPROMData+0x92
    cmp	r0, #1
    beq	MATASK_GetEEPROMData+0xae
    cmp	r0, #1
    bgt	MATASK_GetEEPROMData+0x88
    cmp	r0, #0
    beq	MATASK_GetEEPROMData+0xa8
    b	MATASK_GetEEPROMData+0x2d6
.align 2
    .word gMA

    cmp	r0, #2
    beq	MATASK_GetEEPROMData+0xbe
    cmp	r0, #3
    beq	MATASK_GetEEPROMData+0xd8
    b	MATASK_GetEEPROMData+0x2d6
    cmp	r0, #6
    beq	MATASK_GetEEPROMData+0x174
    cmp	r0, #6
    blt	MATASK_GetEEPROMData+0x16a
    cmp	r0, #250
    bne	MATASK_GetEEPROMData+0xa0
    b	MATASK_GetEEPROMData+0x224
    cmp	r0, #251
    bne	MATASK_GetEEPROMData+0xa6
    b	MATASK_GetEEPROMData+0x232
    b	MATASK_GetEEPROMData+0x2d6
    bl	MABIOS_Start
    b	MATASK_GetEEPROMData+0x228
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_GetEEPROMData+0x2d6
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #0
    mov	r2, #128
    bl	MABIOS_EEPROM_Read
    b	MATASK_GetEEPROMData+0x228
    mov	r0, r5
    add	r0, #213
    bl	EEPROMRegistrationCheck
    mov	r1, r0
    cmp	r1, #0
    bne	MATASK_GetEEPROMData+0xee
    mov	r2, r5
    add	r2, #102
    mov	r0, #37
    b	MATASK_GetEEPROMData+0x148
    ldr	r2, [pc, #44]
    add	r0, r5, r2
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #128
    bl	MAU_memcpy
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #128
    mov	r2, #64
    bl	MABIOS_EEPROM_Read
    b	MATASK_GetEEPROMData+0x228
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    ldr	r2, [pc, #52]
    add	r0, r5, r2
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #64
    bl	MAU_memcpy
    ldr	r1, [pc, #36]
    add	r0, r5, r1
    bl	EEPROMSumCheck
    mov	r1, r0
    cmp	r1, #0
    bne	MATASK_GetEEPROMData+0x160
    mov	r2, r5
    add	r2, #102
    mov	r0, #20
    strb	r0, [r2, #0]
    mov	r0, r5
    add	r0, #104
    strh	r1, [r0, #0]
    ldrb	r0, [r4, #0]
    mov	r0, #250
    strb	r0, [r4, #0]
    b	MATASK_GetEEPROMData+0x2d6
    lsl	r5, r7, #19
    lsl	r0, r0, #0
    lsl	r5, r7, #17
    lsl	r0, r0, #0
    mov	r1, r5
    add	r1, #101
    mov	r0, #1
    strb	r0, [r1, #0]
    b	MATASK_GetEEPROMData+0x224
    ldr	r0, [r5, #112]
    ldr	r1, [r5, #116]
    bl	CopyEEPROMData
    b	MATASK_GetEEPROMData+0x228
    mov	r0, #0
    bl	MA_ChangeSIOMode
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
    ldr	r1, [pc, #96]
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
    ldr	r0, [pc, #68]
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
    b	MATASK_GetEEPROMData+0x2d6
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_GetEEPROMData+0x2d6
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
    ldr	r1, [pc, #116]
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
    ldr	r0, [pc, #88]
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
    mov	r0, r5
    add	r0, #102
    ldrb	r0, [r0, #0]
    mov	r1, r5
    add	r1, #94
    ldrh	r1, [r1, #0]
    bl	MA_SetApiError
    mov	r0, #0
    mov	r1, #0
    bl	MA_TaskSet
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_GetEEPROMData, .-MATASK_GetEEPROMData
");
#endif

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_EEPROM_Read:
    push	{r4, r5, lr}
    ldr	r4, [pc, #48]
    mov	r0, r4
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_EEPROM_Read+0x46
    mov	r0, r4
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #17
    beq	MATASK_EEPROM_Read+0x46
    cmp	r0, #17
    blt	MATASK_EEPROM_Read+0x38
    cmp	r0, #25
    bgt	MATASK_EEPROM_Read+0x38
    cmp	r0, #24
    blt	MATASK_EEPROM_Read+0x38
    bl	MA_DefaultNegaResProc
    mov	r0, r4
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_EEPROM_Read+0x44
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #36]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #3
    beq	MATASK_EEPROM_Read+0xae
    cmp	r0, #3
    bgt	MATASK_EEPROM_Read+0x68
    cmp	r0, #1
    beq	MATASK_EEPROM_Read+0x84
    cmp	r0, #1
    bgt	MATASK_EEPROM_Read+0x94
    cmp	r0, #0
    beq	MATASK_EEPROM_Read+0x7e
    b	MATASK_EEPROM_Read+0x252
.align 2
    .word gMA

    cmp	r0, #5
    beq	MATASK_EEPROM_Read+0xf0
    cmp	r0, #5
    blt	MATASK_EEPROM_Read+0xda
    cmp	r0, #250
    bne	MATASK_EEPROM_Read+0x76
    b	MATASK_EEPROM_Read+0x1a0
    cmp	r0, #251
    bne	MATASK_EEPROM_Read+0x7c
    b	MATASK_EEPROM_Read+0x1ae
    b	MATASK_EEPROM_Read+0x252
    bl	MABIOS_Start
    b	MATASK_EEPROM_Read+0x1a4
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_EEPROM_Read+0x252
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #0
    mov	r2, #128
    bl	MABIOS_EEPROM_Read
    b	MATASK_EEPROM_Read+0x1a4
    ldr	r0, [r5, #112]
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #128
    bl	MAU_memcpy
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    mov	r1, #128
    mov	r2, #64
    bl	MABIOS_EEPROM_Read
    b	MATASK_EEPROM_Read+0x1a4
    ldr	r0, [r5, #112]
    add	r0, #128
    mov	r2, #242
    lsl	r2, r2, #1
    add	r1, r5, r2
    ldr	r1, [r1, #0]
    add	r1, #1
    mov	r2, #64
    bl	MAU_memcpy
    b	MATASK_EEPROM_Read+0x1a0
    mov	r0, #0
    bl	MA_ChangeSIOMode
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
    ldr	r1, [pc, #96]
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
    ldr	r0, [pc, #68]
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
    b	MATASK_EEPROM_Read+0x252
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_EEPROM_Read+0x252
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
    ldr	r1, [pc, #116]
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
    ldr	r0, [pc, #88]
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
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_EEPROM_Read, .-MATASK_EEPROM_Read
");
#endif

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

#if 0
#else
asm("
.align 2
.thumb_func
MATASK_EEPROM_Write:
    push	{r4, r5, lr}
    ldr	r1, [pc, #32]
    mov	r0, r1
    add	r0, #69
    ldrb	r0, [r0, #0]
    cmp	r0, #238
    bne	MATASK_EEPROM_Write+0x4e
    mov	r0, r1
    add	r0, #80
    ldrb	r0, [r0, #0]
    cmp	r0, #24
    beq	MATASK_EEPROM_Write+0x2c
    cmp	r0, #24
    bgt	MATASK_EEPROM_Write+0x28
    cmp	r0, #17
    beq	MATASK_EEPROM_Write+0x4e
    b	MATASK_EEPROM_Write+0x40
.align 2
    .word gMA

    cmp	r0, #26
    bne	MATASK_EEPROM_Write+0x40
    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #8]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #250
    b	MATASK_EEPROM_Write+0x4c
.align 2
    .word gMA

    bl	MA_DefaultNegaResProc
    ldr	r0, [pc, #36]
    add	r0, #98
    ldrb	r1, [r0, #0]
    mov	r1, #251
    strb	r1, [r0, #0]
    ldr	r5, [pc, #28]
    mov	r4, r5
    add	r4, #98
    ldrb	r0, [r4, #0]
    cmp	r0, #3
    beq	MATASK_EEPROM_Write+0xba
    cmp	r0, #3
    bgt	MATASK_EEPROM_Write+0x70
    cmp	r0, #1
    beq	MATASK_EEPROM_Write+0x8e
    cmp	r0, #1
    bgt	MATASK_EEPROM_Write+0x9e
    cmp	r0, #0
    beq	MATASK_EEPROM_Write+0x88
    b	MATASK_EEPROM_Write+0x23a
.align 2
    .word gMA

    cmp	r0, #5
    beq	MATASK_EEPROM_Write+0xd8
    cmp	r0, #5
    bge	MATASK_EEPROM_Write+0x7a
    b	MATASK_EEPROM_Write+0x188
    cmp	r0, #250
    bne	MATASK_EEPROM_Write+0x80
    b	MATASK_EEPROM_Write+0x188
    cmp	r0, #251
    bne	MATASK_EEPROM_Write+0x86
    b	MATASK_EEPROM_Write+0x196
    b	MATASK_EEPROM_Write+0x23a
    bl	MABIOS_Start
    b	MATASK_EEPROM_Write+0x18c
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    mov	r0, #1
    bl	MABIOS_ChangeClock
    b	MATASK_EEPROM_Write+0x23a
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r2, [r5, #112]
    mov	r1, #0
    mov	r3, #128
    bl	MABIOS_EEPROM_Write
    b	MATASK_EEPROM_Write+0x18c
    mov	r1, #240
    lsl	r1, r1, #1
    add	r0, r5, r1
    mov	r1, #0
    strh	r1, [r0, #0]
    mov	r1, r5
    add	r1, #212
    str	r1, [r0, #4]
    ldr	r2, [r5, #112]
    add	r2, #128
    mov	r1, #128
    mov	r3, #64
    bl	MABIOS_EEPROM_Write
    b	MATASK_EEPROM_Write+0x18c
    mov	r0, #0
    bl	MA_ChangeSIOMode
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
    ldr	r1, [pc, #96]
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
    ldr	r0, [pc, #68]
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
    b	MATASK_EEPROM_Write+0x23a
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef

    bl	MABIOS_End
    ldrb	r0, [r4, #0]
    add	r0, #1
    ldrb	r1, [r4, #0]
    strb	r0, [r4, #0]
    b	MATASK_EEPROM_Write+0x23a
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
    ldr	r1, [pc, #116]
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
    ldr	r0, [pc, #88]
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
    pop	{r4, r5}
    pop	{r0}
    bx	r0
.align 2
    .word 0xfffffdff
    .word 0xfffffbff
    .word 0xffffdfff
    .word 0x0000fff7
    .word 0x0000ffef
.size MATASK_EEPROM_Write, .-MATASK_EEPROM_Write
");
#endif

#if 0
#else
void MA_GetTel(MA_TELDATA *pTelData);
asm("
.align 2
.thumb_func
.global MA_GetTel
MA_GetTel:
    push	{r4, r5, lr}
    mov	r5, r0
    bl	SetApiCallFlag
    mov	r0, #24
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_GetTel+0x18
    bl	ResetApiCallFlag
    b	MA_GetTel+0x8e
    mov	r0, r5
    mov	r1, #36
    mov	r2, #0
    bl	MAU_memset
    ldr	r4, [pc, #36]
    mov	r0, r4
    add	r0, #101
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	MA_GetTel+0x50
    mov	r0, #24
    mov	r1, r5
    bl	CopyEEPROMData
    bl	ResetApiCallFlag
    ldrh	r0, [r4, #2]
    ldr	r1, [pc, #12]
    and	r1, r0
    ldrh	r0, [r4, #2]
    strh	r1, [r4, #2]
    b	MA_GetTel+0x8e
.align 2
    .word gMA
    .word 0x0000fffe

    mov	r0, #24
    str	r0, [r4, #112]
    str	r5, [r4, #116]
    ldr	r5, [pc, #60]
    mov	r0, #0
    str	r0, [r5, #0]
    mov	r0, #27
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r1, [r4, #2]
    mov	r0, #1
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    bl	ResetApiCallFlag
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    and	r0, r1
    cmp	r0, #0
    beq	MA_GetTel+0x8a
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #16
    and	r0, r1
    cmp	r0, #0
    bne	MA_GetTel+0x8e
    bl	MAAPI_Main
    pop	{r4, r5}
    pop	{r0}
    bx	r0
    lsl	r4, r1, #4
    lsl	r0, r0, #16
.size MA_GetTel, .-MA_GetTel
");
#endif

#if 0
#else
void MA_GetUserID(char *pUserIDBuf);
asm("
.align 2
.thumb_func
.global MA_GetUserID
MA_GetUserID:
    push	{r4, r5, lr}
    mov	r5, r0
    bl	SetApiCallFlag
    mov	r0, #25
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_GetUserID+0x18
    bl	ResetApiCallFlag
    b	MA_GetUserID+0x8c
    ldr	r4, [pc, #32]
    mov	r0, r4
    add	r0, #101
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	MA_GetUserID+0x44
    mov	r0, #25
    mov	r1, r5
    bl	CopyEEPROMData
    bl	ResetApiCallFlag
    ldrh	r0, [r4, #2]
    ldr	r1, [pc, #12]
    and	r1, r0
    ldrh	r0, [r4, #2]
    strh	r1, [r4, #2]
    b	MA_GetUserID+0x8c
.align 2
    .word gMA
    .word 0x0000fffe

    mov	r0, r5
    mov	r1, #33
    mov	r2, #0
    bl	MAU_memset
    mov	r0, #25
    str	r0, [r4, #112]
    str	r5, [r4, #116]
    ldr	r5, [pc, #60]
    mov	r0, #0
    str	r0, [r5, #0]
    mov	r0, #27
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r1, [r4, #2]
    mov	r0, #1
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    bl	ResetApiCallFlag
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    and	r0, r1
    cmp	r0, #0
    beq	MA_GetUserID+0x88
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #16
    and	r0, r1
    cmp	r0, #0
    bne	MA_GetUserID+0x8c
    bl	MAAPI_Main
    pop	{r4, r5}
    pop	{r0}
    bx	r0
    lsl	r0, r0, #0
    lsl	r4, r1, #4
    lsl	r0, r0, #16
.size MA_GetUserID, .-MA_GetUserID
");
#endif

#if 0
#else
void MA_GetMailID(char *pBufPtr);
asm("
.align 2
.thumb_func
.global MA_GetMailID
MA_GetMailID:
    push	{r4, r5, lr}
    mov	r5, r0
    bl	SetApiCallFlag
    mov	r0, #26
    bl	MA_ApiPreExe
    cmp	r0, #0
    bne	MA_GetMailID+0x18
    bl	ResetApiCallFlag
    b	MA_GetMailID+0x8e
    mov	r0, r5
    mov	r1, #31
    mov	r2, #0
    bl	MAU_memset
    ldr	r4, [pc, #36]
    mov	r0, r4
    add	r0, #101
    ldrb	r0, [r0, #0]
    cmp	r0, #1
    bne	MA_GetMailID+0x50
    mov	r0, #26
    mov	r1, r5
    bl	CopyEEPROMData
    bl	ResetApiCallFlag
    ldrh	r0, [r4, #2]
    ldr	r1, [pc, #12]
    and	r1, r0
    ldrh	r0, [r4, #2]
    strh	r1, [r4, #2]
    b	MA_GetMailID+0x8e
.align 2
    .word gMA
    .word 0x0000fffe

    mov	r0, #26
    str	r0, [r4, #112]
    str	r5, [r4, #116]
    ldr	r5, [pc, #60]
    mov	r0, #0
    str	r0, [r5, #0]
    mov	r0, #27
    mov	r1, #0
    bl	MA_TaskSet
    ldrh	r1, [r4, #2]
    mov	r0, #1
    ldrh	r2, [r4, #2]
    orr	r0, r1
    strh	r0, [r4, #2]
    bl	ResetApiCallFlag
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #15
    and	r0, r1
    cmp	r0, #0
    beq	MA_GetMailID+0x8a
    ldr	r0, [r5, #0]
    mov	r1, #128
    lsl	r1, r1, #16
    and	r0, r1
    cmp	r0, #0
    bne	MA_GetMailID+0x8e
    bl	MAAPI_Main
    pop	{r4, r5}
    pop	{r0}
    bx	r0
    lsl	r4, r1, #4
    lsl	r0, r0, #16
.size MA_GetMailID, .-MA_GetMailID
");
#endif

void MA_GetSMTPServerName(char *dest)
{
    CopyEEPROMString(dest, gMA.smtp_server, sizeof(gMA.smtp_server));
}

void MA_GetPOP3ServerName(char *dest)
{
    CopyEEPROMString(dest, gMA.pop3_server, sizeof(gMA.pop3_server));
}

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

#if 0
#else
asm("
.align 2
.thumb_func
.global MAAPI_Main
MAAPI_Main:
    push	{lr}
    ldr	r0, [pc, #168]
    mov	r2, r0
    add	r2, #97
    ldrb	r1, [r2, #0]
    mov	r3, r0
    cmp	r1, #1
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #2
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #3
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #4
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #8
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #9
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #5
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #27
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #28
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #29
    beq	MAAPI_Main+0x56
    ldrb	r0, [r2, #0]
    cmp	r0, #30
    beq	MAAPI_Main+0x56
    ldr	r0, [r3, #64]
    mov	r1, #1
    and	r0, r1
    cmp	r0, #0
    beq	MAAPI_Main+0xa8
    ldr	r0, [r3, #64]
    mov	r1, #4
    and	r0, r1
    cmp	r0, #0
    bne	MAAPI_Main+0xa8
    ldrh	r1, [r3, #2]
    mov	r0, #2
    and	r0, r1
    cmp	r0, #0
    bne	MAAPI_Main+0xa8
    ldrh	r1, [r3, #2]
    mov	r0, #32
    and	r0, r1
    cmp	r0, #0
    bne	MAAPI_Main+0xa8
    ldr	r0, [r3, #64]
    mov	r1, #128
    lsl	r1, r1, #2
    and	r0, r1
    cmp	r0, #0
    beq	MAAPI_Main+0x8e
    mov	r0, r3
    add	r0, #68
    ldrb	r0, [r0, #0]
    cmp	r0, #21
    bne	MAAPI_Main+0x8e
    bl	MATASK_P2P
    ldr	r0, [pc, #28]
    mov	r1, r0
    add	r1, #97
    ldrb	r0, [r1, #0]
    cmp	r0, #0
    beq	MAAPI_Main+0xa8
    ldr	r0, [pc, #20]
    ldrb	r1, [r1, #0]
    lsl	r1, r1, #2
    add	r1, r1, r0
    ldr	r0, [r1, #0]
    bl	_call_via_r0
    pop	{r0}
    bx	r0
.align 2
    .word gMA
    .word taskProcTable
.size MAAPI_Main, .-MAAPI_Main
");
#endif

u16 MAAPI_GetConditionFlag(void)
{
    return gMA.condition;
}

#if 0
#else
asm("
.align 2
.thumb_func
ErrDetailHexConv:
    push	{r4, r5, lr}
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    mov	r2, #0
    ldr	r0, [pc, #60]
    cmp	r1, r0
    bls	ErrDetailHexConv+0x2a
    ldr	r3, [pc, #60]
    mov	r5, r3
    mov	r3, #128
    lsl	r3, r3, #5
    mov	r4, r3
    mov	r3, r0
    add	r0, r1, r5
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    add	r0, r2, r4
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    cmp	r1, r3
    bhi	ErrDetailHexConv+0x1a
    cmp	r1, #99
    bls	ErrDetailHexConv+0x60
    mov	r0, #128
    lsl	r0, r0, #1
    mov	r3, r0
    mov	r0, r1
    sub	r0, #100
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    add	r0, r2, r3
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    cmp	r1, #99
    bhi	ErrDetailHexConv+0x34
    b	ErrDetailHexConv+0x60
.align 2
    .word 0x000003e7
    .word 0xfffffc18

    mov	r0, r1
    sub	r0, #10
    lsl	r0, r0, #16
    lsr	r1, r0, #16
    mov	r0, r2
    add	r0, #16
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    cmp	r1, #9
    bhi	ErrDetailHexConv+0x50
    add	r0, r2, r1
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    mov	r0, r2
    pop	{r4, r5}
    pop	{r1}
    bx	r1
.size ErrDetailHexConv, .-ErrDetailHexConv
");
#endif

#if 0
#else
u8 MAAPI_ErrorCheck(u16 *pProtocolError);
asm("
.align 2
.thumb_func
.global MAAPI_ErrorCheck
MAAPI_ErrorCheck:
    push	{r4, lr}
    mov	r4, r0
    ldr	r2, [pc, #20]
    ldrh	r1, [r2, #2]
    mov	r3, #2
    mov	r0, r3
    and	r0, r1
    lsl	r0, r0, #16
    lsr	r0, r0, #16
    cmp	r0, #0
    bne	MAAPI_ErrorCheck+0x20
    strh	r0, [r4, #0]
    mov	r0, #0
    b	MAAPI_ErrorCheck+0xda
.align 2
    .word gMA

    ldrb	r0, [r2, #0]
    cmp	r0, #132
    beq	MAAPI_ErrorCheck+0x4c
    cmp	r0, #132
    bgt	MAAPI_ErrorCheck+0x30
    cmp	r0, #131
    beq	MAAPI_ErrorCheck+0x3a
    b	MAAPI_ErrorCheck+0x7c
    cmp	r0, #133
    beq	MAAPI_ErrorCheck+0x6c
    cmp	r0, #135
    beq	MAAPI_ErrorCheck+0x5c
    b	MAAPI_ErrorCheck+0x7c
    ldrb	r0, [r2, #0]
    mov	r1, #0
    mov	r0, #21
    strb	r0, [r2, #0]
    mov	r0, r2
    add	r0, #94
    ldrh	r2, [r0, #0]
    strh	r1, [r0, #0]
    b	MAAPI_ErrorCheck+0x7c
    ldrb	r0, [r2, #0]
    mov	r0, #21
    strb	r0, [r2, #0]
    mov	r1, r2
    add	r1, #94
    ldrh	r0, [r1, #0]
    mov	r0, #1
    b	MAAPI_ErrorCheck+0x7a
    ldrb	r0, [r2, #0]
    mov	r0, #21
    strb	r0, [r2, #0]
    mov	r0, r2
    add	r0, #94
    ldrh	r1, [r0, #0]
    strh	r3, [r0, #0]
    b	MAAPI_ErrorCheck+0x7c
    ldrb	r0, [r2, #0]
    mov	r0, #21
    strb	r0, [r2, #0]
    mov	r1, r2
    add	r1, #94
    ldrh	r0, [r1, #0]
    mov	r0, #3
    strh	r0, [r1, #0]
    ldr	r2, [pc, #44]
    ldrh	r0, [r2, #2]
    ldr	r1, [pc, #44]
    and	r1, r0
    ldrh	r0, [r2, #2]
    strh	r1, [r2, #2]
    cmp	r4, #0
    beq	MAAPI_ErrorCheck+0xb8
    ldrb	r0, [r2, #0]
    cmp	r0, #21
    beq	MAAPI_ErrorCheck+0x9e
    cmp	r0, #21
    blt	MAAPI_ErrorCheck+0xb4
    cmp	r0, #51
    bgt	MAAPI_ErrorCheck+0xb4
    cmp	r0, #48
    blt	MAAPI_ErrorCheck+0xb4
    mov	r0, r2
    add	r0, #94
    ldrh	r0, [r0, #0]
    bl	ErrDetailHexConv
    b	MAAPI_ErrorCheck+0xb6
.align 2
    .word gMA
    .word 0x0000fffd

    mov	r0, #0
    strh	r0, [r4, #0]
    ldr	r3, [pc, #36]
    ldrh	r1, [r3, #2]
    mov	r0, #32
    and	r0, r1
    lsl	r0, r0, #16
    lsr	r2, r0, #16
    cmp	r2, #0
    bne	MAAPI_ErrorCheck+0xd8
    mov	r0, r3
    add	r0, #68
    ldrb	r1, [r0, #0]
    strb	r2, [r0, #0]
    mov	r1, r3
    add	r1, #69
    ldrb	r0, [r1, #0]
    strb	r2, [r1, #0]
    ldrb	r0, [r3, #0]
    pop	{r4}
    pop	{r1}
    bx	r1
.align 2
    .word gMA
.size MAAPI_ErrorCheck, .-MAAPI_ErrorCheck
");
#endif
