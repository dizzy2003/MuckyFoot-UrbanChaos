// MFStdLib.h
// Guy Simmons, 18th December 1997.

#ifndef	MF_STD_LIB_H
#define	MF_STD_LIB_H

//---------------------------------------------------------------

// Standard 'C' includes.
//psx#include	<iostream.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
#include	<string.h>
//psx#include	<time.h>

// Library defines.
#define	_MF_WINDOWS

#ifndef	_WIN32
	#define	_WIN32
#endif

#ifndef	WIN32
	#define	WIN32
#endif

// Specific Windows includes.
/*
#include	<windows.h>
#include	<windowsx.h>
#include	<ddraw.h>
#include	<dinput.h>
#include	<dplay.h>
#include	<dsound.h>
#include	<d3d.h>
#include	<d3dtypes.h>
*/

//---------------------------------------------------------------

#define	TRUE				1
#define	FALSE				0

typedef	unsigned char		UBYTE;
typedef signed char			SBYTE;
typedef char				CBYTE;
typedef unsigned short		UWORD;
typedef signed short		SWORD;
typedef unsigned long		ULONG;
typedef signed long			SLONG;
typedef	unsigned char		BOOL;

typedef signed long			FIX_8;
typedef signed long			FIX_9;
typedef signed long			FIX_10;
typedef signed long			FIX_11;
typedef signed long			FIX_12;
typedef signed long			FIX_13;
typedef signed long			FIX_14;
typedef signed long			FIX_15;
typedef signed long			FIX_16;
typedef signed long			FIX_17;
typedef signed long			FIX_18;
typedef signed long			FIX_19;
typedef signed long			FIX_20;
typedef signed long			FIX_21;
typedef signed long			FIX_22;
typedef signed long			FIX_23;
typedef signed long			FIX_24;
typedef signed long			FIX_25;
typedef signed long			FIX_26;
typedef signed long			FIX_27;
typedef signed long			FIX_28;
typedef signed long			FIX_29;
typedef signed long			FIX_30;

typedef struct
{
	SLONG		X,
				Y;
}MFPoint;

typedef struct
{
	SLONG		Left,
				Top,
				Right,
				Bottom,
				Width,
				Height;
}MFRect;



//
// PSX Bodges
//


#define	GetTickCount()		(0)
//#define	ApproxRoot(x,div)		( ( (x/div)+div )>>1)
//#define	Root(x)				(ApproxRoot(x,ApproxRoot(x,ApproxRoot(x,ApproxRoot(x,128)) )))
#define		Root(x)		SquareRoot0(x)





//---------------------------------------------------------------
// MF Standard includes.

//#include	"StdFile.h"
#include	"StdKeybd.h"
#include	"StdMaths.h"
#include	"StdMem.h"
//#include	"StdMouse.h"

//---------------------------------------------------------------
// Display

#define	FLAGS_USE_3DFX			(1<<0)
#define	FLAGS_USE_3D			(1<<1)
#define	FLAGS_USE_WORKSCREEN	(1<<2)

extern	UBYTE				WorkScreenDepth,
							*WorkScreen;
extern	SLONG				WorkScreenHeight,
							WorkScreenPixelWidth,
							WorkScreenWidth;

SLONG			OpenDisplay(ULONG width, ULONG height, ULONG depth, ULONG flags);
SLONG			SetDisplay(ULONG width,ULONG height,ULONG depth);
SLONG			CloseDisplay(void);
void			ClearDisplay(void);
void			FadeDisplay(UBYTE mode);
void			*LockWorkScreen(void);
void			UnlockWorkScreen(void);
void			ShowWorkScreen(ULONG flags);
void			ClearWorkScreen(UBYTE colour);

//---------------------------------------------------------------
// Host

#define	SHELL_NAME				"Mucky Foot Shell\0"
#define	H_CREATE_LOG			(1<<0)
#define	SHELL_ACTIVE			(LibShellActive())
#define	SHELL_CHANGED			(LibShellChanged())

//#define	main(ac,av)				MF_main(ac,av)

struct MFTime
{
	SLONG		Hours,
				Minutes,
				Seconds,
				MSeconds;
	SLONG		DayOfWeek,		//	0 - 6;		Sunday		=	0
				Day,
				Month,			//	1 - 12;		January		=	1
				Year;
	SLONG		Ticks;			// Number of ticks(milliseconds) since windows started.
};

SLONG			main(UWORD argc, CBYTE** argv);
BOOL			SetupHost(ULONG flags);
void			ResetHost(void);
void			DebugText(CBYTE *error, ...);
void            TraceText(CBYTE *error, ...);
BOOL			LibShellActive(void);
BOOL			LibShellChanged(void);
BOOL			LibShellMessage(const char *pMessage, const char *pFile, ULONG dwLine);

#define	NoError					0


#ifndef NDEBUG

//#define TRACE				TraceText
//#define	LogText				DebugText
//#define	MFMessage			LibShellMessage
//#define	ERROR_MSG(e,m)		{if(!(e)) {}}
//#define ASSERT(e)			{if (!(e)) {}}

//void			DebugText(CBYTE *error, ...);
//void            TraceText(CBYTE *error, ...);
#define	DebugText
#define	TraceText
#define TRACE
#define LogText
#define MFMessage
#define ERROR_MSG(e,m)		{}
#define ASSERT(e)			{if(!(e))asm("break 0");}

#else

#define DebugText
#define TrackText
#define TRACE
#define LogText
#define MFMessage
#define ERROR_MSG(e,m)		{}
#define ASSERT(e)			{}

#endif

//---------------------------------------------------------------
// Input.

#define	MOUSE			0
#define	KEYBOARD		0 //DIDEVTYPE_KEYBOARD 
#define	JOYSTICK		1 //DIDEVTYPE_JOYSTICK


#ifdef PSX
BOOL	GetInputDevice(UBYTE type,UBYTE sub_type);
BOOL	ReadInputDevice(void);
#endif

//---------------------------------------------------------------
//---------------------------------------------------------------
// Sound.

#define	SAMPLE_VOL_MIN		1
#define	SAMPLE_VOL_MAX		2

#define	SAMPLE_PAN_LEFT		3
#define	SAMPLE_PAN_RIGHT	4
#define	SAMPLE_PAN_CENTER	5

#define	SAMPLE_FREQ_MIN		6
#define	SAMPLE_FREQ_MAX		7
#define	SAMPLE_FREQ_ORIG	8


void	LoadSampleList(CBYTE *sample_file);
void	PlaySample(SLONG ref,SWORD sample_no,SLONG vol,SLONG pan,SLONG freq,SLONG pri);




//	New Sound

#define	WAVE_STEREO				(1<<0)
#define	WAVE_POLAR				(1<<1)
#define	WAVE_CARTESIAN			(1<<2)
#define	WAVE_PAN_RATE			(1<<3)
#define	WAVE_DISTANCE_MAPPING	(1<<4)
#define	WAVE_LOOP				(1<<5)
#define	WAVE_SET_LOOP_POINTS	(1<<6)

#define	WAVE_TYPE_MASK		(WAVE_STEREO|WAVE_POLAR|WAVE_CARTESIAN)

#define	WAVE_PLAY_INTERUPT		0
#define	WAVE_PLAY_NO_INTERUPT	1
#define	WAVE_PLAY_OVERLAP		2
#define	WAVE_PLAY_QUEUE			3

struct WaveParams
{
	ULONG		Flags,
				LoopStart,
				LoopEnd,
				Priority;
	union
	{
		//	Stereo.
		struct
		{
			SLONG	Pan,
					Volume;
		}Stereo;

		//	Polar.
		struct
		{
			SLONG	Azimuth,
					Elevation,
					Range;
		}Polar;

		//	Cartesian.
		struct
		{
			SLONG	Scale,
					X,
					Y,
					Z;
		}Cartesian;
	}Mode;
};
void	LoadWaveList(CBYTE *path,CBYTE *file);
void	FreeWaveList(void);
void	PlayWave(SLONG ref,SLONG wave_id,SLONG play_type,WaveParams *the_params);
void	StopWave(SLONG ref,SLONG wave_id);
void	SetListenerPosition(SLONG x,SLONG y,SLONG z,SLONG scale);

//---------------------------------------------------------------
// Standard macros.

#define	sgn(a)				(((a)<0) ? -1 : 1)
#define	swap(a,b)			{a^=b;b^=a;a^=b;}

#define	in_range(a,min,max)	{if(a>(max))a=(max);else if(a<(min))a=(min);}
#ifndef	min
#define	min(a,b)			(((a)<(b)) ? (a) : (b))
#endif

#ifndef	max
#define	max(a,b)			(((a)>(b)) ? (a) : (b))
#endif

//---------------------------------------------------------------

//
// Stuff put in by Mark...
//

#define INFINITY		0x7fffffff
#define PI				(3.14159265F)
#define WITHIN(x,a,b)	((x) >= (a) && (x) <= (b))
#define SATURATE(x,a,b)	{if ((x) < (a)) {(x) = (a);} else if ((x) > (b)) {(x) = (b);}}
#define SWAP(a,b)		{SLONG temp; temp = (a); (a) = (b); (b) = temp;}
#define SWAP_FL(a,b)	{float temp; temp = (a); (a) = (b); (b) = temp;}
#define MIN(a,b)		(((a) < (b)) ? (a) : (b))
#define MAX(a,b)		(((a) > (b)) ? (a) : (b))
#define SIGN(x)			(((x)) ? (((x) > 0) ? +1 : -1) : 0)

//
// Some maths stuff by mike
//

#define	QDIST3(x,y,z)	((x)>(y) ? ((x)>(z) ? (x)+((y)>>2)+((z)>>2) : (z)+((x)>>2)+((y)>>2)) : ((y)>(z) ? ((y)+((x)>>2)+((z)>>2)) : (z)+((x)>>2)+((y)>>2) ))
#define	QDIST2(x,y)		((x)>(y) ? ((x)+((y)>>1) ):((y)+((x)>>1) ))

#define	SDIST3(x,y,z)	(((x)*(x))+((y)*(y))+((z)*(z)))
#define	SDIST2(x,y)		(((x)*(x))+((y)*(y)))


#endif

