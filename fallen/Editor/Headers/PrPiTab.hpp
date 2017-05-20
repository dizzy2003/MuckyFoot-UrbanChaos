// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_PRIMPICKTAB_HPP_
#define	_PRIMPICKTAB_HPP_

#include	"ModeTab.hpp"
#include	"Stealth.h"
#include	"EditMod.hpp"
#include	"undo.hpp"

/*
#define	CTRL_PRIM_LOAD_BACKGROUND	1
#define	CTRL_PRIM_APPEND_NEW		2
#define	CTRL_PRIM_LOAD_MAP			3
#define	CTRL_PRIM_SAVE_MAP			4
#define	CTRL_PRIM_LORES_TEST		5
#define	CTRL_PRIM_X_AXIS_FREE		6
#define	CTRL_PRIM_Y_AXIS_FREE		7
#define	CTRL_PRIM_Z_AXIS_FREE		8
#define	CTRL_PRIM_GRID_ON			9
*/

#define	X_AXIS					(1<<0)
#define	Y_AXIS					(1<<1)
#define	Z_AXIS					(1<<2)

#define	PRIM_MODE_SINGLE		0
#define	PRIM_MODE_MULTI			1
#define	PRIM_MODE_BACK			2
#define	PRIM_MODE_ANIM_KEY		3
#define	PRIM_MODE_ANIM_MORPH	4

extern	void	record_prim_status(void);

extern	void	apply_user_rotates(struct PrimPoint *point);

class	PrimPickTab	:	public	ModeTab
{
	private:
		SLONG				ListPos;
		SLONG				CurrentPrim;
		SLONG				DragThingView1;
		SLONG				DragThingView2;
		SLONG				DragThingView3;
		SLONG				Axis;
		SLONG				GridFlag;
		SLONG				GridMax;
		SLONG				GridCorner;
		UBYTE				AxisMode,
							PrimTabMode;
		EdRect				PrimRect,
							View1,
							View2,
							View3;
		EditorModule		*Parent;
		UBYTE				RedrawTabContent;
		void				DrawABuildingInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h);
		void				DrawAPrimInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h);
		void				DrawAMultiPrimInRect(ULONG prim,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				PrimScale,BackScale;
	public:
							PrimPickTab(EditorModule *parent);
							~PrimPickTab();
		void				UpdatePrimInfo(void);
		void				DrawPrims(void);
		void				UpdatePrimPickWindow(void);
		void				DrawTabContent(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HiLightObjects(SLONG x,SLONG y,SLONG w,SLONG h);
		inline SLONG		GetCurrentPrim(void)	                                        {return CurrentPrim;}
		inline void			SetCurrentPrim(SLONG prim)	                                        {CurrentPrim=prim;}
		SLONG				SetWorldMouse(ULONG flag);
		SLONG				DragAPrim(UBYTE flags,MFPoint *clicked_point,SLONG button);
		SLONG				DragEngine(UBYTE flags,MFPoint *clicked_point);
		SLONG				KeyboardInterface(void);
		Undo				MyUndo;
		UBYTE				RedrawModuleContent;
		UBYTE				View2Mode;

		inline UBYTE		GetPrimTabMode(void)		{	return PrimTabMode;	}
		inline void			SetPrimTabMode(UBYTE mode)		{	PrimTabMode=mode;	}
};


extern	 void	add_a_background_thing(UWORD prim,SLONG x,SLONG y,SLONG z);

#endif

