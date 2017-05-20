//	GEdit.h
//	Guy Simmons, 27th July 1998.

#ifndef	GEDIT_H
#define	GEDIT_H

#include	"Mission.h"


//---------------------------------------------------------------

#define	UD_TCOMBO		(1<<0)
#define	UD_DEPENDENCY	(1<<1)
#define	UD_COLOUR		(1<<2)
#define	UD_GROUP		(1<<3)
#define	UD_TRIGGEREDBY	(1<<4)
#define	UD_OTCOMBO		(1<<5)
#define	UD_ALL			0xffff


#define	ED_KEYS			(								\
								Keys[KB_LEFT ]	||		\
								Keys[KB_RIGHT]	||		\
								Keys[KB_UP   ]	||		\
								Keys[KB_DOWN ]	||		\
								Keys[KB_HOME ]	||		\
								Keys[KB_END  ]	||		\
								Keys[KB_DEL  ]	||		\
								Keys[KB_PGDN ]	||		\
								Keys[KB_INS  ]	||		\
								Keys[KB_PGUP ]			\
						)

extern HCURSOR		GEDIT_arrow,
					GEDIT_busy;
extern HINSTANCE	GEDIT_hinstance;
extern HWND			GEDIT_client_wnd,
					GEDIT_edit_wnd,
					GEDIT_engine_wnd,
					GEDIT_frame_wnd,
					GEDIT_view_wnd,
					GEDIT_way_wnd,
					GEDIT_workspace_wnd;

int  			gedit(void);
void			controls_to_ep(EventPoint *ep,ULONG flags);
void			ep_to_controls(EventPoint *ep,ULONG flags);
void			ep_to_controls2(EventPoint *ep, SWORD tabpage=-1, HWND wnd=0);
void			controls_to_ep2(EventPoint *ep, SWORD tabpage=-1, HWND wnd=0);
void			menu_no_workspace(void);
void			menu_has_workspace(void);
void			menu_workspace_changed(void);
void			menu_workspace_saved(void);

//---------------------------------------------------------------

#endif
