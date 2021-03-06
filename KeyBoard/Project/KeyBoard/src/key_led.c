/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：key_led.c
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
#include "user_api.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#include "key_led_table.h"

static StKeyScan s_stKeyScan;
static StLedScan s_stLedScan;
static StLedSize s_stLedBlinkState[LED_Y_CNT];


const GPIO_TypeDef *c_pKeyInPort[KEY_X_CNT] = 
{
	KEY_X_PORT_1,		
	KEY_X_PORT_2,		
	KEY_X_PORT_3,		
	KEY_X_PORT_4,		
	KEY_X_PORT_5,		
	KEY_X_PORT_6,		
	KEY_X_PORT_7,		
	KEY_X_PORT_8,		
};

const u16 c_u16KeyInPin[KEY_X_CNT] = 
{
	KEY_X_1,		
	KEY_X_2,		
	KEY_X_3,		
	KEY_X_4,		
	KEY_X_5,		
	KEY_X_6,		
	KEY_X_7,		
	KEY_X_8,			
};

GPIO_TypeDef *const c_pKeyPowerPort[KEY_Y_CNT] = 
{
	KEY_POWER_PORT_1,		
	KEY_POWER_PORT_2,		
	KEY_POWER_PORT_3,		
	KEY_POWER_PORT_4,		
	KEY_POWER_PORT_5,		
	KEY_POWER_PORT_6,		
	KEY_POWER_PORT_7,		
	KEY_POWER_PORT_8,		
	KEY_POWER_PORT_9,		
	KEY_POWER_PORT_10,		
	KEY_POWER_PORT_11,		
	KEY_POWER_PORT_12,		
};

const u16 c_u16KeyPowerPin[KEY_Y_CNT] = 
{
	KEY_POWER_1,		
	KEY_POWER_2,		
	KEY_POWER_3,		
	KEY_POWER_4,		
	KEY_POWER_5,		
	KEY_POWER_6,		
	KEY_POWER_7,		
	KEY_POWER_8,		
	KEY_POWER_9,		
	KEY_POWER_10,		
	KEY_POWER_11,		
	KEY_POWER_12,		
};


const GPIO_TypeDef *c_pLedInPort[LED_X_CNT] = 
{
	LED_X_PORT_1,		
	LED_X_PORT_2,		
	LED_X_PORT_3,		
	LED_X_PORT_4,		
	LED_X_PORT_5,		
	LED_X_PORT_6,		
	LED_X_PORT_7,		
	LED_X_PORT_8,		
	LED_X_PORT_9,		
	LED_X_PORT_10,		
	LED_X_PORT_11,		
	LED_X_PORT_12,		
	LED_X_PORT_13,		
	LED_X_PORT_14,		
	LED_X_PORT_15,		
	LED_X_PORT_16,		
};

const u16 c_u16LedInPin[LED_X_CNT] = 
{
	LED_X_1,		
	LED_X_2,		
	LED_X_3,		
	LED_X_4,		
	LED_X_5,		
	LED_X_6,		
	LED_X_7,		
	LED_X_8,			
	LED_X_9,		
	LED_X_10,		
	LED_X_11,		
	LED_X_12,		
	LED_X_13,		
	LED_X_14,		
	LED_X_15,		
	LED_X_16,		
};


const GPIO_TypeDef *c_pLedPowerPort[LED_Y_CNT] = 
{
	LED_POWER_PORT_1,		
	LED_POWER_PORT_2,		
	LED_POWER_PORT_3,		
	LED_POWER_PORT_4,		
	LED_POWER_PORT_5,		
	LED_POWER_PORT_6,		
	LED_POWER_PORT_7,		
	LED_POWER_PORT_8,		
	LED_POWER_PORT_9,		
	LED_POWER_PORT_10,		
	LED_POWER_PORT_11,		
	LED_POWER_PORT_12,		
};

const u16 c_u16LedPowerPin[LED_Y_CNT] = 
{
	LED_POWER_1,		
	LED_POWER_2,		
	LED_POWER_3,		
	LED_POWER_4,		
	LED_POWER_5,		
	LED_POWER_6,		
	LED_POWER_7,		
	LED_POWER_8,		
	LED_POWER_9,		
	LED_POWER_10,		
	LED_POWER_11,		
	LED_POWER_12,		
};


/* 扫描整个键盘一边 */
/* power 为Y 方向，输入为X 方向 */
static void KeyScanOnce(StKeyScan *pKey)
{
	u32 i;
	u32 u32Cnt = pKey->u32ScanCnt % KEY_SCAN_CNT;
	for (i = 0; i < KEY_Y_CNT; i++)
	{
		u16 u16KeyValue, u16Tmp;
		u32 j;
		for (j = 0; j < KEY_Y_CNT; j++)
		{
#if 0
			GPIO_WriteBit((GPIO_TypeDef *)c_pKeyPowerPort[j], 
				c_u16KeyPowerPin[j], Bit_SET);
#else
			c_pKeyPowerPort[j]->BSRR = c_u16KeyPowerPin[j];
#endif			
		}
#if 0		
		GPIO_WriteBit((GPIO_TypeDef *)c_pKeyPowerPort[i], 
			c_u16KeyPowerPin[i], Bit_RESET);
#else
		c_pKeyPowerPort[i]->BRR = c_u16KeyPowerPin[i];
#endif		

		for (j = 0; j < 40; j++);
		__NOP();
		__NOP();
		__NOP();
		
		u16KeyValue = 0;
		for (j = 0; j < KEY_X_CNT; j++)
		{
#if 0
			u16Tmp = GPIO_ReadInputDataBit((GPIO_TypeDef *)c_pKeyInPort[j],
				c_u16KeyInPin[j]);
#else
			u16Tmp = c_pKeyInPort[j]->IDR & c_u16KeyInPin[j];
			u16Tmp = !!u16Tmp;
#endif
			u16KeyValue |= (u16Tmp << j);
		}

		pKey->stKeyTmp[u32Cnt].u8KeyValue[i] = u16KeyValue & 0xFF;
	}
	c_pKeyPowerPort[i - 1]->BSRR = c_u16KeyPowerPin[i];

	pKey->u32ScanCnt++;	
}

/* 消抖, 正确返回true, 并在pKeyOut中填写数据, 否则返回false */
static bool KeyCheckValue(StKeyValue *pKeyTmp, StKeyValue *pKeyOut)
{
	s32 i, j, s32NotSameCnt;
	s32 s32ValidRow = 0xFF;
	s32NotSameCnt = 0;
	for (i = 0; i < (KEY_SCAN_CNT - 1); i++)
	{
		for (j = 0; j < KEY_X_CNT; j++)
		{
			if(pKeyTmp[i].u8KeyValue[j] != pKeyTmp[i + 1].u8KeyValue[j])
			{
				s32NotSameCnt++;
				break;
			}			
		}
		if (j == KEY_X_CNT)
		{
			s32ValidRow = i;			/* 取两组相同的数据 */
		}
	}
	if (s32NotSameCnt > ((KEY_X_CNT >> 1) + 1))		
	{
		return false; /* 半数以上不一致 */
	}
	if (s32ValidRow >= KEY_SCAN_CNT)
	{
		return false;
	}		

	*pKeyOut = pKeyTmp[s32ValidRow];

	return true;
}

/*校验键位冲突*/
static bool KeyClashCheck(StKeyState *pKeyState, u8 u8Cnt)
{
	u8 a,b,c;

	for (a = 0; a < (u8Cnt - 1); a++)
	{
	 	for( b = (a + 1); b < u8Cnt; b++)
		{
			/* Y 方向 数值一致 */
			if((pKeyState[a].u8KeyLocation & 0xF0) == (pKeyState[b].u8KeyLocation & 0xF0))
			{
				
				/* 检测 a 方向 */
				for(c = 0; c < a; c++)
				{			
					/* X 方向 数值一致 */
					if ((pKeyState[a].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				for(c = (a + 1); c < u8Cnt; c++)
				{
					if ((pKeyState[a].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				/* 检测 b 方向 */
				for(c = 0; c < b; c++)
				{
					if ((pKeyState[b].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				for(c = (b + 1); c < u8Cnt; c++)
				{
					if ((pKeyState[b].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}
			}
		}
	}
	return false;

}


/* 得到扫描到的有效键值, 返回扫描到的数量 */
static u8 KeyGetValid(StKeyScan *pKey)
{
	if (((pKey->u32ScanCnt) & (KEY_SCAN_CNT - 1)) == (KEY_SCAN_CNT - 1))
	{
		s32 i, j;
		bool boHasKey = KeyCheckValue(pKey->stKeyTmp, &(pKey->stKeyNow));
		u8 *pNow, *pOld;
		StKeyState *pKeyState;
		u8 u8KeyCnt;
		if (!boHasKey)
		{
			return 0;
		}
		
		pNow = pKey->stKeyNow.u8KeyValue;
		pOld = pKey->stKeyOld.u8KeyValue;
		pKeyState = pKey->stKeyState;
		u8KeyCnt = 0;


		for (i = 0; i < KEY_Y_CNT; i++)
		{
			u8 u8NotSame = 0;
			u8 u8PressKeep = 0;
			
			u8NotSame =  pNow[i] ^ pOld[i];
			u8PressKeep = pNow[i] & pOld[i];

			if (u8NotSame != 0)
			{
				for (j = 0; j < KEY_X_CNT; j++)
				{
					if ((u8NotSame & (1 << j)) != 0)
					{
						if (u8KeyCnt >= KEY_MIX_MAX)
						{
							goto end;
						}
						pKeyState[u8KeyCnt].u8KeyLocation = ((i << 4) & 0xF0) | (j & 0x0F);
						/* <TODO:> 填写键值 */
						pKeyState[u8KeyCnt].u8KeyValue = g_u8KeyTable[i][j];
						if ((pNow[i] & (1 << j)) != 0)
						{
							pKeyState[u8KeyCnt].u8KeyState = KEY_DOWN;						
						}
						else
						{
							pKeyState[u8KeyCnt].u8KeyState = KEY_UP;
						}
						u8KeyCnt++;
					}
				}	
			}

			if (u8PressKeep != 0)
			{
				for (j = 0; j < KEY_X_CNT; j++)
				{
					if ((u8PressKeep & (1 << j)) != 0)
					{
						if (u8KeyCnt >= KEY_MIX_MAX)
						{
							goto end;
						}
						pKeyState[u8KeyCnt].u8KeyLocation = ((i << 4) & 0xF0) | (j & 0x0F);
						/* <TODO:> 填写键值 */
						pKeyState[u8KeyCnt].u8KeyValue = g_u8KeyTable[i][j];
						pKeyState[u8KeyCnt].u8KeyState = KEY_KEEP;						
						u8KeyCnt++;
					}
				}	
			}
		}
end:
		memcpy(pOld, pNow, KEY_Y_CNT);
		if(u8KeyCnt >= 4)
		{
			if (KeyClashCheck(pKey->stKeyState, u8KeyCnt))
			{
				return 0;
			}
		}
		return u8KeyCnt;
	}
	else
	{
		return 0;
	}
}

/* 按键 X 方向引脚初始化 */
static void KeyXPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	for (i = 0; i < KEY_X_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pKeyInPort[i], &GPIO_InitStructure);
	}
	

}
/* LED X 方向引脚初始化 */
static void LEDXPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	for (i = 0; i < LED_X_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16LedInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pLedInPort[i], &GPIO_InitStructure);
	}

}
/* LED 和 KEY 电源方向引脚初始化 */
static void PowerPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	for (i = 0; i < KEY_Y_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyPowerPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pKeyPowerPort[i], &GPIO_InitStructure);
	}
	for (i = 0; i < LED_Y_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16LedPowerPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pLedPowerPort[i], &GPIO_InitStructure);
	}

	GPIO_InitStructure.GPIO_Pin = LED_POWER_TRANSFORM_ENABLE_PIN;
	GPIO_Init(LED_POWER_TRANSFORM_ENABLE_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = LED_POWER_TRANSFORM_DIR_PIN;
	GPIO_Init(LED_POWER_TRANSFORM_DIR_PORT, &GPIO_InitStructure);

	GPIO_WriteBit(LED_POWER_TRANSFORM_DIR_PORT, 
					LED_POWER_TRANSFORM_DIR_PIN, Bit_SET);
	GPIO_WriteBit(LED_POWER_TRANSFORM_ENABLE_PORT, 
					LED_POWER_TRANSFORM_ENABLE_PIN, Bit_RESET);


}

/* 关闭所有LED */
static void LedCloseAll(void)
{
	/* 关闭所有LED引脚 */
	u32 i;
	for (i = 0; i < LED_X_CNT; i++)
	{
		GPIO_WriteBit((GPIO_TypeDef *)c_pLedInPort[i], 
			c_u16LedInPin[i], Bit_RESET);
	}

}
/* LED电源方向关闭 */
static void LedPowerCloseAll(void)
{
	u32 i;
	for (i = 0; i < LED_Y_CNT; i++)
	{
		GPIO_WriteBit((GPIO_TypeDef *)c_pLedPowerPort[i], 
			c_u16LedPowerPin[i], Bit_SET);
	}
}
/* LED 和 KEY 电源方向关闭 */
static void KeyPowerCloseAll(void)
{
	u32 i;
	for (i = 0; i < KEY_Y_CNT; i++)
	{
		GPIO_WriteBit((GPIO_TypeDef *)c_pKeyPowerPort[i], 
			c_u16KeyPowerPin[i], Bit_SET);
	}

}
/* LED 和 KEY 电源方向关闭 */
static void PowerCloseAll(void)
{
	/* 关闭所有Power引脚 */
	KeyPowerCloseAll();
	LedPowerCloseAll();
}
/* 恢复LED 的点亮 */
/* power为Y，LED 为X 方向 */
static void LedResume(StLedScan *pLenScan)
{
	u32 u32Cnt = pLenScan->u32ScanCnt % LED_Y_CNT;
	StLedSize stLedValue = pLenScan->stLedValue[u32Cnt];
	u32 i;
	LedCloseAll();
	LedPowerCloseAll();
	
	/* 点亮相应的列 */
	for (i = 0; i < LED_X_CNT; i++)
	{
		if ((stLedValue & (0x01 << i)) != 0)
		{
			GPIO_WriteBit((GPIO_TypeDef *)c_pLedInPort[i], 
				c_u16LedInPin[i], Bit_SET);
		}
	}
	GPIO_WriteBit((GPIO_TypeDef *)c_pLedPowerPort[u32Cnt], 
		c_u16LedPowerPin[u32Cnt], Bit_RESET);
	
	pLenScan->u32ScanCnt = u32Cnt + 1;
}

void KeyLedInit(void)
{
	KeyXPinInit();
	LEDXPinInit();
	PowerPinInit();

	PowerCloseAll();

	memset(&s_stKeyScan, 0, sizeof(StKeyScan));
	memset(&s_stLedScan, 0, sizeof(StLedScan));
	memset(&s_stLedBlinkState, 0, sizeof(StLedSize) * LED_Y_CNT);
}


void ChangeLedState(u32 x, u32 y, bool boIsLight)
{
	StLedSize *pLedValue;
	x %= LED_X_CNT;
	y %= LED_Y_CNT;
	pLedValue = &(s_stLedScan.stLedValue[y]);

	if (boIsLight)/* turn on */
	{
		*pLedValue |= (0x01 << x);
	}
	else
	{
		*pLedValue &= (~(0x01 << x));
	}
	
}

void ChangeLedBlinkState(u32 x, u32 y, bool boIsBlink)
{
	StLedSize *pLedValue;
	x %= LED_X_CNT;
	y %= LED_Y_CNT;
	pLedValue = s_stLedBlinkState + y;

	if (boIsBlink)/* turn on */
	{
		*pLedValue |= (0x01 << x);
	}
	else
	{
		*pLedValue &= (~(0x01 << x));
	}
}

static void FlushLedBlink(void)
{
	static u32 u32Time = 0;
	static bool boIsTurnOn = false;
	if (SysTimeDiff(u32Time, g_u32SysTickCnt) > 300)
	{
		StLedSize *pLedValue, stLedBlink;
		u32 i;
		for (i = 0; i < LED_Y_CNT; i++)
		{
			pLedValue = s_stLedScan.stLedValue + i;
			stLedBlink = s_stLedBlinkState[i];
			if (stLedBlink != 0)
			{
				if (boIsTurnOn)
				{
					*pLedValue |= stLedBlink;
				}
				else
				{
					*pLedValue &= (~stLedBlink);
				}
			}
			
		}
		u32Time = g_u32SysTickCnt;	
		boIsTurnOn = !boIsTurnOn;
	}
}

void ChangeAllLedState(bool boIsLight)
{
	u8 u8Value = 0;
	if (boIsLight)
	{
		u8Value = 0xFF;
	}
	memset(&s_stLedBlinkState, 0, sizeof(StLedSize) * LED_Y_CNT);
	memset(s_stLedScan.stLedValue, u8Value, sizeof(StLedSize) * LED_Y_CNT);
}
														
void KeyLedFlush(void)
{
	FlushLedBlink();
	LedResume(&s_stLedScan);
	if ((g_u32SysTickCnt & 0x01) == 0x00) /* even */	
	{
		u8 u8KeyCnt = 0;
		KeyScanOnce(&s_stKeyScan);
		u8KeyCnt = KeyGetValid(&s_stKeyScan);
		if (u8KeyCnt > 0)
		{
			StKeyMixIn *pBuf = KeyBufWriteBegin();
			if (pBuf == NULL)
			{
				return;
			}
			pBuf->u32Cnt = u8KeyCnt;
			pBuf->emKeyType = _Key_Board;
			memcpy(pBuf->unKeyMixIn.stKeyState, s_stKeyScan.stKeyState, 
				sizeof(StKeyState) * u8KeyCnt);
			KeyBufWriteEnd();
		}
	}
}





