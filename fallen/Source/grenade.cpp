// grenade.cpp
//
// grenade code

#include "game.h"
#include "sound.h"
#include "fmatrix.h"
#include "pcom.h"
#include "psystem.h"
#include "animate.h"
#include "dirt.h"
#include "poly.h"
#include "mesh.h"

#include "grenade.h"
#include "statedef.h"
#include "drawxtra.h"

// Grenade
//
// grenade structure

struct Grenade
{
	Thing*	owner;			// person who threw the grenade

	SLONG	x;				// position
	SLONG	y;
	SLONG	z;

	SWORD	yaw;			// orientation
	SWORD	pitch;

	SLONG	dx;				// velocity
	SLONG	dy;
	SLONG	dz;

	SWORD	dyaw;			// rotational velocity
	SWORD	dpitch;

	UWORD	timer;			// time to explosion
	UWORD	rsvd;			// reserved
};

#define	MAX_GRENADES	6

static Grenade	GrenadeArray[MAX_GRENADES];

// InitGrenades
//
// clear grenade data

void InitGrenades()
{
	for (int ii = 0; ii < MAX_GRENADES; ii++)
	{
		GrenadeArray[ii].owner = NULL;
	}
}

// CreateGrenade
//
// create a grenade - returns false if we can't

Grenade	*global_g;

bool CreateGrenade(Thing* owner, SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG timer)
{
	int ix;
	for (ix = 0; ix < MAX_GRENADES; ix++)
	{
		if (!GrenadeArray[ix].owner)	break;
	}

	if (ix == MAX_GRENADES)	return false;

	Grenade*	gp = &GrenadeArray[ix];

	global_g=gp;

	gp->owner = owner;

	gp->x = x;
	gp->y = y;
	gp->z = z;
	gp->yaw = 0;
	gp->pitch = 0;

	gp->dx = dx;
	gp->dy = dy;
	gp->dz = dz;

	gp->dyaw   = 0;//(Random() & 0x3f) - 0x1f;
	gp->dpitch = 50;

	gp->timer = timer;

	return true;
}

// CreateGrenadeFromPerson
//
// create a grenade throw by a person

bool CreateGrenadeFromPerson(Thing* p_person, SLONG timer)
{
	SLONG	vector[3];

	FMATRIX_vector(vector, p_person->Draw.Tweened->Angle, 0);

	SLONG	dx = -vector[0] * 181 >> 11;
	SLONG	dz = -vector[2] * 181 >> 11;
	SLONG	dy = 181 << 6;

	//
	// Is this an enemy attacking someone?
	//

	if (p_person->Genus.Person->PlayerID)
	{
	}
	else
	{
		SLONG i_target = PCOM_person_wants_to_kill(p_person);

		if (i_target)
		{
			Thing *p_target = TO_THING(i_target);

			SLONG dpx = abs(p_target->WorldPos.X - p_person->WorldPos.X >> 8);
			SLONG dpz = abs(p_target->WorldPos.Z - p_person->WorldPos.Z >> 8);

			SLONG pdist = QDIST2(dpx,dpz);

			if (pdist < 0x500)
			{
				dx >>= 1;
				dz >>= 1;
				dy >>= 1;
			}
		}
	}

	SLONG	px,py,pz;

	calc_sub_objects_position(p_person, p_person->Draw.Tweened->AnimTween, SUB_OBJECT_LEFT_HAND, &px, &py, &pz);

	px = (px << 8) + p_person->WorldPos.X;
	py = (py << 8) + p_person->WorldPos.Y;
	pz = (pz << 8) + p_person->WorldPos.Z;

	return CreateGrenade(p_person, px, py, pz, dx, dy, dz, timer);
}

// DrawGrenades
//
// draw grenades

void DrawGrenades()
{
	SLONG angle;

	for (int ii = 0; ii < MAX_GRENADES; ii++)
	{
		Grenade*	gp = &GrenadeArray[ii];

		if (gp->owner)
		{
			MESH_draw_poly(PRIM_OBJ_ITEM_GRENADE, gp->x >> 8, gp->y >> 8, gp->z >> 8, gp->yaw, gp->pitch, 0, NULL, 0xff);

			angle   = GAME_TURN;
			angle <<= 6;
			angle  &= 2047;

			SLONG dx = SIN(angle) >> 8;
			SLONG dz = COS(angle) >> 8;

			extern void BLOOM_draw(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG col, UBYTE opts);

			BLOOM_draw(
				(gp->x >> 8),
				(gp->y >> 8) + 8,
				(gp->z >> 8),
				dx,
				0,dz,
				0x007f5d,
				0);

			/*

			if (p_thing->SubState == SPECIAL_SUBSTATE_ACTIVATED)
			{
			  c0=p_thing->Genus.Special->timer;
			  c0=(c0<<3)&2047;
			  dx=SIN(c0)>>8;
			  dz=COS(c0)>>8;
			  BLOOM_draw(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+25,p_thing->WorldPos.Z>>8,
				dx,0,dz,0x007F5D,0);

			*/
		}
	}
}

// ProcessGrenades
//
// process grenades
void ProcessGrenade(Grenade* gp,SLONG explode,SLONG ii)
{
		if (!gp->owner)	return;

		// run countdown
		SLONG	ticks = 16 * TICK_RATIO >> TICK_SHIFT;

		if (gp->timer < ticks)
		{
			// grenade explodes
			if(explode)
				CreateGrenadeExplosion(gp->x, gp->y, gp->z, gp->owner);
			gp->owner = NULL;
			return;
		}
		else
		{
			gp->timer -= ticks;
		}

		// alert people to grenade
		if(explode)
		if (!(GAME_TURN & 0x0F))
		{
			PCOM_oscillate_tympanum(PCOM_SOUND_GRENADE_FLY, gp->owner, gp->x >> 8, gp->y >> 8, gp->z >> 8);
		}

		// run gravity
		gp->dy -= TICK_RATIO * 2;

		if (gp->dy < -0x2000)	gp->dy = -0x2000;

		// move it
		SLONG	oldx = gp->x;
		SLONG	oldy = gp->y;
		SLONG	oldz = gp->z;

		gp->x += (gp->dx * TICK_RATIO) >> TICK_SHIFT;
		gp->y += (gp->dy * TICK_RATIO) >> TICK_SHIFT;
		gp->z += (gp->dz * TICK_RATIO) >> TICK_SHIFT;

		gp->yaw		+= gp->dyaw;
		gp->pitch	+= gp->dpitch;

		// check for collisions
		SLONG	floor = (PAP_calc_map_height_at(gp->x >> 8, gp->z >> 8) + 11) << 8;

		if (gp->y < floor)
		{
			SLONG	under = floor - gp->y;

			if(explode)
			if (abs(gp->dy) > 0x80000)
			{
				// make a noise
				MFX_play_xyz(ii, S_KICK_CAN, MFX_REPLACE, gp->x, gp->y, gp->z);
				PCOM_oscillate_tympanum(PCOM_SOUND_GRENADE_HIT, gp->owner, oldx >> 8, oldy >> 8, oldz >> 8);
			}

			if ((under > 0x4000) || (gp->dy > 0))
			{
				// hit a building
				if ((oldx >> 16) != (gp->x >> 16))
				{
					gp->x = oldx;
					gp->dx = -(gp->dx >> 1);
				}

				if ((oldz >> 16) != (gp->z >> 16))
				{
					gp->z = oldz;
					gp->dz = -(gp->dz >> 1);
				}
			}
			else
			{
				// hit the floor
				gp->dy = abs(gp->dy) >> 1;
				gp->y = oldy;

				if (gp->dy > 0x400)
				{
//					gp->dx += ((Random() & 0x0F) - 0x07) << 8;
//					gp->dz += ((Random() & 0x0F) - 0x07) << 8;

//					gp->dyaw	+= (Random() & 0x7F) - 0x3F;
//					gp->dpitch	+= (Random() & 0x3F);
				}
				else
				{
					gp->dy = 0;
					gp->dx = 0;
					gp->dz = 0;
					gp->dyaw = 0;
					gp->dpitch = 0;
				}
			}
		}


}



void ProcessGrenades(void)
{
	for (int ii = 0; ii < MAX_GRENADES; ii++)
	{
		Grenade*	gp = &GrenadeArray[ii];

		ProcessGrenade(gp,1,ii);
	}
}

// CreateGrenadeExplosion
//
// cause a grenade to explode

void CreateGrenadeExplosion(SLONG x, SLONG y, SLONG z, Thing* owner)
{

	SLONG ytest = PAP_calc_map_height_at(x>>8,z>>8)<<8;

	if (y<=ytest) y=ytest+1;

	MFX_play_xyz(0, S_EXPLODE_START, MFX_OVERLAP, x, y, z);

	PARTICLE_Add(x, y, z, 0, 0, 0, (Random()&1)?POLY_PAGE_EXPLODE1_ADDITIVE:POLY_PAGE_EXPLODE2_ADDITIVE, 2,
									0x00FFFFFF, PFLAG_SPRITEANI|PFLAG_RESIZE|PFLAG_BOUNCE, 120, 190+(Random()&0x3f), 1, 0, 20);
	PARTICLE_Add(x+(((Random()&0xff)-0x7f)<<4), y+(((Random()&0xff)-0x7f)<<4), z+(((Random()&0xff)-0x7f)<<4),
		0, 0, 0, (Random()&1)?POLY_PAGE_EXPLODE1_ADDITIVE:POLY_PAGE_EXPLODE2_ADDITIVE, 2,
									0x7fFFFFFF, PFLAG_SPRITEANI|PFLAG_RESIZE, 70, 120+(Random()&0x7f), 1, 0, 40);



	int iNumParticles = IWouldLikeSomePyroSpritesHowManyCanIHave ( 20 * 4 );
	iNumParticles /= 4;

#ifdef TARGET_DC
	for (int ii = 0; ii < iNumParticles; ii++)
	{
		// Smoky cloud.
		PARTICLE_Add(x + (((Random()&0x7f)-0x3f)<<9), y + (((Random()&0x3f)-0x1f)<<6), z + (((Random()&0x7f)-0x3f)<<9),
						((Random()&0x1f)-0xf)<<1, (1+(Random()&0xf))<<7, ((Random()&0x1f)-0xf)<<1,
						POLY_PAGE_SMOKECLOUD, 2, 0xFFFFFFFF, PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE|PFLAG_RESIZE,
						150+(Random()&0x3f), 20+(Random()&0x7f), 1, 2+(Random()&3), 3);

		// Flamey bits of the cloud.
		PARTICLE_Add(x + (((Random()&0x7f)-0x3f)<<9), y + (((Random()&0x3f)-0x1f)<<6), z + (((Random()&0x7f)-0x3f)<<9),
						((Random()&0x1f)-0xf)<<1, (1+(Random()&0xf))<<6, ((Random()&0x1f)-0xf)<<1,
						(Random()&1)?POLY_PAGE_EXPLODE1_ADDITIVE:POLY_PAGE_EXPLODE2_ADDITIVE, 2,
						0x7FFFFFFF, PFLAG_SPRITEANI|PFLAG_FADE|PFLAG_RESIZE,
						250+(Random()&0x3f), 50+(Random()&0x7f), 1, 2+(Random()&3), 3);


		// Little bouncy particles.
#if 0
		// These are all too small to see. I dunno - either they're too small or they're too big....
		PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<6, (Random()&0xf)<<6, ((Random()&0x1f)-0xf)<<6,
						POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE,
						240, 20+(Random()&0x1f), 1, 3+(Random()&3), 0);

		if (Random()&3)
		{
			PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<8, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<8,
						POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&1)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_FADE|PFLAG_BOUNCE,
						240, 3, 1, 2+(Random()&3), 0);
		}
		else
		{
			PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<12, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<12,
						POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_FADE,
						240, 3, 1, 3, 0);
		}
#endif
	}



#else

	for (int ii = 0; ii < iNumParticles; ii++)
	{
		PARTICLE_Add(x + (((Random()&0x7f)-0x3f)<<9), y + (((Random()&0x3f)-0x1f)<<6), z + (((Random()&0x7f)-0x3f)<<9),
						((Random()&0x1f)-0xf)<<1, (1+(Random()&0xf))<<6, ((Random()&0x1f)-0xf)<<1,
						POLY_PAGE_SMOKECLOUD, 2, 0xFFFFFFFF, PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE|PFLAG_RESIZE,
						150, 20+(Random()&0x7f), 1, 2+(Random()&3), 3);
		PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<6, (Random()&0x1f)<<6, ((Random()&0x1f)-0xf)<<6,
						POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA,
						240, 20+(Random()&0x1f), 1, 3+(Random()&3), 0);
#ifdef	PSX
		if(ii&1)
		{
#endif


			if (Random()&3)
				PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<8, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<8,
							POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&1)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA|PFLAG_BOUNCE,
							240, 5, 1, 2+(Random()&3), 0);
			else
				PARTICLE_Add(x, y, z, ((Random()&0x1f)-0xf)<<12, (Random()&0x1f)<<8, ((Random()&0x1f)-0xf)<<12,
							POLY_PAGE_EXPLODE1-(Random()&1), 2+((Random()&3)<<2), 0xFFFFFF, PFLAG_GRAVITY|PFLAG_RESIZE2|PFLAG_FADE|PFLAG_INVALPHA,
							240, 5, 1, 3, 0);
#ifdef	PSX
		}
#endif
	}

#endif

	SLONG	sx = x >> 8;
	SLONG	sy = y >> 8;
	SLONG	sz = z >> 8;

	DIRT_new_sparks(sx,sy,sz,2);
	DIRT_new_sparks(sx,sy,sz,2|64);
	DIRT_new_sparks(sx,sy,sz,2|128); // lots of sparks

	DIRT_gust(0, sx, sz, sx + 200, sz);
	DIRT_gust(0, sx, sz, sx - 200, sz);
	DIRT_gust(0, sx, sz, sx, sz + 200);
	DIRT_gust(0, sx, sz, sx, sz - 200);

	create_shockwave(sx, sy, sz, 0x300, 500, owner);
}

void PANEL_draw_gun_sight(SLONG mx,SLONG my,SLONG mz,SLONG accuracy,SLONG scale);

#ifndef PSX
SLONG GAMEMENU_is_paused(void);
SLONG GAMEMENU_slowdown_mul();

void	show_grenade_path(Thing *p_person)
{
	Grenade*	gp ;
	SLONG	x,y,z,x1,y1,z1;
	SLONG	count=(-GAME_TURN);

	if (GAMEMENU_is_paused())
		return;

	if (GAMEMENU_slowdown_mul()==256)
	if(p_person->State==STATE_IDLE)
	if(CreateGrenadeFromPerson(p_person, 6000))
	{
		gp=global_g;

		x=gp->x>>8;
		y=gp->y>>8;
		z=gp->z>>8;

		while(gp->owner)
		{

			ProcessGrenade(gp,0,0);
			x1=gp->x>>8;
			y1=gp->y>>8;
			z1=gp->z>>8;


			if ( ( ( count&7 ) == 0 ) || ( ( count&7 ) == 1 ) )
			{
				AENG_world_line_nondebug (
					x,y,z,
					3,
					0x8000af00,
					x1,y1,z1,
					3,
					0x8000af00,
					TRUE);
			}



			count++;
			x=x1;
			y=y1;
			z=z1;
		}
		PANEL_draw_gun_sight(x,y,z, 1000, 400);

	}
}
#endif
