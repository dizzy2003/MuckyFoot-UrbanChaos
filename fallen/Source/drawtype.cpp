// drawtype.cpp
// Mike Diskett, 15th January 1998.

#include	"Game.h"
#include "memory.h"


//---------------------------------------------------------------

void	init_draw_tweens(void)
{
	SLONG	c0;
	memset((UBYTE*)DRAW_TWEENS,0,sizeof(DrawTween)*MAX_DRAW_TWEENS);
	DRAW_TWEEN_COUNT	=	0;
	for(c0=0;c0<MAX_DRAW_TWEENS;c0++)
	{
		DRAW_TWEENS[c0].Flags|=DT_FLAG_UNUSED;

	}
}

//---------------------------------------------------------------

DrawTween	*alloc_draw_tween(SLONG type)
{
	SLONG			c0;
	DrawTween		*new_draw=0;


	// Run through the camera array & find an unused one.
	for(c0=0;c0<MAX_DRAW_TWEENS;c0++)
	{
		if(DRAW_TWEENS[c0].Flags&DT_FLAG_UNUSED)
		{
			new_draw = TO_DRAW_TWEEN(c0);
			new_draw->Flags&=~DT_FLAG_UNUSED;
//			ASSERT(c0);



#ifdef DEBUG
			// Dump some info out, so I can set up levels correctly.
			static int iLowestCount = 100000;
SLONG count_draw_tween ( void );
			int iCount = count_draw_tween();
			if ( iLowestCount > iCount )
			{
				iLowestCount = iCount;
			}
			TRACE ( "Now only %i tweens left out of %i, lowest was %i\n", iCount, MAX_DRAW_TWEENS, iLowestCount );
#endif


			return(new_draw);
		}
	}
	ASSERT(0);
	return(0);
}

SLONG	count_draw_tween(void)
{
	SLONG			c0;
	SLONG	count=0;


	// Run through the camera array & find an unused one.
	for(c0=0;c0<MAX_DRAW_TWEENS;c0++)
	{
		if(DRAW_TWEENS[c0].Flags&DT_FLAG_UNUSED)
		{
			count++;
		}
	}
	return(count);
}

//---------------------------------------------------------------

void	free_draw_tween(DrawTween *draw_tween)
{
	// Set the camera type to none & free the thing.
//	draw_tween->LDrawType	=	DT_NONE;
	memset((UBYTE*)draw_tween,0,sizeof(DrawTween));
	draw_tween->Flags|=DT_FLAG_UNUSED;


//	ASSERT(0);



}

#define DRAW_MESH_NULL_ANGLE (0xfafa)

//
// DrawMesh functions.
//
#ifndef PSX
void init_draw_meshes(void)
{
	SLONG i;


	//
	// if the angle of a drawmesh is DRAW_MESH_NULL_ANGLE then
	// that drawmesh is unused.
	//

	for (i = 0; i < MAX_DRAW_MESHES; i++)
	{
		DRAW_MESHES[i].Angle = DRAW_MESH_NULL_ANGLE;
	}

	DRAW_MESH_COUNT = 0;
}
#endif

DrawMesh *alloc_draw_mesh(void)
{
	SLONG i;

	DrawMesh *ans;

	ASSERT(DRAW_MESH_COUNT < MAX_DRAW_MESHES);

	for (i = 0; i < MAX_DRAW_MESHES; i++)
	{
		if (DRAW_MESHES[i].Angle == DRAW_MESH_NULL_ANGLE)
		{
			//
			// This is an unused DrawMesh.
			//

			ans = &DRAW_MESHES[i];

			//
			// Mark it as used.
			//

			ans->Angle = 0;
			ans->Cache = NULL;
			ans->Hm    = 255;	// 255 means no Hm.

			return ans;
		}
	}

//	ASSERT(0);

	return NULL;
}

void free_draw_mesh(DrawMesh *drawmesh)
{
	ASSERT(WITHIN(drawmesh, &DRAW_MESHES[0], &DRAW_MESHES[MAX_DRAW_MESHES - 1]));

	//
	// Mark the DrawMesh as used.
	//

	drawmesh->Angle = DRAW_MESH_NULL_ANGLE;
}


