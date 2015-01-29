/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����key_led_ctrl.h
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _KEY_LED_CTRL_H_
#define _KEY_LED_CTRL_H_


typedef enum _tagEmKeyType
{
	_Key_Push_Rod = 0x00,
	_Key_Board,
	_Key_Rock,
	_Key_CodeSwitch,
	_Key_Volume,
	
	_Key_Reserved,
}EmKeyType;


typedef union _tagUnKeyMixIn
{
	StKeyState stKeyState[KEY_MIX_MAX];		/* ɨ��İ�����״̬ */

	u32 u32PushRodValue;					/* �Ƹ˵�ֵ */
	u32 u32VolumeValue;						/* �Ƹ˵�ֵ */
	StRockState stRockState;				/* ҡ�˵�״̬ */

	StCodeSwitchState stCodeSwitchState;	/* ҡ�˵�״̬ */

}UnKeyMixIn;

typedef struct _tagStKeyMixIn
{
	EmKeyType emKeyType;					/* ��ʾunion����ʲô���� */
	u32 u32Cnt;								/* ���ڰ�����ʱ������ */
	UnKeyMixIn unKeyMixIn;					/* ���ֵ */
}StKeyMixIn;

typedef struct _tagStKeyBuf
{
	StKeyMixIn stKeyMixIn[KEY_BUF_MAX];		/* ��󻺳� */

	StIOBuf	stIOBuf[KEY_BUF_MAX];			/* ��Ӧ�Ļ���ͷ */
	StIOBufHandle stIOBufHandle;			/* ������� */
}StKeyBuf;


void KeyBufInit(void);
void *KeyBufGetBuf(void);
void KeyBufGetEnd(void);
void *KeyBufWriteBegin(void);
void KeyBufWriteEnd(void);

#endif
