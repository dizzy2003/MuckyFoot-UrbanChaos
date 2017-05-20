/************************************************************
 *
 *   flamengine.cpp
 *   2D flame (and other fx) engine
 *
 */

#ifndef TARGET_DC


#include "flamengine.h"
#include "texture.h"
#include "poly.h"
#include	<DDLib.h>

extern D3DTexture TEXTURE_texture[];

// override -- replaces convection/blur combination with feedback routine

// #define FEEDBACK

//
// Part The First : Texture lock/unlock/update stuff
// blagged from texture86 stuff
//

SLONG TEXTURE_flame_lock()
{
	HRESULT res;

	res = TEXTURE_texture[TEXTURE_page_menuflame].LockUser(
			&TEXTURE_shadow_bitmap,
			&TEXTURE_shadow_pitch);

	if (FAILED(res))
	{
		TEXTURE_shadow_bitmap = NULL;
		TEXTURE_shadow_pitch  = 0;

		return FALSE;
	}
	else
	{
		TEXTURE_shadow_mask_red	   = TEXTURE_texture[TEXTURE_page_menuflame].mask_red;
		TEXTURE_shadow_mask_green  = TEXTURE_texture[TEXTURE_page_menuflame].mask_green;
		TEXTURE_shadow_mask_blue   = TEXTURE_texture[TEXTURE_page_menuflame].mask_blue;
		TEXTURE_shadow_mask_alpha  = TEXTURE_texture[TEXTURE_page_menuflame].mask_alpha;
		TEXTURE_shadow_shift_red   = TEXTURE_texture[TEXTURE_page_menuflame].shift_red;
		TEXTURE_shadow_shift_green = TEXTURE_texture[TEXTURE_page_menuflame].shift_green;
		TEXTURE_shadow_shift_blue  = TEXTURE_texture[TEXTURE_page_menuflame].shift_blue;
		TEXTURE_shadow_shift_alpha = TEXTURE_texture[TEXTURE_page_menuflame].shift_alpha;

		return TRUE;
	}
}

void TEXTURE_flame_unlock()
{
	TEXTURE_texture[TEXTURE_page_menuflame].UnlockUser();
}

void TEXTURE_flame_update()
{
}

//
// Part The ... er ... First And A Half
//

int FlameRand(int max) {
	return rand()%max;
}


//
// Part The Second : The actual flame engine stuff
//




Flamengine::Flamengine(char *fname) {
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	//int c,d,p,q,v;
	//UBYTE *pt;
	UWORD ver;

//	handle=FileOpen("data\\testfire.pal");
	handle=FileOpen(fname);

	if(handle!=FILE_OPEN_ERROR)	{
		FileRead(handle,(UBYTE*)&ver,sizeof(ver));
//		FileRead(handle,(UBYTE*)&params,sizeof(params));
		// crappy version
		ReadHeader(handle);
		ReadParts(handle);
		FileClose(handle);
	}

/*	// flip palette end-from-end
	for (c=0,d=255;c<128;c++,d--) {
      p=c*3; q=d*3;
	  v=palette[p]; palette[p]=palette[q]; palette[q]=v; p++; q++;
	  v=palette[p]; palette[p]=palette[q]; palette[q]=v; p++; q++;
	  v=palette[p]; palette[p]=palette[q]; palette[q]=v; p++; q++;      
	}*/

	memset(data,0,256*256);
	memset(work,0,256*256);
	
}

Flamengine::~Flamengine() {
}

void Flamengine::ReadHeader(MFFileHandle handle) {
	UBYTE skip;

	FileRead(handle,(UBYTE*)&params.blur,1);
	FileRead(handle,(UBYTE*)&params.dark,1);
	FileRead(handle,(UBYTE*)&params.convec,1);
	FileRead(handle,(UBYTE*)&params.palette,768);

	FileRead(handle,(UBYTE*)&skip,1);

	FileRead(handle,(UBYTE*)&params.free,2);
	FileRead(handle,(UBYTE*)&params.posn,2);
}

void Flamengine::ReadParts(MFFileHandle handle) {
	int i;
	FlameParticle *pp;
	SLONG skip;

    for (i=2000,pp=params.particles;i;i--,pp++) {
	  FileRead(handle,(UBYTE*)&pp->pos.loc.x,1);
	  FileRead(handle,(UBYTE*)&pp->pos.loc.y,1);
	  FileRead(handle,(UBYTE*)&pp->jx,1);
	  FileRead(handle,(UBYTE*)&pp->jy,1);
	  FileRead(handle,(UBYTE*)&pp->ex,1);
	  FileRead(handle,(UBYTE*)&pp->ey,1);
	  FileRead(handle,(UBYTE*)&pp->life,1);

   	  FileRead(handle,(UBYTE*)&skip,1);

	  FileRead(handle,(UBYTE*)&pp->pulse,2);
	  FileRead(handle,(UBYTE*)&pp->prate,1);
	  FileRead(handle,(UBYTE*)&pp->pmode,1);
	  FileRead(handle,(UBYTE*)&pp->wmode,1);

   	  FileRead(handle,(UBYTE*)&skip,3);

	  FileRead(handle,(UBYTE*)&pp->pstart,4);
	  FileRead(handle,(UBYTE*)&pp->pend,4);
	}
}


void Flamengine::Run() {

#ifdef FEEDBACK

	AddParticles2();
	Feedback();

#else

	AddParticles();
	if (params.blur)
	{
		if (!params.randomize)
			ConvectionBlur();
		else
			ConvectionBlur2();
	}
	else
		Darkening();
	UpdateTexture();

#endif


}

//
//   Sparkly bits
//

void Flamengine::AddParticles() {
	int i;
	//UWORD x,y;
	SWORD si;
	UBYTE *pt;
	FlameXY pos;
	FlameParticle *pp;

/* generic bonfire
	for (i=100;i;i--) {
		x=rand()&0xff;
		y=239+(rand()&0xf);
		x+=y<<8;
		data[x]=255;
	}
*/
	for (i=2000,pp=params.particles;i;i--,pp++)
		if (pp->life) {
			pos.ofs=pp->pos.ofs;
			if (pp->jx) pos.loc.x+=FlameRand(pp->jx)-(pp->jx>>1);
			if (pp->jy) pos.loc.y+=FlameRand(pp->jy)-(pp->jy>>1);
			pt=data;
			pt+=pos.ofs;
			*pt=(UBYTE)pp->pulse;
			switch (pp->pmode) {
			case 0: // Ramp up
				pp->pulse+=pp->prate;
				if ((ULONG)pp->pulse>=pp->pend) pp->pulse=(SWORD)pp->pstart;
				break;
			case 1: // Ramp down
				pp->pulse-=pp->prate;
				if ((ULONG)pp->pulse<=pp->pstart) pp->pulse=(SWORD)pp->pend;
				break;
			case 2: // Cycle (up phase)
				si=pp->pulse+pp->prate;
				if (si>(SWORD)pp->pend) {
					si=(SWORD)pp->pend;
					pp->pmode=3;
				}
				pp->pulse=si;
				break;
			case 3: // Cycle (down phase)
				si=pp->pulse-pp->prate;
				if (si<(SWORD)pp->pstart) {
					si=(SWORD)pp->pstart;
					pp->pmode=2;
				}
				pp->pulse=si;
				break;
			}

			switch (pp->wmode) {
			case 1: // fountain
				switch (i&3) {
				case 0:
					if (pp->pos.loc.x>1) pp->pos.loc.x-=2;
					break;
				case 1:
					if (pp->pos.loc.x>0) pp->pos.loc.x--;
					break;
				case 2:
					if (pp->pos.loc.x<254) pp->pos.loc.x+=2;
					break;
				case 3:
					if (pp->pos.loc.x<255) pp->pos.loc.x++;
					break;
				}
				pp->pos.loc.y+=pp->life-10;
				pp->life++;
				if (pp->life>50) {
					pp->life=1;
					pp->pos.loc.x=pp->ex;
					pp->pos.loc.y=pp->ey;
				}
				break;
			case 2: // cascade
				pp->pos.loc.y+=1+(i&3);
				if ((pp->pos.loc.y==255)||(pp->pos.loc.y<pp->ey)) pp->pos.loc.y=pp->ey;
				break;
			case 3: // gravity
				pp->pos.loc.y++;
				if ((pp->pos.loc.y==255)||(pp->pos.loc.y<pp->ey)) pp->pos.loc.y=pp->ey;
				break;
			case 4: // sparks
				switch (i&3) {
				case 0:
					if (pp->pos.loc.x>1) pp->pos.loc.x-=2;
					break;
				case 1:
					if (pp->pos.loc.x>0) pp->pos.loc.x--;
					break;
				case 2:
					if (pp->pos.loc.x<254) pp->pos.loc.x+=2;
					break;
				case 3:
					if (pp->pos.loc.x<255) pp->pos.loc.x++;
					break;
				}
				switch ((i>>1)&3) {
				case 0:
					if (pp->pos.loc.y>1) pp->pos.loc.y-=2;
					break;
				case 1:
					if (pp->pos.loc.y>0) pp->pos.loc.y--;
					break;
				case 2:
					if (pp->pos.loc.y<254) pp->pos.loc.y+=2;
					break;
				case 3:
					if (pp->pos.loc.y<255) pp->pos.loc.y++;
					break;
				}
				pp->life++;
				if (pp->life>50) {
					pp->life=1;
					pp->pos.loc.x=pp->ex;
					pp->pos.loc.y=pp->ey;
				}
				break;
			case 5: // wander
			  pp->pos.loc.x+=FlameRand(11)-5;
			  pp->pos.loc.y+=FlameRand(11)-5;
			  break;
			case 6: // jump
			  pp->pos.loc.x=FlameRand(256);
			  pp->pos.loc.y=FlameRand(256);
			  break;
			}

		}

}

//
//   Moves and/or darkens the image
//

void Flamengine::Darkening() {
	int x,y,i=0;
	UBYTE *pt,*dpt;

	dpt=pt=data;
	pt+=256;
	if (!params.convec) dpt+=256;
	if (params.dark) i=1;
	for (y=255;y;y--) {
		for (x=256;x;x--) {
		  *dpt++=(*pt++)-i;
		}
	}

}

//
//   Straightforward blur box filter but offset one line
//

void Flamengine::ConvectionBlur() {
	UBYTE *pt1,*pt2,*pt3,*pt4,*pt,*wpt;
	int x,y,i;

	wpt=work;
	pt=data;
	pt+=257;
	wpt+=1;
	if (!params.convec) wpt+=256;

	pt1=pt2=pt3=pt4=pt;
	pt1-=1;
	pt2+=1;
	pt3-=256;
	pt4+=256;

	for (y=1;y<254;y++) {
//		for (x=1;x<254;x++) {
		for (x=0;x<256;x++) {
		  i=*pt1;
		  i+=(*pt2)+(*pt3)+(*pt4);
		  i>>=2;
		  i+=*pt;
		  i>>=1;
		  if (params.dark&&i) i--;
		  *wpt=i;

		  pt++; pt1++; pt2++; pt3++; pt4++; wpt++;
		}
//		pt++; pt1++; pt2++; pt3++; pt4++; wpt++;
//		pt++; pt1++; pt2++; pt3++; pt4++; wpt++;
	}
	memcpy(data,work,65536);

/*	// debug override
	memset(data,0,65536);
	pt=data;
	for (y=0;y<256;y++) {
		for (x=0;x<256;x++) {
			*pt=x-y;
			if ((!(x&15))||(!(y&15))) *pt=255;
			pt++;
		}
	}
*/
}


//
// H@X0R'd version 1
//
/*
void Flamengine::ConvectionBlur2() {
	UBYTE *pt1,*pt2,*pt3,*pt4,*pt,*wpt,*ptx;
	SLONG x,y,i;
	static SLONG offset = 0;
	SLONG blah,scale;

	wpt=work;
	pt=data;
	pt+=257;
	wpt+=1;
	if (!params.convec) wpt+=256;

	pt1=pt2=pt3=pt4=pt;
	pt1-=1;
	pt2+=1;
	pt3-=256;
	pt4+=256;

	offset+=160;
	for (y=1;y<254;y++) {
		blah=(y<<2)+offset;
		blah&=2047;
//		if (y<240) {
//			scale=(240-y)*130;
//			scale=(y)*130;
//			ptx=wpt+(SIN(blah)/scale);
			ptx=wpt+SIN(blah)/32768;
//		} else {
//			ptx=wpt;
//		}
		for (x=0;x<256;x++) {
		  i=*pt1;
		  i+=(*pt2)+(*pt3)+(*pt4);
		  i>>=2;
		  i+=*pt;
		  i>>=1;
		  if (params.dark&&i) i--;
//		  *wpt=i;
		  *ptx=i;

		  pt++; pt1++; pt2++; pt3++; pt4++; wpt++; ptx++;
		}
	}
	memcpy(data,work,65536);

}
*/
//
// H@X0R'd version 2
//

struct DarkZones {
	SLONG offset, offtime, midpnt, width;
};

#define ZONES 3

void Flamengine::ConvectionBlur2() {
	UBYTE *pt1,*pt2,*pt3,*pt4,*pt,*wpt;
	SLONG x,y,i,j,dif;
	static DarkZones zones[ZONES];
	SLONG difs[256];

	wpt=work;
	pt=data;
	pt+=257;
	wpt+=1;
	if (!params.convec) wpt+=256;

	pt1=pt2=pt3=pt4=pt;
	pt1-=1;
	pt2+=1;
	pt3-=256;
	pt4+=256;

	for (j=0;j<ZONES;j++) {
	  if (!zones[j].offtime) {
		zones[j].offset=FlameRand(256);
		zones[j].midpnt=50+FlameRand(150);
		zones[j].offtime=zones[j].midpnt*2;
//		zones[j].width=1+FlameRand(3);
	  }
	  zones[j].offtime--;
	}
	for (x=0;x<256;x++) {
	  difs[x]=0;
	  for (j=0;j<3;j++) {
//		dif=abs(zones[j].offset-x)-abs(zones[j].offtime-zones[j].midpnt);
//		dif=abs(x-zones[j].offset)-abs(zones[j].offtime-zones[j].midpnt);
		dif=abs(x-zones[j].offset)-abs(zones[j].midpnt-zones[j].offtime);
		if (dif<0) dif=0;
		difs[x]+=(SLONG)(dif*0.2f);
	  }
	}
	for (y=1;y<254;y++) {
		for (x=0;x<256;x++) {
		  i=*pt1;
		  i+=(*pt2)+(*pt3)+(*pt4);
		  i>>=2;
		  i+=*pt;
		  i>>=1;
//		  if (params.dark&&i) i--;
		  i-=difs[x];
		  if (i<0) i=0;
		  *wpt=(UBYTE)i;

		  pt++; pt1++; pt2++; pt3++; pt4++; wpt++;
		}
	}
	memcpy(data,work,65536);

}


//
//   Locks, pokes, unlocks and updates a texture with the fire data
//

void Flamengine::UpdateTexture() {

  if (TEXTURE_flame_lock()) {
	 SLONG  x;
	 SLONG  y;
     UWORD *image;
	 UWORD  pixel;
	 UBYTE *pt,*pt2;
     UBYTE red, green, blue;


	 // This version scales down for a 64x64 d3d texture
	 /*
	 
	 for (y = 0; y < 64; y++) {
		image  = TEXTURE_shadow_bitmap;
		image += y * (TEXTURE_shadow_pitch >> 1);

		pt=data;
		pt+=256*4*y;

		for (x=0;x<64;x++,image++,pt+=4) {

		  pt2=palette+((*pt)*3);
		  red  =*pt2++;
		  green=*pt2++;
		  blue =*pt2++;

		  pixel  = (red   >> TEXTURE_shadow_mask_red  ) << TEXTURE_shadow_shift_red;
		  pixel |= (green >> TEXTURE_shadow_mask_green) << TEXTURE_shadow_shift_green;
		  pixel |= (blue  >> TEXTURE_shadow_mask_blue ) << TEXTURE_shadow_shift_blue;
		  *image = pixel;
//		  *image = 0xFFFF;
		}

//		pt+=256*3;
	}
*/

	 // This one doesn't.

	 for (y = 0; y < 256; y++) {
		image  = TEXTURE_shadow_bitmap;
		image += y * (TEXTURE_shadow_pitch >> 1);

		pt=data;
		pt+=256*y;

		for (x=0;x<256;x++,image++,pt++) {

		  pt2=params.palette+((*pt)*3);
		  red  =*pt2++;
		  green=*pt2++;
		  blue =*pt2++;

		  pixel  = (red   >> TEXTURE_shadow_mask_red  ) << TEXTURE_shadow_shift_red;
		  pixel |= (green >> TEXTURE_shadow_mask_green) << TEXTURE_shadow_shift_green;
		  pixel |= (blue  >> TEXTURE_shadow_mask_blue ) << TEXTURE_shadow_shift_blue;
		  *image = pixel;
//		  *image = 0xFFFF;
		}

//		pt+=256*3;
	}

	TEXTURE_flame_unlock();
	TEXTURE_flame_update();
  }

}

//
// Blit the texture, suitably alpha'd, onto the backbuffer
//

void Flamengine::Blit() {
    POLY_Point			pp[4],
						*quad[4];

	quad[0]	=	&pp[0];
	quad[1] =	&pp[1];
	quad[2] =	&pp[2];
	quad[3] =	&pp[3];

	pp[0].X		=	0;
	pp[0].Y		=	200;
	pp[0].Z		=	0.5f;
	pp[0].u		=	0.0;
	pp[0].v		=	0.0;
	pp[0].colour	=	0xffffff;
	pp[0].specular=	0xff000000;

	pp[1].X		=	640;
	pp[1].Y		=	200;
	pp[1].Z		=	0.5f;
	pp[1].u		=	2.0;
	pp[1].v		=	0.0;
	pp[1].colour	=	0xffffff;
	pp[1].specular	=	0xff000000;

	pp[2].X		=	0;
	pp[2].Y		=	480;
	pp[2].Z		=	0.5f;
	pp[2].u		=	0.0;
	pp[2].v		=	1.0;
	pp[2].colour	=	0xffffff;
	pp[2].specular	=	0xff000000;

	pp[3].X		=	640;
	pp[3].Y		=	480;
	pp[3].Z		=	0.5f;
	pp[3].u		=	2.0;
	pp[3].v		=	1.0;
	pp[3].colour	=	0xffffff;
	pp[3].specular	=	0xff000000;

	POLY_add_quad(quad,POLY_PAGE_MENUFLAME,FALSE,TRUE);

}

void Flamengine::BlitHalf(CBYTE side) {
    POLY_Point			pp[4],
						*quad[4];

	quad[0]	=	&pp[0];
	quad[1] =	&pp[1];
	quad[2] =	&pp[2];
	quad[3] =	&pp[3];

	if (side) {
		pp[0].X		=	320;
		pp[1].X		=	640;
		pp[2].X		=	320;
		pp[3].X		=	640;
	} else {
		pp[0].X		=	0;
		pp[1].X		=	320;
		pp[2].X		=	0;
		pp[3].X		=	320;
	}

	pp[0].Y		=	200;
	pp[0].Z		=	0.5f;
	pp[0].u		=	0.0;
	pp[0].v		=	0.0;
	pp[0].colour	=	0xffffff;
	pp[0].specular=	0xff000000;

	pp[1].Y		=	200;
	pp[1].Z		=	0.5f;
	pp[1].u		=	1.0;
	pp[1].v		=	0.0;
	pp[1].colour	=	0xffffff;
	pp[1].specular	=	0xff000000;

	pp[2].Y		=	480;
	pp[2].Z		=	0.5f;
	pp[2].u		=	0.0;
	pp[2].v		=	1.0;
	pp[2].colour	=	0xffffff;
	pp[2].specular	=	0xff000000;

	pp[3].Y		=	480;
	pp[3].Z		=	0.5f;
	pp[3].u		=	1.0;
	pp[3].v		=	1.0;
	pp[3].colour	=	0xffffff;
	pp[3].specular	=	0xff000000;

	POLY_add_quad(quad,POLY_PAGE_MENUFLAME,FALSE,TRUE);

}


//
// Blit the texture, skewed for feedback, onto the backbuffer
//

void Flamengine::BlitOffset() {
    POLY_Point			pp[4],
						*quad[4];

	quad[0]	=	&pp[0];
	quad[1] =	&pp[1];
	quad[2] =	&pp[2];
	quad[3] =	&pp[3];

	pp[0].X		=	0;
	pp[0].Y		=	0;
	pp[0].Z		=	0.5f;
	pp[0].u		=	0.0;
	pp[0].v		=	0.0;
	pp[0].colour	=	0xffffff;
	pp[0].specular=	0xff000000;

	pp[1].X		=	256;
	pp[1].Y		=	0;
	pp[1].Z		=	0.5f;
	pp[1].u		=	1.0;
	pp[1].v		=	0.0;
	pp[1].colour	=	0xffffff;
	pp[1].specular	=	0xff000000;

	pp[2].X		=	0;
	pp[2].Y		=	256;
	pp[2].Z		=	0.5f;
	pp[2].u		=	0.0;
	pp[2].v		=	1.0;
	pp[2].colour	=	0xffffff;
	pp[2].specular	=	0xff000000;

	pp[3].X		=	256;
	pp[3].Y		=	256;
	pp[3].Z		=	0.5f;
	pp[3].u		=	1.0;
	pp[3].v		=	1.0;
	pp[3].colour	=	0xffffff;
	pp[3].specular	=	0xff000000;

	POLY_add_quad(quad,POLY_PAGE_MENUFLAME,FALSE,TRUE);

	pp[0].u		=	0.1f; // L
	pp[0].v		=	0.2f; // T
	pp[1].u		=	0.9f; // R
	pp[1].v		=	0.2f; // T
	pp[2].u		=	0.1f; // L
	pp[2].v		=	1.0f; // B
	pp[3].u		=	0.9f; // R
	pp[3].v		=	1.0f; // B
	pp[0].colour	=	0xafafaf;
	pp[1].colour	=	0xafafaf;
	pp[2].colour	=	0xafafaf;
	pp[3].colour	=	0xafafaf;

	POLY_add_quad(quad,POLY_PAGE_MENUFLAME,FALSE,TRUE);

}

//
//   Funky feedback; must be used BEFORE any other screen draw
//


void Flamengine::Feedback() {
    RECT     rcSource, rcDest;
	HRESULT  res;

  // step 1 - create poly with flames on, warped, draw to back buffer

	the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_TARGET);

#ifndef TARGET_DC
	POLY_frame_init(FALSE, FALSE);
#endif
//	Blit();
	BlitOffset();
#ifndef TARGET_DC
	POLY_frame_draw(FALSE,TRUE);
#endif
   

  // step 2 - blit back buffer back to flames


	rcSource.top = 0;		rcSource.left = 0;
	rcSource.bottom = 256;	rcSource.right = 256; 
	rcDest.top = 0;			rcDest.left = 0;
	rcDest.bottom = 256;	rcDest.right = 256;
 
//	res=TEXTURE_texture[TEXTURE_page_menuflame].GetSurface()->Blt(&rcDest,the_display.lp_DD_BackSurface,&rcSource,DDBLT_WAIT,NULL);
	res=TEXTURE_texture[TEXTURE_page_menuflame].GetSurface()->Blt(NULL,the_display.lp_DD_BackSurface,&rcSource,DDBLT_WAIT,NULL);


}

#endif //#ifndef TARGET_DC


