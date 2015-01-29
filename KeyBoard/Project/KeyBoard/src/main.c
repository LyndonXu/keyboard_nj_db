#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
#include "pwm.h"


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

int main()
{
	u32 u32MsgSentTime;
	u32 u32SyncCnt = 0;


	
	MsgBufInit();	
	KeyBufInit();
	GlobalStateInit();
	
	PeripheralPinClkEnable();
	OpenSpecialGPIO();

	ReadSaveData();
	
	KeyLedInit();
	RockPushRodInit();
	CodeSwitchInit();
	BuzzerInit();
	MessageUARTInit();
	PWMCtrlInit();
	

	SysTickInit();

	/* 打开所有LED */
#if 0
	ChangeAllLedState(true);
	u32MsgSentTime = g_u32SysTickCnt;
	while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1500);/* 延时1.5S */
	ChangeAllLedState(false);
#endif


#if 1
	/*  */
	do
	{
		/* ............. */
		bool boIsCheckOK = false;
		
		const StUID *pUID = (StUID *)UID_BASE_ADDR;
		u32 u32PrevAlarmTime = 0;
		
		boIsCheckOK = CheckUID(pUID);
		while (!boIsCheckOK)
		{
			u8 *pMsgIn = (u8 *)MsgInGetBuf();
			FlushBuzzer();

			if (SysTimeDiff(u32PrevAlarmTime, g_u32SysTickCnt) > 5000)
			{
				StartBuzzer(_Buz_Msg_Sync_Err);
				u32PrevAlarmTime = g_u32SysTickCnt;
			}

			if (pMsgIn == NULL)
			{
				continue;
			}
			{
				u8 u8Code1 = pMsgIn[_YNA_Mix], u8Code2 = pMsgIn[_YNA_Cmd];
				u8Code1 ^= 0x55;
				u8Code1 ^= pMsgIn[_YNA_Addr];

				u8Code2 ^= 0xAA;
				u8Code2 ^= pMsgIn[_YNA_Addr];
				
				if ((u8Code1 == 0x07) && (u8Code2 == 0xC0))
				{
					u16 u16Rand = pMsgIn[_YNA_Data3];
					u16Rand <<= 8;
					u16Rand |= pMsgIn[_YNA_Data2];
					boIsCheckOK = WriteUID(pUID, u16Rand);
				}
			}
			MsgInGetEnd();
		}
	}while(0);
#endif


	ChangeEncodeState();

	do
	{
		u32 u32Cnt = 0;	
		StKeyMixIn *pKeyIn;
		while(u32Cnt < 10)
		{
			
			u32Cnt++;
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 150);/* ??150ms */

			pKeyIn = (StKeyMixIn *)KeyBufGetBuf();

			if (pKeyIn == NULL)
			{
				continue;
			}
			if (pKeyIn->emKeyType == _Key_Board)
			{
				if (pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_Cutover_VI)
				{
					KeyBufGetEnd();
					RedressPushRodLimit();
					break;
				}
				else if (pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_Time)
				{
					KeyBufGetEnd();
					RedressVolumeLimit();
					break;
				}
				else if (pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_PGM_1)
				{
					KeyBufGetEnd();
					RedressLedOnLight();
					break;
				}
				else if (pKeyIn->unKeyMixIn.stKeyState[0].u8KeyValue == _Key_PVW_1)
				{
					KeyBufGetEnd();
					RedressLedOffLight();
					break;
				}
			}
			KeyBufGetEnd();
		}
		if (u32Cnt >= 10)
		{
			break;
		}
		
		ChangeAllLedState(false);
		u32MsgSentTime = g_u32SysTickCnt;
		while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 100);
		ChangeAllLedState(true);
	}while(0);

	u32SyncCnt = 0;

	while (u32SyncCnt < 10)
	{
		u8 *pBuf = NULL;
		void *pMsgIn = NULL; 

		u32MsgSentTime = g_u32SysTickCnt;

		pBuf = MsgOutGetBuf();
		if (pBuf == NULL)
		{
			break; /* 严重错误 */
		}
		memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

		pBuf[_YNA_Sync] = 0xAA;
		pBuf[_YNA_Mix] = 0x07;
		pBuf[_YNA_Cmd] = 0xC0;

		YNAGetCheckSum(pBuf);
		MsgOutGetEnd();
		pMsgIn = MsgInGetBuf();
		if (pMsgIn != NULL)
		{
			break;
		}
		u32SyncCnt++;		
		while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1000);/* 延时1S */
	}

	ChangeAllLedState(false);
	GlobalStateInit();
	do 
	{
		StKeyMixIn *pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd();
	}while(1);

	while (1)
	{
		void *pMsgIn = MsgInGetBuf();

		StKeyMixIn *pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn != NULL)
		{
			KeyProcess(pKeyIn);
			KeyBufGetEnd();
		}

		
		if (pMsgIn != NULL)
		{
			PCEchoProcess(pMsgIn);
			MsgInGetEnd();
		}
	}
	
}
