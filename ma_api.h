#ifndef _MA_API
#define _MA_API

#include <AgbTypes.h>

//void SetApiCallFlag();
//void ResetApiCallFlag();
void MA_TaskSet(u8 unk_1, u8 unk_2);
//void MA_End();
//void MA_Stop();
//void MA_TCP_Cut();
//void MA_InitLibrary();
//void MA_InitLibrary2();
//void MA_TCP_Connect();
//void MA_TCP_Disconnect();
//void MA_TCP_SendRecv();
//void MA_GetHostAddress();
//void MA_GetLocalAddress();
//void MA_TelServer();
//void MA_Tel();
//void MA_Receive();
//void MA_SData();
//void MA_GData();
//void MA_Condition();
//void MA_Condition2();
//void MA_Offline();
//void MA_SMTP_Connect();
//void MA_SMTP_Sender();
//void MA_SMTP_Send();
//void MA_POP3_Quit();
//void MA_SMTP_Quit();
//void MA_POP3_Connect();
//void MA_POP3_Stat();
//void MA_POP3_List();
//void MA_POP3_Retr();
//void MA_POP3_Dele();
//void MA_POP3_Head();
//void MA_HTTP_Get();
//void MA_HTTP_Post();
//void MA_EEPROMRead();
//void MA_EEPROMWrite();
//void MA_GetTel();
//void MA_GetUserID();
//void MA_GetMailID();
void MA_GetSMTPServerName(char *dest);
void MA_GetPOP3ServerName(char *dest);
void MAAPI_Main(void);
//void MAAPI_GetConditionFlag();
//void MAAPI_ErrorCheck();

#endif
