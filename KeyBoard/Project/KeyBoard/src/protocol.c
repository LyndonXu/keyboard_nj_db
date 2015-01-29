/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：protocol.c
* 摘要: 协议控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
	
	
#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"
	
#include "buzzer.h"
	
#include "user_init.h"
#include "user_api.h"

#include "key_led_table.h"

#include "protocol.h"

#include "message.h"
#include "flash_save.h"
#include "extern_io_ctrl.h"

u8 g_u8CamAddr = 0;
bool g_boIsBroadcastPlay = false;
bool g_boIsDDRPlay = false;
bool g_boIsEdit = false;
bool g_boIsRockAus = false;


EmProtocol g_emProtocol = _Protocol_Visca;
const u16 g_u16CamLoc[CAM_ADDR_MAX] = 
{
	_Led_Positoin_1,
	_Led_Positoin_2,
	_Led_Positoin_3,
	_Led_Positoin_4,
};

bool ProtocolSelete(u8 u8Key)
{
	EmProtocol emProtocol;
	u16 u16Led;
	bool boIsRight = false;
	
	if (u8Key == _Key_VI_V1)
	{
		u16Led = _Led_VI_V1;
		emProtocol = _Protocol_Visca;
	}
	else
	{
		u16Led = _Led_VI_V2;	
		emProtocol = _Protocol_PecloD;		
	}
	
	if (g_emProtocol == emProtocol)
	{
		boIsRight = true;
	}
	else
	{
		g_emProtocol = emProtocol;
		boIsRight = WriteSaveData();
	}
	
	ChangeAllLedState(false);

	if (boIsRight)
	{
		u32 u32Time = g_u32SysTickCnt;
		ChangeLedState(GET_X(u16Led), GET_Y(u16Led), false);
		while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
	}
	else
	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			u32 u32Time = g_u32SysTickCnt;
			boBlink = !boBlink;
			ChangeLedState(GET_X(u16Led), GET_Y(u16Led), boBlink);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}
	ChangeAllLedState(true);
	return boIsRight;
}


void TurnOnSwitchRockLed(void)
{
	const u16 u16Led[5][8] =
	{
		{
			_Led_Switch_1_1, _Led_Switch_1_2, _Led_Switch_1_3, _Led_Switch_1_4, 
			_Led_Switch_1_5, _Led_Switch_1_6, _Led_Switch_1_7, _Led_Switch_1_8, 
		},
		{
			_Led_Switch_2_1, _Led_Switch_2_2, _Led_Switch_2_3, _Led_Switch_2_4, 
			_Led_Switch_2_5, _Led_Switch_2_6, _Led_Switch_2_7, _Led_Switch_2_8, 
		},
		{
			_Led_Switch_3_1, _Led_Switch_3_2, _Led_Switch_3_3, _Led_Switch_3_4, 
			_Led_Switch_3_5, _Led_Switch_3_6, _Led_Switch_3_7, _Led_Switch_3_8, 
		},
		{
			_Led_Switch_4_1, _Led_Switch_4_2, _Led_Switch_4_3, _Led_Switch_4_4, 
			_Led_Switch_4_5, _Led_Switch_4_6, _Led_Switch_4_7, _Led_Switch_4_8, 
		},
		{
			_Led_Rock_1, _Led_Rock_2, _Led_Rock_3, _Led_Rock_4, 
			_Led_Rock_5, _Led_Rock_6, _Led_Rock_7, _Led_Rock_8, 
		},
	};
	u32 i, j;
	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 8; j++)
		{
			u16 u16LedLocal = u16Led[i][j];
			ChangeLedState(GET_X(u16LedLocal), GET_Y(u16LedLocal), true);
		}
	}
}


void GlobalStateInit(void)
{
	g_u8CamAddr = 0;

	g_boIsPushRodNeedReset = false;
	g_boIsBroadcastPlay = false;
	g_boIsDDRPlay = false;
	g_boIsEdit = false;	
	TurnOnSwitchRockLed();
}


void ChangeEncodeState(void)
{
	u32 u32Cnt = 0;	
	u32 u32MsgSentTime;
	StKeyMixIn *pKeyIn;
	ChangeAllLedState(true);
	while(u32Cnt < 10)
	{
		
		u32Cnt++;
		u32MsgSentTime = g_u32SysTickCnt;
		while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 150);/* 延时150ms */

		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();

		if (pKeyIn == NULL)
		{
			continue;
		}
		if (pKeyIn->emKeyType == _Key_Board)
		{
			if (pKeyIn->u32Cnt != 2)
			{
				continue;
			}
			if ((pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_PGM_1) &&
				(pKeyIn->unKeyMixIn.stKeyState[1].u8KeyValue == _Key_PVW_V4))
			{
				KeyBufGetEnd();
				break;
			}
			else if((pKeyIn->unKeyMixIn.stKeyState[1].u8KeyValue == _Key_PGM_1) &&
				(pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_PVW_V4))
			{
				KeyBufGetEnd();
				break;
			}
			else
			{
				continue;
			}
		}
		KeyBufGetEnd();
	}
	if (u32Cnt >= 10)
	{
		return;
	}
	
	ChangeAllLedState(false);
	g_u32BoolIsEncode = !g_u32BoolIsEncode; 
	u32MsgSentTime = g_u32SysTickCnt;
	ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), true);
	ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), true);
	while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1000);/* 延时1000ms */
	ChangeAllLedState(true);
}

void YNADecode(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}

}
void YNAEncodeAndGetCheckSum(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}
}


void YNAGetCheckSum(u8 *pBuf)
{
	s32 i, s32End;
	u8 u8Sum = pBuf[0];

	if (g_u32BoolIsEncode)
	{
		s32End = PROTOCOL_YNA_ENCODE_LENGTH - 1;	
	}
	else
	{
		s32End = PROTOCOL_YNA_DECODE_LENGTH - 1;
	}
	for (i = 1; i < s32End; i++)
	{
		u8Sum ^= pBuf[i];
	}
	pBuf[i] = u8Sum;
}

void PelcoDGetCheckSum(u8 *pBuf)
{
	s32 i;
	u8 u8Sum = 0;
	for (i = 1; i < 6; i++)
	{
		u8Sum += pBuf[i];
	}
	pBuf[i] = u8Sum;
}


static StKeyState s_stOldKeyState = {0, 0, KEY_UP, KEY_UP};

static StKeyState * FindMatchKey(StKeyMixIn *pKeyIn)
{
	if (s_stOldKeyState.u8KeyState == KEY_UP)
	{
		if (pKeyIn->unKeyMixIn.stKeyState[0].u8KeyState != KEY_DOWN)
		{
			return NULL;
		}
		s_stOldKeyState = pKeyIn->unKeyMixIn.stKeyState[0];
		s_stOldKeyState.u8Reserved = KEY_DOWN;
		goto ok;
	}
	else
	{
		u32 i;
		for (i = 0; i < pKeyIn->u32Cnt; i++)
		{
			if (s_stOldKeyState.u8KeyLocation == 
				pKeyIn->unKeyMixIn.stKeyState[i].u8KeyLocation)
			{
				s_stOldKeyState.u8KeyState = pKeyIn->unKeyMixIn.stKeyState[i].u8KeyState;
				goto ok;
			}
		}
	}
	return NULL;
ok:
	if (s_stOldKeyState.u8KeyState == KEY_UP)
	{
		s_stOldKeyState.u8Reserved = KEY_UP;
	}
	return &s_stOldKeyState;

}
#if 1

static bool KeyBoardProcess(StKeyMixIn *pKeyIn)
{
	u32 i;
	for (i = 0; i < pKeyIn->u32Cnt; i++)
	{
		u8 *pBuf;
		StKeyState *pKeyState = pKeyIn->unKeyMixIn.stKeyState + i;
		u8 u8KeyValue;
	
	u8KeyValue = pKeyState->u8KeyValue;

	if (pKeyState->u8KeyState == KEY_KEEP)
	{
		continue;
	}


	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;
	if (pKeyState->u8KeyState == KEY_UP)
	{
		pBuf[_YNA_Data1] = 0x01;
	}

	/* 处理按键 */
	switch (u8KeyValue)
	{
		case _Key_Record_Record:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_Record_Live:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x00;
			break;
		}
		case _Key_Record_Snap:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0C;
			break;
		}
		case _Key_Record_ChannelRecord:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_Delay_Broadcast_Live_Stop:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = g_boIsBroadcastPlay;
			break;
		}
		case _Key_Delay_Broadcast_Express:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0D;
			break;
		}
		case _Key_Delay_Broadcast_Safe:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0E;
			break;
		}
		case _Key_Listen:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x03;
			break;		
		}
		case _Key_Look:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x05;
			break;		
		}
		case _Key_Projection:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x08;
			break;		
		}
		case _Key_Multi_Views:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x07;
			break;		
		}

		case _Key_Caption:
		case _Key_Angle1:
		case _Key_Angle2:
		case _Key_Time:
		{
			pBuf[_YNA_Cmd] = 0x4B;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Caption;
			break;
		}

		case _Key_VI_V1:
		case _Key_VI_V2:
		case _Key_VI_V3:
		case _Key_VI_V4:
		case _Key_VI_1:
		case _Key_VI_2:
		case _Key_VI_3:
		case _Key_VI_4:
		case _Key_VI_Auto:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_VI_V1;
			break;
		}

		case _Key_DDR_1:
		case _Key_DDR_2:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x0F + u8KeyValue - _Key_DDR_1;
			break;
		}
		case _Key_DDR_Back:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x07;
			break;
		}
		case _Key_DDR_Stop:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x04;
			break;
		}
		case _Key_DDR_Play:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_DDR_Pause:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x03;
			break;
		}
		case _Key_DDR_Front:
		case _Key_DDR_Cycle:
		case _Key_DDR_Single:
		case _Key_DDR_Auto:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x08 + u8KeyValue - _Key_DDR_Front;
			break;
		}

		case _Key_PGM_1:
		case _Key_PGM_2:
		case _Key_PGM_3:
		case _Key_PGM_4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x00;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_1;
			break;
		}
		case _Key_PGM_VGA:
		case _Key_PGM_NET:
		case _Key_PGM_DDR1:
		case _Key_PGM_DDR2:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_VGA;
			break;
		}
		case _Key_PGM_V1:
		case _Key_PGM_V2:
		case _Key_PGM_V3:
		case _Key_PGM_V4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_V1;
			break;
		}

		case _Key_PVW_1:
		case _Key_PVW_2:
		case _Key_PVW_3:
		case _Key_PVW_4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x30;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_1;
			break;
		}
		case _Key_PVW_VGA:
		case _Key_PVW_NET:
		case _Key_PVW_DDR1:
		case _Key_PVW_DDR2:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x40;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_VGA;
			break;
		}
		case _Key_PVW_V1:
		case _Key_PVW_V2:
		case _Key_PVW_V3:
		case _Key_PVW_V4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x50;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_V1;
			break;
		}

		case _Key_Trick_Fade_IN_OUT:
		case _Key_Trick_Left_Erasure:
		case _Key_Trick_PIP:
		case _Key_Trick_Reel:
		case _Key_Trick_Slide:
		case _Key_Trick_Flexible:
		case _Key_Trick_3D:
		case _Key_Trick_Switch:
		case _Key_Trick_Shortcut_1:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data2] = 0x0E + u8KeyValue - _Key_Trick_Fade_IN_OUT;
			break;			
		}

		case _Key_Positoin_1:
		case _Key_Positoin_2:
		case _Key_Positoin_3:
		case _Key_Positoin_4:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_1;
			if (pKeyState->u8KeyState == KEY_UP)
			{
				g_u8CamAddr = pBuf[_YNA_Data2];

				if (g_emProtocol == _Protocol_Visca)
				{
					u32 i;
					for (i = 0; i < CAM_ADDR_MAX; i++)
					{
						ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
						ChangeLedState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
					}
					ChangeLedState(GET_X(g_u16CamLoc[g_u8CamAddr]), 
							GET_Y(g_u16CamLoc[g_u8CamAddr]), true);
				}
				else
				{
					u32 i;
					for (i = 0; i < CAM_ADDR_MAX; i++)
					{
						ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
						ChangeLedState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
					}
					ChangeLedBlinkState(GET_X(g_u16CamLoc[g_u8CamAddr]), 
							GET_Y(g_u16CamLoc[g_u8CamAddr]), true);
				}
			}
			break;
		}
		case _Key_Positoin_VIN_1:
		case _Key_Positoin_VIN_2:
		case _Key_Positoin_VIN_3:
		case _Key_Positoin_VIN_4:
		{
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_VIN_1 + 0x04;
			pBuf[_YNA_Cmd] = 0x44;
			break;			
		}
		case _Key_Positoin_Caption:
		case _Key_Positoin_Angle1:
		case _Key_Positoin_Angle2:
		case _Key_Positoin_Time:
		{
			pBuf[_YNA_Cmd] = 0x4B;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_Caption;
			break;
		}
		case _Key_Positoin_Aus:
		{
			if (pKeyState->u8KeyState == KEY_DOWN)
			{
				g_boIsRockAus = true;
			}
			else
			{
				g_boIsRockAus = false;
			}
			continue;
		}
		case _Key_Cutover_PVM:
		case _Key_Cutover_PGW:
		case _Key_Cutover_FTB:
		case _Key_Cutover_VI:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_Cutover_PVM;
			break;
		}
		case _Key_Cutover_Cut:
		case _Key_Cutover_Auto:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data2] = 0x01 - (u8KeyValue - _Key_Cutover_Cut);
			break;
		}

		case _Key_Switch_1:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data2] = 0xFF;
			break;
		}
		case _Key_Switch_3:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x06;
			break;
		}
		case _Key_Switch_4:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x06;
			break;
		}
		default:
			continue;

	}
	
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	}

	return true;
}

#else
static bool KeyBoardProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	StKeyState *pKeyState = FindMatchKey(pKeyIn);
	u8 u8KeyValue;

	if (pKeyState == NULL)
	{
		return false;
	}

	u8KeyValue = pKeyState->u8KeyValue;

	if (pKeyState->u8KeyState == KEY_KEEP)
	{
		{
			return false;
		}
	}


	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;
	if (pKeyState->u8KeyState == KEY_UP)
	{
		pBuf[_YNA_Data1] = 0x01;
	}

	/* 处理按键 */
	switch (u8KeyValue)
	{
		case _Key_Record_Record:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_Record_Live:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x00;
			break;
		}
		case _Key_Record_Snap:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0C;
			break;
		}
		case _Key_Record_ChannelRecord:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_Delay_Broadcast_Live_Stop:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = g_boIsBroadcastPlay;
			break;
		}
		case _Key_Delay_Broadcast_Express:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0D;
			break;
		}
		case _Key_Delay_Broadcast_Safe:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data2] = 0x0E;
			break;
		}
		case _Key_Listen:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x03;
			break;		
		}
		case _Key_Look:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x05;
			break;		
		}
		case _Key_Projection:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x08;
			break;		
		}
		case _Key_Multi_Views:
		{
			pBuf[_YNA_Cmd] = 0x4C;
			pBuf[_YNA_Data2] = 0x07;
			break;		
		}

		case _Key_Caption:
		case _Key_Angle1:
		case _Key_Angle2:
		case _Key_Time:
		{
			pBuf[_YNA_Cmd] = 0x4B;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Caption;
			break;
		}

		case _Key_VI_V1:
		case _Key_VI_V2:
		case _Key_VI_V3:
		case _Key_VI_V4:
		case _Key_VI_1:
		case _Key_VI_2:
		case _Key_VI_3:
		case _Key_VI_4:
		case _Key_VI_Auto:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_VI_V1;
			break;
		}

		case _Key_DDR_1:
		case _Key_DDR_2:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x0F + u8KeyValue - _Key_DDR_1;
			break;
		}
		case _Key_DDR_Back:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x07;
			break;
		}
		case _Key_DDR_Stop:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x04;
			break;
		}
		case _Key_DDR_Play:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x02;
			break;
		}
		case _Key_DDR_Pause:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x03;
			break;
		}
		case _Key_DDR_Front:
		case _Key_DDR_Cycle:
		case _Key_DDR_Single:
		case _Key_DDR_Auto:
		{
			pBuf[_YNA_Cmd] = 0x47;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x08 + u8KeyValue - _Key_DDR_Front;
			break;
		}

		case _Key_PGM_1:
		case _Key_PGM_2:
		case _Key_PGM_3:
		case _Key_PGM_4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x00;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_1;
			break;
		}
		case _Key_PGM_VGA:
		case _Key_PGM_NET:
		case _Key_PGM_DDR1:
		case _Key_PGM_DDR2:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_VGA;
			break;
		}
		case _Key_PGM_V1:
		case _Key_PGM_V2:
		case _Key_PGM_V3:
		case _Key_PGM_V4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PGM_V1;
			break;
		}

		case _Key_PVW_1:
		case _Key_PVW_2:
		case _Key_PVW_3:
		case _Key_PVW_4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x30;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_1;
			break;
		}
		case _Key_PVW_VGA:
		case _Key_PVW_NET:
		case _Key_PVW_DDR1:
		case _Key_PVW_DDR2:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x40;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_VGA;
			break;
		}
		case _Key_PVW_V1:
		case _Key_PVW_V2:
		case _Key_PVW_V3:
		case _Key_PVW_V4:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data1] |= 0x50;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_PVW_V1;
			break;
		}

		case _Key_Trick_Fade_IN_OUT:
		case _Key_Trick_Left_Erasure:
		case _Key_Trick_PIP:
		case _Key_Trick_Reel:
		case _Key_Trick_Slide:
		case _Key_Trick_Flexible:
		case _Key_Trick_3D:
		case _Key_Trick_Switch:
		case _Key_Trick_Shortcut_1:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data2] = 0x0E + u8KeyValue - _Key_Trick_Fade_IN_OUT;
			break;			
		}

		case _Key_Positoin_1:
		case _Key_Positoin_2:
		case _Key_Positoin_3:
		case _Key_Positoin_4:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_1;
			if (pKeyState->u8KeyState == KEY_UP)
			{
				g_u8CamAddr = pBuf[_YNA_Data2];

				if (g_emProtocol == _Protocol_Visca)
				{
					u32 i;
					for (i = 0; i < CAM_ADDR_MAX; i++)
					{
						ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
						ChangeLedState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
					}
					ChangeLedState(GET_X(g_u16CamLoc[g_u8CamAddr]), 
							GET_Y(g_u16CamLoc[g_u8CamAddr]), true);
				}
				else
				{
					u32 i;
					for (i = 0; i < CAM_ADDR_MAX; i++)
					{
						ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
						ChangeLedState(GET_X(g_u16CamLoc[i]), 
								GET_Y(g_u16CamLoc[i]), false);
					}
					ChangeLedBlinkState(GET_X(g_u16CamLoc[g_u8CamAddr]), 
							GET_Y(g_u16CamLoc[g_u8CamAddr]), true);
				}
			}
			break;
		}
		case _Key_Positoin_VIN_1:
		case _Key_Positoin_VIN_2:
		case _Key_Positoin_VIN_3:
		case _Key_Positoin_VIN_4:
		{
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_VIN_1 + 0x04;
			pBuf[_YNA_Cmd] = 0x44;
			break;			
		}
		case _Key_Positoin_Caption:
		case _Key_Positoin_Angle1:
		case _Key_Positoin_Angle2:
		case _Key_Positoin_Time:
		{
			pBuf[_YNA_Cmd] = 0x4B;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = u8KeyValue - _Key_Positoin_Caption;
			break;
		}
		case _Key_Positoin_Aus:
		{
			if (pKeyState->u8KeyState == KEY_DOWN)
			{
				g_boIsRockAus = true;
			}
			else
			{
				g_boIsRockAus = false;
			}
			return false;
		}
		case _Key_Cutover_PVM:
		case _Key_Cutover_PGW:
		case _Key_Cutover_FTB:
		case _Key_Cutover_VI:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data2] = 0x02 + u8KeyValue - _Key_Cutover_PVM;
			break;
		}
		case _Key_Cutover_Cut:
		case _Key_Cutover_Auto:
		{
			pBuf[_YNA_Cmd] = 0x48;
			pBuf[_YNA_Data2] = 0x01 - (u8KeyValue - _Key_Cutover_Cut);
			break;
		}

		case _Key_Switch_1:
		{
			pBuf[_YNA_Cmd] = 0x44;
			pBuf[_YNA_Data2] = 0xFF;
			break;
		}
		case _Key_Switch_3:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data1] |= 0x10;
			pBuf[_YNA_Data2] = 0x06;
			break;
		}
		case _Key_Switch_4:
		{
			pBuf[_YNA_Cmd] = 0x4A;
			pBuf[_YNA_Data1] |= 0x20;
			pBuf[_YNA_Data2] = 0x06;
			break;
		}
		default:
			return false;

	}
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	return true;
}
#endif

static bool RockProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	u8 u8Cmd;
#if SCENCE_MUTUAL
	TurnOffZoomScence();
	PresetNumInit();
#endif
	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	u8Cmd = pBuf[_YNA_Cmd] = pKeyIn->unKeyMixIn.stRockState.u8RockDir;
	do
	{
		u16 u16Led[]=
		{
			_Led_Rock_1, _Led_Rock_2, _Led_Rock_3, _Led_Rock_4, 
			_Led_Rock_5, _Led_Rock_6, _Led_Rock_7, _Led_Rock_8, 
		};
		if (u8Cmd == 0)
		{
			u32 i;
			for(i = 0; i < 8; i++)
			{
				u16 u16LedLocal = u16Led[i];
				ChangeLedState(GET_X(u16LedLocal), GET_Y(u16LedLocal), true);					
			}
		}
		else 
		{
			if ((u8Cmd & 0x01) != 0)
			{
				ChangeLedState(GET_X(u16Led[5]), GET_Y(u16Led[5]), false);	/* right */					
				ChangeLedState(GET_X(u16Led[7]), GET_Y(u16Led[7]), true);	/* left */				
			}
			else if ((u8Cmd & 0x02) != 0)
			{
				ChangeLedState(GET_X(u16Led[5]), GET_Y(u16Led[5]), true);	/* right */					
				ChangeLedState(GET_X(u16Led[7]), GET_Y(u16Led[7]), false);	/* left */				
			}

			if ((u8Cmd & 0x04) != 0)
			{
				ChangeLedState(GET_X(u16Led[3]), GET_Y(u16Led[3]), false);	/* up */					
				ChangeLedState(GET_X(u16Led[1]), GET_Y(u16Led[1]), true);	/* down */					
				//ChangeLedState(GET_X(u16Led[2]), GET_Y(u16Led[2]), true);	/* down */					
			}
			else if ((u8Cmd & 0x08) != 0)
			{
				ChangeLedState(GET_X(u16Led[3]), GET_Y(u16Led[3]), true);	/* up */					
				ChangeLedState(GET_X(u16Led[1]), GET_Y(u16Led[1]), false);	/* down */					
				//ChangeLedState(GET_X(u16Led[2]), GET_Y(u16Led[2]), false);	/* down */					
			}
		}

	}while(0);
	
	
	pBuf[_YNA_Data1] = pKeyIn->unKeyMixIn.stRockState.u16RockXValue;
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.stRockState.u16RockYValue;

	pBuf[_YNA_Data3] = pKeyIn->unKeyMixIn.stRockState.u16RockZValue;
	if (g_boIsRockAus)
	{
		pBuf[_YNA_Data1] |= (0x01 << 6);
	}
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	if (g_emProtocol == _Protocol_PecloD)
	{
		u8 u8Buf[7];
		u8Buf[_PELCOD_Sync] = 0xFF;
		u8Buf[_PELCOD_Addr] = g_u8CamAddr + 1;
		u8Buf[_PELCOD_Cmd1] = 0;
		u8Buf[_PELCOD_Cmd2] = pKeyIn->unKeyMixIn.stRockState.u8RockDir << 1;
		u8Buf[_PELCOD_Data1] = pKeyIn->unKeyMixIn.stRockState.u16RockXValue;
		u8Buf[_PELCOD_Data2] = pKeyIn->unKeyMixIn.stRockState.u16RockYValue;
		PelcoDGetCheckSum(u8Buf);
		UartSendData(USART2, u8Buf, 7);
	}
	else
	{
		u8 u8Buf[16];
		u8 u8Cmd = pKeyIn->unKeyMixIn.stRockState.u8RockDir << 1;
		static bool boViscaNeedSendZoomStopCmd = false;
		static bool boViscaNeedSendDirStopCmd = false;
		static u8 u8Priority = 0;
		
		u8Cmd &= (PELCOD_DOWN | PELCOD_UP | 
					PELCOD_LEFT | PELCOD_RIGHT |
					PELCOD_ZOOM_TELE | PELCOD_ZOOM_WIDE);
		
		u8Buf[0] = 0x80 + g_u8CamAddr + 1;
		if (u8Priority == 0)
		{
			if ((u8Cmd & (PELCOD_DOWN | PELCOD_UP | PELCOD_LEFT | PELCOD_RIGHT)) != 0)
			{
				u8Priority = 1;
			}
			else if ((u8Cmd & (PELCOD_ZOOM_TELE | PELCOD_ZOOM_WIDE)) != 0)
			{
				u8Priority = 2;
			}
		}
		
		if (u8Priority == 1)
		{
			if (boViscaNeedSendDirStopCmd && 
				((u8Cmd & (PELCOD_DOWN | PELCOD_UP | PELCOD_LEFT | PELCOD_RIGHT)) == 0))
			{
				/* 81 01 06 01 18 18 03 03 FF */
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x06;
				u8Buf[3] = 0x01;
				u8Buf[4] = 0x00;
				u8Buf[5] = 0x00;
				u8Buf[6] = 0x03;
				u8Buf[7] = 0x03;
				u8Buf[8] = 0xFF;
				UartSendData(USART2, u8Buf, 9);
				boViscaNeedSendDirStopCmd = false;
				if ((u8Cmd & (PELCOD_ZOOM_WIDE | PELCOD_ZOOM_WIDE)) != 0)
				{
					u8Priority = 2;
				}
				else
				{
					u8Priority = 0;					
				}
			}
			else
			{
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x06;
				u8Buf[3] = 0x01;
				if ((u8Cmd & (PELCOD_LEFT | PELCOD_RIGHT)) != 0)
				{
					u32 u32Tmp = 0x17 * pKeyIn->unKeyMixIn.stRockState.u16RockXValue;
					u32Tmp /= 0x3F;
					u32Tmp %= 0x18;
					u32Tmp += 1;

					u8Buf[4] = u32Tmp;
					if ((u8Cmd & PELCOD_LEFT) != 0)
					{
						u8Buf[6] = 0x01;
					}
					else
					{
						u8Buf[6] = 0x02;
					}

				}
				else
				{
					u8Buf[4] = 0;
					u8Buf[6] = 0x03;
				}
				
				if ((u8Cmd & (PELCOD_UP | PELCOD_DOWN)) != 0)
				{
					u32 u32Tmp = 0x13 * pKeyIn->unKeyMixIn.stRockState.u16RockYValue;
					u32Tmp /= 0x3F;
					u32Tmp %= 0x14;
					u32Tmp += 1;

					u8Buf[5] = u32Tmp;
					if ((u8Cmd & PELCOD_UP) != 0)
					{
						u8Buf[7] = 0x01;
					}
					else
					{
						u8Buf[7] = 0x02;
					}

				}
				else
				{
					u8Buf[5] = 0;
					u8Buf[7] = 0x03;
				}
				u8Buf[8] = 0xFF;
				UartSendData(USART2, u8Buf, 9);	
				boViscaNeedSendDirStopCmd = true;			
			}	
		}
		
		if (u8Priority == 2)
		{
			if (boViscaNeedSendZoomStopCmd && 
					((u8Cmd & (PELCOD_ZOOM_WIDE | PELCOD_ZOOM_TELE)) == 0))
			{
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = 0x00;
				u8Buf[5] = 0xFF;
				UartSendData(USART2, u8Buf, 6);
				boViscaNeedSendZoomStopCmd = false;
				u8Priority = 0;
			}
			else if ((u8Cmd & PELCOD_ZOOM_WIDE) == PELCOD_ZOOM_WIDE)
			{
				u32 u32Tmp = 0x05 * pKeyIn->unKeyMixIn.stRockState.u16RockZValue;
				u32Tmp /= 0x0F;
				u32Tmp %= 6;
				u32Tmp += 2;
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = u32Tmp + 0x30;
				u8Buf[5] = 0xFF;
				UartSendData(USART2, u8Buf, 6);
				boViscaNeedSendZoomStopCmd = true;

			}
			else
			{
				u32 u32Tmp = 0x05 * pKeyIn->unKeyMixIn.stRockState.u16RockZValue;
				u32Tmp /= 0x0F;
				u32Tmp %= 6;
				u32Tmp += 2;
				u8Buf[1] = 0x01;
				u8Buf[2] = 0x04;
				u8Buf[3] = 0x07;
				u8Buf[4] = 0x20 + u32Tmp;
				u8Buf[5] = 0xFF;
				UartSendData(USART2, u8Buf, 6);			
				boViscaNeedSendZoomStopCmd = true;
			}
			
		}
		
		if (u8Cmd == 0)
		{
			u8Priority = 0;
		}
	}
	return true;
}
static bool PushPodProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	pBuf[_YNA_Cmd] = 0x80;
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.u32PushRodValue;
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	return true;
}


static bool CodeSwitchProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	u16 u16Index;

	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x07;

	u16Index = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Index;
	switch (u16Index)
	{
		case 0x00:
		{
			pBuf[_YNA_Cmd] = 0x46;
			break;
		}
		case 0x01:
		{
			pBuf[_YNA_Cmd] = 0x49;
			break;			
		}
		case 0x02:
		{
			pBuf[_YNA_Cmd] = 0x49;
			pBuf[_YNA_Data1] |= 0x10;
			break;			
		}
		case 0x03:
		{
			pBuf[_YNA_Cmd] = 0x49;
			pBuf[_YNA_Data1] |= 0x20;
			break;			
		}
		default:
			return false;

	}
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Cnt;
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	return true;
	
}
static bool VolumeProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;
	pBuf = (u8 *)MsgOutGetBuf();
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x06;

	pBuf[_YNA_Cmd] = 0x80;
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.u32VolumeValue;
	YNAGetCheckSum(pBuf);
	MsgOutGetEnd();
	return true;
}

static PFun_KeyProcess s_KeyProcessArr[_Key_Reserved] = 
{
	PushPodProcess, KeyBoardProcess, RockProcess,
	CodeSwitchProcess, VolumeProcess,
};
bool KeyProcess(StKeyMixIn *pKeyIn)
{
	if (pKeyIn->emKeyType >= _Key_Reserved)
	{
		return false;
	}
	if (s_KeyProcessArr[pKeyIn->emKeyType] != NULL)
	{
		return s_KeyProcessArr[pKeyIn->emKeyType](pKeyIn);
	}
	return false;
}


bool PCEchoProcess(u8 *pMsg)
{
	u8 u8Cmd = pMsg[_YNA_Cmd];
	bool boIsLight = !pMsg[_YNA_Data3];
	u8 u8Array = pMsg[_YNA_Data1] >> 4;
	pMsg[_YNA_Data1] &= 0x0F;

	if (pMsg[_YNA_Mix] == 0x06)
	{
		switch (u8Cmd)
		{
			case 0x80:
			{
				#define VOLUME_LED (20)
				const u16 u16Led[] = 
				{
					_Led_Voice_1, _Led_Voice_2, _Led_Voice_3, _Led_Voice_4,
					_Led_Voice_5, _Led_Voice_6, _Led_Voice_7, _Led_Voice_8,
					_Led_Voice_9, _Led_Voice_10, _Led_Voice_11, _Led_Voice_12,
					_Led_Voice_13, _Led_Voice_14, _Led_Voice_15, _Led_Voice_16,
					_Led_Voice_17, _Led_Voice_18, _Led_Voice_19, _Led_Voice_20,
					_Led_Voice_21, _Led_Voice_22, _Led_Voice_23, _Led_Voice_24,
				};

				u32 i;
				u8 u8Value = pMsg[_YNA_Data2] * VOLUME_LED / 100;
				if (pMsg[_YNA_Data2] < 5)
				{
					u8Value = pMsg[_YNA_Data2];
				}
				else if (pMsg[_YNA_Data2] > 95)
				{
					u8Value = VOLUME_LED - (100 - pMsg[_YNA_Data2]);
				}
				else
				{
					u8Value = 5 + ((pMsg[_YNA_Data2] - 5) * (VOLUME_LED - 8) + 50)/ 100;
				}
				if (u8Value > VOLUME_LED)
				{
					u8Value = VOLUME_LED;
				}
				for (i = 0; i < (sizeof(u16Led) / sizeof(u16)); i++)
				{
					u16 u16LedLocal = u16Led[i];
					ChangeLedState(GET_X(u16LedLocal), GET_Y(u16LedLocal), false);					
				}
				for (i = 0; i < u8Value; i++)
				{
					u16 u16LedLocal = u16Led[i + 2];
					ChangeLedState(GET_X(u16LedLocal), GET_Y(u16LedLocal), true);					
				}
				break;
			}
			
			case 0xC0:
			{
				if (pMsg[_YNA_Data2] == 0x01)
				{
					u8 *pBuf = (u8 *)MsgOutGetBuf();
					if (pBuf == NULL)
					{
						return false;
					}

					memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

					pBuf[_YNA_Sync] = 0xAA;
					pBuf[_YNA_Addr] = g_u8CamAddr;
					pBuf[_YNA_Mix] = 0x06;
					pBuf[_YNA_Cmd] = 0x80;
					pBuf[_YNA_Data2] = VolumeGetCurValue();
					YNAGetCheckSum(pBuf);
					MsgOutGetEnd();
					break;
				}
				else
				{
					return false;
				}
			}
			default:
				return false;
		}
		return true;
	}

	switch (u8Cmd)
	{
		case 0x43:
		{
			ChangeLedState(GET_X(_Led_Positoin_ZOOM), GET_Y(_Led_Positoin_ZOOM), boIsLight);
			break;
		}
		case 0x44:
		{
			if (u8Array == 0x00)
			{
				const u16 u16Led[] = 
				{
					_Led_VI_V1, _Led_VI_V2, _Led_VI_V3, _Led_VI_V4,
					_Led_VI_1, _Led_VI_2, _Led_VI_3, _Led_VI_4,
					_Led_VI_Auto,
				};
				
				u8 u8Led = pMsg[_YNA_Data2];
				if (u8Led >= (sizeof(u16Led) / sizeof(u16)))
				{
					return false;
				}
				ChangeLedState(GET_X(u16Led[u8Led]), GET_Y(u16Led[u8Led]), boIsLight);
			}
			else if (u8Array == 0x01)
			{	
				u8 u8Led = pMsg[_YNA_Data2];
				if (u8Led < 4)
				{
					u16 u16CamAddr = u8Led;
					if (g_emProtocol == _Protocol_Visca)
					{
						u32 i;
						for (i = 0; i < CAM_ADDR_MAX; i++)
						{
							ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
									GET_Y(g_u16CamLoc[i]), false);
							ChangeLedState(GET_X(g_u16CamLoc[i]), 
									GET_Y(g_u16CamLoc[i]), false);
						}
						ChangeLedState(GET_X(g_u16CamLoc[u16CamAddr]), 
								GET_Y(g_u16CamLoc[u16CamAddr]), true);
					}
					else
					{
						u32 i;
						for (i = 0; i < CAM_ADDR_MAX; i++)
						{
							ChangeLedBlinkState(GET_X(g_u16CamLoc[i]), 
									GET_Y(g_u16CamLoc[i]), false);
							ChangeLedState(GET_X(g_u16CamLoc[i]), 
									GET_Y(g_u16CamLoc[i]), false);
						}
						ChangeLedBlinkState(GET_X(g_u16CamLoc[u16CamAddr]), 
								GET_Y(g_u16CamLoc[u16CamAddr]), true);
					}
					if (pMsg[_YNA_Data1] == 1)
					{
						g_u8CamAddr = u16CamAddr;
					}

				}
				else if (u8Led < 8)
				{
					const u16 u16Led[] = 
					{
						_Led_Positoin_VIN_1,
						_Led_Positoin_VIN_2,
						_Led_Positoin_VIN_3,
						_Led_Positoin_VIN_4,
					};
					u8Led -= 4;
					ChangeLedState(GET_X(u16Led[u8Led]), GET_Y(u16Led[u8Led]), boIsLight);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}			
			break;
		}
		case 0x46:
		{
			CodeSwitchSetValue(0, pMsg[_YNA_Data2]);
			break;
		}
		case 0x47:
		{
			if (u8Array == 0x00)
			{
				switch (pMsg[_YNA_Data2])
				{
					case 0x00:
					{
						ChangeLedState(GET_X(_Led_Record_Live), 
							GET_Y(_Led_Record_Live), boIsLight);
						break;
					}					
					case 0x02:
					{
						ChangeLedState(GET_X(_Led_Record_Record), 
							GET_Y(_Led_Record_Record), boIsLight);
						break;
					}					
					case 0x05:
					{
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsEdit = true;
						}					
						break;
					}					
					case 0x06:
					{
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsEdit = false;
						}					
						break;
					}					
					case 0x0C:
					{
						ChangeLedState(GET_X(_Led_Record_Snap), 
							GET_Y(_Led_Record_Snap), boIsLight);
						break;
					}					
				
					case 0x0D:
					{
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Express), 
							GET_Y(_Led_Delay_Broadcast_Express), boIsLight);
						break;
					}					
					case 0x0E:
					{
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Safe), 
							GET_Y(_Led_Delay_Broadcast_Safe), boIsLight);
						break;
					}					
					default:
						return false;
				}
			}
			else if (u8Array == 0x01)
			{
				switch (pMsg[_YNA_Data2])
				{
					case 0x00:
					{
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Live), 
							GET_Y(_Led_Delay_Broadcast_Live), boIsLight);
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Stop), 
							GET_Y(_Led_Delay_Broadcast_Stop), false);
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsBroadcastPlay = true;
						}
						break;
					}					
					case 0x01:
					{
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Live), 
							GET_Y(_Led_Delay_Broadcast_Live), false);
						ChangeLedState(GET_X(_Led_Delay_Broadcast_Stop), 
							GET_Y(_Led_Delay_Broadcast_Stop), boIsLight);
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsBroadcastPlay = false;
						}
						break;
					}					
					case 0x02:
					{
						ChangeLedState(GET_X(_Led_DDR_Play), 
							GET_Y(_Led_DDR_Play), boIsLight);
						ChangeLedState(GET_X(_Led_DDR_Pause), 
							GET_Y(_Led_DDR_Pause), false);
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsDDRPlay = true;
						}
						break;
					}					
					case 0x03:
					{
						ChangeLedState(GET_X(_Led_DDR_Play), 
							GET_Y(_Led_DDR_Play), false);
						ChangeLedState(GET_X(_Led_DDR_Pause), 
							GET_Y(_Led_DDR_Pause), boIsLight);
						if (pMsg[_YNA_Data1] == 0x01)
						{
							g_boIsDDRPlay = false;
						}
						break;
					}					
					case 0x04:
					{
						ChangeLedState(GET_X(_Led_DDR_Stop), 
							GET_Y(_Led_DDR_Stop), boIsLight);
						break;
					}					
					case 0x07:
					{
						ChangeLedState(GET_X(_Led_DDR_Back), 
							GET_Y(_Led_DDR_Back), boIsLight);
						break;
					}					
					case 0x08:
					{
						ChangeLedState(GET_X(_Led_DDR_Front), 
							GET_Y(_Led_DDR_Front), boIsLight);
						break;
					}					
					case 0x09:
					{
						ChangeLedState(GET_X(_Led_DDR_Cycle), 
							GET_Y(_Led_DDR_Cycle), boIsLight);
						break;
					}					
					case 0x0A:
					{
						ChangeLedState(GET_X(_Led_DDR_Single), 
							GET_Y(_Led_DDR_Single), boIsLight);
						break;
					}					
					case 0x0B:
					{
						ChangeLedState(GET_X(_Led_DDR_Auto), 
							GET_Y(_Led_DDR_Auto), boIsLight);
						break;
					}					
					case 0x0F:
					{
						ChangeLedState(GET_X(_Led_DDR_1), 
							GET_Y(_Led_DDR_1), boIsLight);
						break;
					}					
					case 0x10:
					{
						ChangeLedState(GET_X(_Led_DDR_2), 
							GET_Y(_Led_DDR_2), boIsLight);
						break;
					}					
				
					default:
						return false;
				}
			
			}
			else if (u8Array == 0x02)
			{
				ChangeLedState(GET_X(_Led_Record_ChannelRecord), 
					GET_Y(_Led_Record_ChannelRecord), boIsLight);
			}
			else
			{
				return false;
			}
			
			break;
		}
		case 0x48:
		{
			
			u8 u8Led = pMsg[_YNA_Data2];
			if ((u8Array < 6) && ((u8Led >= 2) && (u8Led <= 5)))
			{
				const u16 u16Led[6][4] = 
				{
					{_Led_PGM_1, _Led_PGM_2, _Led_PGM_3, _Led_PGM_4},
					{_Led_PGM_VGA, _Led_PGM_NET, _Led_PGM_DDR1, _Led_PGM_DDR2},
					{_Led_PGM_V1, _Led_PGM_V2, _Led_PGM_V3, _Led_PGM_V4},
					{_Led_PVW_1, _Led_PVW_2, _Led_PVW_3, _Led_PVW_4},
					{_Led_PVW_VGA, _Led_PVW_NET, _Led_PVW_DDR1, _Led_PVW_DDR2},
					{_Led_PVW_V1, _Led_PVW_V2, _Led_PVW_V3, _Led_PVW_V4},
				};
				u8Led -= 0x02;
				ChangeLedState(GET_X(u16Led[u8Array][u8Led]), 
					GET_Y(u16Led[u8Array][u8Led]), boIsLight);
				do
				{
					BitAction emAction;
					emAction = boIsLight ? Bit_SET : Bit_RESET;
					if (u8Array < 2)
					{
						u8 u8Index = u8Array * 4 + u8Led;
						ExternIOCtrl(u8Index, emAction);
					}
					else if ((u8Array > 2) && (u8Array < 5))
					{
						u8 u8Index = (u8Array - 1) * 4 + u8Led;
						ExternIOCtrl(u8Index, emAction);						
					}
				}while(0);
				break;
			
			}
			else if ((u8Array == 0) && (u8Led <= 0x01))
			{
				if (u8Led == 0x00)
				{
					ChangeLedState(GET_X(_Led_Cutover_Auto), 
						GET_Y(_Led_Cutover_Auto), boIsLight);
				}
				else
				{
					ChangeLedState(GET_X(_Led_Cutover_Cut), 
						GET_Y(_Led_Cutover_Cut), boIsLight);
				}
				break;
			}
			else if ((u8Array == 0) && ((u8Led <= 0x16) && (u8Led >= 0x0E)))
			{
				const u16 u16LedTrick[] = 
				{
					_Led_Trick_Fade_IN_OUT,
					_Led_Trick_Left_Erasure,
					_Led_Trick_Reel,
					_Led_Trick_PIP,
					_Led_Trick_Slide,
					_Led_Trick_Flexible,
					_Led_Trick_3D,
					_Led_Trick_Switch,
					_Led_Trick_Shortcut_1,
				};
				u8Led -= 0x0E;
				ChangeLedState(GET_X(u16LedTrick[u8Led]), 
					GET_Y(u16LedTrick[u8Led]), boIsLight);
				break;
				
			}
			return false;
		}
		case 0x49:
		{
			if (u8Array >= 3)
			{
				return false;
			}
			CodeSwitchSetValue(u8Array + 1, pMsg[_YNA_Data2]);
			break;
		}
		case 0x4A:
		{
			if ((pMsg[_YNA_Data2] >= 0x02) && (pMsg[_YNA_Data2] <= 0x05))
			{
				const u16 u16Led[] = 
				{
					_Led_Cutover_PVM, _Led_Cutover_PGW,
					_Led_Cutover_FTB, _Led_Cutover_VI,
				};
				u16 u16LedCur = u16Led[pMsg[_YNA_Data2] - 0x02];
				ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), boIsLight);
			}
			break;
		}
		case 0x4B:
		{
			if (u8Array > 1)
			{
				return false;
			}
			if (pMsg[_YNA_Data2] <= 0x03)
			{
				const u16 u16Led[2][4] =
				{
					{
						_Led_Caption,
						_Led_Angle1, 
						_Led_Angle2, 
						_Led_Time,
					},
					{
						_Led_Positoin_Caption,
					    _Led_Positoin_Angle1, 
					    _Led_Positoin_Angle2, 
						_Led_Positoin_Time,
					} 
				};					
				u16 u16LedCur = u16Led[u8Array][pMsg[_YNA_Data2]];
				ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), boIsLight);
				break;
			}
			else
			{
				return false;
			}
			
		}
		case 0x4C:
		{
			switch (pMsg[_YNA_Data2])
			{
				case 0x03:
				{
					ChangeLedState(GET_X(_Led_Listen), GET_Y(_Led_Listen), boIsLight);
					break;
				}
				case 0x05:
				{
					ChangeLedState(GET_X(_Led_Look), GET_Y(_Led_Look), boIsLight);
					break;
				}
				case 0x07:
				{
					ChangeLedState(GET_X(_Led_Multi_Views), GET_Y(_Led_Multi_Views), boIsLight);
					break;
				}
				case 0x08:
				{
					ChangeLedState(GET_X(_Led_Projection), GET_Y(_Led_Projection), boIsLight);
					break;
				}
				default:
					break;
			}
			break;
		}
		case 0x80:
		{
			const u16 u16Led[8] = 
			{ 
				_Led_TPush_1, _Led_TPush_2, _Led_TPush_3, _Led_TPush_4,
				_Led_TPush_5, _Led_TPush_6, _Led_TPush_7, _Led_TPush_8,
			};	
			u32 i;
			u32 u32Value = pMsg[_YNA_Data2];
			u32Value &= 0xFF;
			
			for (i = 0; i < 8; i++)
			{
				u16 u16LedCur = u16Led[i];
				ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), false);
			}
			if (u32Value < (PUSH_ROD_MAX_VALUE / 3))
			{
				for (i = 0; i < 4; i++)
				{
					u16 u16LedCur = u16Led[i * 2];
					ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), true);
				}
				
			}
			else if	(u32Value < (PUSH_ROD_MAX_VALUE * 2 / 3))
			{
				for (i = 0; i < 8; i++)
				{
					u16 u16LedCur = u16Led[i];
					ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), true);
				}
			}
			else
			{
				for (i = 0; i < 4; i++)
				{
					u16 u16LedCur = u16Led[i * 2 + 1];
					ChangeLedState(GET_X(u16LedCur), GET_Y(u16LedCur), true);
				}
			}
			
			break;
		}
		case 0xC0:
		{
			if (pMsg[_YNA_Data2] == 0x01)
			{
				switch (pMsg[_YNA_Data3])
				{
					case 0x00:
					{
						u8 *pBuf = (u8 *)MsgOutGetBuf();
						if (pBuf == NULL)
						{
							return false;
						}

						memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

						pBuf[_YNA_Sync] = 0xAA;
						pBuf[_YNA_Addr] = g_u8CamAddr;
						pBuf[_YNA_Mix] = 0x07;
						pBuf[_YNA_Cmd] = 0xC0;
						pBuf[_YNA_Data2] = 0x01;
						YNAGetCheckSum(pBuf);
						MsgOutGetEnd();
						break;
					}
					case 0x02:
					{
						ChangeAllLedState(false);
						/* maybe we need turn on some light */
						GlobalStateInit();
						ExternIOClear();
						break;
					}
					case 0x03:
					{
						u8 *pBuf = (u8 *)MsgOutGetBuf();
						if (pBuf == NULL)
						{
							return false;
						}

						memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

						pBuf[_YNA_Sync] = 0xAA;
						pBuf[_YNA_Addr] = g_u8CamAddr;
						pBuf[_YNA_Mix] = 0x07;
						pBuf[_YNA_Cmd] = 0x80;
						pBuf[_YNA_Data2] = PushRodGetCurValue();
						YNAGetCheckSum(pBuf);
						MsgOutGetEnd();
						break;
					}
				
					default:
						return false;
				}
			}				
			break;
		}
		default :
			return false;
		
	}


	return true;
}


