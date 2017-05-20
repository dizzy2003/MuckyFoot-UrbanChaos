// ThingTab.h
// Guy Simmons, 15th January 1998.

#ifndef	THINGTAB_H
#define	THINGTAB_H


#define	THING_MODE_NONE				0
#define	THING_MODE_SELECT_THING		1
#define	THING_MODE_SELECT_SWITCH	2

//---------------------------------------------------------------

class	ThingTab	:	public	ModeTab
{
	private:
		
		BOOL			Update;
		UBYTE			CurrentClass,
						CurrentGenus;
		UWORD			CurrentThing,
						TabMode;
		SLONG			TabData,
						ThingFlags,
						*DataPtr;
		ControlSet		CurrentSet;
		EdRect			CurrentSetRect;

	public:

						ThingTab();
						~ThingTab();

		void			DrawTabContent(void);
		void			UpdateTab(UBYTE update_level);
		UWORD			HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleTab(MFPoint *current_point);
		void			HandleControl(UWORD control_id);
		void			HandleClassControl(UWORD control_id);
		void			HandleBuildingControl(UWORD control_id);
		EditComList		*SelectCommandList(void);

		void			DrawClassSet(void);
		void			UpdateTabInfo(void);
		void			UpdateClassInfo(void);
		void			UpdateCheckBoxes(void);

		inline UBYTE	GetCurrentClass(void)			{	return	CurrentClass;	}
		inline UBYTE	GetCurrentGenus(void)			{	return	CurrentGenus;	}
		inline UBYTE	GetThingFlags(void)				{	return	ThingFlags;		}
		inline void		SetCurrentClass(UBYTE clss)		{	CurrentClass=clss;UpdateTabInfo();		}
		inline void		SetCurrentGenus(UBYTE genus)	{	CurrentGenus=genus;UpdateClassInfo();	}

		inline UWORD	GetTabMode(void)				{	return	TabMode;		}
		inline void		SetTabMode(UWORD mode)			{	TabMode=mode;			}
		inline void		SetTabData(SLONG data)			{	TabData=data;			}
		inline void		SetCurrentThing(UWORD thing)	{	CurrentThing=thing;UpdateClassInfo();	}
};

//---------------------------------------------------------------

#endif
