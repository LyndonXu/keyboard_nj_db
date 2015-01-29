/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_swtich.c
* 摘要: PWM控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2014年11月14日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "pwm.h"
#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"


#include "user_api.h"
#include "key_led_table.h"

#include "flash_save.h"

u16 g_u16LedOnLight = 0;
u16 g_u16LedOffLight = PWM_RESOLUTION;

void PWMCtrlInit()
{

	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;


	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);

	ENABLE_PWM_TIMER();	

	GPIO_InitStructure.GPIO_Pin = MCU_PWM1 | MCU_PWM2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(MCU_PWM_PORT, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = PWM_RESOLUTION - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 18;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(MCU_PWM_TIMER, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = g_u16LedOnLight;//PWM_RESOLUTION >> 1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	MCU_PWM1_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_Pulse = g_u16LedOffLight;//PWM_RESOLUTION >> 1;
	MCU_PWM2_OCInit(MCU_PWM_TIMER, &TIM_OCInitStructure);
	
	MCU_PWM1_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	MCU_PWM2_OCPreloadConfig(MCU_PWM_TIMER, TIM_OCPreload_Enable);
	
	
	TIM_Cmd(MCU_PWM_TIMER, ENABLE);

}
u16 PWM1GetValue(void)
{
	return MCU_PWM1_CCR;
}

void PWM1Plus(void)
{
	u16 u16Tmp = MCU_PWM1_CCR;
	u16Tmp++;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = 0;
	}
	MCU_PWM1_CCR = u16Tmp;
}


void PWM1Minus(void)
{
	u16 u16Tmp = MCU_PWM1_CCR;
	u16Tmp--;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = PWM_RESOLUTION - 1;
	}
	MCU_PWM1_CCR = u16Tmp;

}

u16 PWM2GetValue(void)
{
	return MCU_PWM2_CCR;
}

void PWM2Plus(void)
{
	u16 u16Tmp = MCU_PWM2_CCR;
	u16Tmp++;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = 0;
	}
	MCU_PWM2_CCR = u16Tmp;
}
void PWM2Minus(void)
{
	u16 u16Tmp = MCU_PWM2_CCR;
	u16Tmp--;
	if (u16Tmp >= PWM_RESOLUTION)
	{
		u16Tmp = PWM_RESOLUTION - 1;
	}
	MCU_PWM2_CCR = u16Tmp;

}

void RedressLedOnLight(void)
{
	StKeyMixIn *pKeyIn;

	u32 u32PlusBeginTime;
	u32 u32MinusBeginTime;

	u32 u32PlusCount;
	u32 u32MinusCount;
	
	bool boIsChange = false;
	ChangeAllLedState(true);
	ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), false);
	do 
	{
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd();
	}while(1);
	while(1)
	{
		StKeyState *pKey;
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		
		if (pKeyIn == NULL)
		{
			continue;
		}

		if (pKeyIn->emKeyType != _Key_Board)
		{
			KeyBufGetEnd();
			continue;
		}
		pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
		
		if (pKey->u8KeyValue == _Key_PGM_V4)	/* light Plus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32PlusBeginTime = g_u32SysTickCnt;
				u32PlusCount = 0;
				PWM1Minus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32PlusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32PlusCount++ > 5)
					{
						u32PlusCount = 0;
						PWM1Minus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), false);
			}

		}
		else if (pKey->u8KeyValue == _Key_PVW_V4)	/* light minus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32MinusBeginTime = g_u32SysTickCnt;
				u32MinusCount = 0;
				PWM1Plus();
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32MinusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32MinusCount++ > 5)
					{
						u32MinusCount = 0;
						PWM1Plus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), false);
			}
		}
		else if (pKey->u8KeyValue == _Key_DDR_Auto)
		{
			KeyBufGetEnd();
			break;
		}
		KeyBufGetEnd();
	}
	
	if (boIsChange)
	{
		g_u16LedOnLight = PWM1GetValue();
		if (WriteSaveData())
		{
			u32 u32Time = g_u32SysTickCnt;
			ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), false);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return;
		}
	}
	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			u32 u32Time = g_u32SysTickCnt;
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_PGM_1), GET_Y(_Led_PGM_1), boBlink);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}
}

void RedressLedOffLight(void)
{
	StKeyMixIn *pKeyIn;

	u32 u32PlusBeginTime;
	u32 u32MinusBeginTime;

	u32 u32PlusCount;
	u32 u32MinusCount;
	
	bool boIsChange = false;
	ChangeAllLedState(false);
	ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), true);
	do 
	{
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd();
	}while(1);
	while(1)
	{
		StKeyState *pKey;
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		
		if (pKeyIn == NULL)
		{
			continue;
		}

		if (pKeyIn->emKeyType != _Key_Board)
		{
			KeyBufGetEnd();
			continue;
		}
		pKey = &(pKeyIn->unKeyMixIn.stKeyState[0]);
		
		if (pKey->u8KeyValue == _Key_PGM_V4)	/* light Plus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32PlusBeginTime = g_u32SysTickCnt;
				u32PlusCount = 0;
				PWM2Minus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32PlusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32PlusCount++ > 5)
					{
						u32PlusCount = 0;
						PWM2Minus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PGM_V4), GET_Y(_Led_PGM_V4), false);
			}

		}
		else if (pKey->u8KeyValue == _Key_PVW_V4)	/* light minus */
		{
			if (pKey->u8KeyState == KEY_DOWN)
			{
				u32MinusBeginTime = g_u32SysTickCnt;
				u32MinusCount = 0;
				PWM2Plus();
				boIsChange = true;
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), true);
			}
			else if (pKey->u8KeyState == KEY_KEEP)
			{
				if (SysTimeDiff(u32MinusBeginTime, g_u32SysTickCnt) > 1000)
				{
					if (u32MinusCount++ > 5)
					{
						u32MinusCount = 0;
						PWM2Plus();
						boIsChange = true;
					}
				}
			}
			else
			{
				ChangeLedState(GET_X(_Led_PVW_V4), GET_Y(_Led_PVW_V4), false);
			}
		}
		else if (pKey->u8KeyValue == _Key_DDR_Auto)
		{
			KeyBufGetEnd();
			break;
		}
		KeyBufGetEnd();
	}
	
	if (boIsChange)
	{
		g_u16LedOffLight = PWM2GetValue();
		if (WriteSaveData())
		{
			u32 u32Time = g_u32SysTickCnt;
			ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), false);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return;
		}
	}
	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			u32 u32Time = g_u32SysTickCnt;
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_PVW_1), GET_Y(_Led_PVW_1), boBlink);
			while(SysTimeDiff(u32Time, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}

	ChangeAllLedState(true);
}



