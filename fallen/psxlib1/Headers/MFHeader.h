// MFHeader.h
// Guy Simmons, 1st Februry 1997.

#ifndef	MFHEADER_H
#define	MFHEADER_H

// Standard 'C' includes.
#include	<iostream.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
#include	<time.h>


// Set up Compilation defines.
#include	<MFLbType.h>


// Specific Windows includes.
#ifdef	_MF_WINDOWS

	#ifndef	_WIN32
		#define	_WIN32
	#endif

	#ifndef	WIN32
		#define	WIN32
	#endif

	#include	<windows.h>
	#include	<windowsx.h>
//	#include	<mmsystem.h>
	#include	<ddraw.h>
//	#include	<dinput.h>
	#include	<dplay.h>
	#include	<d3d.h>
	#include	<d3dtypes.h>

	#include	<MFD3D.h>

#endif

// Specific DOS includes.
#ifdef	_MF_DOSX
	#include	<fcntl.h>
	#include	<io.h>
	#include	<share.h>
	#include	<stdio.h>
	#include	<sys/types.h>
	#include	<sys/stat.h>
#endif


// Mucky Foot library includes.
#include	<Display.h>
#include	<Draw2D.h>
#include	<DrawPoly.h>
#include	<Keyboard.h>
#include	<MFErrors.h>
#include	<MFFile.h>
#include	<MFHost.h>
#include	<MFMaths.h>
#include	<MFMem.h>
#include	<MFStd.h>
#include	<MFUtils.h>
#include	<Mouse.h>
#include	<Palette.h>
#include	<Sprites.h>

#endif
