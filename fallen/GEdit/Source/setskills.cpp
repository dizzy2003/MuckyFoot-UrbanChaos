//	SetSkills.cpp
//	Matthew Rosenfeld, 22nd February 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"inline.h"
#include	"gi.h"

#include	"EdStrings.h"
#include	"GEdit.h"


UBYTE skills_list[255];

#define	INIT_COMBO_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	2;												\
									lbitem_str	=	s[1];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,CB_SETCURSEL,d,0);
#define	INIT_LIST_BOX(i,s,d)		the_ctrl	=	GetDlgItem(hWnd,i);								\
									c0			=	2;												\
									lbitem_str	=	s[1];											\
									while(*lbitem_str!='!')											\
									{																\
										SendMessage(the_ctrl,LB_ADDSTRING,0,(LPARAM)lbitem_str);	\
										lbitem_str	=	s[c0++];									\
									}																\
									SendMessage(the_ctrl,LB_SETCURSEL,d,0);


BOOL	CALLBACK	skills_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG		c0	=	0;
	HWND		the_ctrl;
	LPTSTR		lbitem_str;
	SLONG		sel;

	
	switch(message)
	{
		case	WM_INITDIALOG:
			INIT_COMBO_BOX(IDC_COMBO1,wenemy_ai_strings,0);
			INIT_LIST_BOX(IDC_LIST1,wenemy_ability_strings,0);

			PostMessage(hWnd,CBN_SELCHANGE,0,(LONG)GetDlgItem(hWnd,IDC_COMBO1));

			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDC_COMBO1:
					if (HIWORD(wParam)==CBN_SELCHANGE) {
						sel = SendMessage((HWND)lParam,CB_GETCURSEL,0,0)+1;
						SendMessage(GetDlgItem(hWnd,IDC_LIST1),LB_SETCURSEL,skills_list[sel]-1,0);
//						FillList(hWnd,IDC_LIST1,sel);
					}
					break;
				case	IDC_LIST1:
					if (HIWORD(wParam)==LBN_SELCHANGE) {
						sel = SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETCURSEL,0,0)+1;
						skills_list[sel] = SendMessage((HWND)lParam,LB_GETCURSEL,0,0)+1;

					}
					break;
				case	IDOK:
					EndDialog(hWnd,1);
					return  TRUE;
				case	IDCANCEL:
					EndDialog(hWnd,0);
					return	TRUE;
			}
			break;

	}
	return	FALSE;

}



void SetSkills(UBYTE skills[254]) {
	int res;

	memcpy(skills_list,skills,sizeof(skills_list));

	//	Do the dialog.
	res = DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_SET_SKILLS),
					GEDIT_view_wnd,
					(DLGPROC)skills_proc
				);

	if (res==1) memcpy(skills,skills_list,sizeof(skills));

}