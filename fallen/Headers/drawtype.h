#ifndef	DRAWTYPE_H
#define	DRAWTYPE_H	
//
// Draw Types
//

#include "cache.h"

#define RMAX_DRAW_TWEENS		RMAX_PEOPLE+30
#define RMAX_DRAW_MESHES		320 //500

#define MAX_DRAW_TWEENS		(save_table[SAVE_TABLE_DTWEEN].Maximum)
#define MAX_DRAW_MESHES		(save_table[SAVE_TABLE_DMESH].Maximum)

#define	DT_NONE				0
#define	DT_BUILDING			1
#define	DT_PRIM				2
#define	DT_MULTI_PRIM		3
#define	DT_ROT_MULTI		4
#define	DT_EFFECT			5
#define DT_MESH				6
#define DT_TWEEN			7
#define	DT_SPRITE			8
#define DT_VEHICLE			9
#define DT_ANIM_PRIM		10
#define DT_CHOPPER			11
#define DT_PYRO				12
#define DT_ANIMAL_PRIM		13
#define	DT_TRACK			14
#define DT_BIKE				15

#define	DT_FLAG_UNUSED		(1<<7)
#define	DT_FLAG_GUNFLASH	(1<<6)


//
// structs
//

typedef	struct
{
	UBYTE				TweakSpeed;
	SBYTE				Locked;     //which frame if any is locked in place
	UBYTE				FrameIndex;
	UBYTE				QueuedFrameIndex;
	
	SWORD				Angle,AngleTo,
						Roll,DRoll,
						Tilt,TiltTo;

	SLONG				CurrentAnim,
						AnimTween,
						TweenStage;
	struct	GameKeyFrame			*CurrentFrame,
						*NextFrame,
						*InterruptFrame,
						*QueuedFrame;
	struct	GameKeyFrameChunk		*TheChunk;

	UBYTE				Flags;
	UBYTE				Drawn;  //game turn last drawn
	UBYTE				MeshID;
	UBYTE				PersonID;
}DrawTween;

typedef	struct
{
	UWORD				Angle;
	UWORD				Roll;
	UWORD				Tilt;
	UWORD				ObjectId;
	CACHE_Index			Cache;  //ubyte
	UBYTE				Hm;		// 255 => NULL

}DrawMesh;




//
// Functions
//

void		init_draw_tweens(void);
DrawTween	*alloc_draw_tween(SLONG type);
void		free_draw_tween(DrawTween *draw_tween);


//
// DrawMesh functions.
//

void      init_draw_meshes(void);
DrawMesh *alloc_draw_mesh (void);
void      free_draw_mesh  (DrawMesh *drawmesh);






#endif
