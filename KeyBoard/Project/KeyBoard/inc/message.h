/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
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
	u8 u8Buf[MSG_BUF_MAX * PROTOCOL_YNA_ENCODE_LENGTH];		/* ��󻺳� */
	StIOBuf	stIOBuf[MSG_BUF_MAX];						/* ��Ӧ�Ļ���ͷ */
	StIOBufHandle stIOBufHandle;						/* ������� */
}StMsgBuf;

void MessageUARTInit(void);
void MessageUARTConfig(USART_InitTypeDef *pConfig);

void MsgBufInit(void);
void *MsgInGetBuf(void);
void MsgInGetEnd(void);
void *MsgOutGetBuf(void);
void MsgOutGetEnd(void);


#endif
