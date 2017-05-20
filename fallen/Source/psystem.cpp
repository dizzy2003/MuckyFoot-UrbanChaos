//
// psystem.cpp
//
// Proper particle system for handling a wide variety of effects:
// smoke, dust, mud, sparks, blood...
//

#include "game.h"
#include "psystem.h"
#include "mav.h"
#include "FMatrix.h"
#include "C:\fallen\DDEngine\Headers\poly.h"
#include "animate.h"
#include "interact.h"
#include "sound.h"
#include "pcom.h"
#include "pow.h"
#include "fc.h"

// pvt globals...

Particle		particles[PSYSTEM_MAX_PARTICLES];
SLONG			next_free, next_used, particle_count;

// nick this bit

#ifdef	psx
//UBYTE fire_pal[768];
#else
extern UBYTE fire_pal[768];
#endif

static SLONG prev_tick;
static BOOL first_pass;

void PARTICLE_Reset() {
	SLONG c0;

	memset((UBYTE*)particles,0,sizeof(particles));
	particle_count=0;

	for (c0=0;c0<PSYSTEM_MAX_PARTICLES;c0++) {
		particles[c0].next=c0+1;
		particles[c0].prev=c0-1;
	}
	particles[PSYSTEM_MAX_PARTICLES-1].next=0;
	// sacrifice particles[0] to the readability gods...
	next_free=1; next_used=0;
	prev_tick = GetTickCount();
	first_pass = 1;

}

void PARTICLE_Run() {
	SLONG ctr, tx,ty,tz;
	Particle *p;
	SLONG	trans;
	UBYTE*  palptr;
	SLONG   palndx;
	UBYTE isWare;

	
	isWare=(FC_cam->focus->Class == CLASS_PERSON && FC_cam->focus->Genus.Person->Ware);


	// let's try something cunning with tick_shift ...
	// I'm sure this shouldn't SLONG
	//SLONG local_ratio, local_shift;
	ULONG local_ratio, local_shift;
#ifndef PSX
	SLONG cur_tick, tick_diff;

	cur_tick=GetTickCount();
	tick_diff=cur_tick-prev_tick;

	if(first_pass)
	{
		tick_diff=NORMAL_TICK_TOCK;
		first_pass=0;
	}
	local_shift=TICK_SHIFT;
	local_ratio=(tick_diff<<local_shift)/(NORMAL_TICK_TOCK);

	if (local_ratio<256) // we'll get big problems
	{
		local_ratio=0;	 // so ignore it for now, and let it catch up later
	}
	else
		prev_tick=cur_tick;

#else
	local_ratio = TICK_RATIO;
	local_shift = TICK_SHIFT;
#endif

	if (!particle_count) return;

//	p=particles;
//	for (ctr=0;ctr<PSYSTEM_MAX_PARTICLES;ctr++,p++)
	for (p=particles+next_used;p!=particles;)
//		if (p->priority) 
		{
			// these are nice n smooth always (not local_ratio'd)
			tx=p->x+((p->dx*TICK_RATIO)>>TICK_SHIFT);
			ty=p->y+((p->dy*TICK_RATIO)>>TICK_SHIFT);
			tz=p->z+((p->dz*TICK_RATIO)>>TICK_SHIFT);

			if (p->flags & PFLAG_GRAVITY) {
				p->dy-=(0xff*TICK_RATIO)>>TICK_SHIFT;
			}

				if (p->flags & PFLAG_SPRITEANI) {
				  p->sprite+=4;
				}

			if (local_ratio>255) 
			{
/*				if (p->flags & PFLAG_SPRITEANI) {
				  p->sprite+=(TICK_RATIO>>TICK_SHIFT)<<2;
				}*/

				if (p->flags & PFLAG_WANDER) {
				  p->dx+=((rand()&0x1f)-0xf)*4;
				  p->dy+=((rand()&0x1f)-0xf)*4;
				  p->dz+=((rand()&0x1f)-0xf)*4;
				}

				if (p->flags & PFLAG_DRIFT) {
	/*				static SLONG tick=0;
					tick+=8;
				  p->dx+=abs(SIN((p->y+tick)&2047))/2048;
				  p->dz+=abs(SIN((p->y+tick)&2047))/2048;
				  p->dy+=abs(COS((p->y+tick)&2047))/2048;*/
				}

				if (p->flags & PFLAG_DAMPING) {
					p->dx=(p->dx*245)>>8;
					p->dz=(p->dz*245)>>8;
				}

/*				if (p->flags & PFLAG_BOUYANT) {
					// do this after we actually HAVE water in the game again...
				}*/

				if ((p->flags & PFLAG_FADE2)&&(p->life&1)) {
					// I'm sure this shouldn't a SLONG
					//SLONG diff=0x01000000*((p->fade*local_ratio)>>local_shift);
					ULONG diff=0x01000000*((p->fade*local_ratio)>>local_shift);
					if (p->flags & PFLAG_INVALPHA) {
						if ((p->colour&0xFF000000)<0xFF000000-diff)
						  p->colour+=diff;
						else
						  p->life=1; // killlll
					} else {
						if ((p->colour&0xFF000000)>diff)
						  p->colour-=diff;
						else
						  p->life=1; // killlll
					}
				}

				if (p->flags & PFLAG_FADE) {
					// I'm sure this shouldn't a SLONG
					//SLONG diff=0x01000000*((p->fade*local_ratio)>>local_shift);
					ULONG diff=0x01000000*((p->fade*local_ratio)>>local_shift);
					if (p->flags & PFLAG_INVALPHA) {
						if ((p->colour&0xFF000000)<0xFF000000-diff)
						  p->colour+=diff;
						else
						  p->life=1; // killlll
					} else {
						if ((p->colour&0xFF000000)>diff)
						  p->colour-=diff;
						else
						  p->life=1; // killlll
					}
				}
	#ifndef PSX
				if (p->flags & PFLAG_FIRE) {
					trans=(p->colour&0xFF000000)>>24;
					palptr=(trans*3)+fire_pal;
					palndx=(256-trans)*3;
					trans<<=24;
					p->colour=trans+(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				}
	#endif

				if (p->flags & PFLAG_BOUNCE) {
					SLONG tmpy=PAP_calc_map_height_at(tx>>8,tz>>8)<<8;
					if (ty<tmpy) { 
						ty-=tmpy;
						ty=tmpy-ty;
					  p->dy=-(p->dy*180)>>8; //dy*=-0.9;
					  if (abs(p->dy)<256) { // friction applies
						p->dx=(p->dx*180)>>8;
						p->dz=(p->dz*180)>>8;
					  }
					}
				} 
				else
//				if (MAV_inside(tx>>8,ty>>8,tz>>8))
				if ((ty>>8)<PAP_calc_map_height_at(tx>>8,tz>>8))
				{
					if (p->flags & PFLAG_COLLIDE) {
						// twiddle...
					} else {
						if (!isWare)			

						p->life=1; // the -- at end will remove
	//					TRACE("psystem: collide-removed\n");
					}

					if (p->flags & PFLAG_EXPLODE_ON_IMPACT)
					{
						/*POW_create(
							POW_CREATE_MEDIUM,
							tx,
							ty,
							tz,0,0,0);*/
						GameCoord pos = { tx, 0, tz };
						pos.Y=PAP_calc_map_height_at(tx>>8,tz>>8)<<8;
						pos.Y+=1024;
						Thing *pyro = PYRO_create( pos, PYRO_WHOOMPH );
						if (pyro)
							pyro->Genus.Pyro->counter=0; // heh

						create_shockwave(
							tx >> 8,
							ty >> 8,
							tz >> 8,
							0x140,
							80,
							NULL);
					}
				}

//				if ((p->flags & PFLAG_LEAVE_TRAIL)&&(GAME_TURN&3))
				if (p->flags & PFLAG_LEAVE_TRAIL)
				{
					PARTICLE_Add(
						tx + (Random() & 0x9fff) - 0x4fff,
						ty + (Random() & 0x9fff) - 0x4fff,
						tz + (Random() & 0x9fff) - 0x4fff,
						0,0,0,
						POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x7FFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
						300,40,1,3,-1);
					PARTICLE_Add(
						tx + (Random() & 0x4fff) - 0x27ff,
						ty + (Random() & 0x4fff) - 0x27ff,
						tz + (Random() & 0x4fff) - 0x27ff,
						p->dx + (Random() & 0xfff) - 0x7ff          >> 3,
						p->dy + (Random() & 0xfff) - 0x7ff			>> 3,
						p->dz + (Random() & 0xfff) - 0x7ff          >> 3,
						POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x7FFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
						300,60,1,2,-1);
/*					PARTICLE_Add(
						tx + (Random() & 0x3fff) - 0x1fff,
						ty + (Random() & 0x3fff) - 0x1fff,
						tz + (Random() & 0x3fff) - 0x1fff,
						p->dx + (Random() & 0xfff) - 0x7ff          >> 3,
						p->dy + (Random() & 0xfff) - 0x7ff + 0x4000 >> 3,
						p->dz + (Random() & 0xfff) - 0x7ff          >> 3,
						POLY_PAGE_SMOKECLOUD,
						2 + ((Random() & 0x3) << 2),
						0x7fffffff,
						PFLAG_SPRITEANI | PFLAG_SPRITELOOP | PFLAG_FADE,
						100,
						100,
						1,
						1,
						1);*/
				}
				
				if (p->flags & PFLAG_RESIZE) {
					SLONG temp=p->size;
					
					temp+=(p->resize*local_ratio)>>local_shift;
					if (temp<1) { temp=1; p->life=1; } // clear 0-size or less particles
					if (temp>255) temp=255;
					p->size=temp;
				}
				if (p->flags & PFLAG_RESIZE2) {
					SLONG temp=p->size;
					
					temp+=(p->resize*local_ratio)>>local_shift;
					if (temp<1) p->life=1; 
					SATURATE(temp,1,65535);
					p->size=temp;
				}

				if (p->flags & PFLAG_HURTPEOPLE)
				{
					if (GAME_TURN & 0x1)
					{	
						SLONG i;

						UWORD hurt[8];
						SLONG num;

						THING_INDEX i_hurt;
						
						num = THING_find_sphere(
								p->x >> 8,
								p->y >> 8,
								p->z >> 8,
								0xa0,
								hurt,
								8,
								1 << CLASS_PERSON);

						for (i = 0; i < num; i++)
						{
							Thing *p_hurt = TO_THING(hurt[i]);

							extern SLONG is_person_dead(Thing *p_person);

							if (is_person_dead(p_hurt))
							{
								//
								// Don't hurt dead people!
								//
							}
							else
							{
								SLONG dx = abs(p->x - p_hurt->WorldPos.X >> 8);
								SLONG dz = abs(p->z - p_hurt->WorldPos.Z >> 8);

								SLONG dist = QDIST2(dx,dz);

								if (abs(dist) < 0x40)
								{
									SLONG junkx;
									SLONG junkz;

									SLONG ytop;
									SLONG ybot;

									calc_sub_objects_position(
										p_hurt,
										0,
										SUB_OBJECT_LEFT_FOOT,
									   &junkx,
									   &ybot,
									   &junkz);

									calc_sub_objects_position(
										p_hurt,
										0,
										SUB_OBJECT_HEAD,
									   &junkx,
									   &ytop,
									   &junkz);

									ybot <<= 8;
									ytop <<= 8;

									ybot += p_hurt->WorldPos.Y;
									ytop += p_hurt->WorldPos.Y;

									if (WITHIN(p->y, ybot - 0x2000, ytop + 0x2000))
									{
										PainSound(p_hurt);	// scream yer lungs out!

										if ((p_hurt->Genus.Person->Flags2 & FLAG2_PERSON_INVULNERABLE) ||
											(p_hurt->Genus.Person->pcom_bent & PCOM_BENT_PLAYERKILL))
										{
											//
											// Nothing hurts this person.
											//
										}
										else
										{
											p_hurt->Genus.Person->Health -= (0x40 - dist >> 4) + 1;
										}

										if (p_hurt->Genus.Person->Health <= 0)
										{
											set_person_dead(
												p_hurt,
												NULL,
												PERSON_DEATH_TYPE_OTHER,
												FALSE,
												FALSE);
										}
									}
								} // (abs(dist) < 0x40)
							} // is_person_dead() ... else
						} // (i_hurt)
					} // (GAME_TURN & 0x1)
				} // (p->flags & PFLAG_HURTPEOPLE)

			} // (local_ratio>255)

			p->x=tx;
			p->y=ty;
			p->z=tz;


			// ending too soon is preferable to ending too late
			// ending too late is Real Bad
#ifdef	PSX
//			if ((local_ratio>>local_shift))
//				p->life-=local_ratio>>local_shift;
//			else
				p->life-=2;
#else
				SLONG temp=local_ratio>>local_shift;
//			TRACE("old life: %d   subtracting: %d\n",p->life,temp);
			p->life-=local_ratio>>local_shift;
#endif
			if (p->life<=0) {
				SWORD idx = p-particles, temp = p->prev;
				p->priority=0;
				particle_count--;

				// pull it off the used list
				if (p->next) particles[p->next].prev=p->prev;
				if (p->prev) particles[p->prev].next=p->next;
				if (next_used==idx) {
					next_used=p->prev;
					particles[next_used].next=0;
				}
				p->prev=0;

				// join it onto the free list
				particles[next_free].prev=idx;
				p->next=next_free;
				next_free=idx;

				p=particles+temp;

/*				// experimental psystem speeder-upper
				next_free=p-particles;*/
			} else p=particles+p->prev;
		}
}


UWORD PARTICLE_AddParticle(Particle &p) {
//	UBYTE priority=0;
//	UWORD ctr=0;
	UWORD new_particle;


extern SLONG GAMEMENU_menu_type;
	if (GAMEMENU_menu_type != 0/*GAMEMENU_MENU_TYPE_NONE*/)
	{
		// Some sort of pause mode is up - don't make any more particles.
		return 0;
	}


	if (particle_count==PSYSTEM_MAX_PARTICLES) return 0;

/*	while ((particles[next_free].priority>priority)&&(ctr<1000)) {
		next_free++;
		if (next_free==PSYSTEM_MAX_PARTICLES) next_free=0;
		ctr++;
		if ((ctr>200)&&(priority<p.priority)) priority++;
	}
	if (ctr<1000) {
		if (!particles[next_free].priority) particle_count++;
		particles[next_free]=p;
		return particle_count;
	}
	return 0;*/

	// list version...

	if (!next_free) return 0;

	if (p.flags & PFLAG_RESIZE2) {
		p.size<<=8;
		if (!p.resize) {
			p.resize=-(p.size/p.life);
		}
	}

	// pull the particle off the list
	new_particle = next_free;
	next_free=particles[next_free].next;
	particles[next_free].prev=0;
	
	// set its contents
	particles[new_particle]=p;
	particles[new_particle].next=0; // part of pulling off the list, but JIC...
	particles[new_particle].priority=1;

	// join it onto the used list
	particles[next_used].next=new_particle;
	particles[new_particle].prev=next_used;
	next_used = new_particle;

/*	CBYTE msg[10];
	itoa(particle_count,msg,10);
	CONSOLE_text(msg,1000);*/

	return ++particle_count;
}

UWORD PARTICLE_Add(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, UWORD page, UWORD sprite, SLONG colour, SLONG flags, SLONG life, UBYTE size, UBYTE priority, SBYTE fade, SBYTE resize) {
	Particle p;
	p.x=x; p.y=y; p.z=z;
	p.dx=dx; p.dy=dy; p.dz=dz;
	p.page=page; p.sprite=sprite;
	p.colour=colour; p.flags=flags;
	p.life=life; p.size=size; p.priority=priority;
	p.resize=resize; p.fade=fade;
	return PARTICLE_AddParticle(p);
}

//
// -------------------------------------------------------------------------------------
// Shortcuts to some of the more commonly-used effects:
//
#ifndef	PSX
UWORD PARTICLE_Exhaust(SLONG x, SLONG y, SLONG z,UBYTE density,UBYTE disperse) {
	UBYTE i;
	UWORD res;
	Particle p;

	res=1;
	p.page=POLY_PAGE_STEAM; p.sprite=1+((rand()&3)<<2);
	p.colour=0x7FFFFFFF; p.flags=PFLAGS_SMOKE|PFLAG_WANDER;
	p.life=50; p.priority=1;
	p.size=4; p.resize=6; 
	p.fade=disperse;
	p.dx=0; p.dy=0; p.dz=0;
	for (i=density;i&&res;i--) {
		p.x=x+(rand()&0xff)-0x7f;
		p.y=y+(rand()&0xff)-0x7f;
		p.z=z+(rand()&0xff)-0x7f;
		res=PARTICLE_AddParticle(p);
	}
	return res;
}

UWORD PARTICLE_Exhaust2(Thing *object, UBYTE density, UBYTE disperse) 
{
	UBYTE i;
	UWORD res;
	Particle p;
	SLONG x,y,z,dx,dy,dz,ox,oy,oz;
//	float matrix[9], vector[3];
	SLONG matrix[9], vector[3];
/*	float yaw;
	float pitch;
	float roll;
	*/
//	SLONG matrix[9], vector[3], yaw, pitch, roll;
	SLONG vel;
	
	vel=1024-(object->Velocity*128);
	switch (object->DrawType) {
	case DT_MESH:
	case DT_VEHICLE:
/*		yaw   = -float(object->Draw.Mesh->Angle)   * (2.0F * PI / 2048.0F);
		pitch = -float(object->Draw.Mesh->Tilt) * (2.0F * PI / 2048.0F);
		roll  = -float(object->Draw.Mesh->Roll)  * (2.0F * PI / 2048.0F);*/
//		MATRIX_calc(matrix, yaw, pitch, roll);
		if (object->Class==CLASS_BIKE) {
//			vel=BIKE_get_speed(object);
//			TRACE("bikevel %d\n",vel);
//			vel=4096+vel;
			vel=0; // bike_get_speed does not take account of direction :-(
		}
		FMATRIX_calc(matrix, object->Draw.Mesh->Angle, object->Draw.Mesh->Tilt, object->Draw.Mesh->Roll);
		FMATRIX_TRANSPOSE(matrix);
		vector[2]=vel; vector[1]=0; vector[0]=0; 
		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
		dx=vector[0]; dy=vector[1]; dz=vector[2];
/*		if (object->Class==CLASS_BIKE) {
		  ox=oy=oz=0;
		} else {*/
		  ox=dx; oy=dy; oz=dz;
//		}
		break;
	case DT_TWEEN:
	case DT_ROT_MULTI:
/*		yaw   = -float(object->Draw.Tweened->Angle)   * (2.0F * PI / 2048.0F);
		pitch = -float(object->Draw.Tweened->Tilt) * (2.0F * PI / 2048.0F);
		roll  = -float(object->Draw.Tweened->Roll)  * (2.0F * PI / 2048.0F);*/
//		MATRIX_calc(matrix, yaw, pitch, roll);
		FMATRIX_calc(matrix, object->Draw.Mesh->Angle, object->Draw.Mesh->Tilt, object->Draw.Mesh->Roll);
		vector[2]=vel; vector[1]=0; vector[0]=0; 
		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
		dx=vector[0]; dy=vector[1]; dz=vector[2];
		ox=dx; oy=dy; oz=dz;
		break;
	default:
		dx=dy=dz=0;
		ox=oy=oz=0;
	}

	x=object->WorldPos.X+ox;
	y=object->WorldPos.Y+oy+0x600;
	z=object->WorldPos.Z+oz;
	ox=oy=oz=0;
	res=1;
	p.page=POLY_PAGE_STEAM; p.sprite=1+((rand()&3)<<2);
	p.colour=0x3FFFFFFF; p.flags=PFLAGS_SMOKE|PFLAG_WANDER;
	p.life=50; p.priority=1;
	p.size=4; p.resize=6; 
	p.fade=disperse;
//	p.dx=0; p.dy=0; p.dz=0;
	p.dx=dx/2; p.dy=dy/2; p.dz=dz/2;

	#ifdef BIKE
	if (object->Class==CLASS_BIKE) {
	  vel=BIKE_get_speed(object);
	  if (vel>10) vel=0x3FFF; else vel=0xFFF;
	}
	#endif

	for (i=density;i&&res;i--) {

		if (object->Class==CLASS_BIKE) {
		  vector[2]=18768+(rand()&vel); vector[1]=4048; vector[0]=0;
		  FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
		  ox=vector[0]; oy=vector[1]; oz=vector[2];
		}
		
		p.x=ox+x+(rand()&0xff)-0x7f;
		p.y=oy+y+(rand()&0xff)-0x7f;
		p.z=oz+z+(rand()&0xff)-0x7f;
		res=PARTICLE_AddParticle(p);
	}
	return res;
}

UWORD PARTICLE_Steam(SLONG x, SLONG y, SLONG z, UBYTE axis, SLONG vel, SLONG range, UBYTE time) {
	Particle p;
	UBYTE i,dir;
	SLONG res=1;
	SLONG dx,dy,dz,rx,ry,rz;

	switch(axis) {
	case 0:
		dx=vel; dy=dz=0;
		rx=0; ry=rz=range;
		break;
	case 1:
		dx=dz=0; dy=vel;
		rx=rz=range; ry=0;
		break;
	case 2:
		dx=dy=0; dz=vel;
		rx=ry=range; rz=0;

	}

	p.x=x; p.y=y; p.z=z;
	p.page=POLY_PAGE_STEAM; p.sprite=1+((rand()&3)<<2);
	p.colour=0x00FFFFFF+(time<<24); p.flags=PFLAGS_SMOKE|PFLAG_WANDER;
	p.life=45; p.priority=1;
	p.size=4; p.resize=3; p.fade=2;
	for (i=8;i&&res;i--) {
		p.dx=dx;
		p.dy=dy;
		p.dz=dz;
		if (rx) p.dx+=(rand()%rx);
		if (ry) p.dy+=(rand()%ry);
		if (rz) p.dz+=(rand()%rz);
		res=PARTICLE_AddParticle(p);
	}
	return res;
}


UWORD PARTICLE_SGrenade(Thing *object, UBYTE time) {
	Particle p;
	UBYTE i;
	SLONG res=1;

	p.page=POLY_PAGE_SMOKECLOUD2; p.sprite=2+((rand()&3)<<2);
	p.colour=0x7FFFFFFF; p.flags=PFLAGS_SMOKE|PFLAG_DRIFT|PFLAG_SPRITEANI|PFLAG_SPRITELOOP;
	p.life=80; p.priority=1;
	p.size=6; p.resize=6; 
	p.fade=3;
	for (i=3;i&&res;i--) {
		p.x=object->WorldPos.X			+ (rand()&0x3FFF)-0x1FFF;
		p.y=object->WorldPos.Y+0x600;
		p.z=object->WorldPos.Z			+ (rand()&0x3FFF)-0x1FFF;
		p.dx=(rand()&0x7FF)-0x3FF; 
		p.dy=(rand()&0x3FF)+0x100; 
		p.dz=(rand()&0x7FF)-0x3FF; 
		res=PARTICLE_AddParticle(p);
	}
	return res;
}

#endif

