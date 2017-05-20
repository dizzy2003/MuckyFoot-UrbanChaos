//
// Helicopters
// Wockawockawockawocka
//

#include "game.h"
#include "animate.h"
#include "chopper.h"
#include "statedef.h"
#include "dirt.h"
#include "pap.h"
#include "mav.h"
#include "sound.h"
#include "mfx.h"
#include "eway.h"
#include "memory.h"

#include <stdio.h>

#define HARDWIRED_RADIUS	8192
//#define DETECTION_RADIUS	4096
#define DETECTION_RADIUS	1024
#define IGNORAMUS_RADIUS	6144

#define PRIM_OBJ_CHOPPER		74
#define	PRIM_OBJ_CHOPPER_BLADES	75

/***********************************************
 *
 *   Generic helicopter infrastructure stuff
 *
 */


GenusFunctions CHOPPER_functions[CHOPPER_NUMB] =
{
	{CHOPPER_NONE,		NULL},
	{CHOPPER_CIVILIAN,	CIVILIAN_state_function}
};



//
// Zeros out the choppers in the 'the_game' structure.
//

#ifndef PSX
void init_choppers(void)
{
	memset((UBYTE*)CHOPPERS, 0, sizeof(Chopper) * MAX_CHOPPERS);

	CHOPPER_COUNT = 0;
}

//
// Creates a new chopper of the given type.
//
#endif

Thing *alloc_chopper(UBYTE type)
{		 

	SLONG i;

	Thing    *p_thing;
	Chopper  *p_chopper;
	DrawMesh *dm;

	THING_INDEX t_index;
	SLONG       a_index;

	ASSERT(WITHIN(type, 1, CHOPPER_NUMB - 1));

	//
	// Look for an unused chopper structure.
	//

	for (i = 1; i < MAX_CHOPPERS; i++)
	{
		if (CHOPPERS[i].ChopperType == CHOPPER_NONE)
		{
			a_index = i;
			
			goto found_chopper;
		}
	}

	//
	// There are no spare chopper structures.
	//

	TRACE("Run out of chopper structures.");

	return NULL;

  found_chopper:

	//
	// Find the drawmesh structure for the chopper.
	//

	dm = alloc_draw_mesh();

	if (dm == NULL)
	{
		ASSERT(0);
		//
		// Could not allocate a drawmesh structure.
		//

		TRACE("Run out of drawmesh structures.");

		//
		// Free up the chopper structure we allocated.
		//

		TO_CHOPPER(a_index)->ChopperType = CHOPPER_NONE;

		return NULL;
	}


	t_index = alloc_primary_thing(CLASS_CHOPPER);

	if (t_index)
	{
		p_thing  = TO_THING(t_index);
		p_chopper = TO_CHOPPER(a_index);

		//
		// Link the chopper to the thing and back again.
		//

		p_thing->Class			= CLASS_CHOPPER;
		p_thing->Genus.Chopper	= p_chopper;
		p_chopper->ChopperType	= type;
		p_chopper->thing		= p_thing;
		p_chopper->speed		= 2048;
		p_chopper->radius		= HARDWIRED_RADIUS;

		p_chopper->substate		= CHOPPER_substate_landed;


		//
		// Set the draw type and the drawmesh stuff.
		//

		p_thing->DrawType  = DT_CHOPPER;
		p_thing->Draw.Mesh = dm;
		
		dm->Angle    = 0;
		dm->Roll     = 0;
		dm->Tilt     = 0;
//		dm->ObjectId = PRIM_OBJ_TRAFFIC_CONE; 
		dm->ObjectId = PRIM_OBJ_CHOPPER; 
		p_chopper->rotorprim=PRIM_OBJ_CHOPPER_BLADES;

		return p_thing;
	}
	else
	{
		//
		// Free up the chopper structure and the
		// drawmesh structure we allocated.
		//

		TO_CHOPPER(a_index)->ChopperType = CHOPPER_NONE;
		
		free_draw_mesh(dm);
	}
}
#ifndef PSX
void free_chopper(Thing *p_thing)
{
	Chopper *chopper = CHOPPER_get_chopper(p_thing);

	//
	// Free the chopper structure and the thing structure.
	//

	chopper->ChopperType  = CHOPPER_NONE;
	CHOPPER_COUNT       -= 1;

	free_thing(p_thing);
}
#endif

Thing *CHOPPER_create(GameCoord pos, UBYTE type)
{
	Thing *p_thing = alloc_chopper(type);

	if (p_thing != NULL)
	{
		p_thing->WorldPos = pos;

		add_thing_to_map(p_thing);

		//
		// Make it initialise itself.
		//

		set_state_function(p_thing, STATE_INIT);
	}

	return p_thing;
}


Chopper *CHOPPER_get_chopper(struct Thing *chopper_thing)
{
	Chopper *chopper;

	ASSERT(WITHIN(chopper_thing, TO_THING(1),  TO_THING(MAX_THINGS)));
	ASSERT(chopper_thing->Class == CLASS_CHOPPER);

	chopper = chopper_thing->Genus.Chopper;

	ASSERT(WITHIN(chopper, TO_CHOPPER(1), TO_CHOPPER(MAX_CHOPPERS - 1)));

	return chopper;
}

DrawMesh *CHOPPER_get_drawmesh(struct Thing *chopper_thing)
{
	DrawMesh *dm;

	ASSERT(WITHIN(chopper_thing, TO_THING(1),  TO_THING(MAX_THINGS)));
	ASSERT(chopper_thing->Class == CLASS_CHOPPER);

	dm = chopper_thing->Draw.Mesh;

	ASSERT(WITHIN(dm, &DRAW_MESHES[0], &DRAW_MESHES[MAX_DRAW_MESHES - 1]));

	return dm;
}


/*************************************************************
 *
 *   Chopper Utility Belt
 *
 */

SLONG CHOPPER_altitude(Thing *thing) {
//	return thing->WorldPos.Y-(250<<8);
//	return (thing->WorldPos.Y-(250<<8))-(PAP_calc_height_at(thing->WorldPos.X>>8,thing->WorldPos.Z>>8)<<8);
//	return (thing->WorldPos.Y-(250<<8))-(PAP_calc_map_height_at(thing->WorldPos.X>>8,thing->WorldPos.Z>>8)<<8);
	return (thing->WorldPos.Y-(250<<8))-(PAP_calc_map_height_at(thing->WorldPos.X>>8,thing->WorldPos.Z>>8));
}

void CHOPPER_home(Thing *thing, GameCoord new_pos) {
	Chopper *chopper = CHOPPER_get_chopper(thing);
	DrawMesh *dm = CHOPPER_get_drawmesh(thing);
	SLONG dx,dz,angle,dangle;
//	CBYTE msg[300];
	UBYTE accel;
	UWORD speed;

	dx = new_pos.X - thing->WorldPos.X;
	dz = new_pos.Z - thing->WorldPos.Z;

//	chopper->dist = Root( dx*dx + dz*dz );
//	chopper->dist = QDIST2(dx,dz);
	chopper->dist = SDIST2((dx>>8),(dz>>8));

	accel=0;

	if (chopper->dist>(128*128)) {
		if (chopper->dist>(512*512)) {
			//speed=8192; 
			accel=96;
		} else {
			//speed=2048; 
			accel=64;
		}
	}

	if (chopper->since_takeoff < 255)
	{
		accel                   = chopper->since_takeoff >> 2;
		chopper->since_takeoff += 5;
	}

//	speed = QDIST2((dx>>8),(dz>>8))*16;
	speed = chopper->dist/64;
	if (speed<2048) speed=2048;
	if (speed>12288) speed=12288;
	chopper->speed=speed;
	if (accel) {
		if (thing->WorldPos.X > new_pos.X)
			chopper->dx-=accel;
		if (thing->WorldPos.X < new_pos.X)
			chopper->dx+=accel;
		if (thing->WorldPos.Z > new_pos.Z)
			chopper->dz-=accel;
		if (thing->WorldPos.Z < new_pos.Z)
			chopper->dz+=accel;
	}

	if ((chopper->dist>(512*512))||chopper->counter) {
		angle=-Arctan(dx,dz);
		angle&=2047;
		dangle = angle - dm->Angle;
		if (dangle >  1024) {dangle -= 2048;}
		if (dangle < -1024) {dangle += 2048;}

		dangle >>= 3;

		SATURATE(dangle, -10, +10);

		chopper->ry=dangle;

		if (chopper->dist>(512*512)) chopper->counter=128;

	}
	if (chopper->counter) chopper->counter--;

	dm->Angle += chopper->ry;
	dm->Angle &= 2047;
	chopper->ry=(chopper->ry*204)>>8;  //float removed MD


//	sprintf(msg,"dist:%d  counter:%d  spd: %d\n",chopper->dist,chopper->counter,chopper->speed);
//	TRACE(msg);

	//
	// Avoid buildings.
	//

	{
		SLONG dist;
		SLONG mid_x = thing->WorldPos.X >> 8;
		SLONG mid_y = thing->WorldPos.Y >> 8;
		SLONG mid_z = thing->WorldPos.Z >> 8;

//		mid_y -= 0x80;
		mid_y -= 0xff;

		#define CHOPPER_AVOID_SPEED 64

		for (dist = 128; dist <= 0x400; dist += 128)
		{
			if (MAV_inside(mid_x + dist, mid_y, mid_z)) {chopper->dx -= CHOPPER_AVOID_SPEED;}
			if (MAV_inside(mid_x - dist, mid_y, mid_z)) {chopper->dx += CHOPPER_AVOID_SPEED;}
			if (MAV_inside(mid_x, mid_y, mid_z + dist)) {chopper->dz -= CHOPPER_AVOID_SPEED;}
			if (MAV_inside(mid_x, mid_y, mid_z - dist)) {chopper->dz += CHOPPER_AVOID_SPEED;}
		}
	}
}

void CHOPPER_limit(Chopper *chopper) {
  if (chopper->dx >  chopper->speed) chopper->dx= (chopper->dx+chopper->dx+chopper->dx+chopper->speed)>>2;
  if (chopper->dz >  chopper->speed) chopper->dz= (chopper->dz+chopper->dz+chopper->dz+chopper->speed)>>2;
  if (chopper->dx < -chopper->speed) chopper->dx= (chopper->dx+chopper->dx+chopper->dx-chopper->speed)>>2; //*0.25;
  if (chopper->dz < -chopper->speed) chopper->dz= (chopper->dz+chopper->dz+chopper->dz-chopper->speed)>>2; //*0.25;

/*  if (chopper->dx> chopper->speed) chopper->dx= chopper->speed;
  if (chopper->dz> chopper->speed) chopper->dz= chopper->speed;
  if (chopper->dx<-chopper->speed) chopper->dx=-chopper->speed;
  if (chopper->dz<-chopper->speed) chopper->dz=-chopper->speed;*/
}

void CHOPPER_damp(Chopper *chopper, UBYTE factor) {
  UBYTE i;
/*
  for (i=0;i<factor;i++) {
	  if (chopper->dx>32) chopper->dx-=31;
	  if (chopper->dz>32) chopper->dz-=31;
	  if (chopper->dx<32) chopper->dx+=31;
	  if (chopper->dz<32) chopper->dz+=31;
	  if (chopper->dx>0) chopper->dx--;
	  if (chopper->dz>0) chopper->dz--;
	  if (chopper->dx<0) chopper->dx++;
	  if (chopper->dz<0) chopper->dz++;
  }*/

//  chopper->dx*=0.90; //bloody floats
  //chopper->dz*=0.90;
  chopper->dx=(chopper->dx*230)>>8;
  chopper->dz=(chopper->dz*230)>>8;

}

UBYTE CHOPPER_radius_broken(GameCoord pnt, GameCoord ctr, SLONG radius) {
	SLONG dist,dx,dz;

	dx = (pnt.X-ctr.X)>>8;
	dz = (pnt.Z-ctr.Z)>>8;
	return (((dx*dx)+(dz*dz))>(radius*radius));
}


void CHOPPER_predict_altitude(Thing *thing, Chopper *chopper) 
{
  SLONG tx,tz,dx,dz,altitude,gnd;
  SLONG dist;

  if ((chopper->dx==0)&&(chopper->dz==0)) return;

  tx=thing->WorldPos.X>>8;
  tz=thing->WorldPos.Z>>8;

  dx=chopper->dx>>3;
  dz=chopper->dz>>3;

  tx+=dx; tz+=dz;

  gnd=PAP_calc_height_at(tx,tz)+0x10;

//    DIRT_new_water(tx, gnd, tz,     -1, 28,  0);

  
//  altitude=(750<<8)+(thing->WorldPos.Y-(250<<8))-(PAP_calc_map_height_at(tx,tz)<<8);
  altitude=(675<<8)+(PAP_calc_map_height_at(tx,tz)<<8);		// was 750 - lower so we can see it more often

  // try and be higher than our target
  if (chopper->target&&(altitude<chopper->target->WorldPos.Y+(50<<8))) altitude=chopper->target->WorldPos.Y+(50<<8);

  altitude-=CHOPPER_altitude(thing);

//  altitude/=Root(SDIST2((dx>>8),(dz>>8)));
  
  dist=Root( (dx*dx)+(dz*dz) );
  dist = dist ? dist : 1;
  altitude/=dist;
/*
  if ((altitude>20)&&(MAV_inside(tx,gnd,tz))) {
	  TRACE("panic!\n");
	  CHOPPER_damp(chopper,1);
	  CHOPPER_damp(chopper,1);
  }
*/

  //chopper->dy=altitude;

	
	altitude /= 32;

	if (chopper->since_takeoff != 255)
	{
		//
		// Still taking off.
		//

		SATURATE(altitude, -6, +6);
	}

  chopper->dy += altitude;
  chopper->dy -= chopper->dy / 32;
}


/*************************************************************
 *
 *   Specific helicopter behaviour stuff
 *
 */

//
// The functions called when a chopper is in each State.
//

void CHOPPER_fn_init  (Thing *);
void CHOPPER_fn_normal(Thing *);


StateFunction CIVILIAN_state_function[] =
{
	{STATE_INIT,	CHOPPER_fn_init	},
	{STATE_NORMAL,	CHOPPER_fn_normal}
};



// ========================================================
//
// The state functions.
//
// ========================================================

void CHOPPER_fn_init(Thing *thing)
{
	Chopper *chopper = CHOPPER_get_chopper(thing);
	DrawMesh *dm     = CHOPPER_get_drawmesh(thing);

	chopper->home.X = thing->WorldPos.X;
	chopper->home.Y = thing->WorldPos.Y;
	chopper->home.Z = thing->WorldPos.Z;

	//
	// Start being a normal chopper.
	//

	set_state_function(thing, STATE_NORMAL);

}

void CHOPPER_fn_normal(Thing *thing)
{
	GameCoord new_pos;
	SLONG mag, rpos, altitude;
	CBYTE msg[300];

// blatant ects hack
	Thing *darci = NET_PERSON(0);

	Chopper *chopper = CHOPPER_get_chopper(thing);

	if (chopper->substate!=CHOPPER_substate_landed) {
//		play_quick_wave(thing,S_TUNE_SEWER+1,WAVE_PLAY_NO_INTERUPT);
		MFX_play_thing(THING_NUMBER(thing),S_HELI,MFX_LOOPED|MFX_QUEUED|MFX_MOVING,thing);
/*		if (!chopper->channel) chopper->channel=play_object_wave(chopper->channel,thing,S_HELI,WAVE_PLAY_NO_INTERUPT|WAVE_LOOP);
#ifdef USE_A3D
		if (chopper->channel) A3DPosition(chopper->channel,thing->WorldPos.X,thing->WorldPos.Y,thing->WorldPos.Z);
#endif
		*/
	}

	// General stuff -- happens regardless of state

	chopper->rotors+=(chopper->rotorspeed>>2);
	new_pos=thing->WorldPos;
	new_pos.X+=chopper->dx;
	new_pos.Y+=chopper->dy;
	new_pos.Z+=chopper->dz;
	move_thing_on_map(thing, &new_pos);

	altitude=CHOPPER_altitude(thing);

	if (chopper->rotorspeed>0) {
		// Are we low enough to cause ground-level turbulence?
		if (altitude<(1200<<8)) {
			
			// rotor speed and altitude affect the strength of the turbulence
			mag=chopper->rotorspeed-(altitude>>10);
			if (mag>0) {
				rpos=chopper->rotors;
				rpos&=2047;
				new_pos.X=(mag*SIN(rpos))>>10;
				new_pos.Z=(mag*COS(rpos))>>10;

				new_pos.X+=thing->WorldPos.X;
				new_pos.Z+=thing->WorldPos.Z;

				DIRT_gust(
				  thing,
				  thing->WorldPos.X>>8,thing->WorldPos.Z>>8,
				  new_pos.X>>8, new_pos.Z>>8);

				rpos+=1024;
				rpos&=2047;
				new_pos.X=(mag*SIN(rpos))>>10;
				new_pos.Z=(mag*COS(rpos))>>10;
				
				new_pos.X+=thing->WorldPos.X;
				new_pos.Z+=thing->WorldPos.Z;

				DIRT_gust(
				  thing,
				  thing->WorldPos.X>>8,thing->WorldPos.Z>>8,
				  new_pos.X>>8, new_pos.Z>>8);
			}


		}

	}


	// State specific stuff

	switch(chopper->substate) {

	case CHOPPER_substate_idle:
		CHOPPER_damp(chopper,1);
		if (chopper->victim)
			CHOPPER_init_state(thing,CHOPPER_substate_tracking);
		else
		if (!CHOPPER_radius_broken(darci->WorldPos,thing->WorldPos,DETECTION_RADIUS)) {
			if ((!chopper->target)&&!chopper->victim) chopper->target=darci;
			CHOPPER_init_state(thing,CHOPPER_substate_tracking);
		}
		break;

	case CHOPPER_substate_takeoff:
		CHOPPER_damp(chopper,1);
		if (chopper->rotorspeed < 500)
		{
			 chopper->rotorspeed+=10;	// was 2

			 if (chopper->rotorspeed > 150)
			 {
				chopper->dy += chopper->rotorspeed - 150 >> 6;
				chopper->dy += 2;
			 }
		}
		else
			if (altitude<(170<<8)) {
				chopper->dy+=10;			// was 2

				if (chopper->light < 250)
				{
					chopper->light += 5;
				}

				//chopper->light=255;
			}
			else
			/*
				if (chopper->dy>0)
					chopper->dy--;
				else
			*/
			{
				//
				// Don't slow down before you start chasing Darci!
				//

				CHOPPER_init_state(thing,CHOPPER_substate_idle);
			}

		break;

	case CHOPPER_substate_landing:
		CHOPPER_damp(chopper,4);
		if (altitude>0) {
			chopper->dy=((chopper->dy+chopper->dy+chopper->dy-(altitude>>6))*0.25f)-1.0f;
		} else {
			chopper->dy=0;
			if (chopper->light>3) chopper->light-=3;
			if (chopper->light) chopper->light--;
			if (chopper->rotorspeed > 200)
				chopper->rotorspeed--;
			if (chopper->rotorspeed > 0)
				chopper->rotorspeed--;
			else
				CHOPPER_init_state(thing,CHOPPER_substate_landed);
		}
		break;
/*		if (altitude>(40<<8)) {
			chopper->dy--;
		} else
			if (chopper->dy<0) {
				chopper->dy+=2;
				if (chopper->dy>0) chopper->dy=0;
			} else {
				if (chopper->light>3) chopper->light-=3;
				if (chopper->light) chopper->light--;
				if (altitude>0) thing->WorldPos.Y--;
				if (chopper->rotorspeed > 200)
					chopper->rotorspeed--;
				if (chopper->rotorspeed > 0)
					chopper->rotorspeed--;
				else
					CHOPPER_init_state(thing,CHOPPER_substate_landed);
			}
		break;*/

	case CHOPPER_substate_landed:
			if (chopper->victim||!CHOPPER_radius_broken(darci->WorldPos,thing->WorldPos,DETECTION_RADIUS)) {
//				chopper->target=darci;
				CHOPPER_init_state(thing,CHOPPER_substate_takeoff);
			}
		// heh.
		break;

	case CHOPPER_substate_tracking:

		if (chopper->victim) {
			chopper->victim=EWAY_get_person(chopper->victim);
			chopper->target=TO_THING(chopper->victim);
			chopper->victim=0;
		}

/*		// no idea what this does y'know
		if (altitude<(200<<8)) {
			chopper->dy*=1.5;
		}

		// Safety thing -- always ramp over buildings:
		if (altitude<(100<<8)) {
			SLONG alt_wanted;

			alt_wanted=(750<<8)+(PAP_calc_map_height_at(thing->WorldPos.X>>8,thing->WorldPos.Z>>8)<<8);

			thing->WorldPos.Y=(thing->WorldPos.Y+thing->WorldPos.Y+thing->WorldPos.Y+alt_wanted)*0.25;
		}
		
		// nor this. i bet it's important tho.
		if (altitude>(600<<8)) {
			chopper->dy*=0.6;
		}
*/
		CHOPPER_predict_altitude(thing,chopper);

		CHOPPER_home(thing,chopper->target->WorldPos);
		CHOPPER_limit(chopper);

		if (chopper->target->Class==CLASS_PERSON) {
			if ((chopper->target->Genus.Person->PlayerID>0)&&
				(CHOPPER_radius_broken(thing->WorldPos,chopper->home,chopper->radius)))
					CHOPPER_init_state(thing,CHOPPER_substate_homing);
		}

		break;

	case CHOPPER_substate_patrolling:
		GameCoord targ;
		SLONG rot;

		chopper->patrol++;
		// rot=chopper->patrol>>2;
		rot=chopper->patrol;
		rot&=2047;
		targ.X = chopper->home.X + (SIN(rot)*5);
		targ.Z = chopper->home.Z + (COS(rot)*5);
		targ.Y = thing->WorldPos.Y;
		CHOPPER_home(thing,targ);
//		DIRT_new_water(targ.X>>8, PAP_calc_height_at(targ.X>>8,targ.Z>>8)+0x10, targ.Z>>8,     -1, 28,  0);
		break;

	case CHOPPER_substate_homing:
		CHOPPER_predict_altitude(thing,chopper);
		CHOPPER_home(thing,chopper->home);
		CHOPPER_limit(chopper);

// blatant ects hack
		if (!CHOPPER_radius_broken(thing->WorldPos,chopper->home,IGNORAMUS_RADIUS)) {
			if (!CHOPPER_radius_broken(darci->WorldPos,thing->WorldPos,DETECTION_RADIUS)) {
				chopper->target=darci;
				CHOPPER_init_state(thing,CHOPPER_substate_tracking);
			}
		}

		
		break;

		default:
			ASSERT(0);
			break;
	}

//	sprintf(msg,"X:%d  Y:%d  Z:%d  MWHO:%d\n",thing->WorldPos.X,thing->WorldPos.Y,thing->WorldPos.Z,thing->State&FLAGS_ON_MAPWHO);
//	TRACE(msg);

}

/*************************************************************
 *
 *   Helicopter substates -- individual actions
 *
 */

void CHOPPER_init_state(Thing *chopper_thing, UBYTE new_state) {
	Chopper *chopper = CHOPPER_get_chopper(chopper_thing);
	
	TRACE("Chopper: ");
	switch (new_state) {
	case CHOPPER_substate_idle:
		TRACE("Idle\n");
		break;
	case CHOPPER_substate_takeoff:
		TRACE("Takeoff\n");
		chopper->counter       = 0;
		chopper->since_takeoff = 0;
		break;
	case CHOPPER_substate_landing:
		TRACE("Landing\n");
		chopper->counter=0;
		break;
	case CHOPPER_substate_landed:
		TRACE("Landed\n");
		chopper->counter=0;
		break;
	case CHOPPER_substate_tracking:
		if (chopper->victim) {
			chopper->victim=EWAY_get_person(chopper->victim);
			chopper->target=TO_THING(chopper->victim);
			chopper->victim=0;
		}
		if (!(chopper->target)) {
			TRACE("Tracking failed, no target\n");
			CHOPPER_init_state(chopper_thing, CHOPPER_substate_idle);
		} else {
			TRACE("Tracking\n");
		}
		chopper->counter=0;
		break;
	case CHOPPER_substate_homing:
		chopper->target=0;
		chopper->counter=0;
		TRACE("Homing\n");
		break;
	case CHOPPER_substate_patrolling:
		chopper->target=0;
		chopper->patrol=0;
		TRACE("Patrolling\n");
		break;
	}
	chopper->substate = new_state;

}


