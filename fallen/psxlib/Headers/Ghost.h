// MFHost.h		-	Windows.
// Guy Simmons, 1st February 1997.


#define	H_CREATE_LOG			(1<<0)

#define	PAUSE_TIMEOUT			500
#define	PAUSE					(1<<0)
#define	PAUSE_ACK				(1<<1)

extern int						iGlobalCmdShow;
extern HINSTANCE				hGlobalInstance,
								hGlobalPrevInstance;
extern LPSTR					szGlobalCmdLine;

#define	main(ac,av)				MF_main(ac,av)

SLONG	main(UWORD argc, CBYTE** argv);


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
//void	LogText(CBYTE *error, ...);
//void	Time(struct MFTime *the_time);
BOOL	LibShellActive(void);
BOOL	LibShellChanged(void);
BOOL	LibShellMessage(const char *pMessage, const char *pFile, ULONG dwLine);
void	ShellPaused(void);
void	ShellPauseOn(void);
void	ShellPauseOff(void);
