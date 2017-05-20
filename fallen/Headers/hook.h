//
// A grappling hook.
//

#ifndef _HOOK_
#define _HOOK_



//
// Creates the grappling hook lying neatly coiled up on the ground.
//

void HOOK_init(
		SLONG x,
		SLONG z);


//
// The current state of the hook.
//

#define HOOK_STATE_STILL	0
#define HOOK_STATE_SPINNING	1
#define HOOK_STATE_FLYING	2

SLONG HOOK_get_state(void);


//
// Starts spinning the hook about the given point.
// Call this function again to change where the hook spins
// about or the speed with which it spins.
//

void HOOK_spin(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG speed_or_minus_pitch);	// < 0 => it is the actual pitch of the grappling hook.

//
// Releases the hook from spinning.
//

void HOOK_release(void);


//
// Processes the hook.
//

void HOOK_process(void);


//
// Reels in the hook.
//

#define HOOK_REELED_CONTINUE	0
#define HOOK_REELED_IN			1
#define HOOK_REELED_TAUT		2

SLONG HOOK_reel(void);





// ========================================================
//
// DRAWING THE HOOK.
//
// ========================================================

//
// The position of the points returned is high-res. There are 16-bits
// of precision per mapsquare.
// 

#define HOOK_NUM_POINTS 256

void  HOOK_pos_grapple(
		SLONG *x,
		SLONG *y,
		SLONG *z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll);
void  HOOK_pos_point(SLONG point,
		SLONG *x,
		SLONG *y,
		SLONG *z);


#endif
