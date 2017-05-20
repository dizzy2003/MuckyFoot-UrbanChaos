//
// Ingame save functions.
//

#include "game.h"
#include "memory.h"
#include "pcom.h"
#include "statedef.h"
#include "eway.h"
#include "animate.h"
#include "special.h"
#include "tracks.h"

#ifndef	PSX

//
// Low level output functions.
//

#ifndef PSX

FILE *SAVE_handle;

SLONG SAVE_out_data(void *data, ULONG num_bytes)
{
	if (fwrite(data,  1, num_bytes, SAVE_handle) != num_bytes)
	{	
		return FALSE;
	}
	else
	{	
		return TRUE;
	}
}

SLONG LOAD_in_data(void *data, ULONG num_bytes)
{
	DebugText(" read <%d> \n",num_bytes);
	if (fread(data, 1, num_bytes, SAVE_handle) != num_bytes)
	{	
		return FALSE;
	}
	else
	{	
		return TRUE;
	}
}

FILE *LOAD_open()
{
	return MF_Fopen("ingame.sav", "rb");
}
FILE *SAVE_open()
{
	return MF_Fopen("ingame.sav", "wb");
}

#else

int SAVE_handle;

SLONG SAVE_open()
{
	return 0;
}

SLONG LOAD_open()
{
	return 0;
}

SLONG SAVE_out_data(void *data,ULONG num_bytes)
{
//	Compress_Compress(data,num_bytes);
}

SLONG LOAD_in_data(void *data,ULONG num_bytes)
{
//	Compress_Decompress(data,num_bytes);
}

#endif


//
// A compressed person.
//

#define SAVE_PERSON_TYPE_NORMAL				0	// The whole goddam lot is saved.
#define SAVE_PERSON_TYPE_WANDERING_DRIVER	1
#define SAVE_PERSON_TYPE_DEAD				2
#define SAVE_PERSON_TYPE_ARRESTED			3
#define SAVE_PERSON_TYPE_WANDERING_CIV		4
#define SAVE_PERSON_TYPE_FULL				5

#define SAVE_SPECIAL_TYPE_FULL				6

#define SAVE_VEHICLE_TYPE_FULL				7
#define SAVE_VEHICLE_TYPE_WANDERING			8
#define SAVE_SKIP							99
#define SAVE_SKIP_CLASS_NONE				100


#define	SAVE_GAME_EWAY					101

UBYTE	skip=SAVE_SKIP;
UBYTE	skip_class_none=SAVE_SKIP_CLASS_NONE;

typedef struct
{
	UBYTE type;
	UBYTE yaw;
	SBYTE health;
	UBYTE looklike;	// Top four bits is the PersonID, bottom four bits are PersonType
	UWORD x;
	SWORD y;
	UWORD z;
	UWORD other_a;	// The car this person is driving
					// The current anim for dead people
	UWORD other_b;	// The passenger for drivers
	UBYTE ware;
	UBYTE drop;
	UWORD onface;

} SAVE_Person;

typedef struct
{
	UBYTE	Type;
	UBYTE	Person;
	UWORD	Thing;
	UWORD	DrawTween;

} SAVE_Person_extra;

typedef struct
{
	UBYTE	Type;
	UBYTE	Pad;
	UWORD	Thing;
	UWORD	Special;
	UWORD	DrawMesh;

} SAVE_Special_extra;

typedef struct
{
	UBYTE	Type;
	UBYTE	Yaw;
	UWORD	Thing;
	UWORD	x;
	SWORD	y;
	UWORD	z;
	UWORD	driver;
	UWORD	passenger;

} SAVE_just_vehicle;

typedef struct
{
	UBYTE	Type;
	UBYTE	Pad;
	UWORD	Thing;
	UWORD	Vehicle;
	UWORD	DrawMesh;

} SAVE_Vehicle_extra;

//
// Saves out a person thing structure. Returns FALSE on failure.
//

SLONG SAVE_special(Thing *p_special)
{
	SLONG	ret=1;

	SAVE_Special_extra	extra;


	extra.Type = SAVE_SPECIAL_TYPE_FULL;

	extra.Thing = THING_NUMBER(p_special);

	extra.Special = SPECIAL_NUMBER(p_special->Genus.Special);

	ret&=SAVE_out_data(&extra, sizeof(extra));
	ret&=SAVE_out_data(p_special->Genus.Special, sizeof(Special));
	ret&=SAVE_out_data(p_special->Draw.Mesh, sizeof(DrawMesh));
	ret&=SAVE_out_data(p_special, sizeof(Thing));

	return(ret);
}

SLONG	SAVE_vehicle(Thing *p_vehicle)
{
	Thing	*p_driver;
	SLONG	ret=1;
	SAVE_Vehicle_extra	extra;

	if(p_vehicle->Genus.Vehicle->Driver)
	{
		p_driver=TO_THING(p_vehicle->Genus.Vehicle->Driver);
		if (p_driver->Genus.Person->pcom_ai   == PCOM_AI_DRIVER &&
			p_driver->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
		{
			if (p_driver->Genus.Person->Flags & FLAG_PERSON_DRIVING)
			{	
				if (p_driver->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
				{
					SAVE_out_data(&skip, sizeof(skip));
					return(1);

/*
					SAVE_just_vehicle	extrav;

					//
					// dont save info about wandering drivers
					//
					return(1);

					extrav.Type=SAVE_VEHICLE_TYPE_HALF;
					extrav.Yaw =(p_vehicle->Genus.Vehicle->Angle&2047)>>3;
					extrav.Thing = THING_NUMBER(p_vehicle);
					extrav.x=p_vehicle->WorldPos.X>>8;
					extrav.y=p_vehicle->WorldPos.Y>>8;
					extrav.z=p_vehicle->WorldPos.Z>>8;
					extrav.driver=p_vehicle->Genus.Vehicle->Driver;
					extrav.passenger=p_vehicle->Genus.Vehicle->Passenger;

					return(SAVE_out_data(&extrav, sizeof(extrav)));
*/
				}
			}
		}
	}



	extra.Type = SAVE_VEHICLE_TYPE_FULL;

	extra.Thing = THING_NUMBER(p_vehicle);

	extra.Vehicle = VEHICLE_NUMBER(p_vehicle->Genus.Vehicle);

	ret&=SAVE_out_data(&extra, sizeof(extra));
	ret&=SAVE_out_data(p_vehicle->Genus.Vehicle, sizeof(Vehicle));
	ret&=SAVE_out_data(p_vehicle, sizeof(Thing));

	return(ret);

	


}



SLONG SAVE_person(Thing *p_person)
{
	SAVE_Person sp;
	SLONG	ret;

	ASSERT(p_person->Class == CLASS_PERSON);

	memset(&sp, 0, sizeof(sp));

	//
	// If this person is a wandering civ or a wandering civ driver, then
	// we don't have to save much state for him.
	//

	sp.type = SAVE_PERSON_TYPE_NORMAL;

	if (p_person->Genus.Person->pcom_ai   == PCOM_AI_CIV &&
		p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
	{
		//
		// Wandering civs are regenerated when we load the waypoints for the level.
		//

		SAVE_out_data(&skip, sizeof(skip));
		return(1);
		sp.type = SAVE_PERSON_TYPE_WANDERING_CIV;

		return SAVE_out_data(&sp.type, sizeof(UBYTE));
	}


	if (p_person->Genus.Person->pcom_ai   == PCOM_AI_DRIVER &&
		p_person->Genus.Person->pcom_move == PCOM_MOVE_WANDER)
	{
		if (p_person->Genus.Person->Flags & FLAG_PERSON_DRIVING)
		{	
			if (p_person->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL)
			{
				SAVE_out_data(&skip, sizeof(skip));
				return(1);

				sp.type    = SAVE_PERSON_TYPE_WANDERING_DRIVER;
				sp.other_a = p_person->Genus.Person->InCar;
				sp.other_b = p_person->Genus.Person->Passenger;
			}
		}
	}

	if (p_person->State == STATE_DEAD)
	{
		if (p_person->Genus.Person->Flags & FLAG_PERSON_ARRESTED)
		{
			sp.type  = SAVE_PERSON_TYPE_ARRESTED;
			sp.other_b = p_person->SubState;
		}
		else
		{
			sp.type    = SAVE_PERSON_TYPE_DEAD;
		}
		sp.other_a = p_person->Draw.Tweened->CurrentAnim;
	}

	if(sp.type)
	{
		//
		// Fill in the person structure
		//

		sp.x         = p_person->WorldPos.X >> 8;
		sp.y         = p_person->WorldPos.Y >> 8;
		sp.z         = p_person->WorldPos.Z >> 8;
		sp.yaw       = p_person->Draw.Tweened->Angle >> 3;
		sp.health    = p_person->Genus.Person->Health;
//		sp.looklike  = 0;
//		sp.looklike |= p_person->Genus.Person->PersonType & 0xf;
//		sp.looklike |= p_person->Draw.Tweened->PersonID << 4;
		sp.ware      = p_person->Genus.Person->Ware;
		sp.drop      = p_person->Genus.Person->drop;
		sp.onface    = p_person->OnFace;


		if (!SAVE_out_data(&sp, sizeof(SAVE_Person)))
		{
			return FALSE;
		}
	}
	else
	{
		SAVE_Person_extra	extra;

		ret=1;

		extra.Type = SAVE_PERSON_TYPE_FULL;

		extra.Thing = THING_NUMBER(p_person);

		extra.Person = PERSON_NUMBER(p_person->Genus.Person);
		extra.DrawTween   = DRAW_TWEEN_NUMBER(p_person->Draw.Tweened);

		ret&=SAVE_out_data(&extra, sizeof(SAVE_Person_extra));
		ret&=SAVE_out_data(p_person->Draw.Tweened, sizeof(DrawTween));
		ret&=SAVE_out_data(p_person->Genus.Person, sizeof(Person));
		ret&=SAVE_out_data(p_person, sizeof(Thing));
		if(ret==0)
			return(FALSE);

	}
	return(TRUE);
}



SLONG	SAVE_things(void)
{
	SLONG	index;
	Thing	*p_thing;

	for(index=0;index<MAX_THINGS;index++)
	{
		p_thing=TO_THING(index);

		switch(p_thing->Class)
		{
			case	CLASS_PERSON:
				if(!SAVE_person(p_thing))
				{
					return(FALSE);
				}
				break;
			case	CLASS_SPECIAL:
				if(!SAVE_special(p_thing))
				{
					return(FALSE);
				}
				break;
			case	CLASS_VEHICLE:
				if(!SAVE_vehicle(p_thing))
				{
					return(FALSE);
				}
				break;
			case	CLASS_NONE:
				SAVE_out_data(&skip_class_none, sizeof(skip_class_none));
				break;

			default:
				SAVE_out_data(&skip, sizeof(skip));
				
				break;
		}

//		index=p_thing->LinkChild;
	}
	return(0);
}


SLONG	SAVE_eways(void)
{
	UBYTE		marker=SAVE_GAME_EWAY;
	SLONG		c0,res=1;
	EWAY_Way	*ew;

	if (!SAVE_out_data(&marker, sizeof(marker)))
	{
		return	FALSE;
	}

	for(c0=0;c0<EWAY_way_upto;c0++)
	{
		ew = &EWAY_way[c0];

		if(ew->flag & EWAY_FLAG_COUNTDOWN)
		{
			
			res&=SAVE_out_data(&ew->flag, sizeof(ew->flag));
			res&=SAVE_out_data(&ew->timer, sizeof(ew->timer));
		}
		else
		{
			res&=SAVE_out_data(&ew->flag, sizeof(ew->flag));
		}

		if(!res)
			return(FALSE);
	}
	res&=SAVE_out_data(EWAY_timer, sizeof(UWORD)*EWAY_MAX_TIMERS);
	return TRUE;
}



SLONG	SAVE_ingame(CBYTE *fname)
{
	SLONG	ret=1;

#ifndef PSX
	SAVE_handle = MF_Fopen("ingame.sav", "wb");
#else
	SAVE_handle = SAVE_open();
#endif

	ret&=SAVE_things();
	ret&=SAVE_eways();

	MF_Fclose(SAVE_handle);
//	SAVE_VALID=1;
	return(TRUE);

}


//*******************************************************************************************************
//*																										*
//*                                         Load Equivalent												*
//*																										*
//*																										*
//*******************************************************************************************************








SLONG	LOAD_eways(void)
{
	SLONG		c0,res=1;
	EWAY_Way	*ew;


	for(c0=0;c0<EWAY_way_upto;c0++)
	{
		ew = &EWAY_way[c0];

		res&=LOAD_in_data(&ew->flag, sizeof(ew->flag));
		if(ew->flag & EWAY_FLAG_COUNTDOWN)
		{
			
			res&=LOAD_in_data(&ew->timer, sizeof(ew->timer));
		}
		if(!res)
			return(FALSE);
	}
	res&=LOAD_in_data(EWAY_timer, sizeof(UWORD)*EWAY_MAX_TIMERS);
	return(res);
}

void	set_person_default_data(Thing *p_person,SAVE_Person *sp)
{
	p_person->WorldPos.X=sp->x<<8;
	p_person->WorldPos.Y=sp->y<<8;
	p_person->WorldPos.Z=sp->z<<8;
	p_person->Draw.Tweened->Angle=sp->yaw<<3;
	p_person->Genus.Person->Health=sp->health;
	p_person->Genus.Person->Ware=sp->ware;
	p_person->Genus.Person->drop=sp->drop;
	p_person->OnFace=sp->onface;

}

void	LOAD_person_dead(Thing *p_person)
{
	SAVE_Person sp;

	ASSERT(p_person->Class==CLASS_PERSON);

	if (p_person->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
		remove_thing_from_map(p_person);

	DebugText(" load person %d ",THING_NUMBER(p_person));
	LOAD_in_data(&sp.yaw, sizeof(SAVE_Person)-1);

	set_person_default_data(p_person,&sp);



	//
	// recreate what it's like to be dead
	//

	set_anim(p_person,sp.other_a);
	set_generic_person_state_function(p_person,STATE_DEAD);
	p_person->Genus.Person->Action = ACTION_DEAD;
	p_person->Genus.Person->Timer1 = 0;
	p_person->SubState=0;
	TRACKS_Bloodpool(p_person);
	TRACKS_Bloodpool(p_person);

	p_person->Flags&=~FLAGS_ON_MAPWHO;
	add_thing_to_map(p_person);

}

void	LOAD_person_arrested(Thing *p_person)
{
	SAVE_Person sp;
	
	ASSERT(p_person->Class==CLASS_PERSON);

	if (p_person->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
		remove_thing_from_map(p_person);

	DebugText(" load person arrest%d ",THING_NUMBER(p_person));
	LOAD_in_data((&sp.yaw), sizeof(SAVE_Person)-1);

	set_person_default_data(p_person,&sp);



	//
	// recreate what it's like to be dead
	//

	set_anim(p_person,sp.other_a);
	set_generic_person_state_function(p_person,STATE_DEAD);
	p_person->Genus.Person->Action = ACTION_DEAD;
	p_person->Genus.Person->Timer1 = 0;
	p_person->SubState=sp.other_b;
	TRACKS_Bloodpool(p_person);
	TRACKS_Bloodpool(p_person);

	p_person->Flags&=~FLAGS_ON_MAPWHO;
	add_thing_to_map(p_person);
}

void	LOAD_person_full(Thing *p_person)
{
	SAVE_Person_extra	extra;

	ASSERT(p_person->Class==CLASS_PERSON);

	if (p_person->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
		remove_thing_from_map(p_person);

	DebugText(" load person full %d ",THING_NUMBER(p_person));
	LOAD_in_data((&extra.Person),sizeof(extra)-1);

	ASSERT(extra.Thing==THING_NUMBER(p_person));
	ASSERT(extra.Person==PERSON_NUMBER(p_person->Genus.Person));
	ASSERT(extra.DrawTween==DRAW_TWEEN_NUMBER(p_person->Draw.Tweened));

	LOAD_in_data(p_person->Draw.Tweened,sizeof(DrawTween));
	LOAD_in_data(p_person->Genus.Person,sizeof(Person));
	LOAD_in_data(p_person,sizeof(Thing));

	if (p_person->Flags&FLAGS_ON_MAPWHO)
	{
		p_person->Flags&=~FLAGS_ON_MAPWHO;
		add_thing_to_map(p_person);
	}
	else
	{
	 //	Wasn't on mapwho when we saved so don't add it now
	}
}

void	LOAD_special_full(Thing *p_special)
{
	DrawMesh	*draw_mesh;
	Special		*special;

	if (p_special->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
		remove_thing_from_map(p_special);

	SAVE_Special_extra	extra;
	DebugText(" loadspecialfull %d ",THING_NUMBER(p_special));
	LOAD_in_data((&extra.Pad),sizeof(extra)-1);
//	ASSERT(extra.Special==SPECIAL_NUMBER(p_special->Genus.Special));
//	ASSERT(extra.Thing==THING_NUMBER(p_special));

//	ASSERT(p_special->Class==CLASS_SPECIAL);

	if(p_special->Class!=CLASS_SPECIAL)
	{
		SLONG	index;
		DrawMesh *dm;

extern	SLONG	find_empty_special(void);
		index=find_empty_special();

		p_special->Genus.Special=TO_SPECIAL(index);

		dm = alloc_draw_mesh();

		ASSERT(dm);
		p_special->Draw.Mesh	=	dm;
	}

	LOAD_in_data(p_special->Genus.Special,sizeof(Special));
	LOAD_in_data(p_special->Draw.Mesh,sizeof(DrawMesh));
	special  =p_special->Genus.Special;
	draw_mesh=p_special->Draw.Mesh;


	LOAD_in_data(p_special,sizeof(Thing));
	p_special->Genus.Special=special;
	p_special->Draw.Mesh=draw_mesh;

	if (p_special->Flags&FLAGS_ON_MAPWHO)
	{
		p_special->Flags&=~FLAGS_ON_MAPWHO;
		add_thing_to_map(p_special);
	}
	else
	{
	 //	Wasn't on mapwho when we saved so don't add it now
	}
}

void	LOAD_vehicle_full(Thing *p_vehicle)
{
	SAVE_Vehicle_extra	extra;

	ASSERT(p_vehicle->Class==CLASS_VEHICLE);

	if (p_vehicle->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
		remove_thing_from_map(p_vehicle);

	DebugText(" loadvehiclefull %d ",THING_NUMBER(p_vehicle));
	LOAD_in_data((&extra.Pad),sizeof(extra)-1);

	ASSERT(extra.Thing==THING_NUMBER(p_vehicle));
	ASSERT(extra.Vehicle==VEHICLE_NUMBER(p_vehicle->Genus.Vehicle));

	LOAD_in_data(p_vehicle->Genus.Vehicle,sizeof(Vehicle));
	LOAD_in_data(p_vehicle,sizeof(Thing));

	if (p_vehicle->Flags&FLAGS_ON_MAPWHO)
	{
		p_vehicle->Flags&=~FLAGS_ON_MAPWHO;
		add_thing_to_map(p_vehicle);
	}
	else
	{
	 //	Wasn't on mapwho when we saved so don't add it now
	}
}

SLONG	LOAD_types(void)
{
	UBYTE	type;
	UWORD	thing=0;
	Thing	*p_thing;

	SLONG	special=0,person=0,car=0;



	p_thing=TO_THING(0);
	while(LOAD_in_data(&type,1))
	{
		switch(type)
		{
			case		SAVE_PERSON_TYPE_NORMAL:
				ASSERT(0);
				break;
			case		SAVE_PERSON_TYPE_WANDERING_DRIVER:
				ASSERT(0);
				break;
			case		SAVE_PERSON_TYPE_DEAD:
				LOAD_person_dead(p_thing);
				break;
			case		SAVE_PERSON_TYPE_ARRESTED:
				LOAD_person_arrested(p_thing);
				break;
			case		SAVE_PERSON_TYPE_WANDERING_CIV:
				ASSERT(0);
//				LOAD_person_wandering(p_thing);
				break;
			case		SAVE_PERSON_TYPE_FULL:		 
				LOAD_person_full(p_thing);
				person++;
				break;
			                                      			
			case		SAVE_SPECIAL_TYPE_FULL:				 
				LOAD_special_full(p_thing);
				special++;
				break;
													  
			case		SAVE_VEHICLE_TYPE_FULL:				 
				LOAD_vehicle_full(p_thing);
				car++;
				break;
													  
			case		SAVE_GAME_EWAY:
				LOAD_eways();
													  
			case		SAVE_SKIP:					
				switch(p_thing->Class)
				{
					case	CLASS_SPECIAL:
						remove_thing_from_map(p_thing);
						p_thing->Class=CLASS_NONE;
			//			ASSERT(0);
							break;

				}
				break;
			case		SAVE_SKIP_CLASS_NONE:
				remove_thing_from_map(p_thing);
				switch(p_thing->Class)
				{
					case	CLASS_NONE:
						break;

					case	CLASS_SPECIAL:
void	free_special(Thing *special_thing);
							free_special(p_thing);
							break;
					case	CLASS_TRACK:
							break;
					default:
						ASSERT(0);
						break;


				}
				p_thing->Class=CLASS_NONE;

				break;
			default:

				ASSERT(0);
				break;
		}
		p_thing++;
		thing++;
	}

	DebugText(" special %d people %d car %d \n",special,person,car);

	return 0;
}

extern	UWORD	*thing_class_head;


void	fix_thing_lists(void)
{
	SLONG	c0;
	Thing	*p_thing;

	PRIMARY_USED				=	0;
	PRIMARY_UNUSED				=	0;
	PRIMARY_COUNT				=	0;

	for(c0=0;c0<MAX_THINGS;c0++)
	{
		p_thing=TO_THING(c0);

		if(p_thing->Class!=CLASS_NONE)
		{
			//
			// thing is used
			//

			p_thing->LinkParent=0;
			p_thing->LinkChild=PRIMARY_USED;
			PRIMARY_USED=c0;
			PRIMARY_COUNT++;

			thing_class_head[p_thing->Class]=c0;
		}
		else
		{
			//
			// thing is unused
			//

			p_thing->LinkParent=0;
			p_thing->LinkChild=PRIMARY_UNUSED;
			PRIMARY_UNUSED=c0;

		}
	}
}
extern	void free_special(Thing *s_thing);

void	remove_specials(void)
{
	SLONG	index,next;
	Thing	*p_special;

	index=thing_class_head[CLASS_SPECIAL];

	while(index)
	{
		p_special=TO_THING(index);
		next=p_special->NextLink;
		free_special(p_special);
		index=next;
	}

}
SLONG	LOAD_ingame(CBYTE *fname)
{
	SLONG	ret=1;

	 TRACKS_Reset();

	 remove_specials();


void	reload_level(void);
	reload_level();

	DebugText("\nLoadInGame \n\n");
	SAVE_handle = LOAD_open();

	SAVE_handle = LOAD_open();
	LOAD_types();
	MF_Fclose(SAVE_handle);

	fix_thing_lists();
//	SAVE_VALID=1;

	return(TRUE);

}

#endif