//
// psystem.h
//
// Proper particle system for handling a wide variety of effects:
// smoke, dust, mud, sparks, blood...
//

#ifndef _PSYSTEM_H_
#define _PSYSTEM_H_

#ifndef PSX
#define PSYSTEM_MAX_PARTICLES	2048
#else
#define PSYSTEM_MAX_PARTICLES	64
#endif

#include "MFStdLib.h"
#include "game.h"
#include "thing.h"

// standard grav physics:
#define PFLAG_GRAVITY	1
// depth-pressure-linked bouyancy
#define PFLAG_BOUYANT	2
// fade alpha out over time:
#define PFLAG_FADE		4
// fade colour through fire palette over time:
#define PFLAG_FIRE		8
// bounce on collision with walls (else destroyed)
#define PFLAG_COLLIDE	16
// bounce on collision with floors
#define PFLAG_BOUNCE	32
// shrink/expand over time
#define PFLAG_RESIZE	64
// (could use 32+64 as something else)
// wander randomly (smoke etc)
#define PFLAG_WANDER	128
// enhanced wander, bigger range, kinda... er... drifty. theoretically.
#define PFLAG_DRIFT 	256
// animate thru the sprites
#define PFLAG_SPRITEANI	512
// invert the alpha
#define PFLAG_INVALPHA	1024
// shrink/expand over time, "hi res"
#define PFLAG_RESIZE2	2048
// loop the the sprites animation
#define PFLAG_SPRITELOOP 4096
// another crap fadey thing
#define PFLAG_FADE2		 8192
// damp out x/z movement
#define PFLAG_DAMPING	 16384
// hurts people it touches
#define PFLAG_HURTPEOPLE 32768
// explodes on impact
#define PFLAG_EXPLODE_ON_IMPACT 65536
// leaves a smoke trail
#define PFLAG_LEAVE_TRAIL 131072

// macros for commonly used sets
#define PFLAGS_SMOKE	PFLAG_FADE|PFLAG_RESIZE|PFLAG_WANDER
#define PFLAGS_BUBBLE	PFLAG_BOUYANT|PFLAG_EXPAND

struct Particle {
	SLONG	x,y,z;
	SLONG	colour, flags, life;
	UWORD	page, sprite;
	UBYTE	padding, priority;
	SBYTE	fade, resize;
	SWORD	dx,dy,dz;
	UWORD	prev, next;
	UWORD	size;
};


void PARTICLE_Run();
void PARTICLE_Draw();
void PARTICLE_Reset();
UWORD PARTICLE_AddParticle(Particle &p);
UWORD PARTICLE_Add(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, UWORD page, UWORD sprite, SLONG colour, SLONG flags, SLONG life, UBYTE size, UBYTE priority, SBYTE fade, SBYTE resize);
	
// Some of the more commonly-used effects:
UWORD PARTICLE_Exhaust(SLONG x, SLONG y, SLONG z,UBYTE density, UBYTE disperse);
UWORD PARTICLE_Exhaust2(Thing *object, UBYTE density, UBYTE disperse);
UWORD PARTICLE_Steam(SLONG x, SLONG y, SLONG z, UBYTE axis, SLONG vel, SLONG range, UBYTE time);
UWORD PARTICLE_SGrenade(Thing *object, UBYTE time);

#endif

