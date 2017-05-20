//
// The final camera?
//

#ifndef _FC_
#define _FC_




//
// The camera.
//

typedef struct
{
	//
	// What the camera is looking at.
	//

	Thing *focus;			// NULL => Camera is not used.
	SLONG  focus_x;
	SLONG  focus_y;
	SLONG  focus_z;
	SLONG  focus_yaw;
	UBYTE  focus_in_warehouse;

	SLONG x;
	SLONG y;
	SLONG z;
	SLONG want_x;
	SLONG want_y;
	SLONG want_z;
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG yaw;
	SLONG pitch;
	SLONG roll;
	SLONG want_yaw;
	SLONG want_pitch;
	SLONG want_roll;
	SLONG lens;
	SLONG toonear;
	SLONG rotate;
	SLONG nobehind;
	SLONG lookabove;
	UBYTE shake;
	SLONG cam_dist;
	SLONG cam_height;
	SLONG toonear_dist;
	SLONG toonear_x;
	SLONG toonear_y;
	SLONG toonear_z;
	SLONG toonear_yaw;
	SLONG toonear_pitch;
	SLONG toonear_roll;
	SLONG toonear_focus_yaw;
	SLONG smooth_transition;

} FC_Cam;

#define FC_MAX_CAMS 2

extern FC_Cam FC_cam[FC_MAX_CAMS];


//
// Creates nice NULL cameras.
//

void FC_init(void);


//
// Sets up the camera.
//

void FC_look_at(SLONG cam, UWORD thing_index);
void FC_move_to(SLONG cam, SLONG world_x, SLONG world_y, SLONG world_z);	// 8-bits per mapsquare.
void FC_change_camera_type(SLONG cam,SLONG cam_type);


//
// Rotation about the focus thing.
// 

void FC_rotate_left (SLONG cam);
void FC_rotate_right(SLONG cam);
void FC_kill_player_cam(Thing *p_thing);


//
// Processes the cameras.
//

void FC_process(void);


//
// A fast LOS reject. Can the camera see this person?
//

SLONG FC_can_see_person(SLONG cam, Thing *p_person);


//
// Positions the camera over the shoulder of the focus thing and
// at the given pitch.
//

void FC_position_for_lookaround(SLONG cam, SLONG pitch);


//
// Forces the camera to behind the player.
//

void FC_force_camera_behind(SLONG cam);


//
// Initialises the camera for the start of the game.
// Make sure you have already call FC_look_at()
//

void FC_setup_initial_camera(SLONG cam);


//
// Tells the FC module that an explosion went off at (x,y,z) and that
// any nearby cameras should start to shake.
//

void FC_explosion(SLONG x, SLONG y, SLONG z, SLONG force);	// 0 <= force <= 400 ish


#endif
