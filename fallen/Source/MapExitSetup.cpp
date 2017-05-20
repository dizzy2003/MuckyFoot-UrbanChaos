//	MapExitSetup.cpp
//	Matthew Rosenfeld, 7th October 1998.

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


//---------------------------------------------------------------

CBYTE *filename;

//---------------------------------------------------------------

BOOL	CALLBACK	mapexit_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND		the_ctrl;
	SLONG		c0	=	0;


	switch(message)
	{
		case	WM_INITDIALOG:
			DlgDirListComboBox(hWnd,"levels\\*.ucm",IDC_COMBO1,0,DDL_ARCHIVE);
			the_ctrl = GetDlgItem(hWnd,IDC_COMBO1);
			c0=SendMessage(the_ctrl,CB_FINDSTRINGEXACT,-1,(long)filename);
			if (c0==CB_ERR)
				c0=SendMessage(the_ctrl,CB_ADDSTRING,0,(long)filename);
			if (c0!=CB_ERR) 
				SendMessage(the_ctrl,CB_SETCURSEL,c0,0);


			return	TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					the_ctrl = GetDlgItem(hWnd,IDC_COMBO1);
					SendMessage(the_ctrl,WM_GETTEXT,_MAX_PATH,(long)filename);
				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_mapexit_setup(EventPoint *the_ep)
{
	filename = (CBYTE*)the_ep->Data[0];
	if(!filename) {
		filename = (CBYTE*)malloc(_MAX_PATH);
		ZeroMemory(filename,_MAX_PATH);
	}
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_MAPEXIT_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)mapexit_proc
				);
	the_ep->Data[0] = (SLONG) filename;
}

//---------------------------------------------------------------

CBYTE	*get_mapexit_message(EventPoint *ep, CBYTE *msg) {
  if ((!ep)||(!ep->Data[0]))
	  strcpy(msg,"Unknown");
  else
	  strcpy(msg,(CBYTE*)ep->Data[0]);
  return msg;
}
