#ifndef _TICKLIST_H_
#define _TICKLIST_H_

/*******************************************************************
 *
 *							Tick List
 *
 *	Use TICKLIST_INIT passing the appropriate IDC_LISTn and a
 *  !-terminated array of strings. Make sure the listbox is set to
 *  Owner Draw Fixed, Has Strings, Notify, Unsorted.
 *
 */

BOOL	CALLBACK	ticklist_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

SLONG				ticklist_bitmask(HWND hWnd, SLONG id);

void				ticklist_init(HWND hWnd, SLONG id, CBYTE *pc[], SLONG bitmask);
void				ticklist_close(HWND hWnd, SLONG id);

BOOL				ticklist_draw(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL				ticklist_measure(HWND hWnd, WPARAM wParam, LPARAM lParam);


/*
#define	TICKLIST_INIT(i,s,bitmask)									\
	{																\
	SLONG		c0			=	1;									\
	HWND		the_ctrl	=	GetDlgItem(hWnd,i);					\
	LPTSTR		lbitem_str	=	s[0];								\
	while(*lbitem_str!='!')											\
	{																\
		SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)lbitem_str);	\
		SendMessage(the_ctrl,LB_SETITEMDATA,c0-1,(bitmask & (1<<(c0-1))) ? 1 : 0);  \
		lbitem_str	=	s[c0++];									\
	}																\
	WNDPROC previous = (WNDPROC) SetWindowLong(the_ctrl,			\
		GWL_WNDPROC,(long)ticklist_proc);							\
	SetWindowLong(the_ctrl,GWL_USERDATA,(long)previous);			\
	}

#define TICKLIST_CLOSE(i)											\
	{																\
	HWND		the_ctrl	=	GetDlgItem(hWnd,i);					\
	WNDPROC previous = (WNDPROC) GetWindowLong(the_ctrl, GWL_USERDATA); \
	SetWindowLong(the_ctrl,GWL_WNDPROC,(long)previous);				\
	}
	*/

#endif