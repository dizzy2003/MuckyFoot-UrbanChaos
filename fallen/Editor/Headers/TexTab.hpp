// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_TEXTAB_HPP_
#define	_TEXTAB_HPP_

#include	"undo.hpp"
#include	"ModeTab.hpp"
#include	"Stealth.h"



#define	FLAGS_SHOW_TEXTURE	(1<<0)
#define	FLAGS_QUADS			(1<<1)
#define	FLAGS_FIXED			(1<<2)


class	TextureTab	:	public	ModeTab
{
	private:
		ULONG				CurrentTexturePage,
							TextureFlags;

		SLONG				TextureWidth,
							TextureHeight,
							TextureX,
							TextureY,
							TextureZoom;
		EditorModule		*Parent;
		EdRect				ClickRect[4],
							TextureRect;
		EdTexture			CurrentTexture;

		void				do_undo_me_bloody_self_then(SLONG index);
	public:
							TextureTab(EditorModule *parent);
		void				DrawTabContent(void);
		void				DrawTexture(void);
		void				UpdateTexture(void);
		void				UpdateTextureInfo(void);
		void				HandleTab(MFPoint *current_point);
		UWORD				HandleTabClick(UBYTE flags,MFPoint *clicked_point);
		void				HandleControl(UWORD control_id);
		UWORD				ConvertFreeToFixedEle(struct TextureBits *t);
		void				ConvertFixedToFree(struct TextureBits *t);

		inline ULONG		GetTexturePage(void)			{	return CurrentTexturePage;	}
		inline void		SetTexturePage(ULONG page)		{	CurrentTexturePage=page;	}
		inline EdTexture	*GetTexture(void)				{	return &CurrentTexture;		}
		inline ULONG		GetTextureFlags(void)			{	return TextureFlags;		}
		inline void			SetTextureFlags(ULONG flags)	{	TextureFlags=flags;			}
		Undo				MyUndo;
		BOOL				ApplyTexture(struct EditFace *edit_face);
};


#endif

