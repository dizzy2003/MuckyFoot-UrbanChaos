//
// Snipe rifle-mode stuff
//

#ifndef _SNIPE_
#define _SNIPE_


//
// The snipe camera.
//

extern SLONG SNIPE_on;			// TRUE => Snipe mode is active
extern SLONG SNIPE_cam_x;
extern SLONG SNIPE_cam_y;
extern SLONG SNIPE_cam_z;
extern SLONG SNIPE_cam_yaw;		// 16-bit fixed
extern SLONG SNIPE_cam_pitch;	// 16-bit fixed
extern SLONG SNIPE_cam_lens;	// 16-bit fixed


//
// Turns on/off snipe mode.
//

void SNIPE_mode_on (SLONG x, SLONG y, SLONG z, SLONG initial_yaw);	// yaw from 0 - 2047
void SNIPE_mode_off(void);

//
// Changes the sniping direction.
//

#define SNIPE_TURN_LEFT  (1 << 0)
#define SNIPE_TURN_RIGHT (1 << 1)
#define SNIPE_TURN_UP    (1 << 2)
#define SNIPE_TURN_DOWN  (1 << 3)

void SNIPE_turn(SLONG turn);

//
// Processes the snipe-view
//

void SNIPE_process(void);


//
// Shoots someone from the sniper view.
//

void SNIPE_shoot(void);


#endif
