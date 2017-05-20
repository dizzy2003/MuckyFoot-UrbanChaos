// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_MAPEDTAB_HPP_
#define	_MAPEDTAB_HPP_

#include	"ModeTab.hpp"
#include	"Stealth.h"
#include	"EditMod.hpp"
#include	"undo.hpp"

#define	FLOOR_CUT_BRUSH			1
#define	FLOOR_PASTE_BRUSH		2
#define	FLOOR_HOLD_BRUSH		3
#define	FLOOR_CUT_BRUSH_DEF		4


class	MapEdTab	:	public	ModeTab
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
		SLONG				X1,Y1,Z1,X2,Y2,Z2;
	public:
							MapEdTab(EditorModule *parent);
							~MapEdTab();
		void				DrawTabContent(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				SetWorldMouse(ULONG flag);
		SLONG				KeyboardInterface(void);
		SLONG				DragEngine(UBYTE flags,MFPoint *clicked_point);
		SLONG				CalcMapCoord(SLONG	*mapx,SLONG	*mapy,SLONG	*mapz,SLONG	x,SLONG	y,SLONG	w,SLONG	h,MFPoint	*clicked_point);
		SLONG				DragPaint(UBYTE flags);
		SLONG				DragMark(UBYTE flags);
		void				CutFloorBrush(MFPoint *current_point,SLONG button);
		SLONG				MouseInContent(void);
		void				DragAltitude(SLONG mx,SLONG mz);
		SLONG				FlattenArea(void);
		SLONG				SmoothArea(void);
		SLONG				SlopeArea(void);
		void				ChangeMapAltitude(SLONG mx,SLONG mz,SLONG step,UBYTE offset_flag);
//		Undo				MyUndo;
		UBYTE				RedrawModuleContent;
		void				Clear(void);
		UWORD				Mode;
		UWORD				SubMode;
		EditorModule		*Parent;
		BuildTab			*BuildMode;
		MapBlock			CutMapBlock;
		SLONG				RoofTop;
		SLONG				Texture;
};



#endif

