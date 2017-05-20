//	SfxSetup.cpp
//	Matthew Rosenfeld, 15th December 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"fmatrix.h"
#include	"inline.h"
#include	"gi.h"

#include	"EdStrings.h"
#include	"GEdit.h"
#include	"C:\fallen\headers\sound_id.h"
#include	"mmsystem.h"


//---------------------------------------------------------------

SLONG	sfx_type,sfx_id;

//---------------------------------------------------------------

#define	INIT_COMBO_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	1;												\
									lbitem_str	=	s[0];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,d,0);

CBYTE *sfxtypes[] = { "Sound FX", "Music", "!" };

void FillList(HWND hWnd, SLONG id, BOOL music) {
	SLONG i;
	BOOL is_music;
	HWND the_ctrl;

	i=0;
	the_ctrl=GetDlgItem(hWnd,id);
	SendMessage(the_ctrl,LB_RESETCONTENT,0,0);
	while (strcmp(sound_list[i],"!")) {
		is_music=strstr(sound_list[i],"music") ? 1 : 0;
		if (is_music==music) SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)sound_list[i]);
		i++;
	}
}

SLONG SelectToID(HWND hWnd) {
	CBYTE pc[_MAX_PATH];
	HWND the_ctrl=GetDlgItem(hWnd,IDC_LIST1);
	SLONG sel=SendMessage(the_ctrl,LB_GETCURSEL,0,0);
	SendMessage(the_ctrl,LB_GETTEXT,sel,(LPARAM)pc);
	SLONG i=0;
	while (strcmp(sound_list[i],"!")) {
		if (!strcmp(sound_list[i],pc)) return i;
		i++;
	}
	return 0;
	
}

void IDToSelect(HWND hWnd, SLONG id) {
	SendMessage(GetDlgItem(hWnd,IDC_LIST1),LB_SELECTSTRING,-1,(LPARAM)sound_list[id]);	
}


BOOL	CALLBACK	sfx_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	CBYTE		pc[_MAX_PATH],pc2[_MAX_PATH];
	SLONG		sel;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			INIT_COMBO_BOX(IDC_COMBO1,sfxtypes,sfx_type);

			FillList(hWnd, IDC_LIST1, sfx_type);
//			SendMessage(GetDlgItem(hWnd,IDC_LIST1),LB_SETCURSEL,sfx_id,0);
			IDToSelect(hWnd,sfx_id);

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_COMBO1:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						sel = SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
						FillList(hWnd,IDC_LIST1,sel);
					}
					break;
				case	IDC_BUTTON1:
					sel = SendMessage(the_ctrl=GetDlgItem(hWnd,IDC_LIST1),LB_GETCURSEL,0,0);
					SendMessage(the_ctrl,LB_GETTEXT,sel,(SLONG)pc2);
					strcpy(pc,"data\\sfx\\1622\\");
					strcat(pc,pc2);
//					PlaySound(pc,0,SND_FILENAME|SND_ASYNC);
					break;
				case	IDC_BUTTON2:
//					PlaySound(NULL,0,SND_PURGE);
					break;
				case	IDOK:
					sfx_type  = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0);
					sfx_id    = SelectToID(hWnd);

				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;

	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_sfx_setup(EventPoint *the_ep)
{
	//	Set the dialog.
	sfx_type		=	the_ep->Data[0];
	sfx_id   		=	the_ep->Data[1];

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_SEFFECT_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)sfx_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	sfx_type;
	the_ep->Data[1]		=	sfx_id;
}

//---------------------------------------------------------------
