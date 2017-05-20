//
// Yet another explosion system.
//

#ifndef _POW_
#define _POW_



//
// A single element of an explosion.
//

typedef struct
{
	UBYTE next;
	UBYTE frame;			// Which frame of animation it is on...
	UBYTE frame_speed;		// How fast it moves between frames.
	UBYTE damp;				// How damped the motion is.
	SLONG x;
	SLONG y;
	SLONG z;
	SWORD dx;
	SWORD dy;
	SWORD dz;
	UWORD frame_counter;	// Countdown to going onto next frame.
 
} POW_Sprite;

#ifdef TARGET_DC

#define POW_MAX_SPRITES 128

#else

#ifndef PSX
#define POW_MAX_SPRITES 256
#else
#define POW_MAX_SPRITES 192
#endif
#endif

extern POW_Sprite POW_sprite[POW_MAX_SPRITES];
extern UBYTE      POW_sprite_free;


//
// An explosion.
//

typedef struct
{
	UBYTE type;
	UBYTE next;
	UBYTE sprite;	// Index into the linked list of sprites for this POW
	UBYTE mapwho;
	SLONG x;
	SLONG y;
	SLONG z;
	SWORD dx;
	SWORD dy;
	SWORD dz;
	UWORD timer;
	UBYTE flag;
	UBYTE time_warp;
	UWORD padding;
 
} POW_Pow;

#ifdef TARGET_DC

#define POW_MAX_POWS 16

#else

#ifndef PSX
#define POW_MAX_POWS 32
#else
#define POW_MAX_POWS 24
#endif

#endif

extern POW_Pow POW_pow[POW_MAX_POWS];
extern UBYTE   POW_pow_free;
extern UBYTE   POW_pow_used;


//
// The POW 1D mapwho- thats only 32 bytes!
//

extern UBYTE POW_mapwho[PAP_SIZE_LO];




//
// Initialises the explosions.
//

void POW_init(void);


//
// Sets off an explosion.
// 
#define POW_TYPE_UNUSED				 0
#define POW_TYPE_BASIC_SPHERE_LARGE  1
#define POW_TYPE_BASIC_SPHERE_MEDIUM 2
#define POW_TYPE_BASIC_SPHERE_SMALL  3
#define POW_TYPE_SPAWN_SPHERE_LARGE	 4
#define POW_TYPE_SPAWN_SPHERE_MEDIUM 5
#define POW_TYPE_MULTISPAWN			 6
#define POW_TYPE_MULTISPAWN_SEMI	 7
#define POW_TYPE_NUMBER              8

#define POW_CREATE_LARGE_SEMI 0
#define POW_CREATE_MEDIUM     1
#define POW_CREATE_LARGE      2

//
// Creates a high-level explosion.
// 

void POW_new(SLONG type, SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz);

inline	void POW_create(
		SLONG which,
		SLONG x,	// 16-bits per mapsquare
		SLONG y,
		SLONG z,
		SLONG dx,	// 16-bits per mapsquare
		SLONG dy,
		SLONG dz)
{
	switch(which)
	{
		case POW_CREATE_LARGE_SEMI: POW_new(POW_TYPE_MULTISPAWN_SEMI,     x,y,z, dx,dy,dz); break;
		case POW_CREATE_MEDIUM:     POW_new(POW_TYPE_SPAWN_SPHERE_MEDIUM, x,y,z, dx,dy,dz); break;
		case POW_CREATE_LARGE:      POW_new(POW_TYPE_MULTISPAWN,          x,y,z, dx,dy,dz); break;

		default:
			ASSERT(0);
			break;
	}
}
/*
void POW_create(
		SLONG which,
		SLONG x,		// 16-bits per mapsquare
		SLONG y,
		SLONG z,
		SLONG dx = 0,	// 16-bits per mapsquare
		SLONG dy = 0,
		SLONG dz = 0);
*/


//
// Processes the explosions.
//

void POW_process(void);






#endif
