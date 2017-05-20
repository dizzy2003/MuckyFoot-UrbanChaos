# Microsoft Developer Studio Project File - Name="psxlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=psxlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "psxlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "psxlib.mak" CFG="psxlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "psxlib - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "psxlib - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Fallen/psxlib", OUBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "psxlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "psxlib__"
# PROP BASE Intermediate_Dir "psxlib__"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "psxlib__"
# PROP Intermediate_Dir "psxlib__"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "psxlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "psxlib_0"
# PROP BASE Intermediate_Dir "psxlib_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "psxlib_0"
# PROP Intermediate_Dir "psxlib_0"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "EDITOR" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "psxlib - Win32 Release"
# Name "psxlib - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=..\Psxeng\Source\feng.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GHost.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GMaths.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GMem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\malloc.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\MFx.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\myheap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sample.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\vsprintf.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ".hpp"
# Begin Source File

SOURCE=.\Headers\GDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Ghost.h
# End Source File
# Begin Source File

SOURCE=.\Headers\GMaths.h
# End Source File
# Begin Source File

SOURCE=.\Headers\GMem.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Headers\myheap.h
# End Source File
# End Target
# End Project
