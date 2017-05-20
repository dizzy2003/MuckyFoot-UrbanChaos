//
// An 'intelligent' camera?
//

#ifndef _CAM_
#define _CAM_



//
// Set up the camera. Make sure the camera has a sensible focus,
// zoom and mode before you call CAM_process().
//

#define CAM_MODE_CRAPPY			0
#define CAM_MODE_NORMAL			1
#define CAM_MODE_STATIONARY		2
#define CAM_MODE_BEHIND			3
#define CAM_MODE_FIRST_PERSON	4
#define CAM_MODE_THIRD_PERSON	5
#define CAM_MODE_SHOOT_NORMAL	6
#define CAM_MODE_FIGHT_NORMAL	7

#define CAM_LENS_NORMAL		0
#define CAM_LENS_WIDEANGLE	1
#define CAM_LENS_ZOOM		2

extern SLONG CAM_lens;

void CAM_set_mode     (SLONG  mode);
void CAM_set_focus    (Thing *focus);
void CAM_set_zoom     (SLONG  zoom);
void CAM_set_collision(SLONG  boolean);
void CAM_set_behind_up(SLONG  behind_up);
void CAM_set_pos(
		SLONG world_x, 
		SLONG world_y, 
		SLONG world_z);
void CAM_set_angle(
		SLONG yaw,		
		SLONG pitch,
		SLONG roll);

//
// Predefined camera types.
//

#define CAM_TYPE_STANDARD	1
#define CAM_TYPE_LOWER		2
#define CAM_TYPE_BEHIND		3
#define CAM_TYPE_WIDE		4
#define CAM_TYPE_ZOOM		5

void  CAM_set_type(SLONG type);		// Sets up the mode, focus and zoom stuff to the given type.
SLONG CAM_get_type(void);			// Returns either the last type set or NULL if no type has been set.

//
// Gives the camera the shakes...
//

void CAM_set_shake(UBYTE amount);

//
// In first person mode.
//

void CAM_set_dangle(SLONG  dyaw, SLONG  dpitch);
void CAM_get_dangle(SLONG *dyaw, SLONG *dpitch);


//
// Returns the current camera mode.
//

SLONG CAM_get_mode(void);
SLONG CAM_get_zoom(void);

//
// Processes the camera.
//

void CAM_process(void);

//
// In normal mode, these functions tell the camera to rotate left
// and right about the focus thing.
//

void CAM_rotate_left (void);
void CAM_rotate_right(void);


//
// Returns where the camera is.
//

float CAM_get_ryaw(void);

void CAM_get(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll,
		float *radians_yaw,
		float *radians_pitch,
		float *radians_roll);

void CAM_get_psx(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z,
		SLONG *yaw,
		SLONG *pitch,
		SLONG *roll,
		SLONG *radians_yaw,
		SLONG *radians_pitch,
		SLONG *radians_roll);

void CAM_get_pos(
		SLONG *world_x,
		SLONG *world_y,
		SLONG *world_z);

//
// Camera position relative to the focus thing.
//

void CAM_get_dpos(
		SLONG *dpos_x,
		SLONG *dpos_y,
		SLONG *dpos_z,
		SLONG *yaw,
		SLONG *pitch);

void CAM_set_dpos(
		SLONG dpos_x,
		SLONG dpos_y,
		SLONG dpos_z,
		SLONG yaw,
		SLONG pitch);


//
// Puts the camera in a good position to watch the given
// person climb out of the sewers.
//

void CAM_set_to_leave_sewers_position(Thing *);



//
// Sets the camera to look at the focus thing.
// 

void CAM_look_at_thing(SLONG swoop);


#endif


