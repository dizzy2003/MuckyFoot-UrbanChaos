// LevelEdit.cpp
// Guy Simmons, 19th February 1997.

#ifndef	_LEVELED_HPP_
#define	_LEVELED_HPP_

#include	"Edit.h"
#include	"EditMod.hpp"
#include	"PaintTab.hpp"
#include	"PrPiTab.hpp"
#include	"LightTab.hpp"
#include	"ColTab.hpp"
#include	"MapTab.hpp"
#include	"MapEdTab.hpp"
#include	"BuildTab.hpp"
#include	"HmTab.hpp"
#include	"SewerTab.hpp"

#define	TAB_NONE			0
#define	TAB_PAINT			1
#define	TAB_PRIMPICK		2
#define	TAB_LIGHT			3
#define	TAB_MAP				4
#define	TAB_MAPED			5
#define	TAB_BUILD			6
#define TAB_HM				7
#define TAB_SEWER			8

#define	UNDO_NONE			0
#define	UNDO_TEXTURE		1


class	LevelEditor		:	public	EditorModule
{
	private:
		UBYTE			UndoType,
						LastU[4],
						LastV[4];
		ControlSet		PrimSet;
		UWORD			CurrentAnimTmap;

	public:
						~LevelEditor(void);
		void			SetupModule(void);
		void			CreateLevelTabs(void);
		void			DestroyLevelTabs(void);
		void			DrawContent(void);
		void			DrawAnimTmapContent(SLONG current_anim_tmap);
		void			DrawTexStyleContent(void);
		void			DrawPSXTexContent(void);
		SLONG			HandlePSXTexClick(UBYTE flags,MFPoint *clicked_point);
		SLONG			HandleTexStyleClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleAnimTmapClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleContentClick(UBYTE flags,MFPoint *clicked_point);
		void			HandleControlClick(UBYTE flags,MFPoint *clicked_point);
		void			DragEngine(UBYTE flags,MFPoint *clicked_point);
		void			HandleStyleControl(ULONG  control_id);
		void			HandlePSXControl(ULONG  control_id);
		UBYTE			DoStylePopup(MFPoint *clicked_point,UBYTE flags);
		void			SetWallTextureInfo(SLONG	wall,UBYTE page,EdTexture	*current_texture);
		void			TextureFace(SWORD face,PaintTab *texture_mode);

		UBYTE			DoInStylePopup(MFPoint *clicked_point,UBYTE flags);
		void			DrawInTexStyleContent(void);
		void			HandleInStyleControl(ULONG  control_id);
		void			DrawTexInStyleContent(void);
		SLONG			HandleTexInStyleClick(UBYTE flags,MFPoint *clicked_point);


		void			HandleModule(void);
		void			DoFacePopup(MFPoint *clicked_point);
		BOOL			ApplyTexture(struct EditFace *edit_face);
		inline void		SetAnimTexture(SLONG tmap)			{	CurrentAnimTmap=(UWORD)tmap;	}
		ModeTab			*TestMode;
		PaintTab		*PaintMode;
		PrimPickTab		*PrimMode;
		LightTab		*LightMode;
		ColTab			*ColMode;
		MapTab			*MapMode;
		MapEdTab		*MapEdMode;
		BuildTab		*BuildMode;
		HmTab			*HmMode;
		SewerTab		*SewerMode;
		ControlSet		PSXControls;
		ControlSet		StyleControls;
		ControlSet		InStyleControls;
};

struct	LevelEdDefaults
{
	SLONG		Left,
				Top;
	SLONG		Height,
				Width;
};

#endif
