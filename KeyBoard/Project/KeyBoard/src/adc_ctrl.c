/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：adc_ctrl.c
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "app_port.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"


#include "user_api.h"
#include "key_led_table.h"

#include "flash_save.h"
bool g_boIsPushRodNeedReset = false;

u16 g_u16Times = PUSH_ROD_TIMES;
u16 g_u16UpLimit = PUSH_ROD_END;
u16 g_u16DownLimit = PUSH_ROD_BEGIN;

u16 g_u16VolumeTimes = VOLUME_TIMES;
u16 g_u16VolumeUpLimit = VOLUME_END;
u16 g_u16VolumeDownLimit = VOLUME_BEGIN;


static u16 s_vu16ADCTab[ADC_GET_TOTAL * ADC_GET_CNT];
static StRockState s_stRockState;
static StPushRodState s_stPushRodState;
static StVolumeState s_stVolumeState;


const GPIO_TypeDef *c_pADCInPort[ADC_GET_TOTAL] = 
{
	ADC_PORT_1,		
	ADC_PORT_2,		
	ADC_PORT_3,		
	ADC_PORT_4,		
	ADC_PORT_5,		
	ADC_PORT_6,		
	ADC_PORT_7,		
	ADC_PORT_8,		
};

const u16 c_u16ADCInPin[ADC_GET_TOTAL] = 
{
	ADC_PIN_1,		
	ADC_PIN_2,		
	ADC_PIN_3,		
	ADC_PIN_4,		
	ADC_PIN_5,		
	ADC_PIN_6,		
	ADC_PIN_7,		
	ADC_PIN_8,			
};

const u8 c_u8ADCInChannel[ADC_GET_TOTAL] = 
{
	ADC_CHANNEL_1,		
	ADC_CHANNEL_2,		
	ADC_CHANNEL_3,		
	ADC_CHANNEL_4,		
	ADC_CHANNEL_5,		
	ADC_CHANNEL_6,		
	ADC_CHANNEL_7,		
	ADC_CHANNEL_8,			
};

/* ADC引脚初始化,  */
static void ADCGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	for (i = 0; i < ADC_GET_TOTAL; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16ADCInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pADCInPort[i], &GPIO_InitStructure);	
	}

}
/* ADC时钟源初始化,  */
static void ADCTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	/* 1KHz */
	TIM_InitStructure.TIM_Period = 200;       
	TIM_InitStructure.TIM_Prescaler = (360-1);
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	  
	TIM_TimeBaseInit(TIM2, &TIM_InitStructure);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 100;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
}
/* ADC初始化,  */
static void ADCInit(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;
	u32 i;
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADC1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)s_vu16ADCTab;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC_GET_TOTAL * ADC_GET_CNT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = ADC_GET_TOTAL;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channels configuration */ 
	for (i = 0; i < ADC_GET_TOTAL; i++)
	{
		ADC_RegularChannelConfig(ADC1, c_u8ADCInChannel[i], 
			i + 1, ADC_SampleTime_28Cycles5);
	}
	
	/* Regular discontinuous mode channel number configuration */
	ADC_DiscModeChannelCountConfig(ADC1, ADC_GET_TOTAL);
	/* Enable regular discontinuous mode */
	ADC_DiscModeCmd(ADC1, ENABLE);
	
	/* Enable ADC1 external trigger conversion */ 
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	
#if 0
	/* Configure high and low analog watchdog thresholds */
	ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0834, 0x07D0);		//2000~2100
	/* Enable analog watchdog on one regular channel */
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_AllRegEnable);
#endif
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);  
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	
	TIM_Cmd(TIM2,ENABLE);
	
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}

void RockPushRodInit(void)
{
	ADCGPIOInit();

	ADCTimerInit();
	
	ADCInit();

	memset(&s_stRockState, 0, sizeof(StRockState));
	memset(&s_stPushRodState, 0, sizeof(StPushRodState));
}

static u16 ADCGetAverage(u8 u8Channel) 
{
	u32 u32Sum = 0;
	u32 i;
	u8Channel %= ADC_GET_TOTAL;
	for (i = u8Channel; i < ADC_GET_TOTAL * ADC_GET_CNT; i += ADC_GET_TOTAL)
	{
		u32Sum += s_vu16ADCTab[i];
	}
	u32Sum /= ADC_GET_CNT;
	return (u16)u32Sum;
}


/* 返回 true 时, 有数据, 否则没有数据 */
static bool RockGetValue(StRockState *pRockState)
{
	u32 i;
	u16 *pValue = &(pRockState->u16RockXValue);
	u16 u16RockValue = 0;
	
	pRockState->u8RockDir = 0;
	
	for (i = 0; i < 3; i++)
	{
		u16RockValue = ADCGetAverage(i);

		if(u16RockValue <= ROCK_NEGATIVE_BEGIN)		/* negative */
		{	 
			if(u16RockValue <= ROCK_NEGATIVE_END)
			{
				pValue[i] = ROCK_MAX_VALUE;
			}
			else
			{
				pValue[i] = (ROCK_NEGATIVE_BEGIN - u16RockValue) / ROCK_TIMES;				
			}
			if (i == 0)
			{
				pRockState->u8RockDir |= (0x01 << ( i << 1 )); /* left */
			}
			else
			{
				pRockState->u8RockDir |= (0x02 << ( i << 1 )); /* down, wide */
			}
		}
		else if (u16RockValue >= ROCK_POSITIVE_BEGIN)	/* positive */
		{ 	 
			if(u16RockValue >= ROCK_POSITIVE_END)
			{
				pValue[i] = ROCK_MAX_VALUE;
			}
			else
			{
				pValue[i] = (u16RockValue - ROCK_POSITIVE_BEGIN) / ROCK_TIMES;				
			}
			if (i == 0)
			{
				pRockState->u8RockDir |= (0x02 << ( i << 1 )); /* right */
			}
			else
			{
				pRockState->u8RockDir |= (0x01 << ( i << 1 ));/* up, tele */
			}
		}
		else
		{
			pValue[i] = 0;
		}
	}

	pRockState->u16RockXValue >>= 1;
	pRockState->u16RockYValue >>= 1;
	pRockState->u16RockZValue >>= 3;
	
	if (pRockState->u8RockDir == 0)
	{
		if (pRockState->u8RockOldDir != 0)
		{
			memset(pRockState, 0, sizeof(StRockState));
			return true; /* rocker stop */
		}
		return false;
	}
	if (pRockState->u8RockOldDir == pRockState->u8RockDir)
	{
		u8 u8Dir = pRockState->u8RockDir;
		bool boReturn = false;
		if ((u8Dir & (ROCK_X_NEGATIVE_DIR | ROCK_X_POSITIVE_DIR)) != 0)
		{
			if (pRockState->u16RockXValue != pRockState->u16RockXOldValue)
			{
				boReturn = true;
				pRockState->u16RockXOldValue = 
					pRockState->u16RockXValue;
			}
		}
		if ((u8Dir & (ROCK_Y_NEGATIVE_DIR | ROCK_Y_POSITIVE_DIR)) != 0)
		{
			if (pRockState->u16RockYValue != pRockState->u16RockYOldValue)
			{
				boReturn = true;
				pRockState->u16RockYOldValue = 
					pRockState->u16RockYValue;
			}
		}
		if ((u8Dir & (ROCK_Z_NEGATIVE_DIR | ROCK_Z_POSITIVE_DIR)) != 0)
		{
			if (pRockState->u16RockZValue != pRockState->u16RockZOldValue)
			{
				boReturn = true;
				pRockState->u16RockZOldValue = 
					pRockState->u16RockZValue;
			}
		}
		return boReturn;
		
	}
	else
	{
		pRockState->u8RockOldDir = pRockState->u8RockDir;
	}
	return true;
}


void PushRodSetValue(u8 u8Value)
{
	if (u8Value > PUSH_ROD_MAX_VALUE)
	{
		u8Value = PUSH_ROD_MAX_VALUE;
	}
	s_stPushRodState.u8PushRodLEDValue = u8Value / PUSH_ROD_DIFF;
}

u8 PushRodGetCurValue(void)
{
	return s_stPushRodState.u8PushRodValue;
}


/* */
static bool PushRodGetValue(StPushRodState *pPushRodState)
{
	u16 u16Value = ADCGetAverage(PUSH_ROD_CHANNEL);
	u16 u16OldValue = pPushRodState->u8PushRodOldValue;

#if KEYBOARD_UNION
	#define PUSH_ROD_DIFF_UNION	(4000 / (PUSH_ROD_MAX_VALUE + 1))
	if (u16Value < 48)
	{
		u16Value = 0;
	}
	else
	{
		u16Value -= 48;
	}
	if (u16Value >= 4000)
	{
		u16Value = 4000;
	}
	u16Value /= PUSH_ROD_DIFF_UNION; /* 0 ~ PUSH_ROD_MAX_VALUE */

	if (u16Value > PUSH_ROD_MAX_VALUE)
	{
		u16Value = PUSH_ROD_MAX_VALUE;
	}
	
	if (u16Value != u16OldValue)
	{
		pPushRodState->u8PushRodValue = pPushRodState->u8PushRodOldValue = u16Value;
		return true;
	}
#else
	if(u16Value >= g_u16DownLimit)
	{	 
		u16Value = 0;
	}
	else if(u16Value <= g_u16UpLimit)
	{
		u16Value = PUSH_ROD_MAX_VALUE;
	}
	else
	{
		u16Value =	g_u16DownLimit - u16Value;
		u16Value /= g_u16Times;
	}
	if (u16OldValue != u16Value)
	{
		if (g_boIsPushRodNeedReset)
		{
			u16Value -= u16Value % PUSH_ROD_DIFF;
			if (u16Value != (pPushRodState->u8PushRodLEDValue * PUSH_ROD_DIFF))
			{
				pPushRodState->u8PushRodValue = pPushRodState->u8PushRodOldValue = u16Value;
				return false;
			}
			g_boIsPushRodNeedReset = false;
		}
		pPushRodState->u8PushRodValue = pPushRodState->u8PushRodOldValue = u16Value;
		return true;

	}
#endif
	return false;

}


void RockFlush(void)
{
	if (RockGetValue(&s_stRockState))
	{
		StKeyMixIn *pBuf = KeyBufWriteBegin();
		if (pBuf == NULL)
		{
			return;
		}
		pBuf->emKeyType = _Key_Rock;
		memcpy(&(pBuf->unKeyMixIn.stRockState), &s_stRockState, 
			sizeof(StRockState));
		KeyBufWriteEnd();
	}
}
void PushRodFlush(void)
{
	if (PushRodGetValue(&s_stPushRodState))
	{
		StKeyMixIn *pBuf = KeyBufWriteBegin();
		if (pBuf == NULL)
		{
			return;
		}
		pBuf->emKeyType = _Key_Push_Rod;
		pBuf->unKeyMixIn.u32PushRodValue = s_stPushRodState.u8PushRodValue;
		KeyBufWriteEnd();
	}
}




static bool VolumeGetValue(StVolumeState *pVolumeState)
{
	u16 u16Value = ADCGetAverage(VOLUME_CHANNEL);
	u16 u16OldValue = pVolumeState->u8VolumeOldValue;
	u16 u16VolumeRealValue = 0;
	
	if (u16Value < g_u16VolumeDownLimit)
	{
		u16Value = 0;
	}
	else
	{
		u16Value -= g_u16VolumeDownLimit;
	}
	if (u16Value > (g_u16VolumeUpLimit - g_u16VolumeDownLimit))
	{
		u16Value = (g_u16VolumeUpLimit - g_u16VolumeDownLimit);
	}
	u16VolumeRealValue = pVolumeState->u16VolumeRealValue;
	if (u16VolumeRealValue > u16Value)
	{
		u16 u16Tmp = u16VolumeRealValue - u16Value;
		if (u16Tmp < (g_u16VolumeTimes / 4) )
		{
			return false;
		}
	}
	else
	{
		u16 u16Tmp = u16Value - u16VolumeRealValue;
		if (u16Tmp < (g_u16VolumeTimes / 4) )
		{
			return false;
		}

	}
	u16VolumeRealValue = u16Value;
	u16Value /= g_u16VolumeTimes;	/* 0~100 */
	if (u16Value > VOLUME_MAX_VALUE)
	{
		u16Value = VOLUME_MAX_VALUE;
	}
	if (u16Value != u16OldValue)
	{
		pVolumeState->u8VolumeOldValue = pVolumeState->u8VolumeValue = u16Value;
		pVolumeState->u16VolumeRealValue = u16VolumeRealValue;
		return true;
	}
	return false;
	
}

u8 VolumeGetCurValue(void)
{
	return s_stVolumeState.u8VolumeOldValue;
}

void VolumeFlush(void)
{
	if (VolumeGetValue(&s_stVolumeState))
	{
		StKeyMixIn *pBuf = KeyBufWriteBegin();
		if (pBuf == NULL)
		{
			return;
		}
		pBuf->emKeyType = _Key_Volume;
		pBuf->unKeyMixIn.u32VolumeValue = s_stVolumeState.u8VolumeValue;
		KeyBufWriteEnd();
	}
}





static bool  PushRodGetTheRedressLimit(u16 u16UpLimit, u16 u16DownLimit)
{
	u16 u16Diff = 0;
	u16 u16Times = 0;
	u16 u16Ignore = 0;  

	if (u16UpLimit > u16DownLimit)
	{
		return false;
	}

	u16Diff = u16DownLimit - u16UpLimit;

	if (u16Diff < ((PUSH_ROD_MAX_VALUE + 1) * (PUSH_ROD_MIN_TIMES - 1)))
	{
		return false;
	}
	u16Times = u16Diff / (PUSH_ROD_MAX_VALUE + 1);
	
	u16Ignore = u16Diff - (u16Times * (PUSH_ROD_MAX_VALUE + 1));
	if ( u16Ignore < (PUSH_ROD_MAX_VALUE / 2))
	{
		u16Times -= 1;		
		u16Ignore = u16Diff - (u16Times * (PUSH_ROD_MAX_VALUE + 1));
	}
	
	u16Ignore /= 2;
	
	u16UpLimit += u16Ignore;

	u16DownLimit = u16UpLimit + (u16Times * (PUSH_ROD_MAX_VALUE + 1)) - 1;

	g_u16Times = u16Times;
	g_u16UpLimit = u16UpLimit;
	g_u16DownLimit = u16DownLimit;
	return true;
}

void RedressPushRodLimit (void)
{
	u32 u32MsgSentTime;
	u32 u32State = 0;
	u16 u16UpLimit = 0, u16DownLimit = 0;
	StKeyMixIn *pKeyIn;

	ChangeAllLedState(false);
	ChangeLedState(GET_X(_Led_Cutover_VI), GET_Y(_Led_Cutover_VI), true);
	do 
	{
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd();
	}while(1);
	
	u32MsgSentTime = g_u32SysTickCnt;
	while(1)
	{
		StKeyState *pKey;
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		
		if (SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) > 10000) /* 10S */
		{
			ChangeAllLedState(true);
			return;
		}
		
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
		if (u32State == 0) /* get the down limit */
		{
			if (pKey->u8KeyValue == _Key_Cutover_Auto)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_X(_Led_Cutover_Auto), GET_Y(_Led_Cutover_Auto), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_X(_Led_Cutover_Auto), GET_Y(_Led_Cutover_Auto), false);
					u16DownLimit = ADCGetAverage(PUSH_ROD_CHANNEL);
					u32State = 1;
				}
			}
		}
		else if (u32State == 1) /* get the up limit */
		{
			if (pKey->u8KeyValue == _Key_Cutover_FTB)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_X(_Led_Cutover_FTB), GET_Y(_Led_Cutover_FTB), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_X(_Led_Cutover_FTB), GET_Y(_Led_Cutover_FTB), false);
					u16UpLimit = ADCGetAverage(PUSH_ROD_CHANNEL);
					break;
				}
			}			
		}
		KeyBufGetEnd();
	}



	if (PushRodGetTheRedressLimit(u16UpLimit, u16DownLimit))
	{
		if (WriteSaveData())
		{
			ChangeLedState(GET_X(_Led_Cutover_VI), GET_Y(_Led_Cutover_VI), false);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1000);/* 延时1s */
			ChangeAllLedState(true);
			return;
		}
	}

	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_Cutover_VI), GET_Y(_Led_Cutover_VI), boBlink);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 100);/* 延时1s */
			u32BlinkCnt++;
		}
	}
	
	ChangeAllLedState(true);
}




static bool  VolumeGetTheRedressLimit(u16 u16UpLimit, u16 u16DownLimit)
{
	u16 u16Diff = 0;
	u16 u16Times = 0;
	u16 u16Ignore = 0;  

	if (u16UpLimit < u16DownLimit)
	{
		return false;
	}

	u16Diff = u16UpLimit - u16DownLimit;

	if (u16Diff < ((VOLUME_MAX_VALUE + 1) * (VOLUME_MIN_TIMES - 1)))
	{
		return false;
	}
	u16Times = u16Diff / (VOLUME_MAX_VALUE + 1);
	
	u16Ignore = u16Diff - (u16Times * (VOLUME_MAX_VALUE + 1));
	if ( u16Ignore < (VOLUME_MAX_VALUE / 2))
	{
		u16Times -= 1;		
		u16Ignore = u16Diff - (u16Times * (VOLUME_MAX_VALUE + 1));
	}
	
	u16Ignore /= 2;
	
	u16DownLimit += u16Ignore;

	u16UpLimit = u16DownLimit + (u16Times * (VOLUME_MAX_VALUE + 1)) - 1;

	g_u16VolumeTimes = u16Times;
	g_u16VolumeUpLimit = u16UpLimit;
	g_u16VolumeDownLimit = u16DownLimit;
	return true;
}

void RedressVolumeLimit(void)
{
	u32 u32MsgSentTime;
	u32 u32State = 0;
	u16 u16UpLimit = 0, u16DownLimit = 0;
	StKeyMixIn *pKeyIn;

	ChangeAllLedState(false);
	ChangeLedState(GET_X(_Led_Time), GET_Y(_Led_Time), true);
	do 
	{
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		if (pKeyIn == NULL)
		{
			break;
		}
		KeyBufGetEnd();
	}while(1);
	u32MsgSentTime = g_u32SysTickCnt;
	while(1)
	{
		StKeyState *pKey;
		pKeyIn = (StKeyMixIn *)KeyBufGetBuf();
		
		if (SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) > 10000) /* 10S */
		{
			ChangeAllLedState(true);
			return;
		}
		
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
		if (u32State == 0) /* get the down limit */
		{
			if (pKey->u8KeyValue == _Key_Positoin_Caption)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_X(_Led_Positoin_Caption), GET_Y(_Led_Positoin_Caption), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_X(_Led_Positoin_Caption), GET_Y(_Led_Positoin_Caption), false);
					u16DownLimit = ADCGetAverage(VOLUME_CHANNEL);
					u32State = 1;
				}
			}
		}
		else if (u32State == 1) /* get the up limit */
		{
			if (pKey->u8KeyValue == _Key_Positoin_1)
			{
				u32MsgSentTime = g_u32SysTickCnt;
				if (pKey->u8KeyState == KEY_DOWN)
				{
					ChangeLedState(GET_X(_Led_Positoin_1), GET_Y(_Led_Positoin_1), true);
				}
				else if (pKey->u8KeyState == KEY_UP)
				{
					ChangeLedState(GET_X(_Led_Positoin_1), GET_Y(_Led_Positoin_1), false);
					u16UpLimit = ADCGetAverage(VOLUME_CHANNEL);
					break;
				}
			}			
		}
		KeyBufGetEnd();
	}



	if (VolumeGetTheRedressLimit(u16UpLimit, u16DownLimit))
	{
		if (WriteSaveData())
		{
			ChangeLedState(GET_X(_Led_Time), GET_Y(_Led_Time), false);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 1000);/* ??1s */
			ChangeAllLedState(true);
			return;
		}
	}

	{
		bool boBlink = true;
		u32 u32BlinkCnt = 0;
		while (u32BlinkCnt < 10)
		{
			boBlink = !boBlink;
			ChangeLedState(GET_X(_Led_Time), GET_Y(_Led_Time), boBlink);
			u32MsgSentTime = g_u32SysTickCnt;
			while(SysTimeDiff(u32MsgSentTime, g_u32SysTickCnt) < 100);/* ??1s */
			u32BlinkCnt++;
		}
	}
	
	ChangeAllLedState(true);
}
