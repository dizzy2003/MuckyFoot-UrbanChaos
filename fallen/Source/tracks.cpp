//
// tracks.cpp
// tyre tracks, footprints...
// 22 sept 98
//

#include "game.h"
#include "tracks.h"
#include "structs.h"
#ifndef PSX
#include "C:\fallen\DDEngine\Headers\poly.h"
#else
#include "c:\fallen\psxeng\headers\poly.h"
#endif
#include "person.h"
#include "puddle.h"
#include "pap.h"
#include "interact.h"
#include "animate.h"
#include "sound.h"

#include "memory.h"
//...debug...
#include "dirt.h"

#define TRACK_WATER_COLOUR  0x203D60
#define TRACK_MUDDY_COLOUR  0x482000
#define TRACK_ONSNOW_COLOUR 0x000000


Track	*tracks;//[TRACK_BUFFER_LENGTH];
UWORD	track_head,track_tail,track_eob; //stopped them being pointers by MikeD

void TRACKS_InitOnce(SWORD size) 
{
	track_eob=size; //&tracks[TRACK_BUFFER_LENGTH]; // yes, intentionally one past the end of the buffer
	track_head=track_tail=0;//tracks;
	memset((UBYTE*)tracks,0,sizeof(Track)*size);
}

void TRACKS_Reset(SWORD size) 
{
	while (track_tail!=track_head) 
	{
	  remove_thing_from_map(TO_THING(TO_TRACK(track_tail)->thing));
	  free_thing(TO_THING(TO_TRACK(track_tail)->thing));
	  track_tail++;
	  if (track_tail==track_eob) track_tail=0;//tracks;
	}
    TRACKS_InitOnce(size);
}

inline void RShift8(SLONG &x, SLONG &y, SLONG &z) {
  x>>=8; y>>=8; z>>=8;
}



/*
void TRACKS_Draw() {
	Track *walk;

	walk=track_tail;
	while (walk!=track_head) {
		TRACKS_DrawTrack(TO_THING(walk->thing));

		walk++;
		if (walk==track_eob) walk=tracks;
	}
}
*/
// Figure out the offsets given the width
void TRACKS_CalcDiffs(Track &track, UBYTE width) {
/*  SLONG x,z,sf;
  ULONG ux,uz,f;

//  x=(track.dz)*256; z=-(track.dx)*256;
  x=(track.dz); z=-(track.dx);
  //TRACE("track calc x: %d  y: %d\n",x,z);
  ux=abs(x); uz=abs(z);
  ux*=ux; uz*=uz;
  f=ux+uz;
  f=Root(f); if (!f) f=1;
  sf=f;
  TRACE("track -- x: %d  z: %d  f: %d  ux: %d  uz: %d\n",x,z,f,ux,uz);
  x*=width; z*=width;
  x/=sf; z/=sf;
  TRACE("result: x: %d   z: %d\n",x,z);
  track.sx=x; track.sz=z;*/

  SLONG x,z,f;

	x=(track.dz); z=-(track.dx);
  f=Root((x*x)+(z*z));
  if (!f) f=1;
  x*=width; z*=width; x/=f; z/=f; 
  track.sx=x; track.sz=z;

//  TRACE("result: x: %d   z: %d\n",x,z);

}


// Add a track unit supplying exact parameters one by one
void TRACKS_AddQuad(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, SLONG page, SLONG colour, UBYTE width, UBYTE flip, UBYTE flags) {
	Track track;
//	THING_INDEX t_index;
	Thing *thing;

	thing = alloc_thing(CLASS_TRACK);
	if (thing) {
//		thing=TO_THING(t_index);

		track.thing=THING_NUMBER(thing);
		thing->Class = CLASS_TRACK;
		thing->WorldPos.X=x;
		thing->WorldPos.Y=y;
		thing->WorldPos.Z=z;
		thing->DrawType=DT_TRACK;
		thing->Flags=0;
		add_thing_to_map(thing);
		track.dx=dx; track.dy=dy; track.dz=dz;
		track.page=page; track.colour=colour; track.flip=flip; //track.width=width; 
		track.flags=flags; track.splut=0; track.splutmax=1+(width>>1);
		TRACKS_CalcDiffs(track,width);
		TRACKS_AddTrack(track);
	}
}

// Add a track unit supplying a completed track entry
void TRACKS_AddTrack(Track &track) 
{
  *(TO_TRACK(track_head))=track;

  // debug -- back up x/y/z
//  track_head->x=track_head->thing->WorldPos.X;
//  track_head->y=track_head->thing->WorldPos.Y;
//  track_head->z=track_head->thing->WorldPos.Z;

  // hee hee //ho ho
  TO_THING(TO_TRACK(track_head)->thing)->Genus.Track=TO_TRACK(track_head);

  track_head++;
  if (track_head==track_eob) track_head=0; //tracks;
  if (track_head==track_tail) 
  {
//	  TRACE("stomp\n");
      // stomping one, so free it's thing first (woof)
	  remove_thing_from_map(TO_THING(TO_TRACK(track_tail)->thing));
	  free_thing(TO_THING(TO_TRACK(track_tail)->thing));
	  track_tail++;
	  if (track_tail==track_eob) track_tail=0; //tracks;
  }
}

// Add a track unit "intelligently" supplying coordinates and a type of track
UWORD TRACKS_Add(SLONG x, SLONG y, SLONG z, SLONG dx, SLONG dy, SLONG dz, UBYTE type, UWORD last) {
	UBYTE age=last>>8;
	UBYTE lastkind=last&0xff;
	SLONG code,kind,page,colour;
	CBYTE msg[20];
	
	switch (type) {
	case TRACK_TYPE_TYRE_SKID:
//		TRACKS_AddQuad(x, y, z, dx, dy, dz, POLY_PAGE_TYRESKID, 0x00ffffff, 10, 0, TRACK_FLAGS_INVALPHA);
		TRACKS_AddQuad(x, y, z, dx, dy, dz, POLY_PAGE_TYRESKID, 0x00ffffff, 10, 0, 0);
		break;
	case TRACK_TYPE_TYRE: // muddy tyres (not skidmarks)
		if ((!dx)&&(!dy)&&(!dz)) return last;
		if (SDIST2(dx,dz)>25) {
			code=TRACKS_GroundAtXZ(x,z);
			switch(code) {
			case PERSON_ON_WATER:
				kind=TRACK_SURFACE_WATER;
				break;
			case PERSON_ON_GRAVEL:
			case PERSON_ON_GRASS:
				kind=TRACK_SURFACE_MUDDY;
				break;
			default:
				kind=TRACK_SURFACE_NONE;
			}
			if (lastkind!=TRACK_SURFACE_NONE) {
				page=POLY_PAGE_TYRETRACK;
				switch (lastkind) {
				case TRACK_SURFACE_MUDDY:
					colour=TRACK_MUDDY_COLOUR;
					break;
				case TRACK_SURFACE_WATER:
					colour=TRACK_WATER_COLOUR;
					break;
				}
				colour+=((255-age)<<24);
				TRACKS_AddQuad(x, y, z, dx, dy, dz, page, colour, 10, 0, 0);
			}
		}
		// fade or renew tracks
		//
		if (kind==TRACK_SURFACE_NONE) {
			if (age>8) {
				age-=8;
			} else {
				lastkind=TRACK_SURFACE_NONE;
				age=0;
			}
		} else {
			age=255;
			lastkind=kind;
		}
		break;
	case TRACK_TYPE_LEFT_PRINT:
	case TRACK_TYPE_RIGHT_PRINT:
		if (world_type==WORLD_TYPE_SNOW)
			kind=TRACK_SURFACE_ONSNOW;
		else
		{
			code=TRACKS_GroundAtXZ(x,z);
			switch(code) {
			case PERSON_ON_WATER:
				kind=TRACK_SURFACE_WATER;
				break;
			case PERSON_ON_GRAVEL:
			case PERSON_ON_GRASS:
				kind=TRACK_SURFACE_MUDDY;
				break;
			default:
				kind=TRACK_SURFACE_NONE;
			}
		}


		//
		// do combination of kind/lastkind;
		//
		if (lastkind!=TRACK_SURFACE_NONE) {
//			page=POLY_PAGE_FLAMES;
			page=POLY_PAGE_FOOTPRINT;
			switch (lastkind) {
			case TRACK_SURFACE_MUDDY:
				colour=TRACK_MUDDY_COLOUR;
				break;
			case TRACK_SURFACE_WATER:
				colour=TRACK_WATER_COLOUR;
				break;
			case TRACK_SURFACE_ONSNOW:
				colour=TRACK_ONSNOW_COLOUR;
			}
			colour+=((255-age)<<24);
			TRACKS_AddQuad(x, y, z, dx, dy, dz, page, colour, 10, (type==TRACK_TYPE_RIGHT_PRINT), TRACK_FLAGS_FLIPABLE);
		}
		// fade or renew tracks
		//
		if (kind==TRACK_SURFACE_NONE) {
			if (age>8) {
				age-=8;
			} else {
				lastkind=TRACK_SURFACE_NONE;
				age=0;
			}
		} else {
			age=255;
			lastkind=kind;
		}
		break;
	}


	last=(age<<8)+lastkind;
	return last; 
}



SLONG TRACKS_GroundAtXZ(SLONG X, SLONG Z) {
	//
	// Standing in a puddle?
	// 
#ifndef PSX
#ifndef TARGET_DC
	if (PUDDLE_in(X >> 8,Z >> 8))  return PERSON_ON_WATER;
#endif
#endif
	//
	// Check for special floor textures...
	//

	SLONG mx = X >> 16;
	SLONG mz = Z >> 16;

	if (WITHIN(mx, 0, MAP_WIDTH  - 1) &&
		WITHIN(mz, 0, MAP_HEIGHT - 1))
	{
		SLONG page = PAP_2HI(mx,mz).Texture & 0x3ff;

		if (page == 65 ||
			page == 66 ||
			page == 143)
		{
			return PERSON_ON_WOOD;
		}

		if (page >= 69 && page <= 74)
		{
			return PERSON_ON_GRASS;
		}

		if (page == 68 || (page >= 106 && page <= 111))
		{
			return PERSON_ON_GRAVEL;
		}
	}

	return PERSON_ON_DUNNO;
}


void TRACKS_Bleed(Thing *bleeder) {
	
//#ifdef VERSION_GERMAN
	if(!VIOLENCE)
		return; 
//#endif

	UBYTE sz=1+(rand()&0x1f);
	UBYTE u=(Random()&1)?SUB_OBJECT_LEFT_FOOT:SUB_OBJECT_RIGHT_FOOT;
	SLONG dx, dr, dz,x,y,z;
	dr=rand()&2047;
	dx=((SIN(dr)>>8)*sz)>>8;
	dz=((COS(dr)>>8)*sz)>>8;
	if ((dx==0)&&(dz==0)) dz=1;

	calc_sub_objects_position(
			bleeder,
			bleeder->Draw.Tweened->AnimTween,
			u,
		   &x,
		   &y,
		   &z);

	x<<=8; z<<=8;
	x+=bleeder->WorldPos.X;
	z+=bleeder->WorldPos.Z;
	x+=(rand()&0247)-1024; z+=(rand()&2047)-1024;
	y=(PAP_calc_map_height_at(x>>8,z>>8)<<8)+257;
	TRACKS_AddQuad(x, y, z, dx, 0, dz, POLY_PAGE_BLOODSPLAT, 0x00ffffff, sz, 0, TRACK_FLAGS_SPLUTTING);
}

void TRACKS_Bloodpool(Thing *bleeder) {

//#ifdef VERSION_GERMAN
	if(!VIOLENCE)
	 return; 
//#endif

	UBYTE sz=80+(rand()&0x1f);
	SLONG dx, dr, dz,x,y,z;
	dr=rand()&2047;
	dx=((SIN(dr)>>8)*sz)>>8;
	dz=((COS(dr)>>8)*sz)>>8;
	if ((dx==0)&&(dz==0)) dz=1;

	calc_sub_objects_position(
			bleeder,
			bleeder->Draw.Tweened->AnimTween,
			SUB_OBJECT_PELVIS,
		   &x,
		   &y,
		   &z);

	x<<=8; z<<=8;
	x+=bleeder->WorldPos.X;
	z+=bleeder->WorldPos.Z;

	x-=(dx/2); z-=(dz/2);

	y=(PAP_calc_height_at_thing(bleeder,x>>8,z>>8)<<8)+257;
	TRACKS_AddQuad(x, y, z, dx, 0, dz, POLY_PAGE_BLOODSPLAT, 0x00ffffff, sz, 0, TRACK_FLAGS_SPLUTTING);
}
