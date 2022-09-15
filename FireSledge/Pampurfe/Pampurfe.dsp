# Microsoft Developer Studio Project File - Name="Pampurfe" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Pampurfe - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Pampurfe.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Pampurfe.mak" CFG="Pampurfe - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Pampurfe - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Pampurfe - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Pampurfe - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PAMPURFE_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /GR /GX /O2 /Ob2 /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PAMPURFE_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 mdk.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"Release/FireSledge Pampurfe.dll"

!ELSEIF  "$(CFG)" == "Pampurfe - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PAMPURFE_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MTd /W3 /Gm /GR /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PAMPURFE_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mdk.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"Debug/FireSledge Pampurfe.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "Pampurfe - Win32 Release"
# Name "Pampurfe - Win32 Debug"
# Begin Group "basic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\basic\Array.h
# End Source File
# Begin Source File

SOURCE=.\basic\Array.hpp
# End Source File
# Begin Source File

SOURCE=.\basic\basic_def.h
# End Source File
# Begin Source File

SOURCE=.\basic\basic_fnc.h
# End Source File
# Begin Source File

SOURCE=.\basic\basic_fnc.hpp
# End Source File
# End Group
# Begin Group "dsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dsp\BasicMixing.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\BasicMixing.h
# End Source File
# Begin Source File

SOURCE=.\dsp\Biquad.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\Biquad.h
# End Source File
# Begin Source File

SOURCE=.\dsp\Biquad.hpp
# End Source File
# Begin Source File

SOURCE=.\dsp\BiquadS.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\BiquadS.h
# End Source File
# Begin Source File

SOURCE=.\dsp\BiquadS.hpp
# End Source File
# Begin Source File

SOURCE=.\dsp\InterpFncFinite.h
# End Source File
# Begin Source File

SOURCE=.\dsp\InterpFncFiniteAsym.h
# End Source File
# Begin Source File

SOURCE=.\dsp\InterpFncFiniteAsym.hpp
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePole.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePole.h
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePole.hpp
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePoleS.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePoleS.h
# End Source File
# Begin Source File

SOURCE=.\dsp\OnePoleS.hpp
# End Source File
# Begin Source File

SOURCE=.\dsp\RmsDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\RmsDetector.h
# End Source File
# Begin Source File

SOURCE=.\dsp\Shelf.cpp
# End Source File
# Begin Source File

SOURCE=.\dsp\Shelf.h
# End Source File
# Begin Source File

SOURCE=.\dsp\Shelf.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Distorter.cpp
# End Source File
# Begin Source File

SOURCE=.\Distorter.h
# End Source File
# Begin Source File

SOURCE=.\GainFixer.cpp
# End Source File
# Begin Source File

SOURCE=.\GainFixer.h
# End Source File
# Begin Source File

SOURCE=.\MachineInterface.h
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mdk.h
# End Source File
# Begin Source File

SOURCE=.\Pampurfe.cpp
# End Source File
# Begin Source File

SOURCE=.\Pampurfe.h
# End Source File
# Begin Source File

SOURCE=.\resources.rc
# End Source File
# Begin Source File

SOURCE=.\skin.bmp
# End Source File
# Begin Source File

SOURCE=.\SoundEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundEngine.h
# End Source File
# Begin Source File

SOURCE=.\textcolor.bin
# End Source File
# Begin Source File

SOURCE=.\ToneFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ToneFilter.h
# End Source File
# End Target
# End Project
