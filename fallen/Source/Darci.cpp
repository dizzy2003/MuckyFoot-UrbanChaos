// Darci.cpp
// Guy Simmons, 4th January 1998

#include	"Game.h"
#include	"Darci.h"
#include	"animate.h"
#include	"combat.h"
#include	"id.h"
//#include	"c:\fallen\editor\headers\collide.hpp"
//#include	"c:\fallen\editor\headers\map.h"
#include	"statedef.h"
#include	"enter.h"
#include	"pap.h"
#include	"supermap.h"
#include	"ns.h"
#include	"pcom.h"
#include	"walkable.h"
#include	"memory.h"
#include	"sound.h"
#include	"mav.h"

#ifdef TARGET_DC
#include "DIManager.h"
#endif

//---------------------------------------------------------------
// This is all temporary editor reliant stuff.


//#include	"..\Editor\Headers\Thing.h"
//#include	"..\Editor\Headers\engine.h"

//#include	"c:\fallen\editor\headers\prim_draw.h"
/*
struct	SVector
{
	SLONG	X,Y,Z;
};
*/				   

SLONG	calc_height_at(SLONG x,SLONG z);

void	fn_darci_init(Thing *t_thing);

#define	GRAVITY		((4<<8))

//extern	SLONG	calc_height_on_face(SLONG x,SLONG z,SLONG face);
extern	SLONG	set_person_kick_off_wall(Thing	*p_person,SLONG col,SLONG set_pos);
extern	void	add_damage_value_thing(Thing *p_thing,SLONG value);
extern	void	locked_anim_change(Thing *p_person,UWORD locked_object,UWORD anim,SLONG dangle=0);


/*
 Tables we need

  for each person id a chunk address

  for each person, for each move a frame pointer or 0 if move not supported

  Darci
  Roper
  Cop

  Soldier
  Rasters
  triads
  Mafia
  Dockers
  Unknown
  Tramp
  Fat Bloke Inside Buildings
  Child


  1. a table to find the anim number for a particular person
     i.e ANIM_HIT1 is an index into anim_array but each 


*/


//---------------------------------------------------------------
extern	void	fn_person_moveing(Thing *p_person);
extern	void	fn_person_idle(Thing *p_person);

StateFunction	darci_states[]	=
{
	{	STATE_INIT,				fn_darci_init	},
	{	STATE_NORMAL,			NULL	},
	{	STATE_HIT,				NULL			},
	{	STATE_ABOUT_TO_REMOVE,	NULL			},
	{	STATE_REMOVE_ME,		NULL			},
	{	STATE_MOVEING,			fn_person_moveing},
	{	STATE_IDLE,				fn_person_idle	}

};



//---------------------------------------------------------------

void	fn_darci_init(Thing *t_thing)
{

	//
	// Angle set already when the person was created.
	// OnFace set when person created too.
	//

	t_thing->DrawType					=	DT_ROT_MULTI;
	t_thing->Draw.Tweened->Roll			=	0;
	t_thing->Draw.Tweened->Tilt			=	0;
	t_thing->Draw.Tweened->AnimTween	=	0;
	t_thing->Draw.Tweened->TweenStage	=	0;
	t_thing->Draw.Tweened->NextFrame	=	NULL;
	t_thing->Draw.Tweened->QueuedFrame	=	NULL;
	t_thing->Draw.Tweened->TheChunk		=	&game_chunk[0];
	t_thing->Draw.Tweened->FrameIndex	=	0;
	t_thing->Draw.Tweened->Flags		=	FLAGS_DRAW_SHADOW;
//	t_thing->Genus.Person->Health		=	200;

	set_anim(t_thing,ANIM_STAND_READY);
	set_person_idle(t_thing);
	add_thing_to_map(t_thing);
}

//---------------------------------------------------------------
void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);


KeyFrame*	advance_keyframe(KeyFrame *frame,SLONG count)
{
	LogText(" advance to frame %d \n",count);
//	return(frame);
	while(count&&frame->NextFrame)
	{
		frame=frame->NextFrame;
		count--;
	}
	return(frame);
}

#define	DIR_FORWARD		(1<<0)
#define	DIR_BACKWARD	(1<<1)
#define	DIR_LEFT		(1<<2)
#define	DIR_RIGHT		(1<<3)


ULONG	move_thing(SLONG m_dx,SLONG m_dy,SLONG m_dz,struct MapThing *p_thing);
static	SLONG	air_walking=0;

static	THING_INDEX	history_thing[100];
static	SWORD	history=0;


SLONG	do_floor_collide(Thing *p_thing,SWORD	pelvis,SLONG *new_y,SLONG *foot_y,SLONG max_range)
{
	SLONG	x,y,z;
	SLONG	floor_y;

	calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,pelvis?0:3,&x,&y,&z);
	*foot_y=y;

	//
	// So the feet don't poke through walls.
	//

	x >>= 2;
	z >>= 2;

	x+=(p_thing->WorldPos.X)>>8;
	y+=(p_thing->WorldPos.Y)>>8;
	z+=(p_thing->WorldPos.Z)>>8;
#if !defined(PSX) && !defined(TARGET_DC)
	if (p_thing->Flags & FLAGS_IN_SEWERS)
	{
		floor_y =	NS_calc_height_at(x,z);
	}
	else
#endif
	{
		floor_y	=	PAP_calc_height_at_thing(p_thing,x,z);
	}
	*new_y=floor_y;

	if(y<=(floor_y+9) && (floor_y-y)<max_range)
	{
		MSG_add(" feet collide  y %d floory %d\n",y,floor_y);
		return(1);
	}
	else
	return(0);
}

SLONG	predict_collision_with_floor(Thing *p_thing,SWORD pelvis,SLONG *new_y,SLONG *foot_y)
{
	SLONG	ret;
	GameCoord	temp_pos;
	SLONG	temp_velocity,temp_dy;
	SLONG	c0;
	SLONG	dx,dy,dz;

//	MSG_add(" START COLLIDE dy %d posy %d ",p_thing->DY,p_thing->WorldPos.Y);


	temp_pos=p_thing->WorldPos;
	temp_velocity=p_thing->Velocity;
	temp_dy=p_thing->DY;
	ret=do_floor_collide(p_thing,pelvis,new_y,foot_y,10000);
	if(ret)
		return(ret);

//	for(c0=0;c0<3;c0++)
//	if(0)
	{
		SLONG	dx,dy,dz;
		dx = (SIN(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>8; //was 16
		dz = (COS(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>8;
		dy = p_thing->DY;

		dx = (dx*TICK_RATIO)>>(TICK_SHIFT);
		dy = (dy*TICK_RATIO)>>(TICK_SHIFT);
		dz = (dz*TICK_RATIO)>>(TICK_SHIFT);

		p_thing->Velocity+=p_thing->DeltaVelocity;
		p_thing->DY-=(GRAVITY*TICK_RATIO)>>TICK_SHIFT; //gravity
		if(p_thing->DY<-30000)
			p_thing->DY=-30000;

//		MSG_add(" dxyz %d %d %d \n",dx,dy,dz);

		p_thing->WorldPos.X	-=	dx; //(SIN(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>16;
		p_thing->WorldPos.Z	-=	dz; //(COS(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>16;
		p_thing->WorldPos.Y	+=	dy; //p_thing->DY;

	}


//	if(MAV_HEIGHT(temp_pos.WorldPos.X>>16,temp_pos.WorldPos.Z>>16	
	ret=do_floor_collide(p_thing,pelvis,new_y,foot_y,128);

	p_thing->Velocity=temp_velocity;
	p_thing->DY=temp_dy;
	p_thing->WorldPos=temp_pos;
	return(ret);
}

extern	SLONG	find_face_near_y(MAPCO16 x,MAPCO16 y,MAPCO16 z, SLONG ignore_faces_of_this_building,Thing *p_person,SLONG neg_dy,SLONG pos_dy,SLONG *ret_y);
extern	SLONG nearest_point_on_line_and_dist(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z);

SLONG	predict_collision_with_face(Thing *p_thing,SLONG wx,SLONG wy,SLONG wz,SWORD	pelvis,SLONG *new_y,SLONG *foot_y)
{
	SLONG	ret;
	GameCoord	temp_pos;
	SLONG	temp_velocity,temp_dy;
	SLONG	c0;

	SLONG	dx,dy,dz,fx,fy,fz;

	SLONG ignore_building;
	if(p_thing->DY>0)
		return(0);

	//
	// The thing index whose faces we ignore.
	//

	if (p_thing->Flags & FLAGS_IN_BUILDING)
	{
		ignore_building = INDOORS_DBUILDING;
	}
	else
	{
		ignore_building = NULL;
	}

	
	calc_sub_objects_position(
		p_thing,
		255, //p_thing->Draw.Tweened->AnimTween,
		pelvis?0:SUB_OBJECT_LEFT_FOOT,
	   &fx,
	   &fy,
	   &fz);

	if(!pelvis)
	{
		//
		// Get rid of the (x,z) offset of the foot. This fixes the bug where
		// Darci falls through the ground if she is jumping and sliding against
		// a wall.  (Her foot was poking through the wall in front of her.)
		//

		fx = 0;   // maybe shifting by 2 will be even better
		fz = 0;	// Remember to change the corresponding code in plant_feet()

	}


	*foot_y=fy;
	//
	// you can overshoot by your velocity or be 5 short of landing on it
	//

	//ret=find_face_near_y(wx+fx,wy+fy,wz+fz, ignore_building,p_thing,-(abs(p_thing->DY>>8)),5);
	{
		SLONG	min_y,max_y;
		max_y=abs(p_thing->DY>>9);
		min_y=-max_y;
		if(max_y<5)
			max_y=5;
		if(min_y>-100)
			min_y=-100;

		MSG_add(" vel>>1 %d min_y %d max-y %d \n",abs(p_thing->DY>>9),min_y,max_y);


		ret=find_face_near_y(wx+fx,wy+fy,wz+fz, ignore_building,p_thing,min_y,max_y,new_y);
	}



	if(ret==GRAB_FLOOR)
		ret=0;
	else
	{
		//
		// set person pos to be on face
		//


	}
/*
	if(ret)
	{
		CBYTE	str[100];
		sprintf(str," FOUND face at %d y %d \n",ret,(*new_y));
		CONSOLE_text(str);
	}
*/

//		if(ret)
//			break;


	return(ret);
}

SLONG	col_is_fence(SLONG	col)
{
	/*
	SLONG	face;

	face=col_vects[col].Face;
	if(face>0)
	{
		if(prim_faces4[face].Type==FACE_TYPE_FENCE)
			return(1);
	}
	*/
	
//	if(col_vects[col].PrimType==STOREY_TYPE_FENCE || col_vects[col].PrimType==STOREY_TYPE_FENCE_FLAT)
//	if(dfacets[col].FacetType==STOREY_TYPE_FENCE || dfacets[col].FacetType==STOREY_TYPE_FENCE_FLAT)
	if(dfacets[col].FacetType==STOREY_TYPE_FENCE || dfacets[col].FacetType==STOREY_TYPE_FENCE_FLAT || dfacets[col].FacetType==STOREY_TYPE_FENCE_BRICK)
	{
//		MSG_add(" col vect %d is fence \n",col);
		return(1);
		
	}
	else
		return(0);
}

inline BOOL	MagicFrameCheck(Thing *p_person, UBYTE frameindex) {
	if (p_person->Draw.Tweened->FrameIndex>=frameindex) {
		if (!(p_person->Genus.Person->Flags2&FLAG2_SYNC_SOUNDFX)) {
			p_person->Genus.Person->Flags2|=FLAG2_SYNC_SOUNDFX;
			return TRUE;
		}
	} else {
		p_person->Genus.Person->Flags2&=~FLAG2_SYNC_SOUNDFX;
	}
	return FALSE;

}

void	set_person_in_building_through_roof(Thing *p_person,SLONG face)
{
	SLONG	building,storey,wall,best_storey=0;
#ifdef	POO
	wall=prim_faces4[face].ThingIndex;
	if(wall<0)
	{
	}
	else
		return;

	storey=wall_list[-wall].StoreyHead;
	while(storey_list[storey].Prev)
	{
		storey=storey_list[storey].Prev;
		if(storey_list[storey].StoreyType==STOREY_TYPE_NORMAL)
			break;
	}

	if(storey_list[storey].StoreyType==STOREY_TYPE_NORMAL)
	{
		building=storey_list[storey].BuildingHead;

		/*
		storey=ENTER_get_ground_floor(building);
		if(storey)
		{
			p_person->WorldPos.Y-=200<<8;

			if (ENTER_setup(building, storey, TRUE))
			{
				p_person->Flags     |= FLAGS_IN_BUILDING;
				GAME_FLAGS |= GF_INDOORS;



			}
		}

		*/
	}
#endif
}


SLONG	damage_person_on_land(Thing *p_thing)
{
	SLONG sound;
	SLONG damage;

	StopScreamFallSound(p_thing); // just in case

	//
	// When Darci lands she makes a sound depending on how
	// hard she hit the ground.
	//

	     if (p_thing->DY < -20000) {sound = PCOM_SOUND_DROP_BIG;}
	else if (p_thing->DY < -10000) {sound = PCOM_SOUND_DROP_MED;}
	else if (p_thing->DY <  -5000) {sound = PCOM_SOUND_DROP;}
	else                           {sound = NULL;}

	if (sound)
	{
		PCOM_oscillate_tympanum(
			sound,
			p_thing,
			p_thing->WorldPos.X >> 8,
			p_thing->WorldPos.Y >> 8,
			p_thing->WorldPos.Z >> 8);

		//
		// Actually do the sound...
		//	

		// <GRUNT GRUNT GRUNT> :)
	}

	if (p_thing->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL)
	{
		//
		// Only the player can hurt this person.
		//
	}
	else
	if (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE)
	{
		//
		// Nothing hurts this person!
		//
	}
	else
	if (p_thing->Genus.Person->pcom_ai_state==PCOM_AI_STATE_FOLLOWING)
	{
	}
	else
	{
		if (p_thing->DY <= -30000)
		{
			damage = 250;	// Kill the person
			StopScreamFallSound(p_thing);
		}
		else
		{
			damage = -p_thing->DY - 20000;

			if (damage < 0)
			{
				return(0);
			}

			damage = damage / 100;
			PainSound(p_thing);
		}

		p_thing->Genus.Person->Health -= damage;
#ifdef PSX
		if (p_thing->Genus.Person->PlayerID)
		{
			PSX_SetShock(0,damage+48);
		}
#endif
#ifdef TARGET_DC
		if (p_thing->Genus.Person->PlayerID)
		{
			Vibrate ( 5, (float)(damage+48) * 0.01f, 0.0f );
		}
#endif

//		add_damage_value_thing(p_thing,damage>>1);

		if (p_thing->Genus.Person->Health <= 0)
		{
			//
			// sets us up in the right spot to change anim to dead
			//

			//void locked_anim_change(Thing *p_person,UWORD locked_object,UWORD anim);

			//locked_anim_change(p_thing,SUB_OBJECT_LEFT_FOOT,ANIM_STAND_READY);

			set_person_dead(
				p_thing,
				NULL,
				PERSON_DEATH_TYPE_LAND,
				FALSE,
				0);

			return(1);
		}
	}

	return(0);
}


extern SLONG actual_sliding;
extern SLONG last_slide_dist;

UBYTE just_started_falling_off_backwards;

SLONG	projectile_move_thing(Thing *p_thing,SLONG flag)
{
	GameCoord	new_position;
	DrawTween		*draw_info;
	SLONG	face;
	SLONG	ret=0;
	SLONG	col=0;

	SLONG	dx,dy,dz;

	LogText(" projectile\n");

	draw_info	 = p_thing->Draw.Tweened;
	new_position = p_thing->WorldPos;

	dx = (SIN(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>8;
	dz = (COS(p_thing->Draw.Tweened->Angle)*p_thing->Velocity)>>8;  //fixed 8
	dy = p_thing->DY;

	dx = (dx*TICK_RATIO)>>TICK_SHIFT;
	dy = (dy*TICK_RATIO)>>TICK_SHIFT;
	dz = (dz*TICK_RATIO)>>TICK_SHIFT;


	if(p_thing->Genus.Person->Ware)
	{
		SLONG	px,py,pz,wy;
		//
		// slide along warehouse roof
		//

		calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&px,&py,&pz);
		px+=(dx+p_thing->WorldPos.X)>>8;
		py+=(dy+p_thing->WorldPos.Y)>>8;
		pz+=(dz+p_thing->WorldPos.Z)>>8;

		wy=MAVHEIGHT(px>>8,pz>>8)<<6;
		if((py)+80 > wy)
			dy-=((py)+80 - wy)<<8;

	}
	
	ASSERT(p_thing->Class == CLASS_PERSON);

	p_thing->Genus.Person->SlideOdd = 0;

	if (flag & (1|8)) // check for walls
	{
		SLONG	x1,x2,y1,y2,z1,z2;
		{
			{
				//
				// Work out the position of the pelvis.
				//

				calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,0,&x1,&y1,&z1);

				x1 <<= 8;
				y1 <<= 8;
				z1 <<= 8;

				x1 += p_thing->WorldPos.X;
				y1 += p_thing->WorldPos.Y;
				z1 += p_thing->WorldPos.Z;

				//
				// The end of our movement vector.
				//

				x2 = x1 - dx;
				y2 = y1 + dy;
				z2 = z1 - dz;

				if(x2<0||(x2>>16)>127 || z2<0 || (z2>>16)>127)
					return(0);

#if !defined(PSX) && !defined(TARGET_DC)
				if (p_thing->Flags & FLAGS_IN_SEWERS)
				{
					//
					// Slide along the walls of the sewer as you jump.
					//

					NS_slide_along(x1,y1,z1,&x2,&y2,&z2,50);

					//
					// Pretend you never collide.
					//

					col = 0; // This means we can't do any funcky kicks or anything
							 // in the sewers :o(
				}
				else
#endif
				{
					if (!(flag & 8))
					{
						//
						// Collide with things.
						//

						collide_against_things(
							 p_thing,
							 50,
							 x1,  y1,  z1,
							&x2, &y2, &z2);

						if (p_thing->State == STATE_DYING)
						{
							//
							// Darci died while colliding with things.
							//

							return 100;
						}

						//
						// Collision with OB_jects.
						//

						collide_against_objects(
							p_thing,
							30,
							x1,  y1,  z1,
						   &x2, &y2, &z2);
					}

					//
					// Collision with walls.
					//
#ifndef PSX
					if (p_thing->Genus.Person->InsideIndex)
					{
						person_slide_inside(
							p_thing->Genus.Person->InsideIndex,
							x1, y1, z1,
						   &x2,
						   &y2,
						   &z2);
					}
					else
#endif
					{
						SLONG extra_wall_height = 0;

						extra_wall_height = 0; //-p_thing->DY << 3;

						SATURATE(extra_wall_height, 0, 128);

						MSG_add(" extra wall height %d y1 %d y2 %d\n",extra_wall_height,y1,y2);

						if (flag & 8)
						{
							just_started_falling_off_backwards = TRUE;
							
						}

						slide_along(
							x1,y1,z1,
							&x2,&y2,&z2,
							extra_wall_height,			// Stop Darci going through walls- the extra height added to a wall height.
							50,SLIDE_ALONG_FLAG_JUMPING);

						just_started_falling_off_backwards = FALSE;

//
// This next bit of code is probably redundant as we can no longer jump through doors, but It cant do any harm to leave it in (apart from code bloat)
//
						extern SLONG slide_into_warehouse;
						extern SLONG slide_outof_warehouse;
						
						if (slide_into_warehouse)
						{
							p_thing->Genus.Person->Flags |= FLAG_PERSON_WAREHOUSE;
							p_thing->Genus.Person->Ware   = dbuildings[slide_into_warehouse].Ware;
						}

						if (slide_outof_warehouse)
						{
							p_thing->Genus.Person->Flags &= ~FLAG_PERSON_WAREHOUSE;
							p_thing->Genus.Person->Ware   =  0;
						}
					}

					//
					// The collision vector we sliding over.
					//

					if(actual_sliding)
					{
						col = last_slide_colvect;
					}
					else
					{
						col = 0;
					}
				}

				dx = -(x2-x1);
				dz = -(z2-z1);
			}

			if(col)
			{
				#if 0

				//
				// I hope we get this working one day...
				//

				//
				// Do special moves off a wall.
				//

				if ((p_thing->Genus.Person->Flags & FLAG_PERSON_REQUEST_KICK))
				{
					if(set_person_kick_off_wall(p_thing,col,1))
					{
						return(2);
					}
				}
				else
				if((p_thing->Genus.Person->Flags&FLAG_PERSON_REQUEST_JUMP))
				{
					if(set_person_kick_off_wall(p_thing,col,1))
					{
						return(2);
					}
				}
				else

				#endif
				if(col_is_fence(col) || dfacets[col].FacetType == STOREY_TYPE_NORMAL)
				{
					//
					// The person has hit a fence.
					//

					if (dfacets[col].FacetFlags & FACET_FLAG_UNCLIMBABLE)
					{
						//
						// You can't land on this fence- so we'll just slide
						// along it instead.
						//
					}
					else
					{

						if(set_person_land_on_fence(p_thing,col,1))
						{
							return(2);
						}
					}
				}
			}

			if(col)
			{
				MSG_add(" HIT wall !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}
		}
	}
	

	new_position.X	-=	dx;
	new_position.Z	-=	dz;
	new_position.Y	+=	dy;

	if(new_position.X<0||(new_position.X>>16)>127 || new_position.Z<0 || (new_position.Z>>16)>127)
		return(0);

	SATURATE(new_position.X, 0, (PAP_SIZE_HI << 16) - 1);
	SATURATE(new_position.Z, 0, (PAP_SIZE_HI << 16) - 1);

	p_thing->Velocity+=p_thing->DeltaVelocity;
//	p_thing->DY-=GRAVITY; //gravity
	p_thing->DY-=(GRAVITY*TICK_RATIO)>>TICK_SHIFT; //gravity


	if(p_thing->DY<-30000) {
		p_thing->DY=-30000;				// hit terminal velocity... must be scary... so...
	}

	/*
	if(p_thing->DY==-(40<<8) && p_thing->Velocity>10)
	{

		locked_anim_change(p_thing,0,ANIM_FALL_LONG);

	}
	*/

	LogText(" projectile move  vel %d dy %d \n",p_thing->Velocity,p_thing->DY);

//	if(p_thing->Timer1>3)
	if(flag&(2|4)) // under feet
	{
		SLONG	new_y,on_face,foot_y;
		if(face=predict_collision_with_face(p_thing,new_position.X>>8,new_position.Y>>8,new_position.Z>>8,flag&4,&new_y,&foot_y))
		{
			//ASSERT(face>0);

			{
#ifndef PSX
				if(!(GAME_FLAGS & GF_INDOORS))
				if(face>0 && prim_faces4[face].Type==FACE_TYPE_SKYLIGHT)
				{
					ASSERT(0);
					//
					// fall through skylight into building
					//

					set_person_in_building_through_roof(p_thing,face);
					return(0);
				}
#endif
			}
			if(flag&4)     //falling with pelvis as lowest point
				return(1);
			//on_face=calc_height_on_face(new_position.X>>8,new_position.Z>>8,face,&new_y);
			MSG_add(" projectile move hit face %d new y %d pelvis col %d\n",face,new_y,flag&4);
/*
			if(!on_face)
			{
				ASSERT(0);
				//new_y=p_thing->WorldPos.Y>>8;   //use new_y anyway!!!
			}
*/

			p_thing->OnFace=face;

			p_thing->Flags&=~FLAGS_PROJECTILE_MOVEMENT;

			if((flag&4)==0)	 
			{
				//
				// foot collsion with face
				//
				new_position.Y=(new_y-foot_y+5)<<8;

				if (p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START ||
					p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_FORWARDS)
				{
					//new_position.Y   = PAP_calc_map_at_thing(p_thing,new_position.X>>8,new_position.Z>>8);
					new_position.Y   = PAP_calc_map_height_at(new_position.X>>8,new_position.Z>>8);
					new_position.Y  -= 0x50;
					new_position.Y <<= 8;
				}

	//
	// I've put this back in, because we were exiting and not finding ourselves on the face 
	//
				move_thing_on_map(p_thing,&new_position);
			}

			// hurt person
			if(damage_person_on_land(p_thing))
				return(100);

			return(1);
		}
		else
		if(predict_collision_with_floor(p_thing,flag&4,&new_y,&foot_y))
		{
			if(flag&4)
				return(1);
			MSG_add(" predict collision with FLOOR");

			ret=1;

#if !defined(PSX) && !defined(TARGET_DC)
			if (p_thing->Flags & FLAGS_IN_SEWERS)
			{
				p_thing->WorldPos.Y	=	NS_calc_height_at(new_position.X>>8,new_position.Z>>8)<<8;
			}
			else
#endif
			{
				if((flag&4)==0)	 
				{
					//
					// foot collsion with face
					//

					new_position.Y=(new_y-foot_y+5)<<8;

					if (p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START ||
						p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_FORWARDS)
					{
						new_position.Y   = PAP_calc_height_at_thing(p_thing,new_position.X>>8,new_position.Z>>8);
						new_position.Y  -= 0x50;
						new_position.Y <<= 8;
					}

					//
					// I've put this back in, because we were exiting and not finding ourselves on the face 
					//

						
				}
				else
				{
					new_position.Y=PAP_calc_height_at_thing(p_thing,new_position.X>>8,new_position.Z>>8)<<8;
					new_position.X=p_thing->WorldPos.X;
					new_position.Z=p_thing->WorldPos.Z;
				}
/*
				if(p_thing->WorldPos.Y>(new_position.Y+(32<<8)))
				{
					return(0);

				}
*/

				//
				// foot can collide roof top 
				//

				move_thing_on_map(p_thing,&new_position);
			}
			p_thing->OnFace=0;
			p_thing->Flags&=~FLAGS_PROJECTILE_MOVEMENT;
			LogText(" projectile move hit floor new y %d \n",p_thing->WorldPos.Y);
			if(damage_person_on_land(p_thing))
				return(100);
			return(ret);

		}

	}
	
	//
	// Actually move the person.
	//

	move_thing_on_map(p_thing,&new_position);

	//
	// Hit some barrels.
	//

	BARREL_hit_with_sphere(
		p_thing->WorldPos.X >> 8,
		p_thing->WorldPos.Y >> 8,
		p_thing->WorldPos.Z >> 8,
		0x70);

	//
	// Is this person going to fall to his death?
	//

	SLONG death_check;

	if (p_thing->Genus.Person->PlayerID)
	{
		death_check = -12000;
	}
	else
	{
		death_check = -6000;
	}

	if (GAME_FLAGS & GF_NO_FLOOR)
	{
		if (p_thing->WorldPos.Y < 0x0 && p_thing->DY < -6000)
		{
			//
			// Stop Roper popping back onto buildings by falling underneath them.
			//

			p_thing->Velocity = 0;
		}
	}

	if (p_thing->DY < death_check)
	{
		if ((GAME_TURN & 0x3)==THING_NUMBER(p_thing))
		{
			if (p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_START ||
				p_thing->Draw.Tweened->CurrentAnim == ANIM_PLUNGE_FORWARDS)
			{
				//
				// Already falling to your death...
				//
				if (MagicFrameCheck(p_thing,2)) 
				{
					ScreamFallSound(p_thing);	// scream yer lungs out!
				}

				p_thing->Velocity = 0;
			}
			else
			{
				SLONG i;

				SLONG mx;
				SLONG mz;

				SLONG height;

				//
				// Is this person going to die?
				//

				for (i = 1; i < 4; i++)
				{
					mx = (p_thing->WorldPos.X >> 8) - (dx * i >> 5);
					mz = (p_thing->WorldPos.Z >> 8) - (dz * i >> 5);

					if (WITHIN(mx, 0, (PAP_SIZE_HI << 8) - 1) &&
						WITHIN(mz, 0, (PAP_SIZE_HI << 8) - 1))
					{
						height = PAP_calc_map_height_at(mx,mz);

						if (height >= (p_thing->WorldPos.Y >> 8) - 0x600)
						{
							//
							// No death!
							//

							/*

							AENG_world_line(
								p_thing->WorldPos.X >> 8,
								p_thing->WorldPos.Y >> 8,
								p_thing->WorldPos.Z >> 8,
								0x10,
								0xffffff,
								p_thing->WorldPos.X           >> 8,
								p_thing->WorldPos.Y + 0x10000 >> 8,
								p_thing->WorldPos.Z           >> 8,
								0x0,
								0xffeeee,
								FALSE);

							*/

							goto no_death;
						}
					}
				}

				//
				// This person is going to die!
				//

				/*

				AENG_world_line(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x10,
					0xff0000,
					p_thing->WorldPos.X           >> 8,
					p_thing->WorldPos.Y + 0x10000 >> 8,
					p_thing->WorldPos.Z           >> 8,
					0x0,
					0xaacc00,
					FALSE);

				*/

				locked_anim_change(p_thing, SUB_OBJECT_PELVIS, ANIM_PLUNGE_START);

/*				if (MagicFrameCheck(p_thing,0)) 
				{
					ScreamFallSound(p_thing);	// scream yer lungs out!
				}*/

			  no_death:;
			}
		}
	}

	return(ret);

}

//could be logarithmic/ linear/ stepped
void	change_velocity_to(Thing *p_thing,SWORD velocity)
{
	SLONG	dv;

	velocity=(velocity*3)>>2; // fps required , fps used when setting up these values

	dv=velocity-p_thing->Velocity;
	if(dv<0)
	{
		if(dv>-2)
			p_thing->Velocity=velocity;
		else
			p_thing->Velocity+=(dv>>1);

	}
	else if(dv>0)
	{
		if(dv<2)
			p_thing->Velocity=velocity;
		else
			p_thing->Velocity+=(dv>>1);
	}
}

void	change_velocity_to_slow(Thing *p_thing,SWORD velocity)
{
	SLONG	dv;

	velocity=(velocity*3)>>2; // fps required , fps used when setting up these values

	dv=velocity-p_thing->Velocity;
	if(dv<0)
	{
		if(dv>-2)
			p_thing->Velocity=velocity;
		else
			p_thing->Velocity-=1;//(dv>>3);

	}
	else if(dv>0)
	{
		if(dv<2)
			p_thing->Velocity=velocity;
		else
			p_thing->Velocity+=1; //(dv>>3);
	}
}

void	trickle_velocity_to(Thing *p_thing,SWORD velocity)
{
	SLONG	dv;

	velocity=(velocity*3)>>2; // fps required , fps used when setting up these values

	dv=velocity-p_thing->Velocity;
	if(dv<0)
	{
		p_thing->Velocity--;

	}
	else if(dv>0)
	{
		p_thing->Velocity++;
	}
}

void	set_thing_velocity(Thing *t_thing,SLONG vel)
{
	vel=(vel*3)>>2; // fps required , fps used when setting up these values

	t_thing->Velocity=vel;
}


#define	REQUIRED_DIST_JUMP_GRAB	35

extern	SLONG dist_to_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b);
extern	void nearest_point_on_line(	SLONG x1, SLONG z1,	SLONG x2, SLONG z2,	SLONG a,  SLONG b,SLONG *ret_x,SLONG *ret_z);
extern	void calc_things_height(struct MapThing *p_thing); //editor\collide.c
//extern	struct	CollisionVect		col_vects[];
#ifdef	DOG_POO
SLONG	setup_person_for_jump_grab(Thing *p_thing)
{
	SLONG col,dist;
	SLONG	x,z;
	struct	CollisionVect	*p_vect;
	GameCoord	new_position;
	SLONG	m_dx,m_dy,m_dz;
	new_position	=	p_thing->WorldPos;


	m_dx=-((SIN(p_thing->Draw.Tweened->Angle)*REQUIRED_DIST_JUMP_GRAB)>>16);
	m_dz=-((COS(p_thing->Draw.Tweened->Angle)*REQUIRED_DIST_JUMP_GRAB)>>16);
	m_dy=0;
	col=check_vect_vect(m_dx,m_dy,m_dz,p_thing,2);
//	if(!col)
//		col=check_vect_circle(0,0,0,p_thing,80);
//	LogText(" set person for jump col_vect %d \n",col);

	x=p_thing->WorldPos.X>>8;
	z=p_thing->WorldPos.Z>>8;

	if(col)
	{
		SLONG	new_x,new_z,angle;
		SLONG	dx,dz;

		angle=get_point_dist_from_col_vect(col,x,z,&new_x,&new_z,REQUIRED_DIST_JUMP_GRAB);
		LogText(" move person was %d %d now becomes %d %d\n",x,z,new_x,new_z);

		p_thing->Draw.Tweened->Angle=angle;

		new_position.X=new_x<<8;
		new_position.Z=new_z<<8;

		move_thing_on_map(p_thing,&new_position);

		return(1);

	}
	return(0);

}
#endif

extern	void	highlight_face(SLONG face);
extern	void	e_draw_3d_mapwho(SLONG x1,SLONG z1);

void	show_walkable(SLONG mx,SLONG mz)
{
	SLONG	index;
	SLONG	check_face;

	return;
/*

	index=MAP[MAP_INDEX(mx,mz)].Walkable;
	if(index)
		e_draw_3d_mapwho(mx,mz);

	while(index)
	{
		check_face=walk_links[index].Face;
		if(check_face>0)
			highlight_face(check_face);

		index=walk_links[index].Next;
	}
*/
}



//---------------------------------------------------------------

/*

  What Is players Input?

  Direction  +  Punch/Kick/Action/Block/Jump/Crouch/Mode Change

  Direction  (Forwards/Backwards/Left/Right)


  What is players current status?

  (Fast Mode)Run/(Stealth Mode)Walk/(combat mode) fighting

  Sub Status
  Standing/Crouching/Jumping/Climbing/Vaulting/PullingUp/ Shooting/Swimming

  What scenery is player able to interact with

  (MOVEMENT FAST)
	X to Jump Up and vault over all one move ( a high fence)
	X to jump up grab swing onto and continue running
	X to fault over (a small fence)
	Edge to jump from (long Jump)
	steps/slope to move up/down
	Wall to step up and back flip off
	Post to grab and swing arround


  (MOVEMENT STEALTH)
    X to pull up and stand on (1 storey building)
	Wall to sidle along
	Edge to drop down from (1 storey building)



	

Code For pull up onto Storey

  1. Detect Wall, face to stand on at end
  2. Face wall At correct angle for anim
  3. Play Anim to take you up wall
  4. At end of anim
	a. Go to stand anim
	b. Relocate players position so anim change leaves a foot in same place
	c. Set player to be on face.

Code for pull up onto multi height storey

  See 1& 2 above
  3. Play jump anim, cut short or prolong until at correct position for hands to grasp ledge
  see 4 above


Alternate Method for Pull Up Onto Storey

  Break Movement into 3 parts

  jump to dangle position,

  from dangle to pull up onto knee

  from knee to standing.


  Dangle Options
	Swing (If Available)
	Pull Up Onto Knee
	Drop back to Floor


  Kneeling on Edge Options
	Stand Up
	Go Back To Dangle
	Rush Forward
	Roll Forward









//to do

  create a collision poly system that allows players to walk/jump/vault/pull up 
  onto faces

	Fire escapes can be walked up onto

	Ledges can be pulled up onto

	Buildings can be jumped accross from one to the other

  Code for detecting the existence of interactible scenery when interaction is requested

  Code for performing the interaction
	

  Kerbs/ Kerbs as roof tops
  Floor Slopes/Floor Slopes as roof tops


  Do we link existing poly's or do we build special collision polys

  Point 
  {
	UWORD x;
	UWORD Y;
	UWORD Z;
  }; //6

  PolyQuad
  {
	Point[4];
	SWORD	PrimFace;
	UWORD	Flags;	//walkable, verticle, vaultable, high,med,flat,

  }; //4*6=24+4 =28

  PolyTri
  {
	Point[3];
	SWORD	PrimFace;
	UWORD	Flags;	//walkable, verticle, vaultable, high,med,flat,
  }; //3*6=18+4 =22



//jump2 at frame 6

//walk and run are now alligned






  
*/
