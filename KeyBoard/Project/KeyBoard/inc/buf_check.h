/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����buf_check.h
* ժҪ: Э���Զ�������ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _BUF_CHECK_H_
#define _BUF_CHECK_H_

typedef enum _EMCHECKTYPE_    /* ����ö�ٱ��� */
{
	_NOType,
	_PD,
	_PP,
	_YNA,
	/* �����м�����Э�� */
	_CHECK_TYPE_RESERVED,
}EmCheckType;


typedef struct _STBUFCHECK_
{
	u8 u8Buf[BUF_CHECK_SIZE];		/* ����洢������Ϣ������ */
	s32 s32WritePos;						/* ����д���Ķ��� */
	s32 s32CheckPos;						/* �����⵽�Ķ��� */
	EmCheckType emCheckType;
}StBufCheck;



void BufCheckInit(StBufCheck *pBuf);
int BufCheckSum(StBufCheck *pBuf, u8 u8Char, u8 *pOut, s32 *pSize);


#endif
