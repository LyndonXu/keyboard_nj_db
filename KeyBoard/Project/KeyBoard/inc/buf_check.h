/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：buf_check.h
* 摘要: 协议自动检测程序头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _BUF_CHECK_H_
#define _BUF_CHECK_H_

typedef enum _EMCHECKTYPE_    /* 定义枚举变量 */
{
	_NOType,
	_PD,
	_PP,
	_YNA,
	/* 在这中间增加协议 */
	_CHECK_TYPE_RESERVED,
}EmCheckType;


typedef struct _STBUFCHECK_
{
	u8 u8Buf[BUF_CHECK_SIZE];		/* 定义存储接收信息的数组 */
	s32 s32WritePos;						/* 缓存写到哪儿了 */
	s32 s32CheckPos;						/* 缓存检测到哪儿了 */
	EmCheckType emCheckType;
}StBufCheck;



void BufCheckInit(StBufCheck *pBuf);
int BufCheckSum(StBufCheck *pBuf, u8 u8Char, u8 *pOut, s32 *pSize);


#endif
