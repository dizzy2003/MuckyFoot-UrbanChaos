//
// blah blah inputbox blah
//
// it's just useful
//

#include "inputbox.h"
#include "resource.h"

//---------------------------------------------------------------

extern HINSTANCE	GEDIT_hinstance;
extern HWND			GEDIT_view_wnd;

//---------------------------------------------------------------

CBYTE *the_title, *the_txt, *the_prompt;


//---------------------------------------------------------------

BOOL	CALLBACK	ib_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the edit text.
			SendMessage ( hWnd, WM_SETTEXT, 0, (LPARAM)the_title);
			SendMessage ( GetDlgItem(hWnd,IDC_STATIC_PROMPT), WM_SETTEXT, 0, (LPARAM)the_prompt);
			SendMessage	( GetDlgItem(hWnd,IDC_EDIT1),  EM_SETLIMITTEXT, 255, 0);
			SendMessage	( GetDlgItem(hWnd,IDC_EDIT1),
							WM_SETTEXT,
							0,(LPARAM)the_txt
						);

			SetFocus(GetDlgItem(hWnd,IDC_EDIT1));
			return	FALSE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage	(   GetDlgItem(hWnd,IDC_EDIT1),
									WM_GETTEXT,
									_MAX_PATH,
									(LPARAM)the_txt
								);
					EndDialog(hWnd,IDOK);
					return TRUE;
				case	IDCANCEL:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
			EndDialog(hWnd,IDCANCEL);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------


CBYTE *InputBox(CBYTE *title, CBYTE *prompt, CBYTE *txt) {
	SLONG res;

	//	Set the dialog.
	the_title=title;
	the_prompt=prompt;
	the_txt = txt;
	if (!the_txt) {
		the_txt = (CBYTE*)malloc(_MAX_PATH);
		ZeroMemory(the_txt,_MAX_PATH);
	}

	//	Do the dialog.
	res=DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_INPUT_BOX),
					GEDIT_view_wnd,
					(DLGPROC)ib_proc
				);
	if (res==IDCANCEL) {
		if (!txt) free((void*)the_txt);
		return txt;
	}

	//	Get the data.
	return the_txt;

}
