/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：buf_check.c
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "buf_check.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#include "protocol.h"

static const u8 s_u8CheckTypeSize[] = {0, 7, 8, PROTOCOL_YNA_ENCODE_LENGTH};/* 定义相应协议的数据长度 */

#define SET_CHECK_TYPE() \
if (u8Char == 0xAA)\
{\
	pBuf->emCheckType = _YNA;\
	return;\
}\
else\
{\
	pBuf->s32CheckPos++;\
}

/*
 * 初始化检测相关资源
 * 输入: pBuf 指向StBufCheck类型
 * 输出: 无
 */
void BufCheckInit(StBufCheck *pBuf)
{
	if(pBuf == 0)
	{
		return;
	}
	memset(pBuf, 0, sizeof(StBufCheck));
}

/*
 * 获取校验类型, 会更改emCheckType
 * 输入: pBuf 指向StBufCheck类型
 * 输出: 无
 */
static void  BufBeginCheck(StBufCheck *pBuf)
{
	s32 i;
	if (pBuf->s32CheckPos < pBuf->s32WritePos) /* 如果校验位置小于写位置 */
	{
		for (i=pBuf->s32CheckPos; i<pBuf->s32WritePos; i++)
		{
			u8 u8Char = pBuf->u8Buf[i];
			SET_CHECK_TYPE();
		}
	}
	else
	{
		for (i = pBuf->s32CheckPos; i < BUF_CHECK_SIZE; i++)
		{
			u8 u8Char = pBuf->u8Buf[i];
			SET_CHECK_TYPE();
		}
		pBuf->s32CheckPos = 0;
		for (i = pBuf->s32CheckPos; i < pBuf->s32WritePos; i++)
		{
			u8 u8Char = pBuf->u8Buf[i];
			SET_CHECK_TYPE();
		}
	}
}
/*
 * 检测校验和, 内部使用
 * pBuf: [IN] 句柄
 * pOut: [OUT] 检验正确时，结果输出的位置
 * pSize: [OUT]	检验正确时，得到的数据个数
 * 返回:
 * -1:校验错误; 1:校验正确, 并将结果输出到pOut; 0:正在校验
 */
static s32 BufCheckSumInner(StBufCheck *pBuf, u8 *pOut, s32 *pSize)
{
	s32 s32Len = 0;
	/* 获取有效数据长度 */
	if (pBuf->s32CheckPos < pBuf->s32WritePos)
	{
		s32Len = pBuf->s32WritePos - pBuf->s32CheckPos;
	}
	else
	{
		s32Len = BUF_CHECK_SIZE - pBuf->s32CheckPos + pBuf->s32WritePos;
	}
#if 0
	if (pBuf->emCheckType == _PD)
	{
		if(s32Len < s_u8CheckTypeSize[_PD])
		{
			return 0;
		}
		else if(s32Len == s_u8CheckTypeSize[_PD])
		{
			s32 i;
			u8 u8Sum = 0;
			s32 s32Break = (pBuf->s32WritePos - 1) & (BUFCHECK_SIZE - 1);

			for(i=pBuf->s32CheckPos + 1; (i&(BUFCHECK_SIZE - 1)) != s32Break; i++)/* PD: 2~6 */
			{
				u8Sum += pBuf->u8Buf[i&(BUFCHECK_SIZE - 1)];
			}
			if(u8Sum == pBuf->u8Buf[s32Break])
			{
				s32 j=0;
				for(i=pBuf->s32CheckPos; (i&(BUFCHECK_SIZE - 1)) != (pBuf->s32WritePos & (BUFCHECK_SIZE - 1)); i++)
				{
					pOut[j++] = pBuf->u8Buf[i&(BUFCHECK_SIZE - 1)];
				}
				*pSize = j;
				pBuf->s32CheckPos = pBuf->s32WritePos;
				return 1;
			}
			pBuf->s32CheckPos++;
			if(pBuf->s32CheckPos >= BUFCHECK_SIZE)
			{
				pBuf->s32CheckPos = 0;
			}
			return -1;
		}
		else
		{
			return -1;
		}
	}
	else if(pBuf->emCheckType == _PP)
	{
		if(s32Len < s_u8CheckTypeSize[_PP])
		{
			return 0;
		}
		else if(s32Len == s_u8CheckTypeSize[_PP])
		{
			s32 i;
			u8 u8Sum;
			s32 s32Break = (pBuf->s32WritePos - 1) & (BUFCHECK_SIZE - 1);
			i=pBuf->s32CheckPos + 1;
			u8Sum = pBuf->u8Buf[i];
			i++;
			for(; (i&(BUFCHECK_SIZE - 1)) != s32Break; i++)/* PD: 2~6 */
			{
				u8Sum ^= pBuf->u8Buf[i&(BUFCHECK_SIZE - 1)];
			}
			if(u8Sum == pBuf->u8Buf[s32Break])
			{
				s32 j=0;
				for(i=pBuf->s32CheckPos; (i&(BUFCHECK_SIZE - 1)) != (pBuf->s32WritePos & (BUFCHECK_SIZE - 1)); i++)
				{
					pOut[j++] = pBuf->u8Buf[i&(BUFCHECK_SIZE - 1)];
				}
				if(pOut[7] != 0xAF)
				{
					goto err;
				}
				*pSize = j;
				pBuf->s32CheckPos = pBuf->s32WritePos;
				return 1;
			}
err:
			pBuf->s32CheckPos++;
			if(pBuf->s32CheckPos >= BUFCHECK_SIZE)
			{
				pBuf->s32CheckPos = 0;
			}
			return -1;
		}
	}
#endif
	if (pBuf->emCheckType == _YNA)
	{
		s32 s32ProtocolLen = PROTOCOL_YNA_DECODE_LENGTH;
		if (g_u32BoolIsEncode)
		{
			s32ProtocolLen = s_u8CheckTypeSize[_YNA];
		}
		if (s32Len < s32ProtocolLen)
		{
			return 0;
		}
		else if (s32Len == s32ProtocolLen)
		{
			s32 i;
			u8 u8Sum;
			s32 s32Break = (pBuf->s32WritePos - 1) & (BUF_CHECK_SIZE - 1);
			i = pBuf->s32CheckPos;
			u8Sum = pBuf->u8Buf[i];
			i++;
			for (; (i & (BUF_CHECK_SIZE - 1)) != s32Break; i++)/* YNA: 1~7 */
			{
				u8Sum ^= pBuf->u8Buf[i&(BUF_CHECK_SIZE - 1)];
			}
			if (u8Sum == pBuf->u8Buf[s32Break])
			{
				s32 j = 0;
				for ( i= pBuf->s32CheckPos; (i & (BUF_CHECK_SIZE - 1)) != (pBuf->s32WritePos & (BUF_CHECK_SIZE - 1)); i++)
				{
					pOut[j++] = pBuf->u8Buf[i&(BUF_CHECK_SIZE - 1)];
				}
				*pSize = j;
				pBuf->s32CheckPos = pBuf->s32WritePos;
				return 1;
			}
			pBuf->s32CheckPos++;
			if(pBuf->s32CheckPos >= BUF_CHECK_SIZE)
			{
				pBuf->s32CheckPos = 0;
			}
			return -1;
			
		}
		else
		{
			return -1;
		}
	}
	
	return 0;
}
/*
 * 检测校验和
 * pBuf: [IN] 句柄
 * u8Char: [IN] 需要校验的数据
 * pOut: [OUT] 检验正确时，结果输出的位置
 * pSize: [OUT]	检验正确时，得到的数据个数
 * 返回:
 * 返回0，如果得到正确格式的指令，否则返回-1
 */

s32 BufCheckSum(StBufCheck *pBuf, u8 u8Char, u8 *pOut, s32 *pSize)
{
	if(pBuf == NULL || pOut == NULL || pSize == NULL)
	{
		return -1;
	}
	pBuf->u8Buf[pBuf->s32WritePos++] = u8Char;
	if (pBuf->s32WritePos >= BUF_CHECK_SIZE)
	{
		pBuf->s32WritePos = 0;
	}
	if (pBuf->emCheckType == _NOType)		/**/
	{
		BufBeginCheck(pBuf);
	}
	else
	{
		int s32CheckRslt = BufCheckSumInner(pBuf, pOut, pSize);
		if (s32CheckRslt == -1) /* 检测错误 */
		{
			pBuf->emCheckType = _NOType;
		}
		else if (s32CheckRslt == 1) /* 检测正确 */
		{
			pBuf->emCheckType = _NOType;
			return 0;
		}

	}
	return -1;
}


