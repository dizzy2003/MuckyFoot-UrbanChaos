// GameEd.h
// Guy Simmons, 12th January 1998.

#ifndef	GAMEED_H
#define	GAMEED_H

#include	"engine.h"
#include	"ComTab.h"
#include	"CondTab.h"
#include	"SaveTab.h"
#include	"ThingTab.h"

//---------------------------------------------------------------

#define	CLASS_NONE			0
#define	CLASS_PLAYER		1
#define	CLASS_CAMERA		2
#define	CLASS_PROJECTILE	3
#define CLASS_BUILDING		4
#define CLASS_PERSON		5

#define	TAB_NONE			0
#define	TAB_THINGS			1
#define	TAB_LEVELS			2
#define	TAB_COMMANDS		3
#define	TAB_CONDITIONS		4

#define	SELECT_NONE				0
#define	SELECT_WAYPOINT			1
#define	SELECT_NEXT_WAYPOINT	2
#define	SELECT_PREV_WAYPOINT	3
#define	SELECT_COND_TAB_THING	4
#define	SELECT_COND_TAB_SWITCH	5
#define	SELECT_THING_TAB_THING	6
#define	SELECT_THING_TAB_SWITCH	7
#define	SELECT_COM_TAB_WAYPOINT	8
#define	SELECT_COM_TAB_THING	9
#define	SELECT_COM_TAB_SWITCH	10

//---------------------------------------------------------------

class	GameEditor		:	public	EditorModule
{
	private:

		BOOL			FlashState;
		UBYTE			SelectMode;
		SLONG			CurrentThing,
						FlashCount;
		EdItem			BackupItem,
						HilitedItem,
						LastItem,
						SelectedItem;
		EngineStuff		EdEngine;
		MFPoint			DownPoint;

		CommandTab		*CommandMode;
		ConditionTab	*ConditionMode;
		SaveTab			*SaveMode;
		ThingTab		*ThingMode;


	public:

						~GameEditor();

		void			SetupModule(void);
		void			CreateTabs(void);
		void			DestroyTabs(void);

		void			DrawContent(void);
		void			HandleContentClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleControlClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleModule(void);
		void			HandleThingDrag(void);
		UWORD			EngineKeys(void);
		void			DoThingPopup(MFPoint *clicked_point);
		void			DoBlockPopup(MFPoint *clicked_point);
		void			HandleWaypointDrag(void);
		void			DoWaypointPopup(MFPoint *clicked_point);
		void			HandleSizeDrag(void);

		void			GameEdEngine(void);
		SLONG			DrawFacet(UWORD facet_index,SLONG x,SLONG y,SLONG z);
		void			ScanEngine(void);
		void			RenderEngine(void);
		void			MapText(SLONG x,SLONG y,CBYTE *the_str,ULONG col);
		void			MapThingInfo(SLONG x,SLONG y,BucketMapThing *the_map_thing);
		void			MapWaypointInfo(SLONG x,SLONG y,BucketWaypoint *the_waypoint);
		void			MapSphereInfo(SLONG x,SLONG y,BucketSphereArea *the_sphere);

		void			ClearTabMode(void);

		inline void		SetSelectMode(UBYTE mode)	{	SetLocalEscape();SelectMode=mode;BackupItem=SelectedItem;SelectedItem=HilitedItem;	}
		inline void		ClearSelectMode(void)		{	ClearTabMode();ClearLocalEscape();SelectMode=0;SelectedItem=BackupItem;	}
};

//---------------------------------------------------------------

struct	GameEdDefaults
{
	SLONG		Left,
				Top;
	SLONG		Height,
				Width;
};

//---------------------------------------------------------------

#endif
