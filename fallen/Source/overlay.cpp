#include "game.h"
#include "c:\fallen\headers\cam.h"
#include "c:\fallen\headers\statedef.h"
#include "c:\fallen\ddengine\headers\panel.h"
#include "fc.h"
#include "animate.h"
#include "memory.h"
#include "mav.h"
#include "vehicle.h"
#include "eway.h"
#ifndef PSX
#include "ddlib.h"
#include "c:\fallen\ddengine\headers\planmap.h"
#include "c:\fallen\ddengine\headers\poly.h"
#include "font2d.h"
#else
#include "c:\fallen\psxeng\headers\psxeng.h"
#include "c:\fallen\psxeng\headers\panel.h"
#endif
#include	"interfac.h"

#include "eway.h"
#include "xlat_str.h"
#ifdef PSX
#include	"ctrller.h"
#endif

#ifdef	MIKE
#define	VERSION_NTSC	1
#endif

extern	void	add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text);


//
// local prototypes
//
void	OVERLAY_draw_damage_values(void);

//
// This is for everything that gets overlayed during the game
// like panels and text and maps and ...
//



//
// All the draw routines to support this module should go in ddengine\source\panel.cpp
//


#define	INFO_NUMBER	1
#define	INFO_TEXT	2
#ifdef	DAMAGE_TEXT
struct	DamageValue
{
	UBYTE	Type;
	CBYTE	*text_ptr;
	SWORD	Age;
	SWORD	Value;
	SWORD	X;
	SWORD	Y;
	SWORD	Z;

};


#define	MAX_DAMAGE_VALUES	16

struct	DamageValue	damage_values[MAX_DAMAGE_VALUES];
SLONG	damage_value_upto=1;
#endif


struct	TrackEnemy
{
	Thing	*PThing;
	SWORD	State;
	SWORD	Timer;
	SWORD	Face;
};

#define	MAX_TRACK	4

#define	STATE_TRACKING	1
#define	STATE_UNUSED	0

/*
#define	MAX_BEACON	5
struct Beacon
{
	UWORD	X;
	UWORD	Z;
	UWORD	Type;
	UWORD	OTHER;
};

struct	Beacon	beacons[MAX_BEACON];
UWORD	beacon_upto=1;
*/


struct	TrackEnemy	panel_enemy[MAX_TRACK];

#define	USE_CAR			(1)
#define	USE_CABLE		(2)
#define	USE_LADDER		(3)
#define	PICKUP_ITEM		(4)
#define	USE_MOTORBIKE	(5)
#define	TALK			(6)



#define	HELP_MAX_COL	16

#define	HELP_GRAB_CABLE		0
#define	HELP_PICKUP_ITEM	1
#define	HELP_USE_CAR		2
#define	HELP_USE_BIKE		3

#ifndef TARGET_DC
CBYTE	*help_text[]=
{
	"Jump up to grab cables",
	"Press action to pickup items",
	"Press action to use vehicles",
	"Press action to use bikes",
	""
};

UWORD	help_xlat[] = { X_GRAB_CABLE, X_PICK_UP, X_ENTER_VEHICLE, X_USE_BIKE };
#endif

#ifndef PSX
#ifndef TARGET_DC
SLONG should_i_add_message(SLONG type)
{
	static SLONG last_message[4]={0,0,0,0};	// The gameturn when the last message was added.

	ASSERT(WITHIN(type, 0, 3));

	if (last_message[type] >  GAME_TURN ||
		last_message[type] <= GAME_TURN - 100)
	{
		last_message[type] = GAME_TURN;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void	arrow_object(Thing *p_special,SLONG dir,SLONG type)
{
	SLONG	x,y,z;

	if (!should_i_add_message(type))
	{
		return;
	}

	x=p_special->WorldPos.X>>8;
	y=p_special->WorldPos.Y>>8;
	z=p_special->WorldPos.Z>>8;

	y+=((GAME_TURN+THING_INDEX(p_special))<<3)&63;

	//add_damage_text(x,y,z, help_text[type]);
	add_damage_text(x,y,z,XLAT_str(help_xlat[type]));

	/*

	if(dir==1)
	{
		AENG_world_line(x,y,z,1,0xff0000,x,y+64,z,50,0x800000,FALSE);
		AENG_world_line(x,y+64,z,20,0xff0000,x,y+128,z,20,0x800000,FALSE);
	}
	else
	{
		AENG_world_line(x,y+128,z,1,0xff0000,x,y+64,z,50,0x800000,FALSE);
		AENG_world_line(x,y+64,z,20,0xff0000,x,y,z,20,0x800000,FALSE);
	}

	*/
}

void	arrow_pos(SLONG x,SLONG y,SLONG z,SLONG dir,SLONG type)
{
	y+=((GAME_TURN+x)<<3)&63;

	if (!should_i_add_message(type))
	{
		return;
	}

//	add_damage_text(x,y,z, help_text[type]);
	add_damage_text(x,y,z, XLAT_str(help_xlat[type]));
	
	/*

	if(dir==1)
	{
		AENG_world_line(x,y,z,1,0xff0000,x,y+64,z,50,0x800000,FALSE);
		AENG_world_line(x,y+64,z,20,0x800000,x,y+128,z,20,0x400000,FALSE);
	}
	else
	{
		AENG_world_line(x,y-0,z,1,0xff0000,x,y-64,z,50,0x800000,FALSE);
		AENG_world_line(x,y-64,z,20,0x800000,x,y-128,z,20,0x400000,FALSE);
	}
	*/
}
#endif
#endif

void	Time(struct MFTime *the_time);

void	show_help_text(SLONG index)
{
	/*
#ifndef	PSX
	static	ULONG	last_message=0;
	struct MFTime 	the_time;
	ULONG	time_now;

	Time(&the_time);
	time_now=the_time.Ticks;

	if(time_now-last_message>14000)
	{
		last_message=time_now;
		PANEL_new_help_message(help_text[index]);
	}
#endif
	*/
}





#ifdef	UNUSED
void	highlight_cable_grab(void)
{
	SLONG	index;
	SLONG	x,z;
	Thing	*p_person;
	SLONG	exit=0;
	SWORD f_list;
	SWORD i_facet;


	p_person=NET_PERSON(0);

	x=p_person->WorldPos.X>>8;
	z=p_person->WorldPos.Z>>8;

	exit   = FALSE;
	f_list = PAP_2LO(x>>10,z>>10).ColVectHead;


	if (!f_list)
	{
		//
		// No facets on this square.
		//

		return;
	}

	while(!exit)
	{

		SLONG	fdx,fdy,fdz,sdx,sdz,len,step;
		DFacet *df;

		i_facet = facet_links[f_list++];
		ASSERT(i_facet<next_dfacet);

		if (i_facet < 0)
		{
			i_facet = -i_facet;
			exit   =   TRUE;
		}

		df = &dfacets[i_facet];

		fdx = ((df->x[1]-df->x[0])<<8);
		fdz = ((df->z[1]-df->z[0])<<8);

		len=QDIST2(abs(fdx),abs(fdz));
		len=len>>7; // how many 128 lengths there are
		len+=1;

		fdx=(fdx<<8)/len;
		fdz=(fdz<<8)/len;

		sdx=df->x[0]<<16;
		sdz=df->z[0]<<16;

		step=(1<<12)/len;
		if(step<1)
			step=1;

		if (df->FacetType == STOREY_TYPE_CABLE)
		{
			SLONG	along;
			for(along=0;along<(1<<12);along+=step)
			{
				SLONG	wx,wz;
				SLONG	dx,dz;
				SLONG	my;


				wx=sdx>>8;
				wz=sdz>>8;

				dx=abs(wx-x);
				dz=abs(wz-z);

				if(QDIST2(dx,dz)<512)
				{
					CBYTE	str[100];
					SLONG	dy;
					
					my=find_cable_y_along(df,along);

//extern	FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

//					sprintf(str,"%d",((MAVHEIGHT(wx>>8,wz>>8)<<6)-my));
//					FONT2D_DrawString_3d(str,wx,50+(MAVHEIGHT(wx>>8,wz>>8)<<6),wz,0xff0000,60,0);

					dy=abs(((MAVHEIGHT(wx>>8,wz>>8)<<6)-my));
					if(dy>200&&dy<240)
					{
						arrow_pos(wx,my-128,wz,2,HELP_GRAB_CABLE);
						show_help_text(HELP_GRAB_CABLE);
					}
				}
				sdx+=fdx;
				sdz+=fdz;
			}
		}
	}

}
//#endif



SLONG	help_system(void)
{
	Thing	*p_person;
	ULONG collide_types;
	UWORD	found[HELP_MAX_COL];

	SLONG i;
	SLONG num;
	SLONG prim;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	Thing        *p_found;
	VEH_Col      *vc;
	PrimInfo     *pi;
	AnimPrimBbox *apb;

	SLONG	x,y,z;

	SLONG	nearest_car=0,nearest_car_d=0;

	#ifdef BIKE
	SLONG	nearest_bike=0,nearest_bike_d=0;
	#endif

	p_person=NET_PERSON(0);
	if(p_person->State==STATE_DEAD)
		return(0);

	x=p_person->WorldPos.X>>8;
	y=p_person->WorldPos.Y>>8;
	z=p_person->WorldPos.Z>>8;

	collide_types = (1 << CLASS_VEHICLE) |(1 << CLASS_BIKE)|(1<<CLASS_SPECIAL);

	num = THING_find_sphere(
			x,y,z,
			0x300,
			found,
			HELP_MAX_COL,
			collide_types);

	//
	// Scan through
	//

	VEH_col_upto = 0;

	for (i = 0; i < num; i++)
	{
	//	if (found[i] == ignore)		continue;

		ASSERT(WITHIN(VEH_col_upto, 0, VEH_MAX_COL - 1));

		p_found =  TO_THING(found[i]);

		switch(p_found->Class)
		{
			case CLASS_VEHICLE:
				if (!p_found->Genus.Vehicle->Driver && p_found->State != STATE_DEAD)
				{
					if (p_found->Genus.Vehicle->key)
					{
						if (!person_has_special(p_person, p_found->Genus.Vehicle->key))
						{
							continue;
						}
					}

					// Simple bounding circle rejection.

					dx = abs((p_found->WorldPos.X >> 8) - x);
					dz = abs((p_found->WorldPos.Z >> 8) - z);

					dist = QDIST2(dx,dz);

					if (dist <= 512)
					{
						SLONG cx,cz,dy;

						show_help_text(HELP_USE_CAR);

						extern	void	get_car_enter_xz(Thing *p_vehicle,SLONG *cx,SLONG *cz);

						get_car_enter_xz(p_found,&cx,&cz);
						 
						switch(p_found->Genus.Vehicle->Type)
						{
							case VEH_TYPE_VAN:
							case VEH_TYPE_AMBULANCE:	
							case VEH_TYPE_MEATWAGON:
							case VEH_TYPE_JEEP:
							case VEH_TYPE_WILDCATVAN:
								dy=150;
								break;
							default:
								dy=50;
								break;


						}
						arrow_pos(cx,(p_found->WorldPos.Y>>8)+dy,cz,1,HELP_USE_CAR);

					}
				}

				break;

			#ifdef BIKE
			case CLASS_BIKE:
				if (p_found->Genus.Vehicle->Driver)
				{

					//prim = get_vehicle_body_prim(p_found->Genus.Vehicle->Type);
					//pi   = get_prim_info(prim);

					// Simple bounding circle rejection.

					dx = abs((p_found->WorldPos.X >> 8) - x);
					dz = abs((p_found->WorldPos.Z >> 8) - z);

					dist = QDIST2(dx,dz);

					if (dist <= 512)
					{
						SLONG	cx,cz;
						show_help_text(HELP_USE_BIKE);
						 
						arrow_object(p_found,1,HELP_USE_BIKE);
						//draw_arrow(cx,(p_found->WorldPos.Y>>8)+150,cz,1);

					}
				}
				break;
			#endif

			case CLASS_SPECIAL:

				if(should_person_get_item(p_person, p_found))
				{
					switch(p_found->Genus.Special->SpecialType)
					{
						case SPECIAL_TREASURE:
							//
							// don't highlight treasure
							//
							break;

						default:


						// Simple bounding circle rejection.

						dx = abs((p_found->WorldPos.X >> 8) - x);
						dz = abs((p_found->WorldPos.Z >> 8) - z);

						dist = QDIST2(dx,dz);
/*
						if (dist <= 256)
						{
	//						if((GAME_TURN&63)==0)
	//							CONSOLE_text("Stand over item and press action to pickup\n");
							show_help_text(HELP_PICKUP_ITEM);
							arrow_object(p_found,1,HELP_PICKUP_ITEM);
						}
*/
						break;
					}
				}

				break;

		}
	}

	highlight_cable_grab();

//	find_thing_for_this_pos
		
	return(0);
}
#endif



void	track_enemy(Thing *p_thing)
{
	#ifdef OLD_POO

	SLONG	c0;
	SLONG	unused=-1;

	for(c0=0;c0<MAX_TRACK;c0++)
	{
		if(panel_enemy[c0].PThing==p_thing)
		{
			panel_enemy[c0].State=STATE_TRACKING;
			panel_enemy[c0].Timer=5000; //5 seconds
			return;
		}
		else
		if(panel_enemy[c0].State==STATE_UNUSED)
		{
			unused=c0;
		}
	}
	if(unused>=0)
	{
		SLONG	face;
		switch(p_thing->Genus.Person->PersonType)
		{				
			case	0:
				//PERSON_DARCI
			case	1:
			case	2:
				face=5;
				break;
			case	3:
				face=THING_NUMBER(p_thing)&3;   //special boss man
				if(face==0)
					face=1;
				break;

			case	4:
				face=6;   //special boss man
				break;
			case	5:
				face=4;
				break;
			case	6:
			case	7: //soldier
				face=4; // buggered if i know
				break;
			default:
///				ASSERT(0); // oi, guvnor. set a face value for whatever you just created
				face=0;
				break;
		}
		panel_enemy[unused].PThing=p_thing;
		panel_enemy[unused].State=STATE_TRACKING;
		panel_enemy[unused].Timer=15000; //5 seconds
		panel_enemy[unused].Face=face; //5 seconds
	}
	
	#endif
}

struct	TrackEnemy	panel_gun_sight[MAX_TRACK];
UWORD	track_count=0;


void	track_gun_sight(Thing *p_thing,SLONG accuracy)
{
	SLONG	c0;
	SLONG	unused=-1;
	for(c0=0;c0<track_count;c0++)
	{
		if(panel_gun_sight[c0].PThing==p_thing)
		{
			if(accuracy<panel_gun_sight[c0].Timer)
				panel_gun_sight[c0].Timer=accuracy;
			return;
		}
	}
	if(track_count<MAX_TRACK)
	{
		panel_gun_sight[track_count].Timer=accuracy;
		panel_gun_sight[track_count].PThing=p_thing;
		track_count++;
	}
/*
	for(c0=0;c0<MAX_TRACK;c0++)
	{
		if(panel_gun_sight[c0].PThing==p_thing)
		{
			panel_gun_sight[c0].State=STATE_TRACKING;
			panel_gun_sight[c0].Timer=5000; //5 seconds
			panel_gun_sight[c0].Face=accuracy; //5 seconds
			return;
		}
		else
		if(panel_gun_sight[c0].State==STATE_UNUSED)
		{
			unused=c0;
		}
	}
	if(unused>=0)
	{
		SLONG	face;
		panel_gun_sight[unused].PThing=p_thing;
		panel_gun_sight[unused].State=STATE_TRACKING;
		panel_gun_sight[unused].Timer=15000; //5 seconds
		panel_gun_sight[unused].Face=accuracy; //5 seconds
	}
*/
	
}

#ifndef PSX
#ifndef TARGET_DC
void	OVERLAY_draw_tracked_enemies(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_TRACK;c0++)
	{
		if(panel_enemy[c0].State==STATE_TRACKING)
		{
			SLONG	h;

/* draw face */


			h=panel_enemy[c0].PThing->Genus.Person->Health;
			if(h<0)
				h=0;
void	PANEL_draw_face(SLONG x,SLONG y,SLONG face,SLONG size);
#ifndef PSX
			PANEL_draw_face(c0*150+5,450-14,panel_enemy[c0].Face,32);
			PANEL_draw_health_bar(40+c0*150,450,h>>1);
#else
			PANEL_draw_face((c0<<7),214,panel_enemy[c0].Face,16);
			PANEL_draw_health_bar(24+(c0<<7),220,h>>1);
#endif

			PANEL_draw_health_bar(40+c0*150,460,(GET_SKILL(panel_enemy[c0].PThing)*100)/15);

			panel_enemy[c0].Timer-=TICK_TOCK;
//			if(panel_enemy[c0].Timer<0)
			{
				panel_enemy[c0].State=STATE_UNUSED;
				panel_enemy[c0].PThing=0;

			}
		}
	}
}
#endif
#endif

void	OVERLAY_draw_gun_sights(void)
{
	SLONG	c0;
	SLONG	hx,hy,hz;
	Thing	*p_thing;

	// Dodgy internals issue :-)
	POLY_flush_local_rot();

	for(c0=0;c0<track_count;c0++)
	{
		p_thing=panel_gun_sight[c0].PThing;
		switch(p_thing->Class)
		{
			case	CLASS_PERSON:

				// 11 is head
				calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,11,&hx,&hy,&hz);

				hx+=p_thing->WorldPos.X>>8;
				hy+=p_thing->WorldPos.Y>>8;
				hz+=p_thing->WorldPos.Z>>8;

				PANEL_draw_gun_sight(hx,hy,hz,panel_gun_sight[c0].Timer,256);
				break;
			case	CLASS_SPECIAL:
					PANEL_draw_gun_sight(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+30,p_thing->WorldPos.Z>>8,panel_gun_sight[c0].Timer,128);
					break;
			case	CLASS_BAT:

				{
					SLONG scale;

					if (p_thing->Genus.Bat->type == BAT_TYPE_BALROG)
					{
						scale = 450;
					}
					else
					{
						scale = 128;
					}


					PANEL_draw_gun_sight(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+30,p_thing->WorldPos.Z>>8,panel_gun_sight[c0].Timer,scale);
				}

				break;
			case	CLASS_BARREL:
				p_thing=panel_gun_sight[c0].PThing;
				PANEL_draw_gun_sight(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+80,p_thing->WorldPos.Z>>8,panel_gun_sight[c0].Timer,200);
				break;
			case	CLASS_VEHICLE:
				p_thing=panel_gun_sight[c0].PThing;
				PANEL_draw_gun_sight(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+80,p_thing->WorldPos.Z>>8,panel_gun_sight[c0].Timer,450);
				break;
		}
	}
	track_count=0;

	// draw a grenade aim target
extern SLONG person_holding_special(Thing* p_person, UBYTE special);	// I am so naughty.  I blame Mark.

	Thing*	p_player = NET_PERSON(0);

	if (person_holding_special(p_player, SPECIAL_GRENADE))
	{
/*
		SLONG	angle = p_player->Draw.Tweened->Angle;

		// now make the angle more conformant with normality
		angle = 1536 - angle;

		// mask.  I don't mind that.
		angle &= 2047;

		// fortunately the grenade is now frame-rate independant (thanks to me, hurrah!)
		SLONG	addx = COS(angle) * 2150 / 65536;
		SLONG	addz = SIN(angle) * 2150 / 65536;

		PANEL_draw_gun_sight((p_player->WorldPos.X >> 8) + addx, (p_player->WorldPos.Y >> 8) + 128, (p_player->WorldPos.Z >> 8) + addz, 1000, 200);
*/

		if (!p_player->Genus.Person->Ware)
		{

#ifndef PSX
void	show_grenade_path(Thing *p_person);
		show_grenade_path(p_player);
#endif
		}

	}
}

#ifndef PSX
void	OVERLAY_draw_health(void)
{
	SLONG	ph;
	ph=NET_PERSON(0)->Genus.Person->Health;
	if(ph<0)
		ph=0;
	PANEL_draw_health_bar(10,10,ph>>1);

	
}

void	OVERLAY_draw_stamina(void)
{
	SLONG	ph;
	ph=NET_PERSON(0)->Genus.Person->Stamina;
	if(ph<0)
		ph=0;
	PANEL_draw_health_bar(10,30,(ph*100)>>8);

	
}
#endif

extern	void PANEL_draw_local_health(SLONG mx,SLONG my,SLONG mz,SLONG percentage,SLONG radius=60);

void	OVERLAY_draw_enemy_health(void)
{
	Thing *p_person;

	p_person=NET_PERSON(0);

	if(p_person->Genus.Person->Mode==PERSON_MODE_FIGHT||p_person->SubState==SUB_STATE_AIM_GUN)
	{
		if(p_person->Genus.Person->Target)
		{
			Thing	*p_target;
			p_target=TO_THING(p_person->Genus.Person->Target);


			switch(p_target->Class)
			{
				case	CLASS_BAT:
					if(p_target->Genus.Bat->type==BAT_TYPE_BALROG)
					{
#ifndef PSX
						PANEL_draw_local_health(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,(100*p_target->Genus.Bat->health)>>8,300);
#else
						PANEL_draw_local_health(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,(100*p_target->Genus.Bat->health)>>8,150);
#endif
					}
					break;
				case	CLASS_PERSON:

					{
						SLONG percent;

						extern BOOL PersonIsMIB(Thing *p_person);

						if (PersonIsMIB(p_target))
						{
							percent = p_target->Genus.Person->Health * 100 / 700;
						}
						else
						{
//							percent = p_target->Genus.Person->Health >> 1;

							percent = p_target->Genus.Person->Health *100/health[p_target->Genus.Person->PersonType];
						}

						PANEL_draw_local_health(p_target->WorldPos.X>>8,p_target->WorldPos.Y>>8,p_target->WorldPos.Z>>8,percent);
					}

					break;
			}
						
		}
	}
}
#ifdef	PSX
CBYTE	punch[3];
CBYTE	kick[3];

void	init_punch_kick(void)
{
#ifdef	VERSION_NTSC
	SLONG	c0;
	for(c0=0;c0<14;c0++)
	{
		if (PAD_Current->data[c0].input_mask>0)
		{
			if (PAD_Current->data[c0].input_mask&INPUT_MASK_PUNCH)
			{
				switch(PAD_Current->data[c0].pad_button)
				{
					case	PAD_RU:
						sprintf(punch,STR_TRI);
						break;
					case	PAD_RL:
						sprintf(punch,STR_SQUARE);
						break;
					case	PAD_RR:
						sprintf(punch,STR_CIRCLE);
						break;
					case	PAD_RD:
						sprintf(punch,STR_CROSS);
						break;
					case	PAD_FLT:
						sprintf(punch,"L1");
						break;
					case	PAD_FRT:
						sprintf(punch,"R1");
						break;
					case	PAD_FLB:
						sprintf(punch,"L2");
						break;
					case	PAD_FRB:
						sprintf(punch,"R2");
						break;
				}
			}
			if (PAD_Current->data[c0].input_mask&INPUT_MASK_KICK)
			{
				switch(PAD_Current->data[c0].pad_button)
				{
					case	PAD_RU:
						sprintf(kick,STR_TRI);
						break;
					case	PAD_RL:
						sprintf(kick,STR_SQUARE);
						break;
					case	PAD_RR:
						sprintf(kick,STR_CIRCLE);
						break;
					case	PAD_RD:
						sprintf(kick,STR_CROSS);
						break;
					case	PAD_FLT:
						sprintf(kick,"L1");
						break;
					case	PAD_FRT:
						sprintf(kick,"R1");
						break;
					case	PAD_FLB:
						sprintf(kick,"L2");
						break;
					case	PAD_FRB:
						sprintf(kick,"R2");
						break;
				}
			}
		}
	}
#endif
}
#endif

static	SWORD	timer_prev=0;

#undef	MIKE
void	OVERLAY_handle(void)
{
	Thing *darci = NET_PERSON(0);
	Thing *player = NET_PLAYER(0);
	SLONG	panel=1;

	//TRACE ( "OHi" );

#ifdef FINAL
	CBYTE str[8];
#else
	CBYTE str[32];
#endif

	/*

	if (darci && !(darci->Flags & FLAGS_ON_MAPWHO))
	{
		panel=0;
	}
	*/

	// Internal gubbins.
	POLY_flush_local_rot();



#ifdef TARGET_DC
	// Reset the viewport so that text, etc gets drawn even when in letterbox mode.

	//HRESULT hres = (the_display.lp_D3D_Viewport)->EndScene();

	D3DVIEWPORT2 viewData;
	memset(&viewData, 0, sizeof(D3DVIEWPORT2));
	viewData.dwSize = sizeof(D3DVIEWPORT2);

	// A horrible hack for letterbox mode.
	viewData.dwWidth  = 640;
	viewData.dwHeight = 480;
	viewData.dwX = 0;
	viewData.dwY = 0;
	viewData.dvClipX  = -1.0f;
	viewData.dvClipY  =  1.0f;
	viewData.dvClipWidth  = 2.0f;
	viewData.dvClipHeight = 2.0f;
	viewData.dvMinZ = 0.0f;
	viewData.dvMaxZ = 1.0f;
	HRESULT hres = (the_display.lp_D3D_Viewport)->SetViewport2 ( &viewData );

	//hres = (the_display.lp_D3D_Viewport)->BeginScene();

#endif



	PANEL_start();



	if (!EWAY_stop_player_moving())
	{
		if (panel)
		{
#ifndef TARGET_DC
			// This is all fucked - don't do it.
			PANEL_draw_buffered();
#endif
			OVERLAY_draw_gun_sights();
			OVERLAY_draw_enemy_health();
		}
	}

//	OVERLAY_draw_damage_values();
	if(panel)
	{
#ifdef	PSX
			PANEL_new_funky();
#else

/*
		if (Keys[KB_B])
		{
			PANEL_new_funky();
		}
		else
*/
		{

			if(!(GAME_STATE&GS_LEVEL_WON))
			{
				PANEL_last();
			}
		}
#endif
	}


//	if((GAME_TURN&15)==0)

	extern UBYTE draw_map_screen;
/*
	if (!draw_map_screen)
	{
		help_system();
	}
*/

#ifdef	MIKE
	{
		CBYTE	str[100];
		SLONG	count,cbl=0,c0;

	for(c0=0;c0<MAX_THINGS;c0++)
	{
		Thing	*p_thing;

		p_thing=TO_THING(c0);

		if(p_thing->Class==CLASS_SPECIAL)
		{
			if(p_thing->Genus.Special->SpecialType==SPECIAL_TREASURE)
			{
				cbl++;
			}
		}

	}


extern	UBYTE	global_person;
SLONG	count_draw_tween(void);
		count=count_draw_tween();

extern	SLONG	globdx,globdz;

//		sprintf(str," people used %d dt left%d (%d,%d)",global_person,count,globdx,globdz);
		sprintf(str,"specials %d",cbl);
		FONT2D_DrawString(str,100,20);
		globdx=-999;
		globdz=-999;
	}


#endif	
#if 0
	if(MFX_QUICK_still_playing())
	{
		FONT2D_DrawString("MFX QUICK STILL PLAYING",100,20);
	}
#endif


#ifdef	PSX

extern	UBYTE	combo_display;
#ifdef	VERSION_NTSC
	if(combo_display||timer_prev)
	{
		static	SWORD	timer=0;


		timer+=TICK_TOCK;


		if(timer>1500)
			timer=0;

		if(combo_display==1)
			timer_prev=10;
		else
		if(combo_display==2)
			timer_prev=-10;

		combo_display=0;


		if(timer_prev>0)
		{
			timer_prev--;
			if(timer>0 && timer<100||
				timer>200 && timer<300||
				timer>700 && timer<900)

				FONT2D_DrawString(punch,10,80);
		}
		else
		if(timer_prev<0)
		{
			timer_prev++;
			if(timer>0 && timer<100||
				timer>350 && timer<500||
				timer>700 && timer<850)

				FONT2D_DrawString(kick,10,80);
		}

	}
#endif
#endif



	if (!draw_map_screen)
	{
		// Waste not Want no, why have we got 50 bytes.

//		CBYTE	str[50];
		SLONG	crime;
/*
		if(!EWAY_stop_player_moving())
		if(CRIME_RATE_MAX)
		{
			//
			// we have a crime rate
			//
			crime=(CRIME_RATE*100)/CRIME_RATE_MAX;
			sprintf(str,"CRIME RATE %d%%",crime);
			FONT2D_DrawString(str,400,20);
		}
*/

		if(darci)
		{
			if(darci->State==STATE_SEARCH)
			{
#ifndef PSX
				/*

				SLONG percent = darci->Genus.Person->Timer1 >> 8;

				SATURATE(percent, 0, 100);

//				CBYTE	str[50];
				sprintf(str,"%d%%", percent);

				if ((darci->Genus.Person->Timer1 & 0xfff) < 3000 || percent == 100)
				{
					FONT2D_DrawStringCentred(
//						(percent == 100 ? "Complete" : "Searching"),
						XLAT_str( (percent == 100 ? X_COMPLETE : X_SEARCHING) ),
						DisplayWidth  / 2,
						DisplayHeight / 2 - 10,
						0x00ff00,
						256);
				}

				FONT2D_DrawStringCentred(
					str,
					DisplayWidth  / 2,
					DisplayHeight / 2 + 20,
					0x00ff00,
					512);

				*/
#else
extern void PANEL_draw_search(SLONG timer);

				PANEL_draw_search(darci->Genus.Person->Timer1);
#endif
			}
		}
	}

#ifndef PSX
	PANEL_inventory(darci, player);
#endif


//#if 0
				// I have found the offending code, Holmes!
				// Now we must teach these heathen a lesson in coding manners.
				// I shall fetch the larger of my beating sticks.
extern	UBYTE	cheat;


#ifndef TARGET_DC
	if(cheat==2)
	{
		CBYTE	str[50];
		ULONG	in;

extern	SLONG	tick_tock_unclipped;
		if(tick_tock_unclipped==0)
			tick_tock_unclipped=1;

#ifndef PSX
		extern SLONG SW_tick1;
		extern SLONG SW_tick2;
#endif
		extern	ULONG	debug_input;
		in=debug_input;

		extern	SLONG	geom;
		extern	SLONG	EWAY_cam_jumped;
		extern	SLONG	look_pitch;

//		sprintf(str,"(%d,%d,%d) fps %d up %d down %d left %d right %d geom %d",darci->WorldPos.X>>16,darci->WorldPos.Y>>16,darci->WorldPos.Z>>16,((1000)/tick_tock_unclipped)+1,in&INPUT_MASK_FORWARDS,in&INPUT_MASK_BACKWARDS,in&INPUT_MASK_LEFT,in&INPUT_MASK_RIGHT,geom);
#ifndef PSX

// for eidos build		sprintf(str,"(%d,%d,%d) fps %d render ticks %d",darci->WorldPos.X>>16,darci->WorldPos.Y>>16,darci->WorldPos.Z>>16,((1000)/tick_tock_unclipped)+1, SW_tick2 - SW_tick1);
		sprintf(str,"(%d,%d,%d) fps %d",darci->WorldPos.X>>16,darci->WorldPos.Y>>16,darci->WorldPos.Z>>16,((1000)/tick_tock_unclipped)+1);
#else
		sprintf(str,"(%d,%d,%d) fps %d",darci->WorldPos.X>>16,darci->WorldPos.Y>>16,darci->WorldPos.Z>>16,((1000)/tick_tock_unclipped)+1);
#endif

#ifdef	PSX
		FONT2D_DrawString(str,20,212,0xffffff,256);
		

#else
		FONT2D_DrawString(str,2,2,0xffffff,256);
#endif
	}
#endif

//#endif

	if (GAME_STATE & GS_LEVEL_LOST)
	{
#ifdef PSX
			PANEL_draw_eog(0);
#endif

#ifndef	PSX

		/*

		FONT2D_DrawStringCentred(
			"Level Lost",
			DisplayWidth  / 2,
			DisplayHeight / 2 - 10,
			0x00ff00,
			512);

			if(SAVE_VALID)
			{
				FONT2D_DrawStringCentred(
					"R-Load AutoSave    Space- Quit",
					DisplayWidth  / 2,
					DisplayHeight / 2 + 40,
					0x00ff00,
					512);
			}
			else
			{
				FONT2D_DrawStringCentred(
					"R- replay    Space- Quit",
					DisplayWidth  / 2,
					DisplayHeight / 2 + 40,
					0x00ff00,
					512);
			}

		*/

#endif
	}
	else
	if (GAME_STATE & GS_LEVEL_WON)
	{
#ifndef	PSX

		/*

		FONT2D_DrawStringCentred(
			"Level Complete",
			DisplayWidth  / 2,
			DisplayHeight / 2 - 10,
			0x00ff00,
			512);

		FONT2D_DrawStringCentred(
			"Space to Continue",
			DisplayWidth  / 2,
			DisplayHeight / 2 + 40,
			0x00ff00,
			512);

		*/
#else
		PANEL_draw_eog(1);
//		POLY2D_TextImage(IMAGE_LEVEL_COMPLETE,(DisplayWidth>>1)-98,(DisplayHeight>>1)-10,0x00ff00);
//		if (GAME_TURN&24)
//			draw_text_at(DISPLAYWIDTH-128,212,STR_CROSS" to Continue",0x00ffff);
#endif

	}

	/*

	//
	// Draw useful info...
	//

	{
		CBYTE str[58];

		sprintf(str, "%d", NET_PERSON(0)->Draw.Tweened->Roll);

		FONT2D_DrawStringCentred(
			str,
			DisplayWidth  / 2,
			30,
			0x00ff00,
			16);
	}

	*/

	PANEL_finish();
/*
	if (GAME_STATE & GS_LEVEL_WON)
	{
		extern void ScoresDraw(void);	// From attract

		ScoresDraw();
	}
*/

	//TRACE ( "OHo" );

}


/*
void	set_beacon(SLONG bx,SLONG by,SLONG bz)
{
	
	if(beacon_upto<(MAX_BEACON-1))
	{
		beacons[beacon_upto].X=bx;
		beacons[beacon_upto].Z=bz;
		beacon_upto++;
	}
	
}


ULONG	col_type[]=
{
	0xff0000,
	0xff0000,
	0x00ff00,
	0x00ffff,
	0x000080,
	0x0000a0,
	0x0000c0,
	0x0000e0,
	0x0000ff,
	0x0000ff,
	0x0000ff,
	0x0000ff,
	0x0000ff
};
*/

void	overlay_beacons(void)
{
#ifndef PSX
/*
	SLONG	c0;
	Thing			*t_thing;
	THING_INDEX		current_thing;

	current_thing	=	PRIMARY_USED;
	while(current_thing)
	{
		t_thing			=	TO_THING(current_thing);
		current_thing	=	t_thing->LinkChild;

		if(t_thing->Class==CLASS_PERSON)
		{
			map_beacon_draw(
				t_thing->WorldPos.X>>8,
				t_thing->WorldPos.Z>>8,
				col_type[t_thing->Genus.Person->PersonType],
				(t_thing->Genus.Person->PlayerID) ? BEACON_FLAG_POINTY : 0,
				t_thing->Draw.Tweened->Angle);
		}
	}
//	map_beacon_draw(NET_PERSON(0)->WorldPos.X>>8,NET_PERSON(0)->WorldPos.Z>>8);

	for(c0=1;c0<beacon_upto;c0++)
	{
		map_beacon_draw(beacons[c0].X,beacons[c0].Z,0x0000ff,BEACON_FLAG_BEACON,0);

	}
*/
#endif
}

#ifdef	DAMAGE_TEXT
SLONG	get_damage_index(void)
{
	SLONG	oldest=-1,oldest_age=-1;
	SLONG	c0;

	if(damage_value_upto<MAX_DAMAGE_VALUES)
	{
		damage_value_upto++;
		return(damage_value_upto-1);
	}

	for(c0=1;c0<damage_value_upto;c0++)
	{
		if(damage_values[c0].Age==-1)
			return(c0);

		if(damage_values[c0].Age>oldest_age)
		{
			oldest_age=damage_values[c0].Age;
			oldest=c0;
		}
	}
	return(oldest);
}

void	free_damage_index(SLONG index)
{
	if(index==damage_value_upto-1)
		damage_value_upto--;

	damage_values[index].Age=-1;
}
#endif
void	add_damage_value(SWORD x,SWORD y,SWORD z,SLONG value)
{
#ifdef	DAMAGE_TEXT
	SLONG	index;

	index=get_damage_index();
	if(index)
	{
		damage_values[index].X=x;
		damage_values[index].Y=y;
		damage_values[index].Z=z;
		damage_values[index].Age=0;
		damage_values[index].Value=value;
		damage_values[index].Type=INFO_NUMBER;
	}
	else
	{
//		ASSERT(0);
	}
#endif
	
}

void	add_damage_text(SWORD x,SWORD y,SWORD z,CBYTE *text)
{
#ifdef	DAMAGE_TEXT
	SLONG	index;

	index=get_damage_index();
	if(index)
	{
		damage_values[index].X=x;
		damage_values[index].Y=y;
		damage_values[index].Z=z;
		damage_values[index].Age=0;
		damage_values[index].text_ptr=text;
		damage_values[index].Type=INFO_TEXT;
	}
	else
	{
//
		ASSERT(0);
	}
#endif	
}

void	add_damage_value_thing(Thing *p_thing,SLONG value)
{
#ifdef	DAMAGE_TEXT
	SLONG	dx,dy,dz;

	calc_sub_objects_position(p_thing,p_thing->Draw.Tweened->AnimTween,SUB_OBJECT_HEAD,&dx,&dy,&dz);
	dx+=p_thing->WorldPos.X>>8;
	dy+=p_thing->WorldPos.Y>>8;
	dz+=p_thing->WorldPos.Z>>8;

	add_damage_value(dx,dy,dz,value);
#endif
	
}

#ifdef	DAMAGE_TEXT
void	OVERLAY_draw_damage_values(void)
{
	SLONG	c0;
	CBYTE	str[10];

	for(c0=1;c0<damage_value_upto;c0++)
	{
		if(damage_values[c0].Age>=0)
		{
			damage_values[c0].Age++;

extern	void FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

		
			{
				UWORD	fade;
				ULONG	col;
				fade=damage_values[c0].Age;

				if(damage_values[c0].Value<10)
				{
					col=0xff00;
				}
				else
				if(damage_values[c0].Value<20)
				{
					col=0x80ff00;
				}
				else
				if(damage_values[c0].Value<30)
				{
					col=0xffff00;
				}
				else
				{
					col=0xffffff;
				}



				switch(damage_values[c0].Type)
				{
					case	INFO_NUMBER:
						sprintf(str,"%d",damage_values[c0].Value);

						FONT2D_DrawString_3d(str, damage_values[c0].X,damage_values[c0].Y,damage_values[c0].Z, col, 512, fade);
						break;
					case	INFO_TEXT:
						FONT2D_DrawString_3d(damage_values[c0].text_ptr, damage_values[c0].X,damage_values[c0].Y,damage_values[c0].Z, col, 512, fade);
						break;

				}
			}


			if(damage_values[c0].Age>63)
			{
				free_damage_index(c0);
			}

			damage_values[c0].Y+=(damage_values[c0].Age>>4)+1;

		}
	}
}
#endif

void	init_overlay(void)
{
//	beacon_upto=1;
#ifdef	DAMAGE_TEXT
	damage_value_upto=1;
	memset((UBYTE*)damage_values,0,sizeof(struct DamageValue)*MAX_DAMAGE_VALUES);
#endif
//	memset((UBYTE*)beacons,0,sizeof(struct	Beacon)*MAX_BEACON);
	memset((UBYTE*)panel_enemy,0,sizeof(struct	TrackEnemy)*MAX_TRACK);

}
