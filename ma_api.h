#ifndef _MA_API
#define _MA_API

#include "libma.h"

void SetApiCallFlag(void);
void ResetApiCallFlag(void);
void MA_TaskSet(u8 task, u8 step);
void MA_End(void);
void MA_Stop(void);
void MA_TCP_Cut(void);
void MA_InitLibrary(u8 *pHardwareType);
void MA_InitLibrary2(u8 *pHardwareType);
void MA_TCP_Connect(u8 *unk_1, u8 *unk_2, u16 unk_3);
void MA_TCP_Disconnect(u8 unk_1);
void MA_TCP_SendRecv(u8 unk_1, int unk_2, u8 unk_3, int unk_4);
void MA_GetHostAddress(u8 *unk_1, char *unk_2);
void MA_GetLocalAddress(u8 *address);
void MA_TelServer(const char *pTelNo, const char *pUserID, const char *pPassword);
void MA_Tel(const char *pTelNo);
void MA_Receive(void);
void MA_SData(const u8 *pSendData, u8 sendSize, u8 *pResult);
void MA_GData(u8 *pRecvData, u8 *pRecvSize);
void MA_Condition(u8 *pCondition);
void MA_Condition2(u8 *pCondition);
void MA_Offline(void);
void MA_SMTP_Connect(const char *pMailAddress);
void MA_SMTP_Sender(const char * const pRecipients[]);
void MA_SMTP_Send(const char *pSendData, u16 sendSize, int endFlag);
void MA_POP3_Quit(void);
void MA_SMTP_Quit(void);
void MA_POP3_Connect(const char *pUserID, const char *pPassword);
void MA_POP3_Stat(u16 *pNum, u32 *pSize);
void MA_POP3_List(u16 mailNo, u32 *pSize);
void MA_POP3_Retr(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize);
void MA_POP3_Dele(u16 mailNo);
void MA_POP3_Head(u16 mailNo, u8 *pRecvData, u16 recvBufSize, u16 *pRecvSize);
void MA_HTTP_Get(const char *pURL, char *pHeadBuf, u16 headBufSize, u8 *pRecvData,
    u16 recvBufSize, u16 *pRecvSize, const char *pUserID, const char *pPassword);
void MA_HTTP_Post(const char *pURL, char *pHeadBuf, u16 headBufSize,
    const u8 *pSendData, u16 sendSize, u8 *pRecvData, u16 recvBufSize,
    u16 *pRecvSize, const char *pUserID, const char *pPassword);
void MA_EEPROMRead(u8 *unk_1);
void MA_EEPROMWrite(u8 *unk_1);
void MA_GetTel(MA_TELDATA *pTelData);
void MA_GetUserID(char *pUserIDBuf);
void MA_GetMailID(char *pBufPtr);
void MA_GetSMTPServerName(char *dest);
void MA_GetPOP3ServerName(char *dest);
void MAAPI_Main(void);
u16 MAAPI_GetConditionFlag(void);
u8 MAAPI_ErrorCheck(u16 *pProtocolError);

#endif // _MA_API
