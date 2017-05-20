#include	"game.h"
#include	"statedef.h"
#include	"pcom.h"
#include	"memory.h"
#include	"wmove.h"
#include	"animate.h"

extern	SLONG	is_person_dead(Thing *p_person);

#define	GUN_RANGE_BLOCKS	13

#define	GUN_ANGLE_RANGE		64

extern	SLONG	people_allowed_to_hit_each_other(Thing *p_victim,Thing *p_agressor);

SLONG	get_gun_aim_stats(Thing *p_person,SLONG *range,SLONG *spread)
{
	if (p_person->Genus.Person->PersonType == PERSON_MIB1 ||
		p_person->Genus.Person->PersonType == PERSON_MIB2 ||
		p_person->Genus.Person->PersonType == PERSON_MIB3)
	{
		//
		// MIB are ninja shooting machines with built-in AK47s.
		//

	   *range  = 8 << 8;
	   *spread = 20;

		return TRUE;
	}

	if (p_person->Genus.Person->Flags&FLAG_PERSON_GUN_OUT)
	{
		*range=7<<8;
		*spread=15;
		return(1);

	}
	else
	if (p_person->Genus.Person->SpecialUse)
	{
		Thing *p_special = TO_THING(p_person->Genus.Person->SpecialUse);

		//
		// Using a special.
		//

		switch (p_special->Genus.Special->SpecialType)
		{
			case	SPECIAL_AK47:
				*range=8<<8;
				*spread=20;
				return(1);

			case	SPECIAL_SHOTGUN:
				*range=6<<8;
				*spread=30;
				return(1);

			default:

				//
				// This isn't a weapon you shoot!
				//

				return(0);
		}

	}
	return(0);


}

/*

//
// spread is Angle Range  +/-  for p_person to be aiming at p_target
//

//
// range is ideal range of weapon (can be used upto twice ideal range) 
//
SLONG	calc_target_score(Thing *p_person,Thing *p_target,SLONG range,SLONG spread)
{
	SLONG	dx,dy,dz,dist;
	SLONG	angle;
	SLONG	angle_diff;
	SLONG	score;
	SLONG	adx,ady,adz;


	dx=p_person->WorldPos.X-p_target->WorldPos.X>>8;
	if(abs(dx) > range<<1)
		return(-1);

	dy=p_person->WorldPos.Y-p_target->WorldPos.Y>>8;
	if(abs(dy) >6*256)
		return(-1);

	dz=p_person->WorldPos.Z-p_target->WorldPos.Z>>8;
 	if(abs(dz) > range<<1)
		return(-1);

	adx=abs(dx);
	ady=abs(dy);
	adz=abs(dz);

	dist=QDIST3(adx,ady,adz);//dx*dx+dy*dy+dz*dz;

	spread=spread+(((range)-dist)>>5); // add bonus spread depend upon range of target (dependant on ideal range of gun) can be negative if beyond ideal range
	if(spread<5)
		spread=5;


	if(dist>range<<1)
		return(-1);

	angle		=	Arctan(dx,-dz); //-1024;
	angle_diff=angle-p_person->Draw.Tweened->Angle;
	angle_diff=(angle_diff+2048)&2047;

	if(angle_diff>= spread && angle_diff<=2048-spread)
		return(-1);
  
	if(angle_diff< spread)
		score=spread-angle_diff; //+dist;
	else
		score=angle_diff-(2048-spread); //+dist;

	score<<=1; //0->128

	score+=((range<<1)-dist)>>5; // +  0->160
  
	if(p_target->State==STATE_DEAD)
		score>>=5;
	if(p_target->State==STATE_DYING)
		score>>=4;

	return(score);
}



THING_INDEX	find_target(Thing *p_person)
{
	THING_INDEX	current_thing,best=0;
	Thing	*t_thing;
	SLONG	score,best_score=-1;
	SLONG	range,spread;


	if(get_gun_aim_stats(p_person,&range,&spread)==0)
	{
		//
		// no target
		//

		return(0);
	}


	current_thing = PRIMARY_USED;

	while(current_thing)
	{
		t_thing	= TO_THING(current_thing);


		if(t_thing->Class==CLASS_PERSON&& t_thing!=p_person && !is_person_dead(t_thing))
		{
			if (t_thing->Genus.Person->Flags & FLAG_PERSON_DRIVING)
			{
				//
				// Don't target driving people.
				//
			}
			else
			{
				//
				// range is an ideal value, double it for maximum range of weapon
				//
				if(can_a_see_b(p_person,t_thing,-(range<<1)))
				{
					score=calc_target_score(p_person,t_thing,range,spread);

					if(score>best_score)
					{
						best_score=score;
						best=current_thing;
					}
				}
			}
		}
		current_thing	=	t_thing->LinkChild;
	}

	return(best);
}

*/

//
// This new function only works for players. Enemies know who they
// should be hitting.
// 

#define MAX_NEW_TARGET_DANGLE (2048 / 7)

extern	SLONG	look_pitch;

SLONG calc_target_score_new(Thing *darci, Thing *p_target)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG angle;
	SLONG dangle;
	SLONG score;

	ASSERT(darci->Genus.Person->PlayerID);

	//
	// Ignore dead and floating people.
	//

	if (p_target->State == STATE_DEAD ||
		p_target->State == STATE_FLOAT)
	{
		return 0;
	}

	//
	// How far is the target?
	//

	dx = p_target->WorldPos.X - darci->WorldPos.X >> 8;
	dy = p_target->WorldPos.Y - darci->WorldPos.Y >> 8;
	dz = p_target->WorldPos.Z - darci->WorldPos.Z >> 8;

	dist = QDIST2(abs(dx),abs(dz));

	//
	// Maximum of 45 degrees up/down shooting angle.
	//

	if (abs(dy) > dist)
	{
		return 0;
	}

	if ((p_target->Class==CLASS_PERSON) && 
		!people_allowed_to_hit_each_other(p_target,darci))
	{
		return 0;
	}

	#if 0

	//
	// You can target police cars if you want! If you blow them up,
	// then you get a red mark.
	//

	if (p_target->Class == CLASS_VEHICLE)
	{
		//
		// Don't target police cars if you're Darci or Roper.
		//

		if (darci->Genus.Person->PersonType == PERSON_DARCI ||
			darci->Genus.Person->PersonType == PERSON_ROPER)
		{
			
		}
	}

	if ((p_target->Class==CLASS_VEHICLE) && 
		((p_target->Genus.Vehicle->Type == VEH_TYPE_POLICE) || (p_target->Genus.Vehicle->Type == VEH_TYPE_MEATWAGON)))
	{
		return 0;
	}

	#endif

	//
	// Relative angle of the target from darci.
	//

	angle   = Arctan(dx,-dz) + 1024;
	angle  &= 2047;
	dangle  = angle - darci->Draw.Tweened->Angle;
	dangle &= 2047;

	if (dangle > 1024)
	{
		dangle -= 2048;
	}

	dangle = abs(dangle);	// dangle to the left or right makes no difference

	//
	// Darci can twist around 45 degrees from side to side. Any more than that
	// and she can't target the person.
	//

	if(darci->SubState==SUB_STATE_RUNNING)
	if (dangle > (MAX_NEW_TARGET_DANGLE>>2))
	{
		return(0);
	}

	if (dangle > MAX_NEW_TARGET_DANGLE)
	{
		return 0;
	}

	//
	// You have to be able to see the person!
	// 

	if (p_target->Class == CLASS_PERSON)
	{
		if (!can_a_see_b(darci, p_target))
		{
			return 0;
		}
	}
	else
	{
		if (p_target->Class == CLASS_SPECIAL)
		{
			//
			// We only target mines.
			//

			if (p_target->Genus.Special->SpecialType != SPECIAL_MINE)
			{
				return 0;
			}
		}

		//
		// Simple los for non-people.
		//

		if (!there_is_a_los(
				darci->WorldPos.X          >> 8,
				darci->WorldPos.Y + 0x6000 >> 8,
				darci->WorldPos.Z          >> 8,
				p_target->WorldPos.X          >> 8,
				p_target->WorldPos.Y + 0x3000 >> 8,
				p_target->WorldPos.Z          >> 8,
				LOS_FLAG_IGNORE_PRIMS))
		{
			return 0;
		}
	}

	//
	// The target has a maximum score depending on range. That score is reduced depending
	// on how much 'dangle' they have.
	//

	score = (8 << 8) - dist;

	if (dist <= 0)
	{
		//
		// Too far away...
		//

		return 0;
	}

	score *= MAX_NEW_TARGET_DANGLE - dangle >> 3;

//	if(p_target->Class==CLASS_PERSON)
	if(darci->Genus.Person->PlayerID && (darci->Genus.Person->Flags2&FLAG2_PERSON_LOOK))
	{
		SLONG	angle,dangle;
		//
		// player is in first person look round mode, so take global look_pitch into account
		//

		//
		// look_pitch is a bit odd, its 0 directly infront of you 1900 when looking up and 50 when looking down
		//

		angle   = Arctan(dx*dx+dz*dz,dy*abs(dy)<<1);
		angle  &= 2047;
		angle=((-(angle-512))+2048)&2047;
		dangle  = angle - look_pitch;
		dangle &= 2047;

		if (dangle > 1024)
		{
			dangle -= 2048;
		}

		dangle = abs(dangle);
		SATURATE(dangle,0,128);

		score *= (128 - dangle) >> 2;
		


	}


	//
	// Fiddle with the score to favour certain types of people...
	// 

	if (p_target->Class == CLASS_PERSON)
	{
		if (p_target->Draw.Tweened->CurrentAnim == ANIM_HANDS_UP_LIE)
		{
			//
			// Don't target civs who have 'got down'.
			//

			score = 0;
		}
		else
		if (PCOM_person_wants_to_kill(p_target) == THING_NUMBER(darci))
		{
			score <<= 2;
		}
		else
		{
			if(darci->Genus.Person->PersonType != PERSON_DARCI && darci->Genus.Person->PersonType != PERSON_ROPER)
			{
				//
				// playing a thug
				//

				if (p_target->Genus.Person->PersonType == PERSON_COP  ||
					p_target->Genus.Person->pcom_ai    == PCOM_AI_COP ||
					p_target->Genus.Person->pcom_ai    == PCOM_AI_COP_DRIVER)
				{
					score <<= 1;
				}

			}
			else
			{
				if (p_target->Genus.Person->PersonType == PERSON_COP  ||
					p_target->Genus.Person->pcom_ai    == PCOM_AI_COP ||
					p_target->Genus.Person->pcom_ai    == PCOM_AI_COP_DRIVER)
				{
					score >>= 1;
				}

				switch(p_target->Genus.Person->PersonType)
				{
					case PERSON_THUG_RASTA:
					case PERSON_THUG_GREY:
					case PERSON_THUG_RED:
					case PERSON_MIB1:
					case PERSON_MIB2:
					case PERSON_MIB3:
						
						score += score;
						break;

					default:
						if (p_target->Genus.Person->pcom_ai == PCOM_AI_GANG ||
							p_target->Genus.Person->pcom_ai == PCOM_AI_MIB  ||
							p_target->Genus.Person->pcom_ai == PCOM_AI_GUARD)
						{
							score += score;
						}
						break;
				}
			}
		}

		if (p_target->State == STATE_DYING)
		{
			score >>= 1;
		}

		if (p_target->Genus.Person->PersonType == PERSON_CIV ||
			p_target->Genus.Person->pcom_ai == PCOM_AI_CIV)
		{
			score=(score*180)>>8;

		}
		else
		if(p_target->Genus.Person->PersonType == PERSON_HOSTAGE)
		{
			score=(score*170)>>8;
		}
		else
		if(p_target->Genus.Person->PersonType == PERSON_SLAG_TART||p_target->Genus.Person->PersonType == PERSON_SLAG_FATUGLY)
		{
			score=(score*210)>>8;
		}
	}
	else
	if (p_target->Class == CLASS_BAT)
	{
		//
		// Bats are always agressive.
		//

//		score <<= 1;
	}
	else
	if (p_target->Class == CLASS_BARREL)
	{
		//
		// Shoot barrels rather than cones.
		//

		score=(score*220)>>8; //barrels less than thugs 

		if (p_target->Genus.Barrel->type == BARREL_TYPE_CONE ||
			p_target->Genus.Barrel->type == BARREL_TYPE_BIN)
		{
			//score >>= 2;
			return(0);
		}

		extern SLONG BARREL_is_stacked(Thing *p_barrel);

		if (BARREL_is_stacked(p_target))
		{
			score -= 0x80;	// Less likely to target stacked barrels.
		}
	}
	else
	if (p_target->Class == CLASS_SPECIAL)
	{
		//
		// Nothing bad/good about mines.
		//
	}
	else
	if (p_target->Class == CLASS_VEHICLE)
	{
		//
		// Is Darci standing on this vehicle?
		//

		if (darci->OnFace > 0)
		{
			ASSERT(WITHIN(darci->OnFace, 1, next_prim_face4 - 1));

			PrimFace4 *f4 = &prim_faces4[darci->OnFace];

			ASSERT(f4->FaceFlags & FACE_FLAG_WALKABLE);

			if (f4->FaceFlags & FACE_FLAG_WMOVE)
			{
				SLONG wmove_index = f4->ThingIndex;

				WMOVE_Face *wf;

				ASSERT(WITHIN(wmove_index, 1, WMOVE_face_upto - 1));

				wf = &WMOVE_face[wmove_index];

				if (wf->thing && TO_THING(wf->thing) == p_target)
				{
					//
					// Phew! Darci is standing on this vehicle. Don't target it.
					//

					return 0;
				}
			}
		}
	}
/*
	if (darci->Genus.Person->Target == THING_NUMBER(p_target))
	{
		//
		// It is good to keep targeting the same thing.
		//

		score <<= 1;
	}
*/

	return score;
}





THING_INDEX find_target_new(Thing *p_person)
{
	SLONG i;
	SLONG range;
	SLONG spread;
	SLONG num_found;
	SLONG enemy;
	SLONG score;
	SLONG best_score  = 0;
	SLONG best_target = NULL;

	Thing *p_found;
	Thing *p_enemy;

	if (!get_gun_aim_stats(p_person, &range, &spread))
	{
		//
		// This person hasn't got a gun out.. so no target.
		// 

		return NULL;
	}

	if (p_person->Genus.Person->PlayerID)
	{
		//
		// Find all people, bats and barrels in range of the gun.
		//

		num_found = THING_find_sphere(
						p_person->WorldPos.X >> 8,
						p_person->WorldPos.Y >> 8,
						p_person->WorldPos.Z >> 8,
						range,
						THING_array,
						THING_ARRAY_SIZE,
						(1 << CLASS_PERSON) | (1 << CLASS_BAT) | (1 << CLASS_BARREL) | (1 << CLASS_VEHICLE) | (1 << CLASS_SPECIAL));
		
		for (i = 0; i < num_found; i++)
		{
			p_found = TO_THING(THING_array[i]);

			if (p_found != p_person)
			{
				score = calc_target_score_new(p_person, p_found);

				/*
				{
					CBYTE	str[100];
extern	void FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);
					
					sprintf(str," %d ",score);
					FONT2D_DrawString_3d(str,p_found->WorldPos.X>>8,(p_found->WorldPos.Y>>8)+256,p_found->WorldPos.Z>>8,0xffffff,32,0);
				}
				*/


				if (best_score < score)
				{
					best_score  = score;
					best_target = THING_array[i];
				}
			}
		}

		return best_target;
	}
	else
	{
		//
		// Is this person particularly out to get someone?
		//

		enemy = PCOM_person_wants_to_kill(p_person);

		if (enemy)
		{
			Thing *p_enemy = TO_THING(enemy);

			//
			// We should not be a player here...
			//

			ASSERT(p_person->Genus.Person->PlayerID == 0);

			if (p_enemy->Genus.Person->Flags & FLAG_PERSON_DRIVING)
			{
				//
				// Target their car!
				//

				ASSERT(p_enemy->Genus.Person->InCar);

				enemy = p_enemy->Genus.Person->InCar;

				ASSERT(TO_THING(enemy)->Class == CLASS_VEHICLE);
			}
			else
			{
				//
				// For an enemy to shoot they should be able to see their enemy
				// (and be facing their enemy... but we won't check for that)
				//

				ASSERT(can_a_see_b(p_person, p_enemy));
			}

			return enemy;
		}
		else
		{
			//
			// A person is shooting without having anyone they want to kill!
			//

			return NULL;
		}
	}
}






#ifndef PSX
SLONG	calc_snipe_target_score(Thing *p_person,Thing *p_target)
{
	SLONG	dx,dy,dz,dist;
	SLONG	angle;
	SLONG	angle_diff;
	SLONG	score;

	dx=p_person->WorldPos.X-p_target->WorldPos.X>>8;
	if(abs(dx) >19*256)
		return(-1);
	dy=p_person->WorldPos.Y-p_target->WorldPos.Y>>8;
	if(abs(dy) >13*256)
		return(-1);
	dz=p_person->WorldPos.Z-p_target->WorldPos.Z>>8;
 	if(abs(dz) >19*256)
		return(-1);

	

	dist=dx*dx+dy*dy+dz*dz;

	if(dist>20*256*10*256)
		return(-1);

	angle		=	Arctan(dx,-dz); //-1024;
	angle_diff=angle-p_person->Draw.Tweened->Angle;
	angle_diff=(angle_diff+2048)&2047;

	if(angle_diff>=56&&angle_diff<=2048-56)
		return(-1);
  
//	dist=Root(dist)>>8; //map blocks
//	dist=(11-dist)<<4;

//	return(dist);
	if(angle_diff<256)
		score=256-angle_diff; //+dist;
	else
		score=angle_diff-(2048-256); //+dist;
  

	return(score);
}


THING_INDEX find_snipe_target(Thing *p_person)
{
	THING_INDEX	current_thing,best=0;
	Thing	*t_thing;
	SLONG	score,best_score=-1;

	current_thing	=	PRIMARY_USED;

	while(current_thing)
	{
		t_thing			=	TO_THING(current_thing);


		if(t_thing->Class==CLASS_PERSON&& t_thing!=p_person)
		{
			score=calc_snipe_target_score(p_person,t_thing);
			if(score>best_score)
			{
				best_score=score;
				best=current_thing;
			}
		}
		current_thing	=	t_thing->LinkChild;
	}
	return(best);
}
#endif