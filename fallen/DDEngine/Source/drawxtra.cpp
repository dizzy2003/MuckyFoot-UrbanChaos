//
// DrawXtra.cpp
// Matthew Rosenfeld	12 October 1998
//
// In order to make fallen more PSX-friendly, the various Really Gnasty Hacks scattered
// throughout the game code to draw "special" objects are being collected together in
// drawxtra.cpp -- stuff like the van, helicopter, footprints, etc.
//

#include "DDLib.h"
#include "drawxtra.h"
#include "FMatrix.h"
#include "animate.h"
#include "mesh.h"
#include "cone.h"
#include "poly.h"
#include "psystem.h"
#include "sprite.h"
#include "spark.h"
#include "dirt.h"
#include "statedef.h"
#include "memory.h"
#include "collide.h"
#include	"tracks.h"
#include	"c:\fallen\headers\fc.h"
#include	"barrel.h"
#include	"eway.h"
#include "mfx.h"
#include "sound.h"
#include "gamemenu.h"



/*************************************************************
 *
 *   Helicopter
 *
 */

void CHOPPER_draw_chopper(Thing *p_chopper) 
{

	Chopper *chopper = CHOPPER_get_chopper(p_chopper);
	SLONG matrix[9], vector[3];


	vector[0]=(chopper->dx/64)&2047; vector[1]=(-chopper->dz/64)&2047; vector[2]=0;
	p_chopper->Draw.Mesh->Roll=vector[0];
	p_chopper->Draw.Mesh->Tilt=vector[1];


    if (!(chopper->target)) {

		FMATRIX_calc(matrix, p_chopper->Draw.Mesh->Angle, p_chopper->Draw.Mesh->Tilt, p_chopper->Draw.Mesh->Roll);

		vector[0]=vector[2]=0; vector[1]=-1;

		FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);

		chopper->spotx=p_chopper->WorldPos.X;
		chopper->spotz=p_chopper->WorldPos.Z;

		chopper->spotdx=chopper->spotdz=0;

	} else {

		SLONG target_x;
		SLONG target_y;
		SLONG target_z;

		//
		// Look at Darci's body.
		//

		calc_sub_objects_position(
			chopper->target,
			chopper->target->Draw.Tweened->AnimTween,
			SUB_OBJECT_PELVIS,
		   &target_x,
		   &target_y,
		   &target_z);

		target_x <<= 8;
		target_y <<= 8;
		target_z <<= 8;

		target_x += chopper->target->WorldPos.X;
		target_y += chopper->target->WorldPos.Y;
		target_z += chopper->target->WorldPos.Z;

		SLONG dx, dz, dist;
		SLONG maxspd = chopper->speed<<6;

		if (chopper->spotx > target_x) chopper->spotdx-=(chopper->since_takeoff >> 1);
		if (chopper->spotz > target_z) chopper->spotdz-=(chopper->since_takeoff >> 1);

		if (chopper->spotx < target_x) chopper->spotdx+=(chopper->since_takeoff >> 1);
		if (chopper->spotz < target_z) chopper->spotdz+=(chopper->since_takeoff >> 1);

		if (chopper->spotdx> maxspd) chopper->spotdx= maxspd;
		if (chopper->spotdx<-maxspd) chopper->spotdx=-maxspd;
		if (chopper->spotdz> maxspd) chopper->spotdz= maxspd;
		if (chopper->spotdz<-maxspd) chopper->spotdz=-maxspd;


		chopper->spotx += chopper->spotdx + chopper->dx;
		chopper->spotz += chopper->spotdz + chopper->dz;

		chopper->spotx = ((chopper->spotx*24.0f)+target_x)*0.04f;	// Was 24 and 0.04
		chopper->spotz = ((chopper->spotz*24.0f)+target_z)*0.04f;

		vector[0]= chopper->spotx - p_chopper->WorldPos.X;

		// unlikely event you're higher than the chopper, it won't aim up at you
		if (target_y>p_chopper->WorldPos.Y)
			vector[1]=0;
		else
		  vector[1]=target_y - p_chopper->WorldPos.Y;

		vector[2]= chopper->spotz - p_chopper->WorldPos.Z;

	}

	MESH_draw_poly_inv_matrix(
		p_chopper->Draw.Mesh->ObjectId,
		p_chopper->WorldPos.X >> 8 ,
		p_chopper->WorldPos.Y >> 8,
		p_chopper->WorldPos.Z >> 8 ,
		-p_chopper->Draw.Mesh->Angle,
		-p_chopper->Draw.Mesh->Tilt,
		-p_chopper->Draw.Mesh->Roll,
		NULL);
	MESH_draw_poly_inv_matrix(
		chopper->rotorprim,
		p_chopper->WorldPos.X >> 8 ,
		p_chopper->WorldPos.Y >> 8,
		p_chopper->WorldPos.Z >> 8 ,
		-(p_chopper->Draw.Mesh->Angle+chopper->rotors),
		-(p_chopper->Draw.Mesh->Tilt),
		-(p_chopper->Draw.Mesh->Roll),
		NULL);


		if (chopper->light) {
			SLONG colour;

			colour=(0x66 * chopper->light)/255;
			colour+=(colour<<8);
			colour<<=8;

			CONE_create(
				p_chopper->WorldPos.X>>8,
				p_chopper->WorldPos.Y>>8,
				p_chopper->WorldPos.Z>>8,
				vector[0],
				vector[1],
				vector[2],
				256.0F * 5.0F,
				256.0F,
				colour,
				0x00000000,
				50);

			CONE_intersect_with_map();

			CONE_draw();

		}

}


/*************************************************************
 *
 *   Tracks (footprints + tyre tracks)
 *
 */



void TRACKS_DrawTrack(Thing *p_thing) 
{
	Track *walk=p_thing->Genus.Track;
	SLONG x,y,z,id,diff;
	POLY_Point  pp[4];
	POLY_Point *quad[4];
	UBYTE fade;
	SLONG wpx,wpy,wpz;
	SWORD sx,sz;

	POLY_flush_local_rot();

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	diff=track_head-TRACK_NUMBER(walk);
	if (diff<0) diff+=TRACK_BUFFER_LENGTH;
	SATURATE(diff,0,255); diff=255-diff;
	diff-=((walk->colour>>24)&0xff);
	SATURATE(diff,0,255); fade=diff;
	if (walk->flags&TRACK_FLAGS_INVALPHA) fade=255-fade;

	// we should be using gamecoord... but... juuust in case. (this is a debug thing)
	/*
	wpx=p_thing->Genus.Track->x>>8;
	wpy=p_thing->Genus.Track->y>>8;
	wpz=p_thing->Genus.Track->z>>8;
	*/
	wpx=p_thing->WorldPos.X>>8;
	wpy=p_thing->WorldPos.Y>>8;
	wpz=p_thing->WorldPos.Z>>8;

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
	x=wpx + sx;
	z=wpz + sz;
	y=wpy;
	POLY_transform(x,y,z,&pp[0]);
	x=wpx - sx;
	z=wpz - sz;
	y=wpy;
	POLY_transform(x,y,z,&pp[1]);
	x=wpx + walk->dx + sx;
	z=wpz + walk->dz + sz;
	y=wpy + walk->dy;
	POLY_transform(x,y,z,&pp[2]);
	x=wpx + walk->dx - sx;
	z=wpz + walk->dz - sz;
	y=wpy + walk->dy;
	POLY_transform(x,y,z,&pp[3]);


	if (walk->flags&TRACK_FLAGS_FLIPABLE) {
		if (walk->flip) {
			pp[0].u=0.5; pp[0].v=1;
			pp[1].u=0.0; pp[1].v=1;
			pp[2].u=0.5; pp[2].v=0;
			pp[3].u=0.0; pp[3].v=0;
		} else {
			pp[0].u=0.0; pp[0].v=1;
			pp[1].u=0.5; pp[1].v=1;
			pp[2].u=0.0; pp[2].v=0;
			pp[3].u=0.5; pp[3].v=0;
		}
	} else {
		pp[0].u=1.0;  pp[0].v=1.0;
		pp[1].u=0.0;  pp[1].v=1.0;
		pp[2].u=1.0;  pp[2].v=0.0;
		pp[3].u=0.0;  pp[3].v=0.0;
	}
	pp[0].specular=pp[1].specular=pp[2].specular=pp[3].specular=0xFF000000;
	pp[0].colour=pp[1].colour=pp[2].colour=pp[3].colour=(fade<<24)+(walk->colour&0x00ffffff);

	if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid() && pp[3].MaybeValid())
	{
		POLY_add_quad(quad,walk->page,FALSE);
	}
}


/*************************************************************
 *
 *   Particle System
 *
 */

extern Particle		particles[PSYSTEM_MAX_PARTICLES];
extern SLONG		next_free, next_used;


void PARTICLE_Draw() 
{
	SLONG ctr;
	UBYTE ndx;
	Particle *p;
	float u,v,w,h,sz;

//	p=particles;
//	for (ctr=0;ctr<PSYSTEM_MAX_PARTICLES;ctr++,p++)
	for (p=particles+next_used;p!=particles;p=particles+p->prev)
//		if (p->priority) 
		{
			if (!p->sprite) {
				u=0; v=0; w=1; h=1;
			} else {
				ndx=p->sprite>>2;
				switch (p->sprite&3) {
				  case 1: // split in half each way
					  if (p->flags&PFLAG_SPRITELOOP) ndx&=3;
					  u=ndx&1; v=ndx>>1;
					  u*=0.5f; v*=0.5f;
					  w=0.5f; h=0.5f;
					  break;
				  case 2: // split in quarters each way
					  if (p->flags&PFLAG_SPRITELOOP) ndx&=0xf;
					  u=ndx&3; v=ndx>>2;
					  u*=0.25f; v*=0.25f;
					  w=0.25f; h=0.25f;
					  break;
				}
			}
			sz=float(p->size);
			if (p->flags&PFLAG_RESIZE2) sz*=0.00390625f;
			SPRITE_draw_tex(
				float(p->x>>8),
				float(p->y>>8),
				float(p->z>>8),
				sz,
				p->colour,
				0xff000000,
				p->page,
				u, v, w, h,
				SPRITE_SORT_NORMAL);		
		}
}


/*************************************************************
 *
 *   Pyros (bonfire like things)
 *
 */


extern UBYTE fire_pal[768];

SLONG	pyro_seed;

void	draw_flames(SLONG x,SLONG y,SLONG z,SLONG lod,SLONG offset);
void	draw_flame_element(SLONG x,SLONG y,SLONG z,SLONG c0,UBYTE base,UBYTE rand=1);

void    POLY_add_line_tex_uv(POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front);



// Pyro detail levels.
#ifdef TARGET_DC

// Lower detail:
#define DUSTWAVE_SECTORS	16
#define FIREBOMB_SPRITES	16

// Define this to dynamically limit the number of pyro sprites.
// This means that you get this many sprites, no
// matter how many explosions are on screen - very helpful
// when four mines go off together.
#define LIMIT_TOTAL_PYRO_SPRITES_PLEASE_BOB 500

#else

// Hi detail:
#define DUSTWAVE_SECTORS	16
#define FIREBOMB_SPRITES	16

// Turned off for the PC. You madmen :-)
//#define LIMIT_TOTAL_PYRO_SPRITES_PLEASE_BOB 100

#endif


#define	DUSTWAVE_MULTIPLY	( 2048 / DUSTWAVE_MULTIPLY )





#ifdef LIMIT_TOTAL_PYRO_SPRITES_PLEASE_BOB

static int iWantedToGivenMultiplier = 256;
static int iNumWantedPyrosThisFrame = 0;
static int iNumFixedPyrosThisFrame = 0;
// Called with how many sprites are wanted.
// Returns the number it can have.
int IWouldLikeSomePyroSpritesHowManyCanIHave ( int iIWantThisMany )
{
	// Keep a tally for this frame.
	iNumWantedPyrosThisFrame += iIWantThisMany;

	return ( ( iWantedToGivenMultiplier * iIWantThisMany ) >> 8 );
}

// If the rout can't change how many it uses, at least call this to warn the pyro system that they will be used.
void IHaveToHaveSomePyroSprites( int iINeedThisMany )
{
	// Keep a tally for this frame.
	iNumFixedPyrosThisFrame += iINeedThisMany;
}




void Pyros_EndOfFrameMarker ( void )
{
	// Figure out how much we were "oversubscribed" by.
	if ( ( iNumFixedPyrosThisFrame + iNumWantedPyrosThisFrame ) > LIMIT_TOTAL_PYRO_SPRITES_PLEASE_BOB )
	{
		// Over the limit - ration pyro bits.
		iWantedToGivenMultiplier = ( ( LIMIT_TOTAL_PYRO_SPRITES_PLEASE_BOB - iNumFixedPyrosThisFrame ) * 256 ) / iNumWantedPyrosThisFrame;
		if ( iWantedToGivenMultiplier < 32 )
		{
			// Silly number - obviously far to much going on - hit the framerate,
			// rather than making it look stupid.
			iWantedToGivenMultiplier = 32;
		}
	}
	else
	{
		// Nope - still under the limit - have all you want.
		iWantedToGivenMultiplier = 256;
	}

	// And reset it.
	iNumWantedPyrosThisFrame = 0;
	iNumFixedPyrosThisFrame = 0;
}


#else

// Not using the limiting stuff - these do nothing exciting.

inline int IWouldLikeSomePyroSpritesHowManyCanIHave ( int iIWantThisMany )
{
	// Sure.
	return ( iIWantThisMany );
}

// If the rout can't change how many it uses, at leats call this to warn the phyro system that they will be used.
inline void IHaveToHaveSomePyroSprites( int iINeedThisMany )
{
	// Does nothing in this case.
}

void Pyros_EndOfFrameMarker ( void )
{
	// Does nothing in this case.
}

#endif



//
//   Pyro Utility Belt
//

inline SLONG lerp_long(SLONG a, SLONG b, UBYTE pos) {
	return ((a*(256-pos))>>8)+((b*pos)>>8);
}

GameCoord lerp_vector(GameCoord a, GameCoord b, UBYTE pos) {
  GameCoord result;

  result.X = lerp_long(a.X,b.X,pos);
  result.Y = lerp_long(a.Y,b.Y,pos);
  result.Z = lerp_long(a.Z,b.Z,pos);

  return result;

}

SLONG	get_pyro_rand(void)
{
	pyro_seed*=31415965;
	pyro_seed+=123456789;
	return(pyro_seed>>8);
}


void SPRITE_draw_tex2(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		float	u,float v,float w,float h,
		SLONG sort)
{
	float screen_size;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid);

	if (mid.IsValid())
	{
		screen_size = POLY_world_length_to_screen(world_size) * mid.Z;

		if (mid.X + screen_size < 0 ||
			mid.X - screen_size > POLY_screen_width ||
			mid.Y + screen_size < 0 ||
			mid.Y - screen_size > POLY_screen_height)
		{
			//
			// Off screen.
			// 
		}
		else
		{
			pp[0].X = mid.X - (screen_size*0.5f);
			pp[0].Y = mid.Y - screen_size;
			pp[1].X = mid.X + (screen_size*0.5f);
			pp[1].Y = mid.Y - screen_size;
			pp[2].X = mid.X - (screen_size*0.5f);
			pp[2].Y = mid.Y + screen_size;
			pp[3].X = mid.X + (screen_size*0.5f);
			pp[3].Y = mid.Y + screen_size;

			pp[0].u = u;
			pp[0].v = v;
			pp[1].u = u+w;
			pp[1].v = v;
			pp[2].u = u;
			pp[2].v = v+h;
			pp[3].u = u+w;
			pp[3].v = v+h;

			pp[0].colour = colour;
			pp[1].colour = colour;
			pp[2].colour = colour;
			pp[3].colour = colour;

			pp[0].specular = specular;
			pp[1].specular = specular;
			pp[2].specular = specular;
			pp[3].specular = specular;

			switch(sort)
			{
				case SPRITE_SORT_NORMAL:
					pp[0].z = mid.z;
					pp[0].Z = mid.Z;
					pp[1].z = mid.z;
					pp[1].Z = mid.Z;
					pp[2].z = mid.z;
					pp[2].Z = mid.Z;
					pp[3].z = mid.z;
					pp[3].Z = mid.Z;
					break;

				case SPRITE_SORT_FRONT:
					pp[0].z = 0.01F;
					pp[0].Z = 1.00F;
					pp[1].z = 0.01F;
					pp[1].Z = 1.00F;
					pp[2].z = 0.01F;
					pp[2].Z = 1.00F;
					pp[3].z = 0.01F;
					pp[3].Z = 1.00F;
					break;
				
				default:
					ASSERT(0);
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, page, FALSE, TRUE);
		}
	}
}



void	draw_flame_element2(SLONG x,SLONG y,SLONG z,SLONG c0)
{
	SLONG	trans;
	SLONG   page;
	float   scale;
	float   u,v,w,h;
	UBYTE*  palptr;
	SLONG   palndx;
	SLONG	dx,dy,dz;

	pyro_seed=54321678+c0;


		w=h=1.0;
		u=v=0.0;

		if(!(c0&3))
			page=POLY_PAGE_FLAMES;
		else
			page=POLY_PAGE_SMOKE;

//		dy=get_pyro_rand()&0x1ff;
		dy=get_pyro_rand()&0x1ff;
		dy+=(GAME_TURN*5);
		dy%=256;
		dx=(((get_pyro_rand()&0xff)-128)*((dy>>2)+150))>>9;
		dz=(((get_pyro_rand()&0xff)-128)*((dy>>2)+150))>>9;
		if (page==POLY_PAGE_FLAMES) {
			dx>>=2; dz>>=2;
		}

		trans=255-dy;

		dx+=x;
		dy+=y;
		dz+=z;


		if(trans>=1)
		{
			switch (page) {
			  case POLY_PAGE_FLAMES:
			    palptr=(trans*3)+fire_pal;
				palndx=(256-trans)*3;
				trans<<=24;
				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				scale=150;
				SPRITE_draw_tex2(
					float(dx),
					float(dy),
					float(dz),
					float(scale),
					trans,
					0,
					page,
					u, v, w, h,
					SPRITE_SORT_NORMAL);
				break;
			  case POLY_PAGE_SMOKE:
				trans+=(trans<<8)|(trans<<16)|(trans<<24);
				scale=((dy-y)>>1)+50;
				dy+=50;
				SPRITE_draw_tex(
					float(dx),
					float(dy),
					float(dz),
					float(scale),
					trans,
					0,
					page,
					u, v, w, h,
					SPRITE_SORT_NORMAL);
				break;
			}
		}
	
}


void PYRO_draw_explosion(Pyrex *pyro);
void PYRO_draw_explosion2(Pyro *pyro);
void PYRO_draw_newdome(Pyro *pyro);
void PYRO_draw_dustwave(Pyro *pyro);
void PYRO_draw_streamer(Pyro *pyro);
void PYRO_draw_twanger(Pyro *pyro);
void PYRO_draw_blob(Pyro *pyro);
void PYRO_draw_armageddon(Pyro *pyro);
/*
void	check_pyro(Pyro *pyro)
{
	static	UBYTE remember[1000];
	SLONG	index;

	index=PYRO_NUMBER(pyro);

	if(pyro->counter)
	if(pyro->counter==remember[index])
	{
		DebugText(" pyro %d counter %d again error thing %d\n",index,pyro->counter,THING_NUMBER(pyro->thing));
//		ASSERT(0);
	}

	remember[index]=pyro->counter;


}
*/
void PYRO_draw_pyro(Thing *p_pyro) {
	Pyro *pyro = PYRO_get_pyro(p_pyro);
	SLONG fx,fy,fz;
	SLONG i,j;
	GameCoord pos;
	float dir[8][2] = { { 0.0f, 1.0f}, { 0.7f, 0.7f}, { 1.0f, 0.0f}, { 0.7f,-0.7f}, { 0.0f,-1.0f}, {-0.7f,-0.7f}, {-1.0f, 0.0f}, {-0.7f, 0.7f} };
	float uvs[8][2] = { { 0.5f, 1.0f}, { 0.85f, 0.85f}, { 1.0f, 0.5f}, { 0.85f, 0.15f}, { 0.5f, 0.0f}, { 0.15f, 0.15f}, { 0.0f, 0.5f}, { 0.15f, 0.85f} };

	POLY_flush_local_rot();

	fx=p_pyro->WorldPos.X;
	fy=p_pyro->WorldPos.Y;
	fz=p_pyro->WorldPos.Z;

	switch(pyro->PyroType) {
	case PYRO_EXPLODE:
		PYRO_draw_explosion((Pyrex*)pyro);
		break;

	case PYRO_FIREWALL:
/*
		for (i=0;i<pyro->counter;i++) {
			pos = lerp_vector(p_pyro->WorldPos,pyro->target,i);
			draw_flame_element(pos.X>>8,pos.Y>>8,pos.Z>>8,i,1);
			if (!(i&3)) {
				pyro_seed=54321678+(i*get_pyro_rand());
				draw_flame_element2(pos.X>>8,pos.Y>>8,pos.Z>>8,get_pyro_rand());
			}
		}*/
		break;
	case PYRO_FIREPOOL:
		if (pyro->thing->Flags&FLAGS_IN_VIEW)
		{
			SLONG radsqr,dx,dz,distsqr,radius,id;
			GameCoord ctr,edge;
			POLY_Point  pp[3];
			POLY_Point *tri[3];
			POLY_Point	temppnt;

			ctr=p_pyro->WorldPos;
			ctr.X>>=8; ctr.Y>>=8; ctr.Z>>=8;

			POLY_transform(ctr.X,ctr.Y+0xa,ctr.Z,pp);

			tri[0] = &pp[0];
			tri[1] = &pp[1];
			tri[2] = &pp[2];

			pp[0].colour = 0xFFFFFFFF;
			pp[1].colour = 0xFFFFFFFF;
			pp[2].colour = 0xFFFFFFFF;
			pp[0].specular = 0xFF000000;
			pp[1].specular = 0xFF000000;
			pp[2].specular = 0xFF000000;

			pp[0].u=0.5; pp[0].v=0.5;

			if (pyro->Flags&PYRO_FLAGS_TOUCHPOINT) {
				radius=pyro->radius;
				distsqr=(pyro->counter*pyro->radius)>>7; // double to get diameter
				distsqr*=distsqr;
			} else {
				radius=(pyro->counter*pyro->radius)>>8;

				edge=ctr; edge.Y+=0xa;
				edge.X+=dir[0][0]*pyro->radii[0]; edge.Z+=dir[0][1]*pyro->radii[0];
				POLY_transform(edge.X,edge.Y,edge.Z,&temppnt);
				pp[2].X=temppnt.X; pp[2].Y=temppnt.Y; pp[2].Z=temppnt.Z;
				pp[2].clip=temppnt.clip; pp[2].z=temppnt.z;
				pp[2].u=uvs[0][0]; pp[2].v=uvs[0][1];

				// Throttle
				int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 16 );

				for (i=0;i<iNumFlames;i++) {
					if (pyro->radii[i]<(unsigned)radius) {
						id=(pyro->counter<<3)+(i<<8);
						id&=2047;
						pyro->radii[i]+=abs(SIN(id)/4095);
					} else {
						id=((GAME_TURN<<1)+(i<<8));
						id&=2047;
						pyro->radii[i]=radius+256+(SIN(id)/256);
					}
					j=(i+1)&7;
					pp[1]=pp[2];

					pp[2].u=uvs[j][0]; pp[2].v=uvs[j][1];
					edge=ctr; edge.Y+=0xa;
					edge.X+=dir[j][0]*pyro->radii[j];
					edge.Z+=dir[j][1]*pyro->radii[j];
					POLY_transform(edge.X,edge.Y,edge.Z,&temppnt);
					pp[2].X=temppnt.X; pp[2].Y=temppnt.Y; pp[2].Z=temppnt.Z;
					pp[2].clip=temppnt.clip; pp[2].z=temppnt.z;
					if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
					{
						POLY_add_triangle(tri,POLY_PAGE_FLAMES,FALSE,TRUE);
					}
				}
			}
			id=0;
			radsqr=radius*radius;

			// Throttle
			int iConst = ( pyro->radius * pyro->radius );
			int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( iConst / ( 25 * 25 ) );

			if ( iNumFlames < 10 )
			{
				iNumFlames = 10;
			}

			int iStepSize = (int)sqrtf ( (float)iConst / (float)iNumFlames );

			for (i=-pyro->radius;i<pyro->radius;i+=iStepSize) {
				for (j=-pyro->radius;j<pyro->radius;j+=iStepSize) {
					id*=31415965;
					id+=123456789;
					if ((i*i+j*j)<radsqr) {
						pos=ctr;
						pos.X+=i; pos.Z+=j;
						if (pyro->Flags&PYRO_FLAGS_TOUCHPOINT) {
							dx=(pyro->target.X>>8)-pos.X;
							dz=(pyro->target.Z>>8)-pos.Z;
							if ((dx*dx+dz*dz)<distsqr)
								draw_flame_element(pos.X,pos.Y,pos.Z,id,1,0);
						} else {
							draw_flame_element(pos.X,pos.Y,pos.Z,id,1,0);
						}
					}
				}
			}

		}
		break;

	case PYRO_BONFIRE:
		if (pyro->thing->Flags&FLAGS_IN_VIEW) 
		{

extern	int AENG_detail_skyline;

			int iNumFlames = 40;
#ifndef TARGET_DC
extern UBYTE sw_hack;
			if (sw_hack || !AENG_detail_skyline)//||ShiftFlag)
#else
			if (!AENG_detail_skyline)//||ShiftFlag)
#endif
			{
				iNumFlames *= 2;
			}
			else
			{
				iNumFlames *= 5;
			}


			// Throttle
			iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( iNumFlames );
			// And draw.
			draw_flames(fx>>8,fy/256,fz>>8,iNumFlames,(SLONG)p_pyro);

			if(AENG_detail_skyline)
			if (!(Random()&7))
			PARTICLE_Add(fx+((Random()&0x9ff)-0x4ff),
						 fy+((Random()&0x9ff)-0x4ff),
						 fz+((Random()&0x9ff)-0x4ff),
				(Random()&0xff)-0x7f,256+(Random()&0x1ff),(Random()&0xff)-0x7f,
				POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFFFFFF,
				PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
				300,70,1,1,2);
		}
		break;

	case PYRO_WHOOMPH:
		if (pyro->thing->Flags&FLAGS_IN_VIEW) 
		{



			UBYTE i, radius;
#if 0
			UBYTE steps=1+(pyro->counter>>4);
#else
			// Throttle
			int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( ( 1+(pyro->counter>>4) ) * 2 );
			UBYTE steps = iNumFlames >> 1;
			if ( steps < 3 )
			{
				steps = 3;
			}

#endif
			UWORD angle=0, step=2048/steps;
			SLONG px, py, pz;

			for (i=0;i<steps;i++)
			{
				radius=SIN(pyro->counter)>>10;
				radius+=(radius>>1);
				px=fx+((SIN(angle)>>6)*radius);
				py=fy;
				pz=fz+((COS(angle)>>6)*radius);
				PARTICLE_Add(px+((Random()&0x13ff)-0x9ff),
							 py+((Random()&0x13ff)-0x9ff),
							 pz+((Random()&0x13ff)-0x9ff),
							 0,0,0,
							 POLY_PAGE_PCFLAMER,2+((Random()&3)<<2),0xaFffffff,
							 PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE,
							 150,70,1,8,2);
				PARTICLE_Add(px+((Random()&0x13ff)-0x9ff),
							 py+((Random()&0x13ff)-0x9ff),
							 pz+((Random()&0x13ff)-0x9ff),
							 0,0,0,
							 POLY_PAGE_PCFLAMER,2+((Random()&3)<<2),0xaFffffff,
							 PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE,
							 150,70,1,8,2);

				angle+=step;
				angle&=2047;
			}

/*			PARTICLE_Add(fx+((Random()&0x9ff)-0x4ff),
						 fy+((Random()&0x9ff)-0x4ff),
						 fz+((Random()&0x9ff)-0x4ff),
						 0,0,0,
						 POLY_PAGE_PCFLAMER,2+((Random()&3)<<2),0xFFffffff,
						 PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE,
						 300,70,1,1,2);*/

/*			draw_flames(fx>>8,fy/256,fz>>8,40,(SLONG)p_pyro);
			if (!(Random()&3))
			PARTICLE_Add(fx+((Random()&0x9ff)-0x4ff),
						 fy+((Random()&0x9ff)-0x4ff),
						 fz+((Random()&0x9ff)-0x4ff),
				(Random()&0xff)-0x7f,256+(Random()&0x1ff),(Random()&0xff)-0x7f,
				POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7FFFFFFF,
				PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE,
				300,70,1,1,2);
				*/
		}
		break;

	case PYRO_IMMOLATE:
		if (pyro->Flags&PYRO_FLAGS_SMOKE) {
				if (pyro->victim)
					pos = pyro->victim->WorldPos;
				else
					pos = pyro->thing->WorldPos;
				if (pyro->Flags&PYRO_FLAGS_STATIC)
				{
					pos.X>>=8; pos.Y>>=8; pos.Z>>=8;

					// Throttle
					int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 40 );

					for (i=0;i<iNumFlames;i++)
					{
						draw_flame_element(pos.X,pos.Y,pos.Z,7+(i<<4),0,0);
					}
				}
				else
				{
					PARTICLE_Add(pos.X,pos.Y+8192,pos.Z,0,1024,0,POLY_PAGE_SMOKE,0,0x7FFFFFFF,
						PFLAGS_SMOKE,80,8,1,3,4);
				}

		}
		if (pyro->Flags&PYRO_FLAGS_FLAME) {
				if (pyro->victim)
					pos = pyro->victim->WorldPos;
				else
					pos = pyro->thing->WorldPos;
				if (pyro->Flags&PYRO_FLAGS_STATIC)
				{
					pos.X>>=8; pos.Y>>=8; pos.Z>>=8;
					// Throttle
					int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 40 );

					for (i=0;i<iNumFlames;i++)
					{
						draw_flame_element(pos.X,pos.Y,pos.Z,i,0,0);
					}
				} else {
					PARTICLE_Add(pos.X,pos.Y+8192,pos.Z,0,1024,0,POLY_PAGE_FLAMES,0,0xffFFFFFF,
						PFLAG_FIRE|PFLAG_FADE|PFLAG_WANDER,80,60,1,4,-1);
				}

		}
		if (pyro->victim) {
			if (!(pyro->victim->Flags&FLAGS_IN_VIEW)) break;
			if ((pyro->Flags&PYRO_FLAGS_FLICKER)||(pyro->Flags&PYRO_FLAGS_BONFIRE)) {
				PrimObject*	p_obj;
				ULONG sp;
				ULONG ep;
				SLONG px,py,pz;
				POLY_Point*	pp;
				SLONG matrix[9];
				GameCoord bob;
				switch(pyro->victim->DrawType) {
				case DT_MESH:
					switch(pyro->victim->Class)
					{
					case CLASS_BARREL:
						{
							bob=BARREL_fire_pos(pyro->victim);
							px = bob.X + (Random()&0x3fff)-0x1fff;
							py = bob.Y + (Random()&0x3fff)-0x13ff;
							pz = bob.Z + (Random()&0x3fff)-0x1fff;
							RIBBON_extend(pyro->radii[0],px>>8,py>>8,pz>>8);
						}					
						break;
					}
					break;
				case DT_CHOPPER:
					{
						p_obj=&prim_objects[pyro->victim->Draw.Mesh->ObjectId];
						sp = p_obj->StartPoint;
						ep = p_obj->EndPoint;
						FMATRIX_calc(
							matrix,
							pyro->victim->Draw.Mesh->Angle,
							pyro->victim->Draw.Mesh->Tilt,
							pyro->victim->Draw.Mesh->Roll);
						pp = &POLY_buffer[POLY_buffer_upto];

						if (pyro->Flags&PYRO_FLAGS_FLICKER) {
							if ((pyro->radii[7]<sp)||(pyro->radii[7]>ep)) pyro->radii[7]=sp;
							pp->x=AENG_dx_prim_points[pyro->radii[7]].X;
							pp->y=AENG_dx_prim_points[pyro->radii[7]].Y;
							pp->z=AENG_dx_prim_points[pyro->radii[7]].Z;

							FMATRIX_MUL(matrix,pp->x,pp->y,pp->z);

							pos.X = (pp->x)+(pyro->victim->WorldPos.X>>8);
							pos.Y = (pp->y)+(pyro->victim->WorldPos.Y>>8); 
							pos.Z = (pp->z)+(pyro->victim->WorldPos.Z>>8);

							RIBBON_extend(pyro->radii[0],pos.X,pos.Y,pos.Z);
							pyro->radii[7]++;
						}
						if (pyro->Flags&PYRO_FLAGS_BONFIRE)
						{
							for (i=sp;i<(signed)ep;i++)
							  if (!(i&7))
							  {
								ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

								pp = &POLY_buffer[POLY_buffer_upto];
								pp->x=AENG_dx_prim_points[i].X;
								pp->y=AENG_dx_prim_points[i].Y;
								pp->z=AENG_dx_prim_points[i].Z;

								FMATRIX_MUL(matrix,pp->x,pp->y,pp->z);

								pos.X = (pp->x)+(pyro->victim->WorldPos.X>>8);
								pos.Y = (pp->y)+(pyro->victim->WorldPos.Y>>8); 
								pos.Z = (pp->z)+(pyro->victim->WorldPos.Z>>8);

								for (j=0;j<8;j++) {
									draw_flame_element2(pos.X,pos.Y,pos.Z,i+(j*128));
									draw_flame_element2(pos.X,pos.Y,pos.Z,i+(j*128)+1);
								}
							  
							  }
						}

						return;
					}
					break;
				case DT_ROT_MULTI:
					if (pyro->Flags & PYRO_FLAGS_FLICKER)
					{
						SLONG px,py,pz;
						UBYTE i,r,p;

						if (pyro->Dummy==2) r=5; else r=1; //r=1 was r=2 MikeD Aug 2000
						r=2;
						for (i=0;i<r;i++) {
							switch(pyro->victim->State){
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
							px += pyro->victim->WorldPos.X >> 8;
							py += pyro->victim->WorldPos.Y >> 8;
							pz += pyro->victim->WorldPos.Z >> 8;
							RIBBON_extend(pyro->radii[i],px,py,pz);
							/*for (p=0;p<5;p++) {
								calc_sub_objects_position(
									pyro->victim,
									pyro->victim->Draw.Tweened->AnimTween,
									p,
									&px,
									&py,
									&pz);
								px = pyro->victim->WorldPos.X >> 8;
								py = pyro->victim->WorldPos.Y >> 8;
								pz = pyro->victim->WorldPos.Z >> 8;
								PARTICLE_Add(px,py,pz,0,512,0,POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x00FFFFFF,
									PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE,
									300,50,1,1,1);
							}*/
						}					
					}
					return;
				}
			}
/*			if (pyro->Flags&PYRO_FLAGS_FACES) {
				PrimObject*	p_obj;
				SLONG sp;
				SLONG ep;
				POLY_Point*	pp;
				SLONG matrix[9];

				switch (pyro->victim->DrawType) {
				case DT_MESH:
				case DT_CHOPPER:
					{
						p_obj=&prim_objects[pyro->victim->Draw.Mesh->ObjectId];
						sp = p_obj->StartPoint;
						ep = p_obj->EndPoint;
						FMATRIX_calc(
							matrix,
							pyro->victim->Draw.Mesh->Angle,
							pyro->victim->Draw.Mesh->Tilt,
							pyro->victim->Draw.Mesh->Roll);
						pp = &POLY_buffer[POLY_buffer_upto];
						if ((pyro->radii[7]<sp)||(pyro->radii[7]>ep)) pyro->radii[7]=sp;
						pp->x=AENG_dx_prim_points[pyro->radii[7]].X;
						pp->y=AENG_dx_prim_points[pyro->radii[7]].Y;
						pp->z=AENG_dx_prim_points[pyro->radii[7]].Z;

						FMATRIX_MUL(matrix,pp->x,pp->y,pp->z);

						pos.X = (pp->x)+(pyro->victim->WorldPos.X>>8);
						pos.Y = (pp->y)+(pyro->victim->WorldPos.Y>>8); 
						pos.Z = (pp->z)+(pyro->victim->WorldPos.Z>>8);

						RIBBON_extend(pyro->radii[0],pos.X,pos.Y,pos.Z);

						pyro->radii[7]++;
						return;
					}
					break;
				case DT_ROT_MULTI:
					{
					SLONG px,py,pz;
					UBYTE i,r,p;

					if (pyro->padding==2) r=5; else r=2;
					for (i=0;i<r;i++) {
						switch(pyro->victim->State){
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
						px += pyro->victim->WorldPos.X >> 8;
						py += pyro->victim->WorldPos.Y >> 8;
						pz += pyro->victim->WorldPos.Z >> 8;
						RIBBON_extend(pyro->radii[i],px,py,pz);
					}					
					}
					return;
				default:
					pos = pyro->victim->WorldPos;
					pos.X>>=8; pos.Y>>=8; pos.Z>>=8;
					for (i=0;i<40;i++)
						draw_flame_element(pos.X,pos.Y,pos.Z,i,0);
					return;					
				}


			  for (i=sp;i<ep;i++)
				  if (!(i&7))
				  {
					ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

					pp = &POLY_buffer[POLY_buffer_upto];
					pp->x=AENG_dx_prim_points[i].X;
					pp->y=AENG_dx_prim_points[i].Y;
					pp->z=AENG_dx_prim_points[i].Z;

					FMATRIX_MUL(matrix,pp->x,pp->y,pp->z);

					pos.X = (pp->x)+(pyro->victim->WorldPos.X>>8);
					pos.Y = (pp->y)+(pyro->victim->WorldPos.Y>>8); 
					pos.Z = (pp->z)+(pyro->victim->WorldPos.Z>>8);

					for (j=0;j<8;j++) {
						draw_flame_element2(pos.X,pos.Y,pos.Z,i+(j*128));
						draw_flame_element2(pos.X,pos.Y,pos.Z,i+(j*128)+1);
					}
				  }


			} else {
				pos = pyro->victim->WorldPos;
				pos.X>>=8; pos.Y>>=8; pos.Z>>=8;
				for (i=0;i<40;i++)
					draw_flame_element(pos.X,pos.Y,pos.Z,i,0);
			}*/
		} else {
			if (pyro->Flags&PYRO_FLAGS_FLICKER) {
				pos=pyro->thing->WorldPos;
				pos.X+=(rand()/2);
				pos.Z+=(rand()/2);
				RIBBON_extend(pyro->radii[0],pos.X>>8,pos.Y/256,pos.Z>>8);
			}
		}
		break;

	case PYRO_DUSTWAVE:
		PYRO_draw_dustwave(pyro);
		break;

	case PYRO_EXPLODE2:
		PYRO_draw_explosion2(pyro);
		break;

	case PYRO_STREAMER:
		PYRO_draw_streamer(pyro);
		break;

	case PYRO_TWANGER:
		PYRO_draw_twanger(pyro);
		{
		  SLONG str;
		  if (pyro->counter<30) {
		    str=pyro->counter*5;
		  } else {
			str=(285-pyro->counter*2);
			str>>=1;
		  }
		  if (str>0) BLOOM_flare_draw(pyro->thing->WorldPos.X>>8,pyro->thing->WorldPos.Y>>8,pyro->thing->WorldPos.Z>>8,str);
		}
		break;

	case PYRO_NEWDOME:
		PYRO_draw_newdome(pyro);
		break;

	case PYRO_SPLATTERY:
		// this only creates other things so no drawing
		break;

	case PYRO_FIREBOMB:
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

				// Ten "clocks" of particle-creation, each particle lives for about 70 clocks.
				int iNumSprites = IWouldLikeSomePyroSpritesHowManyCanIHave ( FIREBOMB_SPRITES * 10 * 70 );
				iNumSprites /= ( 10 * 70 );
				int iMultiplier = ( 16 * (1<<7) ) / iNumSprites;


				for (d=0;d<iNumSprites;d++) {
					int iAngle = d * iMultiplier;
					if ((pyro->Flags&PYRO_FLAGS_WAVE)&&(pyro->counter<6))
					{
#if 0
						x=SIN((d<<7)+(Random()&127))>>3; z=COS((d<<7)+(Random()&127))>>3;
#else
						x=SIN(iAngle+(Random()&127))>>3;
						z=COS(iAngle+(Random()&127))>>3;
#endif

						PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
							x,(Random()&0xff),z,
							POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x7FFFFFFF,
							PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING,
							55+(Random()&0x3f),80,1,8-(Random()&3),4);
					}

					x=SIN(iAngle)>>4; z=COS(iAngle)>>4;
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
						POLY_PAGE_FLAMES2,2+((Random()&3)<<2),h,
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
					POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0x7FFFFFFF,
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING|PFLAG_GRAVITY,
					75+(Random()&0x3f),160,1,5+(Random()&3),-(2+(Random()&3)));
			}
			if (pyro->counter<240) {
				if ((pyro->counter>110)&&(pyro->counter<140)) {
					PARTICLE_Add(pyro->thing->WorldPos.X,pyro->thing->WorldPos.Y,pyro->thing->WorldPos.Z,
						0,0,0,
						POLY_PAGE_FLAMES2,2+((Random()&3)<<2),0xffFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE,
						100,255,1,20,5);
				}
				//if (pyro->counter>10) pyro->counter+=10;
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
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FIRE|PFLAG_FADE|PFLAG_RESIZE|PFLAG_DAMPING,
					70+(Random()&0x3f),100,1,2,4+(Random()&3));
				}
			}
		
		}
		break;

	case PYRO_GAMEOVER:
		PYRO_draw_armageddon(pyro);
		break;

	case PYRO_HITSPANG:
		// in case you were wondering, a spang is a sort of ricochet/blast effect
		// that appears when bullets hit you (or, i guess, walls, cars, etc)

//		check_pyro(pyro); //debug tool to find a bug MD 30-12-99

		// make sure it is processed


//		if(pyro->counter)
#ifdef PSX
		{
		  POLY_Point pt1,pt2;
		  SLONG x,y,z;

//		  POLY_transform(x+pyro->target.X,y+pyro->target.Y,z+pyro->target.Z,&pt1);

		  // hardwired test
/*		  x=NET_PERSON(0)->WorldPos.X>>8;
		  y=(NET_PERSON(0)->WorldPos.Y>>8)+256;
		  z=NET_PERSON(0)->WorldPos.Z>>8;
		  POLY_transform(x,y,z,&pt1);*/

		  x=pyro->thing->WorldPos.X>>8;
		  y=pyro->thing->WorldPos.Y>>8;
		  z=pyro->thing->WorldPos.Z>>8;

		  if (pyro->counter)
			POLY_transform(x+pyro->target.X,y+pyro->target.Y,z+pyro->target.Z,&pt1);
		  else
			POLY_transform(x+(pyro->target.X<<1),y+(pyro->target.Y<<1),z+(pyro->target.Z<<1),&pt1);


		  POLY_transform(x,y,z,&pt2);

		  pt1.colour=pt2.colour=0xFFFFFFFF;
		  pt1.specular=pt2.specular=0xFF000000;

		  switch(pyro->counter)
		  {
		  case 0:
			pt1.u=0;   pt1.v=0;
			pt2.u=0.5; pt2.v=0.5;
			break;
		  case 1:
			pt1.u=0;   pt1.v=0.5;
			pt2.u=0.5; pt2.v=1.0;
			break;
		  case 2:
			pt1.u=0.5; pt1.v=0;
			pt2.u=1.0; pt2.v=0.5;
			break;
		  case 3:
			pt1.u=0.5; pt1.v=0.5;
			pt2.u=1.0; pt2.v=1.0;
			break;
		  }
		  if (POLY_valid_line(&pt1,&pt2)) 
		  {
			if (pyro->counter>1)
				POLY_add_line_tex_uv(&pt1,&pt2,42,42,POLY_PAGE_HITSPANG,0);
			else
				POLY_add_line_tex_uv(&pt1,&pt2,22,22,POLY_PAGE_HITSPANG,0);
		  }
		}
#endif
		break;
	}

}


extern RadPoint PYRO_defaultpoints[16];

#define TEXSCALE  0.003f
#define TEXSCALE2 0.006f

void PYRO_draw_explosion(Pyrex *pyro) {
	POLY_Point  pp[3];
	POLY_Point *tri[3];
	UBYTE i,j;
	SLONG ok,spec;
	SLONG cx,cy,cz;
	RadPoint points[17];

	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;

	for (i=0;i<16;i++) {
		points[i].x=(PYRO_defaultpoints[i].x*pyro->points[i].radius)>>16;
		points[i].y=(PYRO_defaultpoints[i].y*pyro->points[i].radius)>>16;
		points[i].z=(PYRO_defaultpoints[i].z*pyro->points[i].radius)>>16;
	}
	points[16].y=(65535*pyro->points[i].radius)>>16;

	spec=(255-(pyro->thing->Genus.Pyro->Timer1*4));
	if (spec<0) spec=0;
	spec*=0x010101;

	DIRT_gust(pyro->thing,cx,cz,(points[0].x/4)+cx,(points[0].z/4)+cz);
	DIRT_gust(pyro->thing,cx,cz,(points[4].x/4)+cx,(points[4].z/4)+cz);

	// Draw bottom "rung"
	for (i=0;i<8;i++) {

		j=i+1;
		if (j==8) j=0;
		POLY_transform(	points[i].x+cx,
						points[i].y+cy,
						points[i].z+cz,
						&pp[0]);
		pp[0].u=points[i].x*TEXSCALE;
		pp[0].v=points[i].z*TEXSCALE;
		POLY_transform(	points[i+8].x+cx,
						points[i+8].y+cy,
						points[i+8].z+cz,
						&pp[1]);
		pp[1].u=points[i+8].x*TEXSCALE;
		pp[1].v=points[i+8].z*TEXSCALE;
		POLY_transform(	points[j+8].x+cx,
						points[j+8].y+cy,
						points[j+8].z+cz,
						&pp[2]);
		pp[2].u=points[j+8].x*TEXSCALE;
		pp[2].v=points[j+8].z*TEXSCALE;
		pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+(pyro->thing->Genus.Pyro->Timer1<<24);
		pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
		if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
		{
			POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
		}

		POLY_transform(	points[i].x+cx,
						points[i].y+cy,
						points[i].z+cz,
						&pp[0]);
		pp[0].u=points[i].x*TEXSCALE;
		pp[0].v=points[i].z*TEXSCALE;
		POLY_transform(	points[j+8].x+cx,
						points[j+8].y+cy,
						points[j+8].z+cz,
						&pp[1]);
		pp[1].u=points[j+8].x*TEXSCALE;
		pp[1].v=points[j+8].z*TEXSCALE;
		POLY_transform(	points[j].x+cx,
						points[j].y+cy,
						points[j].z+cz,
						&pp[2]);
		pp[2].u=points[j].x*TEXSCALE;
		pp[2].v=points[j].z*TEXSCALE;
		pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+(pyro->thing->Genus.Pyro->Timer1<<24);
		pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
		if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
		{
			POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
		}
	}

	// Draw top "rung"
	for (i=8;i<16;i++) {
		j=i+1;
		if (j==16) j=8;
		POLY_transform(	points[i].x+cx,
						points[i].y+cy,
						points[i].z+cz,
						&pp[0]);
		pp[0].u=points[i].x*TEXSCALE;
		pp[0].v=points[i].z*TEXSCALE;
		POLY_transform(	cx,
						points[16].y+cy,
						cz,
						&pp[1]);
		pp[1].u=0;
		pp[1].v=0;
		POLY_transform(	points[j].x+cx,
						points[j].y+cy,
						points[j].z+cz,
						&pp[2]);
		pp[2].u=points[j].x*TEXSCALE;
		pp[2].v=points[j].z*TEXSCALE;
		pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+(pyro->thing->Genus.Pyro->Timer1<<24);
		pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
		if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
		{
			POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
		}
	}

}


// Lo Detail:
//#define DUSTWAVE_SECTORS	8
//#define DUSTWAVE_MULTIPLY	256
/*
void PYRO_draw_dustwave(Pyro *pyro) {
	POLY_Point  pp[3],mid;
	POLY_Point *tri[3] = { &pp[0], &pp[1], &pp[2] };
	SLONG cx,cy,cz,ok,fade;
	UBYTE sections, pass, sector, next;
	SLONG dxs[DUSTWAVE_SECTORS],dys[DUSTWAVE_SECTORS], dists[4], heights[4];
	float thisscale, nextscale;

	// we'll need these to add on to relative coords
	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;
	// and we'll often need to join stuff back up to the centre
	POLY_transform( cx, cy, cz, &mid);
	mid.u=mid.v=0;
/+
	sections=4;
	if (pyro->counter<192) sections=3;
	if (pyro->counter<128) sections=2;
	if (pyro->counter<64) sections=1;

+/	// debug override
	sections=3;

	for(sector=0;sector<DUSTWAVE_SECTORS;sector++) {
		dxs[sector]=SIN(sector*DUSTWAVE_MULTIPLY)/256;
		dys[sector]=COS(sector*DUSTWAVE_MULTIPLY)/256;
	}

	// precalc the ring data
	for(pass=0;pass<4;pass++) {
		switch(pass) {
		case 0:
			/+
			if (pyro->counter<120) {
				dists[0]=SIN(pyro->counter*4)/128;
			} else {
				dists[0]=512+(pyro->counter-120);
			}
			+/
			if (pyro->counter<60) {
				dists[0]=SIN(pyro->counter*6)/128;
			} else {
				dists[0]=(SIN(60*6)/128)+((pyro->counter-60)*2);
			}
			heights[0]=2;
			break;
		case 1:
			dists[1]=(dists[0]*SIN(pyro->counter*2))/65536;
			heights[1]=SIN(pyro->counter*4)/1024;
			break;
		case 2:
			dists[2]=(dists[1]*SIN(pyro->counter*2))/65536;
			heights[2]=heights[1]*0.75;
			break;
		case 3:
			dists[3]=(dists[2]*SIN(pyro->counter*2))/65536;
			heights[3]=2;
			break;
		}
	}

	if (pyro->counter<192)
		fade=0;
	else
		fade=((pyro->counter-192)*4)<<24;

	// draw the data
	for(pass=0;pass<sections;pass++) {
		for(sector=0;sector<DUSTWAVE_SECTORS;sector++) {
			next=sector+1;
			if (next==DUSTWAVE_SECTORS) next=0;

			thisscale=TEXSCALE*((4-pass)*0.25);
			nextscale=TEXSCALE*((3-pass)*0.25);

			POLY_transform( cx+((dists[pass]*dxs[sector])/256), cy+heights[pass], cz+((dists[pass]*dys[sector])/256), &pp[0]);
			POLY_transform( cx+((dists[pass]*dxs[next])/256),   cy+heights[pass], cz+((dists[pass]*dys[next])/256),   &pp[1]);
			pp[0].u=dxs[sector]*thisscale;	pp[0].v=dys[sector]*thisscale;
			pp[1].u=dxs[next  ]*thisscale;	pp[1].v=dys[next  ]*thisscale;

			if ((pass<sections-1)||(pass==2)) {
				POLY_transform( cx+(dists[pass+1]*dxs[sector])/256, cy+heights[pass+1], cz+(dists[pass+1]*dys[sector])/256, &pp[2]);
				pp[2].u=(dxs[sector]*nextscale);	pp[2].v=(dys[sector]*nextscale);
			} else {
				pp[2]=mid;
			}
			pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+fade;
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_DUSTWAVE,FALSE);
			}

			if ((pass<sections-1)||(pass==2)) {
				POLY_transform( cx+((dists[pass+1]*dxs[next])/256), cy+heights[pass+1], cz+((dists[pass+1]*dys[next])/256), &pp[0]);
				pp[0].u=(dxs[next]*nextscale);	pp[0].v=(dys[next]*nextscale);
				pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+fade;
				pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000;
				if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
				{
					POLY_add_triangle(tri,POLY_PAGE_DUSTWAVE,FALSE);
				}
			}
		}
	}

	// add some shock:
//	for(sector=0;sector<DUSTWAVE_SECTORS;sector++)
	{
		static UBYTE shock_sector=0;
		DIRT_gust(pyro->thing,
			cx+((dists[2]*dxs[shock_sector])/256),
			cz+((dists[2]*dys[shock_sector])/256),
			cx+((dists[1]*dxs[shock_sector])/256),
			cz+((dists[1]*dys[shock_sector])/256)
			);
		shock_sector++;
		if (shock_sector==DUSTWAVE_SECTORS) shock_sector=0;
	}
	

}
*/



void PYRO_draw_dustwave(Pyro *pyro) {
	POLY_Point  pp[3],mid;
	POLY_Point *tri[3] = { &pp[0], &pp[1], &pp[2] };
	SLONG cx,cy,cz,ok,fade;
	UBYTE sections, pass, sector, next;
	SLONG dxs[DUSTWAVE_SECTORS],dys[DUSTWAVE_SECTORS], dists[4], heights[4];
	float thisscale, nextscale;

	// we'll need these to add on to relative coords
	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;
	// and we'll often need to join stuff back up to the centre
	POLY_transform( cx, cy, cz, &mid);
//	mid.u=mid.v=0;
	mid.u=0.5;
	mid.v=1.0;

	sections=3;


	int iNumSectors = IWouldLikeSomePyroSpritesHowManyCanIHave ( DUSTWAVE_SECTORS );

	if ( iNumSectors < 7 )
	{
		// Looks silly with less than 7
		iNumSectors = 7;
	}

	int iMultiplier = 2048 / iNumSectors;

	for(sector=0;sector<iNumSectors;sector++)
	{
		dxs[sector]=SIN((sector*iMultiplier)) >> 8;
		dys[sector]=COS((sector*iMultiplier)) >> 8;
	}

#if 0
	// What the fuck?
	// Why not just - er - do these in order? Why the for and the switch?
	// Bonkers, mate. ATF.


	// precalc the ring data
	for(pass=0;pass<4;pass++) {
		switch(pass) {
		case 0:
			if (pyro->counter>1)
				dists[0]=512+SIN(pyro->counter*4)/256;
			else
				dists[0]=256+SIN(pyro->counter*4)/256;
/*			if (pyro->counter<60) {
				dists[0]=SIN(pyro->counter*6)/128;
			} else {
				dists[0]=(SIN(60*6)/128)+((pyro->counter-60)*2);
			}*/
			heights[0]=2;
			break;
		case 1:
			dists[1]=(dists[0]*SIN(pyro->counter*4))/65536;
			heights[1]=SIN(pyro->counter*4)/1024;
			break;
		case 2:
			dists[2]=(dists[1]*SIN(pyro->counter*4))/65536;
			heights[2]=heights[1]*0.75f;
			break;
		case 3:
			dists[3]=(dists[2]*SIN(pyro->counter*4))/65536;
			heights[3]=2;
			break;
		}
	}
#else
	// precalc the ring data
	if (pyro->counter>1)
		dists[0]=512+SIN(pyro->counter*4)/256;
	else
		dists[0]=256+SIN(pyro->counter*4)/256;
/*	if (pyro->counter<60) {
		dists[0]=SIN(pyro->counter*6)/128;
	} else {
		dists[0]=(SIN(60*6)/128)+((pyro->counter-60)*2);
	}*/
	heights[0]=2;

	dists[1]=(dists[0]*SIN(pyro->counter*4))/65536;
	heights[1]=SIN(pyro->counter*4)/1024;

	dists[2]=(dists[1]*SIN(pyro->counter*4))/65536;
	heights[2]=heights[1]*0.75f;

	dists[3]=(dists[2]*SIN(pyro->counter*4))/65536;
	heights[3]=2;
#endif

/*	if (pyro->counter<192)
		fade=0;
	else
		fade=((pyro->counter-192)*4)<<24;
		*/
	fade=pyro->counter<<25;

	// draw the data
	for(pass=0;pass<sections;pass++) {
		for(sector=0;sector<iNumSectors;sector++) {
			next=sector+1;
			if (next==iNumSectors) next=0;

			thisscale=TEXSCALE*((4.0f-pass)*0.25f);
			nextscale=TEXSCALE*((3.0f-pass)*0.25f);

			POLY_transform( cx+((dists[pass]*dxs[sector])/256), cy+heights[pass], cz+((dists[pass]*dys[sector])/256), &pp[0]);
			POLY_transform( cx+((dists[pass]*dxs[next])/256),   cy+heights[pass], cz+((dists[pass]*dys[next])/256),   &pp[1]);
//			pp[0].u=dxs[sector]*thisscale;	pp[0].v=dys[sector]*thisscale;
//			pp[1].u=dxs[next  ]*thisscale;	pp[1].v=dys[next  ]*thisscale;
			pp[0].u=0.0f; pp[0].v=pass*0.40f;
			pp[1].u=1.0f; pp[1].v=pass*0.40f;

			if ((pass<sections-1)||(pass==2)) {
				POLY_transform( cx+(dists[pass+1]*dxs[sector])/256, cy+heights[pass+1], cz+(dists[pass+1]*dys[sector])/256, &pp[2]);
				pp[2].u=0.0f; pp[2].v=(pass+1)*0.40f;
			} else {
				pp[2]=mid;
			}
			pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF|fade;
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000;
			ok=pp[0].clip&pp[1].clip&pp[2].clip;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_DUSTWAVE,FALSE);
			}

			if ((pass<sections-1)||(pass==2)) {
				POLY_transform( cx+((dists[pass+1]*dxs[next])/256), cy+heights[pass+1], cz+((dists[pass+1]*dys[next])/256), &pp[0]);
//				pp[0].u=(dxs[next]*nextscale);	pp[0].v=(dys[next]*nextscale);
				pp[0].u=1; pp[0].v=(pass+1)*0.40f;
				pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+fade;
				pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000;
				if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
				{
					POLY_add_triangle(tri,POLY_PAGE_DUSTWAVE,FALSE);
				}
			}
		}
	}

	// add some shock:
//	for(sector=0;sector<DUSTWAVE_SECTORS;sector++)
	{
		static UBYTE shock_sector=0;
		DIRT_gust(pyro->thing,
			cx+((dists[2]*dxs[shock_sector])/256),
			cz+((dists[2]*dys[shock_sector])/256),
			cx+((dists[1]*dxs[shock_sector])/256),
			cz+((dists[1]*dys[shock_sector])/256)
			);
		shock_sector++;
		if (shock_sector==iNumSectors) shock_sector=0;
	}
	

}






RadPoint PYRO_defaultpoints2[32];

void PYRO_draw_explosion2(Pyro *pyro) {

	POLY_Point  pp[3];
	POLY_Point *tri[3];
	UBYTE i,j,k,b;
	SLONG ok,spec,fade[4];
	SLONG cx,cy,cz;
	RadPoint points[33];
	SLONG sc_radius;
	float subscale; // deal with it -- tex coords :P

//	subscale=(TEXSCALE2*((256-pyro->counter)>>8));
//	subscale=(256-pyro->counter);
	subscale=(170-pyro->counter);
	subscale/=32;
	subscale*=TEXSCALE2;


	// -- called only once, to init --
	if (!PYRO_defaultpoints2[0].flags) {
		SLONG height,radius;
		RadPoint *pt;

		PYRO_defaultpoints2[0].flags=1;
		pt=PYRO_defaultpoints2;
		for (i=0;i<4;i++) {

			height=SIN(i*128);
			radius=COS(i*128)>>8;

			// generate ring x,y
	
			for (j=0;j<8;j++) {
				pt->x=(radius*((SLONG)SIN(j*256)))/256;
				pt->z=(radius*((SLONG)COS(j*256)))/256;
				pt->y=height;
				pt++;
			}
	   }
	}
	// --


	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;

	sc_radius=(pyro->radius*pyro->scale)/256;


	// Throttle in a dodgy way.
	int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 8 * 4 );
	iNumFlames >>= 2;
	int iIncrement = 1;
	// Should we bin some?
	if ( iNumFlames < 6 )
	{
		iIncrement = 2;
	}

	for (i=0;i<32;i += iIncrement) {
		points[i].x=(PYRO_defaultpoints2[i].x*sc_radius)>>16;
		points[i].y=(PYRO_defaultpoints2[i].y*sc_radius)>>16;
		points[i].z=(PYRO_defaultpoints2[i].z*sc_radius)>>16;
	}
	points[32].y=(65535*sc_radius)>>16;

	spec=(255-(pyro->counter*2));
	if (spec<0) spec=0;
	spec*=0x010101;
//	fade=(pyro->counter<<24);

	if (pyro->counter>170) {
		fade[3]=SIN((pyro->counter-170)*6)>>8;

		fade[2]=fade[3]*2;
		if (fade[2]>255) fade[2]=255;

		fade[1]=fade[2]*2;
		if (fade[1]>255) fade[1]=255;

		fade[0]=fade[1]*2;
		if (fade[0]>255) fade[0]=255;

		fade[0]<<=24;
		fade[1]<<=24;
		fade[2]<<=24;
		fade[3]<<=24;
	} else fade[0]=fade[1]=fade[2]=fade[3]=0;

//	DIRT_gust(pyro->thing,cx,cz,(points[0].x/4)+cx,(points[0].z/4)+cz);
//	DIRT_gust(pyro->thing,cx,cz,(points[4].x/4)+cx,(points[4].z/4)+cz);



	// Draw bottom rungs
	for (k=0;k<3;k++) {
		b=k*8;
		for (i=b;i<b+8;i+=iIncrement) {
			j=i+iIncrement;
			if (j==b+8) j=b;
			POLY_transform(	points[i].x+cx,
							points[i].y+cy,
							points[i].z+cz,
							&pp[0]);
//			pp[0].u=points[i].x*TEXSCALE2;
//			pp[0].v=points[i].z*TEXSCALE2;
			pp[0].u=points[i].x*subscale;
			pp[0].v=points[i].z*subscale;
			POLY_transform(	points[i+8].x+cx,
							points[i+8].y+cy,
							points[i+8].z+cz,
							&pp[1]);
//			pp[1].u=points[i+8].x*TEXSCALE2;
//			pp[1].v=points[i+8].z*TEXSCALE2;
			pp[1].u=points[i+8].x*subscale;
			pp[1].v=points[i+8].z*subscale;
			POLY_transform(	points[j+8].x+cx,
							points[j+8].y+cy,
							points[j+8].z+cz,
							&pp[2]);
//			pp[2].u=points[j+8].x*TEXSCALE2;
//			pp[2].v=points[j+8].z*TEXSCALE2;
			pp[2].u=points[j+8].x*subscale;
			pp[2].v=points[j+8].z*subscale;
			pp[0].colour=0xFFFFFF+fade[k];
			pp[1].colour=pp[2].colour=0xFFFFFF+fade[k+1];
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
			ok=pp[0].clip&pp[1].clip&pp[2].clip;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
			}

/*			POLY_transform(	points[i].x+cx,
							points[i].y+cy,
							points[i].z+cz,
							&pp[0]);
			pp[0].u=points[i].x*TEXSCALE2;
			pp[0].v=points[i].z*TEXSCALE2;
			POLY_transform(	points[j+8].x+cx,
							points[j+8].y+cy,
							points[j+8].z+cz,
							&pp[1]);
			pp[1].u=points[j+8].x*TEXSCALE2;
			pp[1].v=points[j+8].z*TEXSCALE2;*/
			POLY_transform(	points[j].x+cx,
							points[j].y+cy,
							points[j].z+cz,
							&pp[1]);
//			pp[1].u=points[j].x*TEXSCALE2;
//			pp[1].v=points[j].z*TEXSCALE2;
			pp[1].u=points[j].x*subscale;
			pp[1].v=points[j].z*subscale;
			pp[1].colour=0xFFFFFF+fade[k];
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
			}
		}
	}

	// Draw top "rung"
	for (i=24;i<32;i+=iIncrement) {
		j=i+iIncrement;
		if (j==32) j=24;
		POLY_transform(	points[i].x+cx,
						points[i].y+cy,
						points[i].z+cz,
						&pp[0]);
//		pp[0].u=points[i].x*TEXSCALE2;
//		pp[0].v=points[i].z*TEXSCALE2;
		pp[0].u=points[i].x*subscale;
		pp[0].v=points[i].z*subscale;
		POLY_transform(	cx,
						points[32].y+cy,
						cz,
						&pp[1]);
		pp[1].u=0;
		pp[1].v=0;
		POLY_transform(	points[j].x+cx,
						points[j].y+cy,
						points[j].z+cz,
						&pp[2]);
//		pp[2].u=points[j].x*TEXSCALE2;
//		pp[2].v=points[j].z*TEXSCALE2;
		pp[2].u=points[j].x*subscale;
		pp[2].v=points[j].z*subscale;
		pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+fade[3];
		pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
		if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
		{
			POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
		}
	}



}



void PYRO_draw_newdome(Pyro *pyro) {

	POLY_Point  pp[3];
	POLY_Point *tri[3];
	UBYTE i,j,k,b;
	SLONG ok,spec,fade[4];
	SLONG cx,cy,cz;
	RadPoint points[33];
	SLONG sc_radius;
	float subscale; // deal with it -- tex coords :P
	float u,v;
	UBYTE iu,iv;
	ULONG store_seed;

//	subscale=(TEXSCALE2*((256-pyro->counter)>>8));
//	subscale=(256-pyro->counter);
	subscale=(170-pyro->counter);
	subscale/=32;
	subscale*=TEXSCALE2;


	// -- called only once, to init --
	if (!PYRO_defaultpoints2[0].flags) {
		SLONG height,radius;
		RadPoint *pt;

		PYRO_defaultpoints2[0].flags=1;
		pt=PYRO_defaultpoints2;
		for (i=0;i<4;i++) {

			height=SIN(i*128);
			radius=COS(i*128)>>8;
//			radius+=Random()&0x7f;

			// generate ring x,y
	
			for (j=0;j<8;j++) {
				pt->x=(radius*((SLONG)SIN(j*256)))/256;
				pt->z=(radius*((SLONG)COS(j*256)))/256;
				pt->y=height;
				pt++;
			}
	   }
	}
	// --

	store_seed=GetSeed();
	SetSeed((ULONG)pyro);


	tri[0] = &pp[0];
	tri[1] = &pp[1];
	tri[2] = &pp[2];

	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;

	sc_radius=(pyro->radius*pyro->scale)/256;

	// Throttle in a dodgy way.
	int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 8 * 4 );
	iNumFlames >>= 2;
	int iIncrement = 1;
	// Should we bin some?
	if ( iNumFlames < 6 )
	{
		iIncrement = 2;
	}



	for (i=0;i<32;i += iIncrement) {
		points[i].x=(PYRO_defaultpoints2[i].x*sc_radius)>>16;
		points[i].y=(PYRO_defaultpoints2[i].y*sc_radius)>>16;
		points[i].z=(PYRO_defaultpoints2[i].z*sc_radius)>>16;
	}
	points[32].y=(65535*sc_radius)>>16;

	spec=(255-(pyro->counter*1));
	if (spec<0) spec=0;
	spec*=0x010101;

	if (pyro->counter>170) {
		fade[3]=SIN((pyro->counter-170)*6)>>8;

		fade[2]=fade[3]*2;
		if (fade[2]>255) fade[2]=255;

		fade[1]=fade[2]*2;
		if (fade[1]>255) fade[1]=255;

		fade[0]=fade[1]*2;
		if (fade[0]>255) fade[0]=255;

		fade[0]<<=24;
		fade[1]<<=24;
		fade[2]<<=24;
		fade[3]<<=24;
	} else fade[0]=fade[1]=fade[2]=fade[3]=0;

	iu=(pyro->counter>>4);
	iv=iu>>2;
	iu=iu&3;
	u=(float)iu; v=(float)iv;
	u*=0.25f; v*=0.25f;
	SPRITE_draw_tex(cx,cy,cz,
		pyro->radius<<2,0xFFFFFF|(pyro->counter<<24),0xff000000,POLY_PAGE_EXPLODE1_ADDITIVE-(Random()&1),u,v,0.25,0.25,SPRITE_SORT_NORMAL);
	SPRITE_draw_tex(cx,cy,cz,
		pyro->radius<<3,0xFFFFFF|(pyro->counter<<24),0xff000000,POLY_PAGE_EXPLODE1_ADDITIVE-(Random()&1),u,v,0.25,0.25,SPRITE_SORT_NORMAL);

	// Draw bottom rungs
	for (k=0;k<3;k++) {
		b=k*8;
		for (i=b;i<b+8;i += iIncrement) {
			j=i+iIncrement;
			if (j==b+8) j=b;
			POLY_transform(	points[i].x+cx,
							points[i].y+cy,
							points[i].z+cz,
							&pp[0]);
			pp[0].u=points[i].x*subscale;
			pp[0].v=points[i].z*subscale;
			POLY_transform(	points[i+8].x+cx,
							points[i+8].y+cy,
							points[i+8].z+cz,
							&pp[1]);
			pp[1].u=points[i+8].x*subscale;
			pp[1].v=points[i+8].z*subscale;
			POLY_transform(	points[j+8].x+cx,
							points[j+8].y+cy,
							points[j+8].z+cz,
							&pp[2]);
			pp[2].u=points[j+8].x*subscale;
			pp[2].v=points[j+8].z*subscale;
			pp[0].colour=0xFFFFFF+fade[k];
			pp[1].colour=pp[2].colour=0xFFFFFF+fade[k+1];
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
			ok=pp[0].clip&pp[1].clip&pp[2].clip;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
			}

			if (Random()&3) {
			iu=(pyro->counter>>4);
			iv=iu>>2;
			iu=iu&3;
			u=(float)iu; v=(float)iv;
			u*=0.25f; v*=0.25f;
			SPRITE_draw_tex(points[j].x+cx,points[j].y+cy,points[j].z+cz,
				pyro->radius<<1,0xFFFFFF|(pyro->counter<<24),0xff000000,POLY_PAGE_EXPLODE1_ADDITIVE-(Random()&1),u,v,0.25,0.25,SPRITE_SORT_NORMAL);
			}

			POLY_transform(	points[j].x+cx,
							points[j].y+cy,
							points[j].z+cz,
							&pp[1]);
			pp[1].u=points[j].x*subscale;
			pp[1].v=points[j].z*subscale;
			pp[1].colour=0xFFFFFF+fade[k];
			pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
			}
		}
	}

	// Draw top "rung"
	for (i=24;i<32;i += iIncrement) {
		j=i+iIncrement;
		if (j==32) j=24;
		if (Random()&1)
			SPRITE_draw_tex(points[i].x+cx,points[i].y+cy,points[i].z+cz,
				pyro->radius<<1,0xFFFFFF|(pyro->counter<<24),0xff000000,POLY_PAGE_EXPLODE1_ADDITIVE,u,v,0.25,0.25,SPRITE_SORT_NORMAL);
		POLY_transform(	points[i].x+cx,
						points[i].y+cy,
						points[i].z+cz,
						&pp[0]);
		pp[0].u=points[i].x*subscale;
		pp[0].v=points[i].z*subscale;
		POLY_transform(	cx,
						points[32].y+cy,
						cz,
						&pp[1]);
		pp[1].u=0;
		pp[1].v=0;
		POLY_transform(	points[j].x+cx,
						points[j].y+cy,
						points[j].z+cz,
						&pp[2]);
		pp[2].u=points[j].x*subscale;
		pp[2].v=points[j].z*subscale;
		pp[0].colour=pp[1].colour=pp[2].colour=0xFFFFFF+fade[3];
		pp[0].specular=pp[1].specular=pp[2].specular=0xFF000000+spec;
		if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
		{
			POLY_add_triangle(tri,POLY_PAGE_BIGBANG,FALSE);
		}
	}

	SetSeed(store_seed);


}






void PYRO_alpha_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{
	POLY_Point p1;
	POLY_Point p2;

	POLY_transform(float(x1), float(y1), float(z1), &p1);
	POLY_transform(float(x2), float(y2), float(z2), &p2);

	if (POLY_valid_line(&p1, &p2))
	{
		p1.colour   = colour1;
		p1.specular = 0xff000000;

		p2.colour   = colour2;
		p2.specular = 0xff000000;

		POLY_add_line(&p1, &p2, float(width1), float(width2), POLY_PAGE_ALPHA, sort_to_front);
	}
}

void PYRO_draw_twanger(Pyro *pyro) {
	SLONG cx,cy,cz,c;
	SLONG dx,dy,dz,tx,ty;
	SLONG col1,col2,dir,ang;
	UBYTE i;

	cx=pyro->thing->WorldPos.X>>8;
	cy=pyro->thing->WorldPos.Y>>8;
	cz=pyro->thing->WorldPos.Z>>8;


	// Throttle in a dodgy way.
	int iNumFlames = IWouldLikeSomePyroSpritesHowManyCanIHave ( 8 );
	int iIncrement = 1;
	// Should we bin some?
	if ( iNumFlames < 6 )
	{
		iIncrement = 2;
	}


	for (i=0;i<8;i += iIncrement) {
		ang=pyro->radii[i]&0xff;
		dir=(pyro->radii[i]>>4);

		c=((COS(ang)>>8)*pyro->counter)/128;

		dx=((SIN(dir)/256)*c)/256;
		dy=0;
		dz=((COS(dir)/256)*c)/256;

		// scale
		dx=(dx*pyro->scale)/256;
		dz=(dz*pyro->scale)/256;

		if ((!pyro->tints[0])&&!pyro->tints[1]) {
			col1=0x00FFFFFF+((COS(pyro->counter*2)&0xFF00)<<16);
			col2=0x00FFFFFF-(SIN(pyro->counter*2)>>8);
		} else {
			col1=pyro->tints[0]+((COS(pyro->counter*2)&0xFF00)<<16);
			col2=pyro->tints[1];
		}

		//debugoverride
//		col1=0xFFFFFFFF;
//		col2=0x00FFFF00;

		c=256-(COS(pyro->counter*2)>>8);

		PYRO_alpha_line(cx,cy,cz,2,col1,cx+dx,cy+dy,cz+dz,c,col2,1);
	}

}


void PYRO_draw_streambit(Pyro *pyro,SLONG cx, SLONG cy, SLONG cz, SLONG c, UBYTE i) {
	SLONG x,y,z,dx,dy,dir;

	// get dir index
	dir=(pyro->radii[i+4]>>8)*16;
	// dx/dy specify steepness of streamer
	dx=SIN(pyro->radii[i+4]&0xff)/256;
	dy=COS(pyro->radii[i+4]&0xff)/256;
	y=((SIN(c*4)/256)*dy)+cy;
	// x/z handle the bearing
	c=(c*dx)/128;
	x=((SIN(dir)*c)/128)+cx;
	z=((COS(dir)*c)/128)+cz;
	PARTICLE_Add(x, y, z, 0, -2, 0, POLY_PAGE_STEAM, 1+((rand()&3)<<2), 0x888888, PFLAG_RESIZE|PFLAG_FADE, 40+(rand()&0xf), 4, 1, 5, 2);
}

void PYRO_draw_streamer(Pyro *pyro) {
	UBYTE i;
	SLONG cx,cy,cz;

	cx=pyro->thing->WorldPos.X;
	cy=pyro->thing->WorldPos.Y;
	cz=pyro->thing->WorldPos.Z;

	for (i=0;i<4;i++)
		if ((pyro->radii[i]>64)&&(pyro->radii[i]<320)) {
			PYRO_draw_streambit(pyro,cx,cy,cz,pyro->radii[i]-64,i);
			PYRO_draw_streambit(pyro,cx,cy,cz,pyro->radii[i]-60,i);
		} else
			if ((pyro->radii[i]>320)&&(pyro->radii[i]<400)) {
				pyro->radii[i]=400;
				pyro->counter--;
			}

}

void PYRO_draw_blob(Pyro *p_thing) 
{

}


void PYRO_draw_armageddon(Pyro *pyro)
{
	Thing *thing;
	GameCoord pos;
	SWORD i,j;

	if (GAMEMENU_is_paused()) return;

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

	SLONG and_1;
	SLONG and_2;

#ifndef TARGET_DC
	if (SOFTWARE)
	{
		and_1 = 7;
		and_2 = 7;
	}
	else
#endif
	{
#ifdef TARGET_DC
		// Ease off a bit.
		and_1 = 3;
		and_2 = 7;
#else
		and_1 = 2;
		and_2 = 3;
#endif
	}

	if (!(Random()&and_1))
	{
		pos=pyro->thing->WorldPos;
		pos.X+=((Random()&0xff00)-0x7f00)<<3;
		pos.Z+=((Random()&0xff00)-0x7f00)<<3;
		pos.Y=PAP_calc_map_height_at(pos.X>>8,pos.Z>>8)<<8;
		thing=PYRO_create(pos,PYRO_NEWDOME);
		if (thing)
			thing->Genus.Pyro->scale=(400+Random()&0x7f)+(pyro->counter<<1);
	}
	if (!(Random()&and_2))
	{
		SLONG flags = PFLAG_SPRITEANI | PFLAG_SPRITELOOP  | PFLAG_EXPLODE_ON_IMPACT | PFLAG_LEAVE_TRAIL;
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


/*************************************************************
 *
 *   Animals
 *
 */

#if 0

extern	void ANIM_obj_draw(Thing *p_thing,DrawTween *dt);
extern	void ANIM_obj_draw_warped(Thing *p_thing,DrawTween *dt);

void ANIMAL_draw(Thing *p_thing) 
{
	UBYTE i;
	Animal *animal=ANIMAL_get_animal(p_thing);
//	DrawTween *dt=ANIMAL_get_drawtween(animal);

	p_thing->WorldPos.Y=100<<8;
	ANIM_obj_draw(p_thing,p_thing->Draw.Tweened);

/*		FIGURE_draw_prim_tween(
			start_object + object_offset,
			(p_thing->WorldPos.X >> 8)+xd,
			(p_thing->WorldPos.Y >> 8)+yd,
			(p_thing->WorldPos.Z >> 8)+zd,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			colour,
			specular);
*/

/*
	for (i=0;i<ANIMAL_body_size(animal);i++)
		ANIM_obj_draw_warped(p_thing,dt++);*/
	
}

#endif

/*************************************************************
 *
 *   Ribbons
 *
 */

void RIBBON_draw_ribbon(Ribbon *ribbon) {
	POLY_Point  pp[3];
	POLY_Point *tri[3] = { &pp[0], &pp[1], &pp[2] };
	UBYTE i,p,ctr;
	SLONG id,spread;
	float vo,vs; // floats and what's more you'll damn well like 'em cos i said so

//	TRACE("(rib: %d %d %d\n",ribbon->Head,ribbon->Tail,ribbon->Size);
	i=ribbon->Tail;
	p=ctr=0;
	vo=ribbon->Scroll;
	vo*= 0.015625f;
	vs=ribbon->TextureV;
	vs=1.0f/vs;
	do {
		POLY_transform(ribbon->Points[i].X,	ribbon->Points[i].Y, ribbon->Points[i].Z, &pp[p]);
		if (ribbon->Flags & RIBBON_FLAG_FADE) {
			if (ribbon->Flags & RIBBON_FLAG_IALPHA)
				id=((256*ctr)/ribbon->FadePoint);
			else
				id=255-((256*ctr)/ribbon->FadePoint);
			if (id<0) id=0;
			if (id>255) id=255;
			pp[p].colour=(id<<24)|ribbon->RGB;
		} else {
			pp[p].colour=(((SLONG)ribbon->FadePoint)<<24)|ribbon->RGB;
		}
		pp[p].specular=0xFF000000;
		if (i&1) {
			pp[p].u=0;
		} else {
			pp[p].u=ribbon->TextureU;
		}
		pp[p].v=i; // heh
		pp[p].v*=vs;
		pp[p].v+=vo;
		p++; i++; ctr++;
		if (i==ribbon->Size) i=0;
		if (p==3) p=0;
		if (ctr>2) {
			if (pp[0].MaybeValid() && pp[1].MaybeValid() && pp[2].MaybeValid())
			{
				POLY_add_triangle(tri,ribbon->Page,FALSE);
			}
		}
		ASSERT(ctr<MAX_RIBBON_SIZE);
	} while (i!=ribbon->Head); // egad!

	
}

/*************************************************************
 *
 *   Light Blooms
 *
 */

// x,y,z specify light source centre
// dx,dy,dz its direction normal, scaled so 0 -> 0, 1 -> 256, or 0,0,0 for an omni light
// col is an 0x00RRGGBB colour value. Brightness does count.

extern SLONG AENG_cam_vec[3];

/*
void BLOOM_draw_floats(float x, float y, float z, float dx, float dy, float dz, SLONG col) {
	float dot;
	float cx,cy,cz;
	SLONG sz, rgba, scale;
	POLY_Point  pt1,pt2;

	cx=AENG_cam_vec[0]; cx/=65536.0;
	cy=AENG_cam_vec[1]; cy/=65536.0;
	cz=AENG_cam_vec[2]; cz/=65536.0;
	dot=(dx*cx)+(dy*cy)+(dz*cz);
//	TRACE("CamVec %d %d %d\n",AENG_cam_vec[0],AENG_cam_vec[1],AENG_cam_vec[2]);
//	TRACE("dot %f\n",dot);
	sz=((col&0xff)+((col&0xff00)>>8)+((col&0xff0000)>>16))>>2;

	// draw the glow bloom if the light is pointing towards us
	if (dot<0) {
	  rgba=-255*dot;
	  rgba<<=24;
	  rgba+=col;
//	  SPRITE_draw(x,y,z,sz,rgba,0xFF000000,POLY_PAGE_BLOOM1,SPRITE_SORT_NORMAL);
	  SPRITE_draw(x,y,z,sz,rgba,0xFF000000,POLY_PAGE_BLOOM1,SPRITE_SORT_FRONT);
	}

	// draw the flare bloom
	scale=sz<<2;
	POLY_transform(x,y,z,&pt1);
	POLY_transform(x+(dx*scale),y+(dy*scale),z+(dz*scale),&pt2);

	rgba=255-abs(255*dot);
	rgba<<=24;
	pt1.colour=col|rgba;
//	pt2.colour=col;
	pt2.colour=col|rgba;
	pt1.specular=pt2.specular=0xFF000000;

	sz*=3;

	if (POLY_valid_line(&pt1, &pt2))
		POLY_add_line_tex(&pt1, &pt2, sz>>2, sz>>1, POLY_PAGE_BLOOM2, 0);
//		POLY_add_line(&pt1, &pt2, sz>>2, sz>>1, POLY_PAGE_ALPHA, 0);

}
*/

void SPRITE_draw_rotated(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		SLONG page,
		SLONG sort,
		SLONG rotate)
{
	float screen_size;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];
	SLONG opp,adj,xofs,yofs,angle;

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid);

	if (mid.IsValid())
	{
		screen_size = POLY_world_length_to_screen(world_size) * mid.Z;

		if (mid.X + screen_size < 0 ||
			mid.X - screen_size > POLY_screen_width ||
			mid.Y + screen_size < 0 ||
			mid.Y - screen_size > POLY_screen_height)
		{
			//
			// Off screen.
			// 
		}
		else
		{
			if (rotate==0xffffff) {
				opp=(DisplayWidth>>1)-mid.X;
				adj=(DisplayHeight>>1)-mid.Y;
				angle=-Arctan(opp,adj);
				angle&=2047;
			} else angle=rotate;

			xofs=screen_size*SIN(angle);
			yofs=screen_size*COS(angle);
			xofs>>=16; yofs>>=16;

			pp[0].X = mid.X - xofs;
			pp[0].Y = mid.Y - yofs;
			pp[1].X = mid.X + yofs;
			pp[1].Y = mid.Y - xofs;
			pp[2].X = mid.X - yofs;
			pp[2].Y = mid.Y + xofs;
			pp[3].X = mid.X + xofs;
			pp[3].Y = mid.Y + yofs;

/*			pp[0].X = mid.X - screen_size;
			pp[0].Y = mid.Y - screen_size;
			pp[1].X = mid.X + screen_size;
			pp[1].Y = mid.Y - screen_size;
			pp[2].X = mid.X - screen_size;
			pp[2].Y = mid.Y + screen_size;
			pp[3].X = mid.X + screen_size;
			pp[3].Y = mid.Y + screen_size;
*/
			pp[0].u = 0;
			pp[0].v = 0;
			pp[1].u = 1;
			pp[1].v = 0;
			pp[2].u = 0;
			pp[2].v = 1;
			pp[3].u = 1;
			pp[3].v = 1;

			pp[0].colour = pp[1].colour = pp[2].colour = pp[3].colour = colour;

			pp[0].specular = pp[1].specular = pp[2].specular = pp[3].specular = 0xFF000000;

			switch(sort)
			{
				case SPRITE_SORT_NORMAL:

					mid.z -= 1.0F / 64.0F;
					mid.Z += 1.0F / 64.0F;

					if (mid.z < 0.0F)
					{
						mid.z = 0.0F;
					}

					if (mid.Z > 0.999F)
					{
						mid.Z = 0.999F;
					}

					pp[0].z = mid.z;
					pp[0].Z = mid.Z;
					pp[1].z = mid.z;
					pp[1].Z = mid.Z;
					pp[2].z = mid.z;
					pp[2].Z = mid.Z;
					pp[3].z = mid.z;
					pp[3].Z = mid.Z;
					break;

				case SPRITE_SORT_FRONT:
					pp[0].z = 0.01F;
					pp[0].Z = 1.00F;
					pp[1].z = 0.01F;
					pp[1].Z = 1.00F;
					pp[2].z = 0.01F;
					pp[2].Z = 1.00F;
					pp[3].z = 0.01F;
					pp[3].Z = 1.00F;
					break;
				
				default:
					ASSERT(0);
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, page, FALSE, TRUE);
		}
	}
}

extern SLONG AENG_cur_fc_cam;

const UBYTE flare_table[7][3] =
{
	{ 132 , 92  , 80	},
	{ 135 , 114 , 79	},
	{ 128 , 134 , 78	},
	{ 81  , 125 , 73	},
	{ 78  , 132 , 127	},
	{ 76  , 88  , 126	},
	{ 101 , 73  , 125	}

};


void BLOOM_flare_draw(SLONG x, SLONG y, SLONG z, SLONG str) {
	POLY_Point  pt1,pt2,pt3,pt4;
	SLONG dx,dy,fx,fy,cx,cy;
	SLONG i,sz,j;
	SLONG scale;
	POLY_Point *quad[4];
	SLONG fc_x, fc_y, fc_z, dummy;

	if (!EWAY_grab_camera(
		&fc_x, &fc_y, &fc_z,
		&dummy, &dummy, &dummy, &dummy))
	{
		fc_x=FC_cam[AENG_cur_fc_cam].x;
		fc_y=FC_cam[AENG_cur_fc_cam].y;
		fc_z=FC_cam[AENG_cur_fc_cam].z;
	}


	if (!there_is_a_los(x,y,z,
						fc_x>>8,fc_y>>8,fc_z>>8,
						LOS_FLAG_IGNORE_PRIMS)) return;

	POLY_transform(x,y,z,&pt1);

	if ((pt1.X<0)||(pt1.X>POLY_screen_width)||(pt1.Y<0)||(pt1.Y>POLY_screen_height)) return;

	cx=DisplayWidth>>1;
	cy=DisplayHeight>>1;
	dx=pt1.X-cx;
	dy=pt1.Y-cy;

//	dx>>=4; dy>>=4;
	dx<<=4; dy<<=4;

	pt1.z=0.01F; pt1.Z=1.00F;

	pt2=pt3=pt4=pt1;
	pt1.u=0; pt1.v=0;
	pt2.u=1; pt2.v=0;
	pt3.u=0; pt3.v=1;
	pt4.u=1; pt4.v=1;

	quad[0]=&pt1;
	quad[1]=&pt2;
	quad[2]=&pt3;
	quad[3]=&pt4;

	//
	// Darker the further the lens flare is from the camera.
	//

	{
		SLONG dx = abs(x - (fc_x >> 8));
		SLONG dy = abs(y - (fc_y >> 8));
		SLONG dz = abs(z - (fc_z >> 8));

		SLONG dist = QDIST3(dx,dy,dz);
		
		// scale = 256 - (dist * 256 / (16 * 256)); // surely that's nonsense mark?
		scale = 256 - (dist / 16);

		if (scale<0) return;

		//TRACE("flare scale: %d  str: %d\n",scale, str);

		SATURATE(scale, 0, 256);
//		SATURATE(str,   0, 255);

		scale  *= str;
		scale >>= 8;
	}
	

//	scale=str;

	//TRACE("final flare scale: %d\n",scale);
	if (!scale) return;

	for (i=-12;i<15;i++) 
	  if (i) {
		j=abs(i>>2);
		j*=i;
		fx=cx+((j*dx)>>8);
		fy=cy+((j*dy)>>8);
		sz=abs(abs(i)-3)*8;
		if (abs(i)>7) sz>>=1;
		if (abs(i)>11) sz>>=1;
		pt1.X=fx-sz; pt1.Y=fy-sz;
		pt2.X=fx+sz; pt2.Y=fy-sz;
		pt3.X=fx-sz; pt3.Y=fy+sz;
		pt4.X=fx+sz; pt4.Y=fy+sz;
		pt1.specular=pt2.specular=pt3.specular=pt4.specular=0;

		SLONG r,g,b;

		r=flare_table[(i>>2)+3][0];
		g=flare_table[(i>>2)+3][1];
		b=flare_table[(i>>2)+3][2];

		r*=scale; r>>=8;
		g*=scale; g>>=8;
		b*=scale; b>>=8;

//		j=(i>>2)+3;
//		TRACE("ival : str : r,g,b  %d : %d, %d,%d,%d\n",j, str,r,g,b);

		pt1.colour=pt2.colour=pt3.colour=pt4.colour= r | (g<<8) | (b<<16);

/*		  //(str<<24)|
		  ( flare_table[(i>>2)+4][0] ) |
		  ( flare_table[(i>>2)+4][1] <<  8) |
		  ( flare_table[(i>>2)+4][2] << 16);*/

/*		  ((flare_table[(i>>2)+4][0] * scale >> 8) <<  0) |
		  ((flare_table[(i>>2)+4][1] * scale >> 8) <<  8) |
		  ((flare_table[(i>>2)+4][2] * scale >> 8) << 16);*/

		if (POLY_valid_quad(quad))
			POLY_add_quad(quad, POLY_PAGE_LENSFLARE, FALSE, true);
	  }

	
}

void BLOOM_draw(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG col, UBYTE opts) {
	SLONG a,b,c,dot;
	SLONG rgba,sz;
//	POLY_Point  pp[4],pt1,pt2;
	POLY_Point  pt1,pt2;
//	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };
	SLONG fc_x, fc_y, fc_z, dummy;

	POLY_flush_local_rot();

	if (!EWAY_grab_camera(
		&fc_x, &fc_y, &fc_z,
		&dummy, &dummy, &dummy, &dummy))
	{
		fc_x=FC_cam[AENG_cur_fc_cam].x;
		fc_y=FC_cam[AENG_cur_fc_cam].y;
		fc_z=FC_cam[AENG_cur_fc_cam].z;
	}

	// check LOS against wall
	SLONG	losy = (opts & BLOOM_RAISE_LOS) ? y + 16 : y;
	if (!there_is_a_los(x,losy,z,
						fc_x>>8,fc_y>>8,fc_z>>8,
						LOS_FLAG_IGNORE_PRIMS)) return;

	if ((!dx)&&(!dy)&&(!dz)) 
		dot=-255;
	else {
		// first order of the day: calculate the dot product of the light and view normal
		a=(dx*AENG_cam_vec[0])/65536;
		b=(dy*AENG_cam_vec[1])/65536;
		c=(dz*AENG_cam_vec[2])/65536;
		dot = a+b+c;
	}

	sz=((col&0xff)+((col&0xff00)>>8)+((col&0xff0000)>>16))>>2;

	// draw the "glow bloom" if the light is pointing towards us
	if ((dot<0)||(opts&BLOOM_GLOW_ALWAYS)) {
	  rgba=abs(dot);
	  rgba<<=24;
	  rgba|=col;
//	  SPRITE_draw((float)x,(float)y,(float)z,sz<<1,rgba,0xFF000000,POLY_PAGE_BLOOM1,SPRITE_SORT_NORMAL);
	  SPRITE_draw_rotated((float)x,(float)y,(float)z,sz<<2,rgba,POLY_PAGE_BLOOM1,SPRITE_SORT_NORMAL,0xffffff);

	  // lil bit o lens flare
	  if (opts&BLOOM_LENSFLARE) {
	    rgba=abs(dot);
	    rgba>>=1;
		if (opts&BLOOM_FLENSFLARE) rgba>>=1;
	    BLOOM_flare_draw(x,y,z,rgba);
	  }
	}

	if (opts&BLOOM_BEAM) {

		// scale the flare
		dx*=sz; dy*=sz; dz*=sz;
		dx>>=5; dy>>=5; dz>>=5;

		// draw the "flare bloom" reaching out
		POLY_transform(x,y,z,&pt1);
		POLY_transform(x+dx,y+dy,z+dz,&pt2);

		rgba=255-abs(dot);
		rgba<<=24;
		pt1.colour=col|rgba;
		pt2.colour=col;
		pt1.specular=pt2.specular=0xFF000000;

		sz*=3;
		
		if (POLY_valid_line(&pt1, &pt2))
			POLY_add_line_tex(&pt1, &pt2, sz>>2, sz, POLY_PAGE_BLOOM2, 0);
	}

}

/*************************************************************
 *
 *   "Specials" that have their own extra draw stuff
 *
 */


void DRAWXTRA_Special(Thing *p_thing) {
	SLONG dx,dz,c0, flags=0;

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
		flags=BLOOM_BEAM;
		// FALL THRU
	//case SPECIAL_GRENADE:
		if (p_thing->SubState == SPECIAL_SUBSTATE_ACTIVATED)
		{
		  c0=p_thing->Genus.Special->timer;
		  c0=(c0<<3)&2047;
		  dx=SIN(c0)>>8;
		  dz=COS(c0)>>8;
		  BLOOM_draw(p_thing->WorldPos.X>>8,(p_thing->WorldPos.Y>>8)+25,p_thing->WorldPos.Z>>8,
			dx,0,dz,0x007F5D,flags);
		}
		break;

		//  no default -- not all specials have extra stuff.
	}
}

/*************************************************************
 *
 *   DRAW2D -- some utils for the Widgets library
 *   hardly worth sticking here except to keep PC-specific stuff out of widgets
 *
 */

void DRAW2D_Box(SLONG x, SLONG y, SLONG ox, SLONG oy, SLONG rgb, UBYTE flag, UBYTE depth) {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };
//	SLONG page = flag ? POLY_PAGE_SUBTRACTIVE : POLY_PAGE_ALPHA;
	SLONG page = flag ? POLY_PAGE_SUBTRACTIVEALPHA : POLY_PAGE_ADDITIVEALPHA;
	float fdepth=(float)depth;

	pp[0].colour=rgb; pp[0].specular=0;
	pp[0].Z=fdepth/256.0f;

	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=x;	pp[0].Y=y;
	pp[1].X=ox;	pp[1].Y=y;
	pp[2].X=x;	pp[2].Y=oy;
	pp[3].X=ox;	pp[3].Y=oy;
	
	POLY_add_quad(quad,page,FALSE,TRUE);

}

void DRAW2D_Box_Page(SLONG x, SLONG y, SLONG ox, SLONG oy, SLONG rgb, SLONG page, UBYTE depth) {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };
	float fdepth=(float)depth;

	pp[0].colour=rgb; pp[0].specular=0;
	pp[0].Z=fdepth/256.0f;

	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=x;	pp[0].Y=y;
	pp[1].X=ox;	pp[1].Y=y;
	pp[2].X=x;	pp[2].Y=oy;
	pp[3].X=ox;	pp[3].Y=oy;
	
	POLY_add_quad(quad,page,FALSE,TRUE);

}

void DRAW2D_Tri(SLONG x, SLONG y, SLONG ox, SLONG oy, SLONG tx, SLONG ty, SLONG rgb, UBYTE flag) {
	POLY_Point  pp[3];
	POLY_Point *tri[3] = { &pp[0], &pp[1], &pp[2] };
	SLONG page = flag ? POLY_PAGE_SUBTRACTIVEALPHA : POLY_PAGE_ADDITIVEALPHA;

	pp[0].colour=rgb; pp[0].specular=0;
	pp[0].Z=0.5f;

	pp[1]=pp[2]=pp[0];

	pp[0].X=x;	pp[0].Y=y;
	pp[1].X=ox;	pp[1].Y=oy;
	pp[2].X=tx;	pp[2].Y=ty;
	
	POLY_add_triangle(tri,page,FALSE,TRUE);
}

void DRAW2D_Sprite(SLONG x, SLONG y, SLONG ox, SLONG oy, float u, float v, float ou, float ov, SLONG page, SLONG rgb) {
	POLY_Point  pp[4];
	POLY_Point *quad[4] = { &pp[0], &pp[1], &pp[2], &pp[3] };

	pp[0].colour=rgb; pp[0].specular=0;
	pp[0].Z=0.5f;

	pp[1]=pp[2]=pp[3]=pp[0];

	pp[0].X=x;	pp[0].Y=y;	pp[0].u=u;	pp[0].v=v;
	pp[1].X=ox;	pp[1].Y=y;	pp[1].u=ou;	pp[1].v=v;
	pp[2].X=x;	pp[2].Y=oy;	pp[2].u=u;	pp[2].v=ov;
	pp[3].X=ox;	pp[3].Y=oy;	pp[3].u=ou;	pp[3].v=ov;
	
	POLY_add_quad(quad,page,FALSE,TRUE);
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
 *	     I put a comment into the header file instead.
 *
 *   No, you don't understand. The banners are so you can easily
 *   find where one section (eg MIBs) ends and the next section
 *   (eg final_glow) begins. Comments in the header are no use
 *   whatsoever for this.
 *
 *       I rarely use 'banners' in my code.
 *
 *   I noticed.
 */


void DRAWXTRA_final_glow(SLONG x, SLONG y, SLONG z, UBYTE fade)
{
	static SLONG rotation = 0;

	POLY_Point mid;

	// Internal DC gubbins.
	POLY_flush_local_rot();

	POLY_transform(
		float(x),
		float(y),
		float(z),
	   &mid);

	if (!(mid.clip & POLY_CLIP_TRANSFORMED))
	{
		return;
	}

	rotation += 10 * TICK_RATIO >> TICK_SHIFT;

	//
	// Push forward in the z-buffer a bit...
	//

	mid.z -= 1.0F / 22.0F;

	if (mid.z < POLY_ZCLIP_PLANE)
	{
		mid.z = POLY_ZCLIP_PLANE;
	}

	mid.Z = POLY_ZCLIP_PLANE / mid.z;

	//
	// Draw overlays rotated sprites.
	//

	SLONG i;
	SLONG j;
	SLONG angle;
	SLONG colour;

	float dx;
	float dy;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	colour  = fade * 0x60 >> 8;
	colour |= colour << 8;
	colour |= colour << 8;

	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = colour;
	pp[0].specular = 0x00000000;
	pp[0].Z        = mid.Z;
	pp[0].z        = mid.z;

	pp[1].u        = 1.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = colour;
	pp[1].specular = 0x00000000;
	pp[1].Z        = mid.Z;
	pp[1].z        = mid.z;

	pp[2].u        = 0.0F;
	pp[2].v        = 1.0F;
	pp[2].colour   = colour;
	pp[2].specular = 0x00000000;
	pp[2].Z        = mid.Z;
	pp[2].z        = mid.z;

	pp[3].u        = 1.0F;
	pp[3].v        = 1.0F;
	pp[3].colour   = colour;
	pp[3].specular = 0x00000000;
	pp[3].Z        = mid.Z;
	pp[3].z        = mid.z;

	POLY_fadeout_point(&pp[0]);
	POLY_fadeout_point(&pp[1]);
	POLY_fadeout_point(&pp[2]);
	POLY_fadeout_point(&pp[3]);

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	for (i = 0; i < 4; i++)
	{
		switch(i)
		{
			case 0: angle =  rotation >> 0; break;
			case 1: angle = -rotation >> 0; break;
			case 2: angle =  rotation >> 1; break;
			case 3: angle = -rotation >> 1; break;
		}

		angle += i << 6;
		angle += i << 3;
		angle &= 2047;

		dx = float(SIN(angle)) * mid.Z * (1.0F / 32.0F);
		dy = float(COS(angle)) * mid.Z * (1.0F / 32.0F);
	
		for (j = 0; j < 4; j++)
		{
			if (j & 1)
			{
				pp[j].X = mid.X + dx;
				pp[j].Y = mid.Y + dy;
			}
			else
			{
				pp[j].X = mid.X - dx;
				pp[j].Y = mid.Y - dy;
			}

			if (j & 2)
			{
				pp[j].X += -dy;
				pp[j].Y += +dx;
			}
			else
			{
				pp[j].X -= -dy;
				pp[j].Y -= +dx;
			}
		}

		POLY_add_quad(quad, POLY_PAGE_FINALGLOW, FALSE, TRUE);
	}
}
