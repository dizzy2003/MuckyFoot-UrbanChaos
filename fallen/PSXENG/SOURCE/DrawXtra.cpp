/*
** File: DrawXtra.cpp
**
** PSX Version of various Extra functions
**
** Author: James Watson (with help/hinderance from M.Adami & M.Rosenfeld
*/

#include	"game.h"
#include	"facet.h"
#include	"c:\fallen\DDEngine\headers\DrawXtra.h"
#include	"poly.h" 
#include	"psxeng.h"
#include	"c:\fallen\headers\fmatrix.h"
#include	"c:\fallen\psxeng\headers\engine.h"
#include	"c:\fallen\headers\thing.h"
#include	"c:\fallen\headers\psystem.h"
#include	"c:\fallen\headers\dirt.h"
#include	"c:\fallen\headers\animate.h"
#include	"c:\fallen\headers\spark.h"
#include	"c:\fallen\headers\statedef.h"
#include	"c:\fallen\headers\sound_id.h"
#include	"mfx.h"
#include	"c:\fallen\headers\pow.h"

extern PSX_POLY_Point *perm_pp_array;

SLONG	steam_seed;

SLONG	get_steam_rand(void)
{
	steam_seed*=31415965;
	steam_seed+=123456789;
	return(steam_seed>>8);
}

void	draw_flames(SLONG x,SLONG y,SLONG z,SLONG lod,SLONG offset)
{
	SLONG	c0;
	SLONG	trans;
	SLONG   page;
	SLONG	scale;
//	float   scale;
//	float   u,v,w,h;
	UWORD	u,v;//,w,h;
//	UBYTE*  palptr;
//	SLONG   palndx;
//	float   wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4;
	//
	// waft gently up from x,y,z
	//

	steam_seed=54321678+offset;

	for(c0=0;c0<lod*3;c0+=1)
	{
		SLONG	dx,dy,dz;

//		w=h=32;
		u=v=0;

		dy=get_steam_rand()&0x1ff;
		dy+=(GAME_TURN*5);
		dy%=384;
		dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
		dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;

		if (page==POLY_PAGE_FLAMES) 
		{
			dx>>=2; dz>>=2;
		}

		switch(c0&3)
		{
		case 0:
			page=POLY_PAGE_FLAMES;
			break;
		case 1:
		case 2:
			page=POLY_PAGE_SMOKE;
			break;
		case 3:
			page=POLY_PAGE_FLAMES2;
			dy=(GAME_TURN+c0)/2;
			u=(dy&3);
			v=((dy&12)>>2);
//			w=h=32;
			dy=0;
		}

		trans=255-dy;

		if(trans>=1)
		{
			trans>>=1;
			switch (page) {
			  case POLY_PAGE_FLAMES:
//			    palptr=(trans*3)+fire_pal;
//				palndx=(256-trans)*3;
//				trans<<=24;
				trans+=(trans<<8)|(trans<<16);
//				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				//scale=(dy>>2)+50;
				scale=(dy>>4)+50;
				break;
			  case POLY_PAGE_FLAMES2:
				trans=0x3F3F3F;
				scale=100;
//				dy=y;
				break;
			  case POLY_PAGE_SMOKE:
				trans+=(trans<<8)|(trans<<16);//|(trans<<24);
				scale=((dy-y)>>1)+50;
				dy+=50;
				break;
			}

			dx+=x;
			dy+=y;
			dz+=z;

			SPRITE_draw(
				dx,
				dy,
				dz,
				scale,
				trans,
				(page==POLY_PAGE_SMOKE)?0x7f000000:0xff000000,
				page+(u+(v<<3)),
//				u, v, w, h,
//				wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4,
				1);
		}
	}
}

#if 0
void	draw_flame_element(SLONG x,SLONG y,SLONG z,SLONG c0,UBYTE base)
{
//	SLONG	c0;
	SLONG	trans;
	SLONG   page;
	SLONG   scale;
	UBYTE   u,v,w,h;
//	UBYTE*  palptr;
//	SLONG   palndx;
//	float   wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4;
	SLONG	dx,dy,dz;
	//
	// waft gently up from x,y,z
	//

	steam_seed=54321678+(c0*get_steam_rand());
	
	w=h=32;
	u=v=0;


	dy=get_steam_rand()&0x1ff;
	dy+=(GAME_TURN*5);
	dy%=500;
	dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
	dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;

	if (page==POLY_PAGE_FLAMES) 
	{
		dx>>=2; dz>>=2;
	}

	if(!(c0&3))
		page=POLY_PAGE_FLAMES;
	else
	  if (c0&4)
		page=POLY_PAGE_SMOKE;
	  else 
	  {
		page=POLY_PAGE_FLAMES2;
		dy=(GAME_TURN+c0)/2;
		u=(dy&3);
		v=((dy&12)>>2);
		w=h=8;
		dy=0;
	  }

	  trans=255-dy;

	  dx+=x;
	  dy+=y;
	  dz+=z;


	  if(trans>=1)
	  {
	  	switch (page) 
		{
	  	  case POLY_PAGE_FLAMES:
				trans+=(trans<<8)|(trans<<16);
				scale=50;
				break;
		  case POLY_PAGE_FLAMES2:
				trans=0x00FFFFFF;
				scale=100;
				dy=y;
				break;
		  case POLY_PAGE_SMOKE:
				trans+=(trans<<8)|(trans<<16);//|(trans<<24);
				scale=((dy-y)>>1)+50;
				dy+=50;
				break;
		}

		SPRITE_draw(
				dx,
				dy,
				dz,
				scale,
				trans,
				0xff000000,
				page+(u+(v<<3)),
//				u, v, w, h,
				1);
	  }
}
#endif

void TRACKS_DrawTrack(Thing *p_thing) 
{
	Track			*walk=p_thing->Genus.Track;
	SLONG			x,y,z,id,diff;
	PSX_POLY_Point  *pp=perm_pp_array;
	POLY_FT4		*p;
//	UBYTE			fade;
	UBYTE			u,v;
	SLONG			wpx,wpy,wpz;
	SWORD			sx,sz;


/*
	diff=track_head-TRACK_NUMBER(walk);
	if (diff<0) diff+=TRACK_BUFFER_LENGTH;
	SATURATE(diff,0,255); diff=255-diff;
	diff-=((walk->colour>>24)&0xff);
	SATURATE(diff,0,255); fade=diff;
*/
	// we should be using gamecoord... but... juuust in case. (this is a debug thing)
	/*
	wpx=p_thing->Genus.Track->x>>8;
	wpy=p_thing->Genus.Track->y>>8;
	wpz=p_thing->Genus.Track->z>>8;
	*/

	wpx=(p_thing->WorldPos.X>>8)-POLY_cam_x;
	wpy=(p_thing->WorldPos.Y>>8)-POLY_cam_y;
	wpz=(p_thing->WorldPos.Z>>8)-POLY_cam_z;

	sx=walk->sx;
	sz=walk->sz;

	if (walk->flags&TRACK_FLAGS_SPLUTTING) {
		walk->splut++;
		if (walk->splut==walk->splutmax) {
			walk->flags&=~TRACK_FLAGS_SPLUTTING;
		} else {
			sx*=walk->splut; sx/=walk->splutmax;
			sz*=walk->splut; sz/=walk->splutmax;
		}
	}
	pp[0].World.vx=wpx+sx;
	pp[0].World.vy=wpy;
	pp[0].World.vz=wpz+sz;

	pp[1].World.vx=wpx-sx;
	pp[1].World.vy=wpy;
	pp[1].World.vz=wpz-sz;

	pp[2].World.vx=wpx+walk->dx+sx;
	pp[2].World.vy=wpy+walk->dy;
	pp[2].World.vz=wpz+walk->dz+sz;

	pp[3].World.vx=wpx+walk->dx-sx;
	pp[3].World.vy=wpy+walk->dy;
	pp[3].World.vz=wpz+walk->dz-sz;

	gte_RotTransPers4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,
				   &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,
				   &pp[0].P,&pp[0].Flag,&z);

	if((pp[0].Flag&(1<<31))==0)
	{
		if(pp[0].Word.SX>-30 && pp[0].Word.SX<350)
		if(pp[0].Word.SY>-300 && pp[0].Word.SY<256+30)
		{

			ALLOCPRIM(p,POLY_FT4);
			u=getPSXU(walk->page);
			v=getPSXV(walk->page);

			setPolyFT4(p);
			if (walk->page==POLY_PAGE_BLOODSPLAT)
				p->tpage=getPSXTPageE(walk->page)&~(3<<5);				 
			else
				p->tpage=(getPSXTPageE(walk->page)&~(3<<5))|(2<<5);
			p->clut=getPSXClutE(walk->page);
			setSemiTrans(p,1);	 

			setXY4(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,
					 pp[2].Word.SX,pp[2].Word.SY,pp[3].Word.SX,pp[3].Word.SY);

			if (walk->flags&TRACK_FLAGS_FLIPABLE) {
				if (walk->flip) 
					setUV4(p,u+15,v+31,u,v+31,u+15,v+31,u,v);
				else
					setUV4(p,u,v+31,u+15,v+31,u,v,u+15,v);
			} else 
				setUV4(p,u+31,v+31,u,v+31,u+31,v,u,v);		

			setRGB0(p,(walk->colour&0xff)>>1,(walk->colour>>9)&0x7f,(walk->colour>>17)&0x7f);
			
			z=get_z_sort(z);
			DOPRIM(z,p);
		}
	}
}
/*
const UBYTE flare_table[7][3] =
{
	{ 132>>1,92>>1,80>>1 	},
	{ 135>>1,114>>1,79>>1	},
	{ 128>>1,134>>1,78>>1	},
	{ 81>>1,125>>1,73>>1	},
	{ 78>>1,132>>1,127>>1	},
	{ 76>>1,88>>1,126>>1	},
	{ 101>>1,73>>1,125>>1	}

};

void BLOOM_flare_draw(SLONG x, SLONG y, SLONG z, SLONG str) {
	PSX_POLY_Point  *pp=perm_pp_array;
	SLONG dx,dy,fx,fy,cx,cy;
	SLONG i,sz,j;
	POLY_FT4 *p;

//	if (!there_is_a_los(x,y,z,POLY_cam_x>>8,POLY_cam_y>>8,POLY_cam_z>>8)) return;

	pp->World.vx=x-POLY_cam_x;
	pp->World.vy=y-POLY_cam_y;
	pp->World.vz=z-POLY_cam_z;
	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

	if ((pp->Word.SX<0)||(pp->Word.SX>512)||(pp->Word.SY<0)||(pp->Word.SY>256)) return;

	cx=DisplayWidth>>1;
	cy=DisplayHeight>>1;
	dx=pp->Word.SX-cx;
	dy=pp->Word.SY-cy;

	dx<<=4; dy<<=4;

	p=(POLY_FT4*)the_display.CurrentPrim;
	for (i=-13;i<15;i+=2) 
	  {
		j=1+abs(i>>2);
		j*=i;
		sz=abs(abs(i)-3)*6;
		if (abs(i)>7) sz>>=1;
		if (abs(i)>11) sz>>=1;
		fx=cx+((j*dx)>>8)-sz;
		fy=cy+((j*dy)>>8)-sz;

		setPolyFT4(p);
		setXYWH(p,fx,fy,sz<<1,sz<<1);

		setUVWH(p,getPSXU(POLY_PAGE_LENSFLARE),getPSXV(POLY_PAGE_LENSFLARE),31,31);

		setRGB0(p,flare_table[(i>>2)+4][0],flare_table[(i>>2)+4][1],flare_table[(i>>2)+4][2]);
		setSemiTrans(p,1);
		p->tpage=getPSXTPage(POLY_PAGE_LENSFLARE)|(3<<5);
		p->clut=getPSXClut(POLY_PAGE_LENSFLARE);
		DOPRIM(FLARE_OTZ,p);
		p++;
	  }
    the_display.CurrentPrim=(UBYTE*)p;
}
*/


void BLOOM_draw(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG col, UBYTE opts) 
{
	SLONG a,b,c,dot;
	SLONG rgba,sz;

	PSX_POLY_Point  *pp=perm_pp_array;

	if(SOFTWARE)
		return;

	if ((!dx)&&(!dy)&&(!dz)) 
		dot=-255;
	else 
	{
		// first order of the day: calculate the dot product of the light and view normal
		a=(dx*AENG_cam_vec[0])/65536;
		b=(dy*AENG_cam_vec[1])/65536;
		c=(dz*AENG_cam_vec[2])/65536;
		dot = a+b+c;
	}

	sz=((col&0xff)+((col&0xff00)>>8)+((col&0xff0000)>>16))>>2;

	// draw the "glow bloom" if the light is pointing towards us
	if ((dot<0)||(opts&BLOOM_GLOW_ALWAYS)) 
	{
//	  rgba=abs(dot);
//	  rgba<<=24;
//	  rgba|=col;
	  SPRITE_draw_rotated(x,y,z,sz<<1,col,0xff000000,POLY_PAGE_BLOOM1,1,0xffffff,1);

/*	  // lil bit o lens flare
	  if (opts&BLOOM_LENSFLARE) 
	  {
	    rgba=abs(dot);
	    rgba>>=1;
		if (opts&BLOOM_FLENSFLARE) rgba>>=1;
	    BLOOM_flare_draw(x,y,z,rgba);
	  }
*/
	}

	if (opts&BLOOM_BEAM) 
	{
		// scale the flare
		dx*=sz; dy*=sz; dz*=sz;
		dx>>=5; dy>>=5; dz>>=5;

		// draw the "flare bloom" reaching out
		
		pp[0].World.vx=x-POLY_cam_x;
		pp[0].World.vy=y-POLY_cam_y;
		pp[0].World.vz=z-POLY_cam_z;
		pp[1].World.vx=x+dx-POLY_cam_x;
		pp[1].World.vy=y+dy-POLY_cam_y;
		pp[1].World.vz=z+dz-POLY_cam_z;

		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

		sz*=3;
		if(((pp[0].Flag|pp[1].Flag)&(1<<31))==0)
		if ((max(pp[0].Word.SX,pp[1].Word.SX)>0)&&(min(pp[0].Word.SX,pp[1].Word.SX)<DISPLAYWIDTH)&&
			(max(pp[0].Word.SY,pp[1].Word.SY)>0)&&(min(pp[0].Word.SY,pp[1].Word.SY)<256))
				POLY_add_line_tex(&pp[0], &pp[1], sz>>2, sz, POLY_PAGE_BLOOM2, col, 0);

	}

}

extern Particle		particles[PSYSTEM_MAX_PARTICLES];
extern SLONG		next_free, next_used;

void PARTICLE_Draw() 
{
	SLONG ctr,col;
	UBYTE ndx;
	Particle *p;
	UBYTE u,v,w,h;
	SLONG	size;

	for (p=&particles[next_used];p!=&particles[0];p=&particles[p->prev])
	{
//			if (!p->sprite) 
//			{
				u=0; v=0; w=31; h=31;
/*			} 
			else 
			{
				ndx=p->sprite>>2;
				switch (p->sprite&3) 
				{
				  case 1: // split in half each way
					  u=ndx&1; v=ndx>>1;
					  u<<=4; v<<=4;
					  w=15; h=15;
					  break;
				  case 2: // split in quarters each way
					  u=ndx&3; v=ndx>>2;
					  u<<=3; v<<=3;
					  w=7; h=7;
					  break;
				}
			}
		*/
//			col=(p->colour>>24);
//			col|=(col<<8)|(col<<16);

//			col+=col<<8;
//			col+=col<<16;
			size=(p->size);
			if(p->flags&PFLAG_RESIZE2)
				size>>=8;

			SPRITE_draw_tex(
				(p->x>>8),
				(p->y>>8),
				(p->z>>8),
				size,
				p->colour,
//				p->colour,
				(p->page==POLY_PAGE_SMOKE)?0x7f000000:0xff000000,
				p->page,
				u, v, w, h,
				1);
	}
}

void DRAWXTRA_Special(Thing *p_thing) {
	SLONG dx,dz,c0;
	
	switch(p_thing->Genus.Special->SpecialType) {

	case SPECIAL_MINE:
		if (p_thing->SubState == SPECIAL_SUBSTATE_ACTIVATED)
		{
		  c0=(p_thing->Genus.Special->counter>>1)&2047;
		  dx=SIN(c0)>>8;
		  dz=COS(c0)>>8;
		  BLOOM_draw(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+25,p_thing->WorldPos.Z>>8,
			dx,0,dz,0x7F0000,BLOOM_BEAM);
		}
		else 
		{
		  c0=3+(THING_NUMBER(p_thing)&7);
		  c0=(((GAME_TURN*c0)+(THING_NUMBER(p_thing)*9))<<4)&2047;
		  dx=SIN(c0)>>8;
		  dz=COS(c0)>>8;
		  BLOOM_draw(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+15,p_thing->WorldPos.Z>>8,
			dx,0,dz,0x7F0000,0);
		}
		break;
	case SPECIAL_EXPLOSIVES:
		if (p_thing->SubState == SPECIAL_SUBSTATE_ACTIVATED)
		{
		  c0=p_thing->Genus.Special->timer;
		  c0=(c0<<3)&2047;
		  dx=SIN(c0)>>8;
		  dz=COS(c0)>>8;
		  BLOOM_draw(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+25,p_thing->WorldPos.Z>>8,
			dx,0,dz,0x007F5D,BLOOM_BEAM);
		}
		break;


		//  no default -- not all specials have extra stuff.
	}
}

void PYRO_draw_dustwave(Pyro *pyro);
void PYRO_draw_armageddon(Pyro *pyro);

void PYRO_draw_pyro(Thing *p_pyro) {
	Pyro *pyro = PYRO_get_pyro(p_pyro);
	SLONG fx,fy,fz;
	SLONG i,j;
	GameCoord pos;
    PSX_POLY_Point *pp=perm_pp_array;
    SLONG	x,y,z;


//	float dir[8][2] = { { 0.0f, 1.0f}, { 0.7f, 0.7f}, { 1.0f, 0.0f}, { 0.7f,-0.7f}, { 0.0f,-1.0f}, {-0.7f,-0.7f}, {-1.0f, 0.0f}, {-0.7f, 0.7f} };
//	float uvs[8][2] = { { 0.5f, 1.0f}, { 0.85f, 0.85f}, { 1.0f, 0.5f}, { 0.85f, 0.15f}, { 0.5f, 0.0f}, { 0.15f, 0.15f}, { 0.0f, 0.5f}, { 0.15f, 0.85f} };

	fx=p_pyro->WorldPos.X;
	fy=p_pyro->WorldPos.Y;
	fz=p_pyro->WorldPos.Z;

	switch(pyro->PyroType) {
	case PYRO_EXPLODE:
//		PYRO_draw_explosion((Pyrex*)pyro);
		break;

	case PYRO_FIREWALL:
		break;
	case PYRO_FIREPOOL:
		break;
	case PYRO_BONFIRE:
		draw_flames(fx>>8,fy>>8,fz>>8,10,(SLONG)p_pyro);
		break;

	case PYRO_IMMOLATE:

		// Hey these two will always work.

		if (pyro->Flags&PYRO_FLAGS_SMOKE) 
		{
				if (pyro->victim)
					pos = pyro->victim->WorldPos;
				else
					pos = pyro->thing->WorldPos;

				if (pyro->Flags&PYRO_FLAGS_STATIC) 
				{
					pos.X>>=8; pos.Y>>=8; pos.Z>>=8;
					draw_flames(pos.X,pos.Y,pos.Z,4,(SLONG)p_pyro);
				} 
				else 
				{
					PARTICLE_Add(pos.X,pos.Y+8192,pos.Z,0,1024,0,POLY_PAGE_SMOKE,0,0x7FFFFFFF,
						PFLAGS_SMOKE,80,8,1,3,4);
				}

		}
		if (pyro->Flags&PYRO_FLAGS_FLAME) 
		{
				if (pyro->victim)
					pos = pyro->victim->WorldPos;
				else
					pos = pyro->thing->WorldPos;
				if (pyro->Flags&PYRO_FLAGS_STATIC) 
				{
					pos.X>>=8; pos.Y>>=8; pos.Z>>=8;
					draw_flames(pos.X,pos.Y,pos.Z,4,(SLONG)p_pyro);
				} 
				else 
				{
					PARTICLE_Add(pos.X,pos.Y+8192,pos.Z,0,1024,0,POLY_PAGE_FLAMES,0,0xffFFFFFF,
						PFLAG_FIRE|PFLAG_FADE|PFLAG_WANDER,80,60,1,4,-1);
				}

		}

		// This is the bit I'm dreading, if it's not a victimless crime.

		if (pyro->victim) 
		{
			SLONG scale=0,y_off=0;

			switch(pyro->victim->DrawType)
			{
			case	DT_ANIM_PRIM:
				if (pyro->victim->Class!=CLASS_BAT)
					break;
				scale=2;
				y_off=-96;
			case	DT_ROT_MULTI:
				if (pyro->Flags & PYRO_FLAGS_FLICKER)
				{
					SLONG px,py,pz,dx,dz;
					UBYTE i,r,p;
					
					dx = -(SIN(pyro->victim->Draw.Tweened->Angle) * pyro->victim->Velocity) >> 9;
					dz = -(COS(pyro->victim->Draw.Tweened->Angle) * pyro->victim->Velocity) >> 9;


					if (pyro->Dummy==2) r=3; else r=2;
					for (i=0;i<r;i++) 
					{
						switch(pyro->victim->State)
						{
						case STATE_DYING:	p=7;	break;
						case STATE_DEAD:	p=3;	break;
						default:			p=0xf;	break;
						}
						p=rand()&p;
						calc_sub_objects_position(
							pyro->victim,
							pyro->victim->Draw.Tweened->AnimTween,
							p,
							&px,
							&py,
							&pz);				
						px = (px<<scale) + (pyro->victim->WorldPos.X >> 8);
						py = ((py+y_off)<<scale) + (pyro->victim->WorldPos.Y >> 8);
						pz = (pz<<scale) + (pyro->victim->WorldPos.Z >> 8);
						if (i&1)
							PARTICLE_Add(px<<8,(py<<8)+8192,pz<<8,dx,768,dz,POLY_PAGE_SMOKE,0,0x7FFFFFFF,
											PFLAGS_SMOKE|PFLAG_DAMPING,80,8,1,3,4);
						else
							PARTICLE_Add(px<<8,(py<<8)+8192,pz<<8,dx,512,dz,POLY_PAGE_FLAMES,0,0xffFFFFFF,
											PFLAG_FIRE|PFLAG_FADE|PFLAG_WANDER|PFLAG_DAMPING,80,60,1,4,-1);
					}					
				}
				break;

			default:
				pos = pyro->victim->WorldPos;
				pos.X>>=8; pos.Y>>=8; pos.Z>>=8;

				draw_flames(pos.X,pos.Y,pos.Z,4,(SLONG)p_pyro);
				break;
			}
		}
		break;

	case PYRO_DUSTWAVE:
		PYRO_draw_dustwave(pyro);
		break;

	case PYRO_EXPLODE2:
//		PYRO_draw_explosion2(pyro);
		break;

	case PYRO_STREAMER:
//		PYRO_draw_streamer(pyro);
		break;

	case PYRO_TWANGER:
//		PYRO_draw_twanger(pyro);
/*		{
		  SLONG str;
		  if (pyro->counter<30) {
		    str=pyro->counter*5;
		  } else {
			str=(285-pyro->counter*2);
			str>>=1;
		  }
		  if (str>0) BLOOM_flare_draw(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,str);
		}*/
		break;

	case PYRO_NEWDOME:
//		PYRO_draw_newdome(pyro);
		break;

//	case PYRO_SMOKEGEN:
		// this only creates other pyros so no drawing
//		break;

//	case PYRO_ONESMOKE:
		// temp behaviour: draw a sprite
//		BLOOM_flare_draw(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,0x75);
//		{
		//	SLONG col=(255-pyro->counter)<<24;
		//col|=0x007f7f7f;
		//SPRITE_draw_tex(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,50,col,0xff000000,POLY_PAGE_FLAMES, 0.0,0.0,1.0,1.0, SPRITE_SORT_NORMAL);
//		PYRO_draw_blob(pyro);
//		}
//		break;
	case PYRO_FIREBOMB:
#ifdef GONNA_FIREBOMB_YOUR_ASS
		if (!(pyro->thing->Flags&FLAGS_IN_VIEW)) break;
		// temp behaviour: draw a sprite
//		BLOOM_flare_draw(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,0x75);
		{
		//	SLONG col=(255-pyro->counter)<<24;
		//col|=0x007f7f7f;
		//SPRITE_draw_tex(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,50,col,0xff000000,POLY_PAGE_FLAMES, 0.0,0.0,1.0,1.0, SPRITE_SORT_NORMAL);
		//PYRO_draw_blob(pyro);
		
			SLONG x,y,z,d,h,i;

			if (pyro->counter<10) 
			{
				for (d=0;d<4;d++) {
					if ((pyro->Flags&PYRO_FLAGS_WAVE)&&(pyro->counter<6)) {
					x=SIN((d<<7)+(Random()&127))>>3; z=COS((d<<7)+(Random()&127))>>3;
					PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
						x,(Random()&0xff),z,
						POLY_PAGE_EXPLODE1,2+((Random()&3)<<2),0x7FFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING,
						55+(Random()&0x3f),80,1,8-(Random()&3),4);
					}
					x=SIN(d<<7)>>4; z=COS(d<<7)>>4;
					i=Random()&0xff;
					if (pyro->counter>3) {
						i-=pyro->counter*15;
						if (i<0) i=0;
						h=i;
						i=SIN(h<<1)>>7;
						y=COS(h<<1)>>4;
					} else {
						y=(128+(Random()&0xff))<<4;
					}
					x*=i; z*=i;
					x>>=8; z>>=8;
					h=(127+(Random()&0x7f))<<24;
					h|=0xFFFFFF;
					
					PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
						x,y,z,
						POLY_PAGE_EXPLODE1,2+((Random()&3)<<2),h,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING|PFLAG_GRAVITY,
						70+(Random()&0x3f),160,1,6,-4);
				}
				d=Random()&2047;
				x=SIN(d)>>4; z=COS(d)>>4;
				d=Random()&0xff;
				x*=d; z*=d;
				x>>=8; z>>=8;
				PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
					x,(128+(Random()&0xff))<<4,z,
					POLY_PAGE_EXPLODE1,2+((Random()&3)<<2),0x7FFFFFFF,
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING|PFLAG_GRAVITY,
					75+(Random()&0x3f),160,1,5+(Random()&3),-(2+(Random()&3)));
			}
			if (pyro->counter<240) {
				if ((pyro->counter>110)&&(pyro->counter<140)) {
					PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
						0,0,0,
						POLY_PAGE_EXPLODE1,2+((Random()&3)<<2),0xffFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE,
						100,255,1,20,5);
				}
				if (pyro->counter>10) pyro->counter+=10;
				if ((pyro->counter>4)&&(pyro->counter<110)) {
				d=Random()&2047;
				x=SIN(d)>>4; z=COS(d)>>4;
				//h=Random()&0xff;
				h=(Random()&0x7f);
				i=SIN(h<<1)>>8;
				y=COS(h<<1)>>4;
				x*=i; z*=i;
				x>>=8; z>>=8;
				//h=(127+(Random()&0x7f))<<24;
				//h|=0x7003f;
				h=0x7fffffff;
				PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
					x,y,z,
//						POLY_PAGE_FLAMES2,2+((Random()&3)<<2),h,
					POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),h,
					PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE|PFLAG_DAMPING,
					70+(Random()&0x3f),100,1,2,4+(Random()&3));
				}
			}
		
		}
#endif
		break;

	case PYRO_HITSPANG:
		// in case you were wondering, a spang is a sort of ricochet/blast effect
		// that appears when bullets hit you (or, i guess, walls, cars, etc)
//		  POLY_transform(x+pyro->target.X,y+pyro->target.Y,z+pyro->target.Z,&pt1);

//		  if(pyro->counter)
		  {
			  x=(pyro->thing->WorldPos.X>>8);
			  y=(pyro->thing->WorldPos.Y>>8);
			  z=(pyro->thing->WorldPos.Z>>8);
			  if (pyro->counter)
			  {
				  pp[0].World.vx=x+(pyro->target.X)-POLY_cam_x;
				  pp[0].World.vy=y+(pyro->target.Y)-POLY_cam_y;
				  pp[0].World.vz=z+(pyro->target.Z)-POLY_cam_z;
			  }
			  else
			  {
 				  pp[0].World.vx=x+(pyro->target.X<<1)-POLY_cam_x;
				  pp[0].World.vy=y+(pyro->target.Y<<1)-POLY_cam_y;
				  pp[0].World.vz=z+(pyro->target.Z<<1)-POLY_cam_z;
			  }
	//		  pp[0].World.vx=(pyro->thing->WorldPos.X>>8)+256-POLY_cam_x;
	//		  pp[0].World.vy=(pyro->thing->WorldPos.Y>>8)+0-POLY_cam_y;
	//		  pp[0].World.vz=(pyro->thing->WorldPos.Z>>8)+0-POLY_cam_z;

			  pp[1].World.vx=x-POLY_cam_x;
			  pp[1].World.vy=y-POLY_cam_y;
			  pp[1].World.vz=z-POLY_cam_z;

			  gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
			  gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

			  {
				  SLONG	index=pyro->counter;
				  pp[0].World.vx=(index&2)*8;
				  pp[1].World.vx=((index&2)*8)+15;
				  pp[0].World.vy=(index&1)*16;
				  pp[1].World.vy=((index&1)*16)+15;
			  }

			  if(!((pp[0].Flag|pp[1].Flag)&(1<<31)))
			  {
		  		if (pyro->counter)
		  			POLY_add_line_tex_uv(&pp[0],&pp[1],42,42,POLY_PAGE_HITSPANG,0xffffffff,0);
		  		else
		  			POLY_add_line_tex_uv(&pp[0],&pp[1],22,22,POLY_PAGE_HITSPANG,0xffffffff,0);
			  }
		  }
		break;

	case PYRO_GAMEOVER:
		PYRO_draw_armageddon(pyro);
		break;

	}

}

//
// Map drawing routines
//
// Draw the city map file.
//

#define SHAPE_MAX_SPARKY_POINTS 8

void SHAPE_sparky_line(
		SLONG num_points,
		SLONG px[],
		SLONG py[],
		SLONG pz[],
		ULONG colour,
		SLONG width)
{
	ASSERT(WITHIN(num_points, 2, SHAPE_MAX_SPARKY_POINTS));

	SLONG i;

	PSX_POLY_Point *pp=perm_pp_array;
	POLY_FT4 *p;
	UBYTE u,v;

/*
	SLONG p1;
	SLONG p2;

	SLONG dx;
	SLONG dy;
	SLONG len;
	SLONG overlen;
	SLONG size;

	SLONG pnx;
	SLONG pny;
	SLONG n1_valid;
	SLONG n2_valid;

	POLY_Point pp1;
	POLY_Point pp2;	

	SLONG nx[SHAPE_MAX_SPARKY_POINTS];
	SLONG ny[SHAPE_MAX_SPARKY_POINTS];

	PSX_POLY_Point *pp=perm_pp_array;

	//
	// Transform all the points along the middle of the line.
	//
*/
	for(i=0;i<num_points-1; i++)
	{
		pp[0].World.vx=px[i]-POLY_cam_x;
		pp[0].World.vy=py[i]+(width>>1)-POLY_cam_y;
		pp[0].World.vz=pz[i]-POLY_cam_z;

		pp[1].World.vx=px[i+1]-POLY_cam_x;
		pp[1].World.vy=py[i+1]+(width>>1)-POLY_cam_y;
		pp[1].World.vz=pz[i+1]-POLY_cam_z;

		pp[2].World.vx=px[i]-POLY_cam_x;
		pp[2].World.vy=py[i]-(width>>1)-POLY_cam_y;
		pp[2].World.vz=pz[i]-POLY_cam_z;

		pp[3].World.vx=px[i+1]-POLY_cam_x;
		pp[3].World.vy=py[i+1]-(width>>1)-POLY_cam_y;
		pp[3].World.vz=pz[i+1]-POLY_cam_z;

		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		gte_RotTransPers3(&pp[1].World,&pp[2].World,&pp[3].World,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,&pp[1].P,&pp[1].Flag,&pp[0].Z);

		ALLOCPRIM(p,POLY_FT4);
		setPolyFT4(p);

		u=getPSXU(POLY_PAGE_BOLT);
		v=getPSXV(POLY_PAGE_BOLT)+(Random()&24);	// Get a random bolt of lightning.

		setXY4(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,
				 pp[2].Word.SX,pp[2].Word.SY,pp[3].Word.SX,pp[3].Word.SY);
		setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);
		setSemiTrans(p,1);
		setUVWH(p,u,v,31,8);
		p->tpage=getPSXTPageE(POLY_PAGE_BOLT);
		p->clut=getPSXClutE(POLY_PAGE_BOLT);
		DOPRIM(get_z_sort(pp[1].Z>>1),p);
	}
}


#define DUSTWAVE_SECTORS	8
#define DUSTWAVE_MULTIPLY	256

void PYRO_draw_dustwave(Pyro *pyro) 
{
	PSX_POLY_Point  *pp=perm_pp_array;
	SLONG cx,cy,cz,fade,z;
	UBYTE sections, pass, sector, next;
	SLONG dxs[DUSTWAVE_SECTORS],dys[DUSTWAVE_SECTORS], dists[4], heights[4];
	POLY_FT4 *p;
//	SLONG thisscale,nextscale;

	UBYTE u,v;

	u=getPSXU(POLY_PAGE_DUSTWAVE);
	v=getPSXV(POLY_PAGE_DUSTWAVE);

	// we'll need these to add on to relative coords
	cx=(pyro->thing->WorldPos.X>>8)-POLY_cam_x;
	cy=(pyro->thing->WorldPos.Y>>8)-POLY_cam_y;
	cz=(pyro->thing->WorldPos.Z>>8)-POLY_cam_z;
	// and we'll often need to join stuff back up to the centre
//	POLY_transform( cx, cy, cz, &mid);
//	mid.u=mid.v=0;
//	mid.u=0.5;
//	mid.v=1.0;

	sections=3;

	for(sector=0;sector<DUSTWAVE_SECTORS;sector++) 
	{
		dxs[sector]=SIN(sector*DUSTWAVE_MULTIPLY)>>8;
		dys[sector]=COS(sector*DUSTWAVE_MULTIPLY)>>8;
	}

	// precalc the ring data
	for(pass=0;pass<4;pass++) 
	{
		switch(pass) 
		{
		case 0:
			if (pyro->counter>1)
				dists[0]=512+(SIN(pyro->counter*4)>>8);
			else
				dists[0]=256+(SIN(pyro->counter*4)>>8);

			heights[0]=2;
			break;
		case 1:
			dists[1]=(dists[0]*SIN(pyro->counter*4))>>16;
			heights[1]=SIN(pyro->counter*4)>>10;
			break;
		case 2:
			dists[2]=(dists[1]*SIN(pyro->counter*4))>>16;
			heights[2]=(heights[1]|(heights[1]<<1))>>4;
			break;
		case 3:
			dists[3]=(dists[2]*SIN(pyro->counter*4))>16;
			heights[3]=2;
			break;
		}
	}

/*	if (pyro->counter<192)
		fade=0;
	else
		fade=((pyro->counter-192)*4)<<24;
		*/
	fade=128-pyro->counter;

	// draw the data
	for(pass=0;pass<sections;pass++) 
	{
		for(sector=0;sector<DUSTWAVE_SECTORS;sector++) 
		{
			next=sector+1;
			if (next==DUSTWAVE_SECTORS) next=0;

			pp[0].World.vx=cx+((dists[pass]*dxs[sector])>>8);
			pp[0].World.vy=cy+heights[pass];
			pp[0].World.vz=cz+((dists[pass]*dys[sector])>>8);

			pp[1].World.vx=cx+((dists[pass]*dxs[next])>>8);
			pp[1].World.vy=cy+heights[pass];
			pp[1].World.vz=cz+((dists[pass]*dys[next])>>8);

			pp[2].World.vx=cx+((dists[pass+1]*dxs[sector])>>8);
			pp[2].World.vy=cy+heights[pass+1];
			pp[2].World.vz=cz+((dists[pass+1]*dys[sector])>>8);

			pp[3].World.vx=cx+((dists[pass+1]*dxs[next])>>8);
			pp[3].World.vy=cy+heights[pass+1];
			pp[3].World.vz=cz+((dists[pass+1]*dys[next])>>8);

			gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
			gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
			gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
			gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

			z=get_z_sort(pp[0].Z>>1);

			ALLOCPRIM(p,POLY_FT4);
			setPolyFT4(p);
			setXY4(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,
			   	     pp[2].Word.SX,pp[2].Word.SY,pp[3].Word.SX,pp[3].Word.SY);
			setRGB0(p,fade,fade,fade);
			setUV4(p,u,v+31-(10*pass),u+31,v+31-(10*pass),u,v+21-(10*pass),u+31,v+21-(10*pass));
			setSemiTrans(p,1);
			p->tpage=getPSXTPageE(POLY_PAGE_DUSTWAVE);
			p->clut=getPSXClutE(POLY_PAGE_DUSTWAVE);
			DOPRIM(z,p);
		}
	}

	// add some shock:
//	for(sector=0;sector<DUSTWAVE_SECTORS;sector++)
	{
		static UBYTE shock_sector=0;
		DIRT_gust(pyro->thing,
			cx+POLY_cam_x+((dists[2]*dxs[shock_sector])>>8),
			cz+POLY_cam_z+((dists[2]*dys[shock_sector])>>8),
			cx+POLY_cam_x+((dists[1]*dxs[shock_sector])>>8),
			cz+POLY_cam_z+((dists[1]*dys[shock_sector])>>8)
			);
		shock_sector++;
		if (shock_sector==DUSTWAVE_SECTORS) shock_sector=0;
	}
	

}

/*************************************************************
 *
 *   MIBS
 *   they blow up properly now.
 *
 */


void DRAWXTRA_MIB_destruct(Thing *p_thing)
{
	UBYTE i;
	SLONG ctr=p_thing->Genus.Person->Timer1;
	GameCoord posn;
	Thing *thing;
	SLONG j;

	p_thing->WorldPos.Y+=SIN(ctr>>2)>>7;

	calc_sub_objects_position(
		p_thing,
		p_thing->Draw.Tweened->AnimTween,
		SUB_OBJECT_PELVIS,
	   &posn.X,
	   &posn.Y,
	   &posn.Z);

	posn.X<<=8; posn.Y<<=8; posn.Z<<=8;
	posn.X+=p_thing->WorldPos.X;
	posn.Y+=p_thing->WorldPos.Y;
	posn.Z+=p_thing->WorldPos.Z;
/*
	if (ctr>32 * 20 * 5)
	{
		POLY_Point pt1,pt2;

	    POLY_transform(posn.X>>8,(posn.Y>>8)+1000,posn.Z>>8,&pt1);
	    POLY_transform(posn.X>>8,PAP_calc_map_height_at(posn.X>>8,posn.Z>>8),posn.Z>>8,&pt2);

		pt1.colour=pt2.colour=0xFFFFFFFF;
		pt1.specular=pt2.specular=0xFF000000;
		pt1.u=0; pt1.v=0;
		pt2.u=1.0; pt2.v=0.25;
	    if (POLY_valid_line(&pt1,&pt2)) 
		  POLY_add_line_tex_uv(&pt1,&pt2,142,142,POLY_PAGE_LITE_BOLT,0);
	}
*/
	if (ctr>1200+p_thing->Genus.Person->ammo_packs_pistol)
	{

		//
		// A dynamic light lightning-bolt flash that only lasts one frame.
		//

		UBYTE dlight;

		dlight = NIGHT_dlight_create(
					(posn.X >> 8),
					(posn.Y >> 8) + 0x80,
					(posn.Z >> 8),
					90+(Random()&0x1f),
					5,
					25,
					30);
		
		if (dlight)
		{
			NIGHT_dlight[dlight].flag |= NIGHT_DLIGHT_FLAG_REMOVE;
		}


		p_thing->Genus.Person->ammo_packs_pistol = (3200-ctr)>>3;
		thing=PYRO_create(posn,PYRO_TWANGER);
		if (thing) {
			thing->StateFn(thing);
			if (Random()&0xf)
			{
				thing->Genus.Pyro->tints[0]=0x0000FFFF;
				thing->Genus.Pyro->tints[1]=0x000000FF;
			}
			else
			{
				thing->Genus.Pyro->tints[0]=0x00FFFFFF;
				thing->Genus.Pyro->tints[1]=0x0000FFFF;
			}
			j=ctr-1199;
			if (j>400) j=400;
			thing->Genus.Pyro->scale=j;
		}
	} else p_thing->Genus.Person->ammo_packs_pistol=0;

	if (GAME_TURN&1)
	{

		SPARK_Pinfo p1;
		SPARK_Pinfo p2;

		UBYTE limbs[] = { SUB_OBJECT_LEFT_HAND, SUB_OBJECT_RIGHT_HAND, SUB_OBJECT_LEFT_FOOT, SUB_OBJECT_RIGHT_FOOT };
		
		p1.type   = SPARK_TYPE_GROUND;
		p1.flag   = 0;
		p1.person = THING_NUMBER(p_thing);
//		p1.limb   = limbs[Random()&3];
		p1.dist	  = SPARK_TYPE_GROUND;
		p1.x=posn.X>>8; p1.y=posn.Y>>8; p1.z=posn.Z>>8;
		if (ctr<400)
		{
			p1.x+=(Random()&0xff)-0x7f;
			p1.z+=(Random()&0xff)-0x7f;
		} else
			if (ctr<800)
			{
				p1.x+=(Random()&0x1ff)-0xff;
				p1.z+=(Random()&0x1ff)-0xff;
			}
			else
			{
				p1.x+=(Random()&0x3ff)-0x1ff;
				p1.z+=(Random()&0x3ff)-0x1ff;
			}

		p2.type   = SPARK_TYPE_LIMB;
		p2.flag   = 0;
		p2.person = THING_NUMBER(p_thing);
		p2.limb   = SUB_OBJECT_PELVIS;

		SPARK_create(
			&p1,
			&p2,
			25);
	}

}

/*************************************************************
 *
 *   final_glow is the fx Mark did for the final level,
 *   but didn't bother putting a banner in for them, so I'm
 *   doing it now....
 *
 *	 I put a comment into the header file instead.
 */


void DRAWXTRA_final_glow(SLONG x, SLONG y, SLONG z, UBYTE fade)
{
	static SLONG rotation;

	POLY_Point mid;

	rotation += 10 * TICK_RATIO >> TICK_SHIFT;

	SPRITE_draw_rotated(x,y,z,512,0x7f7f7f,0xff000000,POLY_PAGE_FINALGLOW,1,rotation,1);
}

void PYRO_draw_armageddon(Pyro *pyro)
{
	Thing *thing;
	GameCoord pos;
	SWORD i,j;

	move_thing_on_map(pyro->thing,&NET_PERSON(0)->WorldPos);

    // let's start things going nuts
//	for (i=0;i<5;i++)
/*	{
		pos=pyro->thing->WorldPos;
		pos.X+=((Random()&0xff00)-0x7f00)<<2;
		pos.Z+=((Random()&0xff00)-0x7f00)<<2;
		thing = PYRO_create(pos,PYRO_TWANGER);
		if (thing) {
			thing->StateFn(thing);
			if (Random()&0xf)
			{
				thing->Genus.Pyro->tints[0]=0x00FFFF00;
				thing->Genus.Pyro->tints[1]=0x00FF0000;
			}
			else
			{
				thing->Genus.Pyro->tints[0]=0x00FFFFFF;
				thing->Genus.Pyro->tints[1]=0x00FFFF00;
			}
			j=pyro->counter;
			j*=3;
			thing->Genus.Pyro->scale=j;
		}
	}*/

	if (!(Random()&2))
	{
		SLONG pow_type;
		pos=pyro->thing->WorldPos;
		pos.X+=((Random()&0xff00)-0x7f00)<<3;
		pos.Z+=((Random()&0xff00)-0x7f00)<<3;
		pos.Y=PAP_calc_map_height_at(pos.X>>8,pos.Z>>8)<<8;
		pow_type=POW_TYPE_BASIC_SPHERE_SMALL+(Random()&3)-(pyro->counter>>6);
		if (pow_type==POW_TYPE_UNUSED) pow_type=POW_TYPE_SPAWN_SPHERE_LARGE;
		POW_new(pow_type,pos.X,pos.Y,pos.Z,0,0,0);
//		thing=PYRO_create(pos,PYRO_NEWDOME);
//		thing->Genus.Pyro->scale=(400+Random()&0x7f)+(pyro->counter<<1);
	}

	if (!(Random()&3))
	{
		SLONG flags = PFLAG_EXPLODE_ON_IMPACT | PFLAG_LEAVE_TRAIL;
		if (Random()&1) flags|=PFLAG_GRAVITY ;
		pos=pyro->thing->WorldPos;
		pos.X+=((Random()&0xff00)-0x7f00)<<3;
		pos.Z+=((Random()&0xff00)-0x7f00)<<3;
		pos.Y=PAP_calc_map_height_at(pos.X>>8,pos.Z>>8)<<8;

		PARTICLE_Add(
			pos.X,
			pos.Y+0x1000,
			pos.Z,
			0,
			(0xff+(Random()&0xff)<<4),
			0,
			POLY_PAGE_METEOR,
			2 + ((Random() & 0x3) << 2),
			0xffffffff,
			flags,
			100,
			160,
			1,
			1,
			1);

		MFX_play_xyz(THING_NUMBER(pyro->thing),S_BALROG_FIREBALL,MFX_OVERLAP,pos.X,pos.Y,pos.Z);
	}

}
