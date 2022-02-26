//===================================================================
//	libma.h
//	モバイルアダプタ通信モジュール Ver.2.0.1
//
//	Copyright (C) 2001,NINTENDO Co.,Ltd.
//===================================================================
#ifndef _LIB_MA_H
#define _LIB_MA_H

#include <Agb.h>

#ifdef __cplusplus
extern "C" {
#endif

//===================================================================
//	定数
//===================================================================

//-----------------------------
//	コンディションフラグ
//-----------------------------
#define MA_CONDITION_APIWAIT			((u16)0x0001)	// API処理中フラグ
#define MA_CONDITION_ERROR				((u16)0x0002)	// エラー発生フラグ
#define MA_CONDITION_BUFFER_FULL		((u16)0x0004)	// 受信バッファフルフラグ
#define MA_CONDITION_PTP_GET			((u16)0x0008)	// 端末間通信受信フラグ
#define MA_CONDITION_CONNECT			((u16)0x0010)	// 電話接続中フラグ

#define MA_CONDITION_SHIFT				(8)
#define MA_CONDITION_IDLE				(0x00)			// アイドル
#define MA_CONDITION_PPP				(0x01)			// PPP接続中
#define MA_CONDITION_P2P_SEND			(0x02)			// PtoP接続中（発呼側）
#define MA_CONDITION_P2P_RECV			(0x03)			// PtoP接続中（発呼側）
#define MA_CONDITION_SMTP				(0x04)			// SMTPセッション中
#define MA_CONDITION_POP3				(0x05)			// POP3セッション中
#define MA_CONDITION_RESERVED			(0x06)			// （予約）
#define MA_CONDITION_LOST				(0x07)			// 端末が繋がっていない又は端末電源OFF
#define MA_CONDITION_MASK				((u16)0xff00)

//-----------------------------
//	機器番号
//-----------------------------
#define MATYPE_PDC						(0x00)			// PDCアダプタ
#define MATYPE_CDMA						(0x01)			// CDMAアダプタ
#define MATYPE_PHS_DoCoMo				(0x02)			// PHS(DoCoMo､アステル)アダプタ
#define MATYPE_PHS_Pocket				(0x03)			// PHS(DDI-Pocket)アダプタ

//-----------------------------
//	エラーコード
//-----------------------------
#define MAAPIE_MA_NOT_FOUND				(0x10)			// MAが接続されていない
#define MAAPIE_CALL_FAILED				(0x11)			// 発呼出来ない（電話機が繋がっていない）
#define MAAPIE_BUSY						(0x12)			// 回線話中
#define MAAPIE_CONNECT					(0x13)			// 回線接続失敗
#define MAAPIE_EEPROM_SUM				(0x14)			// EEPROMﾁｪｯｸｻﾑｴﾗｰ
#define MAAPIE_SYSTEM					(0x15)			// システムエラー
#define MAAPIE_CANNOT_EXECUTE_LOW		(0x16)			// 処理不可能状態（低レベル）
#define MAAPIE_ILLEGAL_PARAMETER_LOW	(0x17)			// パラメータ異常（低レベル）

#define MAAPIE_ILLEGAL_PARAMETER		(0x20)			// APIコール時のパラメータ異常
#define MAAPIE_CANNOT_EXECUTE			(0x21)			// 処理不可能状態
#define MAAPIE_PPP_CONNECT				(0x22)			// PPP接続に失敗した
#define MAAPIE_OFFLINE					(0x23)			// 既に通信が切断されている
#define MAAPIE_TCP_CONNECT				(0x24)			// TCP接続に失敗した
#define MAAPIE_REGISTRATION				(0x25)			// EEPROM登録エラー
#define MAAPIE_TIMEOUT					(0x26)			// アダプタ応答タイムアウトエラー

#define MAAPIE_SMTP						(0x30)			// SMTPエラー
#define MAAPIE_POP3						(0x31)			// POP3エラー
#define MAAPIE_HTTP						(0x32)			// HTTPエラー
#define MAAPIE_GB_CENTER				(0x33)			// GBセンターエラー

// MAAPIE_POP3の詳細エラーコード
#define MAPOP3E_USER_NOT_FOUND			(0x02)			// (USERに対する応答)ユーザID無効
#define MAPOP3E_LOGIN_FAILED			(0x03)			// (PASSに対する応答)ﾊﾟｽﾜｰﾄﾞ無効、ﾒｰﾙﾎﾞｯｸｽがﾛｯｸされている
#define MAPOP3E_NO_SUCH_MESSAGE			(0x04)			// (LIST,RETR,DELE,TOPに対する応答)指定メール番号無し
#define MAPOP3E_UNKNOWN					(0x00)			// 不明


//===================================================================
//	型定義
//===================================================================
// 受信データ
typedef struct {

	char	telNo[17];		// 電話番号
	char	comment[17];	// コメント

} MA_TELDATA;

//===================================================================
//	外部関数宣言
//===================================================================
// 割り込みハンドラ
void MA_IntrSerialIO(void);
void MA_IntrTimer(void);

// API関数群
void MA_InitLibrary(
	u8 *pHardwareType
);

void MA_InitLibrary2(
	u8 *pHardwareType
);

void MA_End(void);

void MA_TelServer(
	const char *pTelNo,
	const char *pUserID,
	const char *pPassword
);

void MA_Tel(
	const char *pTelNo
);

void MA_Receive(void);

void MA_SData(
	const u8 *pSendData,
	u8 sendSize,
	u8 *pResult
);

void MA_GData(
	u8 *pRecvData,
	u8 *pRecvSize
);

void MA_Condition
(
	u8 *pCondition
);

void MA_Condition2
(
	u8 *pCondition
);

void MA_Offline(void);

void MA_SMTP_Connect(
	const char *pMailAddress
);

void MA_SMTP_Sender(
	const char * const pRecipients[]	//const char *pRecipients[]を変更	2001.03.12
);

void MA_SMTP_Send(
	const char *pSendData,				//const u8 *pSendData,を変更	2001.03.12
	u16 sendSize,
	int endFlag
);

void MA_SMTP_Quit(void);

void MA_POP3_Connect(
	const char *pUserID,
	const char *pPassword
);

void MA_POP3_Stat(
	u16 *pNum,
	u32 *pSize
);

void MA_POP3_List(
	u16	mailNo,
	u32 *pSize
);

void MA_POP3_Retr(
	u16 mailNo,
	u8	*pRecvData,
	u16 recvBufSize,
	u16 *pRecvSize
);

void MA_POP3_Dele(
	u16 mailNo
);

void MA_POP3_Head(
	u16	mailNo,
	u8	*pRecvData,
	u16	recvBufSize,
	u16	*pRecvSize
);

void MA_POP3_Quit(void);

void MA_HTTP_Get(
	const char	*pURL,
	char		*pHeadBuf,
	u16			headBufSize,
	u8			*pRecvData,
	u16			recvBufSize,
	u16			*pRecvSize,
	const char	*pUserID,
	const char	*pPassword
);

void MA_HTTP_Post(
	const char	*pURL,
	char		*pHeadBuf,
	u16			headBufSize,
	const u8	*pSendData,
	u16			sendSize,
	u8			*pRecvData,
	u16			recvBufSize,
	u16			*pRecvSize,
	const char	*pUserID,
	const char	*pPassword
);

void MA_GetTel(
	MA_TELDATA *pTelData
);

void MA_GetUserID(
	char *pUserIDBuf
);

void MA_GetMailID(
	char *pBufPtr
);

void MA_Stop(void);

void MA_TCP_Cut(void);

// API状態確認
u16 MAAPI_GetConditionFlag(void);

u8 MAAPI_ErrorCheck(
	u16 *pProtocolError
);

#ifdef __cplusplus
}		// extern "C"
#endif
#endif	// _LIB_MA_H
