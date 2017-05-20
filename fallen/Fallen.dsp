# Microsoft Developer Studio Project File - Name="Fallen" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Fallen - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Fallen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Fallen.mak" CFG="Fallen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fallen - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Fallen - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Fallen", QDAAAAAA"
# PROP Scc_LocalPath "."
# PROP WCE_FormatVersion ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fallen - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MLd /W3 /GR /GX /O2 /Ob2 /I "Headers" /I "c:\fallen\ledit\headers" /I "c:\fallen\sedit\headers" /I "c:\fallen\DDLibrary\Headers" /I "c:\fallen\gedit\headers" /I "c:\fallen\ddengine\headers" /D "NDEBUG" /D "_RELEASE" /D "WIN32" /D "_WINDOWS" /D "FINAL" /Fr /YX"Game.h" /FD /D /NODEFAULTLIB:libcmtd.lib" " /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib amstrmid.lib quartz.lib strmbase.lib binkw32.lib mss32.lib ddraw.lib dinput.lib dxguid.lib /nologo /subsystem:windows /machine:I386 /libpath:"c:\fallen\bink" /libpath:"c:\miles"
# SUBTRACT LINK32 /profile /debug

!ELSEIF  "$(CFG)" == "Fallen - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /Zp1 /W2 /GR /GX /Zi /Od /I "c:\fallen\gedit\headers" /I "Headers" /I "c:\fallen\ledit\headers" /I "c:\fallen\sedit\headers" /I "c:\fallen\DDLibrary\Headers" /I "c:\fallen\ddengine\headers" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "TEST_DC" /YX"Game.h" /FD /D /NODEFAULTLIB:libcmtd.lib" " /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dplayx.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib amstrmid.lib quartz.lib strmbase.lib binkw32.lib mss32.lib ddraw.lib dinput.lib dxguid.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"c:\fallen\bink" /libpath:"c:\miles" /libpath:"c:\fallen\miles" /libpath:"c:\mflib1\libs" /libpath:"C:\qsound\QM411SDK\qmdx"
# SUBTRACT LINK32 /profile /incremental:no /map

!ENDIF 

# Begin Target

# Name "Fallen - Win32 Release"
# Name "Fallen - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Source\Anim.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\animal.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\animtmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Attract.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\balloon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\barrel.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\bat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\bike.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\build2.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\building.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\canid.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\chopper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\cnet.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\collide.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Controls.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Cop.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Darci.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\dike.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\dirt.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\door.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\drawtype.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\drip.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Effect.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\elev.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Enemy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Env.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\env2.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\eway.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\fc.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\fire.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\FMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\fog.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\frontend.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\gamemenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\glitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\grenade.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\guns.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\heap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Hierarchy.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\hm.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\hook.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\inside2.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\interact.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\interfac.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\io.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Map.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\maths.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\mav.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\memory.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\mist.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\morph.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\music.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\night.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\nightpsx.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ns.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ob.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\overlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\pap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\pause.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\pcom.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Person.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Pjectile.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\plat.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\playcuts.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\pow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Prim.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\psystem.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\puddle.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\pyro.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\qedit.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\qmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ribbon.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\road.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Roper.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\save.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\shadow.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\sm.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\snipe.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\sound_id.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\soundenv.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\spark.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Special.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\stair.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\startscr.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\State.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\supermap.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Switch.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Thing.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\Thug.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\tracks.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\trip.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\vehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\walkable.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\wand.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\ware.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\widget.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\wmove.cpp
# End Source File
# Begin Source File

SOURCE=.\Source\xlat_str.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Headers\america.h
# End Source File
# Begin Source File

SOURCE=.\Editor\Headers\Anim.h
# End Source File
# Begin Source File

SOURCE=.\Headers\animal.h
# End Source File
# Begin Source File

SOURCE=.\Headers\animate.h
# End Source File
# Begin Source File

SOURCE=.\Headers\animtmap.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Attract.h
# End Source File
# Begin Source File

SOURCE=.\Headers\az.h
# End Source File
# Begin Source File

SOURCE=.\Headers\balloon.h
# End Source File
# Begin Source File

SOURCE=.\Headers\bang.h
# End Source File
# Begin Source File

SOURCE=.\Headers\barrel.h
# End Source File
# Begin Source File

SOURCE=.\Headers\bat.h
# End Source File
# Begin Source File

SOURCE=.\Headers\bike.h
# End Source File
# Begin Source File

SOURCE=.\Headers\building.h
# End Source File
# Begin Source File

SOURCE=.\Headers\cache.h
# End Source File
# Begin Source File

SOURCE=.\Headers\cam.h
# End Source File
# Begin Source File

SOURCE=.\Headers\canid.h
# End Source File
# Begin Source File

SOURCE=.\Headers\chopper.h
# End Source File
# Begin Source File

SOURCE=.\Headers\cloth.h
# End Source File
# Begin Source File

SOURCE=.\Headers\cnet.h
# End Source File
# Begin Source File

SOURCE=.\Headers\collide.h
# End Source File
# Begin Source File

SOURCE=.\Headers\combat.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Command.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Controls.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Cop.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Darci.h
# End Source File
# Begin Source File

SOURCE=.\Headers\demo.h
# End Source File
# Begin Source File

SOURCE=.\Headers\dirt.h
# End Source File
# Begin Source File

SOURCE=.\Headers\drawtype.h
# End Source File
# Begin Source File

SOURCE=.\Headers\drip.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Editor.h
# End Source File
# Begin Source File

SOURCE=.\Headers\elev.h
# End Source File
# Begin Source File

SOURCE=.\Headers\eway.h
# End Source File
# Begin Source File

SOURCE=.\Headers\fc.h
# End Source File
# Begin Source File

SOURCE=.\Headers\fire.h
# End Source File
# Begin Source File

SOURCE=.\Headers\FMatrix.h
# End Source File
# Begin Source File

SOURCE=.\Headers\fog.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Furn.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Game.h
# End Source File
# Begin Source File

SOURCE=.\Headers\glitter.h
# End Source File
# Begin Source File

SOURCE=.\Headers\guns.h
# End Source File
# Begin Source File

SOURCE=.\Headers\heap.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Hierarchy.h
# End Source File
# Begin Source File

SOURCE=.\Headers\hm.h
# End Source File
# Begin Source File

SOURCE=.\Headers\hook.h
# End Source File
# Begin Source File

SOURCE=.\Headers\id.h
# End Source File
# Begin Source File

SOURCE=.\Headers\inside2.h
# End Source File
# Begin Source File

SOURCE=.\Headers\interact.h
# End Source File
# Begin Source File

SOURCE=.\Headers\interfac.h
# End Source File
# Begin Source File

SOURCE=.\Headers\io.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Level.h
# End Source File
# Begin Source File

SOURCE=.\Headers\light.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Map.h
# End Source File
# Begin Source File

SOURCE=.\Headers\maths.h
# End Source File
# Begin Source File

SOURCE=.\Headers\mav.h
# End Source File
# Begin Source File

SOURCE=.\Headers\memory.h
# End Source File
# Begin Source File

SOURCE=.\Headers\mist.h
# End Source File
# Begin Source File

SOURCE=.\Headers\morph.h
# End Source File
# Begin Source File

SOURCE=.\Headers\music.h
# End Source File
# Begin Source File

SOURCE=.\Headers\night.h
# End Source File
# Begin Source File

SOURCE=.\Headers\noserver.h
# End Source File
# Begin Source File

SOURCE=.\Headers\ns.h
# End Source File
# Begin Source File

SOURCE=.\Headers\ob.h
# End Source File
# Begin Source File

SOURCE=.\Headers\overlay.h
# End Source File
# Begin Source File

SOURCE=.\Headers\pap.h
# End Source File
# Begin Source File

SOURCE=.\Headers\pause.h
# End Source File
# Begin Source File

SOURCE=.\Headers\pcom.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Person.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Pjectile.h
# End Source File
# Begin Source File

SOURCE=.\Headers\plat.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Player.h
# End Source File
# Begin Source File

SOURCE=.\Headers\pq.h
# End Source File
# Begin Source File

SOURCE=.\Editor\Headers\Prim.h
# End Source File
# Begin Source File

SOURCE=.\Headers\PSystem.h
# End Source File
# Begin Source File

SOURCE=.\Headers\puddle.h
# End Source File
# Begin Source File

SOURCE=.\Headers\pyro.h
# End Source File
# Begin Source File

SOURCE=.\Headers\qedit.h
# End Source File
# Begin Source File

SOURCE=.\Headers\qmap.h
# End Source File
# Begin Source File

SOURCE=.\Headers\road.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Roper.h
# End Source File
# Begin Source File

SOURCE=.\Headers\sample.h
# End Source File
# Begin Source File

SOURCE=.\Headers\save.h
# End Source File
# Begin Source File

SOURCE=.\Headers\sewer.h
# End Source File
# Begin Source File

SOURCE=.\Headers\shadow.h
# End Source File
# Begin Source File

SOURCE=.\Headers\sm.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Sound.h
# End Source File
# Begin Source File

SOURCE=.\Headers\sound_id.h
# End Source File
# Begin Source File

SOURCE=.\Headers\spark.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Special.h
# End Source File
# Begin Source File

SOURCE=.\Headers\stair.h
# End Source File
# Begin Source File

SOURCE=.\Headers\startscr.h
# End Source File
# Begin Source File

SOURCE=.\Headers\State.h
# End Source File
# Begin Source File

SOURCE=.\Headers\statedef.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Structs.h
# End Source File
# Begin Source File

SOURCE=.\Headers\supermap.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Switch.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Thing.h
# End Source File
# Begin Source File

SOURCE=.\Headers\Thug.h
# End Source File
# Begin Source File

SOURCE=.\Headers\tracks.h
# End Source File
# Begin Source File

SOURCE=.\Headers\trip.h
# End Source File
# Begin Source File

SOURCE=.\DDEngine\Headers\truetype.h
# End Source File
# Begin Source File

SOURCE=.\Headers\vehicle.h
# End Source File
# Begin Source File

SOURCE=.\Headers\walkable.h
# End Source File
# Begin Source File

SOURCE=.\Headers\ware.h
# End Source File
# Begin Source File

SOURCE=.\Headers\water.h
# End Source File
# Begin Source File

SOURCE=.\Headers\widget.h
# End Source File
# Begin Source File

SOURCE=.\Headers\wmove.h
# End Source File
# Begin Source File

SOURCE=.\Headers\xlat_str.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\DDLibrary\arrowcop.cur
# End Source File
# Begin Source File

SOURCE=.\DDLibrary\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\DDLibrary\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\DDLibrary\DDlib.rc
# End Source File
# Begin Source File

SOURCE=.\DDLibrary\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\DDLibrary\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\text\lang_english.txt
# End Source File
# Begin Source File

SOURCE=.\notes.txt
# End Source File
# End Target
# End Project
