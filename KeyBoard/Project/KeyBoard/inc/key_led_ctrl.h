/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：key_led_ctrl.h
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
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
	StKeyState stKeyState[KEY_MIX_MAX];		/* 扫描的按键的状态 */

	u32 u32PushRodValue;					/* 推杆的值 */
	u32 u32VolumeValue;						/* 推杆的值 */
	StRockState stRockState;				/* 摇杆的状态 */

	StCodeSwitchState stCodeSwitchState;	/* 摇杆的状态 */

}UnKeyMixIn;

typedef struct _tagStKeyMixIn
{
	EmKeyType emKeyType;					/* 标示union域是什么内容 */
	u32 u32Cnt;								/* 仅在按键的时候有用 */
	UnKeyMixIn unKeyMixIn;					/* 混合值 */
}StKeyMixIn;

typedef struct _tagStKeyBuf
{
	StKeyMixIn stKeyMixIn[KEY_BUF_MAX];		/* 最大缓冲 */

	StIOBuf	stIOBuf[KEY_BUF_MAX];			/* 相应的缓冲头 */
	StIOBufHandle stIOBufHandle;			/* 缓冲控制 */
}StKeyBuf;


void KeyBufInit(void);
void *KeyBufGetBuf(void);
void KeyBufGetEnd(void);
void *KeyBufWriteBegin(void);
void KeyBufWriteEnd(void);

#endif
