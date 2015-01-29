/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：flash_save.h
* 摘要: 操作flash相关程序头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#ifndef _FALSH_SAVE_H_
#define _FALSH_SAVE_H_

#define GET_UID_CNT(Byte)		(96 / (Byte * 8))
#define UID_BASE_ADDR			(0x1FFFF7E8)

#define DATA_SAVE_ADDR  (0x0800F000)
#define DATA_SAVE_HEAD  (0xA5A5)
#define UID_CHECK_ADDR			(0x0800F800)

typedef struct _tagStUID
{
	u16 u16UID[GET_UID_CNT(sizeof(u16))];
}StUID;

bool WriteUID(const StUID *pUID, u32 u32Srand);
bool CheckUID(const StUID *pUID);

void ReadSaveData(void);
bool WriteSaveData(void);

#endif
