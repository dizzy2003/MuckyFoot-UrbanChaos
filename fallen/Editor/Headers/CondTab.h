// CondTab.h
// Guy Simmons, 16th March 1998.

#ifndef	CONDTAB_H
#define	CONDTAB_H

#include	"EdCom.h"

#define	COND_MODE_NONE			0
#define	COND_MODE_SELECT_THING	1
#define	COND_MODE_SELECT_SWITCH	2

//---------------------------------------------------------------

class	ConditionTab	:	public	ModeTab
{
	private:
		UWORD			DataField,
						TabMode;
		SLONG			TabData;
		EditCondition	*DataCondition;
		EditCondList	*CurrentCList;


	public:
						ConditionTab();
						~ConditionTab();

		void			DrawTabContent(void);
		void			UpdateTab(UBYTE update_level);
		UWORD			HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleTab(MFPoint *current_point);
		void			HandleControl(UWORD control_id);
		void			DoCListPopup(MFPoint *clicked_point,EditCondList *the_clist);
		void			DoConditionPopup(MFPoint *clicked_point,UWORD select_pos);
		void			CommonConditionOptions(ULONG id,EditCondition *the_condition);
		EditCondList	*SelectConditionList(void);

		void			DrawListsBox(void);
		void			DrawCurrentList(void);

		UWORD			ListsHilitePos(MFPoint *current_point);
		EditCondList	*HilitetedList(UWORD select_pos);
		UWORD			CurrentListHilitePos(MFPoint *current_point);
		EditCondition	*HilitetedCondition(UWORD select_pos);

		inline UWORD	GetTabMode(void)				{	return	TabMode;		}
		inline void		SetTabMode(UWORD mode)			{	TabMode=mode;			}
		inline void		SetTabData(SLONG data)			{	TabData=data;			}
};

//---------------------------------------------------------------

#endif
