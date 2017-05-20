// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_LIGHTTAB_HPP_
#define	_LIGHTTAB_HPP_

#include	"ModeTab.hpp"
#include	"Stealth.h"
#include	"EditMod.hpp"
#include	"undo.hpp"


#define		LIGHT_TAB_MODE_WAIT			 0
#define		LIGHT_TAB_MODE_PLACE_LIGHT	 1
#define		LIGHT_TAB_MODE_EDIT_LIGHT	 2
#define		LIGHT_TAB_MODE_DRAG_LIGHT	 3
#define		LIGHT_TAB_MODE_PLACE_AMBIENT 4
#define		LIGHT_TAB_MODE_REPEAT_PLACE_LIGHT	 5


class	LightTab	:	public	ModeTab
{
	private:
		SLONG				Axis;
		SLONG				GridFlag;
		UBYTE				AxisMode;
		EdRect				View1;
		EdRect				View2;
		EdRect				View3;
		UBYTE				RedrawTabContent;
		UBYTE				Shadow;
		UWORD				CurrentLight;
	public:
							LightTab(EditorModule *parent);
							~LightTab();
		void				DrawTabContent(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				HandleModuleContentClick(MFPoint	*clicked_point,UBYTE flags,SLONG x,SLONG y,SLONG w,SLONG h);
		SLONG				SetWorldMouse(ULONG flag);
		SLONG				KeyboardInterface(void);
		SWORD				CreateLightThing(SLONG x,SLONG y,SLONG z,SLONG bright);
		void				DeleteLightThing(SWORD thing);
		SLONG				DragALight(UBYTE flags,MFPoint *clicked_point,UWORD copy);
		SLONG				DragEngine(UBYTE flags,MFPoint *clicked_point);
		void				SetAmbientAngle(void);
		void				RecalcAllLights(void);
		void				SmoothGroup(void);
		Undo				MyUndo;
		UBYTE				RedrawModuleContent;
		UWORD				Mode;
		SLONG				ClickOnLight(MFPoint *clicked_point);
		EditorModule		*Parent;
};


extern	 void	add_a_background_thing(UWORD prim,SLONG x,SLONG y,SLONG z);
extern	void	apply_light_to_map(SLONG x,SLONG y,SLONG z,SLONG bright);

#endif

