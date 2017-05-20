//	MessageSetup.cpp
//	Guy Simmons, 2nd September 1998.

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



//
// Returns TRUE if it thinks the given string is a place or street name.
//

SLONG is_street_name(CBYTE *str_in)
{
	SLONG  i;
	SLONG  num_spaces;
	CBYTE  str[512];
	CBYTE *ch;

	//
	// Create a local uppercase version.
	//

	for (i = 0; i < 512; i++)
	{
		str[i] = toupper(str_in[i]);

		if (str[i] == '\000')
		{
			break;
		}
	}

	//
	// How many spaces?
	//

	num_spaces = 0;

	for (ch = str; *ch; ch++)
	{
		if (*ch == ' ')
		{
			num_spaces += 1;
		}
	}

	if (num_spaces > 6)
	{
		//
		// Too many spaces...
		//

		return FALSE;
	}

	//
	// Words the string can't contain.
	// 

	CBYTE *dont_contain[] =
	{
		" HIM",
		" THEM",
		"WHO ",
		"WHEN ",
		"IF ",
		"THERE",
		"WHICH ",
		"LOOK ",
		" IT ",
		"YOU ",
		" GOT",
		"MY ",
		"HIS ",
		"HAVE ",
		"WILL ",
		"WOULD ",
		"MOST ",
		" BE",
		" US",
		"BUT ",
		"IS ",
		"MR ",
		"MRS ",
		"MISS ",
		" ME,",
		" GET ",
		" AN ",
		" A ",
		" OUR ",
		" SIR",
		" OR ",
		"!"
	};

	i = 0;

	while(1)
	{
		if (dont_contain[i][0] == '!')
		{
			break;
		}

		if (strstr(str, dont_contain[i]))
		{
			return FALSE;
		}

		i++;
	}

	//
	// Any string that ends in ST or RD or DRV
	//

	for (ch = str; *ch; ch++);

	while(1)
	{
		ch -= 1;

		if (ch == str)
		{
			goto no_suffix;
		}

		if (isalpha(*ch))
		{
			break;
		}

	}

	if (ch - str >= 5)
	{
		if (ch[-2] == ' ' &&
			ch[-1] == 'S' &&
			ch[ 0] == 'T')
		{
			//
			// Street...
			//

			return TRUE;
		}

		if (ch[-2] == ' ' &&
			ch[-1] == 'R' &&
			ch[ 0] == 'D')
		{
			//
			// Road...
			//

			return TRUE;
		}

		if (ch[-2] == ' ' &&
			ch[-1] == 'D' &&
			ch[ 0] == 'R')
		{
			//
			// Drive...
			//

			return TRUE;
		}


		if (ch[-3] == ' ' &&
			ch[-2] == 'D' &&
			ch[-1] == 'R' &&
			ch[ 0] == 'V')
		{
			//
			// Drive...
			//

			return TRUE;
		}

		if (ch[-2] == ' ' &&
			ch[-1] == 'B' &&
			ch[ 0] == 'V')
		{
			//
			// Boulevard...
			//

			return TRUE;
		}

		if (ch[-3] == ' ' &&
			ch[-2] == 'B' &&
			ch[-1] == 'V' &&
			ch[ 0] == 'D')
		{
			//
			// Boulevard...
			//

			return TRUE;
		}
	}

  no_suffix:;

	//
	// Words the string must contain.
	//

	CBYTE *must_contain[] =
	{
		"STREET",
		"DRV",
		"SQUARE",
		" AVE",
		"BLVD",
		"PARK",
		"PLAZA",
		"APARTMENT",
		"BEACH",
		"DRIVE",
		"METALWORKS",
		"GARDEN",
		"PROJECT",
		" ST.",
		" RD.",
		" AV.",
		"FACILITY",
		"BOATHOUSE",
		"HIGHWAY",
		"!"
	};

	i = 0;

	while(1)
	{
		if (must_contain[i][0] == '!')
		{
			break;
		}

		if (strstr(str, must_contain[i]))
		{
			return TRUE;
		}

		i++;
	}

	return FALSE;
}






//---------------------------------------------------------------

CBYTE		*message_text;
SLONG		 message_time, message_who;

extern CBYTE *WaypointExtra(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

#define STR_LEN 800

BOOL	CALLBACK	ms_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	SLONG len, ep, c0;
	HWND  the_ctrl;
	CBYTE str[STR_LEN], msg[STR_LEN];
	EventPoint	*ep_ptr, *ep_base=current_mission->EventPoints;


	switch(message)
	{
		case	WM_INITDIALOG:
			//	Set up the edit text.
			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							EM_SETLIMITTEXT,
							2550,0
						);

			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_SETTEXT,
							0,(LPARAM)message_text
						);

			// and the spinner
			SendMessage(
							GetDlgItem(hWnd,IDC_SPIN1),
							UDM_SETPOS,
							0, MAKELONG(message_time,0)
						);

			// and the said-by combo
			the_ctrl	=	GetDlgItem(hWnd,IDC_COMBO1);
			strcpy(str,"Radio");
			SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);
			SendMessage(the_ctrl,CB_SETCURSEL,0,0);

			strcpy(str,"Place/Street-name");
			SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);

			strcpy(str,"Tutorial help message");
			SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);

			ep	=	current_mission->UsedEPoints;
			c0  =   3;
			while (ep) {
				ep_ptr	=	TO_EVENTPOINT(ep_base,ep);
				if ((ep_ptr->WaypointType==WPT_CREATE_PLAYER)||(ep_ptr->WaypointType==WPT_CREATE_ENEMIES)) {
					WaypointExtra(ep_ptr,msg);
					sprintf(str,"%d%c: %s",ep,'A' + ep_ptr->Group,msg);
					SendMessage(the_ctrl,CB_ADDSTRING,0,(LPARAM)str);
					if (ep==message_who) SendMessage(the_ctrl,CB_SETCURSEL,c0,0);
					c0++;
				}
				ep =	ep_ptr->Next;
			}

			if (message_who == 0xffff)
			{
				SendMessage(the_ctrl,CB_SETCURSEL,1,0);
			}

			if (message_who == 0xfffe)
			{
				SendMessage(the_ctrl,CB_SETCURSEL,2,0);
			}

			SetFocus(GetDlgItem(hWnd,IDC_EDIT1));
			return	FALSE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;
			}
			break;

		case	WM_CLOSE:
			len=SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_GETTEXTLENGTH,
							0,0
						)+1;
			if (message_text) free(message_text);
			message_text =	(CBYTE*)malloc(len);
			ZeroMemory(message_text,len);
			
			SendMessage	(
							GetDlgItem(hWnd,IDC_EDIT1),
							WM_GETTEXT,
							len,
							(LPARAM)message_text
						);
			message_time = SendMessage( GetDlgItem(hWnd,IDC_SPIN1),UDM_GETPOS,0,0);

			message_who  =	SendMessage	(
											GetDlgItem(hWnd,IDC_COMBO1),
											CB_GETCURSEL,
											0,0
										);
			// now translate phoney people indices to real one
			if (message_who<3)
			{
				switch(message_who)
				{
					case 0: message_who = 0;      break;
					case 1: message_who = 0xffff; break;
					case 2: message_who = 0xfffe; break;

					default:
						ASSERT(0);
						break;
				}

			} else {
				memset(str,0,STR_LEN);
				SendMessage(GetDlgItem(hWnd,IDC_COMBO1),CB_GETLBTEXT,message_who,(long)str);
				sscanf(str,"%d",&message_who);
			}

			EndDialog(hWnd,0);
			return	TRUE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	do_message_setup(EventPoint *the_ep)
{

	//	Set the dialog.
	message_text	=	(CBYTE*)the_ep->Data[0];
	message_time	=	the_ep->Data[1];
	message_who 	=	the_ep->Data[2];
	if (!message_time) message_time=4;
	if(!message_text)
	{
		message_text	=	(CBYTE*)malloc(STR_LEN);
		ZeroMemory(message_text,STR_LEN);
		SetEPTextID(the_ep);
	}

	//	Do the dialog.
	DialogBox	(
					GEDIT_hinstance,
					MAKEINTRESOURCE(IDD_MESSAGE_SETUP),
					GEDIT_view_wnd,
					(DLGPROC)ms_proc
				);

	//	Get the data.
	the_ep->Data[0]		=	(SLONG)message_text;	//	
	the_ep->Data[1]		=	message_time;
	the_ep->Data[2]		=	message_who;
}

//---------------------------------------------------------------

/*
CBYTE	*get_message_message(EventPoint *ep, CBYTE *msg) {
	msg[0]=0;
	if (ep&&ep->Data[0])
		strcpy(msg,(CBYTE*)ep->Data[0]);
	return msg;
}
*/