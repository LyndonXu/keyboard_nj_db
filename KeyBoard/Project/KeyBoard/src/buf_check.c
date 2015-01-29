/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����buf_check.c
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
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

static const u8 s_u8CheckTypeSize[] = {0, 7, 8, PROTOCOL_YNA_ENCODE_LENGTH};/* ������ӦЭ������ݳ��� */

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
 * ��ʼ����������Դ
 * ����: pBuf ָ��StBufCheck����
 * ���: ��
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
 * ��ȡУ������, �����emCheckType
 * ����: pBuf ָ��StBufCheck����
 * ���: ��
 */
static void  BufBeginCheck(StBufCheck *pBuf)
{
	s32 i;
	if (pBuf->s32CheckPos < pBuf->s32WritePos) /* ���У��λ��С��дλ�� */
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
 * ���У���, �ڲ�ʹ��
 * pBuf: [IN] ���
 * pOut: [OUT] ������ȷʱ����������λ��
 * pSize: [OUT]	������ȷʱ���õ������ݸ���
 * ����:
 * -1:У�����; 1:У����ȷ, ������������pOut; 0:����У��
 */
static s32 BufCheckSumInner(StBufCheck *pBuf, u8 *pOut, s32 *pSize)
{
	s32 s32Len = 0;
	/* ��ȡ��Ч���ݳ��� */
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
 * ���У���
 * pBuf: [IN] ���
 * u8Char: [IN] ��ҪУ�������
 * pOut: [OUT] ������ȷʱ����������λ��
 * pSize: [OUT]	������ȷʱ���õ������ݸ���
 * ����:
 * ����0������õ���ȷ��ʽ��ָ����򷵻�-1
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
		if (s32CheckRslt == -1) /* ������ */
		{
			pBuf->emCheckType = _NOType;
		}
		else if (s32CheckRslt == 1) /* �����ȷ */
		{
			pBuf->emCheckType = _NOType;
			return 0;
		}

	}
	return -1;
}


