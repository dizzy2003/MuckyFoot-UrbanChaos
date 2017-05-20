// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_COLTAB_HPP_
#define	_COLTAB_HPP_

#include	"ModeTab.hpp"
#include	"Stealth.h"
#include	"EditMod.hpp"
#include	"undo.hpp"

#define	COL_TYPE_PLANE	1
#define	COL_TYPE_BEZIER	2

struct	ColInfo
{
	UWORD	Type;
	UWORD	Next;
	union	
	{
		struct
		{
			SLONG	Left;
			SLONG	Right;
			SLONG	Top;
			SLONG	Bottom;
			SLONG	Depth;
		}Plane;
		struct
		{
			SLONG	Top;
			SLONG	Bottom;
			SLONG	X[4];
			SLONG	Z[4];
		}Bezier;
	};
};

class	ColTab	:	public	ModeTab
{
	private:
		SLONG				Axis;
		SLONG				GridFlag;
		UBYTE				AxisMode;
		EdRect				View1;
		EdRect				View2;
		EdRect				View3;
		UBYTE				RedrawTabContent;
		UWORD				CurrentCol;
		UWORD				ClipView;
	public:
							ColTab(EditorModule *parent);
							~ColTab();
		void				DrawTabContent(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				SetWorldMouse(ULONG flag);
		SLONG				KeyboardInterface(void);
		SLONG				DragACol(UBYTE flags,MFPoint *clicked_point,UWORD copy);
		SLONG				DragEngine(UBYTE flags,MFPoint *clicked_point);
//		Undo				MyUndo;
		UBYTE				RedrawModuleContent;
		void				Recalc(void);
		void				Clear(void);
		UWORD				PlaceBezier(void);
		UWORD				Mode;
		EditorModule		*Parent;
};


#define	MAX_COL_INFO	1000
	
extern	struct	ColInfo	col_info[MAX_COL_INFO];
extern	UWORD	next_col_info;

#endif

