/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：flash_save.c
* 摘要: 操作flash相关程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
#include "key_led.h"
#include "adc_ctrl.h"
#include "pwm.h"
#include "flash_save.h"

typedef struct _tagStSave
{
	u16 u16Head;
	u16 u16Times;
	u16 u16UpLimit;
	u16 u16DownLimit;
	u16 u16VolumeTimes;
	u16 u16VolumeUpLimit;
	u16 u16VolumeDownLimit;
	u16 u16LedOnLight;
	u16 u16LedOffLight;
	u16 u16CheckDum;
}StSave;

#define CHECK_SIZE		((sizeof(StSave) / sizeof(u16)) - 1)

void ReadSaveData(void)
{
	StSave stSave = {0};

	u16 u16CheckSum = 0, *pTmp = (u16 *)(&stSave);
	u32 i;
	
	memcpy(&stSave, (void *)DATA_SAVE_ADDR, sizeof(StSave));

	if (stSave.u16Head != DATA_SAVE_HEAD)
	{
		goto end;
	}

	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pTmp[i];
	}

	if (u16CheckSum == stSave.u16CheckDum)
	{	
		g_u16Times = stSave.u16Times;
		g_u16UpLimit = stSave.u16UpLimit;
		g_u16DownLimit = stSave.u16DownLimit;
		
		g_u16VolumeTimes = stSave.u16VolumeTimes;
		g_u16VolumeUpLimit = stSave.u16VolumeUpLimit;
		g_u16VolumeDownLimit = stSave.u16VolumeDownLimit;
		
		g_u16LedOnLight = stSave.u16LedOnLight;
		g_u16LedOffLight = stSave.u16LedOffLight;

		return;
	}
	
end:

	g_u16Times = PUSH_ROD_TIMES;
	g_u16UpLimit = PUSH_ROD_END;
	g_u16DownLimit = PUSH_ROD_BEGIN;
	
	g_u16VolumeTimes = VOLUME_TIMES;
	g_u16VolumeUpLimit = VOLUME_END;
	g_u16VolumeDownLimit = VOLUME_BEGIN;
	
	g_u16LedOnLight = 0;
	g_u16LedOffLight = PWM_RESOLUTION;
}
bool WriteSaveData(void)
{
	FLASH_Status FLASHStatus = FLASH_COMPLETE;

	StSave stSave = {0};

	u16 u16CheckSum = 0, *pData = (u16 *)(&stSave);
	u32 i, u32Addr = DATA_SAVE_ADDR;
	
	stSave.u16Head = DATA_SAVE_HEAD;
	
	stSave.u16Times = g_u16Times;
	stSave.u16UpLimit = g_u16UpLimit;
	stSave.u16DownLimit = g_u16DownLimit;
	
	stSave.u16VolumeTimes = g_u16VolumeTimes;
	stSave.u16VolumeUpLimit = g_u16VolumeUpLimit;
	stSave.u16VolumeDownLimit = g_u16VolumeDownLimit;
	stSave.u16LedOnLight = g_u16LedOnLight;
	stSave.u16LedOffLight = g_u16LedOffLight;

	
	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pData[i];
	}
	stSave.u16CheckDum = u16CheckSum;


	FLASH_Unlock();
		
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	
	FLASHStatus = FLASH_ErasePage(DATA_SAVE_ADDR);

	
	for (i = 0; i < (CHECK_SIZE + 1); i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(u32Addr, pData[i]);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
	}
	
	FLASH_Lock();
	return true;
}


s32 Compare(const void *pLeft, const void *pRight)
{
	return (*((u16 *)pLeft)) - (*((u16 *)pRight));
}

bool WriteUID(const StUID *pUID, u32 u32Srand)
{
	u32 i;
	u32 u32Addr = UID_CHECK_ADDR;
	u16 *pData;
	u16 u16ORNum;
	StUID stUIDRand = {0};
	StUID stUIDAddr = {0};
	
	u16 u16Cnt = 0;

	FLASH_Status FLASHStatus = FLASH_COMPLETE;
	
	
	srand(u32Srand);
	u16ORNum = rand();
	for (i = 0; i < GET_UID_CNT(sizeof(u16)); i++)
	{
		u32 j;
		stUIDRand.u16UID[i] = pUID->u16UID[i] ^ u16ORNum;
		while(1)
		{
			bool boIsGetAGoodAddr = true;
			stUIDAddr.u16UID[i] = rand() & 0x3FF;
			if (stUIDAddr.u16UID[i] <= (GET_UID_CNT(sizeof(u16)) + 1))
			{
				continue;
			}
			for (j = 0; j < i; j++)
			{
				if (stUIDAddr.u16UID[j] == stUIDAddr.u16UID[i])
				{
					boIsGetAGoodAddr = false;
					break;
				}
			}
			if (boIsGetAGoodAddr)
			{
				break;
			}
		}
		
	}
	pData = stUIDAddr.u16UID;
	qsort(pData, GET_UID_CNT(sizeof(u16)), sizeof(u16), Compare);

#if 1
	FLASH_Unlock();
		
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	
	FLASHStatus = FLASH_ErasePage(u32Addr);
	FLASHStatus = FLASH_ErasePage(u32Addr + 1024);

	FLASH_ProgramHalfWord(u32Addr, u16ORNum);
	u32Addr += sizeof(u16);
	
	for (i = 1; i < GET_UID_CNT(sizeof(u16)) + 1; i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(u32Addr, pData[i - 1]);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
		
	}
	
	for (; i < 1024; i++)
	{
		if (i == stUIDAddr.u16UID[u16Cnt])
		{
			FLASHStatus = FLASH_ProgramHalfWord(u32Addr, stUIDRand.u16UID[u16Cnt++]);
		}
		else
		{
			FLASHStatus = FLASH_ProgramHalfWord(u32Addr, rand());
		}
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
	}
	
	FLASH_Lock();
#endif
	return true;
}

bool CheckUID(const StUID *pUID)
{
	u32 i;
	u16 *pData;
	u16 u16ORNum;

	pData = (u16 *)UID_CHECK_ADDR;

	u16ORNum = *pData;

	for (i = 1; i < GET_UID_CNT(sizeof(u16)) + 1; i++)
	{
		u16 u16TmpUID;
		if (pData[i] > 0x3FF)
		{
			return false;
		}
		u16TmpUID = pData[pData[i]];
		u16TmpUID ^= u16ORNum;
		if (u16TmpUID != pUID->u16UID[i - 1])
		{
			return false;
		}
		
	}
	return true;
}
