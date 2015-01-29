/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：extern_io_ctrl.h
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _EXTERN_IO_CTRL_H_
#define _EXTERN_IO_CTRL_H_

#define IO_1				GPIO_Pin_12
#define IO_2				GPIO_Pin_13
#define IO_3				GPIO_Pin_14
#define IO_4				GPIO_Pin_15
#define IO_5				GPIO_Pin_8
#define IO_6				GPIO_Pin_9
#define IO_7				GPIO_Pin_10
#define IO_8				GPIO_Pin_11
#define IO_9				GPIO_Pin_12
#define IO_10				GPIO_Pin_13
#define IO_11				GPIO_Pin_14
#define IO_12				GPIO_Pin_15
#define IO_13				GPIO_Pin_6
#define IO_14				GPIO_Pin_7
#define IO_15				GPIO_Pin_8
#define IO_16				GPIO_Pin_9


#define IO_PORT_1		GPIOB
#define IO_PORT_2		GPIOB
#define IO_PORT_3		GPIOB
#define IO_PORT_4		GPIOB
#define IO_PORT_5		GPIOD
#define IO_PORT_6		GPIOD
#define IO_PORT_7		GPIOD
#define IO_PORT_8		GPIOD
#define IO_PORT_9		GPIOD
#define IO_PORT_10		GPIOD
#define IO_PORT_11		GPIOD
#define IO_PORT_12		GPIOD
#define IO_PORT_13		GPIOC
#define IO_PORT_14		GPIOC
#define IO_PORT_15		GPIOC
#define IO_PORT_16		GPIOC

#define IO_CNT			16

void ExternIOInit(void);
void ExternIOCtrl(u8 u8Index, BitAction emAction);
#endif
