# Microsoft Developer Studio Project File - Name="outro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=outro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "outro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "outro.mak" CFG="outro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "outro - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "outro - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Fallen/outro", RRCAAAAA"
# PROP Scc_LocalPath "."
# PROP WCE_FormatVersion ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "outro - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp1 /W2 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "outro - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /W2 /Gm /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "outro - Win32 Release"
# Name "outro - Win32 Debug"
# Begin Source File

SOURCE=.\Always.h
# End Source File
# Begin Source File

SOURCE=.\back.cpp
# End Source File
# Begin Source File

SOURCE=.\back.h
# End Source File
# Begin Source File

SOURCE=.\cam.cpp
# End Source File
# Begin Source File

SOURCE=.\cam.h
# End Source File
# Begin Source File

SOURCE=.\checker.cpp
# End Source File
# Begin Source File

SOURCE=.\checker.h
# End Source File
# Begin Source File

SOURCE=.\credits.cpp
# End Source File
# Begin Source File

SOURCE=.\credits.h
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\imp.cpp
# End Source File
# Begin Source File

SOURCE=.\imp.h
# End Source File
# Begin Source File

SOURCE=.\Key.h
# End Source File
# Begin Source File

SOURCE=.\lmap.cpp
# End Source File
# Begin Source File

SOURCE=.\lmap.h
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\Matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Matrix.h
# End Source File
# Begin Source File

SOURCE=.\mf.cpp
# End Source File
# Begin Source File

SOURCE=.\mf.h
# End Source File
# Begin Source File

SOURCE=.\midasdll.h
# End Source File
# Begin Source File

SOURCE=.\os.cpp
# End Source File
# Begin Source File

SOURCE=.\os.h
# End Source File
# Begin Source File

SOURCE=.\slap.cpp
# End Source File
# Begin Source File

SOURCE=.\slap.h
# End Source File
# Begin Source File

SOURCE=.\Tga.cpp
# End Source File
# Begin Source File

SOURCE=.\Tga.h
# End Source File
# Begin Source File

SOURCE=.\wire.cpp
# End Source File
# Begin Source File

SOURCE=.\wire.h
# End Source File
# End Target
# End Project
