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
static void MA_HTTP_GetPost(const char *pURL, char *pHeadBuf, u16 headBufSize, const u8 *pSendData, u16 sendSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword, u8 task);
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
    gMA.task_step = unk_2;
    if (gMA.task == 0) gMA.condition &= ~MA_CONDITION_APIWAIT;
}

static void MA_SetApiError(u8 error, u16 unk_2)
{
    gMA.unk_96 = 0;
    gMA.error_unk_94 = unk_2;
    MA_SetError(error);
}

static int ApiValisStatusCheck(u8 unk_1)
{
    static int ret;

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
    gMA.error_unk_94 = 0;

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
    gMA.task_error = 0;
    gMA.task_error_unk_2 = 0;
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
    static const char strEndMultiLine[] = "\r\n.\r\n";

    if (MAU_strcmp(gMA.unk_1788, strEndMultiLine) == 0) {
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
    switch (gMA.task_step) {  // MAGIC
    case 0:
        gMA.task_step++;

    case 1:
        MA_BiosStop();
        gMA.task_step++;
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

        gMA.cmd_recv = 0;
        MA_TaskSet(TASK_UNK_00, 0);
        break;
    }
}

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

    if (gMA.task == TASK_UNK_1F) {
        ResetApiCallFlag();
        return;
    }

    if (gMA.task == TASK_UNK_1E) {
        MA_SetApiError(MAAPIE_CANNOT_EXECUTE, 0);
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = gMA.cmd_cur;
    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
    if (!(gMA.condition & MA_CONDITION_UNK_5 || gMA.unk_92 == 3)) {
        gMA.condition |= MA_CONDITION_APIWAIT;
        MA_TaskSet(TASK_UNK_1F, 2);
        ResetApiCallFlag();
    } else {
        gMA.condition |= MA_CONDITION_APIWAIT;
        MA_TaskSet(TASK_UNK_1F, 0);
        ResetApiCallFlag();
    }
}

static void MATASK_TCP_Cut(void)
{
    int param = gMA.task_step;

    switch (param) {
    case 0:
        gMA.task_step++;

    case 1:
        if (gMA.unk_112 == 0x23 &&  // MAGIC
                gMA.cmd_recv != (MACMD_ERROR | MAPROT_REPLY)) {
            gMA.sockets[0] = gMA.buffer_unk_480.data[0];
            gMA.task_step++;
        } else if (gMA.sockets_used[0] != TRUE) {
            gMA.task_step = 3;
            break;
        } else {
            gMA.task_step++;
        }

    case 2:
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 3:
        gMA.unk_92 = param;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        gMA.cmd_recv = 0;
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
    gMA.unk_112 = (u32)pHardwareType;
    MA_TaskSet(task, 0);

    gMA.condition |= MA_CONDITION_APIWAIT;
    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_InitLibrary(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;

    case 2:
        MABIOS_End();
        gMA.task_step++;
        break;

    case 3:
        *(u8 *)gMA.unk_112 = gMA.adapter_type + 0x78;  // MAGIC

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
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)unk_1;
    gMA.unk_116 = (u32)unk_2;
    gMA.unk_120 = unk_3;
    MA_TaskSet(TASK_UNK_20, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_Connect(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_TCPCONNECT:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_step) {  // MAGIC
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPConnect(&gMA.buffer_unk_480, (u8 *)gMA.unk_116, gMA.unk_120);
        gMA.task_step++;
        break;

    case 1:
        *(u8 *)gMA.unk_112 = gMA.buffer_unk_480.data[0];
        MAU_Socket_Add(gMA.buffer_unk_480.data[0]);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        gMA.unk_92 = 3;  // MAGIC
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = unk_1;
    MA_TaskSet(TASK_UNK_21, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_Disconnect(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.unk_112);
        gMA.task_step++;
        break;

    case 1:
        MAU_Socket_Delete(gMA.unk_112);
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

    gMA.unk_112 = unk_1;
    gMA.unk_116 = unk_2;
    gMA.unk_120 = unk_3;
    gMA.unk_124 = unk_4;
    MA_TaskSet(TASK_UNK_22, 0);
    ResetApiCallFlag();
}

static void MATASK_TCP_SendRecv(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf0;
            MAU_Socket_Delete(gMA.unk_112);
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    if (MA_GetCondition() & MA_CONDITION_UNK_6) {
        gMA.task_error = MAAPIE_OFFLINE;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        MAU_Socket_Delete(gMA.unk_112);
    }

    switch (gMA.task_step) {  // MAGIC
    case 0:
        MABIOS_Data(&gMA.buffer_unk_480, (u8 *)gMA.unk_116, gMA.unk_120, gMA.unk_112);
        gMA.task_step++;
        break;

    case 1:
        MAU_memcpy((u8 *)gMA.unk_116, &gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        *(u8 *)gMA.unk_124 = gMA.buffer_unk_480.size - 1;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)unk_1;
    gMA.unk_116 = (u32)unk_2;
    MA_TaskSet(TASK_UNK_23, 0);
    ResetApiCallFlag();
}

static void MATASK_GetHostAddress(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DNSREQUEST:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        default:
            MA_DefaultNegaResProc();
            break;
        }
    }

    switch (gMA.task_step) {  // MAGIC
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_DNSRequest(&gMA.buffer_unk_480, (char *)gMA.unk_116);
        gMA.task_step++;
        break;

    case 1:
        MAU_memcpy((u8 *)gMA.unk_112, gMA.buffer_unk_480.data, 4);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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
    static u16 tmp;
    static u16 sum;
    static int i;

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

    gMA.unk_112 = (u32)pTelNo;
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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
        case MACMD_OFFLINE:
            break;

        case MACMD_TEL:
        case MACMD_CHECKSTATUS:
        case MACMD_CHANGECLOCK:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        case MACMD_EEPROM_READ:
            gMA.task_error = MAAPIE_EEPROM_SUM;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;

        case MACMD_PPPCONNECT:
            gMA.task_error = MAAPIE_PPP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf9;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_step++;
        break;

    case 3:
        if (gMA.buffer_unk_480.data[0] == 0xff) {
            gMA.task_error = MAAPIE_CALL_FAILED;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;
        }
        if (gMA.unk_101 == 0) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
            gMA.task_step++;
            break;
        }
        gMA.task_step = 6;
        break;

    case 4:
        if (!EEPROMRegistrationCheck(&gMA.unk_212[1])) {
            gMA.task_error = MAAPIE_REGISTRATION;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            return;
        }
        MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_step++;
        break;

    case 5:
        MAU_memcpy(&gMA.eeprom_unk_1275[2], &gMA.buffer_unk_480.data[1], 0x40);
        if (!EEPROMSumCheck(gMA.prevbuf)) {
            gMA.task_error = MAAPIE_EEPROM_SUM;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            return;
        }
        gMA.unk_101 = 1;
        gMA.task_step++;
        break;

    case 6:
        MAU_memcpy(gMA.unk_828, &gMA.prevbuf[4], 8);
        MAU_memcpy(gMA.smtp_server, gMA.eeprom_unk_1223, 0x2c);
        gMA.task_step++;
        break;

    case 7:
        MABIOS_Tel(MA_GetCallTypeFromHarwareType(gMA.adapter_type), (char *)gMA.unk_112);
        gMA.task_step++;
        break;

    case 8:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_PPPConnect(&gMA.buffer_unk_480, (char *)gMA.unk_116, (char *)gMA.unk_120, gMA.unk_828, gMA.unk_832);
        gMA.task_step++;
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
        gMA.task_step++;
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)pTelNo;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_04, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_Tel(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_TEL:
        case MACMD_CHECKSTATUS:
        case MACMD_CHANGECLOCK:
        case MACMD_EEPROM_READ:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_step++;
        break;

    case 3:
        if (gMA.buffer_unk_480.data[0] == 0xff) {
            gMA.task_error = MAAPIE_CALL_FAILED;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;
        }

        MABIOS_Tel(MA_GetCallTypeFromHarwareType(gMA.adapter_type), (char *)gMA.unk_112);
        gMA.task_step++;
        break;

    case 4:
        gMA.status |= STATUS_UNK_9;
        gMA.unk_92 = 7;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_P2P_SEND << MA_CONDITION_SHIFT;
        gMA.unk_112 = 0;
        gMA.buffer_unk_480.size = 0;
        InitPrevBuf();
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.error_unk_94);
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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_WAITCALL:
            if (gMA.unk_81 != 0) {
                MA_DefaultNegaResProc();
                gMA.task_step = 0xfa;
            }
            break;

        case MACMD_CHANGECLOCK:
        case MACMD_EEPROM_READ:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        MABIOS_WaitCall();
        gMA.task_step++;
        break;

    case 3:
        if (gMA.cmd_recv != (MACMD_WAITCALL | MAPROT_REPLY)) {
            gMA.task_step--;
            break;
        }
        gMA.task_step++;
        break;

    case 4:
        gMA.status |= STATUS_UNK_9;
        gMA.unk_92 = 8;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_P2P_RECV << MA_CONDITION_SHIFT;
        gMA.unk_112 = 0;
        gMA.buffer_unk_480.size = 0;
        InitPrevBuf();
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfa:
        MABIOS_End();
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.error_unk_94);
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

    gMA.unk_112 = (u32)pSendData;
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

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
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

    gMA.unk_112 = (u32)pCondition;
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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_CHECKSTATUS:
            MA_DefaultNegaResProc();
            if (gMA.unk_116 == 1) {
                gMA.task_step = 0xfa;
            } else {
                gMA.task_step = 0xfc;
            }
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_84;
        MABIOS_CheckStatus(&gMA.buffer_unk_480);
        gMA.task_step++;
        break;

    case 2:
        *(u8 *)gMA.unk_112 = MA_ProcessCheckStatusResponse(gMA.buffer_unk_480.data[0]);
        if (0xef < gMA.unk_84[2]) {  // MAGIC
            *(u8 *)gMA.unk_112 |= 0x80;  // MAGIC
        }
        if (gMA.unk_116 == 1) {
            gMA.task_step = 3;
        } else {
            MA_TaskSet(TASK_UNK_00, 0);
        }
        break;

    case 3:
        MABIOS_End();
        gMA.task_step++;
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
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xfc:
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_step = 0;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        if (gMA.unk_92 != 7 && gMA.unk_92 != 8) {
            MABIOS_PPPDisconnect();
        }
        gMA.task_step++;
        break;

    case 1:
        MABIOS_Offline();
        gMA.task_step++;
        break;

    case 2:
        MABIOS_End();
        gMA.task_step++;
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
        gMA.task_step++;
        break;

    case 101:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }
        gMA.unk_112 = gMA.counter_timeout200msec[gMA.sio_mode];
        gMA.task_step++;
        break;

    case 102:
        if (--gMA.unk_112 != 0) break;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 103:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        gMA.sockets[0] = 0;
        gMA.sockets_used[0] = FALSE;
        gMA.task_step = 0;
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
    gMA.unk_112 = (u32)pMailAddress;
    MA_TaskSet(TASK_UNK_0B, 0);
    ResetApiCallFlag();
}

static void MATASK_SMTP_Connect(void)
{
    static char *cp1, *cp2;
    static int smtpRes;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPCONNECT:
        case MACMD_DNSREQUEST:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf1;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_DNSRequest(&gMA.buffer_unk_480, gMA.unk_880);
        gMA.task_step++;
        break;

    case 1:
        MAU_memcpy(gMA.ipaddr, gMA.buffer_unk_480.data, 4);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPConnect(&gMA.buffer_unk_480, gMA.ipaddr, 25);
        gMA.task_step++;
        break;

    case 2:
        gMA.sockets[0] = gMA.buffer_unk_480.data[0];
        gMA.sockets_used[0] = TRUE;
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 3:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 220) {
                MAU_strcpy(gMA.unk_880, POP3_Helo);
                cp1 = &gMA.unk_880[5];
                cp2 = (u8 *)gMA.unk_112;
                while (*cp2 && *cp2 != '@') *cp1++ = *cp2++;
                *cp1 = '\0';
                MAU_strcat(gMA.unk_880, POP3_Newl);

                InitPrevBuf();
                (&gMA.buffer_unk_480)->size = 0;
                (&gMA.buffer_unk_480)->data = gMA.unk_212;
                MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
                gMA.task_step++;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        break;

    case 4:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 250) {
                gMA.task_step++;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        break;

    case 5:
        gMA.unk_92 = 4;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_SMTP << MA_CONDITION_SHIFT;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

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

    gMA.unk_112 = (u32)pRecipients;
    MA_TaskSet(TASK_UNK_0C, 0);
    ResetApiCallFlag();
}

static void MATASK_SMTP_Sender(void)
{
    static int smtpRes;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        MAU_strcpy(gMA.unk_880, POP3_From);
        MAU_strcat(gMA.unk_880, *(char **)gMA.unk_112);
        MAU_strcat(gMA.unk_880, POP3_From_Newl);
        gMA.unk_112 += 4;
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
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
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 250) {
                gMA.task_step++;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
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
            gMA.task_step++;
            break;
        }
        gMA.task_step = 4;
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
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 250) {
                gMA.task_step = 2;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        break;

    case 4:
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.unk_96 = 1;
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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
    gMA.unk_112 = (u32)pSendData;
    gMA.unk_116 = sendSize;
    gMA.unk_120 = endFlag;
    gMA.unk_124 += sendSize;
    MA_TaskSet(TASK_UNK_0D, 0);
    ResetApiCallFlag();
}

static void MATASK_SMTP_Send(void)
{
    static int smtpRes;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        if (gMA.unk_96 == 1) {
            gMA.unk_96 = 0;
            gMA.task_step = 100;
            break;
        }
        gMA.task_step++;

    case 1:
        if (gMA.unk_116 >= 0xff) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, (u8 *)gMA.unk_112, 254, gMA.sockets[0]);
            gMA.unk_112 += 254;
            gMA.unk_116 -= 254;
            gMA.unk_132 = gMA.task_step;
            gMA.task_step = 50;
            break;
        }
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, (u8 *)gMA.unk_112, gMA.unk_116, gMA.sockets[0]);
        if (gMA.unk_120 == 1) {
            gMA.unk_132 = gMA.task_step + 1;
            gMA.task_step = 50;
            break;
        }

        gMA.unk_132 = 3;
        gMA.task_step = 50;
        break;

    case 2:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 250) {
                gMA.task_step++;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        break;

    case 3:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 50:
        gMA.unk_128 = gMA.counter_adapter[gMA.sio_mode];
        gMA.task_step++;

    case 51:
        if (gMA.unk_128-- == 0) gMA.task_step = gMA.unk_132;
        break;

    case 100:
        MAU_strcpy(gMA.unk_880, POP3_Data);
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
        gMA.unk_132 = gMA.task_step;
        gMA.task_step = 50;
        break;

    case 101:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        } 

        smtpRes = CheckSMTPResponse(gMA.prevbuf);
        if (!smtpRes) {
            gMA.error_unk_94 =
                (gMA.prevbuf[0] - '0') * 100 +
                (gMA.prevbuf[1] - '0') * 10 +
                (gMA.prevbuf[2] - '0');
            if (gMA.error_unk_94 == 354) {
                gMA.task_step = 1;
                break;
            }

            gMA.task_error = MAAPIE_SMTP;
            gMA.task_error_unk_2 = gMA.error_unk_94;
            gMA.task_step = 0xf0;
            break;
        }

        gMA.task_error = MAAPIE_SMTP;
        gMA.task_error_unk_2 = 0;
        gMA.task_step = 0xf0;
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        InitPrevBuf();
        MAU_strcpy(gMA.unk_880, POP3_Quit);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 1:
        ConcatPrevBuf(gMA.buffer_unk_480.data + 1, gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        gMA.unk_112 = gMA.counter_timeout200msec[gMA.sio_mode];
        gMA.task_step++;
        break;

    case 2:
        gMA.unk_112--;
        if (gMA.unk_112 != 0) break;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
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
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)gMA.unk_880;
    MA_GetPOP3ServerName((char *)gMA.unk_112);

    gMA.unk_116 = gMA.unk_112 + MAU_strlen((char *)gMA.unk_112) + 1;
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

static void MATASK_POP3_Connect(void)
{
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPCONNECT:
        case MACMD_DNSREQUEST:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf1;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_DNSRequest(&gMA.buffer_unk_480, (char *)gMA.unk_112);
        gMA.task_step++;
        break;

    case 1:
        MAU_memcpy(gMA.ipaddr, gMA.buffer_unk_480.data, sizeof(gMA.ipaddr));
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPConnect(&gMA.buffer_unk_480, gMA.ipaddr, 110);  // MAGIC
        gMA.task_step++;
        break;

    case 2:
        gMA.sockets[0] = gMA.buffer_unk_480.data[0];
        gMA.sockets_used[0] = TRUE;
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 3:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            InitPrevBuf();
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, (char *)gMA.unk_116, MAU_strlen((char *)gMA.unk_116), gMA.sockets[0]);
            gMA.task_step++;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 4:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            InitPrevBuf();
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, (char *)gMA.unk_120, MAU_strlen((char *)gMA.unk_120), gMA.sockets[0]);
            gMA.task_step++;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 2;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 5:
        ConcatPrevBuf(&gMA.buffer_unk_480.data[1], gMA.buffer_unk_480.size - 1);
        if (!MAU_CheckCRLF(gMA.prevbuf, gMA.prevbuf_size)) {
            (&gMA.buffer_unk_480)->size = 0;
            (&gMA.buffer_unk_480)->data = gMA.unk_212;
            MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
            break;
        }

        pop3res = CheckPOP3Response(gMA.prevbuf);
        if (pop3res == 0) {
            gMA.task_step++;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 3;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 6:
        gMA.unk_92 = 5;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_POP3 << MA_CONDITION_SHIFT;
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

void MA_POP3_Stat(u16 *pNum, u32 *pSize)
{
    SetApiCallFlag();
    if (!MA_ApiPreExe(TASK_UNK_10)) {
        ResetApiCallFlag();
        return;
    }

    gMA.unk_112 = (u32)pNum;
    gMA.unk_116 = (u32)pSize;
    MA_TaskSet(TASK_UNK_10, 0);
    ResetApiCallFlag();
}

static void MATASK_POP3_Stat(void)
{
    static const char *cp;
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        MAU_strcpy(gMA.unk_880, POP3_Stat);
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
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
            gMA.task_step++;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 2:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)pSize;
    MAU_strcpy(gMA.unk_880, POP3_List);
    MAU_itoa(mailNo, &gMA.unk_880[MAU_strlen(gMA.unk_880)], 10);
    MAU_strcat(gMA.unk_880, POP3_Newl);
    MA_TaskSet(TASK_UNK_11, 0);

    ResetApiCallFlag();
}

static void MATASK_POP3_List(void)
{
    static const char *cp;
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
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
            gMA.task_step = 100;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 4;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 100:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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
    gMA.unk_112 = (u32)pRecvData;
    gMA.unk_116 = recvBufSize;
    gMA.unk_120 = (u32)pRecvSize;

    if (gMA.condition & MA_CONDITION_BUFFER_FULL) {
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        if (recvBufSize == 0) {
            gMA.unk_112 = recvBufSize;
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

static void MATASK_POP3_Retr(void)
{
    static const char *cp;
    static int dataLen;
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
        break;

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
                        break;
                    }
                    ConcatPrevBuf(&gMA.buffer_unk_480.data[1], cp - (char *)&gMA.buffer_unk_480.data[1]);

                    pop3res = CheckPOP3Response(gMA.prevbuf);
                    if (pop3res == 0) {
                        dataLen -= cp - (char *)&gMA.buffer_unk_480.data[1];
                        gMA.buffer_unk_480.data = (char *)&cp[-1];
                        gMA.unk_140 = 1;
                    } else if (pop3res == 1) {
                        gMA.task_error = MAAPIE_POP3;
                        gMA.task_error_unk_2 = 4;
                        gMA.task_step = 0xf0;
                        break;
                    } else {
                        gMA.task_error = MAAPIE_POP3;
                        gMA.task_error_unk_2 = 0;
                        gMA.task_step = 0xf0;
                        break;
                    }
                }

                if (gMA.unk_116 >= dataLen) {
                    MAU_memcpy((u8 *)gMA.unk_112, &gMA.buffer_unk_480.data[1], dataLen);
                    gMA.unk_112 += dataLen;
                    gMA.unk_116 -= dataLen;
                    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
                    *(u16 *)gMA.unk_120 += dataLen;
                } else {
                    MAU_memcpy((u8 *)gMA.unk_112, &gMA.buffer_unk_480.data[1], gMA.unk_116);
                    gMA.prevbuf_size = dataLen - gMA.unk_116;
                    MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[gMA.unk_116 + 1], gMA.prevbuf_size);
                    *(u16 *)gMA.unk_120 += gMA.unk_116;
                    gMA.unk_116 = 0;
                    gMA.condition |= MA_CONDITION_BUFFER_FULL;
                    MA_TaskSet(TASK_UNK_00, 0);
                    gMA.condition |= MA_CONDITION_APIWAIT;
                    break;
                }
            }
        } else {
            gMA.status &= ~STATUS_UNK_15;
        }

        if (dataLen >= 5) {
            MakeEndLineBuffer(gMA.buffer_unk_480.data + gMA.buffer_unk_480.size - 5, 5);
            if (IsEndMultiLine() == TRUE) {
                gMA.task_step = 100;
                break;
            }
        } else if (dataLen != 0) {
            MakeEndLineBuffer(&gMA.buffer_unk_480.data[1], dataLen);
            if (IsEndMultiLine() == TRUE) {
                gMA.task_step = 100;
                break;
            }
        }

        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
        break;

    case 100:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

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
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
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
            gMA.task_step++;
        } else if (pop3res == 1) {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 4;
            gMA.task_step = 0xf0;
        } else {
            gMA.task_error = MAAPIE_POP3;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
        }
        break;

    case 2:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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
    gMA.unk_112 = (u32)pRecvData;
    gMA.unk_116 = recvBufSize;
    gMA.unk_120 = (u32)pRecvSize;

    if (gMA.condition & MA_CONDITION_BUFFER_FULL) {
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        if (recvBufSize == 0) {
            gMA.unk_112 = recvBufSize;
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
    static const char *cp;
    static int dataLen;
    static int pop3res;

    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_DATA:
            gMA.task_error = MAAPIE_TCP_CONNECT;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xf0;
            break;

        case MACMD_TCPDISCONNECT:
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xf1;
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

    switch (gMA.task_step) {
    case 0:
        InitPrevBuf();
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, gMA.unk_880, MAU_strlen(gMA.unk_880), gMA.sockets[0]);
        gMA.task_step++;
        break;

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
                        break;
                    }
                    ConcatPrevBuf(&gMA.buffer_unk_480.data[1], cp - (char *)&gMA.buffer_unk_480.data[1]);
                    pop3res = CheckPOP3Response(gMA.prevbuf);
                    if (pop3res == 0) {
                        dataLen -= cp - (char *)&gMA.buffer_unk_480.data[1];
                        gMA.buffer_unk_480.data = (char *)&cp[-1];
                        gMA.unk_140 = 1;
                    } else if (pop3res == 1) {
                        gMA.task_error = MAAPIE_POP3;
                        gMA.task_error_unk_2 = 4;
                        gMA.task_step = 0xf0;
                        break;
                    } else {
                        gMA.task_error = MAAPIE_POP3;
                        gMA.task_error_unk_2 = 0;
                        gMA.task_step = 0xf0;
                        break;
                    }
                }

                if (gMA.unk_116 >= dataLen) {
                    MAU_memcpy((u8 *)gMA.unk_112, &gMA.buffer_unk_480.data[1], dataLen);
                    gMA.unk_112 += dataLen;
                    gMA.unk_116 -= dataLen;
                    gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
                    *(u16 *)gMA.unk_120 += dataLen;
                } else {
                    MAU_memcpy((u8 *)gMA.unk_112, &gMA.buffer_unk_480.data[1], gMA.unk_116);
                    gMA.prevbuf_size = dataLen - gMA.unk_116;
                    MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[gMA.unk_116 + 1], gMA.prevbuf_size);
                    *(u16 *)gMA.unk_120 += gMA.unk_116;
                    gMA.unk_116 = 0;
                    gMA.condition |= MA_CONDITION_BUFFER_FULL;
                    MA_TaskSet(TASK_UNK_00, 0);
                    gMA.condition |= MA_CONDITION_APIWAIT;
                    break;
                }
            }
        } else {
            gMA.status &= ~STATUS_UNK_15;
        }

        if (dataLen >= 5) {
            MakeEndLineBuffer(gMA.buffer_unk_480.data + gMA.buffer_unk_480.size - 5, 5);
            if (IsEndMultiLine() == TRUE) {
                gMA.task_step = 100;
                break;
            }
        } else {
            if (dataLen != 0) {
                MakeEndLineBuffer(&gMA.buffer_unk_480.data[1], dataLen);
                if (IsEndMultiLine() == TRUE) {
                    gMA.task_step = 100;
                    break;
                }
            }
        }
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_Data(&gMA.buffer_unk_480, NULL, 0, gMA.sockets[0]);
        break;

    case 100:
        MA_TaskSet(TASK_UNK_00, 0);
        break;

    case 0xf0:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_TCPDisconnect(&gMA.buffer_unk_480, gMA.sockets[0]);
        gMA.task_step++;
        break;

    case 0xf1:
        gMA.unk_92 = 3;
        gMA.condition &= ~MA_CONDITION_MASK;
        gMA.condition |= MA_CONDITION_PPP << MA_CONDITION_SHIFT;
        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
        MA_TaskSet(TASK_UNK_00, 0);
        gMA.sockets_used[0] = FALSE;
        break;
    }
}

static const char *ExtractServerName(char *pServerName, const char *pURL, u8 *unk_3, u8 *unk_4)
{
    static const char strDownload[] = "gameboy.datacenter.ne.jp/cgb/download";
    static const char strUpload[] = "gameboy.datacenter.ne.jp/cgb/upload";
    static const char strUtility[] = "gameboy.datacenter.ne.jp/cgb/utility";
    static const char strRanking[] = "gameboy.datacenter.ne.jp/cgb/ranking";
    static const char strHttp[] = "http://";

    static const char *cp;
    static const char *tmpp;
    static int len;

    if (MAU_strnicmp(pURL, strHttp, sizeof(strHttp) - 1) == 0) {
        pURL += sizeof(strHttp) - 1;
    }

    cp = MAU_strchr(pURL, '/');
    if (!cp) {
        if (MAU_strchr(pURL, '\0') - pURL > 0xff) {
            *pServerName = '\0';
            return NULL;
        }
        MAU_strcpy(pServerName, pURL);
        *unk_3 = 0;
    } else {
        len = cp - pURL;
        if (len > 0xff) {
            *pServerName = '\0';
            return NULL;
        }

        MAU_memcpy(pServerName, pURL, len);
        pServerName[len] = '\0';

        if (MAU_strnicmp(pURL, strDownload, sizeof(strDownload) - 1) == 0) {
            *unk_4 = 1;
            tmpp = MAU_strrchr(pURL, '/');
            tmpp++;
            if (tmpp[0] >= '0' && tmpp[0] <= '9') {
                *unk_3 = 1;
            } else {
                *unk_3 = 0;
            }
        } else if (MAU_strnicmp(pURL, strUpload, sizeof(strUpload) - 1) == 0) {
            *unk_4 = 2;
            tmpp = MAU_strrchr(pURL, '/');
            tmpp++;
            if (tmpp[0] >= '0' && tmpp[0] <= '9') {
                *unk_3 = 2;
            } else {
                *unk_3 = 0;
            }
        } else if (MAU_strnicmp(pURL, strUtility, sizeof(strUtility) - 1) == 0) {
            *unk_4 = 3;
            *unk_3 = 3;
        } else if (MAU_strnicmp(pURL, strRanking, sizeof(strRanking) - 1) == 0) {
            *unk_4 = 4;
            tmpp = MAU_strrchr(pURL, '/');
            tmpp++;
            if (tmpp[0] >= '0' && tmpp[0] <= '9') {
                *unk_3 = 4;
            } else {
                *unk_3 = 0;
            }
        } else {
            *unk_3 = 0;
        }
    }

    return cp;
}

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
static const char strEmpty[] asm(".LstrEmpty") = "";

void MA_HTTP_Get(const char *pURL, char *pHeadBuf, u16 headBufSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword)
{
    MA_HTTP_GetPost(pURL, pHeadBuf, headBufSize, NULL, 0, pRecvData, recvBufSize, pRecvSize, pUserID, pPassword, TASK_UNK_16);
}

void MA_HTTP_Post(const char *pURL, char *pHeadBuf, u16 headBufSize, const u8 *pSendData, u16 sendSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword)
{
    MA_HTTP_GetPost(pURL, pHeadBuf, headBufSize, pSendData, sendSize, pRecvData, recvBufSize, pRecvSize, pUserID, pPassword, TASK_UNK_17);
}

#define param PARAM(PARAM_HTTP_GETPOST)
void MA_HTTP_GetPost(const char *pURL, char *pHeadBuf, u16 headBufSize, const u8 *pSendData, u16 sendSize, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword, u8 task)
{
    int len;
    const char *pServerPath;
    u8 server_unk_1;
    u8 server_unk_2;

    SetApiCallFlag();
    if (!MA_ApiPreExe(task)) {
        ResetApiCallFlag();
        return;
    }

    param.task = task;
    param.pRecvData = pRecvData;
    param.recvBufSize = recvBufSize;
    param.pRecvSize = pRecvSize;
    param.pSendData = pSendData;
    param.sendSize = sendSize;
    param.pUserID = pUserID;
    param.pPassword = pPassword;
    param.headBufSize = headBufSize;
    param.pHeadBuf = pHeadBuf;

    if (!(gMA.condition & MA_CONDITION_BUFFER_FULL)) {
        if (recvBufSize == 0) {
            param.pRecvData = NULL;
            if (pRecvSize != NULL) *pRecvSize = 0;
        } else {
            if (pRecvData == NULL) {
                MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
                ResetApiCallFlag();
                return;
            }
            *pRecvSize = 0;
        }

        if (param.headBufSize != 0 && param.pHeadBuf != NULL) {
            param.pHeadBuf[0] = '\0';
        }

        len = MAU_strlen(pURL);
        if (len > 1024 || len == 0) {
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }

        server_unk_2 = 0;
        pServerPath = ExtractServerName(gMA.unk_880, pURL, &server_unk_1, &server_unk_2);
        if (gMA.unk_880[0] == '\0') {
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }

        param.server_unk_1 = server_unk_1;
        if (server_unk_2 == 0) {
            param.pUserID = strEmpty;
            param.pPassword = strEmpty;
        } else {
            if ((len = MAU_strlen(pUserID)) > 16 ||
                    (len = MAU_strlen(pPassword)) > 16) {
                MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
                ResetApiCallFlag();
                return;
            }
        }

        if ((server_unk_2 == 1 && param.task == TASK_UNK_17) ||
                (server_unk_2 == 2 && param.task == TASK_UNK_16) ||
                (server_unk_2 == 3 && param.task == TASK_UNK_17) ||
                (server_unk_2 == 4 && param.task == TASK_UNK_16)) {
            MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
            ResetApiCallFlag();
            return;
        }

        if (!gMA.unk_876) {
            recvBufSize = 1;
        } else {
            recvBufSize = 0;
        }

        if (recvBufSize == 1) {
            if (pServerPath) {
                param.pServerPath = pServerPath;
            } else {
                param.pServerPath = strServerRoot;
            }
        } else {
            param.pServerPath = pURL;
        }

        param.pServerPathLen = MAU_strlen(param.pServerPath);
        param.unk_10 = param.pServerPath;
        param.unk_11 = param.pServerPathLen;
        param.unk_2 = 7;

        InitPrevBuf();
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;
        gMA.unk_1796 = 0;
        gMA.unk_1794 = 0;
        gMA.status &= ~STATUS_UNK_16;
        gMA.unk_1798[0] = '\0';
        param.unk_14 = 0;
        gMA.unk_188 = 0;
        gMA.unk_92 = 6;

        if (!recvBufSize) {
            MAU_memcpy(gMA.ipaddr, &gMA.unk_876, 4);
            MA_TaskSet(task, 2);
        } else {
            MA_TaskSet(task, 0);
        }
    } else {
        gMA.condition &= ~MA_CONDITION_BUFFER_FULL;
        gMA.status &= ~STATUS_UNK_15;

        if (recvBufSize == 0) {
            param.pRecvData = 0;
            if (pRecvSize != NULL) *pRecvSize = 0;
            gMA.status |= STATUS_UNK_15;
            MA_TaskSet(task, 110);
        } else {
            if (pRecvData == NULL) {
                MA_SetApiError(MAAPIE_ILLEGAL_PARAMETER, 0);
                ResetApiCallFlag();
                return;
            }

            *pRecvSize = 0;
            if (gMA.prevbuf_size != 0 && gMA.prevbuf_size <= recvBufSize) {
                MAU_memcpy(pRecvData, gMA.prevbuf, gMA.prevbuf_size);
                pRecvData += gMA.prevbuf_size;
                recvBufSize -= gMA.prevbuf_size;
                *pRecvSize += gMA.prevbuf_size;
                gMA.prevbuf_size = 0;

                param.pRecvData = pRecvData;
                param.recvBufSize = recvBufSize;
                param.pRecvSize = pRecvSize;

                gMA.status |= STATUS_UNK_15;
                MA_TaskSet(task, 110);
            } else {
                MAU_memcpy(pRecvData, gMA.prevbuf, recvBufSize);
                gMA.prevbuf_size -= recvBufSize;
                MAU_memcpy(gMA.prevbuf, &gMA.prevbuf[recvBufSize], gMA.prevbuf_size);
                *pRecvSize = recvBufSize;
                gMA.condition |= MA_CONDITION_BUFFER_FULL;
                ResetApiCallFlag();
                return;
            }
        }
    }

    ResetApiCallFlag();
}
#undef param

#define ConcatUserAgent_WriteAscii(dest, code) \
{ \
    if (code < 0x20 || code >= 0x7f) { \
        *dest = '0'; \
    } else { \
        *dest = code; \
    } \
}

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
    switch (gMA.unk_112) {  // MAGIC
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
    switch (gMA.unk_112) {
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
        switch (gMA.unk_112) {
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
        switch (gMA.unk_112) {
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
        switch (gMA.unk_112) {
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
void MATASK_HTTP_GetPost(void);
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
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_CHANGECLOCK:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        case MACMD_EEPROM_READ:
            gMA.task_error = MAAPIE_EEPROM_SUM;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
        gMA.task_step++;
        break;

    case 3:
        if (!EEPROMRegistrationCheck(&gMA.unk_212[1])) {
            gMA.task_error = MAAPIE_REGISTRATION;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;
        }
        MAU_memcpy(gMA.prevbuf, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_step++;
        break;

    case 4:
        MAU_memcpy(&gMA.eeprom_unk_1275[2], &gMA.buffer_unk_480.data[1], 0x40);
        if (!EEPROMSumCheck(gMA.prevbuf)) {
            gMA.task_error = MAAPIE_EEPROM_SUM;
            gMA.task_error_unk_2 = 0;
            gMA.task_step = 0xfa;
            break;
        }
        gMA.unk_101 = 1;
        MABIOS_End();
        gMA.task_step++;
        break;

    case 5:
        CopyEEPROMData(gMA.unk_112, (char *)gMA.unk_116);
        gMA.task_step++;
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
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.error_unk_94);
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

    gMA.unk_112 = (u32)unk_1;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1C, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_EEPROM_Read(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_CHANGECLOCK:
        case MACMD_EEPROM_READ:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0, 0x80);
        gMA.task_step++;
        break;

    case 3:
        MAU_memcpy((u8 *)gMA.unk_112, &gMA.buffer_unk_480.data[1], 0x80);
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Read(&gMA.buffer_unk_480, 0x80, 0x40);
        gMA.task_step++;
        break;

    case 4:
        MAU_memcpy(&((u8 *)gMA.unk_112)[0x80], &gMA.buffer_unk_480.data[1], 0x40);
        MABIOS_End();
        gMA.task_step++;
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
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = (u32)unk_1;
    *(vu32 *)REG_TM3CNT = 0;
    MA_TaskSet(TASK_UNK_1D, 0);

    ResetApiCallFlag();

    if (!(*(vu32 *)REG_TM3CNT & TMR_IF_ENABLE) || !(*(vu32 *)REG_TM3CNT & TMR_ENABLE)) {
        MAAPI_Main();
    }
}

static void MATASK_EEPROM_Write(void)
{
    if (gMA.cmd_recv == (MACMD_ERROR | MAPROT_REPLY)) {
        switch (gMA.cmd_last) {
        case MACMD_END:
            break;

        case MACMD_CHANGECLOCK:
        case MACMD_EEPROM_WRITE:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfa;
            break;

        default:
            MA_DefaultNegaResProc();
            gMA.task_step = 0xfb;
            break;
        }
    }

    switch (gMA.task_step) {
    case 0:
        MABIOS_Start();
        gMA.task_step++;
        break;

    case 1:
        gMA.task_step++;
        MABIOS_ChangeClock(MA_SIO_WORD);
        break;

    case 2:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Write(&gMA.buffer_unk_480, 0, (u8 *)gMA.unk_112, 0x80);
        gMA.task_step++;
        break;

    case 3:
        (&gMA.buffer_unk_480)->size = 0;
        (&gMA.buffer_unk_480)->data = gMA.unk_212;
        MABIOS_EEPROM_Write(&gMA.buffer_unk_480, 0x80, &((u8 *)gMA.unk_112)[0x80], 0x40);
        gMA.task_step++;
        break;

    case 4:
        MABIOS_End();
        gMA.task_step++;
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
        gMA.task_step++;
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

        MA_SetApiError(gMA.task_error, gMA.task_error_unk_2);
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

    gMA.unk_112 = TASK_UNK_18;
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
    gMA.unk_112 = TASK_UNK_19;
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

    gMA.unk_112 = TASK_UNK_1A;
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

static void (*const taskProcTable[])(void) = {
    NULL,
    MATASK_InitLibrary,
    MATASK_InitLibrary,
    MATASK_TelServer,
    MATASK_Tel,
    MATASK_Receive,
    NULL,
    NULL,
    MATASK_Condition,
    MATASK_Condition,
    MATASK_Offline,
    MATASK_SMTP_Connect,
    MATASK_SMTP_Sender,
    MATASK_SMTP_Send,
    MATASK_SMTP_POP3_Quit,
    MATASK_POP3_Connect,
    MATASK_POP3_Stat,
    MATASK_POP3_List,
    MATASK_POP3_Retr,
    MATASK_POP3_Dele,
    MATASK_POP3_Head,
    MATASK_SMTP_POP3_Quit,
    MATASK_HTTP_GetPost,
    MATASK_HTTP_GetPost,
    NULL,
    NULL,
    NULL,
    MATASK_GetEEPROMData,
    MATASK_EEPROM_Read,
    MATASK_EEPROM_Write,
    MATASK_Stop,
    MATASK_TCP_Cut,
    MATASK_TCP_Connect,
    MATASK_TCP_Disconnect,
    MATASK_TCP_SendRecv,
    MATASK_GetHostAddress,
    NULL
};

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
        gMA.error_unk_94 = 0;  // MAGIC
        break;

    case MAAPIE_UNK_84:
        gMA.error = MAAPIE_SYSTEM;
        gMA.error_unk_94 = 1;  // MAGIC
        break;

    case MAAPIE_UNK_87:
        gMA.error = MAAPIE_SYSTEM;
        gMA.error_unk_94 = 2;  // MAGIC
        break;

    case MAAPIE_UNK_85:
        gMA.error = MAAPIE_SYSTEM;
        gMA.error_unk_94 = 3;  // MAGIC
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
            *pProtocolError = ErrDetailHexConv(gMA.error_unk_94);
            break;

        default:
            *pProtocolError = 0;
            break;
        }
    }

    if (!(gMA.condition & MA_CONDITION_UNK_5)) {
        gMA.cmd_cur = 0;
        gMA.cmd_recv = 0;
    }
    return gMA.error;
}
