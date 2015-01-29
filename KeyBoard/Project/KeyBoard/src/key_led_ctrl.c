/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：key_led_ctrl.c
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

/* 主函数使用, 查询是否有事件, 有返回 StKeyMixIn 类型指针(需转换) */
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
/* 主函数使用, KeyBufGetBuf返回值不为NULL的时候, 用此函数结束使用 */
void KeyBufGetEnd(void)
{
	USE_CRITICAL();

	ENTER_CRITICAL();
	ReadBufEnd(&(s_stKeyBuf.stIOBufHandle));
	EXIT_CRITICAL();    
}

/* 中断使用, 得到一些缓存, 将键值, 推杆值, 摇杆值, 写入 
 * 因为只有一个中断使用此函数, 所以不需要进行临界保护
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
/* 中断使用, KeyBufWriteBegin返回值不为NULL的时候, 用此函数结束使用*/
void KeyBufWriteEnd(void)
{
    WriteBufEnd(&(s_stKeyBuf.stIOBufHandle));
}



