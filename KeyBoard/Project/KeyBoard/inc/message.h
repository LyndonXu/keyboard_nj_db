/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message.h
* 摘要: 协议自动检测程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define MSG_RX_PIN 					GPIO_Pin_10
#define MSG_TX_PIN					GPIO_Pin_9
#define MSG_PORT					GPIOA
#define MSG_UART					USART1

#define ENABLE_MSG_UART()			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

#define MSG_UART_IRQ_CHANNEL		USART1_IRQn
#define MSG_UART_IRQ				USART1_IRQHandler


typedef struct _tagStMsgBuf
{
	u8 u8Buf[MSG_BUF_MAX * PROTOCOL_YNA_ENCODE_LENGTH];		/* 最大缓冲 */
	StIOBuf	stIOBuf[MSG_BUF_MAX];						/* 相应的缓冲头 */
	StIOBufHandle stIOBufHandle;						/* 缓冲控制 */
}StMsgBuf;

void MessageUARTInit(void);
void MessageUARTConfig(USART_InitTypeDef *pConfig);

void MsgBufInit(void);
void *MsgInGetBuf(void);
void MsgInGetEnd(void);
void *MsgOutGetBuf(void);
void MsgOutGetEnd(void);


#endif
