// TexTab.hpp
// Guy Simmons, 20th February 1997

#ifndef	_UNDO_HPP_
#define	_UNDO_HPP_

#include	"Prim.h"
#define	MAX_UNDO	100


#define	UNDO_APPLY_TEXTURE_PRIM4	1
#define	UNDO_APPLY_TEXTURE_PRIM3	2
#define	UNDO_APPLY_TEXTURE_CUBE		3
#define	UNDO_PLACE_OBJECT 			4
#define	UNDO_DEL_OBJECT				5
#define	UNDO_MOVE_OBJECT			6
#define	UNDO_PLACE_CUBE				7
#define	UNDO_MOVE_TEXTURE			8
#define	UNDO_APPLY_PRIM4			9
#define	UNDO_APPLY_PRIM3			10


struct	GenericUndo
{
	UBYTE	Type;
	union
	{
		struct	
		{
			UBYTE	DrawFlags;
			UBYTE	Colour;
			UWORD	Face;
			UWORD	Page;
			UBYTE	U[4];
			UBYTE	V[4];
		}Texture;
		struct	
		{
			UWORD	Thing;
			UWORD	Prim;
			SLONG 	X;
			SLONG	Y;
			SLONG	Z;
		}Object;
		struct	
		{
			UWORD	PCube;
			UWORD	CCube;
			SLONG 	X;
			SLONG	Y;
			SLONG	Z;
		}Cube;
		struct	
		{
			UWORD	Ele;
			UWORD	Face;
			UWORD	Text1;
			UWORD	Text2;
		}Ele;
	};
};


class	Undo
{
	private:
			void		advance_current_undo(UBYTE undo_mode);
			void		retreat_current_undo(UBYTE undo_mode);
			SWORD	index;
			SWORD	index_undo;
	public:
					Undo(void);
			void	ApplyPrim4(UBYTE undo_mode,UWORD face,PrimFace4 *the_prim4);
			void	ApplyPrim3(UBYTE undo_mode,UWORD face,PrimFace3 *the_prim3);
			void	ApplyTexturePrim4(UBYTE undo_mode,UWORD page,UWORD face,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2,UBYTE u3,UBYTE v3,UBYTE u4,UBYTE v4);
			void	ApplyTexturePrim3(UBYTE undo_mode,UWORD page,UWORD face,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2,UBYTE u3,UBYTE v3);
			void	ApplyTextureCube(UBYTE undo_mode,UWORD ele,UWORD face,UWORD text1,UWORD text2);
			void	MoveTexture(UBYTE undo_mode,UWORD page,UWORD face,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2,UBYTE u3,UBYTE v3,UBYTE u4,UBYTE v4);
			void	PlaceObject(UBYTE undo_mode,UWORD prim,UWORD thing,SLONG x,SLONG y,SLONG z);
			void	MoveObject(UBYTE undo_mode,UWORD thing,SLONG dx,SLONG dy,SLONG dz);
			void	DelObject(UBYTE undo_mode,UWORD prim,UWORD thing,SLONG x,SLONG y,SLONG z);
			void	PlaceCube(UBYTE undo_mode,UWORD prev_cube,UWORD cur_cube,SLONG x,SLONG y,SLONG z);
			SLONG	DoUndo(UBYTE undo_mode);

			GenericUndo	undo_info[MAX_UNDO];
			GenericUndo	undo_undo_info[MAX_UNDO];
};

#endif

