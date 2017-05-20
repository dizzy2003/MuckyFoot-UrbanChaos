//
// Pyrotechnics
// Burning (and maybe Exploding) Things
//

#ifndef PYRO_H
#define PYRO_H

#ifdef PSX
#define MAX_PYROS	32 //64
#else
#ifdef	MIKE
#define MAX_PYROS	32
#else
#define MAX_PYROS	64
#endif
#endif

#define PYRO_NONE		0   // zilch, nada
#define PYRO_FIREWALL	1   // a wall of fire along a ground vector
#define PYRO_FIREPOOL	2	// A burning radius
#define	PYRO_BONFIRE	3	// A small pyre, like the old burning puddles
#define PYRO_IMMOLATE   4	// toast another Thing. muhaha.
#define PYRO_EXPLODE	5	// A massive kerblam
#define	PYRO_DUSTWAVE	6	// A sort of volume shockwave
#define PYRO_EXPLODE2	7	// A new massive kerblam. maybe.
#define	PYRO_STREAMER	8	// Set of four smoke trails. Perfect accompanyment to any explosion
#define PYRO_TWANGER	9	// Beams of light that sorta twang in at the start of a big bang
#define PYRO_FLICKER	10	// Flickering flames using ribbon engine
#define PYRO_HITSPANG	11	// Spang for when stuff gets shot
#define PYRO_FIREBOMB   12	// New firey explosions
#define PYRO_SPLATTERY	13	// A what the hell. Blood splats.
#define PYRO_IRONICWATERFALL 14 // it's a waterfall. as a pyro. _i_ think that's ironic. sorta.
#define PYRO_NEWDOME	15  // A pyro explosion dome with sprite boundaries
#define PYRO_WHOOMPH	16  // What happens when a Balrog fireball hits the ground
#define PYRO_GAMEOVER	17  // It's The End Of The World As We Know It, And I Feel Fine

#define PYRO_RANGE      18

#define PYRO_FLAGS_INSTANT		1		// entire area lights instantly
#define PYRO_FLAGS_WAVE			2		// flamewave - moving flame
#define PYRO_FLAGS_INFINITE		4		// keep burning forever (else die after a while)
#define PYRO_FLAGS_TOUCHPOINT	8		// pools being lit from a point other than the centre

// immolation flags, mutually exclusive to the rest
#define PYRO_FLAGS_FLICKER		1		// all-over flickering
#define	PYRO_FLAGS_BONFIRE		2		// all-over spritey crap
#define PYRO_FLAGS_FLAME		4		// spritey rising bits
#define PYRO_FLAGS_SMOKE		8		// thick bonfire smoke
#define PYRO_FLAGS_STATIC		16		// not attached to a moving object

typedef struct
{
	UWORD		Flags;			// pyro flags
	UBYTE		PyroType;		// firewall, bonfire, etc
	UBYTE		dlight;			// dynamic light as appropriate
	Thing*		thing;			// points at its thing. ooer.
	Thing*		victim;			// to be immolated
	GameCoord	target;			// for fire walls; or touchpoint of pools?
	SLONG		radius;			// for area-effect fire
	SLONG		counter;		// various timings
	ULONG		radii[8];		// for firepool base
	SLONG		scale;			// size adjustment
	SLONG		tints[2];		// for flarey things
	SLONG		soundid;		// might make this a byte index later
	SWORD		Timer1;			//was in thing
	UWORD		Dummy;
}Pyro;

typedef struct
{
	SLONG		x,y,z,flags;
}RadPoint;

typedef struct
{
	UWORD		radius, delta;
}PyrPoint;

typedef struct
{
	UWORD		Flags;			// pyro flags
	UBYTE		PyroType;		// firewall, bonfire, etc
	UBYTE		padding;
	Thing*		thing;			// points at its thing. ooer.
	PyrPoint	points[17];		// 8 bottom rung, 8 top run, 1 centre
}Pyrex;

typedef Pyro* PyroPtr;

void   init_pyros(void);
struct Thing *alloc_pyro(UBYTE type);
//void   free_pyro (struct Thing *pyro_thing);


//
// The pyro state functions.
//

extern GenusFunctions PYRO_functions[PYRO_RANGE];

extern StateFunction BONFIRE_state_function[];
extern StateFunction IMMOLATE_state_function[];
extern StateFunction EXPLODE_state_function[];
extern StateFunction DUSTWAVE_state_function[];
extern StateFunction EXPLODE2_state_function[];
extern StateFunction STREAMER_state_function[];
extern StateFunction TWANGER_state_function[];
extern StateFunction FIREBOMB_state_function[];
extern StateFunction IRONICWATERFALL_state_function[];
extern StateFunction NEWDOME_state_function[];
extern StateFunction PYRO_state_function[]; // generic

void PYRO_init_state(Thing *pyro_thing, UBYTE new_state);

//
// Creates a pyro thing of the given type.  It puts it at the
// given position on the mapwho and puts it into state STATE_INIT.
//

Thing *PYRO_create(GameCoord pos, UBYTE type);

//
// Creates one or more pyro things according to a set of flags
//

void PYRO_construct(GameCoord posn, SLONG types, SLONG scale);

//
// Creates a hitspang from the shooter to the given target
// Versions according to target type (person or coordinate)
//

void PYRO_hitspang(Thing *p_person, Thing *victim);
void PYRO_hitspang(Thing *p_person, SLONG x, SLONG y, SLONG z);

//
// Returns the Pyro structure associated with the given pyro thing.
//

Pyro  *PYRO_get_pyro (Thing *pyro_thing);

// blast radius

void PYRO_blast_radius(SLONG x, SLONG y, SLONG z, SLONG radius, SLONG strength);


#endif
