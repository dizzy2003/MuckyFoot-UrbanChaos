// Director.cpp
//
// director object

#include "AutoRun.h"

static TCHAR*	ReadString(FILE* fd);
static ULONG	ReadNumber(FILE* fd);
static COLORREF	ReadRGB(FILE* fd);

// Menu
//
// construct from file
//
// syntax:
//
// PREINSTALL				menu name - PREINSTALL and POSTINSTALL must exist
// "menubackground.bmp"		bitmap name
// "Arial",11,1				font name,size,bold flag
// 0,255,0					RGB colour for normal text
// 255,255,0				RGB colour for selected text
// 16,80,20					left border, top border, line seperation, in pixels
// "Install Urban Chaos","open","setup.exe","",1	for each item <string>,<verb>,<document>,<directory>,<line seperation>
// ,-1 for line seperation is the menu terminator
//
// see readme.txt for details of command format

Menu::Menu(FILE* fd)
{
	strcpy(Name, ReadString(fd));

	strcpy(Bitmap, ReadString(fd));

	strcpy(FontName, ReadString(fd));
	FontSize = ReadNumber(fd);
	FontWeight = ReadNumber(fd);

	ColourNormal = ReadRGB(fd);
	ColourSelected = ReadRGB(fd);

	LeftBorder = ReadNumber(fd);
	TopBorder = ReadNumber(fd);
	LineSpacing = ReadNumber(fd);

	MenuItem*	last = NULL;

	for (;;)
	{
		MenuItem*	item = new MenuItem;

		strcpy(item->Text, ReadString(fd));

		strcpy(item->Verb, ReadString(fd));
		strcpy(item->Document, ReadString(fd));
		strcpy(item->Directory, ReadString(fd));

		item->Spacing = ReadNumber(fd);
		item->Next = NULL;

		if (last)	last->Next = item;
		else		Item = item;

		last = item;

		if (item->Spacing & 0x80000000)	break;
	}

	Next = NULL;
}

// ~Menu
//
// destructor

Menu::~Menu()
{
	while (Item)
	{
		MenuItem*	item = Item;
		Item = item->Next;

		delete item;
	}
}

// Director(fd)
//
// construct from file
//
// syntax:
//
// "Urban Chaos CD-ROM"		window title
// "Urban Chaos"			app name in Uninstall part of registry
// "fallen.exe"				program name in App Paths part of registry
// 2						number of menus
//
// [menus]

Director::Director(FILE* fd)
{
	strcpy(WindowTitle, ReadString(fd));
	strcpy(AppName, ReadString(fd));
	strcpy(ExeName, ReadString(fd));

	ULONG	menus = ReadNumber(fd);

	Menu*	last = NULL;

	for (ULONG ii = 0; ii < menus; ii++)
	{
		Menu*	menu = new Menu(fd);

		if (last)	last->Next = menu;
		else		MenuList = menu;

		last = menu;
	}

	PreInstall = FindMenu("PREINSTALL");
	PostInstall = FindMenu("POSTINSTALL");

	LoadRegistryStrings();
}

// ~Director
//
// destructor

Director::~Director()
{
	while (MenuList)
	{
		Menu*	menu = MenuList;
		MenuList = menu->Next;
		delete menu;
	}
}

// LoadRegistryStrings
//
// load strings from the registry

void Director::LoadRegistryStrings()
{
	strcpy(UninstallString, "");
	strcpy(AppPath, "");

	HKEY	hCurrentVersion;

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hCurrentVersion))
	{
		HKEY	hUninstall;

		if (!RegOpenKeyEx(hCurrentVersion, "Uninstall", 0, KEY_READ, &hUninstall))
		{
			HKEY	hOurUninstall;

			if (!RegOpenKeyEx(hUninstall, AppName, 0, KEY_READ, &hOurUninstall))
			{
				DWORD	len = MAX_PATH;

				RegQueryValueEx(hOurUninstall, "UninstallString", NULL, NULL, (unsigned char*)UninstallString, &len);

				RegCloseKey(hOurUninstall);
			}

			RegCloseKey(hUninstall);
		}

		HKEY	hAppPaths;

		if (!RegOpenKeyEx(hCurrentVersion, "App Paths", 0, KEY_READ, &hAppPaths))
		{
			HKEY	hOurAppPath;

			if (!RegOpenKeyEx(hAppPaths, ExeName, 0, KEY_READ, &hOurAppPath))
			{
				DWORD	len = MAX_PATH;

				RegQueryValueEx(hOurAppPath, "Path", NULL, NULL, (unsigned char*)AppPath, &len);
				if ((len >= 2) && (AppPath[len - 2] == '\\'))	AppPath[len - 2] = '\0';

				RegCloseKey(hOurAppPath);
			}

			RegCloseKey(hAppPaths);
		}
		
		RegCloseKey(hCurrentVersion);
	}

	

	if (UninstallString[0] && AppPath[0])	Active = PostInstall;
	else									Active = PreInstall;
}

// FindMenu
//
// find a named menu

Menu* Director::FindMenu(TCHAR* name)
{
	Menu*	menu = MenuList;

	while (menu)
	{
		if (!stricmp(menu->Name, name))		break;
		menu = menu->Next;
	}

	return menu;
}

//
// file reading utilities
//

static TCHAR szBuffer[MAX_PATH];

// ReadString
//
// read to the next terminator (, or newline)
// skips "" so e.g. "Eddie, Matt","Mike, Mark" gives two strings
// \" is accepted as a quoted "
//
// note that no overflow checks are made - don't put huge strings into the definition files, please!

static TCHAR* ReadString(FILE* fd)
{
	TCHAR*	wptr = szBuffer;
	bool	quoted = false;
	bool	wasquoted = false;

	for (;;)
	{
		int	ch = fgetc(fd);

		if ((ch == '\n') || (ch == EOF) || ((ch == ',') && !quoted))
		{
			if ((wptr != szBuffer) || (ch == EOF))
			{
				*wptr = '\0';
				return szBuffer;
			}
			if ((wptr == szBuffer) && wasquoted)
			{
				*wptr = '\0';
				return szBuffer;
			}
			// otherwise continue
		}
		else if (ch == '\"')
		{
			quoted = !quoted;
			wasquoted = true;
		}
		else if (ch == ' ')
		{
			if (quoted)	*wptr++ = ' ';
		}
		else
		{
			*wptr++ = ch;
		}
	}
}

// ReadNumber
//
// read a number

static ULONG ReadNumber(FILE* fd)
{
	return ULONG(atoi(ReadString(fd)));
}

// ReadRGB
//
// read an RGB value

static COLORREF ReadRGB(FILE* fd)
{
	int	r = ReadNumber(fd);
	int	g = ReadNumber(fd);
	int	b = ReadNumber(fd);

	return RGB(r,g,b);
}
