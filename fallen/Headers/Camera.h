// Camera.h
// Guy Simmons, 5th December 1997.

#ifndef	CAMERA_H
#define	CAMERA_H

//---------------------------------------------------------------

#define	MAX_CAMERAS			8

#define	CAMERA_NONE			0
#define	CAMERA_TRACKER		1
#define	CAMERA_FIXED		2

//---------------------------------------------------------------

// struct	Thing;		// Prototype 'Thing' structure definition.

struct	CameraMan
{

	COMMON(CameraType)

	SLONG			CameraDX,
					CameraDY,
					CameraDZ;
	SLONG			PrevDX;
	SLONG			PrevDY;
	SLONG			PrevDZ;
	void			(*StateFn)(Thing*);	// Things state function.
	THING_INDEX		FocusThing;
};

//---------------------------------------------------------------

void	init_cameras(void);
Thing	*alloc_camera(UBYTE type);
void	free_camera(Thing *camera_thing);
Thing	*create_camera(UBYTE type,GameCoord *start_pos,Thing *track_thing);
void	set_camera_type(Thing *c_thing,UBYTE type);
void	lock_camera_position(Thing *c_thing,GameCoord *lock_pos,BOOL snap);
void	free_camera_position(Thing *c_thing,GameCoord *rel_pos,BOOL snap);
void	process_t_camera(Thing *camera_thing);
void	process_f_camera(Thing *camera_thing);
void	spin_camera_around_subject(Thing *c_thing);
void	backup_camera(Thing *c_thing);
void	restore_old_camera(Thing *c_thing);

//---------------------------------------------------------------

#endif

