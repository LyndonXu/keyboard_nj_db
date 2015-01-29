/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：extern_io_ctrl.c
* 摘要: 外部普通IO控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2014年11月14日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "extern_io_ctrl.h"


GPIO_TypeDef * const c_pIOPort[IO_CNT] = 
{
	IO_PORT_1,
	IO_PORT_2,
	IO_PORT_3,
	IO_PORT_4,
	IO_PORT_5,
	IO_PORT_6,
	IO_PORT_7,
	IO_PORT_8,
	IO_PORT_9,
	IO_PORT_10,
	IO_PORT_11,
	IO_PORT_12,
	IO_PORT_13,
	IO_PORT_14,
	IO_PORT_15,
	IO_PORT_16,
};

const u16 c_u16IOPin[IO_CNT] = 
{
	IO_1,
	IO_2,
	IO_3,
	IO_4,
	IO_5,
	IO_6,
	IO_7,
	IO_8,
	IO_9,
	IO_10,
	IO_11,
	IO_12,
	IO_13,
	IO_14,
	IO_15,
	IO_16,
};

void ExternIOInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	for (i = 0; i < IO_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16IOPin[i];
		GPIO_Init(c_pIOPort[i], &GPIO_InitStructure);
		GPIO_WriteBit(c_pIOPort[i], c_u16IOPin[i], Bit_SET);
	}
}

void ExternIOCtrl(u8 u8Index, BitAction emAction)
{
	if (u8Index >= IO_CNT)
	{
		return;
	}
	GPIO_WriteBit(c_pIOPort[u8Index], c_u16IOPin[u8Index], emAction);
}


