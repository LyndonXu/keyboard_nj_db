/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����protocol.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define PROTOCOL_YNA_ENCODE_LENGTH			10	/* Э�鳤�� */
#define PROTOCOL_YNA_DECODE_LENGTH			8	/* Э�鳤�� */


typedef enum _tagEmMainState
{
	_Main_State_Sync,
	_Main_State_Normal,
	
}EmMainState;


enum
{
	_YNA_Sync,
	_YNA_Addr,
	_YNA_Mix,
	_YNA_Cmd,
	_YNA_Data1,
	_YNA_Data2,
	_YNA_Data3,
	_YNA_CheckSum,
};
#define CAM_ADDR_MAX			4


enum
{
	_Blink_Set = 0,
	_Blink_Record,
	_Blink_Switch_Video,
};

#define TOTAL_BLINK_CNT		3



typedef bool (*PFun_KeyProcess)(StKeyMixIn *pKeyIn);

struct _tagStUnionNumKey;

typedef bool (*PFun_TimeReach)(struct _tagStUnionNumKey *pKey);

typedef struct _tagStUnionNumKeyBase
{
	u32 u32StartTime;
	u32 u32EndTime;
	u32 u32BlinkTime;	/* can not be 0 */
	u8 u8RedLedX;
	u8 u8RedLedY;
	u8 u8GreenLedX;
	u8 u8GreenLedY;
	u8 u8KeyValue;
	bool boIsValid;
	bool boIsLight;
	bool boIsSend;
}StUnionNumKeyBase;

typedef struct _tagStUnionNumKey
{
	StUnionNumKeyBase stTenNum;
	StUnionNumKeyBase stUnitNum;
	PFun_TimeReach pFunTimeReach;
}StUnionNumKey;

typedef struct _tagStLedBlinkCtrl
{
	bool boCanBlink;
	bool boIsLight;
	u8 u8X;
	u8 u8Y;
	u16 u16BlinkTime; 
	u32 u32StartTime;
}StLedBlinkCtrl;


void GlobalStateInit(void);

void ChangeEncodeState(void);
void YNADecode(u8 *pBuf);
void YNAEncodeAndGetCheckSum(u8 *pBuf);

void YNAGetCheckSum(u8 *pBuf);
bool KeyProcess(StKeyMixIn *pKeyIn);
bool PCEchoProcess(u8 *pMsg);
void TurnOnSwitchLed(void);

#endif


