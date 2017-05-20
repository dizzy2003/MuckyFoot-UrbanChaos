// Sprites.h
// Guy Simmons, 13th February 1997.

#ifndef	_SPRITES_H_
#define	_SPRITES_H_

#ifndef	_MF_TYPES_H_
	#include	<MFTypes.h>
#endif

#define	END_LINE				0x00
#define	COPY_PIXELS				0x01
#define	SKIP_PIXELS				0x02
#define	DUPLICATE_PIXELS		0x03
#define	FINISHED				0x04


typedef struct
{
	UBYTE		*SpriteData;
	UWORD		SpriteHeight;
	UWORD		SpriteWidth;
}BSprite;


extern	void	(*DrawBSprite)(SLONG x,SLONG y,BSprite *the_sprite);
extern	void	(*DrawBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite);

extern	void	DrawBSpritePal16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
extern	void	DrawBSpritePal32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);

extern	void	(*DrawMonoBSprite)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);
extern	void	(*DrawMonoBSpriteC)(SLONG x,SLONG y,BSprite *the_sprite,ULONG colour);


extern	void	DrawBSpritePalC16(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);
extern	void	DrawBSpritePalC32(SLONG x,SLONG y,BSprite *the_sprite,UBYTE *pal);

extern	void	SetupBSprites(BSprite *sprite_ref,UBYTE *sprite_data);

#endif
