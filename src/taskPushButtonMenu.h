#ifndef __TASKPUSHBUTTONMENU_H__
#define __TASKPUSHBUTTONMENU_H__

#define NOT_PUSHED			0
#define	SHORT_PUSH			1
#define LONG_PUSH			2

#define PUSHB_SHORT_MIN		1					// Min pushdown for valid push (x 10ms)
#define	PUSHB_LONG_MIN		50					// Min pushdown for Menu Mode (x 10ms)

#define MENU_BUTTON			ENCODER_SWITCH

//extern bool		MENU_mode;					// LCD Menu mode (declared in Mobo_config.h)


extern void vStartTaskPushButtonMenu(void);

#endif
