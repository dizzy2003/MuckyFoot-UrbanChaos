# Microsoft Developer Studio Project File - Name="MFLib1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=MFLib1 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MFLib1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MFLib1.mak" CFG="MFLib1 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MFLib1 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "MFLib1 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""$/MFLib1", NABAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "MFLib1 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Source\C\Windows\MRelease"
# PROP Intermediate_Dir ".\Source\C\Windows\MRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_RELEASE" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Libs\M_win1_r.lib"

!ELSEIF  "$(CFG)" == "MFLib1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Source\C\Windows\MDebug"
# PROP Intermediate_Dir ".\Source\C\Windows\MDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Libs\M_win1_d.lib"

!ENDIF 

# Begin Target

# Name "MFLib1 - Win32 Release"
# Name "MFLib1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\Source\C\Windows\D3D.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DModes.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\Draw2d.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DrawBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DrawLine.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DrawPnt.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DrawPxl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\DrawRect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\File.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\Keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\Maths.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\Mem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\MFHost.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\Mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Windows\Palette.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\QuickTxt.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\Sprites.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\TextDef.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\C\Common\Trig.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\Headers\Display.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Draw2d.h
# End Source File
# Begin Source File

SOURCE=.\Headers\DrawPoly.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Keyboard.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFD3D.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFErrors.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFFile.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFHeader.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFHost.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFLbType.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFMaths.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFMem.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFStd.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFTypes.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MFUtils.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Mouse.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Palette.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Sprites.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
