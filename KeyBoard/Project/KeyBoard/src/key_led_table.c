#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "key_led.h"
#include "key_led_table.h"

u8 g_u8KeyTable[KEY_Y_CNT][KEY_X_CNT] = 
{
	{
		_Key_Record_Record, _Key_Record_Live, _Key_Record_Snap, _Key_Record_ChannelRecord,
		_Key_Delay_Broadcast_Live_Stop, _Key_Delay_Broadcast_Express,
		_Key_Delay_Broadcast_Safe, _Key_Caption,
	},	/* 1 */
	{
		_Key_VI_V1, _Key_VI_V2, _Key_VI_V3, _Key_VI_V4, 
		_Key_VI_1, _Key_VI_2, _Key_VI_3, _Key_VI_4, 
	},	/* 2 */
	{
		_Key_Angle1, _Key_Angle2, _Key_Time, _Key_DDR_1,
		_Key_DDR_Back, _Key_DDR_Play_Pause, _Key_DDR_Cycle, _Key_DDR_Auto,
	},	/* 3 */
	{
		_Key_VI_Auto, _Key_Switch_1, _Key_Switch_2, _Key_DDR_2,
		_Key_DDR_Front, _Key_DDR_Stop, _Key_DDR_Single, 0
	},	/* 4 */
	{
		_Key_PGM_1, _Key_PGM_2, _Key_PGM_3, _Key_PGM_4,
		_Key_PGM_VGA, _Key_PGM_NET, _Key_PGM_DDR1, _Key_PGM_DDR2,
	},	/* 5 */
	{
		_Key_PVW_1, _Key_PVW_2, _Key_PVW_3, _Key_PVW_4,
		_Key_PVW_VGA, _Key_PVW_NET, _Key_PVW_DDR1, _Key_PVW_DDR2,		
	},	/* 6 */
	{
		_Key_PGM_V1, _Key_PGM_V2, _Key_PGM_V3, _Key_PGM_V4,
		_Key_Trick_Fade_IN_OUT, _Key_Trick_Left_Erasure,
		_Key_Trick_Reel, _Key_Trick_PIP,
	},	/* 7 */
	{
		_Key_PVW_V1, _Key_PVW_V2, _Key_PVW_V3, _Key_PVW_V4,
		_Key_Trick_Shortcut_1, _Key_Trick_Shortcut_2,
		_Key_Trick_Shortcut_3, _Key_Trick_Shortcut_4,
				
	},	/* 8 */
	{
		_Key_Trick_Shortcut_5, _Key_Trick_Shortcut_6,
		_Key_Trick_Shortcut_7, _Key_Trick_Shortcut_8,
		_Key_Positoin_Caption, _Key_Positoin_Angle1,
		_Key_Positoin_Angle2, _Key_Positoin_Time,
					
	},	/* 9 */
	{
		_Key_Cutover_Cut, _Key_Cutover_Auto, _Key_Switch_3, _Key_Switch_4,
		_Key_Cutover_PVM, _Key_Cutover_PGW, _Key_Cutover_FTB, _Key_Cutover_VI,
	},	/* 10 */ 
	{
		_Key_Positoin_1, _Key_Positoin_2, _Key_Positoin_3, _Key_Positoin_4,
		_Key_Positoin_ZOOM, 
	},	/* 11 */
	{
		_Key_Positoin_VIN_1, _Key_Positoin_VIN_2, 
		_Key_Positoin_VIN_3, _Key_Positoin_VIN_4,
		_Key_Positoin_Aus,					
	},	/* 12 */
};

/* dp, g, f, e, d, c, b, a */
const u8 g_u8LED7Code[] = 
{
	0x3F,		// 0
	0x06,		// 1
	0x5B,		// 2
	0x4F,		// 3
	0x66,		// 4
	0x6D,		// 5
	0x7D,		// 6
	0x07,		// 7
	0x7F,		// 8
	0x6F,		// 9
	0x77,		// A
	0x7C,		// B
	0x39,		// C
	0x5E,		// D
	0x79,		// E
	0x71,		// F
	0x40,		// -
};
 

const u16 g_u16CamAddrLoc[CAM_ADDR_MAX] = 
{
	0,
};

