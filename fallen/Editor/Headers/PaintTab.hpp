// PaintTab.hpp
// Guy Simmons, 7th April 1997

#ifndef	_PAINTTAB_HPP_
#define	_PAINTTAB_HPP_

#include	"undo.hpp"
#include	"ModeTab.hpp"
#include	"BuildTab.hpp"
#include	"Stealth.h"


#define	PALETTE_PAINT		0
#define	TEXTURE_PAINT		1
#define	PLANAR_PAINT		2
#define	ANIM_TMAP_PAINT		3
#define	FLOOR_PAINT			4
#define	STYLE_PAINT			5
#define	STYLE_DEFINE		6
#define	INSTYLE_DEFINE		7
#define	PSX_TEX_DEFINE		8


#define	FLOOR_CUT_BRUSH			1
#define	FLOOR_PASTE_BRUSH		2


#define	FLAGS_SHOW_TEXTURE	(1<<0)
#define	FLAGS_QUADS			(1<<1)
#define	FLAGS_FIXED			(1<<2)
extern	CBYTE	texture_style_names[200][21];

extern	UWORD	floor_texture_sizes[];

class	PaintTab	:	public	ModeTab
{
	private:
		UBYTE				CurrentColour;
		SLONG				CurrentTexturePage;
		ULONG				PaintMode,
							TextureFlags;
		SLONG				TextureWidth,
							TextureHeight,
							TextureX,
							TextureY,
							TextureZoom;
		EdRect				ClickRect[4],
							AnimRect,
							PaintRect;
		EdTexture			CurrentTexture;
		SLONG				CurrentAnimTmap;
		SLONG				ShowAnimTmap;


		void				do_undo_me_bloody_self_then(SLONG index);

	public:
							PaintTab(EditorModule *parent);
							~PaintTab();
		void				UpdateTabInfo(void);
		void				DrawTabContent(void);
		void				DrawTexture(void);
		void				DrawStyleTexture(void);
		void				UpdateTexture(void);
		void				UpdateTextureInfo(void);
		void				DrawPalette(void);
		void				DrawAnimTmapSelector(void);
		void				UpdatePalette(void);
		void				UpdatePaletteInfo(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		void				HandlePaletteControl(UWORD control_id);
		void				HandleTextureControl(UWORD control_id);
		void				HandleStyleControl(UWORD control_id);
		void				SelectColour(MFPoint *clicked_point);
		void				SelectTexture(MFPoint *clicked_point);
		void				SelectStyle(MFPoint *clicked_point);
		void				SelectAnimTexture(MFPoint *clicked_point);
		void				PlanarMapping(MFPoint *clicked_point);
		void				DoPlanarMap(void);
		void				DoPlanarMapF(void);
		void				SetEditAnimTexture(MFPoint *clicked_point);
//		UWORD				ConvertFreeToFixedEle(struct TextureBits *t);
		UWORD				ConvertFreeToFixedEle(struct TextureBits *t,SLONG *x,SLONG *y,SLONG *width,SLONG *height,SLONG *page);
		void				ConvertFixedToFree(struct TextureBits *t);
		void				ConvertMiniTex(struct	MiniTextureBits	*tex);
		UWORD				ConvertTexToMiniTex(void);
		void				CutFloorBrush(BuildTab *BuildMode,MFPoint *current_point);

		inline SLONG		GetTexturePage(void)			{	return CurrentTexturePage;	}
		inline void			SetTexturePage(SLONG page)		{	CurrentTexturePage=page;	}
		inline EdTexture	*GetTexture(void)				{	return &CurrentTexture;		}
		inline ULONG		GetTextureFlags(void)			{	return TextureFlags;		}
		inline void			SetTextureFlags(ULONG flags)	{	TextureFlags=flags;			}
		inline ULONG		GetPaintMode(void)				{	return PaintMode;			}
		inline void			SetPaintMode(ULONG mode)		{	PaintMode=mode;				}
		inline UWORD		GetCurrentColour(void)			{	return CurrentColour;		}
		inline void			SetCurrentColour(UWORD col)			{	CurrentColour=col;		}
		inline ULONG		GetAnimTmap(void)				{	return CurrentAnimTmap;		}
		inline void			SetAnimTmap(ULONG v)			{	CurrentAnimTmap=v;			}

		Undo				MyUndo;
		BOOL				ApplyTexture(struct EditFace *edit_face);
		EditorModule		*Parent;
		ControlSet			PaletteSet,
							StyleSet,
							inStyleSet,
							TextureSet;
		SLONG				CurrentTextureRot;
		SLONG				CurrentStyleEdit;
		SLONG				CurrentStylePos;
		SLONG				CurrentInStyleEdit;
		SLONG				CurrentInStylePos;
		SLONG				SubMode,SubStatus;
		MapBlock			CutMapBlock;

};


#endif

