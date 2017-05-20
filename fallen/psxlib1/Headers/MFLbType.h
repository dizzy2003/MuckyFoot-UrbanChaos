// MFLbType.h
// Guy Simmons, 31st January 1997.

#ifndef	_MFLBTYPE_H_
#define	_MFLBTYPE_H_

//----------------------------------------------------------------------
// Common header for generic types, macros & functions
//----------------------------------------------------------------------

/*
	Predefined macros		-	Environment & Compilation defines.

	
	Compile state:
		_DEBUG			Compiles a debug build.

		_RELEASE		Comples a release build.

	Compiler:
		_CW_			Codewarrior

		__MSC__			Microsoft Visual C++.

		__WATCOMC__		Watcom C++ Optimising compiler.

	Host Platform:
		_MF_WINDOWS		Microsoft Windows OS conforming to Win32 specification (Win95, NT3.51).

		_MF_DOSX		Extended DOS.

		_MAC68K			Macintosh 68K based machines.

		_MACPPC			Macintosh PPC based machines.

		_PSX			Sony Playstation.

	Host Processor:
		_X86_			Intel compatible (PC).

		_68K_			Motorola 68K series compatible (Amiga, Macintosh, Pilot).

		_PPC_			Motorola PPC series compatible (M2, Macintosh).

		_MIPS_			(PSX)
*/


#ifdef	_DEBUG
	#ifdef	_RELEASE
		#error	Cannot compile Debug & Release builds at the same time.
	#endif
#else
	#ifndef	_RELEASE
		#error	_DEBUG or _RELEASE need to be defined.
	#endif
#endif

#ifdef __WINDOWS_386__ 		// Watcom predefined for Windows build
	#define _MF_WINDOWS
#elif defined(WIN32)
	#define _MF_WINDOWS
#endif

#ifdef	__DOS__
	#define	_MF_DOSX
#endif


/*
	Implicit library linking	-	Set according to compile state.

	Naming convention.
	
	Compiler	:	(C)Codewarrior,
					(M)MSDev,
					(W)Watcom.

	Platform	:	(_DOS)Extended dos,
					(_M68)Macintosh 68K,
					(_MPP)Macintosh PPC,
					(_PSX)Playstation,
					(_WIN)Windows.

	Version		:	1

	Compilation	:	(_D)Debug,
					(_R)Release.


*/

#ifdef	_DEBUG
	#ifdef _MSC_VER
		#pragma comment(lib, "ddraw.lib")
		#ifndef TARGET_DC
			#pragma comment(lib, "dplay.lib")
			#pragma comment(lib, "d3drm.lib")
		#else
			#pragma comment(lib, "dplayx.lib")
		#endif
		#pragma comment(lib, "M_WIN1_D")
	#elif defined(__WATCOMC__)
		#ifdef _MF_WINDOWS
			#pragma library("W_WIN1_D")
		#elif defined(_MF_DOSX)
			#pragma library("W_DOS1_D")
		#endif
	#endif
#else
	#ifdef _MSC_VER
		#pragma comment(lib, "ddraw.lib")
		#ifndef TARGET_DC
			#pragma comment(lib, "dplay.lib")
			#pragma comment(lib, "d3drm.lib")
		#else
			#pragma comment(lib, "dplayx.lib")
		#endif
		#pragma comment(lib, "M_WIN1_R")
	#elif defined(__WATCOMC__)
		#ifdef _MF_WINDOWS
			#pragma library("W_WIN1_R")
		#elif defined(_MF_DOSX)
			#pragma library("W_DOS1_R")
		#endif
	#endif
#endif

//----------------------------------------------------------------------

#endif
