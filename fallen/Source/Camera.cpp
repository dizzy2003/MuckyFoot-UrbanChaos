// Camera.cpp
// Guy Simmons, 5th December 1997.

#include	"Game.h"
#include	"id.h"
#include	"statedef.h"

extern Camera		test_view;
void	set_camera_angle(Thing *c_thing,SLONG lag);

//---------------------------------------------------------------

void	init_cameras(void)
{
	memset(CAMERAS,0,sizeof(CAMERAS));
	CAMERA_COUNT	=	0;
}

//---------------------------------------------------------------

Thing	*alloc_camera(UBYTE type)
{
	SLONG			c0;
	CameraMan		*new_camera;
	Thing			*camera_thing	=	NULL;


	// Run through the camera array & find an unused one.
	for(c0=0;c0<MAX_CAMERAS;c0++)
	{
		if(CAMERAS[c0].CameraType==CAMERA_NONE)
		{
			camera_thing	=	alloc_thing(CLASS_CAMERA);
			if(camera_thing)
			{
				new_camera	=	TO_CAMERA(c0);
				new_camera->CameraType	=	type;
				new_camera->Thing		=	THING_NUMBER(camera_thing);

				camera_thing->Genus.Camera	=	new_camera;
			}
			break;
		}
	}
	return	camera_thing;
}

//---------------------------------------------------------------

void	free_camera(Thing *camera_thing)
{
	// Set the camera type to none & free the thing.
	camera_thing->Genus.Camera->CameraType	=	CAMERA_NONE;
	free_thing(camera_thing);
}

//---------------------------------------------------------------

Thing	*create_camera(UBYTE type,GameCoord *start_pos,Thing *track_thing)
{
	CameraMan		*t_camera;
	GameCoord		default_pos;
	Thing			*camera_thing	=	NULL;


	camera_thing	=	alloc_camera(type);
	if(camera_thing)
	{
		// Initialise the cameras relative position to the tracking thing.
		t_camera				=	camera_thing->Genus.Camera;
		t_camera->CameraDX		=	0;
		t_camera->CameraDY		=	1024-256;
		t_camera->CameraDZ		=	1024+256;
		t_camera->FocusThing	=	THING_NUMBER(track_thing);

		// Set camera world position.
		camera_thing->WorldPos	=	*start_pos;

		// Set the type & add it to the map.
		set_camera_type(camera_thing,type);
		add_thing_to_map(camera_thing);
	}

	return	camera_thing;
}

void	set_up_camera(Thing *camera_thing,GameCoord *start_pos,Thing *track_thing)
{
	GameCoord		default_pos;
	CameraMan		*t_camera;

	{
		// Initialise the cameras relative position to the tracking thing.
		t_camera				=	camera_thing->Genus.Camera;
		t_camera->CameraDX		=	0;
		t_camera->CameraDY		=	1024-256;
		t_camera->CameraDZ		=	1024+256;
		t_camera->FocusThing	=	THING_NUMBER(track_thing);

		// Set camera world position.
		camera_thing->WorldPos	=	*start_pos;

		// Set the type & add it to the map.
//		set_camera_type(camera_thing,type);
		camera_thing->StateFn	=	(void(*)(Thing*))process_t_camera;
		add_thing_to_map(camera_thing);
	}


}

//---------------------------------------------------------------

void	set_camera_type(Thing *c_thing,UBYTE type)
{
	// Set the camera state function.
	switch(type)
	{
		case	CAMERA_NONE:
			c_thing->StateFn	=	NULL;
			break;
		case	CAMERA_TRACKER:
			c_thing->StateFn	=	(void(*)(Thing*))process_t_camera;
			break;
		case	CAMERA_FIXED:
			c_thing->StateFn	=	(void(*)(Thing*))process_f_camera;
			break;
	}
}

//---------------------------------------------------------------

void	lock_camera_position(Thing *c_thing,GameCoord *lock_pos,BOOL snap)
{
	CameraMan		*t_camera;

	
	backup_camera(c_thing);	
	t_camera			=	c_thing->Genus.Camera;
	t_camera->CameraDX	=	lock_pos->X>>8;
	t_camera->CameraDY	=	lock_pos->Y>>8;
	t_camera->CameraDZ	=	lock_pos->Z>>8;
	if(snap)
	{
		c_thing->WorldPos	=	*lock_pos;
//		set_camera_angle(c_thing,0);

	}
	set_camera_type(c_thing,CAMERA_FIXED);
}

//---------------------------------------------------------------

void	backup_camera(Thing *c_thing)
{
	CameraMan *t_camera;

	t_camera			=	c_thing->Genus.Camera;

	t_camera->PrevDX=t_camera->CameraDX;
	t_camera->PrevDY=t_camera->CameraDY;
	t_camera->PrevDZ=t_camera->CameraDZ;
	t_camera->StateFn=c_thing->StateFn;
}

void	restore_old_camera(Thing *c_thing)
{
	CameraMan *t_camera;

	t_camera			=	c_thing->Genus.Camera;

	t_camera->CameraDX=t_camera->PrevDX;
	t_camera->CameraDY=t_camera->PrevDY;
	t_camera->CameraDZ=t_camera->PrevDZ;
	c_thing->StateFn=t_camera->StateFn;

}

void	free_camera_position(Thing *c_thing,GameCoord *rel_pos,BOOL snap)
{
	CameraMan		*t_camera;

	
	t_camera			=	c_thing->Genus.Camera;
	backup_camera(c_thing);	
//	t_camera->PrevDX	=	rel_pos->X;
	t_camera->CameraDX	=	rel_pos->X;
	t_camera->CameraDY	=	rel_pos->Y;
	t_camera->CameraDZ	=	rel_pos->Z;
	if(snap)
	{
	}
	set_camera_type(c_thing,CAMERA_TRACKER);
}

void	set_game_camera(SLONG dx,SLONG dy,SLONG dz)
{
	CameraMan		*t_camera;
	Thing			*c_thing;

	c_thing    = TO_THING(TO_CAMERA(0)->Thing); 
	t_camera   = c_thing->Genus.Camera;
	backup_camera(c_thing);

	t_camera->CameraDX	=	dx;
	t_camera->CameraDY	=	dy;
	t_camera->CameraDZ	=	dz;

}

//---------------------------------------------------------------
//
// A thing may required a special camera angle if its inside a building,
// or if it's on a firescape
//
SLONG	special_camera_angle_required_for_thing(Thing *p_thing,GameCoord *new_pos)
{
	SLONG wall;
	SLONG storey;
	SLONG building;
	SLONG thing;

	Thing *p_fthing;

	if(p_thing->OnFace)
	{
		if(p_thing->OnFace>0)
		{
			if(prim_faces4[p_thing->OnFace].Type==FACE_TYPE_FIRE_ESCAPE)
			{
				SLONG	building,storey,wall,face_x,face_y,face_z;
				SLONG	dx,dz;

				wall = prim_faces4[p_thing->OnFace].ThingIndex;

				if (wall<0)
				{
					storey   = wall_list[-wall].StoreyHead;
					building = storey_list[storey].BuildingHead;
					thing    = building_list[building].ThingIndex;

					p_fthing = TO_THING(thing);
				}
				else
				{
					//
					// face must be a building thing
					//
					return(0);

				}

				wall=storey_list[storey].WallHead;

				dx = wall_list[wall].DX;
				dz = wall_list[wall].DZ;

				wall = wall_list[wall].Next;

				dx -= wall_list[wall].DX;
				dz -= wall_list[wall].DZ;

				face_x = p_fthing->WorldPos.X;
				face_y = p_fthing->WorldPos.Y;
				face_z = p_fthing->WorldPos.Z;

				new_pos->X =(face_x+dx)<<8;
				new_pos->Y =p_thing->WorldPos.Y+(1000<<8);
				new_pos->Z =(face_z-dz*4)<<8;

				return(1);

			}
		}
		
	}
	return(0);

}

//---------------------------------------------------------------
//
// lag is now divided by 256 so lots of lag would be 64, not much lag would be 200, no lag would be 256 
//

//
// tries to point the camera at the player
//
void	set_camera_angle_for_pos(Thing *c_thing,SLONG lag)
{
	SWORD			angle_diff,angle_to;
	SLONG	t_dx,t_dy,t_dz;
	CameraMan		*t_camera;
	GameCoord		camera_position,
					dest_position;
	Thing			*track_thing;

	t_camera		=	c_thing->Genus.Camera;
	track_thing		=	TO_THING(t_camera->FocusThing);
	camera_position	=	c_thing->WorldPos;


	calc_sub_objects_position(track_thing,track_thing->Draw.Tweened->AnimTween,0,&t_dx,&t_dy,&t_dz); //0 is pelvis

	angle_to	=	(SWORD)Arctan	(
										(((camera_position.X-track_thing->WorldPos.X)>>8)-t_dx),
										(((camera_position.Z-track_thing->WorldPos.Z)>>8)-t_dz)
									);
	angle_diff	=	(angle_to-test_view.CameraAngle)&2047;
	if(angle_diff>1024)
		angle_diff	-=	2048;

//	if(abs(angle_diff)<300)
//		lag>>=1;
//	lag=lag*abs(angle_diff)>>9;
//	if(lag>200)
//		lag=200;
	if(track_thing->State==STATE_IDLE||USER_INTERFACE==0)
	{
		test_view.CameraAngle	=	(test_view.CameraAngle+((angle_diff*lag)>>8))&2047; //wal >>2 not 0
	}

	angle_to	=	Arctan	(
								(camera_position.Y>>8) - ((track_thing->WorldPos.Y>>8)+t_dy),
								Hypotenuse	(
												((camera_position.X-track_thing->WorldPos.X)>>8)-t_dx,
												((camera_position.Z-track_thing->WorldPos.Z)>>8)-t_dz
											)
							)-1024;
	angle_diff	=	(angle_to-test_view.CameraTilt)&2047;
	if(angle_diff>1024)
		angle_diff	-=	2048;
	test_view.CameraTilt	=	(test_view.CameraTilt+((angle_diff*lag)>>8))&2047; //>>0 was >>2
}

//
// Tries to point the camera in the direction the player is going
//
void	set_camera_rangle_for_player_angle(Thing *c_thing,SLONG lag)
{
	SWORD			angle_diff,angle_to;
	SLONG	t_dx,t_dy,t_dz;
	CameraMan		*t_camera;
	GameCoord		camera_position,
					dest_position;
	Thing			*track_thing;

	t_camera		=	c_thing->Genus.Camera;
	track_thing		=	TO_THING(t_camera->FocusThing);
	camera_position	=	c_thing->WorldPos;



	angle_to	=	(-(track_thing->Draw.Tweened->Angle-512)+512+2048)&2047;
	angle_diff	=	(angle_to-test_view.CameraAngle)&2047;

	if(angle_diff>1024)
		angle_diff	-=	2048;

	test_view.CameraRAngle	=	(test_view.CameraRAngle+((angle_diff*lag)>>8))&2047; //wal >>2 not 0

}

void	process_t_camera(Thing *c_thing)
{
	SLONG			distance,
					sin,cos,
					tx		=	0,
					ty		=	0,
					tz		=	0;
	CameraMan		*t_camera;
	GameCoord		camera_position,
					dest_position;
	Thing			*track_thing;
	SLONG	t_dx,t_dy,t_dz;
	SLONG	dx,dy,dz;
	SLONG	car=0;
	static	first_person=0;

	CBYTE str[100];
//	LogText(" process camera\n");


	t_camera		=	c_thing->Genus.Camera;
	track_thing		=	TO_THING(t_camera->FocusThing);


	camera_position	=	c_thing->WorldPos;

	if(!track_thing->Draw.Tweened)
		return;

//	set_camera_rangle_for_player_angle(c_thing,80); //very lagged

	calc_sub_objects_position(track_thing,track_thing->Draw.Tweened->AnimTween,0,&t_dx,&t_dy,&t_dz); //0 is pelvis

	//
	// bodge to stop camera bobbing when you run
	//
	if(abs(t_dy)<50)
		t_dy=0;

	if(special_camera_angle_required_for_thing(track_thing,&dest_position))
	{
		
	}
	else
	if ((GAME_FLAGS & GF_INDOORS) && Keys[KB_Q])
	{
		//
		// Indoors, put the camera at a point in the room.
		//

		UBYTE room = ID_get_mapsquare_room(track_thing->WorldPos.X >> ELE_SHIFT, track_thing->WorldPos.Z >> ELE_SHIFT);

		if (room == 0)
		{
			//
			// The player is inside a building, but not inside a
			// room!
			//

			dest_position = camera_position;
		}
		else
		{
			ID_get_room_camera(room, &dest_position.X, &dest_position.Y, &dest_position.Z);

			camera_position = dest_position;
		}
	}
	else
	{
		SLONG angle;
		SLONG tilt;

		if(USER_INTERFACE==1)
		{
			switch(track_thing->Class)
			{
				case CLASS_PERSON:
					if(track_thing->State==STATE_IDLE)
					{
						angle = track_thing->Draw.Tweened->Angle;
					}
					else
					{
						// camera angle is flipped arround 512
						angle = (-(test_view.CameraAngle-512)+512+2048)&2047; //track_thing->Draw.Tweened->Angle;
					}
					tilt  =	track_thing->Draw.Tweened->Tilt;
					car=0;
					break;
				case CLASS_VEHICLE:
					angle = track_thing->Genus.Furniture->RAngle;
					tilt  =	track_thing->Draw.Mesh->Tilt;
					car=1;
					break;

			}
		}
		else
		{
			switch(track_thing->Class)
			{
				case CLASS_PERSON:
					angle = track_thing->Draw.Tweened->Angle;
					tilt  =	track_thing->Draw.Tweened->Tilt;
					car=0;
					break;
				case CLASS_VEHICLE:
					angle = track_thing->Genus.Furniture->RAngle;
					tilt  =	0; //track_thing->Draw.Mesh->Tilt;
					car=1;
					break;

			}
		}
//		angle = (-(test_view.CameraRAngle-512)+512+2048)&2047; //track_thing->Draw.Tweened->Angle;


		// Rotate camera vector around Y axis.
		sin		=	SIN(angle)>>2;
		cos		=	COS(angle)>>2;
		tx		=	t_camera->CameraDX;
		tz		=	t_camera->CameraDZ;
		dest_position.X	=	((cos*tx) + (sin*tz))>>14;
		dest_position.Z	=	((-sin*tx) + (cos*tz))>>14;

		// Rotate camera vector around X axis.
		sin		=	SIN(tilt)>>2; //bagginess
		cos		=	COS(tilt)>>2;
		ty		=	t_camera->CameraDY;
		tz		=	dest_position.Z;
		dest_position.Y	=	((cos*ty) + (sin*tz))>>14;
		dest_position.Z	=	((-sin*ty) + (cos*tz))>>14;

		// This is the position the camera wants to be at.
		dest_position.X +=	(track_thing->WorldPos.X>>8)+(t_dx);
		dest_position.Y	+=	(track_thing->WorldPos.Y>>8)+(t_dy);
		dest_position.Z	+=	(track_thing->WorldPos.Z>>8)+(t_dz);
	}

	// Now create a vector between the current position & the new position.
	tx		=	dest_position.X-(camera_position.X>>8);
	ty		=	dest_position.Y-(camera_position.Y>>8);
	tz		=	dest_position.Z-(camera_position.Z>>8);

	// Set the camera angle & tilt.

	// Calculate the distance.
	distance	=	Hypotenuse(Hypotenuse(tx,ty),tz);

	// Normalise the vector.
	// I don't think the camera will ever be close enough for distance to be 0, but saying that...
	if(distance==0)
		distance=1;
	tx	=	(tx<<10)/distance;
	ty	=	(ty<<10)/distance;
	tz	=	(tz<<10)/distance;

	// Dampen the distance.  This defines how baggy the camera is, should be variable.
	distance	>>=	(4-car*3); //was 5
	dx=tx*distance;
	dy=ty*distance;
	dz=tz*distance;
/*

	dx=(dx*TICK_RATIO)>>TICK_SHIFT;
	dy=(dy*TICK_RATIO)>>TICK_SHIFT;
	dz=(dz*TICK_RATIO)>>TICK_SHIFT;
*/
	// Now set the cameras new position.
	camera_position.X	=	((camera_position.X<<10-8)+(dx))>>10-8;
	camera_position.Y	=	((camera_position.Y<<10-8)+(dy>>2))>>10-8;
	camera_position.Z	=	((camera_position.Z<<10-8)+(dz))>>10-8;

//	camera_position=dest_position;


	// Set the new position.
	move_thing_on_map(c_thing,&camera_position);
//	move_thing_on_map(c_thing,&dest_position);

	set_camera_angle_for_pos(c_thing,100+car*120); //always look straight at player

	// Set camera position.
	test_view.CameraX	=	(float)(camera_position.X>>8);
	test_view.CameraY	=	(float)(camera_position.Y>>8);
	test_view.CameraZ	=	(float)(camera_position.Z>>8);

	if (Keys[KB_V])
	{
		if(first_person)
		{
			first_person=0;
		}
		else
		{
			if(ShiftFlag)
				first_person=2;
			else
				first_person=1;
		}
	}

	if (first_person) //Keys[KB_V])
	{
		extern Engine the_engine;


		//
		// Use the mouse to look around.
		// 

		SLONG dx;
		SLONG dy;
		
		SLONG yaw;
		SLONG pitch;

		dx = (MouseX - the_engine.HalfViewWidth);
		dy = (MouseY - the_engine.HalfViewHeight);
		dx<<=1;
		dy<<=1;

		#define MAX_CAM_PITCH	(400)
		#define MAX_CAM_YAW		(1024)

		yaw   = (-dx * MAX_CAM_YAW)   / the_engine.HalfViewWidth;
		pitch = ( dy * MAX_CAM_PITCH) / the_engine.HalfViewHeight;

		yaw   += track_thing->Draw.Tweened->Angle;
		pitch += track_thing->Draw.Tweened->Tilt;

		yaw    = 1024 - yaw;	// !?

		yaw   &= 2047;
		pitch &= 2047;

		if (first_person==2)
		{
			static float camera_b_dist = 512.0F;

			if (Keys[KB_P9]) {camera_b_dist += 40.0F;}
			if (Keys[KB_P6]) {camera_b_dist -= 40.0F;}

			SATURATE(camera_b_dist, 100.0F, 4000.0F);

			//
			// Look at the track_thing.
			//

			extern void MATRIX_vector(float vector[3], float yaw, float pitch);

			float vector[3];
			float f_yaw;
			float f_pitch;

			f_yaw   = float(yaw)   * (2.0 * PI / 2048.0F);
			f_pitch = float(pitch) * (2.0 * PI / 2048.0F);

			MATRIX_vector(
				vector,
				f_yaw,
				f_pitch);

			test_view.CameraX = float(track_thing->WorldPos.X>>8);
			test_view.CameraY = float(track_thing->WorldPos.Y>>8);
			test_view.CameraZ = float(track_thing->WorldPos.Z>>8);

			test_view.CameraY += 0x80;

			#define CAMERA_B_DIST (camera_b_dist)

			test_view.CameraX -= vector[0] * CAMERA_B_DIST;
			test_view.CameraY -= vector[1] * CAMERA_B_DIST;
			test_view.CameraZ -= vector[2] * CAMERA_B_DIST;

			yaw    = -yaw;
			pitch  =  pitch;

			yaw   &= 2047;
			pitch &= 2047;
		}
		else
		{
			//
			// Look through the eyes of the trackthing.
			//

			test_view.CameraX = float(track_thing->WorldPos.X>>8);
			test_view.CameraY = float(track_thing->WorldPos.Y>>8);
			test_view.CameraZ = float(track_thing->WorldPos.Z>>8);

			test_view.CameraY += 0xb0;
		}

		test_view.CameraAngle = yaw;
		test_view.CameraTilt  = pitch;
	}
}

//---------------------------------------------------------------

void	process_f_camera(Thing *c_thing)
{
	SWORD			angle_diff,angle_to;
	SLONG			distance,
					sin,cos,
					tx		=	0,
					ty		=	0,
					tz		=	0;
	CameraMan		*t_camera;
	GameCoord		camera_position,
					dest_position;
	Thing			*track_thing;
	SLONG	t_dx,t_dy,t_dz;


	t_camera		=	c_thing->Genus.Camera;
	track_thing		=	TO_THING(t_camera->FocusThing);
	camera_position	=	c_thing->WorldPos;

	if(!track_thing->Draw.Tweened)
		return;

	calc_sub_objects_position(track_thing,track_thing->Draw.Tweened->AnimTween,0,&t_dx,&t_dy,&t_dz); //0 is pelvis


	// This is the position the camera wants to be at.
	dest_position.X	=	t_camera->CameraDX;
	dest_position.Y	=	t_camera->CameraDY;
	dest_position.Z	=	t_camera->CameraDZ;

	// Now create a vector between the current position & the new position.
	tx		=	dest_position.X-(camera_position.X>>8);
	ty		=	dest_position.Y-(camera_position.Y>>8);
	tz		=	dest_position.Z-(camera_position.Z>>8);

	// Set the camera angle & tilt.
	angle_to	=	(SWORD)Arctan	(
										(camera_position.X>>8)-((track_thing->WorldPos.X>>8)+t_dx),
										(camera_position.Z>>8)-((track_thing->WorldPos.Z>>8)+t_dz)
									);
	angle_diff	=	(angle_to-test_view.CameraAngle)&2047;
	if(angle_diff>1024)
		angle_diff	-=	2048;
	test_view.CameraAngle	=	(test_view.CameraAngle+(angle_diff>>2))&2047; //wal >>2 not 0

	angle_to	=	Arctan	(
								(camera_position.Y>>8) - ((track_thing->WorldPos.Y>>8)+t_dy),
								Hypotenuse	(
												(camera_position.X>>8)-((track_thing->WorldPos.X>>8)+t_dx),
												(camera_position.Z>>8)-((track_thing->WorldPos.Z>>8)+t_dz)
											)
							)-1024;
	angle_diff	=	(angle_to-test_view.CameraTilt)&2047;
	if(angle_diff>1024)
		angle_diff	-=	2048;
	test_view.CameraTilt	=	(test_view.CameraTilt+(angle_diff>>2))&2047; //>>0 was >>2

	// Calculate the distance.
	distance	=	Hypotenuse(Hypotenuse(tx,ty),tz);

	// Normalise the vector.
	// I don't think the camera will ever be close enough for distance to be 0, but saying that...
	if(distance==0)
		distance=1;
	tx	=	(tx<<14)/distance;
	ty	=	(ty<<14)/distance;
	tz	=	(tz<<14)/distance;

	// Dampen the distance.  This defines how baggy the camera is, should be variable.
	distance	>>=	4; //was 5

	// Now set the cameras new position.
	camera_position.X	=	((camera_position.X<<14-8)+(tx*distance))>>14-8;
	camera_position.Y	=	((camera_position.Y<<14-8)+(ty*distance))>>14-8;
	camera_position.Z	=	((camera_position.Z<<14-8)+(tz*distance))>>14-8;

	// Set the new position.
	move_thing_on_map(c_thing,&camera_position);

	// Set camera position.
	test_view.CameraX	=	(float)(camera_position.X>>8);
	test_view.CameraY	=	(float)(camera_position.Y>>8);
	test_view.CameraZ	=	(float)(camera_position.Z>>8);
}

//---------------------------------------------------------------

SWORD	spin_step	=	0;

void	spin_camera_around_subject(Thing *c_thing)
{
	SLONG			distance;
	CameraMan		*t_camera;


	t_camera		=	c_thing->Genus.Camera;

	distance	=	1536;

	t_camera->CameraDX	=	(SIN(spin_step&2047)*distance)>>16;
	t_camera->CameraDY	=	512;
	t_camera->CameraDZ	=	(COS(spin_step&2047)*distance)>>16;

	spin_step	+=	16;

	process_t_camera(c_thing);
}

//---------------------------------------------------------------
