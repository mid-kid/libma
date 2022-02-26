//===================================================================
//	libma.h
//	���o�C���A�_�v�^�ʐM���W���[�� Ver.2.0.1
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
//	�萔
//===================================================================

//-----------------------------
//	�R���f�B�V�����t���O
//-----------------------------
#define MA_CONDITION_APIWAIT			((u16)0x0001)	// API�������t���O
#define MA_CONDITION_ERROR				((u16)0x0002)	// �G���[�����t���O
#define MA_CONDITION_BUFFER_FULL		((u16)0x0004)	// ��M�o�b�t�@�t���t���O
#define MA_CONDITION_PTP_GET			((u16)0x0008)	// �[���ԒʐM��M�t���O
#define MA_CONDITION_CONNECT			((u16)0x0010)	// �d�b�ڑ����t���O

#define MA_CONDITION_SHIFT				(8)
#define MA_CONDITION_IDLE				(0x00)			// �A�C�h��
#define MA_CONDITION_PPP				(0x01)			// PPP�ڑ���
#define MA_CONDITION_P2P_SEND			(0x02)			// PtoP�ڑ����i���đ��j
#define MA_CONDITION_P2P_RECV			(0x03)			// PtoP�ڑ����i���đ��j
#define MA_CONDITION_SMTP				(0x04)			// SMTP�Z�b�V������
#define MA_CONDITION_POP3				(0x05)			// POP3�Z�b�V������
#define MA_CONDITION_RESERVED			(0x06)			// �i�\��j
#define MA_CONDITION_LOST				(0x07)			// �[�����q�����Ă��Ȃ����͒[���d��OFF
#define MA_CONDITION_MASK				((u16)0xff00)

//-----------------------------
//	�@��ԍ�
//-----------------------------
#define MATYPE_PDC						(0x00)			// PDC�A�_�v�^
#define MATYPE_CDMA						(0x01)			// CDMA�A�_�v�^
#define MATYPE_PHS_DoCoMo				(0x02)			// PHS(DoCoMo��A�X�e��)�A�_�v�^
#define MATYPE_PHS_Pocket				(0x03)			// PHS(DDI-Pocket)�A�_�v�^

//-----------------------------
//	�G���[�R�[�h
//-----------------------------
#define MAAPIE_MA_NOT_FOUND				(0x10)			// MA���ڑ�����Ă��Ȃ�
#define MAAPIE_CALL_FAILED				(0x11)			// ���ďo���Ȃ��i�d�b�@���q�����Ă��Ȃ��j
#define MAAPIE_BUSY						(0x12)			// ����b��
#define MAAPIE_CONNECT					(0x13)			// ����ڑ����s
#define MAAPIE_EEPROM_SUM				(0x14)			// EEPROM�����Ѵװ
#define MAAPIE_SYSTEM					(0x15)			// �V�X�e���G���[
#define MAAPIE_CANNOT_EXECUTE_LOW		(0x16)			// �����s�\��ԁi�჌�x���j
#define MAAPIE_ILLEGAL_PARAMETER_LOW	(0x17)			// �p�����[�^�ُ�i�჌�x���j

#define MAAPIE_ILLEGAL_PARAMETER		(0x20)			// API�R�[�����̃p�����[�^�ُ�
#define MAAPIE_CANNOT_EXECUTE			(0x21)			// �����s�\���
#define MAAPIE_PPP_CONNECT				(0x22)			// PPP�ڑ��Ɏ��s����
#define MAAPIE_OFFLINE					(0x23)			// ���ɒʐM���ؒf����Ă���
#define MAAPIE_TCP_CONNECT				(0x24)			// TCP�ڑ��Ɏ��s����
#define MAAPIE_REGISTRATION				(0x25)			// EEPROM�o�^�G���[
#define MAAPIE_TIMEOUT					(0x26)			// �A�_�v�^�����^�C���A�E�g�G���[

#define MAAPIE_SMTP						(0x30)			// SMTP�G���[
#define MAAPIE_POP3						(0x31)			// POP3�G���[
#define MAAPIE_HTTP						(0x32)			// HTTP�G���[
#define MAAPIE_GB_CENTER				(0x33)			// GB�Z���^�[�G���[

// MAAPIE_POP3�̏ڍ׃G���[�R�[�h
#define MAPOP3E_USER_NOT_FOUND			(0x02)			// (USER�ɑ΂��鉞��)���[�UID����
#define MAPOP3E_LOGIN_FAILED			(0x03)			// (PASS�ɑ΂��鉞��)�߽ܰ�ޖ����AҰ��ޯ����ۯ�����Ă���
#define MAPOP3E_NO_SUCH_MESSAGE			(0x04)			// (LIST,RETR,DELE,TOP�ɑ΂��鉞��)�w�胁�[���ԍ�����
#define MAPOP3E_UNKNOWN					(0x00)			// �s��


//===================================================================
//	�^��`
//===================================================================
// ��M�f�[�^
typedef struct {

	char	telNo[17];		// �d�b�ԍ�
	char	comment[17];	// �R�����g

} MA_TELDATA;

//===================================================================
//	�O���֐��錾
//===================================================================
// ���荞�݃n���h��
void MA_IntrSerialIO(void);
void MA_IntrTimer(void);

// API�֐��Q
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
	const char * const pRecipients[]	//const char *pRecipients[]��ύX	2001.03.12
);

void MA_SMTP_Send(
	const char *pSendData,				//const u8 *pSendData,��ύX	2001.03.12
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

// API��Ԋm�F
u16 MAAPI_GetConditionFlag(void);

u8 MAAPI_ErrorCheck(
	u16 *pProtocolError
);

#ifdef __cplusplus
}		// extern "C"
#endif
#endif	// _LIB_MA_H
