/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����key_led_ctrl.c
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
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

static StKeyBuf s_stKeyBuf;

void KeyBufInit(void)
{
	memset(&s_stKeyBuf, 0, sizeof(StKeyBuf));
	IOBufInit(&(s_stKeyBuf.stIOBufHandle),
				s_stKeyBuf.stIOBuf,
				KEY_BUF_MAX,
				s_stKeyBuf.stKeyMixIn,
				sizeof(StKeyMixIn) * KEY_BUF_MAX);
}

/* ������ʹ��, ��ѯ�Ƿ����¼�, �з��� StKeyMixIn ����ָ��(��ת��) */
void *KeyBufGetBuf(void)
{
	USE_CRITICAL();
	StIOBuf *pBufHeader;
	void *pReturn = NULL;

	if (s_stKeyBuf.stIOBufHandle.u32Used == 0)
	{
		return NULL;
	}

	ENTER_CRITICAL();
	pBufHeader = ReadBufBegin(&(s_stKeyBuf.stIOBufHandle));
	EXIT_CRITICAL();

	if (pBufHeader != NULL)
	{
		pReturn = pBufHeader->pBuf;
	}
	
	return pReturn;
}
/* ������ʹ��, KeyBufGetBuf����ֵ��ΪNULL��ʱ��, �ô˺�������ʹ�� */
void KeyBufGetEnd(void)
{
	USE_CRITICAL();

	ENTER_CRITICAL();
	ReadBufEnd(&(s_stKeyBuf.stIOBufHandle));
	EXIT_CRITICAL();    
}

/* �ж�ʹ��, �õ�һЩ����, ����ֵ, �Ƹ�ֵ, ҡ��ֵ, д�� 
 * ��Ϊֻ��һ���ж�ʹ�ô˺���, ���Բ���Ҫ�����ٽ籣��
 */
void *KeyBufWriteBegin(void)
{
	StIOBuf *pBufHeader;
	void *pReturn = NULL;	
	pBufHeader = WriteBufBegin(&(s_stKeyBuf.stIOBufHandle));
	if (pBufHeader != NULL)
	{
		pReturn = pBufHeader->pBuf;
	}
	return pReturn;
}
/* �ж�ʹ��, KeyBufWriteBegin����ֵ��ΪNULL��ʱ��, �ô˺�������ʹ��*/
void KeyBufWriteEnd(void)
{
    WriteBufEnd(&(s_stKeyBuf.stIOBufHandle));
}



