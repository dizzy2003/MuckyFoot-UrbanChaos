// TexTab.hpp
// MCD, 20th February 1997

#ifndef	_MAPTAB_HPP_
#define	_MAPTAB_HPP_

#include	"ModeTab.hpp"
#include	"Stealth.h"
#include	"EditMod.hpp"
#include	"undo.hpp"

#define	MAP_TYPE_PLANE	1
#define	MAP_TYPE_BEZIER	2

struct	MapInfo
{
	SLONG	Left;
	SLONG	Right;
	SLONG	Top;
	SLONG	Bottom;

	SLONG	X;
	SLONG	Y;
	SLONG	Z;
	SWORD	AngleY;
	SWORD	Background;
	void	*PtrMap;
	SWORD	MapWidth;
	SWORD	MapHeight;
	CBYTE	Name[20];
	SLONG	Dummy[4];


};

class	MapTab	:	public	ModeTab
{
	private:
		SLONG				Axis;
		SLONG				GridFlag;
		UBYTE				AxisMode;
		EdRect				View1;
		EdRect				View2;
		EdRect				View3;
		UBYTE				RedrawTabContent;
		UWORD				CurrentMap;
		UWORD				DefMode;
	public:
							MapTab(EditorModule *parent);
							~MapTab();
		void				DrawTabContent(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				SetWorldMouse(ULONG flag);
		SLONG				KeyboardInterface(void);
		SLONG				DragAMap(UBYTE flags,MFPoint *clicked_point,UWORD copy);
		SLONG				DragAMapDef(UBYTE flags,MFPoint *clicked_point,UWORD copy);
		SLONG				DragAMapDefXYZ(UBYTE flags,MFPoint *clicked_point,UWORD copy);
		SLONG				DragEngine(UBYTE flags,MFPoint *clicked_point);
		void				SetMapPos(SLONG x,SLONG y,SLONG z);
//		Undo				MyUndo;
		UBYTE				RedrawModuleContent;
		void				Recalc(void);
		void				Clear(void);
		UWORD				PlaceBezier(void);
		UWORD				Mode;
		EditorModule		*Parent;
};


#define	MAX_MAP_INFO	1000
	
extern	struct	MapInfo	map_info[MAX_MAP_INFO];
extern	UWORD	next_map_info;

#endif

