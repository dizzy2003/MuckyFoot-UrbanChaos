# Microsoft Developer Studio Project File - Name="GEdit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GEdit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GEdit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GEdit.mak" CFG="GEdit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GEdit - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GEdit - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Fallen/GEdit", OHBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GEdit - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /GX /O2 /I "c:\fallen\ddlibrary\headers" /I "c:\fallen\headers" /I "c:\fallen\gedit\headers" /I "c:\fallen\ledit\headers" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x809
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W3 /GX /ZI /Od /I "c:\fallen\ddlibrary\headers" /I "c:\fallen\headers" /I "c:\fallen\gedit\headers" /I "c:\fallen\ledit\headers" /D "_WINDOWS" /D "EDITOR" /D "BUILD_PSX" /D "WIN32" /D "_DEBUG" /FR /YX /FD /c
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x809
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "GEdit - Win32 Release"
# Name "GEdit - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\activatesetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\animSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\barrelSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\bombSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\BonusSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\burnsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\CameraSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\CamTargetSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\converse.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\countersetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\CreatureSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\cutscene.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\dlightSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\EdStrings.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\enemyflagsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\EnemySetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\EngWind.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\ExtendSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\GEdit.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\inputbox.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ItemSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\locksetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\MapExitSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\MapView.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\MessageSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\Mission.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\MoveSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\NavSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\PeeSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\platformSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\PlayerSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\propedit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ResetSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\setskills.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\SfxSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\signsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\spotfxSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\stallsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\SubClass.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\TabCtl.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ticklist.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\TransferSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\TrapSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\treasuresetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\TriggerSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\VehicleSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\VfxSetup.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\warefxsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\waypointSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\WayWind.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Source\wptSetup.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\WSpace.cpp

!IF  "$(CFG)" == "GEdit - Win32 Release"

!ELSEIF  "$(CFG)" == "GEdit - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Headers\CameraSetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\CamTargetSetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\EdStrings.h
# End Source File
# Begin Source File

SOURCE=.\Headers\EnemySetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\EngWind.h
# End Source File
# Begin Source File

SOURCE=.\Headers\GEdit.h
# End Source File
# Begin Source File

SOURCE=.\Headers\ItemSetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\MapView.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Mission.h
# End Source File
# Begin Source File

SOURCE=.\Headers\PlayerSetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\SubClass.h
# End Source File
# Begin Source File

SOURCE=.\Headers\TabCtl.h
# End Source File
# Begin Source File

SOURCE=.\Headers\TriggerSetup.h
# End Source File
# Begin Source File

SOURCE=.\Headers\WayWind.h
# End Source File
# Begin Source File

SOURCE=.\Headers\WSpace.h
# End Source File
# End Group
# End Target
# End Project
