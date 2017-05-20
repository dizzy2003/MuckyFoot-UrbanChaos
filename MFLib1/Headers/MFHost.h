// MFHost.h		-	Windows.
// Guy Simmons, 1st February 1997.


#define	H_CREATE_LOG			(1<<0)

#ifdef	_MF_WINDOWS

extern int						iGlobalCmdShow;
extern HINSTANCE				hGlobalInstance,
								hGlobalPrevInstance;
extern LPSTR					szGlobalCmdLine;

#define	main(ac,av)				MF_main(ac,av)

SLONG	main(UWORD argc, TCHAR** argv);

#endif

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

BOOL	SetupHost(ULONG flags);
void	ResetHost(void);
void	LogText(CBYTE *error, ...);
void	Time(struct MFTime *the_time);
