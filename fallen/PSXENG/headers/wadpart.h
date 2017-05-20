// wadpart.h
//
// Particle system for Wadmenu, this provides animating elements for
// the frontend.
//

#ifndef _WADPART_H
#define _WADPART_H

#define WADPART_MAXPARTICLES	96

typedef struct {
	SLONG vx,vy;
} PVECTOR;

typedef struct {
	UBYTE	used;			// Is this particle used
	UBYTE	type;			// Type of particle
	UWORD	life;			// Life time of particle (in frames)
	SLONG	value;			// String ID, Texture Page or Texture ID
	SLONG	scale;			// Scale (*256)
	SLONG	colour;			// Colour of particle
	SLONG	flags;			// Flags (including render flags for type)
	PVECTOR velocity;		// Speed of particle movement (*256)
	PVECTOR location;		// Location of particle (*256)
} W_Particle;

#define WADPART_TYPE_TEXT	0
#define WADPART_TYPE_IMAGE	1
#define WADPART_TYPE_CHAR	2
#define WADPART_TYPE_LEAF	3
#define WADPART_TYPE_BLOOD	4
#define WADPART_TYPE_RAIN	5
#define WADPART_TYPE_ANIM	6
#define WADPART_TYPE_BOARD	7
#define WADPART_TYPE_RECT	8
#define WADPART_TYPE_STRING 9

#define WADPART_FLAG_NORMAL		0x00000000
#define WADPART_FLAG_ACCEL		0x00000100
#define WADPART_FLAG_DECEL		0x00000200
#define WADPART_FLAG_GRAVITY	0x00000400
#define WADPART_FLAG_FADE		0x00000800
#define WADPART_FLAG_AMBIENT	0x00001000
#define WADPART_FLAG_FADEIN		0x00002000
#define WADPART_FLAG_FLUTTER	0x00004000
#define WADPART_FLAG_PULSE		0x00008000
#define WADPART_FLAG_WIND		0x00010000
#define WADPART_FLAG_ANIMATED	0x00020000
#define WADPART_FLAG_EXPAND		0x00040000
#define WADPART_FLAG_SHRINK		0x00080000

#define WADPART_FLAG_LEAF		(WADPART_FLAG_GRAVITY|WADPART_FLAG_FLUTTER|WADPART_FLAG_AMBIENT|WADPART_FLAG_WIND)
#define WADPART_FLAG_SNOW		(WADPART_FLAG_GRAVITY|WADPART_FLAG_FLUTTER|WADPART_FLAG_AMBIENT|WADPART_FLAG_PULSE)
#define WADPART_FLAG_BLOOD		(WADPART_FLAG_ACCEL|WADPART_FLAG_AMBIENT)
#define WADPART_FLAG_RAIN		(WADPART_FLAG_GRAVITY|WADPART_FLAG_AMBIENT)
#define WADPART_FLAG_SPLASH		(WADPART_FLAG_ANIMATED|WADPART_FLAG_AMBIENT)
#define WADPART_FLAG_RIPPLE		(WADPART_FLAG_EXPAND|WADPART_FLAG_FADE|WADPART_FLAG_AMBIENT)

#define WADPART_PAGE_LEAF		454

extern W_Particle Wadpart_Particle[WADPART_MAXPARTICLES];

extern void Wadpart_Init();
extern SLONG Wadpart_Sync();
extern void Wadpart_AddTextParticle(SLONG text,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags);
extern void Wadpart_AddStringParticle(char *text,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags);
extern void Wadpart_AddImageParticle(SLONG image,SLONG colour,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags);
extern void Wadpart_AddCharExplode(SLONG text,SLONG colour,SLONG x,SLONG y,SLONG life,SLONG flags);
extern void Wadpart_AddRainParticle(SLONG image,SLONG x,SLONG y,SLONG vx,SLONG vy,SLONG scale,SLONG life,SLONG flags);
extern void Wadpart_Render(void);
extern void Wadpart_AddLeafParticle(SLONG image,SLONG colour,SLONG x,SLONG y,SLONG scale,SLONG flags);
extern void Wadpart_AddBloodParticle(SLONG x,SLONG y,SLONG vx,SLONG vy,SLONG scale,SLONG flags);
extern void Wadpart_AddBoardParticle(SLONG image,SLONG x,SLONG y,SLONG dx,SLONG dy,SLONG life,SLONG flags);
extern void Wadpart_AddRectParticle(SLONG x,SLONG y,SLONG w,SLONG h,SLONG colour,SLONG flags);


#endif