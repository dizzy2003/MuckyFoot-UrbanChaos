// ComTab.h
// Guy Simmons, 9th March 1998.

#ifndef	COMTAB_H
#define	COMTAB_H

#include	"EdCom.h"

#define	COM_MODE_NONE				0
#define	COM_MODE_SELECT_THING		1
#define	COM_MODE_SELECT_WAYPOINT	2
#define	COM_MODE_SELECT_SWITCH		3

//---------------------------------------------------------------

class	CommandTab	:	public	ModeTab
{
	private:
		UWORD			DataField,
						TabMode;
		SLONG			TabData;
		EditCommand		*DataCommand;
		EditComList		*CurrentComList;

	public:
						CommandTab();
						~CommandTab();

		void			DrawTabContent(void);
		void			UpdateTab(UBYTE update_level);
		UWORD			HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleTab(MFPoint *current_point);
		void			HandleControl(UWORD control_id);
		void			DoComListPopup(MFPoint *clicked_point,EditComList *the_comlist);
		void			DoCommandPopup(MFPoint *clicked_point,UWORD select_pos);
		void			CommonCommandOptions(ULONG id,EditCommand *the_command);
		EditCondList	*CommandTab::SelectConditionList(void);

		void			DrawListsBox(void);
		void			DrawCurrentList(void);

		UWORD			ListsHilitePos(MFPoint *current_point);
		EditComList		*HilitetedList(UWORD select_pos);
		UWORD			CurrentListHilitePos(MFPoint *current_point);
		EditCommand		*HilitetedCommand(UWORD select_pos);

		inline UWORD	GetTabMode(void)				{	return	TabMode;		}
		inline void		SetTabMode(UWORD mode)			{	TabMode=mode;			}
		inline void		SetTabData(SLONG data)			{	TabData=data;			}
};

//---------------------------------------------------------------

#endif
