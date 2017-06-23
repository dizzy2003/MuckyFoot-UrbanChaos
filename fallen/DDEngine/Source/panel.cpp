#include	"game.h"

#include	"aeng.h"
#include	"poly.h"
#include	"sprite.h"
#include	"fallen/headers/supermap.h"
#include	"fallen/headers/cam.h"
#include	"fc.h"
#include	<math.h>
#include	"panel.h"
#include	"drawxtra.h"
#include	"statedef.h"
#include	"font2d.h"
#include	"eway.h"
#include	"DDLib.h"
#include	"fallen/ddengine/headers/map.h"
#include	"memory.h"
#include	"mfx.h"
#include	"xlat_str.h"
#include	"sound_id.h"
#include	"pcom.h"
#include	"env.h"

#ifdef TARGET_DC
#include "target.h"
#endif


#ifdef TARGET_DC
// intrinsic maths
#include <shsgintr.h>
#endif


#define		HEALTH_BAR_WIDTH	100
#define		HEALTH_BAR_HEIGHT	10
#define		HEALTH_BAR_VERT_GAP	2
#define		HEALTH_BAR_HORIZ_GAP	2


#ifdef TARGET_DC
// Slightly shifts each line of text a bit closer than the last to get correct
// drawing behaviour on autosort cards (i.e. the DC).
// This might also be a good idea on the PC, you never know.
#define BODGE_MY_PANELS_PLEASE_BOB Yes please
#endif


#ifdef TARGET_DC
void PANEL_draw_VMU_ammo_counts ( void );
#endif






// Coords of the bottom-left corner.
#ifdef TARGET_DC
// Got to keep away from the edges.
int m_iPanelXPos = 32;
int m_iPanelYPos = 480 - 32;

#else
int m_iPanelXPos = 0;
int m_iPanelYPos = 480;
#endif





#ifdef BODGE_MY_PANELS_PLEASE_BOB
#define DEPTH_BODGE_START (0.95f)
#define DEPTH_BODGE_INC	(0.00001f)
float PANEL_fDepthBodgeValue;

#endif



#ifdef BODGE_MY_PANELS_PLEASE_BOB
void PANEL_ResetDepthBodge ( void )
{
	PANEL_fDepthBodgeValue = DEPTH_BODGE_START;
}

float PANEL_GetNextDepthBodge ( void )
{
	PANEL_fDepthBodgeValue += DEPTH_BODGE_INC;
	ASSERT ( PANEL_fDepthBodgeValue < 1.0f );
	return ( PANEL_fDepthBodgeValue );
}
#else
// Just use 0.9 as a goodish value.
float PANEL_GetNextDepthBodge ( void )
{
	return ( 0.9f );
}

void PANEL_ResetDepthBodge ( void )
{
	// Do nothing.
}

#endif




UBYTE		PANEL_scanner_poo=0;


void PANEL_draw_quad(
		float left,
		float top,
		float right,
		float bottom,
		SLONG page,
		ULONG colour,
		float u1,
		float v1,
		float u2,
		float v2)
{
	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	float fWDepthBodge = PANEL_GetNextDepthBodge();
	float fZDepthBodge = 1.0f - fWDepthBodge;


	pp[0].X        = left;
	pp[0].Y        = top;
	pp[0].z        = fZDepthBodge;
	pp[0].Z        = fWDepthBodge;
	pp[0].u        = u1;
	pp[0].v        = v1;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = right;
	pp[1].Y        = top;
	pp[1].z        = fZDepthBodge;
	pp[1].Z        = fWDepthBodge;
	pp[1].u        = u2;
	pp[1].v        = v1;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = left;
	pp[2].Y        = bottom;
	pp[2].z        = fZDepthBodge;
	pp[2].Z        = fWDepthBodge;
	pp[2].u        = u1;
	pp[2].v        = v2;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = right;
	pp[3].Y        = bottom;
	pp[3].z        = fZDepthBodge;
	pp[3].Z        = fWDepthBodge;
	pp[3].u        = u2;
	pp[3].v        = v2;
	pp[3].colour   = colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, page, FALSE, TRUE);
}


void PANEL_crap_text ( int x, int y, char *string )
{
	FONT2D_DrawString(
		string,
		x + 1,
		y + 1,
		0x000000,
		256,
		POLY_PAGE_FONT2D,
		0);

	FONT2D_DrawString(
		string,
		x,
		y,
		0xffffee,
		256,
		POLY_PAGE_FONT2D,
		0);
}



void PANEL_draw_face(SLONG x,SLONG y,SLONG face,SLONG size)
{
	float left;
	float right;
	float top;
	float bottom;

	left   = (float) x;
	top    = (float) y;
	right  = left + (float) size;
	bottom = top  + (float) size;

	SATURATE(face, 1, 6);

	PANEL_draw_quad(
		left,
		top,
		right,
		bottom,
		POLY_PAGE_FACE1+face-1);
}


#if 0
// No longer used!
void PANEL_draw_angelic_status(SLONG x, SLONG y, SLONG size, SLONG am_i_an_angel)
{
	float left;
	float right;
	float top;
	float bottom;

	left   = (float) x;
	top    = (float) y;
	right  = left + (float) size;
	bottom = top  + (float) size;

	SLONG page;

	page = (am_i_an_angel) ? POLY_PAGE_ANGEL : POLY_PAGE_DEVIL;

	PANEL_draw_quad(
		left,
		top,
		right,
		bottom,
		page);
}

void PANEL_draw_press_button(SLONG x, SLONG y, SLONG size, SLONG frame)
{
	float left;
	float right;
	float top;
	float bottom;

	left   = (float) x;
	top    = (float) y;
	right  = left + (float) size;
	bottom = top  + (float) size;

	SLONG page;

	page = (frame & 1) ? POLY_PAGE_PRESS1 : POLY_PAGE_PRESS2;

	PANEL_draw_quad(
		left,
		top,
		right,
		bottom,
		page);
}
#endif


#ifndef TARGET_DC
extern UBYTE sw_hack;
#endif

void	PANEL_draw_health_bar(SLONG x,SLONG y,SLONG percentage)
{
#ifndef TARGET_DC
	if (!sw_hack)
#endif
	{
		AENG_draw_rect(x,y,HEALTH_BAR_WIDTH,HEALTH_BAR_HEIGHT,0x000000,2,POLY_PAGE_COLOUR);
	}

	if(percentage<0)
	{
		percentage=0;
	}
	else
	if(percentage>100)
	{
		percentage=100;
	}

	AENG_draw_rect(x+HEALTH_BAR_HORIZ_GAP,y+HEALTH_BAR_VERT_GAP,((HEALTH_BAR_WIDTH-HEALTH_BAR_HORIZ_GAP*2)*percentage)/100,6,0xff0000,1,POLY_PAGE_COLOUR);

}

#define B0 (1 << 0)
#define B1 (1 << 1)
#define B2 (1 << 2)
#define B3 (1 << 3)
#define B4 (1 << 4)
#define B5 (1 << 5)
#define B6 (1 << 6)

#define PANEL_SEG_L	(16.0F)
#define PANEL_SEG_W ( 4.0F)

void PANEL_draw_number(float x, float y, UBYTE digit)	// 0 <= digit <= 9... Not ASCII!
{
	UBYTE number[10] =
	{
		B0|B1|B2|B4|B5|B6,
		B2|B5,
		B0|B2|B3|B4|B6,
		B0|B2|B3|B5|B6,
		B1|B2|B3|B5,
		B0|B1|B3|B5|B6,
		B0|B1|B3|B4|B5|B6,
		B0|B2|B5,
		B0|B1|B2|B3|B4|B5|B6,
		B0|B1|B2|B3|B5
	};

	struct
	{
		UBYTE d1;
		UBYTE d2;
		float dx;
		float dy;

	} bit[7] =
	{
		{0,1,0.0F,PANEL_SEG_W/2.0F},
		{0,2,PANEL_SEG_W/2.0F,0.0F},
		{1,3,PANEL_SEG_W/2.0F,0.0F},
		{2,3,0.0F,PANEL_SEG_W/2.0F},
		{2,4,PANEL_SEG_W/2.0F,0.0F},
		{3,5,PANEL_SEG_W/2.0F,0.0F},
		{4,5,0.0F,PANEL_SEG_W/2.0F},
	};

	float x1;
	float y1;

	float x2;
	float y2;

	float dx;
	float dy;

	SLONG b;
	ULONG colour;

	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	ASSERT(WITHIN(digit, 0, 9));

	for (b = 0; b < 7; b++)
	{
		//
		// Work out the positions of the two points.
		//

		x1 = x + ((bit[b].d1 &  1) ? PANEL_SEG_L : 0.0F);
		y1 = y +  (bit[b].d1 >> 1) * PANEL_SEG_L;

		x2 = x + ((bit[b].d2 &  1) ? PANEL_SEG_L : 0.0F);
		y2 = y +  (bit[b].d2 >> 1) * PANEL_SEG_L;

		//
		// What colour?
		//

		colour = (number[digit] & (1 << b)) ? 0xeeff0000 : 0x88110000;

		//
		// The offset vector to make a rect from a line.
		//

		dx = bit[b].dx;
		dy = bit[b].dy;

		//
		// The four POLY_Points...
		//


		float fWDepthBodge = PANEL_GetNextDepthBodge();
		float fZDepthBodge = 1.0f - fWDepthBodge;


		pp[0].X        = x1 + dx;
		pp[0].Y        = y1 + dy;
		pp[0].z        = fZDepthBodge;
		pp[0].Z        = fWDepthBodge;
		pp[0].u        = 0.0F;
		pp[0].v        = 0.0F;
		pp[0].colour   = colour;
		pp[0].specular = 0xff000000;

		pp[1].X        = x1 - dx;
		pp[1].Y        = y1 - dy;
		pp[1].z        = fZDepthBodge;
		pp[1].Z        = fWDepthBodge;
		pp[1].u        = 1.0F;
		pp[1].v        = 0.0F;
		pp[1].colour   = colour;
		pp[1].specular = 0xff000000;

		pp[2].X        = x2 + dx;
		pp[2].Y        = y2 + dy;
		pp[2].z        = fZDepthBodge;
		pp[2].Z        = fWDepthBodge;
		pp[2].u        = 0.0F;
		pp[2].v        = 1.0F;
		pp[2].colour   = colour;
		pp[2].specular = 0xff000000;

		pp[3].X        = x2 - dx;
		pp[3].Y        = y2 - dy;
		pp[3].z        = fZDepthBodge;
		pp[3].Z        = fWDepthBodge;
		pp[3].u        = 1.0F;
		pp[3].v        = 1.0F;
		pp[3].colour   = colour;
		pp[3].specular = 0xff000000;

		POLY_add_quad(quad, POLY_PAGE_ALPHA, FALSE, TRUE);
	}
}

#ifndef TARGET_DC

typedef struct
{
	float time;
	float x;
	float y;

} PANEL_Store;

#define PANEL_MAX_STORES 8

PANEL_Store PANEL_store[PANEL_MAX_STORES];
SLONG       PANEL_store_upto;

#endif

/*
void PANEL_draw_timer_do(float time, float x, float y)
{
	CBYTE  countdown[32];
	CBYTE *ch;

	SLONG mins = 0;

	ASSERT(time < 1000.0F);

	while(time >= 60.0F)
	{
		mins += 1;
		time -= 60.0F;
	}

	sprintf(countdown, "%02d %05.2f", mins, time);

	for (ch = countdown; *ch; ch++)
	{
		if (WITHIN(*ch, '0', '9'))
		{
			x -= 11.0F;
		}
		else
		{
			x -= 6.0F;
		}
	}

	float x_start = x;

	for (ch = countdown; *ch; ch++)
	{
		if (WITHIN(*ch, '0', '9'))
		{
			PANEL_draw_number(x, y, *ch - '0');

			x += 22.0F;
		}
		else
		{
			x += 12.0F;
		}
	}

	//
	// An alpha overlay over it all.
	//

	PANEL_draw_quad(
		x_start -  4.0F,
		y       -  4.0F,
		x       +  4.0F,
		y       + 36.0F,
		POLY_PAGE_ALPHA_OVERLAY,
		0xaa000000);
}
*/

#ifndef TARGET_DC
void PANEL_draw_buffered()
{

	SLONG i;

	for (i = 0; i < PANEL_store_upto; i++)
	{
		/*

		PANEL_draw_timer_do(
			PANEL_store[i].time,
			PANEL_store[i].x,
			PANEL_store[i].y);

		*/

		float time = PANEL_store[i].time;
		float x    = PANEL_store[i].x;
		float y    = PANEL_store[i].y;

		CBYTE  countdown[32];

		SLONG mins = 0;

		ASSERT(time < 1000.0F);

		while(time >= 60.0F)
		{
			mins += 1;
			time -= 60.0F;
		}

		sprintf(countdown, "%02d:%02d", mins, SLONG(time));

		if ((time<30)&&!mins)
		{
			static UWORD pulse=0;
			SLONG colour;
			pulse+=(TICK_RATIO*80)>>TICK_SHIFT;
			colour=(SIN(pulse&2047)>>9)+128;
			colour=colour|(colour<<8);
			FONT2D_DrawStringCentred(countdown, m_iPanelXPos + 171, m_iPanelYPos - 118, 0xff0000|colour, 256 + 64);

		}
		else
		{
			FONT2D_DrawStringCentred(countdown, m_iPanelXPos + 171, m_iPanelYPos - 118, 0xffffff, 256 + 64);
		}
	}

	PANEL_store_upto = 0;

}
#endif

#ifdef TARGET_DC
SLONG slPANEL_draw_timer_time = -1;
void PANEL_draw_timer(SLONG time, SLONG x, SLONG y)
{
	slPANEL_draw_timer_time = time;
}

#else

void PANEL_draw_timer(SLONG time, SLONG x, SLONG y)
{
	if (WITHIN(PANEL_store_upto, 0, PANEL_MAX_STORES - 1))
	{
		PANEL_store[PANEL_store_upto].time = float(time) * (1.0F / 100.0F);
		PANEL_store[PANEL_store_upto].x    = float(x);
		PANEL_store[PANEL_store_upto].y    = float(y);

		PANEL_store_upto += 1;
	}
}
#endif


extern	void POLY_add_rect(POLY_Point *p1, SLONG width,SLONG height,  SLONG page, UBYTE sort_to_front);

void PANEL_draw_local_health(SLONG mx,SLONG my,SLONG mz,SLONG percentage,SLONG radius=60)
{
	POLY_Point p1;

	float	dx,dy,dz,len;

//	ASSERT(percentage>=0 && percentage<=100);


	dx=POLY_cam_x-(float)mx;
	dy=POLY_cam_y-(float)my;
	dz=POLY_cam_z-(float)mz;

#ifdef TARGET_DC
	len=_InvSqrtA(dx*dx+dy*dy+dz*dz);

	dx=(dx*((float)radius))*len;
	dy=(dy*((float)radius))*len;
	dz=(dz*((float)radius))*len;
#else
	len=sqrt(dx*dx+dy*dy+dz*dz);

	dx=(dx*((float)radius))/len;
	dy=(dy*((float)radius))/len;
	dz=(dz*((float)radius))/len;
#endif

	//
	// move towards the camera a bit so you sort infront of the person
	//



	POLY_transform(float(mx+dx), float(my+dy-10), float(mz+dz), &p1);


	// Roper and others have >100% health - clamp it.
	if ( percentage > 100 )
	{
		percentage = 100;
	}


#ifdef TARGET_DC


	p1.X-=27.0f;

	// Drag it forwards, otherwise at distance it becomes hidden in the pavement.
	p1.Z+=0.02F;


	p1.colour=0xc0000000|0x0f;
	p1.specular=0xff000000;

	if(p1.IsValid())
	{
		POLY_add_rect(&p1, 54,8,POLY_PAGE_COLOUR,0);
	}
	else
	{
		return;
	}

	p1.X+=2.0f;

	p1.colour=0x40000000|0xff0000;
	p1.specular=0xff000000;

	p1.Y+=2.0F;

	// Only very slightly forwards - the DC is nice and accurate.
	p1.Z+=0.00001F;

	if(p1.IsValid())
	{
		POLY_add_rect(&p1,percentage>>1,4,POLY_PAGE_COLOUR,0);
//		AENG_draw_rect(p1.X,p1.Y,percentage>>1,2,0xb0000000,3,POLY_PAGE_COLOUR);
//		POLY_add_line(&p1, &p2, (float)2.0, 2.0, POLY_PAGE_COLOUR, 0);
	}





#else


	// PC version

	p1.X-=27.0f;

	p1.colour=0xc0000000|0x0f;
	p1.specular=0xff000000;

	if(p1.IsValid())
	{
		if (!sw_hack)
		{
			POLY_add_rect(&p1, 54,4,POLY_PAGE_COLOUR,0);
		}
	}
	else
		return;

	p1.X+=2.0f;

	p1.colour=0x40000000|0xff0000;
	p1.specular=0xff000000;

	p1.Y+=1.0F;

	p1.Z+=0.01F;

	if(p1.IsValid())
	{
		POLY_add_rect(&p1,percentage>>1,2,POLY_PAGE_COLOUR,0);
//		AENG_draw_rect(p1.X,p1.Y,percentage>>1,2,0xb0000000,3,POLY_PAGE_COLOUR);
//		POLY_add_line(&p1, &p2, (float)2.0, 2.0, POLY_PAGE_COLOUR, 0);
	}

#endif


}

void PANEL_draw_gun_sight(SLONG mx,SLONG my,SLONG mz,SLONG accuracy,SLONG scale)
{
	SLONG	angle,cangle;
	SLONG	c0;
	SLONG	dx1,dy1,dx2,dy2;
	POLY_Point p1,p2,pstart;
	SLONG	r_in,r_out;
	ULONG	col;
	SLONG	sat_acc;

#define	RADIUS_OUT	164
#define	RADIUS_IN	84

	angle=accuracy;
	POLY_transform(float(mx), float(my-36), float(mz), &pstart);
	r_in = (POLY_world_length_to_screen(RADIUS_IN) * pstart.Z*scale)/256;
	r_out = (POLY_world_length_to_screen(RADIUS_OUT) * pstart.Z*scale)/256;

	sat_acc=accuracy;
	SATURATE(sat_acc,0,255)




	col =  sat_acc<<8;
	col |= (255-sat_acc)<<16;

	for(c0=0;c0<4;c0++)
	{

		p2=p1=pstart;
		switch(c0)
		{
			case	0:
				cangle=+angle;
				break;
			case	1:
				cangle=-angle;
				break;
			case	2:
				cangle=1024+angle;
				break;
			case	3:
				cangle=1024-angle;
				break;
		}

//		cangle+=GAME_TURN<<5;
		dx1=((COS(cangle&2047)*r_out)>>16);
		dy1=((SIN(cangle&2047)*r_out)>>16);
		dx2=((COS(cangle&2047)*r_in)>>16);
		dy2=((SIN(cangle&2047)*r_in)>>16);

		p1.X+=(float)dx1;
		p1.Y+=(float)dy1;
		p2.X+=(float)dx2;
		p2.Y+=(float)dy2;

		p1.colour=0x40000000|col;
		p2.colour=0x40000000|col;
		p1.specular=0xff000000;
		p2.specular=0xff000000;

		if(p1.IsValid() && p2.IsValid())
		{
			SLONG width;
			width=(30*scale)>>8;
			POLY_add_line(&p1, &p2, (float)width, 0.0F, POLY_PAGE_COLOUR_ALPHA, 0);
//			POLY_add_line(&p1, &p2, 20.0F, 0.0F, POLY_PAGE_COLOUR_ALPHA, 0);
		}

	}

//	r_out=(r_out*270)>>8;
//	r_out*=(270+256-SATURATE(accuracy,0,256))>>3;
	r_in=20+(sat_acc>>2);
	r_in=(r_in*scale)>>8;
	r_out=r_in+((80*scale)>>8);

	r_in = POLY_world_length_to_screen(r_in) * pstart.Z;
	r_out = POLY_world_length_to_screen(r_out) * pstart.Z;

	for(c0=0;c0<4;c0++)
	{

		p2=p1=pstart;
		switch(c0)
		{
			case	0:
				cangle=512;
				break;
			case	1:
				cangle=-512;
				break;
			case	2:
				cangle=1024;
				break;
			case	3:
				cangle=0;
				break;
		}

//		cangle+=GAME_TURN<<5;
		dx1=((COS(cangle&2047)*(r_out))>>16);
		dy1=((SIN(cangle&2047)*(r_out))>>16);
		dx2=((COS(cangle&2047)*(r_in))>>16);
		dy2=((SIN(cangle&2047)*(r_in))>>16);

		p1.X+=(float)dx1;
		p1.Y+=(float)dy1;
		p2.X+=(float)dx2;
		p2.Y+=(float)dy2;

		p1.colour=0x40000000|col;
		p2.colour=0x40000000|col;
		p1.specular=0xff000000;
		p2.specular=0xff000000;

		if(p1.IsValid() && p2.IsValid())
		{
			SLONG	width;

			width=(5*scale)>>8;
			POLY_add_line(&p1, &p2, (float)width, (float)width,POLY_PAGE_COLOUR_ALPHA, 0);
		}
//		POLY_add_line(&p1, &p2, 5.0F, 5.0F, POLY_PAGE_COLOUR_ALPHA, 0);

	}

}


#if 0


#define	COMPASS_MID_X		600.0f
#define	COMPASS_MID_Y		40.0f
#define	COMPASS_RAD			40.0f

void PANEL_draw_compass_angle(float dx,float dy,ULONG	col)
{
	float	x=0,y=1.0;
	POLY_Point p1;
	POLY_Point p2;

	p1.X        = COMPASS_MID_X;
	p1.Y        = COMPASS_MID_Y;
	p1.z        = 0.0F;
	p1.Z        = 1.0F;
	p1.u        = 1.0F;
	p1.v        = 1.0F;
	p1.colour   = col;
	p1.specular = 0xff000000;

	p2.X        = COMPASS_MID_X+dx*COMPASS_RAD;
	p2.Y        = COMPASS_MID_Y+dy*COMPASS_RAD;
	p2.z        = 0.0F;
	p2.Z        = 1.0F;
	p2.u        = 1.0F;
	p2.v        = 1.0F;
	p2.colour   = 0xff0000;
	p2.specular = col;

	POLY_setclip(&p1);
	POLY_setclip(&p2);

	if (POLY_valid_line(&p1, &p2))
	{
		POLY_add_line(&p1, &p2, 1.0, 0.0, POLY_PAGE_COLOUR, 1);
	}

/*
	p2.X        = COMPASS_MID_X+dx*(COMPASS_RAD-10.0);
	p2.Y        = COMPASS_MID_Y+dy*(COMPASS_RAD-10.0);
	p1.z        = 0.0F;
	p1.Z        = 1.0F;
	p1.u        = 1.0F;
	p1.v        = 1.0F;
	p1.colour   = 0xff0000;
	p1.specular = 0xff000000;

	p2.X        = COMPASS_MID_X+dx*COMPASS_RAD;
	p2.Y        = COMPASS_MID_Y+dy*COMPASS_RAD;
	p2.z        = 0.0F;
	p2.Z        = 1.0F;
	p2.u        = 1.0F;
	p2.v        = 1.0F;
	p2.colour   = 0xff0000;
	p2.specular = 0xff000000;

	POLY_setclip(&p1);
	POLY_setclip(&p2);

	if (POLY_valid_line(&p1, &p2))
	{
		POLY_add_line(&p1, &p2, 3.0, 0.0, POLY_PAGE_COLOUR, 1);
	}
*/


}

void PANEL_draw_compass_north(void)
{
	float	x=0,y=1.0;
	float	dx,dy;
	float	angle;
	POLY_Point p1;
	POLY_Point p2;
#ifdef	OLD_CAM
	angle=CAM_get_ryaw();
	angle=PI+angle;

	dx=x*cos(angle)-y*sin(angle);
	dy=x*sin(angle)+y*cos(angle);

	PANEL_draw_compass_angle(dx,dy,0xff0000);
#endif
}


void PANEL_draw_compass_to(SLONG dx,SLONG dy)
{
	float	fdx,fdy;
	float	dist;

	fdx=dx;
	fdy=dy;

	dist=sqrt(fdx*fdx+fdy*fdy);

	fdx=fdx/dist;
	fdy=fdy/dist;

	PANEL_draw_compass_angle(dx,dy,0xffff00);
}



#endif


//
// The positions of all the icons on the IC page.
//

#define PANEL_IC_BACKBOX			0		// These are on page 1
#define PANEL_IC_AK47				1
#define PANEL_IC_GRENADE			2
#define PANEL_IC_AK47_AMMO_ONE		3
#define PANEL_IC_AK47_AMMO_GROUP	4
#define PANEL_IC_PISTOL				5
#define PANEL_IC_DARCI_OUTLINE		6
#define PANEL_IC_ROPER_OUTLINE		7
#define PANEL_IC_DARCI				8
#define PANEL_IC_ROPER				9
#define PANEL_IC_SHOTGUN			10

#define PANEL_IC_ROPER_HAND			11
#define PANEL_IC_DARCI_HAND			12
#define PANEL_IC_KNIFE				13
#define PANEL_IC_SHOTGUN_AMMO_GROUP	14
#define PANEL_IC_SHOTGUN_AMMO_ONE	15
#define PANEL_IC_PISTOL_AMMO_GROUP	16
#define PANEL_IC_PISTOL_AMMO_ONE	17
#define PANEL_IC_BIG_AMMO_GROUP		18
#define PANEL_IC_BIG_AMMO_ONE		19
#define PANEL_IC_GRENADE_AMMO_GROUP	20
#define PANEL_IC_GRENADE_AMMO_ONE	21

#define PANEL_IC_DIGIT_0			22
#define PANEL_IC_DIGIT_1			23
#define PANEL_IC_DIGIT_2			24
#define PANEL_IC_DIGIT_3			25
#define PANEL_IC_DIGIT_4			26
#define PANEL_IC_DIGIT_5			27
#define PANEL_IC_DIGIT_6			28
#define PANEL_IC_DIGIT_7			29
#define PANEL_IC_DIGIT_8			30
#define PANEL_IC_DIGIT_9			31

#define PANEL_IC_BASEBALLBAT		32
#define PANEL_IC_EXPLOSIVES         33
#define PANEL_IC_GRIMREAPER			34
#define PANEL_IC_DANGERMETER		35

#define PANEL_IC_BUBBLE_START		36
#define PANEL_IC_BUBBLE_MIDDLE		37
#define PANEL_IC_BUBBLE_END			38

#define PANEL_IC_DOT				39

#define PANEL_IC_CLIP_PISTOL		40
#define PANEL_IC_CLIP_SHOTGUN		41
#define PANEL_IC_CLIP_AK47			42

#define PANEL_IC_GEAR_BOX			43
#define PANEL_IC_GEAR_LOW			44
#define PANEL_IC_GEAR_HIGH			45

#define PANEL_IC_NUMBER				46

typedef struct
{
	float u1;
	float v1;
	float u2;
	float v2;
	SLONG page;

} PANEL_Ic;

#define PIC(uv) ((uv) / 256.0F)

PANEL_Ic PANEL_ic[PANEL_IC_NUMBER] =
{
	{PIC(  0), PIC(  0), PIC( 99), PIC( 81), 0},		// Backbox
	{PIC(  1), PIC( 83), PIC(191), PIC(137), 0},		// AK47
	{PIC(100), PIC(  1), PIC(140), PIC( 64), 0},		// Grenade
	{PIC(141), PIC(  1), PIC(146), PIC( 13), 0},		// Single ammo
	{PIC(158), PIC(  1), PIC(208), PIC( 13), 0},		// 10 ammos
	{PIC(142), PIC( 15), PIC(221), PIC( 75), 0},		// Pistol
	{PIC(  0), PIC(144), PIC( 55), PIC(255), 0},		// Darci outline
	{PIC( 55), PIC(144), PIC(109), PIC(256), 0},		// Roper outline
	{PIC(110), PIC(149), PIC(160), PIC(255), 0},		// Darci
	{PIC(160), PIC(148), PIC(210), PIC(255), 0},		// Roper
	{PIC(210), PIC( 95), PIC(254), PIC(256), 0},		// Shotgun

	//
	// These are all on the second page.
	//

	#define PICALL1(a,b,c,d) {PIC(a),PIC(b),PIC(c),PIC(d),1}

	PICALL1(  1,   1,  75,  50),
	PICALL1( 75,   5, 151,  46),
	PICALL1(  1,  55,  97,  70),
	PICALL1(  5,  81,  45,  93),
	PICALL1( 48,  81,  53,  93),
	PICALL1( 16,  95,  66, 104),
	PICALL1(  5,  95,  10, 104),
	PICALL1( 15, 108,  39, 122),
	PICALL1(  5, 108,  13, 122),
	PICALL1( 18, 123,  54, 141),
	PICALL1(  4, 123,  16, 141),

	#define PICALL0(a,b,c,d) {PIC(a),PIC(b),PIC(c),PIC(d),0}

	//
	// These are all squeezed in on the first page.
	//

	PICALL0(241, 77, 255, 94),		// Digit 0
	PICALL0(225,  3, 240, 20),		// 1
	PICALL0(225, 21, 240, 38),		// 2
	PICALL0(225, 40, 240, 57),		// 3
	PICALL0(240,  3, 256, 20),		// 4
	PICALL0(241, 21, 255, 38),		// 5
	PICALL0(241, 40, 255, 57),		// 6
	PICALL0(226, 59, 240, 76),		// 7
	PICALL0(241, 59, 255, 76),		// 8,
	PICALL0(226, 77, 240, 94),		// 9

	//
	// On the second page.
	//

	PICALL1(  3, 141, 170, 164),	// Baseball bat
	PICALL1( 11, 176,  44, 223),	// The explosives
	PICALL1(208, 187, 236, 214),	// The grim reaper
	PICALL1(190, 125, 221, 256),	// The danger meter

	PICALL1(224, 108, 242, 172),	// Bubble start
	PICALL1(242, 108, 243, 172),	// Bubble middle
	PICALL1(243, 108, 256, 172),	// Bubble end

	PICALL1(140, 197, 154, 211),	// Dot

	PICALL1( 79, 225, 100, 252),	// Clips for pistol, shotgun and AK47
	PICALL1(121, 227, 143, 252),
	PICALL1(152, 225, 174, 250),

	PICALL1(205, 215, 254, 256),	// Gearbox
	PICALL1(178, 225, 199, 249),	// Low gear
	PICALL1(240, 182, 254, 205),	// High gear

};

#define PANEL_PAGE_NORMAL    0
#define PANEL_PAGE_ALPHA     1
#define PANEL_PAGE_ADDITIVE  2
#define PANEL_PAGE_ALPHA_END 3
#define PANEL_PAGE_NUMBER    4

UWORD PANEL_page[4][PANEL_PAGE_NUMBER] =
{
	{
		POLY_PAGE_IC_NORMAL,
		POLY_PAGE_IC_ALPHA,
		POLY_PAGE_IC_ADDITIVE,
		POLY_PAGE_IC_ALPHA_END
	},

	{
		POLY_PAGE_IC2_NORMAL,
		POLY_PAGE_IC2_ALPHA,
		POLY_PAGE_IC2_ADDITIVE,
		POLY_PAGE_IC2_ALPHA_END
	}
};



void PANEL_funky_quad(
		SLONG which,
		SLONG x,
		SLONG y,
		SLONG panel_page,
		ULONG colour,
		float width  = -1.0F,
		float height = -1.0F)
{
	float left;
	float top;
	float right;
	float bottom;

	SLONG page;

	ASSERT(WITHIN(which,      0, PANEL_IC_NUMBER   - 1));
	ASSERT(WITHIN(panel_page, 0, PANEL_PAGE_NUMBER - 1));

	PANEL_Ic *pi = &PANEL_ic[which];

	page = PANEL_page[pi->page][panel_page];

	if (which == PANEL_IC_SHOTGUN)
	{
		//
		// The shotgun is on its side.
		//

		left = float(x);
		top  = float(y);

		if (width  == -1.0F) {width  = (pi->v2 - pi->v1) * 256.0F;}
		if (height == -1.0F) {height = (pi->u2 - pi->u1) * 256.0F;}

		right  = left + width;
		bottom = top  + height;

		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		float fWDepthBodge = PANEL_GetNextDepthBodge();
		float fZDepthBodge = 1.0f - fWDepthBodge;



		pp[0].X        = left;
		pp[0].Y        = top;
		pp[0].z        = fZDepthBodge;
		pp[0].Z        = fWDepthBodge;
		pp[0].u        = pi->u2;
		pp[0].v        = pi->v1;
		pp[0].colour   = colour;
		pp[0].specular = 0xff000000;

		pp[1].X        = right;
		pp[1].Y        = top;
		pp[1].z        = fZDepthBodge;
		pp[1].Z        = fWDepthBodge;
		pp[1].u        = pi->u2;
		pp[1].v        = pi->v2;
		pp[1].colour   = colour;
		pp[1].specular = 0xff000000;

		pp[2].X        = left;
		pp[2].Y        = bottom;
		pp[2].z        = fZDepthBodge;
		pp[2].Z        = fWDepthBodge;
		pp[2].u        = pi->u1;
		pp[2].v        = pi->v1;
		pp[2].colour   = colour;
		pp[2].specular = 0xff000000;

		pp[3].X        = right;
		pp[3].Y        = bottom;
		pp[3].z        = fZDepthBodge;
		pp[3].Z        = fWDepthBodge;
		pp[3].u        = pi->u1;
		pp[3].v        = pi->v2;
		pp[3].colour   = colour;
		pp[3].specular = 0xff000000;

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		POLY_add_quad(quad, page, FALSE, TRUE);
	}
	else
	{
		left = float(x);
		top  = float(y);

		if (width  == -1.0F) {width  = (pi->u2 - pi->u1) * 256.0F;}
		if (height == -1.0F) {height = (pi->v2 - pi->v1) * 256.0F;}

		right  = left + width;
		bottom = top  + height;

		PANEL_draw_quad(
			left,
			top,
			right,
			bottom,
			page,
			colour,
			pi->u1,
			pi->v1,
			pi->u2,
			pi->v2);
	}
}



#ifdef PSX

//
// The global base of the funky panel (useful for splitscreen mode)
//

SLONG PANEL_funky_ybase;

//
// Where to draw bits of the panel.
//

#define PANEL_IC_BBX 30
#define PANEL_IC_BBY (PANEL_funky_ybase - 110)

#define PANEL_IC_PX 10
#define PANEL_IC_PY (PANEL_funky_ybase - 135)

#define PANEL_IC_GX 55
#define PANEL_IC_GY (PANEL_funky_ybase - 67)

#define PANEL_IC_AX 75
#define PANEL_IC_AY (PANEL_funky_ybase - 30)

#define PANEL_IC_MBX 50
#define PANEL_IC_MBY (PANEL_funky_ybase - 100)

#define PANEL_IC_SPLX 40
#define PANEL_IC_SPLY (PANEL_funky_ybase - 124)

#define PANEL_IC_DMX 600
#define PANEL_IC_DMY (PANEL_funky_ybase - 40)

#define PANEL_IC_MESX 140
#define PANEL_IC_MESY (PANEL_funky_ybase - 124)

#define PANEL_IC_CRX 320
#define PANEL_IC_CRY 60

#define PANEL_IC_SCANX (PANEL_IC_BBX + 64)
#define PANEL_IC_SCANY (PANEL_IC_BBY + 25)

#define PANEL_IC_BTX (PANEL_IC_BBX + 30)
#define PANEL_IC_BTY (PANEL_IC_BBY - 20)

#define PANEL_IC_CLIPX 120
#define PANEL_IC_CLIPY (PANEL_funky_ybase - 32)

#define PANEL_IC_GBX (130)
#define PANEL_IC_GBY (PANEL_funky_ybase - 50)

#endif

//
// The funky hearbeat. A circular buffer.
//

typedef struct
{
	float x;	// Normalised from  0 to 1
	float y;	// Normalised from -1 to 1
	ULONG colour;

} PANEL_Beat;

#define PANEL_NUM_BEATS 32

PANEL_Beat PANEL_beat[2][PANEL_NUM_BEATS];
SLONG      PANEL_beat_head[2];
ULONG      PANEL_beat_tick[2];
SLONG      PANEL_beat_last_ammo[2];
SLONG      PANEL_beat_last_specialtype[2];
float      PANEL_beat_x[2];



#if 0

//
// Processes and draws the heartbeat. Stamina goes from 0 to 1- where 1 means no
// stamina left... i.e. heart beating amazingly fast.
//

void PANEL_do_heartbeat(SLONG which, float stamina, SLONG death)
{
	SLONG i;
	SLONG b1;
	SLONG b2;

	float amp;
	float phase;

	ULONG beat_colour;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	PANEL_Beat *pb1;
	PANEL_Beat *pb2;

	SLONG r;
	SLONG g;
	SLONG c1;
	SLONG c2;
	SLONG mul;

	//
	// How fast the ticker sweeps across the display.
	//

	float dx = 0.02F + stamina * 0.03F;

	//
	// The amplitude of the two beats.
	//

	float amp1 = 0.6F + stamina * 0.3F;
	float amp2 = 0.3F + stamina * 0.5F;

	ULONG now = GetTickCount();

	//
	// Process the heatbeat 20 times a second. But no more than 4 times a
	// gameturn.
	//

	if (PANEL_beat_tick[which] < now - (1000 / 20) * 4)
	{
		PANEL_beat_tick[which] = now - (1000 / 20) * 4;
	}

	//
	// The colour fades from green to red depending on your health.
	//

	if (stamina < 0.5F)
	{
		beat_colour = 0x00ff00;
	}
	else
	if (stamina < 0.75F)
	{
		r = SLONG((stamina - 0.5F) * 255.0F / 0.25F);

		SATURATE(r, 0, 255);

		beat_colour = 0x00ff00 | (r << 16);
	}
	else
	{
		g = 255 - SLONG((stamina - 0.75f) * 255.0F / 0.25F);

		SATURATE(g, 0, 255);

		beat_colour = 0xff0000 | (g << 8);
	}

	//
	// Process the beats.
	//

	while(PANEL_beat_tick[which] < now)
	{
		PANEL_beat_tick[which] += 1000 / 20;
		PANEL_beat_head[which] += 1;
		PANEL_beat_head[which] &= PANEL_NUM_BEATS - 1;
		PANEL_beat_x[which]    += dx;

		if (PANEL_beat_x[which] > 1.0F)
		{
			PANEL_beat_x[which] -= 1.0F;
		}

		if (death)
		{
			//
			// The next beat is a dead beat. FLATLINE
			//

			PANEL_beat[which][PANEL_beat_head[which]].x      = PANEL_beat_x[which];
			PANEL_beat[which][PANEL_beat_head[which]].y      = 0;
			PANEL_beat[which][PANEL_beat_head[which]].colour = 0xff0000;
		}
		else
		{
			//
			// What is the amplitude here?
			//

			if (WITHIN(PANEL_beat_x[which], 0.1F, 0.5F))
			{
				amp  = 1.0F + (float)sin((-PI * 0.5F) + (PANEL_beat_x[which] - 0.1F) * (2.0F * PI / 0.4F));
				amp *= 0.5F * amp1;
			}
			else
			if (WITHIN(PANEL_beat_x[which], 0.6F, 0.9F))
			{
				amp  = 1.0F + (float)sin((-PI * 0.5F) + (PANEL_beat_x[which] - 0.6F) * (2.0F * PI / 0.3F));
				amp *= 0.5F * amp2;
			}
			else
			{
				amp = 0.00F;
			}

			//
			// How about phase?
			//

			phase = PANEL_beat_x[which] * 30.0F;

			//
			// The next beat.
			//

			PANEL_beat[which][PANEL_beat_head[which]].x      = PANEL_beat_x[which];
			PANEL_beat[which][PANEL_beat_head[which]].y      = sin(phase) * amp;
			PANEL_beat[which][PANEL_beat_head[which]].colour = beat_colour;
		}
	}

	//
	// Set up the beat quad.
	//

	pp[0].z        = 0.0F;
	pp[0].Z        = 1.0F;
	pp[0].u        = 214.0F / 256.0F;
	pp[0].v        =   2.0F / 256.0F;
	pp[0].specular = 0xff000000;

	pp[1].z        = 0.0F;
	pp[1].Z        = 1.0F;
	pp[1].u        = 214.0F / 256.0F;
	pp[1].v        =  13.0F / 256.0F;
	pp[1].specular = 0xff000000;

	pp[2].z        = 0.0F;
	pp[2].Z        = 1.0F;
	pp[2].u        = 220.0F / 256.0F;
	pp[2].v        =   2.0F / 256.0F;
	pp[2].specular = 0xff000000;

	pp[3].z        = 0.0F;
	pp[3].Z        = 1.0F;
	pp[3].u        = 220.0F / 256.0F;
	pp[3].v        =  13.0F / 256.0F;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Draw the beats now.
	//

	b1  = PANEL_beat_head[which];
	b2  = PANEL_beat_head[which] - 1;

	b1 &= PANEL_NUM_BEATS - 1;
	b2 &= PANEL_NUM_BEATS - 1;

	//
	// Mapping from beat space to screen space.
	//

	#define PANEL_BEAT_SX(x) (PANEL_IC_BBX + 35 + (x) * 55.0F)
	#define PANEL_BEAT_SY(y) (PANEL_IC_BBY + 23 + (y) * 15.0F)

	//#define PANEL_BEAT_SX(x) (PANEL_IC_BBX + 35  + (x) * 600.0F)
	//#define PANEL_BEAT_SY(y) (PANEL_IC_BBY - 123 + (y) * 100.0F)

	for (i = 0; i < PANEL_NUM_BEATS - 1; i++)
	{
		pb1 = &PANEL_beat[which][b1];
		pb2 = &PANEL_beat[which][b2];

		//
		// Work out the colours of the beats... fading away.
		//

		r = (pb1->colour >> 16) & 0xff;
		g = (pb1->colour >>  8) & 0xff;

		mul = PANEL_NUM_BEATS - i;

		r = r * mul >> 5;
		g = g * mul >> 5;

		c1 = (r << 16) | (g << 8);

		r = (pb2->colour >> 16) & 0xff;
		g = (pb2->colour >>  8) & 0xff;

		mul = PANEL_NUM_BEATS - i - 1;

		if (mul <= 0)
		{
			c2 = 0;
		}
		else
		{
			r = r * mul >> 5;
			g = g * mul >> 5;

			c2 = (r << 16) | (g << 8);
		}

		if (pb1->x > pb2->x)
		{
			pp[0].X        = PANEL_BEAT_SX(pb1->x);
			pp[0].Y        = PANEL_BEAT_SY(pb1->y) - 5.5F;
			pp[0].colour   = c1;

			pp[1].X        = PANEL_BEAT_SX(pb1->x);
			pp[1].Y        = PANEL_BEAT_SY(pb1->y) + 5.5F;
			pp[1].colour   = c1;

			pp[2].X        = PANEL_BEAT_SX(pb2->x);
			pp[2].Y        = PANEL_BEAT_SY(pb2->y) - 5.5F;
			pp[2].colour   = c2;

			pp[3].X        = PANEL_BEAT_SX(pb2->x);
			pp[3].Y        = PANEL_BEAT_SY(pb2->y) + 5.5F;
			pp[3].colour   = c2;

			POLY_add_quad(quad, POLY_PAGE_IC_ADDITIVE, FALSE, TRUE);
		}

		b1 -= 1;
		b2 -= 1;

		b1 &= PANEL_NUM_BEATS - 1;
		b2 &= PANEL_NUM_BEATS - 1;
	}
}

#endif

//
// The different types of ammo.
//

#define PANEL_AMMO_PISTOL  0
#define PANEL_AMMO_SHOTGUN 1
#define PANEL_AMMO_AK47	   2
#define PANEL_AMMO_GRENADE 3
#define PANEL_AMMO_NUMBER  4

typedef struct
{
	float width;
	float height;
	SLONG size_group;
	SLONG page_group;
	SLONG page_one;

} PANEL_Ammo;

PANEL_Ammo PANEL_ammo[PANEL_AMMO_NUMBER] =
{
	{
		5.0F,
		9.0F,
		10,
		PANEL_IC_PISTOL_AMMO_GROUP,
		PANEL_IC_PISTOL_AMMO_ONE
	},

	{
		5.0F,
		12.0F,
		8,
		PANEL_IC_SHOTGUN_AMMO_GROUP,
		PANEL_IC_SHOTGUN_AMMO_ONE
	},

	{
		5.0F,
		12.0F,
		10,
		PANEL_IC_AK47_AMMO_GROUP,
		PANEL_IC_AK47_AMMO_ONE,
	},

	{
		12.0F,
		18.0F,
		3,
		PANEL_IC_GRENADE_AMMO_GROUP,
		PANEL_IC_GRENADE_AMMO_ONE
	},
};



//
// Ammo that has been used up getting tossed off the screen.
//

typedef struct
{
	UWORD used;
	UWORD type;
	float x;
	float y;
	float angle;
	float dx;
	float dy;

} PANEL_Toss;

#define PANEL_MAX_TOSSES 8

PANEL_Toss PANEL_toss[PANEL_MAX_TOSSES];
SLONG      PANEL_toss_last;
ULONG      PANEL_toss_tick;


//
// A floating point number between 0 and 1.0F
//

static inline float frand(void)
{
	SLONG irand = rand();
	float ans   = float(irand) * (1.0F / float(RAND_MAX));

	return ans;
}

//
// Creates a new toss.
//

void PANEL_new_toss(
		SLONG type,
		float sx,
		float sy)
{
	PANEL_Toss *pt;
	PANEL_Ammo *pa;

	ASSERT(WITHIN(type, 0, PANEL_AMMO_NUMBER - 1));

	PANEL_toss_last += 1;
	PANEL_toss_last &= PANEL_MAX_TOSSES - 1;

	pt = &PANEL_toss[PANEL_toss_last];
	pa = &PANEL_ammo[type];

	pt->used  =  TRUE;
	pt->type  =  type;
	pt->x     =  sx + pa->width  * 0.5F;
	pt->y     =  sy + pa->height * 0.5F;
	pt->dx    =  3.5F + frand();
	pt->dy    = -5.0F - frand();
	pt->angle =  0.0F;
}

//
// Processes and tosses and draws them.
//

void PANEL_do_tosses(void)
{
	SLONG i;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	PANEL_Toss *pt;

	ULONG now = GetTickCount();

	//
	// Process 20 times a second but no more than 4 times a frame.
	//

	if (PANEL_toss_tick < now - (1000 / 20) * 4)
	{
		PANEL_toss_tick = now - (1000 / 20) * 4;
	}

	while(PANEL_toss_tick < now)
	{
		PANEL_toss_tick += 1000 / 20;

		for (i = 0; i < PANEL_MAX_TOSSES; i++)
		{
			pt = &PANEL_toss[i];

			if (pt->used)
			{
				pt->x     += pt->dx;
				pt->y     += pt->dy;
				pt->angle += 0.8F;
				pt->dx    *= 0.9F;
				pt->dy    += 1.5F;

				if (pt->y > 490.0F)
				{
					pt->used = FALSE;
				}
			}
		}
	}

	//
	// Set up the toss quad.
	//

	float fWDepthBodge = PANEL_GetNextDepthBodge();
	float fZDepthBodge = 1.0f - fWDepthBodge;

	pp[0].z        = fZDepthBodge;
	pp[0].Z        = fWDepthBodge;
	pp[0].colour   = 0xffffffff;
	pp[0].specular = 0xff000000;

	pp[1].z        = fZDepthBodge;
	pp[1].Z        = fWDepthBodge;
	pp[1].colour   = 0xffffffff;
	pp[1].specular = 0xff000000;

	pp[2].z        = fZDepthBodge;
	pp[2].Z        = fWDepthBodge;
	pp[2].colour   = 0xffffffff;
	pp[2].specular = 0xff000000;

	pp[3].z        = fZDepthBodge;
	pp[3].Z        = fWDepthBodge;
	pp[3].colour   = 0xffffffff;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Draw them.
	//

	{
		float ax;
		float ay;

		float bx;
		float by;

		SLONG page;

		PANEL_Ammo *pa;

		for (i = 0; i < PANEL_MAX_TOSSES; i++)
		{
			pt = &PANEL_toss[i];

			if (pt->used)
			{
				ASSERT(WITHIN(pt->type, 0, PANEL_AMMO_NUMBER - 1));

				pa = &PANEL_ammo[pt->type];

				ax = (float)sin(pt->angle) * pa->height * 0.5F;
				ay = (float)cos(pt->angle) * pa->height * 0.5F;

				bx = (float)sin(pt->angle + PI * 0.5F) * pa->width * 0.5F;
				by = (float)cos(pt->angle + PI * 0.5F) * pa->width * 0.5F;

				pp[0].X = pt->x - bx - ax;
				pp[0].Y = pt->y - by - ay;
				pp[0].u = PANEL_ic[pa->page_one].u1;
				pp[0].v = PANEL_ic[pa->page_one].v1;

				pp[1].X = pt->x + bx - ax;
				pp[1].Y = pt->y + by - ay;
				pp[1].u = PANEL_ic[pa->page_one].u2;
				pp[1].v = PANEL_ic[pa->page_one].v1;

				pp[2].X = pt->x - bx + ax;
				pp[2].Y = pt->y - by + ay;
				pp[2].u = PANEL_ic[pa->page_one].u1;
				pp[2].v = PANEL_ic[pa->page_one].v2;

				pp[3].X = pt->x + bx + ax;
				pp[3].Y = pt->y + by + ay;
				pp[3].u = PANEL_ic[pa->page_one].u2;
				pp[3].v = PANEL_ic[pa->page_one].v2;

				page = PANEL_page[PANEL_ic[pa->page_one].page][PANEL_PAGE_ALPHA_END];

				POLY_add_quad(quad, page, FALSE, TRUE);
			}
		}
	}
}
extern	UBYTE	estate;

//
// Draws a face at (x,y). The face is given by the Thing. NULL => Radio message.
//

#define PANEL_FACE_LARGE 1
#define PANEL_FACE_SMALL 2

void PANEL_new_face(
		Thing *who,
		float  x,
		float  y,
		SLONG  size)
{
	SLONG face;
	SLONG page;
	float u;
	float v;
	float width;
	float uvwidth;

	#define PANEL_FACE_RADIO		0
	#define PANEL_FACE_DARCI		1
	#define PANEL_FACE_ROPER		2
	#define PANEL_FACE_THUG_RASTA	3
	#define PANEL_FACE_THUG_GREY	4
	#define PANEL_FACE_THUG_RED		5
	#define PANEL_FACE_CIV_1		6
	#define PANEL_FACE_CIV_2		7
	#define PANEL_FACE_CIV_3		8
	#define PANEL_FACE_CIV_4		9
	#define PANEL_FACE_SLAG			10
	#define PANEL_FACE_FAT_SLAG		11
	#define PANEL_FACE_HOSTAGE		12
	#define PANEL_FACE_WORKMAN		13
	#define PANEL_FACE_BOSS_GOATIE	14
	#define PANEL_FACE_BOSS_SHAVEN	15
	#define PANEL_FACE_COP			16
	#define PANEL_FACE_COMMISSIONER 17
	#define PANEL_FACE_MIB			18
	#define PANEL_FACE_1920S_MAN	19
	#define PANEL_FACE_1920S_GIRL	20
	#define PANEL_FACE_TRAMP		21
	#define PANEL_FACE_NUMBER       22

	//
	// The position of the faces on each page given in 32 pixel steps.
	//

	typedef struct
	{
		UBYTE page;
		UBYTE u;
		UBYTE v;

	} PANEL_Face;

	PANEL_Face face_large[PANEL_FACE_NUMBER] =
	{
		{0,4,4},	// Radio
		{0,0,0},	// Darci
		{0,2,0},	// Roper
		{0,4,0},	// Thug rasta
		{1,0,4},	// Thug grey
		{0,4,2},	// Thug red
		{0,6,2},	// Civ1
		{0,6,4},	// Civ2
		{0,4,6},	// Civ3
		{0,6,6},	// Civ4
		{0,0,6},	// Slag
		{0,2,6},	// Fat ugly slag
		{0,2,4},	// Hostage
		{1,2,4},	// Workman,
		{0,6,0},	// Boss with a goatie
		{0,6,0},	// Boss with no goatie
		{0,0,4},	// Cop
		{0,6,0},	// Commissioner
		{0,0,2},	// MIB
		{1,2,6},	// 1920s man
		{1,4,6},	// 1920s girl
		{1,0,6},	// Tramp
	};

	PANEL_Face face_small[PANEL_FACE_NUMBER] =
	{
		{1,2,2},	// Radio
		{1,0,0},	// Darci
		{1,1,0},	// Roper
		{1,2,0},	// Thug rasta
		{1,4,0},	// Thug grey
		{1,2,1},	// Thug red
		{1,3,1},	// Civ1
		{1,3,2},	// Civ2
		{1,2,3},	// Civ3
		{1,3,3},	// Civ4
		{1,0,3},	// Slag
		{1,1,3},	// Fat ugly slag
		{1,1,2},	// Hostage
		{1,5,0},	// Workman,
		{1,3,0},	// Boss with a goatee
		{1,3,0},	// Boss with no goatee
		{1,0,2},	// Cop
		{1,0,3},	// Commissioner
		{1,0,1},	// MIB
		{1,6,0},	// 1920s man
		{1,7,0},	// 1920s girl
		{1,4,1},	// Tramp
	};

	if (who == NULL || who->Class != CLASS_PERSON)
	{
		face = PANEL_FACE_RADIO;
	}
	else
	{
		switch(who->Genus.Person->PersonType)
		{
			case PERSON_DARCI:
				face = PANEL_FACE_DARCI;
				break;

			case PERSON_ROPER:
				face = PANEL_FACE_ROPER;
				break;

			case PERSON_COP:
				face = PANEL_FACE_COP;
				break;

			case PERSON_CIV:

				switch(who->Draw.Tweened->MeshID)
				{
					case 7:

						//
						// Normal (i.e. non-wandering) civs.
						//

						switch(who->Draw.Tweened->PersonID - 6)
						{
							case 0:

								//
								// Pinhead.
								//

								face = PANEL_FACE_CIV_3;

								break;

							case 2:

								//
								// Oriental chap.
								//

								face = PANEL_FACE_CIV_4;

								break;

							case 1:
							case 3:

								//
								// Black bloke with moustache.
								//

								face = PANEL_FACE_CIV_1;

								break;

							default:
								ASSERT(0);
								break;
						}

						break;

					case 8:

						//
						// Male fake wandering civ.
						//

						face = PANEL_FACE_1920S_MAN;

						break;

					case 9:

						//
						// Female fake wandering civ.
						//

						face = PANEL_FACE_1920S_GIRL;

						break;

					default:
						ASSERT(0);
						break;
				}
				break;

			case PERSON_THUG_RASTA:
				face = PANEL_FACE_THUG_RASTA;
				break;

			case PERSON_THUG_GREY:
				face = PANEL_FACE_THUG_GREY;
				break;

			case PERSON_THUG_RED:
				face = PANEL_FACE_THUG_RED;
				break;

			case PERSON_SLAG_TART:
				face = PANEL_FACE_SLAG;
				break;

			case PERSON_SLAG_FATUGLY:
				face = PANEL_FACE_FAT_SLAG;
				break;

			case PERSON_HOSTAGE:
				face = PANEL_FACE_HOSTAGE;
				break;

			case PERSON_MECHANIC:
				face = PANEL_FACE_WORKMAN;
				break;

			case PERSON_TRAMP:
				if(estate)
				{
					face = PANEL_FACE_BOSS_GOATIE;
				}
				else
				{
					face = PANEL_FACE_TRAMP;

				}
				break;

			case PERSON_MIB1:
			case PERSON_MIB2:
			case PERSON_MIB3:
				face = PANEL_FACE_MIB; //I'm in disguise as a fat slag
				break;

			default:
				ASSERT(0);
				break;
		}
	}

	ASSERT(WITHIN(face, 0, PANEL_FACE_NUMBER - 1));

	PANEL_Face *pf;

	switch(size)
	{
		case PANEL_FACE_LARGE:
			pf      = &face_large[face];
			width   =  64;
			uvwidth =  64 / 256.0F;
			break;

		case PANEL_FACE_SMALL:
			pf      = &face_small[face];
			width   =  32;
			uvwidth =  32 / 256.0F;
			break;

		default:
			ASSERT(0);
			break;
	}

	u    = float(pf->u) * (32.0F / 256.0F);
	v    = float(pf->v) * (32.0F / 256.0F);
	page = (pf->page) ? POLY_PAGE_FACE2 : POLY_PAGE_FACE1;

	PANEL_draw_quad(
		x,
		y,
		x + width,
		y + width,
		page,
		0xffffffff,
		u,
		v,
		u + uvwidth,
		v + uvwidth);
}


//
// A circular queue of text messages.
//

// The French actually has a 257-character message. Grrrr.
#define PANEL_TEXT_MAX_LENGTH 300
typedef struct
{
	Thing *who;			// Who is saying the message. NULL => computer message
	CBYTE  text[PANEL_TEXT_MAX_LENGTH+2];
	SLONG  delay;		// 0 => unused.
	SLONG  turns;		// The number of turns this message has been alive for.

} PANEL_Text;

#define PANEL_MAX_TEXTS 8	// Power of 2 please!

PANEL_Text PANEL_text[PANEL_MAX_TEXTS];
SLONG      PANEL_text_head;	// Always acces PANEL_text[PANEL_text_head & (PANEL_MAX_TEXTS - 1)]
SLONG      PANEL_text_tail;	// Always acces PANEL_text[PANEL_text_tail & (PANEL_MAX_TEXTS - 1)]
ULONG      PANEL_text_tick;


void PANEL_new_text_init(void)
{
	memset(PANEL_text, 0, sizeof(PANEL_text));

	PANEL_text_head = 0;
	PANEL_text_tail = 0;
	PANEL_text_tick = 0;
}



void PANEL_new_text(Thing *who, SLONG delay, CBYTE *fmt, ...)
{
	CBYTE *ch;

	PANEL_Text *pt;

	//
	// Early out on NULL strings or strings with just spaces in them.
	//

	if (fmt == NULL)
	{
		return;
	}

	for (ch = fmt; *ch; ch++)
	{
		if (!isspace(*ch))
		{
			goto found_non_white_space;
		}
	}

	return;

  found_non_white_space:;

	//
	// Work out the real message.
	//

	CBYTE   message[1024];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	//
	// Make sure the message isn't too long.
	//

	if (strlen(message) >= PANEL_TEXT_MAX_LENGTH)
	{
		ASSERT(0);

		//
		// strlen doesn't include the NULL byte.
		//

		return;
	}

	/*

	//
	// Convert to uppercase.
	//

	for (ch = message; *ch; *ch++ = toupper(*ch));

	*/

	//
	// Do we already have this message?
	//

	SLONG i;

	for (i = 0; i < PANEL_MAX_TEXTS; i++)
	{
		pt = &PANEL_text[i];

		if (pt->delay)
		{
			if (pt->who == who)
			{
				if (strcmp(pt->text, message) == 0)
				{
					//
					// Don't put up the same message twice.
					//

					pt->delay = delay;

					return;
				}
			}
		}
	}


	//
	// Create a new message at the tail of the queue.
	//

	pt = &PANEL_text[PANEL_text_tail & (PANEL_MAX_TEXTS - 1)];

	pt->who   = who;
	pt->delay = delay;
	pt->turns = 0;

	strcpy(pt->text, message);

	//
	// Process the message so we put in vertical '|'s
	//

	PANEL_text_tail += 1;

	if (!who) MFX_play_ambient(0,S_RADIO_MESSAGE,0);

}

void PANEL_new_text_process()
{
	SLONG i;

	PANEL_Text *pt;

	ULONG now = GetTickCount();

	//
	// Process 20 times a second but no more than 4 times a frame.
	//

	if (PANEL_text_tick < now - (1000 / 10) * 4)
	{
		PANEL_text_tick = (now - (1000 / 10) * 4);
	}

	while(PANEL_text_tick < now)
	{
		PANEL_text_tick += 1000 / 10;

		for (i = 0; i < PANEL_MAX_TEXTS; i++)
		{
			pt = &PANEL_text[i];

			if (pt->delay)
			{
				pt->delay -= 1000 / 10;

				if (pt->delay < 0)
				{
					pt->delay = 0;
				}
			}
		}
	}

	for (i = 0; i < PANEL_MAX_TEXTS; i++)
	{
		pt = &PANEL_text[i];

		pt->turns += 1;
	}
}

#ifdef PSX
void PANEL_new_text_draw()
{
	SLONG i;
	SLONG ybase;
	SLONG y = PANEL_IC_MESY;
	SLONG height;

	PANEL_Text *pt;

	for (i = PANEL_text_head; i < PANEL_text_tail; i++)
	{
		pt = &PANEL_text[i & (PANEL_MAX_TEXTS - 1)];

		if (i == PANEL_text_head)
		{
			if (pt->delay == 0)
			{
				PANEL_text_head += 1;
			}
		}

		if (pt->delay != 0)
		{
			//
			// Draw this message. Start off with the face.
			//

			ybase = y;

			PANEL_new_face(
				pt->who,
				PANEL_IC_MESX,
				ybase - 3,
				PANEL_FACE_SMALL);

			/*

			PANEL_funky_quad(
				pt->face,
				PANEL_IC_MESX,
				ybase - 3,
				PANEL_PAGE_NORMAL,
				0xffffff);

			*/

			//
			// The text....
			//

			height  = FONT2D_DrawStringWrap(pt->text, PANEL_IC_MESX + 36 + 18, y, 0x00ff00) - y;
			height += 25;

			//
			// The speech bubble.
			//

			PANEL_funky_quad(
				PANEL_IC_BUBBLE_START,
				PANEL_IC_MESX + 36,
				ybase - 3,
				PANEL_PAGE_ALPHA,
				0xffffffff,
			    18.0F,
				height);

			PANEL_funky_quad(
				PANEL_IC_BUBBLE_MIDDLE,
				PANEL_IC_MESX + 36 + 18,
				ybase - 3,
				PANEL_PAGE_ALPHA,
				0xffffffff,
			    float(FONT2D_rightmost_x) - (PANEL_IC_MESX + 36 + 18),
				height);

			PANEL_funky_quad(
				PANEL_IC_BUBBLE_END,
				float(FONT2D_rightmost_x),
				ybase - 3,
				PANEL_PAGE_ALPHA,
				0xffffffff,
			   -1.0F,
				height);

			if (height < 34)
			{
				height = 34;
			}

			y += height;
		}
	}
}
#endif


//
// The help message.
//


CBYTE PANEL_help_message[256];
SLONG PANEL_help_timer;

void PANEL_new_help_message(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	va_list	ap;

	va_start(ap, fmt);
	vsprintf(PANEL_help_message, fmt, ap);
	va_end  (ap);

	//
	// How long the message lasts for.
	//

	PANEL_help_timer = 160 * 20 * 5;	// 5 seconds....
}

void PANEL_help_message_do()
{
	PANEL_help_timer -= 160 * TICK_RATIO >> TICK_SHIFT;

	if (PANEL_help_timer <= 0)
	{
		PANEL_help_timer = 0;
	}
	else
	{
		//
		// Draw the help text in a speech bubble at the top of the screen.
		//

		SLONG height;

		#define PANEL_IC_HELPX 10
		#define PANEL_IC_HELPY 10

		height  = FONT2D_DrawStringWrap(PANEL_help_message, PANEL_IC_HELPX + 18, PANEL_IC_HELPY, 0x00ff00) - PANEL_IC_HELPY;
		height += 25;

		//
		// The speech bubble.
		//

		PANEL_funky_quad(
			PANEL_IC_BUBBLE_START,
			PANEL_IC_HELPX,
			PANEL_IC_HELPY - 3,
			PANEL_PAGE_ALPHA,
			0xffffffff,
			18.0F,
			height);

		PANEL_funky_quad(
			PANEL_IC_BUBBLE_MIDDLE,
			PANEL_IC_HELPX + 18,
			PANEL_IC_HELPY - 3,
			PANEL_PAGE_ALPHA,
			0xffffffff,
			float(FONT2D_rightmost_x) - (PANEL_IC_HELPX + 18),
			height);

		PANEL_funky_quad(
			PANEL_IC_BUBBLE_END,
			float(FONT2D_rightmost_x),
			PANEL_IC_HELPY - 3,
			PANEL_PAGE_ALPHA,
			0xffffffff,
		   -1.0F,
			height);
	}
}




//
// The panel in widscreen cut-scene mode.
//

THING_INDEX PANEL_wide_top_person;
THING_INDEX PANEL_wide_bot_person;
SLONG       PANEL_wide_top_is_talking;	// TRUE/FALSE for who is talking now
CBYTE       PANEL_wide_text[256];

void PANEL_new_widescreen()
{
	//
	// We have to stop the clipping getting rid of whatever we are doing!
	//

	extern float POLY_screen_clip_top;
	extern float POLY_screen_clip_bottom;

	POLY_screen_clip_top    = 0.0F;
	POLY_screen_clip_bottom = float(DisplayHeight);

	//
	// Clear the widescreen borders to black.
	//

	PANEL_draw_quad(
		0.0F,
		0.0F,
		float(DisplayWidth),
		80.0F,
		POLY_PAGE_COLOUR,
		0x00000000);

	PANEL_draw_quad(
		0.0F,
		float(DisplayHeight) - 80.0F,
		float(DisplayWidth),
		float(DisplayHeight),
		POLY_PAGE_COLOUR,
		0x00000000);

	//
	// Who is talking?
	//

	if (EWAY_conversation_happening(
			&PANEL_wide_top_person,
			&PANEL_wide_bot_person))
	{
		//
		// Sort in thing number order!
		//

		if (PANEL_wide_top_person > PANEL_wide_bot_person)
		{
			SWAP(PANEL_wide_top_person, PANEL_wide_bot_person);
		}
	}

	//
	// Steal new_text messages!
	//

	SLONG i;

	PANEL_Text *pt;

	for (i = 0; i < PANEL_MAX_TEXTS; i++)
	{
		pt = &PANEL_text[i];

		if (pt->delay && pt->turns <= 1)
		{
			if (pt->who == NULL)
			{
				//
				// This is a message that goes on the bottom.
				//

				strcpy(PANEL_wide_text, pt->text);

				PANEL_wide_top_is_talking = FALSE;

				pt->delay = 0;
			}
			else
			{
				if (THING_NUMBER(pt->who) == PANEL_wide_top_person)
				{
					//
					// This is a message that goes on the top.
					//

					strcpy(PANEL_wide_text, pt->text);

					PANEL_wide_top_is_talking = TRUE;

					pt->delay = 0;
				}
				else
				{
					//
					// This is a message that goes on the bottom.
					//

					strcpy(PANEL_wide_text, pt->text);

					PANEL_wide_top_is_talking = FALSE;
					PANEL_wide_bot_person     = THING_NUMBER(pt->who);

					pt->delay = 0;
				}
			}
		}
	}

	//
	// Draw the two faces.
	//

	if (PANEL_wide_top_person != NULL)
	{
		PANEL_new_face(
			TO_THING(PANEL_wide_top_person),
			float(DisplayWidth) - 72.0F,
			8.0F,
			PANEL_FACE_LARGE);
	}

	//
	// Draw the text for who is speaking.
	//

	if (PANEL_wide_text[0])
	{
		PANEL_new_face(
			(PANEL_wide_bot_person) ? TO_THING(PANEL_wide_bot_person) : NULL,
			8.0F,
			float(DisplayHeight) - 72.0F,
			PANEL_FACE_LARGE);

		if (PANEL_wide_top_is_talking)
		{
			// Need to move this down so the bottom line is on the bottom of the black bit.
			// This keeps the first line out of the 32-pixel "danger zone" that may not be
			// shown on crap TV.

			SLONG iYpos = FONT2D_DrawStringRightJustify(
				PANEL_wide_text,
				DisplayWidth  - 80,
				0,
				0xff0ffff,
				256,
				POLY_PAGE_FONT2D,
				0,
				TRUE);
			iYpos = 75 - iYpos;
			FONT2D_DrawStringRightJustify(
				PANEL_wide_text,
				DisplayWidth  - 80,
				iYpos,
				0xff0ffff,
				256,
				POLY_PAGE_FONT2D,
				0,
				FALSE);
		}
		else
		{
			FONT2D_DrawStringWrap(
				PANEL_wide_text,
				80,
				DisplayHeight - 80 + 5,
				0xffffff);
		}
	}
}

#define PANEL_MAX_BEACON_COLOURS 12

ULONG PANEL_beacon_colour[PANEL_MAX_BEACON_COLOURS] =
{
	0xffff00,
	0xccff00,
	0x4488ff,
	0xff0000,
	0x00ff00,
	0x0000ff,
	0xff4400,
	0xffffff,
	0xff00ff,
	0xaaffaa,
	0x00ffff,
	0xff4488
};



#ifdef PSX

//
// Draws the map beacons...
//

void PANEL_draw_beacons(void)
{
	SLONG i;

	float dx;
	float dz;
	float dist;
	float dangle;
	float score;

	float ax;
	float ay;

	float bx;
	float by;

	float cx;
	float cy;

	POLY_Point  pp [3];
	POLY_Point *tri[3];

	MAP_Beacon *mb;

	ULONG colour;

	Thing *darci = NET_PERSON(0);

	SLONG best_beacon = NULL;
	float best_score  = float(INFINITY);

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			continue;
		}

		if (mb->track_thing)
		{
			Thing *p_track = TO_THING(mb->track_thing);

			mb->wx = p_track->WorldPos.X >> 8;
			mb->wz = p_track->WorldPos.Z >> 8;

			extern SLONG is_person_dead(Thing *p_person);

			if (p_track->Class == CLASS_PERSON && p_track->State == STATE_DEAD)
			{
				//
				// Don't draw beacons to dead people.
				//

				if (p_track->SubState == SUB_STATE_DEAD_INJURED)
				{
					//
					// Except if they're only injured...
					//
				}
				else
				{
					continue;
				}
			}
		}

		colour = PANEL_beacon_colour[i % PANEL_MAX_BEACON_COLOURS] | (0xff000000);

		//
		// Work out the distance and relative angle of this beacon from darci.
		//

		dx = float(mb->wx - (darci->WorldPos.X >> 8));
		dz = float(mb->wz - (darci->WorldPos.Z >> 8));

		dist = fabs(dx) + fabs(dz);

		if (dist < 256.0F * 4.0F)
		{
			score  = dist * (1.0F / (256.0F * 4.0F));
			score *= 2.0F * PI * 22.0F / 360.0F;

			PANEL_funky_quad(
				PANEL_IC_DOT,
				PANEL_IC_SCANX - 7.0F,
				PANEL_IC_SCANY - 7.0F,
				PANEL_PAGE_ALPHA_END,
				colour);
		}
		else
		{
			if (PANEL_scanner_poo)
				dangle = (float)atan2(dx,dz) - float(darci->Draw.Tweened->Angle) * (2.0F * PI / 2048.0F);
			else
				dangle = (float)atan2(dx,dz) - float(FC_cam[0].yaw>>8) * (2.0F * PI / 2048.0F);
			score  = (float)fmod(dangle, 2.0F * PI) - PI;

			if (score > +PI) {score -= 2.0F * PI;}
			if (score < -PI) {score += 2.0F * PI;}

			//
			// The arrow positions...
			//

			ax = (float)sin(dangle - 0.5F) * 33.0F * 1.5F;
			ay = (float)cos(dangle - 0.5F) * 20.0F * 1.5F;

			bx = (float)sin(dangle + 0.5F) * 33.0F * 1.5F;
			by = (float)cos(dangle + 0.5F) * 20.0F * 1.5F;

			cx = (float)sin(dangle) * 33.0F * -0.25F;
			cy = (float)cos(dangle) * 20.0F * -0.25F;

			//
			// Draw an arrow in this direction.
			//

			float fWDepthBodge = PANEL_GetNextDepthBodge();
			float fZDepthBodge = 1.0f - fWDepthBodge;


			pp[0].X        = PANEL_IC_SCANX + cx;
			pp[0].Y        = PANEL_IC_SCANY + cy;
			pp[0].z        = fZDepthBodge;
			pp[0].Z        = fWDepthBodge;
			pp[0].u        = 134.0F / 256.0F;
			pp[0].v        = 189.0F / 256.0F;
			pp[0].colour   = colour;
			pp[0].specular = 0xff000000;

			pp[1].X        = PANEL_IC_SCANX + ax;
			pp[1].Y        = PANEL_IC_SCANY + ay;
			pp[1].z        = fZDepthBodge;
			pp[1].Z        = fWDepthBodge;
			pp[1].u        = 153.0F / 256.0F;
			pp[1].v        = 180.0F / 256.0F;
			pp[1].colour   = colour;
			pp[1].specular = 0xff000000;

			pp[2].X        = PANEL_IC_SCANX + bx;
			pp[2].Y        = PANEL_IC_SCANY + by;
			pp[2].z        = fZDepthBodge;
			pp[2].Z        = fWDepthBodge;
			pp[2].u        = 153.0F / 256.0F;
			pp[2].v        = 198.0F / 256.0F;
			pp[2].colour   = colour;
			pp[2].specular = 0xff000000;

			tri[0] = &pp[0];
			tri[1] = &pp[1];
			tri[2] = &pp[2];

			POLY_add_triangle(tri, POLY_PAGE_IC2_ALPHA_END, FALSE, TRUE);
		}

		if (fabs(score) < best_score)
		{
			best_score  = fabs(score);
			best_beacon = i;
		}
	}

	if (best_beacon)
	{
		ASSERT(WITHIN(best_beacon, 1, MAP_MAX_BEACONS - 1));

		mb = &MAP_beacon[best_beacon];

		extern CBYTE *EWAY_get_mess(SLONG index);

		FONT2D_DrawString(
			EWAY_get_mess(mb->index),
			PANEL_IC_BTX + 2,
			PANEL_IC_BTY + 2,
			0x00000000,
			192);

		FONT2D_DrawString(
			EWAY_get_mess(mb->index),
			PANEL_IC_BTX,
			PANEL_IC_BTY,
			PANEL_beacon_colour[best_beacon % PANEL_MAX_BEACON_COLOURS],
			192);

	}
}

#endif


#define PANEL_WHO_DARCI 0
#define PANEL_WHO_ROPER 1

#ifdef PSX
void PANEL_new_funky_do(SLONG which, SLONG where)
{
	if (EWAY_stop_player_moving())
	{
		//
		// There is a widescreen cutscene playing.
		//

		PANEL_new_widescreen();

		return;
	}
	else
	{
		PANEL_wide_top_person     = NULL;
		PANEL_wide_bot_person     = NULL;
		PANEL_wide_top_is_talking = FALSE;
		PANEL_wide_text[0]        = '\000';
	}

	SLONG i;

	ULONG health_colour;
	SLONG which_gun;
	SLONG which_ammo;
	SLONG which_dx = 0;
	SLONG which_dy = 0;
	SLONG clip_which  = 0;
	SLONG clip_number = 0;

	SLONG who;			// One of PANEL_WHO_*
	SLONG health;		// From 0 to 256
	SLONG stamina;		// From 0 to 256
	SLONG specialtype;	// The gun in use or NULL if not using a special,
	SLONG ammo;			// The amount of ammo used by the gun

	ASSERT(WITHIN(which, 0, 1));

	Thing *darci  = NET_PERSON(which);
	Thing *player = NET_PLAYER(which);
	Thing *p_special;

	if (darci->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialUse);

		specialtype = p_special->Genus.Special->SpecialType;
		ammo        = p_special->Genus.Special->ammo;
	}
	else
	{
		specialtype = NULL;
		ammo        = 0;
	}

	if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		specialtype = SPECIAL_GUN;
		ammo        = darci->Genus.Person->Ammo;
	}

	who     = (darci->Genus.Person->PersonType == PERSON_ROPER) ? PANEL_WHO_ROPER : PANEL_WHO_DARCI;
	health  =  darci->Genus.Person->Health;
	stamina =  darci->Genus.Person->Stamina;

	//
	// Set where the bottom of the panel should be.
	//

	PANEL_funky_ybase = (where & 1) ? (DisplayHeight >> 1) : DisplayHeight;

	//
	// The background and the character.
	//

	PANEL_funky_quad(
		PANEL_IC_BACKBOX,
		PANEL_IC_BBX,
		PANEL_IC_BBY,
		PANEL_PAGE_NORMAL,
		0xffffffff);

	PANEL_funky_quad(
		(who == PANEL_WHO_DARCI) ? PANEL_IC_DARCI : PANEL_IC_ROPER,
		PANEL_IC_PX,
		PANEL_IC_PY,
		PANEL_PAGE_ALPHA,
		0xffffffff);

	//
	// The colour we draw the health outline.
	//

	{
		SLONG r = 0;
		SLONG g = 0;
		SLONG b = 0;

		if (health > 133)
		{
			r = 255 - ((health - 133) * 255) / 67;
			g = 255;
		}
		else
		if (health < 66)
		{
			r = 100 + (health * 155) / 66;
			g = 0;
		}
		else
		{
			r = 255;
			g = ((health - 66) * 255) / 66;
		}

		SATURATE(r, 0, 255);
		SATURATE(g, 0, 255);

		health_colour = (r << 16) | (g << 8) | 0x10;	// A little bit of blue...
	}

	PANEL_funky_quad(
		(who == PANEL_WHO_DARCI) ? PANEL_IC_DARCI_OUTLINE : PANEL_IC_ROPER_OUTLINE,
		PANEL_IC_PX - 6,
		PANEL_IC_PY - 3,
		PANEL_PAGE_ADDITIVE,
		health_colour);

	#if WEVE_REPLACED_THE_HEARTBEAT_WITH_A_SCANNER

	//
	// No heart beat any more :(
	//

	PANEL_do_heartbeat(which, 1.0F - float(stamina) * (1.0F / 256.0F), darci->State == STATE_DEAD);

	#endif

	//
	// If the player is holding a grenade that has had its pin pulled then
	// show how long the grenade has before it goes off.
	//

	{
		Thing *p_special;

		if (darci->Genus.Person->SpecialUse)
		{
			p_special = TO_THING(darci->Genus.Person->SpecialUse);

			if (p_special->Genus.Special->SpecialType == SPECIAL_GRENADE &&
				p_special->SubState                   == SPECIAL_SUBSTATE_ACTIVATED)
			{
				//
				// Alert! The player is holding a primed grenade!
				//

				ULONG colour;
				SLONG secsleft;


				secsleft = p_special->Genus.Special->timer / (16 * 20) + 1;

				if (secsleft > 6)
				{
					secsleft = 6;
				}

				switch(secsleft)
				{
					case 0:
					case 1:
						colour = 0xff3300;
						break;

					case 2:
						colour = 0xff8800;
						break;

					case 3:
						colour = 0x88ff00;
						break;

					default:
						colour = 0x00ff00;
						break;
				}

				PANEL_funky_quad(
					PANEL_IC_DIGIT_0 + secsleft,
					PANEL_IC_MBX,
					PANEL_IC_MBY,
					PANEL_PAGE_ADDITIVE,
					colour);
			}
		}
	}

	//
	// Which gun shall we draw.
	//

	switch(specialtype)
	{
		default:

			if (who == PANEL_WHO_DARCI)
			{
				which_gun = PANEL_IC_DARCI_HAND;
			}
			else
			{
				which_gun = PANEL_IC_ROPER_HAND;
			}

			which_ammo = -1;

			break;

		case SPECIAL_GUN:
			which_gun   = PANEL_IC_PISTOL;
			which_ammo  = PANEL_AMMO_PISTOL;
			clip_which  = PANEL_IC_CLIP_PISTOL;
			clip_number = darci->Genus.Person->ammo_packs_pistol;
			break;

		case SPECIAL_AK47:
			which_gun   =  PANEL_IC_AK47;
			which_ammo  =  PANEL_AMMO_AK47;
			which_dx    = -45;
			clip_which  =  PANEL_IC_CLIP_AK47;
			clip_number =  darci->Genus.Person->ammo_packs_ak47;
			break;

		case SPECIAL_BASEBALLBAT:
			which_gun  =  PANEL_IC_BASEBALLBAT;
			which_ammo = -1;
			which_dx   = -50;
			break;

		case SPECIAL_KNIFE:
			which_gun  = PANEL_IC_KNIFE;
			which_ammo = -1;
			which_dy   =  15;
			break;

		case SPECIAL_SHOTGUN:
			which_gun   =  PANEL_IC_SHOTGUN;
			which_ammo  =  PANEL_AMMO_SHOTGUN;
			which_dx    = -35;
			which_dy    =  10;
			clip_which  =  PANEL_IC_CLIP_SHOTGUN;
			clip_number =  darci->Genus.Person->ammo_packs_shotgun;
			break;

		case SPECIAL_GRENADE:
			which_gun  =  PANEL_IC_GRENADE;
			which_ammo =  PANEL_AMMO_GRENADE;
			break;

		case SPECIAL_EXPLOSIVES:
			which_gun  =  PANEL_IC_EXPLOSIVES;
			which_ammo =  PANEL_AMMO_GRENADE;
			break;
	}

	PANEL_funky_quad(
		which_gun,
		PANEL_IC_GX + which_dx,
		PANEL_IC_GY + which_dy,
		PANEL_PAGE_ALPHA_END,
		0xffffffff);

	if (which_ammo == -1)
	{
		//
		// This weapon has no ammo.
		//
	}
	else
	{
		SLONG ax = PANEL_IC_AX;
		SLONG ay = PANEL_IC_AY;

		ASSERT(WITHIN(which_ammo, 0, PANEL_AMMO_NUMBER - 1));

		PANEL_Ammo *pa = &PANEL_ammo[which_ammo];

		while(ammo >= pa->size_group)
		{
			PANEL_funky_quad(
				pa->page_group,
				ax,
				ay,
				PANEL_PAGE_ALPHA_END,
				0xffffffff);

			ax += 2;
			ay += 7;

			ammo -= pa->size_group;
		}

		for (i = 0; i < ammo; i++)
		{
			PANEL_funky_quad(
				pa->page_one,
				ax,
				ay,
				PANEL_PAGE_ALPHA_END,
				0xffffffff);

			ax += pa->width;
		}

		if (PANEL_beat_last_specialtype[which] == specialtype && PANEL_beat_last_ammo[which] > ammo)
		{
			while(i < PANEL_beat_last_ammo[which])
			{
				PANEL_new_toss(
					which_ammo,
					float(ax),
					float(ay));

				ax += pa->width;

				if (i % pa->size_group == 0)
				{
					ax -= pa->width * pa->size_group - 2;
					ay += 7;
				}
				else
				{
				}

				i  += 1;
			}
		}
	}


	if (clip_number)
	{
		PANEL_funky_quad(
			clip_which,
			PANEL_IC_CLIPX,
			PANEL_IC_CLIPY,
			PANEL_PAGE_ALPHA_END,
			0xffffffff);

		{
			CBYTE text[16];

			sprintf(text, "x%d", clip_number);

extern void FONT2D_DrawString_NoTrueType(CBYTE*str, SLONG x, SLONG y, ULONG rgb, SLONG scale, SLONG page, SWORD fade);

			FONT2D_DrawString_NoTrueType(
				text,
				PANEL_IC_CLIPX + 20,
				PANEL_IC_CLIPY + 4,
				0x00ff00,
				256,
				POLY_PAGE_FONT2D,
				0);
		}
	}

//  skip_the_gun_and_ammo:;

	PANEL_do_tosses();

	PANEL_beat_last_specialtype[which] = specialtype;
	PANEL_beat_last_ammo[which]        = ammo;

	//
	// The grimreapers for the number of civs you've killed.
	//

	{
		float x;

		x = PANEL_IC_SPLX;

		for (i = 0; i < player->Genus.Player->RedMarks; i++)
		{
			PANEL_funky_quad(
				PANEL_IC_GRIMREAPER,
				x,
				PANEL_IC_SPLY,
				PANEL_PAGE_ALPHA_END,
				0xffffffff);

			x += 20.0F;

			if (i == 4)
			{
				x += 12.0F;
			}
		}
	}

	//
	// The danger meter.
	//

	{
		ULONG danger_colour;

		switch(player->Genus.Player->Danger)
		{
			case 0:
				goto no_danger_meter;

			case 1: danger_colour = 0xffff0000; break;
			case 2: danger_colour = 0xffffdd00; break;
			case 3: danger_colour = 0xff00dd00; break;

			default:
				break;
		}

		PANEL_funky_quad(
			PANEL_IC_DANGERMETER,
			PANEL_IC_DMX,
			PANEL_IC_DMY,
			PANEL_PAGE_ALPHA,
			danger_colour);
	}

  no_danger_meter:;

	//
	// The crime rate.
	//

	if (GAME_FLAGS & GF_SHOW_CRIMERATE)
	{
		CBYTE crimerate[128];

		sprintf(crimerate, "%d%% ", CRIME_RATE);

		FONT2D_DrawStringCentred(
			crimerate,
			PANEL_IC_CRX,
			PANEL_IC_CRY,
			0x00ff00);

		FONT2D_DrawStringCentred(
			XLAT_str(X_CRIME_RATE,crimerate),
			PANEL_IC_CRX,
			PANEL_IC_CRY - 20,
			0x00ff00);
	}

	//
	// The text.
	//

	PANEL_new_text_process();
	PANEL_new_text_draw();
	PANEL_help_message_do();

	if (darci->Genus.Person->Flags & FLAG_PERSON_DRIVING)
	{
		//
		// Draw the gear you are in.
		//

		Thing *p_vehicle = TO_THING(darci->Genus.Person->InCar);

		PANEL_funky_quad(
			PANEL_IC_GEAR_BOX,
			PANEL_IC_GBX,
			PANEL_IC_GBY,
			PANEL_PAGE_ALPHA,
			0xffffffff);

		if (p_vehicle->Genus.Vehicle->Siren)
		{
			//
			// High gear.
			//

			PANEL_funky_quad(
				PANEL_IC_GEAR_HIGH,
				PANEL_IC_GBX + 25 -  3,
				PANEL_IC_GBY + 18 - 21,
				PANEL_PAGE_ALPHA_END,
				0xffffffff);
		}
		else
		{
			//
			// Low gear.
			//

			PANEL_funky_quad(
				PANEL_IC_GEAR_LOW,
				PANEL_IC_GBX + 25 - 17,
				PANEL_IC_GBY + 18 - 20,
				PANEL_PAGE_ALPHA_END,
				0xffffffff);
		}
	}

	//
	// The on-screen map beacons.
	//

	PANEL_draw_beacons();
}

void PANEL_new_funky()
{
	//
	// We have to stop the clipping getting rid of whatever we are doing!
	//

	extern float POLY_screen_clip_top;
	extern float POLY_screen_clip_bottom;

	POLY_screen_clip_top    = 0.0F;
	POLY_screen_clip_bottom = float(DisplayHeight);

	if (FC_cam[1].focus)
	{
		//
		// We are in splitscreen mode.
		//

		PANEL_new_funky_do(0,1);
//		PANEL_new_funky_do(1,0);
	}
	else
	{
		//
		// One player mode.
		//

		PANEL_new_funky_do(0,0);
	}
}
#endif //#ifdef PSX


void	PANEL_start(void)
{
#ifndef TARGET_DC
	POLY_frame_init(FALSE, FALSE);
#endif
}

void	PANEL_finish(void)
{
#ifndef TARGET_DC
	POLY_frame_draw(TRUE,TRUE);
#endif
}


// ========================================================
//
// FADEOUT STUFF
//
// ========================================================

SLONG PANEL_fadeout_time; // 0 => No fadeout currently active.


void PANEL_fadeout_init()
{
	PANEL_fadeout_time = 0;
}

void PANEL_fadeout_start()
{
	if (!PANEL_fadeout_time)
	{
		PANEL_fadeout_time = GetTickCount();
	}
}

float angle_mul = 0.004F;
float zoom_mul  = 0.500F;

void PANEL_fadeout_draw()
{
	if (PANEL_fadeout_time)
	{
#ifndef TARGET_DC
		POLY_frame_init(FALSE,FALSE);
#endif

		//
		// Make the fadeout zoom in and turn.
		//

		float angle = float(GetTickCount() - PANEL_fadeout_time) * angle_mul;
		float zoom  = angle * zoom_mul;

		float xdu = -(float)cos(angle) * zoom * 1.33F;
		float xdv =  (float)sin(angle) * zoom * 1.33F;

		float ydu =  (float)sin(angle) * zoom;
		float ydv =  (float)cos(angle) * zoom;

		SLONG colour;

		colour = 0xffffffff;

		//
		// Add a single crappy poly!
		//

		POLY_Point  pp  [4];
		POLY_Point *quad[4];

		float fWDepthBodge = PANEL_GetNextDepthBodge();
		float fZDepthBodge = 1.0f - fWDepthBodge;

		pp[0].X        = 0.0F;
		pp[0].Y        = 0.0F;
		pp[0].z        = fZDepthBodge;
		pp[0].Z        = fWDepthBodge;
		pp[0].u        = 0.5F - xdu - ydu;
		pp[0].v        = 0.5F - xdv - ydv;
		pp[0].colour   = colour;
		pp[0].specular = 0xff000000;

		pp[1].X        = 640.0F;
		pp[1].Y        = 0.0F;
		pp[1].z        = fZDepthBodge;
		pp[1].Z        = fWDepthBodge;
		pp[1].u        = 0.5F + xdu - ydu;
		pp[1].v        = 0.5F + xdv - ydv;
		pp[1].colour   = colour;
		pp[1].specular = 0xff000000;

		pp[2].X        = 0.0F;
		pp[2].Y        = 480.0F;
		pp[2].z        = fZDepthBodge;
		pp[2].Z        = fWDepthBodge;
		pp[2].u        = 0.5F - xdu + ydu;
		pp[2].v        = 0.5F - xdv + ydv;
		pp[2].colour   = colour;
		pp[2].specular = 0xff000000;

		pp[3].X        = 640.0F;
		pp[3].Y        = 480.0F;
		pp[3].z        = fZDepthBodge;
		pp[3].Z        = fWDepthBodge;
		pp[3].u        = 0.5F + xdu + ydu;
		pp[3].v        = 0.5F + xdv + ydv;
		pp[3].colour   = colour;
		pp[3].specular = 0xff000000;

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		POLY_add_quad(quad, POLY_PAGE_FADECAT, FALSE, TRUE);

		//
		// Darken the screen at the end.
		//

		if (GetTickCount() > (unsigned)PANEL_fadeout_time + 768)
		{
			SLONG bright;

			//
			// Fadeout the colour.
			//

			bright = GetTickCount() - (PANEL_fadeout_time + 768);

			SATURATE(bright, 0, 255);

			colour = bright << 24;

#ifdef TARGET_DC
			// On the DC, there's a hardware bug that means the UV clamp down't quite work.
			// So we need to darken the whole screen this way.
			PANEL_draw_quad(
				0.0f,
				0.0f,
				640.0f,
				480.0f,
				POLY_PAGE_ALPHA,
				colour);
#else
			#define PANEL_BLACKEN_WIDTH 70

			PANEL_draw_quad(
				320.0F - PANEL_BLACKEN_WIDTH,
				240.0F - PANEL_BLACKEN_WIDTH,
				320.0F + PANEL_BLACKEN_WIDTH,
				240.0F + PANEL_BLACKEN_WIDTH,
				POLY_PAGE_ALPHA,
				colour);
#endif
		}

#ifndef TARGET_DC
		POLY_frame_draw(FALSE,FALSE);
#endif
	}
}

SLONG PANEL_fadeout_finished()
{
	if (PANEL_fadeout_time)
	{
		if (GetTickCount() > (unsigned)PANEL_fadeout_time + 1024)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//
// The coordinates of each sprite
//

#define PANEL_LSPRITE_BACKGROUND 0
#define PANEL_LSPRITE_AK47		 1
#define PANEL_LSPRITE_SHOTGUN	 2
#define PANEL_LSPRITE_LOW_GEAR	 3
#define PANEL_LSPRITE_HIGH_GEAR	 4
#define PANEL_LSPRITE_GRENADE	 5
#define PANEL_LSPRITE_ARROW		 6
#define PANEL_LSPRITE_QMARK		 7
#define PANEL_LSPRITE_DOT		 8
#define PANEL_LSPRITE_PISTOL	 9
#define PANEL_LSPRITE_TEXT_BOX	 10
#define PANEL_LSPRITE_BBB		 11
#define PANEL_LSPRITE_FIST       12
#define PANEL_LSPRITE_EXPLOSIVES 13
#define PANEL_LSPRITE_KNIFE      14
#define PANEL_LSPRITE_NUMBER     15

typedef struct
{
	SLONG page;
	float u1;
	float v1;
	float u2;
	float v2;

} PANEL_Lsprite;

/*
PANEL_Lsprite PANEL_lsprite[PANEL_LSPRITE_NUMBER] =
{
	#define PLS(a,b,c,d) {float(a) / 256.0F, float(b) / 256.F, float(c) / 256.F, float(d) / 256.F}

	#if THE_REAL_POSITIONS

	PLS(0,114, 142,256),
	PLS(1,1, 76,44),
	PLS(0,46, 77,87),
	PLS(77,1, 149,51),
	PLS(78,56, 148,106),
	PLS(182,32, 220,79),	// Grenade
	PLS(223,42, 247,68),
	PLS(226,76, 246,103),
	PLS(189,83, 211,105),	// Dot
	PLS(186,106, 256,158),
	PLS(144,112, 180,148),
	PLS(145,157, 217,204),	// BBB
	PLS(219,160, 255,204),
	PLS(146, 207, 195,246),
	PLS(1,95, 75,111),

	#else

	//
	// Made all coordinates even.
	//

	PLS(0,114, 142,256),
	PLS(0,0, 76,44),
	PLS(0,46, 78,88),
	PLS(76,0, 150,52),
	PLS(78,56, 148,106),
	PLS(182,32, 220,80),	// Grenade
	PLS(222,42, 248,68),
	PLS(226,76, 246,104),
	PLS(188,82, 212,106),	// Dot
	PLS(186,106, 256,158),
	PLS(144,112, 180,148),
//	PLS(144,156, 218,204),	// BBB
	PLS(145,157, 219,205),	// BBB 2
	PLS(218,160, 256,204),
	PLS(146, 206, 196,246),
	PLS(0,94, 76,112),

	#endif
};
*/

PANEL_Lsprite PANEL_lsprite[PANEL_LSPRITE_NUMBER] =
{
#define PLS(p, a,b,c,d) { p, float(a) / 256.0F, float(b) / 256.F, float(c) / 256.F, float(d) / 256.F}

	/*

	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,  90, 212, 256),			// display
	PLS(POLY_PAGE_LASTPANEL_ALPHA,  70,  50, 142,  90),			// AK
	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,  50,  70,  88),			// Shotgun
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 174,   0, 246,  50),			// LoGear
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 104,   0, 174,  50),			// HiGear
	PLS(POLY_PAGE_LASTPANEL_ALPHA,  70,   0, 104,  46),			// Grenade
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 224, 102, 242, 114),			// Arrow
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 226,  76, 246, 104),			// ?
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 230, 176, 238, 184),			// Dot
	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,   0,  70,  50),			// Pistol
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 218, 128, 250, 160),			// TxtBox
	PLS(POLY_PAGE_LASTPANEL2_ALPHA,  0, 122,  72, 168),			// BBB
	PLS(POLY_PAGE_LASTPANEL2_ALPHA,126, 122, 162, 168),			// Fist
	PLS(POLY_PAGE_LASTPANEL2_ALPHA, 72, 122, 120, 162),			// splosive
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 140,  62, 212,  84),			// knife

	*/

	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,  90, 212, 256),			// display
	PLS(POLY_PAGE_LASTPANEL_ALPHA,  70,  52, 141,  89),			// AK
	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,  52,  69,  88),			// Shotgun
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 175,   0, 246,  50),			// LoGear
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 104,   0, 174,  50),			// HiGear
	PLS(POLY_PAGE_LASTPANEL_ALPHA,  70,   0, 104,  46),			// Grenade
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 224, 102, 242, 114),			// Arrow
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 226,76, 246,103),			// ?
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 230, 176, 238, 184),			// Dot
	PLS(POLY_PAGE_LASTPANEL_ALPHA,   0,   0,  69,  50),			// Pistol
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 218, 128, 250, 160),			// TxtBox
	PLS(POLY_PAGE_LASTPANEL2_ALPHA,  0, 124,  71, 168),			// BBB
	PLS(POLY_PAGE_LASTPANEL2_ALPHA,126, 124, 162, 168),			// Fist
	PLS(POLY_PAGE_LASTPANEL2_ALPHA, 72, 123, 120, 161),			// splosive
	PLS(POLY_PAGE_LASTPANEL_ALPHA, 141,  62, 212,  84),			// knife

};



//
// Draws an arrow.
//

void PANEL_last_arrow(float x, float y, float angle, float size, ULONG colour, UBYTE is_dot=0)
{
	PANEL_Lsprite *pls;
	if (is_dot)
		pls = &PANEL_lsprite[PANEL_LSPRITE_DOT];
	else
		pls = &PANEL_lsprite[PANEL_LSPRITE_ARROW];

	float dx = sin(angle) * size;
	float dy = cos(angle) * size;

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	float fWDepthBodge = PANEL_GetNextDepthBodge();
	float fZDepthBodge = 1.0f - fWDepthBodge;

	pp[0].X        = x + dx + (-dy);
	pp[0].Y        = y + dy + (+dx);
	pp[0].z        = fZDepthBodge;
	pp[0].Z        = fWDepthBodge;
	pp[0].x        = 0.0F;
	pp[0].y        = 0.0F;
	pp[0].u        = pls->u1;
	pp[0].v        = pls->v1;
	pp[0].colour   = colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = x + dx - (-dy);
	pp[1].Y        = y + dy - (+dx);
	pp[1].z        = fZDepthBodge;
	pp[1].Z        = fWDepthBodge;
	pp[1].x        = 0.0F;
	pp[1].y        = 0.0F;
	pp[1].u        = pls->u2;
	pp[1].v        = pls->v1;
	pp[1].colour   = colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = x - dx + (-dy);
	pp[2].Y        = y - dy + (+dx);
	pp[2].z        = fZDepthBodge;
	pp[2].Z        = fWDepthBodge;
	pp[2].x        = 0.0F;
	pp[2].y        = 0.0F;
	pp[2].u        = pls->u1;
	pp[2].v        = pls->v2;
	pp[2].colour   = colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = x - dx - (-dy);
	pp[3].Y        = y - dy - (+dx);
	pp[3].z        = fZDepthBodge;
	pp[3].Z        = fWDepthBodge;
	pp[3].x        = 0.0F;
	pp[3].y        = 0.0F;
	pp[3].u        = pls->u2;
	pp[3].v        = pls->v2;
	pp[3].colour   = colour;
	pp[3].specular = 0xff000000;

	POLY_add_quad(quad, pls->page, FALSE, TRUE);
}


void PANEL_last_bubble(float x1, float y1, float x2, float y2)
{
	POLY_Point pp[16];

	#define SET_PP(qn, qx,qy, qu,qv)			\
	{											\
		pp[qn].x        = 0.0F;					\
		pp[qn].y        = 0.0F;					\
		pp[qn].X        = qx;					\
		pp[qn].Y        = qy;					\
		pp[qn].z        = fZDepthBodge;			\
		pp[qn].Z        = fWDepthBodge;			\
		pp[qn].u        = qu;					\
		pp[qn].v        = qv;					\
		pp[qn].colour   = 0xffffffff;			\
		pp[qn].specular = 0xff000000;			\
	}

	float fWDepthBodge = PANEL_GetNextDepthBodge();
	float fZDepthBodge = 1.0f - fWDepthBodge;

	PANEL_Lsprite *pls = &PANEL_lsprite[PANEL_LSPRITE_TEXT_BOX];

	SET_PP(0, x1,        y1, pls->u1,                   pls->v1);
	SET_PP(1, x1 + 8.0F, y1, pls->u1 + (8.0F / 256.0F), pls->v1);
	SET_PP(2, x2 - 8.0F, y1, pls->u2 - (8.0F / 256.0F), pls->v1);
	SET_PP(3, x2,        y1, pls->u2,                   pls->v1);

	SET_PP(4, x2, y1 + 8.0F, pls->u2, pls->v1 + (8.0F / 256.0F));
	SET_PP(5, x2, y2 - 8.0F, pls->u2, pls->v2 - (8.0F / 256.0F));

	SET_PP(6, x2,        y2, pls->u2,                   pls->v2);
	SET_PP(7, x2 - 8.0F, y2, pls->u2 - (8.0F / 256.0F), pls->v2);
	SET_PP(8, x1 + 8.0F, y2, pls->u1 + (8.0F / 256.0F), pls->v2);
	SET_PP(9, x1,        y2, pls->u1,                   pls->v2);

	SET_PP(10, x1, y2 - 8.0F, pls->u1, pls->v2 - (8.0F / 256.0F));
	SET_PP(11, x1, y1 + 8.0F, pls->u1, pls->v1 + (8.0F / 256.0F));

	SET_PP(12, x1 + 8.0F, y1 + 8.0F, pls->u1 + (8.0F / 256.0F), pls->v1 + (8.0F / 256.0F));
	SET_PP(13, x2 - 8.0F, y1 + 8.0F, pls->u2 - (8.0F / 256.0F), pls->v1 + (8.0F / 256.0F));
	SET_PP(14, x2 - 8.0F, y2 - 8.0F, pls->u2 - (8.0F / 256.0F), pls->v2 - (8.0F / 256.0F));
	SET_PP(15, x1 + 8.0F, y2 - 8.0F, pls->u1 + (8.0F / 256.0F), pls->v2 - (8.0F / 256.0F));

	SLONG i;

	SLONG p1;
	SLONG p2;

	POLY_Point *quad[4];

	struct
	{
		UBYTE p1;
		UBYTE p2;
		UBYTE p3;
		UBYTE p4;

	} blah[9] =
	{
		{0,1,11,12},
		{1,2,12,13},
		{2,3,13,4},
		{11,12,10,15},
		{12,13,15,14},
		{13,4,14,5},
		{10,15,9,8},
		{15,14,8,7},
		{14,5,7,6}
	};

	for (i = 0; i < 9; i++)
	{
		quad[0] = &pp[blah[i].p1];
		quad[1] = &pp[blah[i].p2];
		quad[2] = &pp[blah[i].p3];
		quad[3] = &pp[blah[i].p4];

		POLY_add_quad(quad, pls->page, FALSE, TRUE);
	}
}



SLONG PANEL_sign_which;
SLONG PANEL_sign_flip;
SLONG PANEL_sign_time;

void PANEL_flash_sign(SLONG sign, SLONG flip)
{
	PANEL_sign_time  = GetTickCount();
	PANEL_sign_flip  = flip;
	PANEL_sign_which = sign;
}



CBYTE PANEL_info_message[512];
ULONG PANEL_info_time;

void PANEL_new_info_message(CBYTE *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(PANEL_info_message, fmt, ap);
	va_end  (ap);

	PANEL_info_time = GetTickCount();
}



void PANEL_darken_screen(SLONG x)
{
	PANEL_draw_quad(640.0F - float(x), 0.0F, 640.0F, 480.0F, POLY_PAGE_ALPHA_OVERLAY, 0x88000000);
}

// this sucks!
inline SLONG BodgePageIntoAddAlpha(SLONG oldpage)
{
	if (oldpage==POLY_PAGE_LASTPANEL_ALPHA) return POLY_PAGE_LASTPANEL_ADDALPHA;
	return POLY_PAGE_LASTPANEL2_ADDALPHA;
}
// this sucks too!
inline SLONG BodgePageIntoAdd(SLONG oldpage)
{
	if (oldpage==POLY_PAGE_LASTPANEL_ALPHA) return POLY_PAGE_LASTPANEL_ADD;
	return POLY_PAGE_LASTPANEL2_ADD;
}
// this sucks as well!
inline SLONG BodgePageIntoSub(SLONG oldpage)
{
	if (oldpage==POLY_PAGE_LASTPANEL_ALPHA) return POLY_PAGE_LASTPANEL_SUB;
	return POLY_PAGE_LASTPANEL2_SUB;
}


/*************************************************************
 *
 *   Pop-up inventory panel
 *
 */

void PANEL_inv_weapon(SLONG x, SLONG y, SLONG item, UBYTE who, SLONG rgb, UBYTE sel) {
/*	SLONG which_gun, which_dx=0;

	//
	// Which gun shall we draw.
	//
	switch(item)
	{
		default:
			if (who == PANEL_WHO_DARCI)
				which_gun = PANEL_IC_DARCI_HAND;
			else
				which_gun = PANEL_IC_ROPER_HAND;
			break;

		case SPECIAL_GUN:
			which_gun  = PANEL_IC_PISTOL;
			break;

		case SPECIAL_AK47:
			which_gun  =  PANEL_IC_AK47;
			which_dx   = -45;
			break;

		case SPECIAL_BASEBALLBAT:
			which_gun  =  PANEL_IC_BASEBALLBAT;
			which_dx   = -50;
			break;

		case SPECIAL_KNIFE:
			which_gun  = PANEL_IC_KNIFE;
//			which_dy   =  15;
			break;

		case SPECIAL_SHOTGUN:
			which_gun  =  PANEL_IC_SHOTGUN;
			which_dx   = -35;
			break;

		case SPECIAL_GRENADE:
			which_gun  =  PANEL_IC_GRENADE;
			break;

		case SPECIAL_EXPLOSIVES:
			which_gun  =  PANEL_IC_EXPLOSIVES;
			break;
		case SPECIAL_WIRE_CUTTER:
			which_gun  =  PANEL_IC_EXPLOSIVES;
			break;
	}

	PANEL_funky_quad(
		which_gun,
		x + which_dx,
		y,
		PANEL_PAGE_ALPHA_END,
		0x00ffffff|alpha);
	*/

	SLONG sprite = -1, faded;

	switch(item)
	{
		case SPECIAL_GUN:
			sprite = PANEL_LSPRITE_PISTOL;
		break;
		case SPECIAL_SHOTGUN:
			sprite = PANEL_LSPRITE_SHOTGUN;
			break;
		case SPECIAL_AK47:
			sprite = PANEL_LSPRITE_AK47;
			break;
		case SPECIAL_EXPLOSIVES:
			sprite = PANEL_LSPRITE_EXPLOSIVES;
			break;
		case SPECIAL_GRENADE:
			sprite = PANEL_LSPRITE_GRENADE;
			break;
		case SPECIAL_KNIFE:
			sprite = PANEL_LSPRITE_KNIFE;
			break;
		case SPECIAL_BASEBALLBAT:
			sprite = PANEL_LSPRITE_BBB;
			break;
		default:
			sprite = PANEL_LSPRITE_FIST;
			break;
	}

	//
	// Centre the sprite.
	//

	ASSERT(WITHIN(sprite, 0, PANEL_LSPRITE_NUMBER - 1));

	PANEL_Lsprite *pls = &PANEL_lsprite[sprite];

	float uwidth = (pls->u2 - pls->u1) * 256.0F;
	float vwidth = (pls->v2 - pls->v1) * 256.0F;

#ifdef TARGET_DC
	// Got to make it much more obvious which weapon is selected.
	if (sel)
	{
		// Enlarge it.
		uwidth *= 2.0f;
		vwidth *= 2.0f;
	}
	else
	{
		// Dark red
		faded=(rgb&0xff)>>1;
		faded|=(faded<<8)|(faded<<16)|(faded<<24);
		rgb = faded;
	}
#else
	faded=(rgb&0xff)>>1;
	faded|=(faded<<8)|(faded<<16)|(faded<<24);
	if (!sel) rgb=faded;
#endif



/*	PANEL_draw_quad(
		(float)x - uwidth * 0.35F,
		(float)y - vwidth * 0.35F,
		(float)x + uwidth * 0.35F,
		(float)y + vwidth * 0.35F,
		POLY_PAGE_LASTPANEL_SUB,
//		0x7f007fff,
	    faded,
		pls->u1,
		pls->v1,
		pls->u2,
		pls->v2);*/
/*	PANEL_draw_quad(
		(float)x - 30,
		(float)y - 30,
		(float)x + 30,
		(float)y + 30,
		POLY_PAGE_SHADOW_OVAL,
		faded,
		0.0, 0.0, 1.0, 1.0
	);*/
/*	PANEL_draw_quad(
		(float)x + 2 - uwidth * 0.25F,
		(float)y + 2 - vwidth * 0.25F,
		(float)x + 2 + uwidth * 0.25F,
		(float)y + 2 + vwidth * 0.25F,
		BodgePageIntoSub(pls->page),
//		0x7f007fff,
	    faded,
		pls->u1,
		pls->v1,
		pls->u2,
		pls->v2);*/

	PANEL_draw_quad(
		(float)x - uwidth * 0.25F,
		(float)y - vwidth * 0.25F,
		(float)x + uwidth * 0.25F,
		(float)y + vwidth * 0.25F,
		BodgePageIntoAdd/*Alpha*/(pls->page),
		rgb,
		pls->u1,
		pls->v1,
		pls->u2,
		pls->v2);
}


#define PANEL_ADDWEAPON(item) { draw_list[draw_count]=item; draw_count++; }
#define ITEM_SEPERATION (150)

void PANEL_inventory(Thing *darci, Thing *player) {
	SLONG rgb,rgb2;
	CBYTE draw_list[10];
	UBYTE draw_count=0;
	Thing *p_special = NULL;
	SLONG x,c0;
	UBYTE current_item = 0;
	SLONG sel;

	UWORD CONTROLS_inv_fade = player->Genus.Player->PopupFade;

	if (!CONTROLS_inv_fade) return;
	if (darci->Genus.Person->Flags & FLAG_PERSON_DRIVING) return;



// Bad Dog! Obsolete code.
//extern SLONG EWAY_cam_active;
	//if (EWAY_cam_active) return;

	if ( EWAY_stop_player_moving() )
	{
		return;
	}


/*	rgb=((SLONG)(CONTROLS_inv_fade-1))<<24;
	rgb2=((SLONG)(CONTROLS_inv_fade-1)>>1)<<24;
	DRAW2D_Box_Page(0, 20, 640, 80, rgb2|0xAFA583, POLY_PAGE_ALPHA_OVERLAY, 128);
	DRAW2D_Box_Page(0,20,CONTROLS_inv_fade*3,21,rgb|0xFFFFFF,POLY_PAGE_ALPHA_OVERLAY,128);
	DRAW2D_Box_Page(640-(CONTROLS_inv_fade*3),79,640,80,rgb|0xFFFFFF,POLY_PAGE_ALPHA_OVERLAY,128);

	CBYTE stat_up[10];
	sprintf(stat_up,"%s: %d",XLAT_str(X_STR),player->Genus.Player->Strength);
	FONT2D_DrawString(stat_up,0,1,0xffffff,256,POLY_PAGE_FONT2D,rgb);
	sprintf(stat_up,"%s: %d",XLAT_str(X_CON),player->Genus.Player->Constitution);
	FONT2D_DrawString(stat_up,80,1,0xffffff,256,POLY_PAGE_FONT2D,rgb);
	sprintf(stat_up,"%s: %d",XLAT_str(X_STA),player->Genus.Player->Stamina);
	FONT2D_DrawString(stat_up,160,1,0xffffff,256,POLY_PAGE_FONT2D,rgb);
	sprintf(stat_up,"%s: %d",XLAT_str(X_DEX),player->Genus.Player->Skill);
	FONT2D_DrawString(stat_up,240,1,0xffffff,256,POLY_PAGE_FONT2D,rgb);
*/
	PANEL_ADDWEAPON(0);

	sel=darci->Genus.Person->SpecialUse;

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special)
		{
			ASSERT(p_special->Class == CLASS_SPECIAL);
			if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
				SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_WIRE_CUTTER)
			{
				if (THING_NUMBER(p_special)==darci->Genus.Person->SpecialUse)
				{
					current_item=draw_count;
					sel=p_special->Genus.Special->SpecialType;
				}
			  PANEL_ADDWEAPON(p_special->Genus.Special->SpecialType);
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN)
	{
		if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
		{
			current_item=draw_count;
			sel=SPECIAL_GUN;
		}
		PANEL_ADDWEAPON(SPECIAL_GUN);
	}

	current_item = player->Genus.Player->ItemFocus;

	if (current_item==-1) return;

//	x = 320 - (50*draw_count) - (current_item*50);
/*	x = 320 - (current_item*ITEM_SEPERATION);
	for (c0=0;c0<draw_count;c0++) {
		PANEL_inv_weapon(x,20,draw_list[c0],0,rgb);
		x+=ITEM_SEPERATION;
	}
	*/

	if (draw_list[current_item]&&!sel) sel=draw_list[current_item];


	int iYPos, iYInc;
	if ( m_iPanelYPos > 300 )
	{
		// Inventory starts above and goes upwards.
		iYPos = m_iPanelYPos - 150;		//330
		iYInc = -35;
	}
	else
	{
		// Inventory starts below and goes down.
		iYPos = m_iPanelYPos + 20;
		iYInc = 35;
	}


	rgb=CONTROLS_inv_fade-1;
	rgb2=rgb|(rgb<<8)|(rgb<<24);
	rgb|=(rgb<<8)|(rgb<<16)|(rgb<<24);
	for (c0=0;c0<draw_count;c0++) {
//		if ((c0!=current_item)&&(draw_list[c0]!=darci->Genus.Person->SpecialUse))
		if (draw_list[c0]!=sel)
		{
			PANEL_inv_weapon(m_iPanelXPos+170,iYPos,draw_list[c0],0,rgb, 0);
		}
		else
		{
			PANEL_inv_weapon(m_iPanelXPos+170,iYPos,draw_list[c0],0,rgb ,1);
		}
		iYPos += iYInc;
	}
}


#define PANEL_LAST_HEIGHT	165
#define SCANNER_LAST_CTR_X	74
#define SCANNER_LAST_CTR_Y	(480 - PANEL_LAST_HEIGHT + 74)
#define SCANNER_LAST_CTR_U	74
#define SCANNER_LAST_CTR_V	74





void PANEL_last()
{


	//TRACE ( "PDi" );

	bool bPanelIsAtBottomOfScreen = ( m_iPanelYPos > 300 );


#ifdef TARGET_DC
	PANEL_draw_VMU_ammo_counts();
#endif


	if (EWAY_tutorial_string)
	{
		//
		// Darken the whole screen...
		//

		PANEL_darken_screen(640);

#ifndef TARGET_DC
		POLY_frame_draw(FALSE, FALSE);
		POLY_frame_init(FALSE, FALSE);
#endif

		//
		// Draw a speech bubble in the top left of the screen.
		//

#ifdef TARGET_DC
		// We have to keep stuff away from the edges by 32 pixels. Waaa.
		SLONG height = FONT2D_DrawStringWrapTo(
						EWAY_tutorial_string,
						32,
						32,
						0xffffff,
						256,
						POLY_PAGE_FONT2D,
						0,
						640-32*2);

		PANEL_last_bubble(
			32 - 6,
			32 - 4,
			float(FONT2D_rightmost_x) + 6.0F,
			height + 18);

		// Do it again so it's brighter.
		FONT2D_DrawStringWrapTo(
						EWAY_tutorial_string,
						32,
						32,
						0xffffff,
						256,
						POLY_PAGE_FONT2D,
						0,
						640-32*2);


#else
		#define PANEL_TUT_X 16
		#define PANEL_TUT_Y 16

		SLONG height = FONT2D_DrawStringWrap(
							EWAY_tutorial_string,
							PANEL_TUT_X,
							PANEL_TUT_Y,
							0xffffff);

		PANEL_last_bubble(
			PANEL_TUT_X - 6,
			PANEL_TUT_Y - 4,
			float(FONT2D_rightmost_x) + 6.0F,
			PANEL_TUT_Y + height + 4);

#endif

		return;
	}


	Thing *darci = NET_PERSON(0);

	if (darci == NULL)
	{
		return;
	}

	if (EWAY_stop_player_moving())
	{
		//
		// There is a widescreen cutscene playing.
		//

		PANEL_new_widescreen();

		return;
	}
	else
	{
		PANEL_wide_top_person     = NULL;
		PANEL_wide_bot_person     = NULL;
		PANEL_wide_top_is_talking = FALSE;
		PANEL_wide_text[0]        = '\000';
	}

	//
	// Draw the background first.
	//

	/*
	PANEL_draw_quad(
		0.0F,
		480.0F - 165.0F,
		212.0F,
		480.0F,
		POLY_PAGE_LASTPANEL_ALPHA,
		0xffffffff,
		0.0F,
		90.0F / 256.0F,
		212.0F / 256.0F,
		1.0F);*/

	PANEL_draw_quad(
		(float)( m_iPanelXPos + 0 ),
		(float)( m_iPanelYPos - 165 ),
		(float)( m_iPanelXPos + 212 ),
		(float)( m_iPanelYPos - 0 ),
		POLY_PAGE_LASTPANEL_ALPHA,
		0xffffffff,
		0.0F,
		90.0F / 256.0F,
		212.0F / 256.0F,
		1.0F);


/*	PANEL_draw_quad(
		0.0F,
		480.0F - 142.0F,
		126.0F,
		480.0F,
		POLY_PAGE_LASTPANEL_ALPHA,
		0xffffffff,
		0.0F,
		114.0F / 256.0F,
		126.0F / 256.0F,
		1.0F);

	PANEL_draw_quad(
		126.0F,
		480.0F - 142.0F,
		186.0F,
		480.0F,
		POLY_PAGE_LASTPANEL_ALPHA,
		0xffffffff,
		126.0F / 256.0F,
		114.0F / 256.0F,
		127.0F / 256.0F,
		1.0F);

	PANEL_draw_quad(
		186.0F,
		480.0F - 142.0F,
		186.0F + 142.0F - 127.0F,
		480.0F,
		POLY_PAGE_LASTPANEL_ALPHA,
		0xffffffff,
		127.0F / 256.0F,
		114.0F / 256.0F,
		142.0F / 256.0F,
		1.0F);
*/
	//
	// The weapon/ammo box.
	//

	SLONG sprite = -1;
	CBYTE text[64];

	text[0] = '\000';

	int iGrenadeCountdown = -1;

	if (darci->Genus.Person->Flags & FLAG_PERSON_DRIVING)
	{
		Thing *p_vehicle = TO_THING(darci->Genus.Person->InCar);

		sprite = PANEL_LSPRITE_LOW_GEAR;

/*		if (p_vehicle->Genus.Vehicle->Siren)
		{
			sprite = PANEL_LSPRITE_HIGH_GEAR;

			sprintf(text, "High");
		}
		else
		{
			sprite = PANEL_LSPRITE_LOW_GEAR;

			sprintf(text, "Low");
		}*/
	}
	else
	{
		if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
		{
			sprite = PANEL_LSPRITE_PISTOL;

			if (darci->Genus.Person->ammo_packs_pistol)
			{
				sprintf(text, "%d\\%d", darci->Genus.Person->Ammo, darci->Genus.Person->ammo_packs_pistol/15);
			}
			else
			{
				sprintf(text, "%d", darci->Genus.Person->Ammo);
			}
		}
		else
		if (darci->Genus.Person->SpecialUse)
		{
			Thing *p_special = TO_THING(darci->Genus.Person->SpecialUse);

			switch(p_special->Genus.Special->SpecialType)
			{
				case SPECIAL_SHOTGUN:

					sprite = PANEL_LSPRITE_SHOTGUN;

					if (darci->Genus.Person->ammo_packs_shotgun)
					{
						sprintf(text, "%d\\%d", p_special->Genus.Special->ammo, darci->Genus.Person->ammo_packs_shotgun/SPECIAL_AMMO_IN_A_SHOTGUN);
					}
					else
					{
						sprintf(text, "%d", p_special->Genus.Special->ammo);
					}

					break;

				case SPECIAL_AK47:

					sprite = PANEL_LSPRITE_AK47;

					if (darci->Genus.Person->ammo_packs_ak47)
					{
						sprintf(text, "%d\\%d", p_special->Genus.Special->ammo, darci->Genus.Person->ammo_packs_ak47/SPECIAL_AMMO_IN_A_AK47);
					}
					else
					{
						sprintf(text, "%d", p_special->Genus.Special->ammo);
					}
					break;

				case SPECIAL_EXPLOSIVES:
					sprite = PANEL_LSPRITE_EXPLOSIVES;
					sprintf(text, "%d", p_special->Genus.Special->ammo);
					break;

				case SPECIAL_GRENADE:
					sprite = PANEL_LSPRITE_GRENADE;
					//
					// If the player is holding a grenade that has had its pin pulled then
					// show how long the grenade has before it goes off.
					//
					if (p_special->SubState == SPECIAL_SUBSTATE_ACTIVATED)
					{
						SLONG secsleft;


						secsleft = p_special->Genus.Special->timer / (16 * 20) + 1;
						SATURATE(secsleft,0,6);

#ifdef TARGET_DC
						// This gets done later, over the grenade icon.
						iGrenadeCountdown = secsleft;
#else
						ULONG colour, colours[]={0xff3300,0xff8800,0x88ff00,0x888888};
						if (secsleft<1)
							colour=*colours;
						else
							if (secsleft>3)
								colour=colours[3];
							else
								colour=colours[secsleft];

						itoa(secsleft,text,10);

						FONT2D_DrawString(
							text,
							m_iPanelXPos + 141,
							m_iPanelYPos - 53,
							colour,
							256);
#endif

					}
#ifdef TARGET_DC
					else
					{
						iGrenadeCountdown = -1;
					}
#endif

					itoa(p_special->Genus.Special->ammo,text,10);
					break;

				case SPECIAL_KNIFE:
					sprite = PANEL_LSPRITE_KNIFE;
					break;

				case SPECIAL_BASEBALLBAT:
					sprite = PANEL_LSPRITE_BBB;
					break;

				default:
					sprite = PANEL_LSPRITE_QMARK;
					break;
			}
		}
		else
		{
			sprite = PANEL_LSPRITE_FIST;
		}
	}

	//
	// Centre the sprite.
	//

	{
		ASSERT(WITHIN(sprite, 0, PANEL_LSPRITE_NUMBER - 1));

		PANEL_Lsprite *pls = &PANEL_lsprite[sprite];

		float uwidth = (pls->u2 - pls->u1) * 256.0F;
		float vwidth = (pls->v2 - pls->v1) * 256.0F;

		/*
		float x1 = 170.0F - uwidth * 0.5F;
		float y1 = 417.0F - vwidth * 0.5F;
		float x2 = 170.0F + uwidth * 0.5F;
		float y2 = 417.0F + vwidth * 0.5F;
		*/
		float x1 = (float)( m_iPanelXPos + 170 ) - uwidth * 0.5F;
		float y1 = (float)( m_iPanelYPos -  63 ) - vwidth * 0.5F;
		float x2 = (float)( m_iPanelXPos + 170 ) + uwidth * 0.5F;
		float y2 = (float)( m_iPanelYPos -  63 ) + vwidth * 0.5F;

		if (ftol(uwidth) & 0x1)
		{
			x1 -= 0.5F;
			x2 -= 0.5F;
		}

		if (ftol(vwidth) & 0x1)
		{
			y1 -= 0.5F;
			y2 -= 0.5F;
		}

		// And move the M16 up a bit, so it doesn't cover the ammo count.
		if ( sprite == PANEL_LSPRITE_AK47 )
		{
			y1 -= 8.0F;
			y2 -= 8.0F;
		}

		PANEL_draw_quad(
			x1, y1,
			x2, y2,
			BodgePageIntoAdd/*Alpha*/(pls->page),
			0xFFffffff,
			pls->u1,
			pls->v1,
			pls->u2,
			pls->v2);
	}

	//
	// Draw the ammo.
	//

	if (text)
	{
		FONT2D_DrawStringRightJustify(
			text,
			m_iPanelXPos + 215, // 214,
			m_iPanelYPos - 50,	// 427,
			0xffffff,
			256 + 64);

		/*

		FONT2D_DrawStringRightJustify(
			text,
			m_iPanelXPos + 217, // 214,
			m_iPanelYPos - 51,	// 427,
			0x000000,
			384);

		*/

		/*

		FONT2D_DrawStringRightJustify(
			text,
			m_iPanelXPos + 214, // 214,
			m_iPanelYPos - 53,	// 427,
			0xffffff,
			256);

		*/
	}


#ifdef TARGET_DC
	if ( iGrenadeCountdown >= 0 )
	{
		// Draw the countdown over the grenade.
		ULONG colour = 0xff3300;
		if ( iGrenadeCountdown <= 4 )
		{
			static int iFlash = 0;
			iFlash += ( 5 - iGrenadeCountdown );
			if ( ( iFlash & 0x4 ) == 0 )
			{
				// Flash it.
				colour = 0x000000;
			}
		}
		itoa(iGrenadeCountdown,text,10);

		// BRIGHTER! BIGGER! MORE! You can barely see the damn countdown on the PC.
		FONT2D_DrawString(
			text,
			m_iPanelXPos + 160 + 2,
			m_iPanelYPos - 73 + 2,
			0x00000000,
			512);

		FONT2D_DrawString(
			text,
			m_iPanelXPos + 160 - 2,
			m_iPanelYPos - 73 - 2,
			0x00000000,
			512);

		FONT2D_DrawString(
			text,
			m_iPanelXPos + 160 - 2,
			m_iPanelYPos - 73 + 2,
			0x00000000,
			512);

		FONT2D_DrawString(
			text,
			m_iPanelXPos + 160 + 2,
			m_iPanelYPos - 73 - 2,
			0x00000000,
			512);

		FONT2D_DrawString(
			text,
			m_iPanelXPos + 160,
			m_iPanelYPos - 73,
			colour,
			512);
	}
#endif


	//
	// And the crime rate, if applicable
	//
	if (GAME_FLAGS & GF_SHOW_CRIMERATE)
	{
		CBYTE crimerate[64];
		sprintf(crimerate, "%d%%", CRIME_RATE);

		if (CRIME_RATE>=95)
		{
			static UWORD pulse=0;
			SLONG colour;
			pulse+=(TICK_RATIO*80)>>TICK_SHIFT;
			colour=(SIN(pulse&2047)>>9)+128;
			colour=colour|(colour<<8);
			FONT2D_DrawStringCentred(
				crimerate,
				m_iPanelXPos + 170, //170,
				m_iPanelYPos - 116, //364,
				0xff0000|colour);
		} else
			FONT2D_DrawStringCentred(
				crimerate,
				m_iPanelXPos + 170, //170,
				m_iPanelYPos - 116, //364,
				0xffffff);
	}

	#ifndef TARGET_DC
	{
		CBYTE timing[256];
		float	strip;
		SLONG	c;

		extern ULONG AENG_draw_time;
		extern ULONG AENG_poly_add_quad_time;

#ifdef	STRIP_STATS
extern	ULONG	strip_stats[];
		if(strip_stats[0])
			strip=(float)strip_stats[1]/(float)strip_stats[0];



		sprintf(
			timing,
			"Frame time %d strip len %f",
			AENG_draw_time >> 12,strip);

		FONT2D_DrawString(
			timing,
			50,
			65,
			0xffffff);

		for(c=2;c<34;c++)
		{
			sprintf(
				timing,
				"(%d)%d",c-1,
				strip_stats[c]);

			FONT2D_DrawString(
				timing,
				10+(((c-2)&7))*75,
				120+(((c-2)&0xf8)>>3)*20,
				0xffffff);

		}
#endif

		sprintf(
			timing,
			"Poly add quad %d",
			AENG_poly_add_quad_time >> 12);

		FONT2D_DrawString(
			timing,
			50,
			80,
			0xffffff);
	}
	#endif

	//
	// Draw the health bar.
	//

	{
		SLONG i;

		float angle;

		float du;
		float dv;

		float u1;
		float v1;
		float u2;
		float v2;

		float last_u1;
		float last_v1;
		float last_u2;
		float last_v2;

		POLY_Point  pp  [4];
		POLY_Point *quad[4];
		POLY_Point *tri [3];

		quad[0] = &pp[0];
		quad[1] = &pp[1];
		quad[2] = &pp[2];
		quad[3] = &pp[3];

		tri[0] = &pp[0];
		tri[1] = &pp[1];
		tri[2] = &pp[2];

		static float blah1 = ( -43 * 2.0F * PI / 360.0F);
		static float blah2 = (-227 * 2.0F * PI / 360.0F);
		UBYTE is_in_car = darci->Genus.Person->InCar ? 1 : 0;
		float car_offset = is_in_car ? 130.0F : 0.0F;

		Thing *the_car=is_in_car? TO_THING(darci->Genus.Person->InCar) : 0;

		#define PLH_MID_U  (71.0F + car_offset)
		#define PLH_MID_V  (71.0F)
		#define PLH_RADIUS (66.0F)

		#define PLH_WIDTH  (10.0F)
		#define PLH_ANGLE1 blah1
		#define PLH_ANGLE2 blah2
		#define PLH_SEGS   (8)

		//#define PLH_MID_X  (74.0F)
		//#define PLH_MID_Y  (480.0F - (256.0F - 166.0F))

		#define PLH_MID_X  ((float)(m_iPanelXPos + 74))
		#define PLH_MID_Y  ((float)(m_iPanelYPos - (256 - 166)))

		angle = PLH_ANGLE1;

		float dangle;
		float fraction;

		dangle = (PLH_ANGLE2 - PLH_ANGLE1) / PLH_SEGS;

		//
		// What fraction of health does Darci have?
		//

		fraction = is_in_car ? float(the_car->Genus.Vehicle->Health) * (1.0F / 300.0F) : float(darci->Genus.Person->Health) * ((darci->Genus.Person->PersonType == PERSON_ROPER) ? (1.0F / 400.0F) : (1.0F / 200.0F));

		SATURATE(fraction, 0.0F, 1.0F);

		dangle *= fraction;

		for (i = 0; i <= PLH_SEGS; i++)
		{
			du = sin(angle);
			dv = cos(angle);

			u1 = (PLH_MID_U + du * (PLH_RADIUS + PLH_WIDTH));
			v1 = (PLH_MID_V + dv * (PLH_RADIUS + PLH_WIDTH));

			u2 = (PLH_MID_U + du * (PLH_RADIUS - PLH_WIDTH));
			v2 = (PLH_MID_V + dv * (PLH_RADIUS - PLH_WIDTH));

			if (i > 0)
			{

				float fWDepthBodge = PANEL_GetNextDepthBodge();
				float fZDepthBodge = 1.0f - fWDepthBodge;

				pp[0].X        = PLH_MID_X + (u1 - PLH_MID_U);
				pp[0].Y        = PLH_MID_Y + (v1 - PLH_MID_V);
				pp[0].z        = fZDepthBodge;
				pp[0].Z        = fWDepthBodge;
				pp[0].x        = 0.0F;
				pp[0].y        = 0.0F;
				pp[0].u        = u1 * (1.0F / 256.0F);
				pp[0].v        = v1 * (1.0F / 256.0F);
				pp[0].colour   = 0xffffffff;
				pp[0].specular = 0xff000000;

				pp[1].X        = PLH_MID_X + (u2 - PLH_MID_U);
				pp[1].Y        = PLH_MID_Y + (v2 - PLH_MID_V);
				pp[1].z        = fZDepthBodge;
				pp[1].Z        = fWDepthBodge;
				pp[1].x        = 0.0F;
				pp[1].y        = 0.0F;
				pp[1].u        = u2 * (1.0F / 256.0F);
				pp[1].v        = v2 * (1.0F / 256.0F);
				pp[1].colour   = 0xffffffff;
				pp[1].specular = 0xff000000;

				pp[2].X        = PLH_MID_X + (last_u1 - PLH_MID_U);
				pp[2].Y        = PLH_MID_Y + (last_v1 - PLH_MID_V);
				pp[2].z        = fZDepthBodge;
				pp[2].Z        = fWDepthBodge;
				pp[2].x        = 0.0F;
				pp[2].y        = 0.0F;
				pp[2].u        = last_u1 * (1.0F / 256.0F);
				pp[2].v        = last_v1 * (1.0F / 256.0F);
				pp[2].colour   = 0xffffffff;
				pp[2].specular = 0xff000000;

				pp[3].X        = PLH_MID_X + (last_u2 - PLH_MID_U);
				pp[3].Y        = PLH_MID_Y + (last_v2 - PLH_MID_V);
				pp[3].z        = fZDepthBodge;
				pp[3].Z        = fWDepthBodge;
				pp[3].x        = 0.0F;
				pp[3].y        = 0.0F;
				pp[3].u        = last_u2 * (1.0F / 256.0F);
				pp[3].v        = last_v2 * (1.0F / 256.0F);
				pp[3].colour   = 0xffffffff;
				pp[3].specular = 0xff000000;

				POLY_add_quad(quad, POLY_PAGE_LASTPANEL2_ALPHA, FALSE, TRUE);
			}

			last_u1 = u1;
			last_v1 = v1;
			last_u2 = u2;
			last_v2 = v2;

			angle += dangle;
		}
	}

	//
	// Draw the stamina marks
	//

	{
		UBYTE i, stamina = darci->Genus.Person->Stamina / 25;
		//SLONG x = 107;
		//SLONG y = 480 - 36;
		SLONG x = m_iPanelXPos + 107;
		SLONG y = m_iPanelYPos - 36;
		SLONG rgb[] = { 0x00FF0000, 0x00C04000, 0x00808000, 0x0040C000, 0x0000ff00 };

		SATURATE(stamina,0,5);

		for (i=0;i<stamina;i++)
		{
			PANEL_draw_quad(
				x, y-10,
				x+10, y,
				POLY_PAGE_LASTPANEL_ADD,
				rgb[i],
				(243.0/256.0),
				(198.0/256.0),
				(253.0/256.0),
				(208.0/256.0));
			x+=3; y-=3;
		}

	}

	//
	// Draw the navigation beacons.
	//

	{
		SLONG i;

		float dx;
		float dz;
		float dist;
		float dangle;
		float score;

		float x;
		float y;

		BOOL thugly;

		POLY_Point  pp [3];
		POLY_Point *tri[3];

		MAP_Beacon *mb;

		ULONG colour;

		SLONG best_beacon = NULL;
		float best_score  = float(INFINITY);

		for (i = 1; i < MAP_MAX_BEACONS; i++)
		{
			mb = &MAP_beacon[i];

			if (!mb->used)
			{
				continue;
			}

			thugly=FALSE;

			if (mb->track_thing)
			{
				Thing *p_track = TO_THING(mb->track_thing);

				mb->wx = p_track->WorldPos.X >> 8;
				mb->wz = p_track->WorldPos.Z >> 8;

				extern SLONG is_person_dead(Thing *p_person);

				if (p_track->Class == CLASS_PERSON)
				{
					switch(p_track->Genus.Person->PersonType)
					{
						case PERSON_THUG_RASTA:
						case PERSON_THUG_GREY:
						case PERSON_THUG_RED:
						case PERSON_MIB1:
						case PERSON_MIB2:
						case PERSON_MIB3:
							thugly=TRUE;
					}
					if (p_track->State == STATE_DEAD)
					{

						//
						// Don't draw beacons to dead people.
						//

						if (p_track->SubState == SUB_STATE_DEAD_INJURED)
						{
							//
							// Except if they're only injured...
							//

							if (p_track->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
							{
								//
								// Except if they're fight test dummies!
								//

								continue;
							}
						}
						else
						{
							continue;
						}
					}
				}
			}

			colour = PANEL_beacon_colour[i % PANEL_MAX_BEACON_COLOURS] | (0xff000000);

			//
			// Work out the distance and relative angle of this beacon from darci.
			//

			dx = float(mb->wx - (darci->WorldPos.X >> 8));
			dz = float(mb->wz - (darci->WorldPos.Z >> 8));

			dist = sqrt(dx*dx + dz*dz);

			/*

			if (dist < 256.0F * 4.0F)
			{
				score  = dist * (1.0F / (256.0F * 4.0F));
				score *= 2.0F * PI * 22.0F / 360.0F;

				PANEL_funky_quad(
					PANEL_IC_DOT,
					PANEL_IC_SCANX - 7.0F,
					PANEL_IC_SCANY - 7.0F,
					PANEL_PAGE_ALPHA_END,
					colour);
			}
			else
			*/
			{
				UBYTE is_dot=0;

				if (PANEL_scanner_poo)
					dangle = atan2(dx,dz) - float(darci->Draw.Tweened->Angle) * (2.0F * PI / 2048.0F);
				else
					dangle = atan2(dx,dz) - float(FC_cam[0].yaw>>8) * (2.0F * PI / 2048.0F);
				score  = (float)fmod(dangle, 2.0F * PI) - PI;

				if (score > +PI) {score -= 2.0F * PI;}
				if (score < -PI) {score += 2.0F * PI;}

				//dist -= 3.0F * 256.0F;
				dist /= 16.0F * 256.0F;

				if (dist < 1.0F) is_dot=1;

				SATURATE(dist, 0.0F, 1.0F);

				#define PLS_MID_X  PLH_MID_X
				#define PLS_MID_Y  PLH_MID_Y
				#define PLS_RADIUS 50.0F

				//if (!(is_dot&&thugly)) // it'll get drawn by the thug scanner below
				{
					x = PLS_MID_X + (float)sin(dangle) * PLS_RADIUS * dist;
					y = PLS_MID_Y + (float)cos(dangle) * PLS_RADIUS * dist;

					float size = (mb->pad&&!is_dot) ? 9.0F : 6.0F;


					SLONG alive = GetTickCount() - mb->ticks;

					if (alive < 4096)
					{
						alive &= 0x100;

						SATURATE(alive, 0, 255);

						colour &= 0x00ffffff;
						colour |= alive << 24;


						/*

						SLONG r = (colour >> 16) & 0xff;
						SLONG g = (colour >>  8) & 0xff;
						SLONG b = (colour >>  0) & 0xff;

						r = r * alive >> 8;
						g = g * alive >> 8;
						b = b * alive >> 8;

						colour = 0xff000000 | (r << 16) | (g << 8) | b;

						*/
					}

					PANEL_last_arrow(x,y, dangle, size, colour, is_dot);
				}
			}

			if (fabs(score) < best_score)
			{
				best_score  = fabs(score);
				best_beacon = i;
			}

			mb->pad = FALSE;
		}

		if (PANEL_info_time > GetTickCount() - 2000)
		{
			SLONG x_right;

			SLONG colour_main;
			SLONG colour_shad;

//			if (!WITHIN(PANEL_info_time, GetTickCount() - 1100, GetTickCount() - 900))


			SLONG now   = GetTickCount();
			SLONG onfor = now - PANEL_info_time;

			if (onfor < 255)
			{
				colour_main = onfor;
				colour_shad = onfor;
			}
			else
			if (onfor < 768)
			{
				colour_main = 0xff;
				colour_shad = 255 - (onfor - 256 >> 1);
			}
			else
			{
				colour_main = 0xff;
				colour_shad = 0x00;
			}

			x_right = onfor >> 1;

			SATURATE(colour_main, 0, 255);
			SATURATE(colour_shad, 0, 255);
			//SATURATE(x_right,    16, 205);
			SATURATE(x_right,    16, 205);

			x_right += m_iPanelXPos;

			colour_main = colour_main | (colour_main << 8) | (colour_main << 16);
			colour_shad = colour_shad | (colour_shad << 8) | (colour_shad << 16);

			{
				FONT2D_DrawStringRightJustifyNoWrap(
					PANEL_info_message,
					x_right + 2,
					m_iPanelYPos - 23 + 2,		//457 + 2,
					colour_shad,
					256);

				FONT2D_DrawStringRightJustifyNoWrap(
					PANEL_info_message,
					x_right,
					m_iPanelYPos - 23,			//457,
					colour_main,
					256);
			}
		}
		else
		{
			if (best_beacon)
			{
				ASSERT(WITHIN(best_beacon, 1, MAP_MAX_BEACONS - 1));

				mb = &MAP_beacon[best_beacon];

				extern CBYTE *EWAY_get_mess(SLONG index);
#ifndef TARGET_DC
				extern UBYTE  sw_hack;

				if (!sw_hack)
#endif
				{
					FONT2D_DrawString(
						EWAY_get_mess(mb->index),
						m_iPanelXPos + 12  + 2,		//12  + 2,
						m_iPanelYPos - 23 + 2,		//457 + 2,
						0x00000000,
						256);
				}

				FONT2D_DrawString(
					EWAY_get_mess(mb->index),
					m_iPanelXPos + 12,		//12
					m_iPanelYPos - 23,		//457,
					PANEL_beacon_colour[best_beacon % PANEL_MAX_BEACON_COLOURS],
					256);

				mb->pad = TRUE;
			}
		}
	}

	//
	// Draw the thugs on the scanner.
	//

	{
		SLONG i;

		float x;
		float y;
		float dx;
		float dz;
		float dist;
		float dangle;
		float size;
		float flash = fabs(sin(float(GAME_TURN) * 0.2F));
		SLONG display;
		ULONG colour;

		PANEL_Lsprite *pls = &PANEL_lsprite[PANEL_LSPRITE_DOT];

		SLONG num_found = THING_find_sphere(
							darci->WorldPos.X >> 8,
							darci->WorldPos.Y >> 8,
							darci->WorldPos.Z >> 8,
							0x1000,
							THING_array,
							THING_ARRAY_SIZE,
							1 << CLASS_PERSON);

		Thing *p_found;

		for (i = 0; i < num_found; i++)
		{
			p_found = TO_THING(THING_array[i]);

			if (p_found == darci)
			{
				continue;
			}

			if (p_found->State == STATE_DEAD)
			{
				continue;
			}

			display = FALSE;

			switch(p_found->Genus.Person->PersonType)
			{
				case PERSON_THUG_RASTA:
				case PERSON_THUG_GREY:
				case PERSON_THUG_RED:
					display = TRUE;
					size    = 0.5F;
					colour  = 0xdd2222;
					break;

				case PERSON_MIB1:
				case PERSON_MIB2:
				case PERSON_MIB3:
					display = TRUE;
					size    = 0.5F;
					colour  = 0xdddddd;
					break;

				default:
					break;
			}

			if (PCOM_person_wants_to_kill(p_found) == THING_NUMBER(darci))
			{
				display = TRUE;
				size    = flash;
			}

			if (display)
			{
				dx = float(p_found->WorldPos.X - darci->WorldPos.X >> 8);
				dz = float(p_found->WorldPos.Z - darci->WorldPos.Z >> 8);

				dist  = sqrt(dx*dx + dz*dz);
				dist *= 1.0F / (16.0F * 256.0F);

				if (dist < 1.0F)
				{
					if (PANEL_scanner_poo)
					{
						dangle = atan2(dx,dz) - float(darci->Draw.Tweened->Angle) * (2.0F * PI / 2048.0F);
					}
					else
					{
						dangle = atan2(dx,dz) - float(FC_cam[0].yaw>>8) * (2.0F * PI / 2048.0F);
					}

					x = PLS_MID_X + (float)sin(dangle) * PLS_RADIUS * dist;
					y = PLS_MID_Y + (float)cos(dangle) * PLS_RADIUS * dist;

					size += 0.5F;
					size *= 2.0F;

					PANEL_draw_quad(
						x - size, y - size,
						x + size, y + size,
						POLY_PAGE_LASTPANEL_ADD,
						colour,
						pls->u1,
						pls->v1,
						pls->u2,
						pls->v2);
				}
			}
		}
	}

	//
	// Draw the text messages.
	//

	PANEL_new_text_process();

	{
		#define PLT_X 214
		#define PLT_Y 360

		SLONG i;
		SLONG ybase;
		SLONG y = m_iPanelYPos - ( 480 - PLT_Y );
		// Left edge.
		SLONG x1 = ( m_iPanelXPos < 260 ) ? ( m_iPanelXPos + PLT_X ) : ( 32 );
		// Right edge.
		SLONG x2 = ( m_iPanelXPos < 260 ) ? ( 640 - 32 ) : ( m_iPanelXPos - 16 );
		SLONG height;

		PANEL_Text *pt;



		// This is a very clunky bodge, in coversations multiple lines
		// of text are added but none are removed from the tail, so when
		// we come back the difference between head and tail can be over
		// the maximum buffer length, which will then wrap around and display
		// the same message multiple times, therefore if we have more than
		// PANEL_MAX_TEXTS messages we need to shift our head forwards.to
		// remove those bogus messages.

		// Nicked from the PSX. Oh, and "klunky" is spelled thusly - TomF.

		if ((PANEL_text_tail-PANEL_text_head)>PANEL_MAX_TEXTS)
		{
			PANEL_text_head=PANEL_text_tail-(PANEL_MAX_TEXTS-1);
		}



		for (i = PANEL_text_head; i < PANEL_text_tail; i++)
		{
			pt = &PANEL_text[i & (PANEL_MAX_TEXTS - 1)];

			if (i == PANEL_text_head)
			{
				if (pt->delay == 0)
				{
					PANEL_text_head += 1;
				}
			}

			if (pt->delay != 0)
			{
				//
				// Draw this message. Start off with the face.
				//

				ybase = y;

				PANEL_new_face(
					pt->who,
					x1,
					ybase - 2,
					PANEL_FACE_SMALL);

				//
				// The text....
				//

				height  = FONT2D_DrawStringWrapTo (	pt->text,
													x1 + 36 + 6,
													y+2,
													0xffffff,
													256,
													POLY_PAGE_FONT2D,
													0,
													x2
													) - y;
				height += 20;

				//
				// The speech bubble.
				//

				PANEL_last_bubble(
					x1 + 36,
					ybase - 4,
					float(FONT2D_rightmost_x) + 6.0F,
					ybase - 2 + height);

#ifdef TARGET_DC
				// draw it again, so it's brighter.
				FONT2D_DrawStringWrapTo (	pt->text,
													x1 + 36 + 6,
													y+2,
													0xffffff,
													256,
													POLY_PAGE_FONT2D,
													0,
													x2
													);

#endif

				if (height < 34)
				{
					height = 34;
				}

				y += height;
			}
		}
	}


#ifdef TARGET_DC
	// This is instead of that wank PANEL_draw_buffered tripe.
	if ( slPANEL_draw_timer_time >= 0 )
	{
		float time = slPANEL_draw_timer_time * ( 1.0f / 100.0f );

		CBYTE countdown[8];

		SLONG mins = 0;

		ASSERT(time < 1000.0F);

		while(time >= 60.0F)
		{
			mins += 1;
			time -= 60.0F;
		}

		sprintf(countdown, "%02d:%02d", mins, SLONG(time));

		if ((time<30)&&!mins)
		{
			static UWORD pulse=0;
			SLONG colour;
			pulse+=(TICK_RATIO*80)>>TICK_SHIFT;
			colour=(SIN(pulse&2047)>>9)+128;
			colour=colour|(colour<<8);
			FONT2D_DrawStringCentred(countdown, m_iPanelXPos + 171, m_iPanelYPos - 118, 0xff0000|colour, 256 + 64);
		}
		else
		{
			FONT2D_DrawStringCentred(countdown, m_iPanelXPos + 171, m_iPanelYPos - 118, 0xffffff, 256 + 64);
		}

		slPANEL_draw_timer_time = -1;
	}
#endif


	//
	// Draw the signs.
	//

	SLONG dtime = GetTickCount() - PANEL_sign_time;

	if (dtime < 3000)
	{
		dtime %= 600;

		if (dtime < 400)
		{
			float du;
			float dv;

			float u_mid;
			float v_mid;

			u_mid = (PANEL_sign_which & 1) ? 0.75F : 0.25F;
			v_mid = (PANEL_sign_which & 2) ? 0.75F : 0.25F;

			du = (PANEL_sign_flip & PANEL_SIGN_FLIP_LEFT_AND_RIGHT) ? -0.25F : +0.25F;
			dv = (PANEL_sign_flip & PANEL_SIGN_FLIP_TOP_AND_BOTTOM) ? -0.25F : +0.25F;

			#define PANEL_SIGN_X 320
			//#define PANEL_SIGN_Y 100
			int iYPos;
			if ( bPanelIsAtBottomOfScreen )
			{
				iYPos = 100;
			}
			else
			{
				iYPos = 480 - 100;
			}

			PANEL_draw_quad(
				PANEL_SIGN_X - 64,
				iYPos - 64,
				PANEL_SIGN_X + 64,
				iYPos + 64,
				POLY_PAGE_SIGN,
				0xffffffff,
				u_mid - du,
				v_mid - dv,
				u_mid + du,
				v_mid + dv);
		}
	}

	//
	// The panel search mode.
	//

	{
		Thing *darci = NET_PERSON(0);

		if(darci)
		{
			if(darci->State==STATE_SEARCH)
			{
				//
				// Percentage complete...
				//

				float percent;

				percent = darci->Genus.Person->Timer1 * (1.0F / (100.0F * 256.0F));

				SATURATE(percent, 0.0F, 1.0F);

				//
				// Draw the background box...
				//

				PANEL_last_bubble(
					320 - 80,
					220 - 16,
					320 + 80,
					220 + 16);

				//
				// The completion bar...
				//

				PANEL_draw_quad(
					320 - 75,
					220 - 12,
					320 - 75 + 151 * percent,
					220 + 13,
					POLY_PAGE_COLOUR,
					0x7788bb);

				if ((darci->Genus.Person->Timer1 & 0xfff) < 3000 || percent == 1.0F)
				{
					CBYTE *text;

					if (percent == 1.0F)
					{
						text = XLAT_str(X_COMPLETE);
					}
					else
					{
						text = XLAT_str(X_SEARCHING);
					}

					FONT2D_DrawStringCentred(text, 320, 220 - 8, 0xffffff);
				}
			}
		}
	}

	{

		int iYPos = bPanelIsAtBottomOfScreen ? 0 : 360;


#ifdef TARGET_DC
	extern bool g_bShowDarcisPositionOnScreen;
		CBYTE text[64];
		if ( g_bShowDarcisPositionOnScreen )
		{
			sprintf(text, "Darci is at (%d,%d)", NET_PERSON(0)->WorldPos.X >> 16, NET_PERSON(0)->WorldPos.Z >> 16);
			PANEL_crap_text ( 320, iYPos + 40, text );
		}

	extern bool g_bCheatsEnabled;
		if ( g_bCheatsEnabled )
		{
			sprintf(text, "Cheats are enabled" );
			PANEL_crap_text ( 320, iYPos + 20, text );
		}

	extern bool m_bTweakFramerates;
	extern int m_iDCFramerateMin;
	extern int m_iDCFramerateMax;
	extern SLONG CurDrawDistance;
		if ( m_bTweakFramerates )
		{
			sprintf(text, "Lo %i, hi %i, draw %f", m_iDCFramerateMin, m_iDCFramerateMax, (float)CurDrawDistance / 256.0f );
			PANEL_crap_text ( 320, iYPos + 100, text );
		}

#endif

#ifdef DEBUG
#ifdef TARGET_DC
		// Show the cheat string on screen.
	extern char cCheatString[];
		if ( cCheatString[0] != '\0' )
		{
			sprintf(text, "Cheat string: <%s>", cCheatString );
			PANEL_crap_text ( 320, iYPos + 80, text );
		}
#endif

#if 0
		// Show the fog table mode stuff.
extern float m_fFogTableDebugStart;
extern float m_fFogTableDebugEnd;
extern float m_fFogTableDebugDensity;
extern DWORD m_dwFogTableDebugFogTableMode;
		char *pcMode;
		switch ( m_dwFogTableDebugFogTableMode )
		{
		case D3DFOG_LINEAR:
			pcMode = "Lin";
			break;
		case D3DFOG_EXP:
			pcMode = "Exp";
			break;
		case D3DFOG_EXP2:
			pcMode = "Exp2";
			break;
		case D3DFOG_NONE:
			pcMode = "None";
			break;
		default:
			pcMode = "None";
			break;
		}
		sprintf ( text, "Start %f, end %f, density %f, mode %s",
						m_fFogTableDebugStart,
						m_fFogTableDebugEnd,
						m_fFogTableDebugDensity,
						pcMode );
		PANEL_crap_text ( 20, iYPos + 100, text );
#endif

#endif


#ifdef TARGET_DC
#ifdef _DEBUG
extern int g_iCacheReplacements;
extern bool g_bCacheReplacementThrash;
		// Show the cache performance.
		static char cCacheHistory[31] = "\0";

		for ( int i = 29; i > 0; i-- )
		{
			cCacheHistory[i] = cCacheHistory[i-1];
		}
		cCacheHistory[30] = '\0';
		if ( g_iCacheReplacements > 0 )
		{
			if ( g_iCacheReplacements < 100 )
			{
				cCacheHistory[0] = "0123456789"[g_iCacheReplacements/10];
			}
			else
			{
				// Nads - way too many.
				cCacheHistory[0] = '+';
			}
		}
		else
		{
			cCacheHistory[0] = ' ';
		}

		PANEL_crap_text ( 320, iYPos + 60, cCacheHistory );
		if ( g_bCacheReplacementThrash )
		{
			// Thrash!
			PANEL_crap_text ( 310, iYPos + 60, "T" );
		}
		g_iCacheReplacements = 0;
		g_bCacheReplacementThrash = FALSE;

		// And show the actual cache behaviour.
extern int m_iLRUQueueSize;
extern DWORD m_dwSizeOfQueue;
		sprintf ( text, "Length %i, size %i",
						m_iLRUQueueSize,
						m_dwSizeOfQueue );
		PANEL_crap_text ( 310, iYPos + 80, text );

#endif
#endif
	}


	#ifndef TARGET_DC

	extern SLONG FARFACET_num_squares_drawn;

	{
		CBYTE text[64];

		sprintf(text, "FARFACET squares drawn: %d", FARFACET_num_squares_drawn);



		FONT2D_DrawString(
			text,
			51,
			51,
			0x000000,
			256,
			POLY_PAGE_FONT2D,
			0);

		FONT2D_DrawString(
			text,
			50,
			50,
			0xffffff,
			256,
			POLY_PAGE_FONT2D,
			0);

	}


	extern UBYTE just_asked_for_mode_now;
	extern UBYTE just_asked_for_mode_number;
	extern float music_volume;
	extern SLONG MUSIC_is_playing(void);


	#ifdef _DEBUG
	if (just_asked_for_mode_now)
	{
		just_asked_for_mode_now = FALSE;

		CBYTE text[64];

		sprintf(text, "music mode %d vol %f %s", just_asked_for_mode_number, music_volume, (MUSIC_is_playing()) ? "Playing" : "Silence");

		FONT2D_DrawString(
			text,
			320,
			200,
			0xffff,
			256,
			POLY_PAGE_FONT2D,
			0);
	}
	#endif
	#endif



#ifndef TARGET_DC
	static SLONG i_know = 0;
	static SLONG the_answer = 0;

	if (!i_know)
	{
		the_answer = ENV_get_value_number("iamapsx",FALSE);
		i_know     = TRUE;
	}

	if (the_answer)
	{
		FONT2D_DrawString("PSX mode", 7, 17, 0x000000);
		FONT2D_DrawString("PSX mode", 5, 15, 0xff2288);
	}
#endif

#ifndef TARGET_DC
	{
		static ULONG timestamp_colour = 0;
		static CBYTE version_number[128];

		if (Keys[KB_V])
		{
			timestamp_colour = 0xf0f0f0f0;
		}

		if (timestamp_colour)
		{
			if (!version_number[0])
			{
				CBYTE ts[256];
				float vn;

				sprintf(ts, __DATE__);

				CBYTE *month[12] =
				{
					"Jan",
					"Feb",
					"Mar",
					"Apr",
					"May",
					"Jun",
					"Jul",
					"Aug",
					"Sep",
					"Oct",
					"Nov",
					"Dec"
				};

				SLONG i;

				vn = 0.0F;

				for (i = 0; i < 12; i++)
				{
					if (toupper(ts[0]) == toupper(month[i][0]) &&
						toupper(ts[1]) == toupper(month[i][1]) &&
						toupper(ts[2]) == toupper(month[i][2]))
					{
						vn = i - 8.0F;
					}
				}

				SLONG day = atoi(ts + 4);

				vn += day * 0.03F;

				SLONG year = atoi(ts + 7);

				vn += (year - 1999) * 12;

				sprintf(version_number, "Version number %.2f : Compiled %s", vn, __DATE__);
			}

			timestamp_colour -= 0x10101010;

			//
			// Show the version number.
			//

			FONT2D_DrawString(
				version_number,
				22,
				22,
				0x00000000);

			FONT2D_DrawString(
				version_number,
				20,
				20,
				timestamp_colour);
		}
	}
#endif




#if 0
	// FINAL BUILD DONE! HOORAY!

#ifdef TARGET_DC
	// Just for non-final builds.
	static CBYTE version_number[64] = "";

	if ( version_number[0] == '\0' )
	{
		CBYTE ts[40] = __DATE__;
		float vn;

		CBYTE *month[12] =
		{
			"Jan",
			"Feb",
			"Mar",
			"Apr",
			"May",
			"Jun",
			"Jul",
			"Aug",
			"Sep",
			"Oct",
			"Nov",
			"Dec"
		};

		SLONG i;

		vn = 0.0F;

		for (i = 0; i < 12; i++)
		{
			if (toupper(ts[0]) == toupper(month[i][0]) &&
				toupper(ts[1]) == toupper(month[i][1]) &&
				toupper(ts[2]) == toupper(month[i][2]))
			{
				vn = i - 8.0F;
			}
		}

		SLONG day = atoi(ts + 4);

		vn += day * 0.03F;

		SLONG year = atoi(ts + 7);

		vn += (year - 1999) * 12;

		sprintf(version_number, "Version %.2f", vn );
	}


extern bool bDontShowThePauseScreen;
	if ( !bDontShowThePauseScreen )
	{
		FONT2D_DrawString(
			"(C)2000 Mucky Foot Productions",
			20,
			20,
			0xffffffff);

		FONT2D_DrawString(
			"Test version - not for release",
			20,
			35,
			0xffffffff);

		FONT2D_DrawString(
			version_number,
			20,
			50,
			0xffffffff);
	}

#endif

#endif


	//TRACE ( "PDo" );

}


#ifdef TARGET_DC

static int iVMUAmmoDrawCountdown = 0;

static VMU_Screen vmuscreenTemp;


// The byte patterns of numbers.
#define FROM_BINARY(a,b,c,d,e) ( (a<<4) | (b<<3) | (c<<2) | (d<<1) | (e<<0) )
UBYTE bNumberPattern[11][7] =
{
	{	// 0
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// 1
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,1,1,1,1 ),
	},
	{	// 2
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,1,1,0,0 ),
		FROM_BINARY ( 1,1,0,0,0 ),
		FROM_BINARY ( 1,1,1,1,1 ),
	},
	{	// 3
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// 4
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,1,1,1 ),
		FROM_BINARY ( 0,1,1,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,1,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
	},
	{	// 5
		FROM_BINARY ( 1,1,1,1,1 ),
		FROM_BINARY ( 1,1,0,0,0 ),
		FROM_BINARY ( 1,1,1,1,0 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// 6
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,0,0 ),
		FROM_BINARY ( 1,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// 7
		FROM_BINARY ( 1,1,1,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,1,1,1 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 0,1,1,0,0 ),
		FROM_BINARY ( 0,1,1,0,0 ),
	},
	{	// 8
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// 9
		FROM_BINARY ( 0,1,1,1,0 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 1,1,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,1,1,1,0 ),
	},
	{	// separator dot.
		FROM_BINARY ( 0,0,0,0,0 ),
		FROM_BINARY ( 0,0,0,0,0 ),
		FROM_BINARY ( 0,0,0,1,1 ),
		FROM_BINARY ( 0,0,1,1,1 ),
		FROM_BINARY ( 0,0,1,1,0 ),
		FROM_BINARY ( 0,0,0,0,0 ),
		FROM_BINARY ( 0,0,0,0,0 ),
	},
};

void Panel_Draw_VMU_Character ( int iCharacter, UBYTE *pbScanline, int iXpos )
{
	// Can't do anything too close to the left edge.
	ASSERT ( iXpos < ( 48 - 16 ) );
	// Go to correct X pos (right-hand byte).
	pbScanline += 5 - ( iXpos >> 3 );

	UBYTE *pbSrc = bNumberPattern[iCharacter];
	int iShift = iXpos & 0x7;

	for ( int i = 0; i < 7; i++ )
	{
		UWORD uwSrc = (UWORD)*pbSrc++;
		uwSrc <<= iShift;

		*(pbScanline-0) |= (UBYTE)( uwSrc & 0xff );
		*(pbScanline-1) |= (UBYTE)( uwSrc >> 8 );

		pbScanline += 6;
	}
}


void PANEL_draw_VMU_ammo_counts ( void )
{
	iVMUAmmoDrawCountdown--;
	if ( iVMUAmmoDrawCountdown > 0 )
	{
		return;
	}

	iVMUAmmoDrawCountdown = 30;



	// OK, find out how much ammo of each sort we have, and whether we have the weapons or not.


	// This order matches the order on the VMU screen.
	enum eWeaponNumber
	{
		WEAP_PISTOL = 0,
		WEAP_SHOTGUN = 1,
		WEAP_M16 = 2,
		WEAP_GRENADE = 3,
	};

	int i;
	bool bHaveWeapon[4];
	int iAmmo[4];
	int iMags[4];
	for ( i = 0; i < 4; i++ )
	{
		bHaveWeapon[i] = FALSE;
		iAmmo[i] = 0;
		iMags[i] = 0;
	}

	// Run through the weapons.
	Thing *darci  = NET_PERSON(0);
	Thing *p_special = NULL;

	// The pistol is special.
	if (darci->Flags & FLAGS_HAS_GUN)
	{
		bHaveWeapon[WEAP_PISTOL] = TRUE;
	}


	// THIS IS FUCKING INSANE GUYS. So I'm just going to type it in, and if it doesn't work, it's your fault.
	iMags[WEAP_PISTOL] = darci->Genus.Person->ammo_packs_pistol / 15;
	iAmmo[WEAP_PISTOL] = darci->Genus.Person->Ammo;

	iMags[WEAP_SHOTGUN] = darci->Genus.Person->ammo_packs_shotgun / SPECIAL_AMMO_IN_A_SHOTGUN;
	iMags[WEAP_M16] = darci->Genus.Person->ammo_packs_ak47 / SPECIAL_AMMO_IN_A_AK47;

	ASSERT ( ( darci->Genus.Person->ammo_packs_pistol % 15 ) == 0 );
	ASSERT ( ( darci->Genus.Person->ammo_packs_ak47 % SPECIAL_AMMO_IN_A_AK47 ) == 0 );
	ASSERT ( ( darci->Genus.Person->ammo_packs_shotgun % SPECIAL_AMMO_IN_A_SHOTGUN ) == 0 );

	if ( darci->Genus.Person->SpecialList != NULL )
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special)
		{
			ASSERT(p_special->Class == CLASS_SPECIAL);
			switch ( p_special->Genus.Special->SpecialType )
			{
			case SPECIAL_SHOTGUN:
				// We have a shotty.
				ASSERT ( !bHaveWeapon[WEAP_SHOTGUN] );
				bHaveWeapon[WEAP_SHOTGUN] = TRUE;
				iAmmo[WEAP_SHOTGUN] = p_special->Genus.Special->ammo;
				break;
			case SPECIAL_AK47:
				// We have an AK47, which we now call an M16.
				ASSERT ( !bHaveWeapon[WEAP_M16] );
				bHaveWeapon[WEAP_M16] = TRUE;
				iAmmo[WEAP_M16] = p_special->Genus.Special->ammo;
				break;
			case SPECIAL_GRENADE:
				// Should only have one grenade special.
				ASSERT ( !bHaveWeapon[WEAP_GRENADE] );
				bHaveWeapon[WEAP_GRENADE] = TRUE;
				iAmmo[WEAP_GRENADE] = p_special->Genus.Special->ammo;
				if ( iAmmo[WEAP_GRENADE] == 0 )
				{
					// Er... this means we don't have grenades.
					bHaveWeapon[WEAP_GRENADE] = FALSE;
				}
				break;
			default:
				// You think I care?
				break;
			}

			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}


	// Crappy fudge. Driving Bronze and other training missions start you off
	// with 15 pistol rounds, but no pistol, which is silly. So if we don't have a
	// a pistol, pretend we have no rounds or clips as well.
	if ( !bHaveWeapon[WEAP_PISTOL] )
	{
		iAmmo[WEAP_PISTOL] = 0;
		iMags[WEAP_PISTOL] = 0;
	}

extern VMU_Screen *pvmuscreenAmmo;

	// Copy the weapon bitmaps over.
	memcpy ( vmuscreenTemp.bData, pvmuscreenAmmo->bData, 32 * 6 );
	vmuscreenTemp.bRotated = FALSE;

#define WEAPON_BAR_SCANLINE_SIZE 8
	for ( i = 0; i < 4; i++ )
	{
		if ( !bHaveWeapon[i] )
		{
			if ( ( iAmmo[i] == 0 ) && ( iMags[i] == 0 ) )
			{
				// No ammo or mags and we don't have it. So blank it.
				memset ( vmuscreenTemp.bData + 6 * WEAPON_BAR_SCANLINE_SIZE * i, 0, 6 * WEAPON_BAR_SCANLINE_SIZE );
			}
			else
			{
				// We have some ammo, but don't actually have the weapon yet. Grey it out.
				UBYTE *pbScreen = vmuscreenTemp.bData + 6 * WEAPON_BAR_SCANLINE_SIZE * i;
				UBYTE bMask = 0x55;
				for ( int j = 0; j < WEAPON_BAR_SCANLINE_SIZE; j++ )
				{
					*pbScreen++ &= bMask;
					*pbScreen++ &= bMask;
					*pbScreen++ &= bMask;
					*pbScreen++ &= bMask;
					*pbScreen++ &= bMask;
					*pbScreen++ &= bMask;
					bMask = ~bMask;
				}
			}
		}


// X positions of the left side of the char, counted from the left of the screen.
#define WEAPON_MAGS_TENS 22
#define WEAPON_MAGS_UNIT 16
#define WEAPON_SEPARATOR 12
#define WEAPON_AMMO_TENS  6
#define WEAPON_AMMO_UNIT  0

		UBYTE *pbScreen = vmuscreenTemp.bData + 6 * WEAPON_BAR_SCANLINE_SIZE * i;
		if ( iMags[i] > 0 )
		{
			// Draw mag number.
			if ( iMags[i] >= 10 )
			{
				Panel_Draw_VMU_Character ( ( iMags[i] / 10 ), pbScreen, WEAPON_MAGS_TENS );
			}
			Panel_Draw_VMU_Character ( ( iMags[i] % 10 ), pbScreen, WEAPON_MAGS_UNIT );
			// The separator - character 10.
			Panel_Draw_VMU_Character ( 10, pbScreen, WEAPON_SEPARATOR );
		}

		if ( ( iMags[i] > 0 ) || ( iAmmo[i] > 0 ) || bHaveWeapon[i] )
		{
			// Draw ammo number.
			if ( iAmmo[i] >= 10 )
			{
				Panel_Draw_VMU_Character ( ( iAmmo[i] / 10 ), pbScreen, WEAPON_AMMO_TENS );
			}
			Panel_Draw_VMU_Character ( ( iAmmo[i] % 10 ), pbScreen, WEAPON_AMMO_UNIT );
		}
	}

	// And write the screen out.
	WriteLCDScreenToCurrentController ( &vmuscreenTemp );

#undef WEAPON_BAR_SCANLINE_SIZE
#undef WEAPON_MAGS_TENS
#undef WEAPON_MAGS_UNIT
#undef WEAPON_SEPARATOR
#undef WEAPON_AMMO_TENS
#undef WEAPON_AMMO_UNIT

}


#endif


#ifndef TARGET_DC

void PANEL_draw_completion_bar(SLONG completion)
{
	#define START_R 50
	#define START_G 59
	#define START_B	80

	#define END_R 210
	#define END_G 216
	#define END_B 208

	SLONG along;

	SLONG r;
	SLONG g;
	SLONG b;

#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif

	SLONG i;

	SLONG colour;

	for (i = 0; i < (completion >> 3); i += 1)
	{
		r = START_R + (END_R - START_R) * i >> 5;
		g = START_G + (END_G - START_G) * i >> 5;
		b = START_B + (END_B - START_B) * i >> 5;

		colour = (r << 16) | (g << 8) | (b << 0);

		PANEL_draw_quad(
			5  + i * 20,  455,
			23 + i * 20,  475,
			POLY_PAGE_COLOUR,
			colour);
	}

#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
#endif
}

#endif






bool bScreensaverEnabled = FALSE;
// Darkness of screensaver, from 0(off)->0xffff(full on)
int iScreenSaverDarkness = 0;

int iScreensaverXPos = 320;
int iScreensaverYPos = 240;
int iScreensaverXInc = 4;
int iScreensaverYInc = 4;
int iScreensaverAngle = 0;
int iScreensaverAngleInc = 0x2ff;

DWORD dwPseudorandomSeed = 0;

DWORD dwGetRandomishNumber ( void )
{
	dwPseudorandomSeed *= 51929;
	dwPseudorandomSeed ^= dwPseudorandomSeed >> 3;
	dwPseudorandomSeed ^= dwPseudorandomSeed << 8;
	dwPseudorandomSeed += 31415;

	return ( dwPseudorandomSeed >> 8 );
}


#define SCREENSAVER_SIZE 128

void PANEL_enable_screensaver ( void )
{
	if ( !bScreensaverEnabled )
	{
		bScreensaverEnabled = TRUE;
		iScreensaverXPos = 320;
		iScreensaverYPos = 240;
		iScreensaverXInc = 4;
		iScreensaverYInc = 4;
	}
}

void PANEL_disable_screensaver ( bool bImmediately )
{
	bScreensaverEnabled = FALSE;
	if ( bImmediately )
	{
		// Bin the fade in.
		iScreenSaverDarkness = 0;
	}
}

void PANEL_screensaver_draw ( void )
{
	if ( bScreensaverEnabled )
	{
		iScreenSaverDarkness += 100;
		if ( iScreenSaverDarkness > 0xffff )
		{
			iScreenSaverDarkness = 0xffff;
		}
	}
	else
	{
		iScreenSaverDarkness -= 10000;
		if ( iScreenSaverDarkness < 0 )
		{
			iScreenSaverDarkness = 0;
		}
	}

	if ( iScreenSaverDarkness == 0 )
	{
		// don't do it.
		return;
	}


#ifndef TARGET_DC
	POLY_frame_init(FALSE,FALSE);
#endif



	// Bounce around the screen.
	iScreensaverXPos += iScreensaverXInc;
	iScreensaverYPos += iScreensaverYInc;
	iScreensaverAngle += iScreensaverAngleInc;
	if ( iScreensaverXPos > 640 - SCREENSAVER_SIZE )
	{
		iScreensaverXPos = 640 - SCREENSAVER_SIZE;
		iScreensaverXInc = -( (signed)( dwGetRandomishNumber() & 0x3 ) + 2 );
		iScreensaverAngleInc = ( (signed)( dwGetRandomishNumber() & 0xfff ) - 0x7ff );
	}
	else if ( iScreensaverXPos < 0 )
	{
		iScreensaverXPos = 0;
		iScreensaverXInc = ( (signed)( dwGetRandomishNumber() & 0x3 ) + 2 );
		iScreensaverAngleInc = ( (signed)( dwGetRandomishNumber() & 0xfff ) - 0x7ff );
	}

	if ( iScreensaverYPos > 480 - SCREENSAVER_SIZE )
	{
		iScreensaverYPos = 480 - SCREENSAVER_SIZE;
		iScreensaverYInc = -( (signed)( dwGetRandomishNumber() & 0x3 ) + 2 );
		iScreensaverAngleInc = ( (signed)( dwGetRandomishNumber() & 0xfff ) - 0x7ff );
	}
	else if ( iScreensaverYPos < 0 )
	{
		iScreensaverYPos = 0;
		iScreensaverYInc = ( (signed)( dwGetRandomishNumber() & 0x3 ) + 2 );
		iScreensaverAngleInc = ( (signed)( dwGetRandomishNumber() & 0xfff ) - 0x7ff );
	}

	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	DWORD dwColour = ( ( iScreenSaverDarkness & 0xff00 ) << 16 );

	float fSinAngle, fCosAngle;
	float fAngle = (float)( iScreensaverAngle & 0xffff ) * ( 2.0f * 3.1415927f / 65536.0f );
#ifdef TARGET_DC
	_SinCosA ( &fSinAngle, &fCosAngle, fAngle );
#else
	fSinAngle = sinf ( fAngle );
	fCosAngle = cosf ( fAngle );
#endif
	fSinAngle *= 0.65f;
	fCosAngle *= 0.65f;

	// First draw the "spotlight" logo.
	pp[0].X        = (float)iScreensaverXPos;
	pp[0].Y        = (float)iScreensaverYPos;
	pp[0].z        = 0.0f;
	pp[0].Z        = 0.99999f;
	pp[0].u        = 0.5f + fSinAngle;
	pp[0].v        = 0.5f + fCosAngle;
	pp[0].colour   = dwColour;
	pp[0].specular = 0xff000000;

	pp[1].X        = (float)iScreensaverXPos + SCREENSAVER_SIZE;
	pp[1].Y        = (float)iScreensaverYPos;
	pp[1].z        = 0.0f;
	pp[1].Z        = 0.99999f;
	pp[1].u        = 0.5f - fCosAngle;
	pp[1].v        = 0.5f + fSinAngle;
	pp[1].colour   = dwColour;
	pp[1].specular = 0xff000000;

	pp[2].X        = (float)iScreensaverXPos;
	pp[2].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[2].z        = 0.0f;
	pp[2].Z        = 0.99999f;
	pp[2].u        = 0.5f + fCosAngle;
	pp[2].v        = 0.5f - fSinAngle;
	pp[2].colour   = dwColour;
	pp[2].specular = 0xff000000;

	pp[3].X        = (float)iScreensaverXPos + SCREENSAVER_SIZE;
	pp[3].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[3].z        = 0.0f;
	pp[3].Z        = 0.99999f;
	pp[3].u        = 0.5f - fSinAngle;
	pp[3].v        = 0.5f - fCosAngle;
	pp[3].colour   = dwColour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_FADE_MF, FALSE, TRUE);

	// Now draw the darkener around it.
	// Top block.
	pp[0].X        = 0.0f;
	pp[0].Y        = 0.0f;
	pp[1].X        = 640.0f;
	pp[1].Y        = 0.0f;
	pp[2].X        = 0.0f;
	pp[2].Y        = (float)iScreensaverYPos;
	pp[3].X        = 640.0f;
	pp[3].Y        = (float)iScreensaverYPos;
	POLY_add_quad(quad, POLY_PAGE_COLOUR_ALPHA, FALSE, TRUE);

	// Bottom block.
	pp[0].X        = 0.0f;
	pp[0].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[1].X        = 640.0f;
	pp[1].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[2].X        = 0.0f;
	pp[2].Y        = 480.0f;
	pp[3].X        = 640.0f;
	pp[3].Y        = 480.0f;
	POLY_add_quad(quad, POLY_PAGE_COLOUR_ALPHA, FALSE, TRUE);

	// Left block.
	pp[0].X        = 0.0f;
	pp[0].Y        = (float)iScreensaverYPos;
	pp[1].X        = (float)iScreensaverXPos;
	pp[1].Y        = (float)iScreensaverYPos;
	pp[2].X        = 0.0f;
	pp[2].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[3].X        = (float)iScreensaverXPos;
	pp[3].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	POLY_add_quad(quad, POLY_PAGE_COLOUR_ALPHA, FALSE, TRUE);

	// Right block.
	pp[0].X        = (float)iScreensaverXPos + SCREENSAVER_SIZE;
	pp[0].Y        = (float)iScreensaverYPos;
	pp[1].X        = 640.0f;
	pp[1].Y        = (float)iScreensaverYPos;
	pp[2].X        = (float)iScreensaverXPos + SCREENSAVER_SIZE;
	pp[2].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	pp[3].X        = 640.0f;
	pp[3].Y        = (float)iScreensaverYPos + SCREENSAVER_SIZE;
	POLY_add_quad(quad, POLY_PAGE_COLOUR_ALPHA, FALSE, TRUE);


#ifndef TARGET_DC
	POLY_frame_draw(FALSE,FALSE);
#endif

}
