/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：message.c
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
#include "app_port.h"
#include "io_buf_ctrl.h"
#include "buf_check.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#include "protocol.h"
#include "message.h"


static StMsgBuf s_stInBuf;
static StMsgBuf s_stOutBuf;

static StBufCheck s_stBufCheck;	

const u32 u32ArrUnknow[64];

/*
 * 外部消息串口初始化
 * 输入: 无
 * 输出: 无
 */
void MessageUARTInit(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	ENABLE_MSG_UART();

	USART_StructInit(&USART_InitStructure);
	USART_Init(MSG_UART, &USART_InitStructure);
	USART_ITConfig(MSG_UART, USART_IT_RXNE, DISABLE);
	USART_ITConfig(MSG_UART, USART_IT_TXE, DISABLE);
	USART_Cmd(MSG_UART, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = MSG_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(MSG_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MSG_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MSG_PORT, &GPIO_InitStructure);
	
#if MSG_EN_PIN
	GPIO_InitStructure.GPIO_Pin = MSG_RX_EN_PIN | MSG_TX_EN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(MSG_EN_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(MSG_EN_PORT, MSG_RX_EN_PIN);
    GPIO_SetBits(MSG_EN_PORT, MSG_TX_EN_PIN);
#endif
	NVIC_InitStructure.NVIC_IRQChannel = MSG_UART_IRQ_CHANNEL; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(MSG_UART, USART_IT_RXNE, ENABLE);
}

/*
 * 外部消息串口配置: 波特率, 停止位, 校验方式等
 * 输入: pConfig: USART_InitTypeDef的指针
 * 输出: 无
 */
void MessageUARTConfig(USART_InitTypeDef *pConfig)
{
	if(pConfig == NULL)
	{
		return;
	}
	pConfig->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	pConfig->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(MSG_UART, pConfig);
}

void MsgBufInit(void)
{
	memset(&s_stInBuf, 0, sizeof(StMsgBuf));
	IOBufInit(&(s_stInBuf.stIOBufHandle),
				s_stInBuf.stIOBuf,
				MSG_BUF_MAX,
				s_stInBuf.u8Buf,
				MSG_BUF_MAX * PROTOCOL_YNA_ENCODE_LENGTH);

	memset(&s_stOutBuf, 0, sizeof(StMsgBuf));
	IOBufInit(&(s_stOutBuf.stIOBufHandle),
				s_stOutBuf.stIOBuf,
				MSG_BUF_MAX,
				s_stOutBuf.u8Buf,
				MSG_BUF_MAX * PROTOCOL_YNA_ENCODE_LENGTH);
}

/* 主函数使用, 查询是否有事件, 协议所需要的一个包的指针 */
void *MsgInGetBuf(void)
{
	USE_CRITICAL();
	StIOBuf *pBufHeader;
	void *pReturn = NULL;

	if (s_stInBuf.stIOBufHandle.u32Used == 0)
	{
		return NULL;
	}

	ENTER_CRITICAL();
	pBufHeader = ReadBufBegin(&(s_stInBuf.stIOBufHandle));
	EXIT_CRITICAL();
	if (pBufHeader != NULL)
	{
		pReturn = pBufHeader->pBuf;
	}
	return pReturn;
}
/* 主函数使用, MsgInGetBuf返回值不为NULL的时候, 用此函数结束使用 */
void MsgInGetEnd(void)
{
	USE_CRITICAL();

	ENTER_CRITICAL();
	ReadBufEnd(&(s_stInBuf.stIOBufHandle));
	EXIT_CRITICAL();    
}

#define MSG_SEND_USE_INIT		0

/* 主函数使用, 发送数据, 协议所需要的一个包的指针 */
void *MsgOutGetBuf(void)
{
#if MSG_SEND_USE_INIT
	USE_CRITICAL();
	StIOBuf *pBufHeader;
	void *pReturn = NULL;

	if(s_stOutBuf.stIOBufHandle.u32Used >= s_stOutBuf.stIOBufHandle.u32IOBufCnt)
	{
		return NULL;
	}

	ENTER_CRITICAL();
	pBufHeader = WriteBufBegin(&(s_stOutBuf.stIOBufHandle));

	if (pBufHeader != NULL)
	{
		pReturn = pBufHeader->pBuf;
		if (g_u32BoolIsEncode)
		{
			pBufHeader->u32Used = PROTOCOL_YNA_LENGTH;
		}
		else
		{
			pBufHeader->u32Used = PROTOCOL_YNA_LENGTH;
		}
		
	}
	EXIT_CRITICAL();
#else
	StIOBuf *pBufHeader;
	void *pReturn = NULL;

	if(s_stOutBuf.stIOBufHandle.u32Used >= s_stOutBuf.stIOBufHandle.u32IOBufCnt)
	{
		return NULL;
	}

	pBufHeader = WriteBufBegin(&(s_stOutBuf.stIOBufHandle));

	if (pBufHeader != NULL)
	{
		pReturn = pBufHeader->pBuf;
		if (g_u32BoolIsEncode)
		{
			pBufHeader->u32Used = PROTOCOL_YNA_ENCODE_LENGTH;
		}
		else
		{
			pBufHeader->u32Used = PROTOCOL_YNA_DECODE_LENGTH;
		}
	}

#endif
	return pReturn;
}
/* 主函数使用, MsgOutGetBuf返回值不为NULL的时候, 用此函数结束使用 */
void MsgOutGetEnd(void)
{
#if MSG_SEND_USE_INIT
	USE_CRITICAL();

	ENTER_CRITICAL();
	WriteBufEnd(&(s_stOutBuf.stIOBufHandle));
	EXIT_CRITICAL(); 
	USART_ITConfig(MSG_UART, USART_IT_TXE, ENABLE);
#else
	StIOBuf *pBuf;
	WriteBufEnd(&(s_stOutBuf.stIOBufHandle));
	pBuf = ReadBufBegin(&(s_stOutBuf.stIOBufHandle));

	/* Loop until USARTy DR register is empty */ 
	while(USART_GetFlagStatus(MSG_UART, USART_FLAG_TXE) == RESET)
	{
	}
	while(1)
	{
		u8 u8Value;
		if(pBuf->u32Read >= pBuf->u32Used)
		{
			break;
		}
		u8Value = ((u8 *)(pBuf->pBuf))[pBuf->u32Read++];
		USART_SendData(MSG_UART, u8Value);

		/* Loop until USARTy DR register is empty */ 
		while(USART_GetFlagStatus(MSG_UART, USART_FLAG_TXE) == RESET)
		{
		}
	}	
	ReadBufEnd(&(s_stOutBuf.stIOBufHandle));
#endif
}

/**
  * @brief  This function handles MSG_UART Handler.
  * @param  None
  * @retval None
  */
void MSG_UART_IRQ(void)
{
#if 0
	if (USART_GetITStatus(MSG_UART, USART_IT_RXNE) != RESET)
	{
		/*  */
		u8 u8RxTmp = USART_ReceiveData(MSG_UART);
		StIOBuf *pBuf;
		s32 s32ReceiveSize;
		pBuf = WriteBufBegin(&(s_stInBuf.stIOBufHandle));
		
		if(pBuf == NULL)
		{
			goto check_send;
		}
		if(BufCheckSum(&s_stBufCheck, u8RxTmp, pBuf->pBuf, &s32ReceiveSize) != 0)
		{
			goto check_send;
		}	
		pBuf->u32Used = s32ReceiveSize;	
		WriteBufEnd(&(s_stInBuf.stIOBufHandle));

	}
#else
	if (USART_GetITStatus(MSG_UART, USART_IT_RXNE) != RESET)
	{
		/*  */
		u8 u8RxTmp = USART_ReceiveData(MSG_UART);
		static StIOBuf *pMsgRecvBuf = NULL;
		s32 s32ReceiveSize;
		if (pMsgRecvBuf == NULL)
		{
			pMsgRecvBuf = WriteBufBegin(&(s_stInBuf.stIOBufHandle));
		}
		if(pMsgRecvBuf == NULL)
		{
			goto check_send;
		}
		if(BufCheckSum(&s_stBufCheck, u8RxTmp, pMsgRecvBuf->pBuf, &s32ReceiveSize) != 0)
		{
			goto check_send;
		}	
		pMsgRecvBuf->u32Used = s32ReceiveSize;
		YNADecode(pMsgRecvBuf->pBuf);
		WriteBufEnd(&(s_stInBuf.stIOBufHandle));
		pMsgRecvBuf = NULL;

	}
#endif
check_send: 

#if MSG_SEND_USE_INIT
	if (USART_GetITStatus(MSG_UART, USART_IT_TXE) != RESET)
	{   
    	static StIOBuf *pMsgSendBuf = NULL;
		if (pMsgSendBuf == NULL)
		{
			pMsgSendBuf = ReadBufBegin(&(s_stOutBuf.stIOBufHandle));
		}
		if(pMsgSendBuf == NULL)
		{
			/* Disable the USARTy transmit interrupt */
			USART_ITConfig(MSG_UART, USART_IT_TXE, DISABLE);
			/* may be we need send a message to the task */
			return;
		}
		if(pMsgSendBuf->u32Read >= pMsgSendBuf->u32Used) /* we have finished send the data */
		{
			ReadBufEnd(&(s_stOutBuf.stIOBufHandle));
			
			pMsgSendBuf = NULL;
			
			pMsgSendBuf = ReadBufBegin(&(s_stOutBuf.stIOBufHandle));
			if(pMsgSendBuf == NULL)
			{
				/* Disable the MSG_UART transmit interrupt */
				USART_ITConfig(MSG_UART, USART_IT_TXE, DISABLE);
				return;
			}
			/* may be we need send a message to the task */
		}
	
		if (((u8 *)(pMsgSendBuf->pBuf))[0] == 0)
		{
			USART_SendData(MSG_UART, ((u8 *)(pMsgSendBuf->pBuf))[pMsgSendBuf->u32Read++]);
		}
		/* Write one byte to the transmit data register */
		USART_SendData(MSG_UART, ((u8 *)(pMsgSendBuf->pBuf))[pMsgSendBuf->u32Read++]);
	}   
#else
	if (USART_GetITStatus(MSG_UART, USART_IT_TXE) != RESET)
	{   
    	StIOBuf *pBuf = ReadBufBegin(&(s_stOutBuf.stIOBufHandle));
		if(pBuf == NULL)
		{
			/* Disable the USARTy transmit interrupt */
			USART_ITConfig(MSG_UART, USART_IT_TXE, DISABLE);
			/* may be we need send a message to the task */
			return;
		}
		if(pBuf->u32Read >= pBuf->u32Used) /* we have finished send the data */
		{
			ReadBufEnd(&(s_stOutBuf.stIOBufHandle));

			pBuf = ReadBufBegin(&(s_stOutBuf.stIOBufHandle));
			if(pBuf == NULL)
			{
				/* Disable the MSG_UART transmit interrupt */
				USART_ITConfig(MSG_UART, USART_IT_TXE, DISABLE);
				return;
			}
			/* may be we need send a message to the task */
		}
	
		if (((u8 *)(pBuf->pBuf))[0] == 0)
		{
			USART_SendData(MSG_UART, ((u8 *)(pBuf->pBuf))[pBuf->u32Read++]);
		}
		/* Write one byte to the transmit data register */
		USART_SendData(MSG_UART, ((u8 *)(pBuf->pBuf))[pBuf->u32Read++]);
	}

#endif
}

