AutoRun
-------

AutoRun is a simple CD front-end.  It supports internationalization (Western alphabets only), 2-level
menus and automatic detection of the installation of a product.  The executable is fully configurable
via text files and bitmaps.

Basic Behaviour
---------------

AutoRun must be in the root directory of the CD, and there must be an AUTORUN directory containing at
least one background bitmap and one menu file called English.txt.

When AutoRun is started (via Autorun.inf) it first finds the language name for the user's machine e.g
English, French, Italian etc.  It then looks in AUTORUN for a file called <language>.txt.  If this
file does not exist AutoRun loads English.txt as the default (if this is not found, AutoRun exits).

The file is then loaded and parsed.  This yields a set of Menu Definitions as well as global data -
the window title, the registry key for uninstallation and the registry key for the application.

If these registry keys exist, then the menu called POSTINSTALL is displayed, else the menu called
PREINSTALL is displayed.

Displaying a Menu
-----------------

Each menu can have its own background bitmap, font size, colours, etc.  The bitmap must be in the
AUTORUN subdirectory of the CD, along with the menu definition files.

The window is resized to fit the bitmap, and centred on the screen.  The menu strings are displayed
starting at specified top,left coordinates, and each line is a specified number of pixels below the
last.

When the user points to an item, it changes colour.  Clicking once invokes that item.

Registry Keys
-------------

Some standard registry keys are used:

HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall\<app name>

has value UninstallString which is the command line to run to uninstall the game.  The <app name> is
the normal name (with spaces) of the game e.g. "Urban Chaos"

HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\App Paths\<exe name>

has values (Default) which is the command line to run to execute the game, and Path which is the
directory to enter when running the game.  The <exe name> is the actual name of the executable
e.g. "fallen.exe"  Only Path is used by AutoRun.exe.

File Format
-----------

The file is a simple text file.  Fields are seperated by commas or newline.  Fields should be either
single words or numbers, or strings in quotes ("").  The first 4 lines of the file define the global
data:

"Urban Chaos CD-ROM"				This is the title displayed in the AutoRun window
"Urban Chaos"						This is the name of the registry key containing uninstall info
"fallen.exe"						This is the name of the registry key containing run info
4									The number of menus in the file

This header is followed by the menu data.  Each menu is independent.

Menu Data
---------

The menu data consists of a header, followed by a series of menu items.  The header format is as
follows:

PREINSTALL							The menu name - this can be anything but PREINSTALL and
									POSTINSTALL are the root menus
"background.bmp"					The name of the bitmap used as a background (in AUTORUN\)
"Arial",18,700						The font name, size and weight to be used for the menu
0,255,0								The RGB colour to be used for unselected items
255,255,0							The RGB colour to be used for selected items
22,103,31							Left border, top border and line seperation, in pixels

This header is followed by menu items.  Each item takes the form:

<text>,<verb>,<document>,<directory>,<spacing>

<text>		the menu text which will be printed on-screen.  You should use ... at the end of the
			line if the menu opens a submenu
<verb>		the verb to be executed (see below)
<document>	the document, executable or command-line
<directory>	the directory in which to execute the verb
<spacing>	the number of extra lines to leave after this line - if -1, this is the last menu item

Verbs
-----

If a verb starts with @ then after executing it, AutoRun quits.  You should use @ before running
the setup program, then the setup program should re-run AutoRun so it can check the registry
again.  @ alone can be used for an "exit" menu item

open		used in 90% of cases - opens the document, folder or executable specified
run			runs the specified command-line - this allows you to specify additional arguments
menu		opens the submenu specified - only one level of submenu depth is supported
back		returns to the top-level menu
uninstall	runs the uninstaller (using the command-line from the registry)

Macros
------

The <document> and <directory> are macro-expanded before being used.  The following macros are
replaced:

$SRC$	replaced with the source directory (containing AutoRun.exe) i.e. D: or similar
$DST$	replaced with the destination directory, if installed i.e. C:\Urban Chaos or similar

If <directory> is "" then it defaults to "$SRC$"

Examples
--------

"open","readme.htm",""						opens "<cd root>\readme.htm" in a web browser

"@open","setup.exe",""						runs "<cd root>\setup.exe" then quits

"menu","WEBLINKS",""						opens the WEBLINKS submenu

"open","$SRC$",""							opens <cd root> in explorer

"@","",""									exits the program

"open","http://www.muckyfoot.com"			opens http://www.muckyfoot.com in a web browser

"open","$SRC$\eidosnet\setup.exe","$SRC$\eidosnet"

											runs "<cd root>\eidosnet\setup.exe" in "<cd root>\eidosnet"

"open","$DST$\fallen.exe","$DST$"			runs fallen.exe in the installation directory
