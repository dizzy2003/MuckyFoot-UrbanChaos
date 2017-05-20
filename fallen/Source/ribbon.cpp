//
// ribbon.cpp
// special effects: smoke, fire, wheelspray... all sorts
// matthew rosenfeld 10 nov 1998
//


#include "game.h"
#include "ribbon.h"
#ifndef	PSX
#include "DrawXtra.h"
#endif

static Ribbon	Ribbons[MAX_RIBBONS];


inline GameCoord Coord(SLONG x, SLONG y, SLONG z) { GameCoord bob = {x,y,z}; return bob; }


// ----------------------------------------------------------------------------------------

#ifndef	PSX
void	RIBBON_draw() {
	SLONG i;
	for (i=0;i<MAX_RIBBONS;i++)
		if ((Ribbons[i].Flags&RIBBON_FLAG_USED)&&(RIBBON_length(i+1)>2))
			RIBBON_draw_ribbon(&Ribbons[i]);
}

// ----------------------------------------------------------------------------------------

void	RIBBON_process() {
	SLONG i,j;

	for (i=0;i<MAX_RIBBONS;i++)
		if (Ribbons[i].Flags & RIBBON_FLAG_USED) {
			Ribbons[i].Scroll+=Ribbons[i].SlideSpeed;
			if (Ribbons[i].Flags & RIBBON_FLAG_CONVECT)
				for (j=0;j<Ribbons[i].Size;j++)
					Ribbons[i].Points[j].Y+=22;
			if (Ribbons[i].Life>0) Ribbons[i].Life--;
			if (!Ribbons[i].Life) RIBBON_free(i);
		}
}


// ----------------------------------------------------------------------------------------

void	RIBBON_init() {
	memset((UBYTE*)Ribbons,0,sizeof(Ribbons));
}

// ----------------------------------------------------------------------------------------

SLONG	RIBBON_alloc(SLONG flags, UBYTE max_segments, SLONG page, SLONG life, UBYTE fade, UBYTE scroll, UBYTE u, UBYTE v, SLONG rgb) 
{
	static SLONG	NextRibbon=0;
	SLONG LastRibbon;

#ifdef	PSX
//	return(0);
#endif
	
	LastRibbon=NextRibbon;
	while (Ribbons[NextRibbon++].Flags) {
		if (NextRibbon==MAX_RIBBONS) NextRibbon=0;
		if (NextRibbon==LastRibbon) return 0;
	}

	if (max_segments>MAX_RIBBON_SIZE) max_segments=MAX_RIBBON_SIZE;
	Ribbons[NextRibbon-1].Size=max_segments;
	Ribbons[NextRibbon-1].Page=page;
	Ribbons[NextRibbon-1].RGB=rgb;
	Ribbons[NextRibbon-1].Life=life;
	Ribbons[NextRibbon-1].Flags=flags|RIBBON_FLAG_USED;
	Ribbons[NextRibbon-1].Scroll=0;
	Ribbons[NextRibbon-1].FadePoint=fade;
	Ribbons[NextRibbon-1].SlideSpeed=scroll;
	if (!v) v=max_segments;
	Ribbons[NextRibbon-1].TextureU=u;
	Ribbons[NextRibbon-1].TextureV=v;

	return NextRibbon;
}

// ----------------------------------------------------------------------------------------

void	RIBBON_free(SLONG ribbon) {
	Ribbons[ribbon].Flags=0;
	Ribbons[ribbon].Head=Ribbons[ribbon].Tail=0;
}

// ----------------------------------------------------------------------------------------

void	RIBBON_extend(SLONG ribbon, SLONG x, SLONG y, SLONG z) {
	ribbon--; // 1 based so that "if (!RIBBON_alloc())" is okay
	if (!(Ribbons[ribbon].Flags & RIBBON_FLAG_USED)) return;

	// Don't extend ribbons if we are paused.
extern SLONG GAMEMENU_menu_type;
	if (GAMEMENU_menu_type != 0/*GAMEMENU_MENU_TYPE_NONE*/)
	{
		return;
	}
	

	Ribbons[ribbon].Points[Ribbons[ribbon].Head] = Coord(x, y, z);
	Ribbons[ribbon].Head++;
	if (Ribbons[ribbon].Head==Ribbons[ribbon].Size)
		Ribbons[ribbon].Head=0;
	if (Ribbons[ribbon].Head==Ribbons[ribbon].Tail) {
		Ribbons[ribbon].Tail++;
		if (Ribbons[ribbon].Tail==Ribbons[ribbon].Size)
			Ribbons[ribbon].Tail=0;
	}
}

// ----------------------------------------------------------------------------------------

SLONG	RIBBON_length(SLONG ribbon) {
	SLONG len;
	ribbon--; // 1 based so that "if (!RIBBON_alloc())" is okay
	if (Ribbons[ribbon].Flags & RIBBON_FLAG_USED) {
		len=Ribbons[ribbon].Head-Ribbons[ribbon].Tail;
		if (len<0) len+=Ribbons[ribbon].Size;
		return len+1;
	}
	return 0;
}

// ----------------------------------------------------------------------------------------


void	RIBBON_life(SLONG ribbon, SLONG life) {
	ribbon--; // 1 based so that "if (!RIBBON_alloc())" is okay
	if (Ribbons[ribbon].Flags & RIBBON_FLAG_USED) {
		Ribbons[ribbon].Life=life;
	}
}

#endif

// ----------------------------------------------------------------------------------------
