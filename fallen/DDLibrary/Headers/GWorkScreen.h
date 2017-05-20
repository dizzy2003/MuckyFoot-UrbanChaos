// WorkScreen.h
// Guy Simmons, 10th December 1997

#ifndef	WORK_SCREEN_H
#define	WORK_SCREEN_H



// Never used on DC.
#ifndef TARGET_DC


//---------------------------------------------------------------

extern UBYTE			*WorkScreen,
						WorkScreenDepth;
extern SLONG			WorkScreenHeight,
						WorkScreenWidth,
						WorkScreenPixelWidth;

extern UBYTE			*WorkWindow;
extern SLONG			WorkWindowHeight,
	 					WorkWindowWidth;
extern MFRect			WorkWindowRect;

//---------------------------------------------------------------

void			ShowWorkScreen(ULONG flags);
void			*LockWorkScreen(void);
void			UnlockWorkScreen(void);

void			SetWorkWindowBounds(SLONG left, SLONG top, SLONG width, SLONG height);

inline void		SetWorkWindow(void)	{	WorkWindow=(WorkScreen+WorkWindowRect.Left*WorkScreenDepth+(WorkWindowRect.Top*WorkScreenWidth));	}

MFPoint			*GlobalToLocal(MFPoint *the_point);
void			GlobalXYToLocal(SLONG *x,SLONG *y);
inline BOOL		XYInRect(SLONG x,SLONG y,MFRect *the_rect)			{	if(x>=the_rect->Left&&y>=the_rect->Top&&x<=the_rect->Right&&y<=the_rect->Bottom)return TRUE;else return FALSE;	}
inline BOOL		PointInRect(MFPoint *the_point,MFRect *the_rect)	{	if(the_point->X>=the_rect->Left&&the_point->Y>=the_rect->Top&&the_point->X<=the_rect->Right&&the_point->Y<=the_rect->Bottom)return TRUE;else return FALSE;	}

//---------------------------------------------------------------



#endif //#ifndef TARGET_DC


#endif
