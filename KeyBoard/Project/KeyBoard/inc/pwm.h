/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����code_switch.h
* ժҪ: PWM����ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _PWM_H_
#define _PWM_H_
#define MCU_PWM1							GPIO_Pin_6
#define MCU_PWM2							GPIO_Pin_7
#define MCU_PWM_PORT 						GPIOA
#define MCU_PWM_TIMER 						TIM3

#define MCU_PWM1_CCR						MCU_PWM_TIMER->CCR1
#define MCU_PWM2_CCR						MCU_PWM_TIMER->CCR2

#define MCU_PWM1_OCInit						TIM_OC1Init
#define MCU_PWM2_OCInit						TIM_OC2Init

#define MCU_PWM1_OCPreloadConfig			TIM_OC1PreloadConfig
#define MCU_PWM2_OCPreloadConfig			TIM_OC2PreloadConfig


#define ENABLE_PWM_TIMER()		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)

#define PWM_RESOLUTION			256


void PWMCtrlInit(void);

void RedressLedOnLight(void);
void RedressLedOffLight(void);
extern u16 g_u16LedOnLight;
extern u16 g_u16LedOffLight;

#endif
