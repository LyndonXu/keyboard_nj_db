/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：protocol.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define PROTOCOL_YNA_ENCODE_LENGTH			10	/* 协议长度 */
#define PROTOCOL_YNA_DECODE_LENGTH			8	/* 协议长度 */


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

enum
{
	_PELCOD_Sync,
	_PELCOD_Addr,
	_PELCOD_Cmd1,
	_PELCOD_Cmd2,
	_PELCOD_Data1,
	_PELCOD_Data2,
	_PELCOD_CheckSum,
};
#define PELCOD_ZOOM_WIDE	0x40
#define PELCOD_ZOOM_TELE	0x20
#define PELCOD_DOWN			0x10
#define PELCOD_UP			0x08
#define PELCOD_LEFT			0x04
#define PELCOD_RIGHT		0x02

enum
{
	_Blink_Set = 0,
	_Blink_Record,
	_Blink_Switch_Video,
};

#define TOTAL_BLINK_CNT		3

typedef enum _tagEmProtocol
{
	_Protocol_Visca,
	_Protocol_PecloD,
}EmProtocol;

extern EmProtocol g_emProtocol;


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
bool ProtocolSelete(u8 u8Key);

#endif


