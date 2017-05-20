// Guy Simmons
// 20th February 1997.

#include	"Editor.hpp"
// #include	"filereq.hpp"

//---------------------------------------------------------------

UBYTE InkeyToAscii[]=
{
	/*   0 - 9   */		0,		0,		'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',
	/*  10 - 19  */		'9',	'0',	'-',	'=',	'\b',	'\t',	'q',	'w',	'e',	'r',
	/*  20 - 29  */		't',	'y',	'u',	'i',	'o',	'p',	'[',	']',	0,		0,
	/*  30 - 39  */ 	'a', 	's',	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
	/*  40 - 49  */		'\'',	'`',	0,		'#',	'z',	'x',	'c',	'v',	'b',	'n',
	/*  50 - 59  */		'm',	',',	'.',	'/',	0,		'*',	0,		' ',	0,		0,
	/*  60 - 69  */		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
	/*  70 - 79  */		0,		0,		0,		0,		'-',	0,		0,		0,		'+',	0,
	/*  80 - 89  */		0,		0,		0,		0,		0,		0,		'\\',	0,		0,		0,
	/*  90 - 99  */		0,		0,		0,		0,		0,		0,		'/',	0,		0,		'(',
	/* 100 - 109 */		')',	'/',	'*',	0,		0,		0,		0,		0,		0,		0,
	/* 110 - 119 */		0,		0,		0,		'.',	0,		0,		0,		0,		0,		0,
	/* 120 - 127 */		0,		0,		0,		0,		0,		0,		0,		0
};

UBYTE InkeyToAsciiShift[]=
{
	/*   0 - 9   */		0,		0,		'!',	'"',	'œ',	'$',	'%',	'^',	'&',	'*',
	/*  10 - 19  */		'(',	')',	'_',	'+',	'\b',	'\t',	'Q',	'W',	'E',	'R',
	/*  20 - 29  */		'T',	'Y',	'U',	'I',	'O',	'P',	'{',	'}',	0,		0,
	/*  30 - 39  */ 	'A', 	'S',	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
	/*  40 - 49  */		'@',	'~',	0,		'~',	'Z',	'X',	'C',	'V',	'B',	'N',
	/*  50 - 59  */		'M',	'<',	'>',	'?',	0,		'*',	0,		' ',	0,		0,
	/*  60 - 69  */		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
	/*  70 - 79  */		0,		0,		0,		0,		'-',	0,		0,		0,		'+',	0,
	/*  80 - 89  */		0,		0,		0,		0,		0,		0,		'|',	0,		0,		0,
	/*  90 - 99  */		0,		0,		0,		0,		0,		0,		'/',	0,		0,		'(',
	/* 100 - 109 */		')',	'/',	'*',	0,		0,		0,		0,		0,		0,		0,
	/* 110 - 119 */		0,		0,		0,		'.',	0,		0,		0,		0,		0,		0,
	/* 120 - 127 */		0,		0,		0,		0,		0,		0,		0,		0
};


ControlDef	controls[]	=	
{
	{	V_SLIDER,		0,	"",		   				270,	40,			0,	255						},
	{	0	}

};

CBYTE	a_cunning_plan=0;

FileRequester::FileRequester(CBYTE	*path,CBYTE *extension,CBYTE *title,CBYTE *fname)
{
	SLONG	c0;
	SLONG	x=100,y=100;
	EdRect	ctrl_rect;

	Path=path;
	WildCard=extension;
	Title=title;
	strcpy(FileName,fname);
/*
	SetRect(0,0,300,400);

	OK.SetRect(20,370,40,15);
	Cancel.SetRect(220,370,40,15);
	TextEdit.SetRect(20,350,150,13);
	Controls.InitControlSet(controls);

	ctrl_rect.SetRect(10,101,300,400);
	Controls.ControlSetBounds(&ctrl_rect);

	for(c0=0;c0<25;c0++)
	{
		TextList[c0].SetRect(50,40+c0*12,200,12);
	}
*/
}

UBYTE	edit_and_draw_text(EdRect	*rect,CBYTE *str)
{
	CBYTE	str2[100];
	static	count=0;
	count++;

	strcpy(str2,str);
	if(count&4)
	{
		str2[strlen(str2)+1]=0;
		str2[strlen(str2)]='-';
	}

	rect->FillRect(CONTENT_COL);
	rect->HiliteRect(LOLITE_COL,LOLITE_COL);
	QuickTextC(rect->GetLeft()+2,rect->GetTop()+3,str2,0);

	if(LastKey==KB_BS||LastKey==KB_DEL)
	{
		LastKey=0;
		if(strlen(str))
			str[strlen(str)-1]=0;
	}
	if(LastKey==KB_ENTER)
	{
		LastKey=0;
		return(0);

	}
		if(strlen(str)<99)
			if(InkeyToAscii[LastKey])
			{
				str[strlen(str)+1]=0;
				str[strlen(str)]=InkeyToAscii[LastKey];
				LastKey=0;
			}

	return(1);
}

UBYTE	do_button(EdRect	*rect,CBYTE *str,MFPoint *local_point,UBYTE centered)
{
	SLONG	w;
	SLONG	ret=0;
	w=QTStringWidth(str);

	rect->FillRect(CONTENT_COL);
	if(rect->PointInRect(local_point))
	{
		rect->HiliteRect(LOLITE_COL,HILITE_COL);
		if(LeftMouse.ButtonState)
		{
			LeftMouse.ButtonState=0;
			ret=1;
		}
	}
	else
		rect->HiliteRect(HILITE_COL,LOLITE_COL);
	if(centered)
		QuickTextC(rect->GetLeft()+(rect->GetWidth()-w)/2,rect->GetTop()+3,str,0);
	else
		QuickTextC(rect->GetLeft()+2,rect->GetTop()+3,str,0);

	return(ret);
}


SLONG	FileRequester::Draw()
{
	CBYTE  curr_directory[_MAX_PATH];
	CBYTE	filter[100];
	OPENFILENAME ofn;
	SLONG	save=0,ret=0,c0;

	GetCurrentDirectory(_MAX_PATH, curr_directory);

	if(Title[1]=='a' && Title[2]=='v' && Title[3]=='e')
		save=1;


	sprintf(filter,"Editorfiles %s",WildCard);
	filter[strlen(filter)+2]=0;
	filter[11]=0;


	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hDDLibWindow;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = filter; //"Editor File\0*.ucm\0\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = FileName;
	ofn.nMaxFile          = 100;
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = Path;
	ofn.lpstrTitle        = Title;
	ofn.Flags             = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = &WildCard[2];
	ofn.lCustData         = NULL;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;


	if(save)
		ret=GetSaveFileName(&ofn);
	else
		ret=GetOpenFileName(&ofn);

	if (!ret)
	{
		SetCurrentDirectory(curr_directory);
		return FALSE;
	}
	else
	{
		SetCurrentDirectory(curr_directory);
	}

	for(c0=strlen(FileName);c0>0;c0--)
	{
		if(FileName[c0]=='\\')
		{
			SLONG	c1=0;

			while(FileName[c0+c1])
				FileName[c1++]=FileName[c0+c1+1];
			break;
		}
	}

	return(TRUE);


#ifdef	_MF_WINDOWS_DOG_POO
	SLONG	c0;
//	struct	TbFileFind	file;
	WIN32_FIND_DATA		FileData;
	HANDLE	Handle;
	MFPoint local_point;
	SLONG	exit=0;
	UBYTE	edit_text_flag=0;
	SLONG	skip=0;
	CBYTE	search_path[100];
	SLONG	temp_button_state;

//just draw this once
	SetWorkWindowBounds(100+1,100+1,GetWidth(),GetHeight());
	if(LockWorkScreen())
	{

		FillRect(CONTENT_COL);
		HiliteRect(HILITE_COL,LOLITE_COL);
		QuickText(100,10,Title,0);

//		DrawWindow();
//		Controls.DrawControlSet();

		UnlockWorkScreen();
	}

	while(SHELL_ACTIVE && !exit)
	{
		if(Keys[KB_DOWN])
		{
			Keys[KB_DOWN]=0;
			skip++;
		}
		if(Keys[KB_UP])
		{
			Keys[KB_UP]=0;
			skip--;
			if(skip<0)
			skip=0;
		}
			


		if(LockWorkScreen())
		{

			local_point.X	=MousePoint.X;
			local_point.Y	=MousePoint.Y;

			GlobalToLocal(&local_point);
			temp_button_state=LeftMouse.ButtonState;
//	Do OK & Cancel Buttons

			if(do_button(&OK,"OK",&local_point,1))
				exit=1;
			if(do_button(&Cancel,"Cancel",&local_point,1))
				exit=-1;

//  Do Editor Box 
			if(edit_text_flag)
			{
				edit_text_flag=edit_and_draw_text(&TextEdit,FileName);
			}
			else
				edit_text_flag=do_button(&TextEdit,FileName,&local_point,0);


			QuickTextC(50,25,Path,0);

			strcpy(search_path,Path);
			strcat(search_path,WildCard);
/*
			{
				SLONG	fcount;

				fcount=1;
				Handle = FindFirstFile(search_path, &FileData);
				if(Handle != INVALID_HANDLE_VALUE)
				{
					while(FindNextFile(Handle, &FileData))
						fcount++;
				}


			}
*/

			Handle = FindFirstFile(search_path, &FileData);

//  skip down the files
			if(Handle != INVALID_HANDLE_VALUE)
			if(skip)
			{
				c0=skip;
				while(c0)
				{
					if(Handle != INVALID_HANDLE_VALUE)
						FindNextFile(Handle, &FileData);
					c0--;		
				}
			}

//  draw file names and check for mouse click
			if(Handle != INVALID_HANDLE_VALUE)
			for(c0=0;c0<25;c0++)
			{
				SLONG	box_col,text_col=0;
				SLONG	inrect=0;

				box_col=WHITE_COL;

				if(FileData.cAlternateFileName[0]||FileData.cFileName[0])
				if(TextList[c0].PointInRect(&local_point))
				{
					inrect=1;
					box_col=0;
					text_col=WHITE_COL;
				}

				TextList[c0].FillRect(box_col);
				TextList[c0].HiliteRect(HILITE_COL,LOLITE_COL);


				if(FileData.cAlternateFileName[0])
				{ //use alternative file names
					QuickText(TextList[c0].GetLeft()+10,TextList[c0].GetTop()+2,FileData.cAlternateFileName,text_col);
					if(inrect&&LeftMouse.ButtonState)
					{
						LeftMouse.ButtonState=0;
						strcpy(FileName,FileData.cAlternateFileName);
					}
				}
				else
				{ //use DOS file names
					QuickText(TextList[c0].GetLeft()+20,TextList[c0].GetTop()+2,FileData.cFileName,text_col);
					if(inrect&&LeftMouse.ButtonState)
					{
						LeftMouse.ButtonState=0;
						strcpy(FileName,FileData.cFileName);
					}
				}
				
//get next filename
				if(Handle != INVALID_HANDLE_VALUE)
					if(!FindNextFile(Handle, &FileData))
					{
						FileData.cAlternateFileName[0]=0;
						FileData.cFileName[0]=0;
					}
			}
			if(Handle != INVALID_HANDLE_VALUE)
				FindClose(Handle);

			UnlockWorkScreen();

			if(temp_button_state==LeftMouse.ButtonState)
				LeftMouse.ButtonState=0;

			
		}
		ShowWorkWindow(0);
		if(Keys[KB_ESC])
			exit=1;
		

	}					  
	if(exit==1)
		return(1);
	else
#endif
		return(0);
}

//---------------------------------------------------------------


