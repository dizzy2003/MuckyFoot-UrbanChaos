//
// The orbs
//

#ifndef _ORB_
#define _ORB_



//
// The size of the arena the orbs are contained in.
//

#define ORB_ARENA (32.0F)


//
// The orbs
//

#define ORB_FLAG_USED     (1 << 0)
#define ORB_FLAG_COLLIDED (1 << 1)	// Collided with the landscape last process

typedef struct
{
	ULONG flag;
	float x;
	float y;
	float mass;
	float radius;
	float dx;
	float dy;

} ORB_Orb;

#define ORB_MAX_ORBS 16

extern ORB_Orb ORB_orb[ORB_MAX_ORBS];


//
// Init the orbs.
//

void ORB_init(void);


//
// Create a new orb. Returns NULL on failure.
//

ORB_Orb *ORB_create(float x, float y, float radius);


//
// Processes one gameturn of all the orbs.
//

void ORB_process_all(void);


//
// Draws all the orbs.
//

void ORB_draw_all(float mid_x, float mid_y, float zoom);



#endif

