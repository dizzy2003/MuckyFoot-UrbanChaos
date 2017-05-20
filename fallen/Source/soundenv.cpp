//
//  soundenv.cpp
//
//  Matthew Rosenfeld    16th Dec 98 
//
//  Sound Environment -- basically, creating 3d polys for the 3d sound
//

#include "MFStdLib.h"
#include "game.h"
#include "memory.h"
#include "soundenv.h"
#include "pap.h"
#include "sound.h"

typedef void (*SOUNDENV_CB_fn)(SLONG,SLONG,SLONG,SLONG);


struct AudioGroundQuad {
	SLONG x,y,ox,oy;
};

AudioGroundQuad SOUNDENV_gndquads[64];
int SOUNDENV_gndctr;


//SLONG SOUNDENV_gndlist;

//------------------------------------------------------------------------
//   Workingsing stuff
//------------------------------------------------------------------------


BOOL SOUNDENV_ClearRange(CBYTE map[128][128], SLONG min, SLONG max, SLONG y) {
  SLONG x;
  
  for (x=min;x<=max;x++) {
    if (map[x][y]) return 0;
  }
  for (x=min;x<=max;x++) {

    map[x][y]|=2;
  }
  return 1;
}


void SOUNDENV_Quadify(CBYTE map[128][128], SLONG mx, SLONG my, SLONG mox, SLONG moy, SOUNDENV_CB_fn CBfn) {

  SLONG x,y,blx,bly,blox,bloy;

  for (y=my;y<moy;y++) {
    for (x=mx;x<mox;x++) {
      if (!map[x][y]) {
        blx=blox=x; bly=bloy=y;
        while ((blox<mox)&&!map[blox][y]) {
          map[blox][y]|=2;
          blox++;
        }
        blox--;
        while ((bloy<moy-1)&&SOUNDENV_ClearRange(map,blx,blox,bloy+1)) bloy++;
        CBfn(blx,bly,blox,bloy);
      }
    }
  }

  for (y=my;y<moy;y++)
    for (x=mx;x<mox;x++)
      map[x][y]&=1;
}


//------------------------------------------------------------------------
//   Interfacing stuff
//------------------------------------------------------------------------

void cback(SLONG x, SLONG y, SLONG ox, SLONG oy) {
  SOUNDENV_gndquads[SOUNDENV_gndctr].x=x<<16;
  SOUNDENV_gndquads[SOUNDENV_gndctr].y=y<<16;
  SOUNDENV_gndquads[SOUNDENV_gndctr].ox=(ox+1)<<16;
  SOUNDENV_gndquads[SOUNDENV_gndctr].oy=(oy+1)<<16;
  SOUNDENV_gndctr++;
}

void SOUNDENV_precalc(void) {
  CBYTE tempmap[128][128];
  SLONG x,y;
/*
  SOUNDENV_gndctr=0;
  for (x=0;x<128;x++)
	  for (y=0;y<128;y++) {
		  tempmap[x][y]=(PAP_2HI(x,y).Flags & PAP_FLAG_SEWER_SQUARE) ? 1 : 0;
	  }
  SOUNDENV_Quadify(tempmap,0,0,128,128,cback);
  */
}

extern SLONG	CAM_pos_x,
				CAM_pos_y,
				CAM_pos_z;

void SOUNDENV_upload(void) {

#ifdef USE_A3D
/*
	SLONG i,cx,cz;

	A3DGTag(1); // tag 1 is always going to be the ground.
//	A3DGMaterial(A3D_MAT_CARPET);
	A3DGMaterial(A3D_MAT_SNDPROOF);

	for (i=0;i<SOUNDENV_gndctr;i++) {
		A3DGQuad2D(SOUNDENV_gndquads[i].x,SOUNDENV_gndquads[i].y,SOUNDENV_gndquads[i].ox,SOUNDENV_gndquads[i].oy,-256);
	}

*/

#endif

}


