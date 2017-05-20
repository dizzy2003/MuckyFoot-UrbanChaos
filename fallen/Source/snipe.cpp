//
// Snipe rifle-mode stuff
//

#ifndef TARGET_DC


#include <MFStdLib.h>
#include "game.h"
#include "guns.h"
#include "snipe.h"
#include "combat.h"

SLONG SNIPE_on;
SLONG SNIPE_yaw;		// 8-bit fixed point
SLONG SNIPE_pitch;		// 8-bit fixed point
SLONG SNIPE_dyaw;
SLONG SNIPE_dpitch;
SLONG SNIPE_dlens;
SLONG SNIPE_cam_x;
SLONG SNIPE_cam_y;
SLONG SNIPE_cam_z;
SLONG SNIPE_cam_yaw;	// 16-bit fixed
SLONG SNIPE_cam_pitch;	// 16-bit fixed
SLONG SNIPE_cam_lens;	// 16-bit fixed

#define SNIPE_INITIAL_PITCH (0)
#define SNIPE_LENS_START    (20 << 16)
#define SNIPE_LENS_END      (30 << 16)

void SNIPE_mode_on(SLONG x, SLONG y, SLONG z, SLONG initial_yaw)	// yaw from 0 - 2047
{
	SNIPE_on    =  TRUE;
	SNIPE_yaw   =  initial_yaw         << 16;
	SNIPE_pitch = -SNIPE_INITIAL_PITCH << 16;

	SNIPE_dyaw   = 0;
	SNIPE_dpitch = 0;
	SNIPE_dlens  = 0;

	SNIPE_cam_x = x;
	SNIPE_cam_y = y;
	SNIPE_cam_z = z;

	SNIPE_cam_yaw   = SNIPE_yaw   & ((2048 << 16) - 1);
	SNIPE_cam_pitch = SNIPE_pitch & ((2048 << 16) - 1);
	SNIPE_cam_lens  = SNIPE_LENS_START;
}

void SNIPE_mode_off()
{
	SNIPE_on = FALSE;
}


void SNIPE_turn(SLONG dir)
{
	#define SNIPE_TURN_SPEED_YAW   (0x6000)
	#define SNIPE_TURN_SPEED_PITCH (0x6000)

	if (dir & SNIPE_TURN_LEFT)  {SNIPE_dyaw += SNIPE_TURN_SPEED_YAW;}
	if (dir & SNIPE_TURN_RIGHT) {SNIPE_dyaw -= SNIPE_TURN_SPEED_YAW;}

	if (dir & SNIPE_TURN_UP)   {SNIPE_dpitch += SNIPE_TURN_SPEED_PITCH;}
	if (dir & SNIPE_TURN_DOWN) {SNIPE_dpitch -= SNIPE_TURN_SPEED_PITCH;}
}


void SNIPE_process()
{
	#define SNIPE_PITCH_MAX (+300 << 16)
	#define SNIPE_PITCH_MIN (-300 << 16)

	if (SNIPE_pitch > SNIPE_PITCH_MAX) {SNIPE_dpitch -= SNIPE_pitch - SNIPE_PITCH_MAX >> 4;}
	if (SNIPE_pitch < SNIPE_PITCH_MIN) {SNIPE_dpitch -= SNIPE_pitch - SNIPE_PITCH_MIN >> 4;}

	SNIPE_pitch += SNIPE_dpitch;
	SNIPE_yaw   += SNIPE_dyaw;

	SNIPE_dyaw   -= SNIPE_dyaw   >> 3;
	SNIPE_dpitch -= SNIPE_dpitch >> 3;

	if (SNIPE_cam_lens < SNIPE_LENS_END)
	{
		SNIPE_dlens += 0x100;
	}

	if (SNIPE_dlens > 0x1000)
	{
		SNIPE_dlens = 0x1000;
	}

	SNIPE_cam_lens  += SNIPE_dlens;
	SNIPE_dlens     -= SNIPE_dlens >> 3;

	if (WITHIN(SNIPE_dpitch, -20, +20)) {SNIPE_dpitch = 0;}
	if (WITHIN(SNIPE_dyaw,   -20, +20)) {SNIPE_dyaw   = 0;}
	if (WITHIN(SNIPE_dlens,  -20, +20)) {SNIPE_dlens  = 0;}

	SNIPE_cam_yaw   = SNIPE_yaw   & ((2048 << 16) - 1);
	SNIPE_cam_pitch = SNIPE_pitch & ((2048 << 16) - 1);
}


void SNIPE_shoot()
{
	THING_INDEX i_target;

	Thing *darci = NET_PERSON(0);
	Thing *p_target;

	darci->Draw.Tweened->Angle  = SNIPE_yaw >> 16;
	darci->Draw.Tweened->Angle += 1024;
	darci->Draw.Tweened->Angle &= 2047;

	i_target = find_snipe_target(darci);

	if (i_target)
	{
		apply_hit_to_person(TO_THING(i_target),0,HIT_TYPE_GUN_SHOT_M,260,darci,0);
	}
}



#endif //#ifndef TARGET_DC



