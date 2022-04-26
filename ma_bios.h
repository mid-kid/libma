#ifndef _MA_BIOS
#define _MA_BIOS

#include <AgbTypes.h>

void MABIOS_Init(void);
int MA_GetStatus(void);
u16 MA_GetCondition(void);
u8 MA_ErrorCheck(void);
void MA_SetError(u8 error);
void MA_ChangeSIOMode(u8 mode);
void MA_SetDataInterval(s16 interval_byte, s16 interval_word);
int MA_GetCallTypeFromHarwareType(u8 hardware);
void MABIOS_Null(void);
void MABIOS_Start(void);
void MABIOS_Start2(void);
void MABIOS_End(void);
void MABIOS_Tel(u8 calltype, char *number);
void MABIOS_Offline(void);
void MABIOS_WaitCall(void);
void MABIOS_Data(MA_BUF *data_recv, u8 *data_send, u8 size, u8 socket);
void MABIOS_ReInit(void);
void MABIOS_CheckStatus(MA_BUF *data_recv);
void MABIOS_CheckStatus2(MA_BUF *data_recv);
void MABIOS_ChangeClock(u8 mode);
void MABIOS_EEPROM_Read(MA_BUF *data_recv, u8 offset, u8 size);
void MABIOS_EEPROM_Write(MA_BUF *data_recv, u8 offset, u8 *data_send, u8 size);
void MABIOS_PPPConnect(MA_BUF *data_recv, char *userid, char *password, u8 *dns1, u8 *dns2);
void MABIOS_PPPDisconnect(void);
void MABIOS_TCPConnect(MA_BUF *data_recv, u8 *ip, u16 port);
void MABIOS_TCPDisconnect(MA_BUF *data_recv, u8 socket);
void MABIOS_UDPConnect(MA_BUF *data_recv, u8 *ip, u16 port);
void MABIOS_UDPDisconnect(MA_BUF *data_recv, u8 socket);
void MABIOS_DNSRequest(MA_BUF *data_recv, char *addr);
void MABIOS_TestMode(void);
void MA_CancelRequest(void);
void MA_BiosStop(void);
void MA_SendRetry(void);
void MA_RecvRetry(void);
int MA_ProcessCheckStatusResponse(u8 response);
void MA_DefaultNegaResProc(void);
void MA_IntrTimer(void);
void MA_Bios_disconnect(void);
void MA_IntrSerialIO(void);

#endif
