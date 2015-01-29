/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����code_switch.h
* ժҪ: ���뿪��ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _CODE_SWITCH_H_
#define _CODE_SWITCH_H_

#define USING_SWITCH1
#define USING_SWITCH2
#define USING_SWITCH3
#define USING_SWITCH4

#define CODE_SWITCH1_PIN_A				GPIO_Pin_8
#define CODE_SWITCH1_PIN_A_PORT			GPIOC
#define CODE_SWITCH1_PIN_B				GPIO_Pin_9
#define CODE_SWITCH1_PIN_B_PORT			GPIOC


#define CODE_SWITCH1_INT_SRC			GPIO_PinSource8
#define CODE_SWITCH1_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH1_INT_LINE			EXTI_Line8
#define CODE_SWITCH1_INT_CHANNEL		EXTI9_5_IRQn
//#define CODE_SWITCH1_INT				EXTI15_10_IRQHandler


#define CODE_SWITCH2_PIN_A				GPIO_Pin_10
#define CODE_SWITCH2_PIN_A_PORT			GPIOC
#define CODE_SWITCH2_PIN_B				GPIO_Pin_13
#define CODE_SWITCH2_PIN_B_PORT			GPIOC

#define CODE_SWITCH2_INT_SRC			GPIO_PinSource10
#define CODE_SWITCH2_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH2_INT_LINE			EXTI_Line10
#define CODE_SWITCH2_INT_CHANNEL		EXTI15_10_IRQn
//#define CODE_SWITCH2_INT				EXTI2_IRQHandler

#define CODE_SWITCH3_PIN_A				GPIO_Pin_11
#define CODE_SWITCH3_PIN_A_PORT			GPIOC
#define CODE_SWITCH3_PIN_B				GPIO_Pin_14
#define CODE_SWITCH3_PIN_B_PORT			GPIOC

#define CODE_SWITCH3_INT_SRC			GPIO_PinSource11
#define CODE_SWITCH3_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH3_INT_LINE			EXTI_Line11
#define CODE_SWITCH3_INT_CHANNEL		EXTI15_10_IRQn


#define CODE_SWITCH4_PIN_A				GPIO_Pin_12
#define CODE_SWITCH4_PIN_A_PORT			GPIOC
#define CODE_SWITCH4_PIN_B				GPIO_Pin_15
#define CODE_SWITCH4_PIN_B_PORT			GPIOC

#define CODE_SWITCH4_INT_SRC			GPIO_PinSource12
#define CODE_SWITCH4_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH4_INT_LINE			EXTI_Line12
#define CODE_SWITCH4_INT_CHANNEL		EXTI15_10_IRQn




typedef struct _tagStCodeSwitchState
{
	u16 u16Index;
	
	u16 u16Cnt;
	u16 u16OldCnt;
}StCodeSwitchState;

void CodeSwitchInit(void);
u16 CodeSwitchPluse(u16 u16Index);

u16 CodeSwitchGetValue(u16 u16Index);
u16 CodeSwitchSetValue(u16 u16Index, u16 u16Value);
void CodeSwitchFlush(void);
#endif
