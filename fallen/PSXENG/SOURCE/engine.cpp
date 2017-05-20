//#include	"game.h"
//#include	<MFStdLib.h>
#include	"game.h"
#include	"facet.h"
#include	"c:\fallen\headers\memory.h"
#include	"c:\fallen\psxeng\headers\engine.h"
#include	"c:\fallen\headers\pap.h"
#include	"c:\fallen\headers\ob.h"
#include "c:\fallen\headers\supermap.h"
#ifdef INSIDES_EXIST
#include "c:\fallen\headers\inside2.h"
#endif
#include "c:\fallen\headers\dirt.h"
#include "c:\fallen\headers\night.h"
#include "c:\fallen\headers\bike.h"
#include "c:\fallen\headers\trip.h"
#include "c:\fallen\headers\fc.h"
#include "c:\fallen\headers\fmatrix.h"
#include "c:\fallen\ddengine\headers\drawxtra.h"
#include "c:\fallen\headers\psystem.h"
#include "c:\fallen\headers\animate.h"
#include "c:\fallen\headers\statedef.h"
#include "c:\fallen\headers\spark.h"
#include "c:\fallen\headers\pow.h"
#include "c:\fallen\headers\eway.h"
#include "c:\fallen\headers\mav.h"
#include "c:\fallen\headers\grenade.h"
#include "c:\fallen\headers\pcom.h"
#include "c:\fallen\headers\ware.h"

#include "libpad.h"
#include "libapi.h"

#ifdef VERSION_KANJI
#include "kanji.h"
#endif

#include "panel.h"

extern	SLONG		EWAY_cam_jumped;

extern	UBYTE	roper_pickup;
UBYTE	remove_dead_people;
UBYTE	in_ware=0;

void SHAPE_droplet(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		ULONG colour,
		SLONG page);

SLONG	sea_offset=0;

// For Filtering we need the pad input crap here.

#include	"ctrller.h"
extern ControllerPacket PAD_Input1,PAD_Input2;

// **********************************************

#define	FLOOR_SPLIT_DIST	700

#define AENG_DRAW_DIST	psx_draw_dist

#define	PI16	(SLONG)(3.14159265*65536.0)

void AENG_calc_gamut(SLONG dist);
void PANEL_draw_beacons(void);

extern void NIGHT_lum_init();
extern	void FIGURE_draw_queued(Thing *p_thing,SLONG dist);
extern	void	DoFigureDraw(void);

SLONG	my_draw_dist=20<<8;

SLONG AENG_cam_x;
SLONG AENG_cam_y;
SLONG AENG_cam_z;

SLONG AENG_cam_vec[3];

SLONG AENG_cam_yaw;
SLONG AENG_cam_pitch;
SLONG AENG_cam_roll;
SLONG AENG_lens = (SLONG)(3.3F*65536);
SLONG TEXTURE_set;
UBYTE	double_ot=0;

SLONG psx_draw_dist=13;
UWORD	debug_count[10];
PSX_POLY_Point *perm_pp_array;

#define MAX_PP_ARRAY 4

#define	AENG_LENS	AENG_lens

#if 0

#define AMBIENT_MODE_OFF   0
#define AMBIENT_MODE_RED   1
#define AMBIENT_MODE_GREEN 2
#define AMBIENT_MODE_BLUE  3

SLONG AENG_ambient_mode=AMBIENT_MODE_OFF;
SLONG AENG_ambient_last=AMBIENT_MODE_OFF;

#define AMB_TEXT_HIGH	0x007f7f00
#define AMB_TEXT_LOW	0x003f3f00

SLONG store_amb_red,store_amb_green,store_amb_blue;

#define AENG_AMBIENT_MOVELIGHT	-1

SLONG AENG_amb_focus;
extern void NIGHT_light_the_map(void);

#endif

#if 0
#define ED_MAX_LIGHTS 256

typedef struct
{
	UBYTE range;
	SBYTE red;
	SBYTE green;
	SBYTE blue;
	UBYTE next;
	UBYTE used;
	UBYTE flags;
	UBYTE padding;
//	UWORD padding;
	SLONG x;
	SLONG y;
	SLONG z;
	
} ED_Light;

typedef struct
{
	ED_Light     ed_light[ED_MAX_LIGHTS];
	SLONG        ed_light_free;
	ULONG        night_flag;
	ULONG        night_amb_d3d_colour;
	ULONG        night_amb_d3d_specular;
	SLONG        night_amb_red;
	SLONG        night_amb_green;
	SLONG        night_amb_blue;
	SBYTE        night_lampost_red;
	SBYTE        night_lampost_green;
	SBYTE        night_lampost_blue;
	UBYTE        padding;
	SLONG        night_lampost_radius;
	NIGHT_Colour night_sky_colour;

} ED_Undo;

ED_Undo Light_Save;
/*
void	check_prim_ptr_ni(void **x)
{

	if(*x>the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-100)
		*x=the_display.CurrentDisplayBuffer->PrimMem;

	if( (*x>first_used_bucket_ram) && *x<(first_used_bucket_ram+100))
	{
//		DrawOTag(old_buffer->ot);	//draw the bucketlist
		DrawSync(0);
//		first_used_bucket_ram=last_used_bucket_ram;
		first_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM;
//		flipped=0;
	}
}
*/

SLONG AENG_ambient_movelight()
{
	SLONG moved=0;
	NIGHT_Slight *nsl;
	NIGHT_Smap *ns;
	SLONG x,z,vx,vz;

	SBYTE str[48];

	nsl=&NIGHT_slight[AENG_amb_focus];
	for(x=0;x<PAP_SIZE_LO;x++)
		for(z=0;z<PAP_SIZE_LO;z++)
		{
			ns=&NIGHT_smap[x][z];
			// Curious code here that checks the range of lights in the square to find
			// what square it's in.
			if ((ns->index<=AENG_amb_focus)&&(ns->index+ns->number>AENG_amb_focus))
			{
				vx=x;
				vz=z;
				x=z=PAP_SIZE_LO;
			}
		}

	BLOOM_draw((nsl->x<<2)+(vx<<PAP_SHIFT_LO),nsl->y,(nsl->z<<2)+(vz<<PAP_SHIFT_LO),0,0,0,((nsl->red+128)<<16)+((nsl->green+128)<<8)+(nsl->blue+128),0);

	sprintf(str,"(%d,%d,%d) (%d,%d,%d) %d",(nsl->x<<2)+(vx<<PAP_SHIFT_LO),nsl->y,(nsl->z<<2)+(vz<<PAP_SHIFT_LO),nsl->red,nsl->green,nsl->blue,nsl->radius);
	draw_text_at(32,32,str,0xffff00);

	if (!PadKeyIsPressed(&PAD_Input2,PAD_FRB))
	{
		if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
		{
			nsl->x-=2;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
		{
			nsl->x+=2;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_LU))
		{
			nsl->z-=2;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_LD))
		{
			nsl->z+=2;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_RU))
		{
			nsl->y+=8;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_RD))
		{
			nsl->y-=8;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_RL))
		{
			nsl->radius-=8;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_RR))
		{
			nsl->radius+=8;
			moved=1;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_START))
		{
	 		NIGHT_dfcache_recalc();
			NIGHT_light_the_map();
		}
	
	}
	else // We've pressed down R2 button, which allows us to change lights and edit colours.
	{
		draw_text_at(DISPLAYWIDTH-32,32,"R2",0xffff00);

		SLONG edit_colour=-1;
		if (PadKeyIsPressed(&PAD_Input2,PAD_RR))
			edit_colour=0;
		if (PadKeyIsPressed(&PAD_Input2,PAD_RU))
			edit_colour=1;
		if (PadKeyIsPressed(&PAD_Input2,PAD_RD))
			edit_colour=2;

		switch(edit_colour)
		{
		case -1:
			if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
				if (AENG_amb_focus>1)
					AENG_amb_focus--;
			if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
				if (AENG_amb_focus<NIGHT_slight_upto-2)
					AENG_amb_focus++;
			break;
		case 0:
			if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
				if (nsl->red>-124)
					nsl->red-=4;
			if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
				if (nsl->red<124)
					nsl->red+=4;
			break;
		case 1:
			if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
				if (nsl->green>-124)
					nsl->green-=4;
			if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
				if (nsl->green<124)
					nsl->green+=4;
			break;
		case 2:
			if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
				if (nsl->blue>-124)
					nsl->blue-=4;
			if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
				if (nsl->blue<124)
					nsl->blue+=4;
			break;
		}
	}

	if (moved)
		NIGHT_dfcache_recalc();

	draw_text_at(DISPLAYWIDTH-128,16,"Static Light Mover",0xffff00);

	return 0;
}
#endif

#if 0

void AENG_ambient_editor(int store)
{
	char str[32];
	int step;


	if (PadKeyIsPressed(&PAD_Input2,PAD_FLT))
	{
		if (AENG_ambient_mode==AENG_ambient_last)
			AENG_ambient_mode=AENG_ambient_mode?0:1;
	} else
		AENG_ambient_last=AENG_ambient_mode;
#if 0
	if (PadKeyIsPressed(&PAD_Input2,PAD_FLB))
	{
		SLONG vx,vz;
		vx=NET_PERSON(0)->WorldPos.X>>(PAP_SHIFT_LO+8);
		vz=NET_PERSON(0)->WorldPos.Z>>(PAP_SHIFT_LO+8);
		AENG_amb_focus=NIGHT_smap[vx][vz].index;
		AENG_ambient_mode=AENG_AMBIENT_MOVELIGHT;
	}
#endif
	if (store)
	{
		store_amb_red=NIGHT_amb_red;
		store_amb_green=NIGHT_amb_green;
		store_amb_blue=NIGHT_amb_blue;
	}
#if 0
	if (AENG_ambient_mode==AENG_AMBIENT_MOVELIGHT)
	{
		AENG_ambient_movelight();
		return;
	}
#endif
	if (AENG_ambient_mode==0)
	{
#ifndef	MIKE
		if (GAME_TURN&16)
			draw_text_at(DISPLAYWIDTH-32,16,"L1",AMB_TEXT_HIGH);
#endif
		return;
	}

	step=1;

	if (PadKeyIsPressed(&PAD_Input2,PAD_FRT))
		step<<=1;
	if (PadKeyIsPressed(&PAD_Input2,PAD_FRB))
		step<<=2;

	if (AENG_ambient_mode)
	{
		draw_text_at(DISPLAYWIDTH-160,16,"Ambient Light Editor",AMB_TEXT_HIGH);
		sprintf(str,"Red:\t%03d",NIGHT_amb_red);
		draw_text_at(DISPLAYWIDTH-128,32,str,(AENG_ambient_mode==1)?AMB_TEXT_HIGH:AMB_TEXT_LOW);
		sprintf(str,"Green:\t%03d",NIGHT_amb_green);
		draw_text_at(DISPLAYWIDTH-128,44,str,(AENG_ambient_mode==2)?AMB_TEXT_HIGH:AMB_TEXT_LOW);
		sprintf(str,"Blue:\t%03d",NIGHT_amb_blue);
		draw_text_at(DISPLAYWIDTH-128,56,str,(AENG_ambient_mode==3)?AMB_TEXT_HIGH:AMB_TEXT_LOW);
	}
	draw_text_at(DISPLAYWIDTH-160,72,"Press "STR_CROSS" to update",AMB_TEXT_HIGH);
	draw_text_at(DISPLAYWIDTH-160,84,"or "STR_CIRCLE" to reset",AMB_TEXT_HIGH);
	switch(AENG_ambient_mode)
	{
	case AMBIENT_MODE_OFF:
		break;
	case AMBIENT_MODE_RED:
		if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
			NIGHT_amb_red+=step;
		if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
			NIGHT_amb_red-=step;
		break;
	case AMBIENT_MODE_GREEN:
		if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
			NIGHT_amb_green+=step;
		if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
			NIGHT_amb_green-=step;
		break;
	case AMBIENT_MODE_BLUE:
		if (PadKeyIsPressed(&PAD_Input2,PAD_LR))
			NIGHT_amb_blue+=step;
		if (PadKeyIsPressed(&PAD_Input2,PAD_LL))
			NIGHT_amb_blue-=step;
		break;
	}

	if (PadKeyIsPressed(&PAD_Input2,PAD_LD))
		AENG_ambient_mode=(AENG_ambient_mode%3)+1;
	if (PadKeyIsPressed(&PAD_Input2,PAD_LU))
	{
		AENG_ambient_mode--;
		if (AENG_ambient_mode==0)
			AENG_ambient_mode=3;
	}

	if (PadKeyIsPressed(&PAD_Input2,PAD_RR))
	{
		NIGHT_amb_red=store_amb_red;
		NIGHT_amb_green=store_amb_green;
		NIGHT_amb_blue=store_amb_blue;
	}
	NIGHT_ambient(NIGHT_amb_red,NIGHT_amb_green,NIGHT_amb_blue,NIGHT_amb_norm_x,NIGHT_amb_norm_y,NIGHT_amb_norm_z);

	if (PadKeyIsPressed(&PAD_Input2,PAD_RD))
	{
		NIGHT_dfcache_recalc();
		NIGHT_light_the_map();
	}
}

#endif
volatile	SLONG draw_state __attribute__((section(".rdata")))=0;
SLONG	global_debug=0;
SLONG	global_debug2=0;
void	*sync_point=0;
void	*danger_point=0;
UBYTE	danger_point_type=0;


void	do_danger(void **x)
{
	SLONG	count=0;
	switch(danger_point_type)
	{
		case	0:
			//
			// end of bucket list
			//
			*x=the_display.CurrentDisplayBuffer->PrimMem; //loop to start
			danger_point=first_used_bucket_ram-200;
			danger_point_type=1;
			if(*x>danger_point)
				goto	jesus_that_was_unlikely;
			break;
		case	1:
jesus_that_was_unlikely:;

			//
			// start of currently being drawn prims
			global_debug2=draw_state;
			//
			while( draw_state>-1 && count <50000) //wait for vsync
			{
				count++;
			}

			//
			// now continue
			//
			global_debug=count;
			danger_point=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-200; // 200 hundred from end
			danger_point_type=0;
			break;
	}

}



//   start                     end
//                           |            (last used)
//     |                                  (current pointer)


//     |                                 (last used)
//                           |           (current pointer)
                
ULONG	available_bucket_ram(void)
{
	SLONG	size;
	size=last_used_bucket_ram-the_display.CurrentPrim; 
	if(size<0)
	{
		size=BUCKET_MEM-(-size);
	}
	return(size);
}

#ifdef	OLDSHIT
void	check_prim_ptr_ni(void **x)
{
	SLONG	count=0;
#ifdef	OLD_FLIP
	//
	// empty, so you better optimise me out
	//
#else
//	ASSERT(draw_state>=0);

//	ASSERT(*x<first_used_bucket_ram-200 || *x>first_used_bucket_ram+200);

	if(first_used_bucket_ram-200>*x && danger_point_type==0)
	{
		//
		// 
		//
		ASSERT(0);
	}

	if(*x>danger_point)
	{
		switch(danger_point_type)
		{
			case	0:
				//
				// end of bucket list
				//
				*x=the_display.CurrentDisplayBuffer->PrimMem; //loop to start
				danger_point=first_used_bucket_ram-200;
				danger_point_type=1;
				if(*x>danger_point)
					goto	jesus_that_was_unlikely;
				break;
			case	1:
jesus_that_was_unlikely:;

				//
				// start of currently being drawn prims
				//
				sync_point=*x;

				global_debug=draw_state;
				//DrawSync(0);
				while( draw_state>-1 && count <50000) //wait for vsync
				{
					count++;
				}
/*
				while(draw_state!=-1 && count <1000000) //wait for vsync
				{
					count++;
				}
*/
				global_debug+=count;

				//
				// now continue
				//
				danger_point=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-200; // 200 hundred from end
				danger_point_type=0;
//				ASSERT(0);
				break;
			default:
				ASSERT(0);
				break;
		}
	}

#endif
}
void	check_prim_ptr_old(void **x)
{
	SLONG	count=0;
#ifdef	OLD_FLIP
	//
	// empty, so you better optimise me out
	//
#else
	if(*x>the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-100)
		*x=the_display.CurrentDisplayBuffer->PrimMem;

	if( (*x>first_used_bucket_ram-100) && *x<(first_used_bucket_ram+100))
	{

//		DrawSync(0);   // we have caught up with the still being drawn buckets, so issue a draw sync

		while(draw_state && count <500000)
		{
			count++;
		}


/*
		while(count++<400000)//the_game.Packets[15])
		{
			count+=draw_state;
			//
			// wait for drawsync
			//
		}
*/

		global_debug=count;

		sync_point=*x;

		first_used_bucket_ram=last_used_bucket_ram;
		//
		// 
		//
//		first_used_bucket_ram=0;//the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM;
//		flipped=0;
	}
/*
	if( (*x>first_used_bucket_ram-100) && *x<(first_used_bucket_ram+200))
	{
		ASSERT(0);
	}
*/

	ASSERT(*x<last_used_bucket_ram-60 || *x>=last_used_bucket_ram); //check we havent caught up with ourselves
#endif
}
#endif
BOOL		text_fudge	=	FALSE;
ULONG       text_colour;

//void AENG_world_line_infinite(SLONG ix1, SLONG iy1, SLONG iz1, SLONG iwidth1, ULONG colour1, 
//		SLONG ix2, SLONG iy2, SLONG iz2, SLONG iwidth2, ULONG colour2,
//		SLONG sort_to_front);

void	add_debug_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG colour)
{
}

void	draw_debug_lines(void)
{
}


void FONT_buffer_draw(void)
{
}

extern void PANEL_new_widescreen();
extern void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb);

SLONG PANEL_fadeout_time;

#if 0

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

float angle_mul = 0.002F;
float zoom_mul  = 0.5F;

void PANEL_fadeout_draw()
{
	if (PANEL_fadeout_time)
	{
		POLY_F4 *p;
		DR_TPAGE *p2;
		SLONG fade;
		ALLOCPRIM(p,POLY_F4);
		ALLOCPRIM(p2,DR_TPAGE);

		setPolyF4(p);
		setDrawTPage(p2,0,0,getTPage(0,2,0,0));

		setXYWH(p,0,0,DisplayWidth,DisplayHeight);
		fade=((GetTickCount()-PANEL_fadeout_time)>>3);
		SATURATE(fade,0,255);
		setRGB0(p,fade,fade,fade);
		setSemiTrans(p,1);
		DOPRIM(PANEL_OTZ,p);
		DOPRIM(PANEL_OTZ,p2);
	}
}

SLONG PANEL_fadeout_finished()
{
	if (PANEL_fadeout_time)
	{
		if (GetTickCount() > PANEL_fadeout_time + 2048)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void	PANEL_draw_face(SLONG x,SLONG y,SLONG face,SLONG size)
{
	POLY_FT4 *p;
	SLONG f=face-1;

	ALLOCPRIM(p,POLY_FT4);

	setPolyFT4(p);

	setXYWH(p,x,y,20,16);
	setUVWH(p,(f&3)*16,((f>>2)*16)+160,16,16);

	p->tpage=getPSXTPageE(POLY_PAGE_FACE1);
	p->clut=getPSXClutE(POLY_PAGE_FACE1);
	setRGB0(p,128,128,128);

	DOPRIM(PANEL_OTZ,p);
}
#endif

void AENG_draw_pows(void)
{
	SLONG pow;
	SLONG sprite,psc;

	POW_Pow    *pp;
	POW_Sprite *ps;

	SLONG frame;
	SLONG	count2=0;
	SLONG	sc=0;

	//
	// Draw all used pows. Ignore the NGAMUT for now.
	//
	psc=0;

	for (pow = POW_pow_used; pow && count2++<25; pow = pp->next)
	{
		SLONG	count=0;
		ASSERT(WITHIN(pow, 1, POW_MAX_POWS - 1));
			ASSERT(pow);

		pp = &POW_pow[pow];

		for (sprite = pp->sprite; sprite; sprite = ps->next)
		{
			ASSERT(sprite);
			ps = &POW_sprite[sprite];

			frame=POLY_PAGE_EXPLODE1+(ps->frame&3)+((ps->frame&0xc)<<1);
			if(sprite&1)						 
				sc+=SPRITE_draw(ps->x>>8,ps->y>>8,ps->z>>8,128,0x7f7f7f,0xff000000,frame,1);
			psc++;				
			if(count++>192)
			{

				ASSERT(0);
				printf("sprite=%d\n",sprite);
extern	void POW_init();
				POW_init();

				return;

			}
			if(sc>40)
				return;
		}
	}
	if(count2>=25)
	{
		ASSERT(0);
		POW_init();
		return;
	}
	if (psc)
	{
		SATURATE(psc,16,63);
		PSX_SetShock(0,psc<<2);
	}
	ASSERT(sc<100);
}


void	inner_trip(
		SLONG x1,
		SLONG y1,
		SLONG z1,
		SLONG x2,
		SLONG y2,
		SLONG z2,
		SLONG width,
		ULONG colour,
		UWORD counter,
		UBYTE along)
{
	PSX_POLY_Point	*pp=perm_pp_array;

	pp[0].World.vx=x1-POLY_cam_x;
	pp[0].World.vy=y1-POLY_cam_y;
	pp[0].World.vz=z1-POLY_cam_z;
	pp[1].World.vx=x2-POLY_cam_x;
	pp[1].World.vy=y2-POLY_cam_y;
	pp[1].World.vz=z2-POLY_cam_z;
	pp[2].World.vx=x1-POLY_cam_x;
	pp[2].World.vy=y1+width-POLY_cam_y;
	pp[2].World.vz=z1-POLY_cam_z;
	pp[3].World.vx=x2-POLY_cam_x;
	pp[3].World.vy=y2+width-POLY_cam_y;
	pp[3].World.vz=z2-POLY_cam_z;

	//
	// Transform the four point. If any of them fail abandon
	// the whole line.
	//

	gte_RotTransPers4(&pp[0].World,&pp[1].World,&pp[2].World,&pp[3].World,
					  &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[3].SYSX,
					  &pp[0].P,&pp[0].Flag,&pp[0].Z);
		
	if ((MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>0)||
		(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)||
		(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>0)||
		(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
	{
		//
		// Draw two overlapping lines.
		//
		POLY_F4	*p;

		pp[0].Z=get_z_sort(pp[0].Z);

		ALLOCPRIM(p,POLY_F4);

		setSemiTrans(p,1);

		setPolyF4(p);
		setXY4(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,
				 pp[2].Word.SX,pp[2].Word.SY,pp[3].Word.SX,pp[3].Word.SY);

		setRGB0(p,100,0,0);

		DOPRIM(pp[0].Z,p);
	}

}

void SHAPE_tripwire(
		SLONG x1,
		SLONG y1,
		SLONG z1,
		SLONG x2,
		SLONG y2,
		SLONG z2,
		SLONG width,
		ULONG colour,
		UWORD counter,
		UBYTE along)
{
	SLONG	sx,sy,sz,step;
	SLONG	c0;

	sx=x2-x1;
	sy=y2-y1;
	sz=z2-z1;

	x1<<=3;
	y1<<=3;
	z1<<=3;

	for(c0=0;c0<8;c0++)
	{
		inner_trip(
			x1>>3,y1>>3,z1>>3,
			(x1+sx)>>3,
			(y1+sy)>>3,
			(z1+sz)>>3,
			width,
			colour,
			counter,
			along);

		x1+=sx;
		y1+=sy;
		z1+=sz;
	}
}


void SPRITE_draw_rotated(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort,
		SLONG rotate,
		SLONG	fade)
{
	POLY_FT4* p;
	PSX_POLY_Point *pp=perm_pp_array;
	SLONG size,sizex,sizey,opp,adj,angle,b0;
	UBYTE u,v;

//	return; //md


	pp->World.vx=x-AENG_cam_x;
	pp->World.vy=y-AENG_cam_y;
	pp->World.vz=z-AENG_cam_z;
	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);
	if(pp->Flag&(1<<31))
		return;
	if(fade)
	{
		b0=getPSXFade(pp->P);

		SATURATE(b0,0,128);
	}
	else
		fade=127;

	if (b0==0)
		return;

	if (rotate==0xffffff) {
		opp=(DisplayWidth>>1)-pp->Word.SX;
		adj=(DisplayHeight>>1)-pp->Word.SY;
		angle=-Arctan(opp,adj);
		angle&=2047;
	} else angle=rotate;


	size=(world_size<<7)/(420+(pp->Z));//(world_size<<12)/(420+(pp->Z>>2));///pp->Z;
	if ((pp->Word.SX<-size)||(pp->Word.SX>511+size)||(pp->Word.SY<-size)||(pp->Word.SY>256+size))
		return;

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	sizex=(rcos(angle<<1)*size)>>12;
	sizey=(rsin(angle<<1)*size)>>12;
	setXY4(p,pp->Word.SX-sizex,pp->Word.SY-sizey,
			 pp->Word.SX+sizey,pp->Word.SY-sizex,
			 pp->Word.SX-sizey,pp->Word.SY+sizex,
			 pp->Word.SX+sizex,pp->Word.SY+sizey);
	u=getPSXU(page);
	v=getPSXV(page);

	switch(page)
	{
	case POLY_PAGE_BLOOM1:
		setUVWH(p,u,v,63,63);
		break;
	case POLY_PAGE_FINALGLOW:
		setUVWH(p,u,v,95,95);
		break;
	default:
		setUVWH(p,u,v,31,31);
		break;
	}
	setRGB0(p,((colour>>16)*b0)>>7,(((colour>>8)&0xff)*b0)>>7,((colour&0xff)*b0)>>7);
	if ((specular&0xff000000)||(b0<128))
	{
		setSemiTrans(p,1);
	}
	p->tpage=getPSXTPageE(page);
	p->clut=getPSXClutE(page);
	if (sort)
	{
		pp->Z=get_z_sort((pp->Z)-5); //MikeD
		DOPRIM(pp->Z,p);
	} else
	{
		DOPRIM(PANEL_OTZ,p);
	}

}



SLONG SPRITE_draw(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort)
{
	POLY_FT4* p;
	PSX_POLY_Point *pp=perm_pp_array;
	SLONG size,b0,u,v;

//	return; //md

	pp->World.vx=x-AENG_cam_x;
	pp->World.vy=y-AENG_cam_y;
	pp->World.vz=z-AENG_cam_z;
	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

	if (pp->Flag&(1<<31))
		return(0);

	b0=getPSXFade(pp->P);

	SATURATE(b0,0,128);

	if (b0==0)
		return(0);

	size=(world_size<<7)/(420+(pp->Z));

	if ((pp->Word.SX<=-size)||(pp->Word.SX>DISPLAYWIDTH+size)||(pp->Word.SY<=-size)||(pp->Word.SY>256+size))
		return(0);

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);

	setXY4(p,pp->Word.SX-size,pp->Word.SY-size,
			 pp->Word.SX+size,pp->Word.SY-size,
			 pp->Word.SX-size,pp->Word.SY+size,
			 pp->Word.SX+size,pp->Word.SY+size);

	u=getPSXU(page);
	v=getPSXV(page);
	if (page==POLY_PAGE_BLOOM1)
	{
		setUVWH(p,u,v,63,63);
	}
	else
	{
		setUVWH(p,u,v,31,31);
	}

	setRGB0(p,((colour>>16)*b0)>>7,(((colour>>8)&0xff)*b0)>>7,((colour&0xff)*b0)>>7);

	p->tpage=getPSXTPageE(page);

	if (specular&0xff000000)
	{
		setSemiTrans(p,1);
		if (!(specular&0x80000000))
			p->tpage=(p->tpage&~(3<<5))|(2<<5);
	}

	p->clut=getPSXClutE(page);
	if (sort)
	{
		pp->Z=get_z_sort(pp->Z); //miked
		DOPRIM(pp->Z,p);
	} else
	{
		DOPRIM(PANEL_OTZ,p);
	}
	return(1);
}

void AENG_set_bike_wheel_rotation(UWORD rot,UBYTE prim)
{

	SLONG i;

	PrimObject *po;
	PrimFace4  *f4;

	po = &prim_objects[prim];

	//
	// The texture rotation vector.
	//

	SLONG du1 = SIN(+rot & 2047) * 15 >> 16;
	SLONG dv1 = COS(+rot & 2047) * 15 >> 16;

	SLONG du2 = SIN(-rot & 2047) * 15 >> 16;
	SLONG dv2 = COS(-rot & 2047) * 15 >> 16;

	SLONG u;
	SLONG v;

	static SLONG order[4] = {2, 1, 3, 0};

	//
	// The faces we rotate the textures on are faces 6 and 7.
	//

	f4 = &prim_faces4[po->StartFace4 + 6];

	for (i = 0; i < 4; i++)
	{
		switch(order[i])
		{
			case 0: u = 16 + du1; v = 16 + dv1; break;
			case 1: u = 16 + dv1; v = 16 - du1; break;
			case 2: u = 16 - du1; v = 16 - dv1; break;
			case 3: u = 16 - dv1; v = 16 + du1; break;
		}

		f4[0].UV[i][0] &= ~0x1f;
		f4[0].UV[i][1] &= ~0x1f;

		f4[0].UV[i][0] |= u;
		f4[0].UV[i][1] |= v;

		switch(order[i])
		{
			case 0: u = 16 + du2; v = 16 + dv2; break;
			case 1: u = 16 + dv2; v = 16 - du2; break;
			case 2: u = 16 - du2; v = 16 - dv2; break;
			case 3: u = 16 - dv2; v = 16 + du2; break;
		}

		f4[1].UV[i][0] &= ~0x1f;
		f4[1].UV[i][1] &= ~0x1f;

		f4[1].UV[i][0] |= u;
		f4[1].UV[i][1] |= v;
	} 

}

void SPRITE_draw_tex(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		UBYTE u,
		UBYTE v,
		UBYTE w,
		UBYTE h,
		SLONG sort)
{
	POLY_FT4* p;
	PSX_POLY_Point *pp=perm_pp_array;
	SLONG size;

//	return; //md

	pp->World.vx=x-AENG_cam_x;
	pp->World.vy=y-AENG_cam_y;
	pp->World.vz=z-AENG_cam_z;

	gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

	if(pp->Flag&(1<<31))
		return;

	ASSERT(pp->Z>=0);

	size=(world_size<<7)/(420+(pp->Z));
	if(size==0)
		return;
	if(size>400)
		return;

	ASSERT(size>0);
//	ASSERT(size<320);

//	ASSERT(page<22);

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);

	setXY4(p,pp->Word.SX-size,pp->Word.SY-size,
			 pp->Word.SX+size,pp->Word.SY-size,
			 pp->Word.SX-size,pp->Word.SY+size,
			 pp->Word.SX+size,pp->Word.SY+size);

	setUVWH(p,getPSXU(page)+u,getPSXV(page)+v,w,h);
	setRGB0(p,(colour>>18)&0x3f,(colour>>10)&0x3f,((colour>>2)&0x3f));

	if (specular&0xff000000)
	{
		setSemiTrans(p,1);
		p->tpage=getPSXTPageE(page);

	   	if (specular==0x7f000000)
			p->tpage=((p->tpage)&~(3<<5))|(2<<5);

	}
	else
		p->tpage=getPSXTPageE(page);

	p->clut=getPSXClutE(page);

	if (sort)
	{
		pp->Z=get_z_sort(pp->Z);
		DOPRIM(pp->Z,p);
	} else
	{
		DOPRIM(PANEL_OTZ,p);
	}
}

void AENG_draw_shadow(Thing *thing)
{
	PSX_POLY_Point *pp=perm_pp_array;
	DrawTween *dt = thing->Draw.Tweened;
	int i;
	SLONG x,y,z;//,y2,len;
//	ULONG col;
	SLONG face,y2,len;

	POLY_FT4 *p;
	ULONG	flag=0;

	if(thing->Class==CLASS_PERSON)
	{
		calc_sub_objects_position(thing, dt->AnimTween, SUB_OBJECT_PELVIS , &x, &y, &z);
	}
	else
	{
		x=0;
		z=0;
	}
	x+=(thing->WorldPos.X>>8);
	z+=(thing->WorldPos.Z>>8);

	face=find_face_for_this_pos(x,(thing->WorldPos.Y>>8)+4,z,&y2,0,0);
	if (!face)
		y2=PAP_calc_height_at_thing(thing,x,z);

	len=48-(((thing->WorldPos.Y>>8)-y2)>>1);
	if (len<=0)
		return;

	pp[0].World.vx=(x)-POLY_cam_x-32;
	pp[0].World.vy=y2-POLY_cam_y;
	pp[0].World.vz=(z)-POLY_cam_z-32;

	pp[1].World.vx=pp[0].World.vx+64;
	pp[1].World.vy=pp[0].World.vy;
	pp[1].World.vz=pp[0].World.vz;

	pp[2].World.vx=pp[0].World.vx;
	pp[2].World.vy=pp[0].World.vy;
	pp[2].World.vz=pp[0].World.vz+64;

	pp[3].World.vx=pp[0].World.vx+64;
	pp[3].World.vy=pp[0].World.vy;
	pp[3].World.vz=pp[0].World.vz+64;

	// Get darker the closer to the ground we are.

//	for(i=0;i<4;i++)
	{
		SLONG	poo;
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
		gte_RotTransPers3(&pp[0].World,&pp[1].World,&pp[2].World,&pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&poo,&flag,&z);
		flag|=pp[3].Flag;
	}

	if((flag&(1<<31))==0)
	if ((MAX4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)>=0)&&
		(MIN4(pp[0].Word.SX,pp[1].Word.SX,pp[2].Word.SX,pp[3].Word.SX)<DISPLAYWIDTH)&&
		(MAX4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)>=0)&&
		(MIN4(pp[0].Word.SY,pp[1].Word.SY,pp[2].Word.SY,pp[3].Word.SY)<256))
	{
		

		ALLOCPRIM(p,POLY_FT4);

		setPolyFT4(p);
		setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
				 pp[1].Word.SX,pp[1].Word.SY,
				 pp[2].Word.SX,pp[2].Word.SY,
				 pp[3].Word.SX,pp[3].Word.SY);
		p->tpage=getPSXTPageE(POLY_PAGE_SHADOW)&~(3<<5)|(2<<5);
		p->clut=getPSXClutE(POLY_PAGE_SHADOW);
		setUVWH(p,getPSXU(POLY_PAGE_SHADOW),getPSXV(POLY_PAGE_SHADOW),31,31);
		setRGB0(p,len,len,len);
		setSemiTrans(p,1);
		z=MIN(z,pp[3].Z);
		pp[0].Z=get_z_sort_near(z+10);
		DOPRIM(pp[0].Z,p);
	}
}


/*
void PANEL_draw_number(SLONG x, SLONG y, UBYTE digit)	// 0 <= digit <= 9... Not ASCII!
{
	POLY_F4 *p;
	SLONG b;

	p=(POLY_F4*)the_display.CurrentPrim;
	the_display.CurrentPrim+=sizeof(POLY_FT4);

	for(b=0;b<7;b++)
	{
		if (SEG_number[digit]&(1<<b))
		{
			setPolyF4(p);
			setRGB0(p,255,0,0);
			switch(b)
			{
			case 0:
				setXYWH(p,x+2,y,PANEL_SEG_L,PANEL_SEG_W);
				break;
			case 1:
				setXYWH(p,x,y+2,PANEL_SEG_W,PANEL_SEG_L);
				break;
			case 2:
				setXYWH(p,x+16,y+2,PANEL_SEG_W,PANEL_SEG_L);
				break;
			case 3:
				setXYWH(p,x+2,y+17,PANEL_SEG_L,PANEL_SEG_W);
				break;
			case 4:
				setXYWH(p,x,y+20,PANEL_SEG_W,PANEL_SEG_L);
				break;
			case 5:
				setXYWH(p,x+16,y+20,PANEL_SEG_W,PANEL_SEG_L);
				break;
			case 6:
				setXYWH(p,x+2,y+34,PANEL_SEG_L,PANEL_SEG_W);
			}
			DOPRIM(PANEL_OTZ,p);
			p++;
		}
	}
	the_display.CurrentPrim=(unsigned char *)p;
}
*/


SLONG timerx,timery,timert=0;
CBYTE timerstr[12];

#if 0
void PANEL_render_timer()
{
	POLY_FT4 *p;
	TILE *tp;
	SLONG x;

	char *c=timerstr;

	x=(DISPLAYWIDTH>>1)-((5*6)+4);
	while(*c)
	{
		int ch;
		if (*c==':')
			ch=11;
		else
			ch=*c-'0';

		ALLOCPRIM(p,POLY_FT4);
		setPolyFT4(p);
		setXYWH(p,x,timery,11,23);
		setUVWH(p,getPSXU(POLY_PAGE_LEDNUMBER)+(12*(ch&1)),getPSXV(POLY_PAGE_LEDNUMBER)+((ch<<3)&0xf0),11,15);
		setRGB0(p,128,128,128);
		p->tpage=getPSXTPageE(POLY_PAGE_LEDNUMBER);
		p->clut=getPSXClutE(POLY_PAGE_LEDNUMBER);
		DOPRIM(PANEL_OTZ,p);
		if (ch==11)
			x+=4;
		else
			x+=12;
		c++;
	}

	if (timerstr[0])
	{
		ALLOCPRIM(tp,TILE);
		setTile(tp);
		setXY0(tp,timerx-1,timery-1);
		tp->w=(12*7)+2;
		tp->h=17;
		setRGB0(tp,0,0,0);
		setSemiTrans(tp,1);
		DOPRIM(PANEL_OTZ,tp);
	}


	if (timert)
	timert--;

	if (timert==0)
		timerstr[0]=0;
}
#endif

void PANEL_draw_buffered(void)
{
}

void PANEL_draw_timer(SLONG time, SLONG x, SLONG y)
{
//	timerx=x;
//	timery=y;
	timert+=2;
	sprintf(timerstr,"%d:%02d",(time/6000),(time/100)%60);
}

void	AENG_draw_gun_point(SLONG x,SLONG y,SLONG dx1,SLONG dy1,SLONG dx2,SLONG dy2,SLONG col,SLONG z)
{
	POLY_F3 *p;
	ALLOCPRIM(p,POLY_F3);
	setPolyF3(p);
	setSemiTrans(p,1);
	setRGB0(p,(col>>16)&0xff,(col>>8)&0xff,col&0xff);
	setXY3(p,x+dx1-(dy1>>2),y+dy1+(dx1>>2),x+dx1+(dy1>>2),y+dy1-(dx1>>2),x+dx2,y+dy2);
	DOPRIM(get_z_sort(z-8),p);
}

void	AENG_draw_gun_line(SLONG x,SLONG y,SLONG dx1,SLONG dy1,SLONG dx2,SLONG dy2,SLONG col,SLONG z)
{
	LINE_F2 *p;
	ALLOCPRIM(p,LINE_F2);
	setLineF2(p);
	setSemiTrans(p,1);
	setRGB0(p,(col>>16)&0xff,(col>>8)&0xff,col&0xff);
	setXY2(p,x+dx1,y+dy1,x+dx2,y+dy2);
	DOPRIM(get_z_sort(z-8),p);
}


void PANEL_draw_gun_sight(SLONG mx,SLONG my,SLONG mz,SLONG accuracy,SLONG scale)
{
	SLONG	angle,cangle;
	SLONG	c0;
	SLONG	dx1,dy1,dx2,dy2;
	PSX_POLY_Point pp;
	SLONG	r_in,r_out;
	ULONG	col;
	SLONG	sat_acc;

#define	RADIUS_OUT	82
#define	RADIUS_IN	42

	angle=accuracy;
	pp.World.vx=mx-POLY_cam_x;
	pp.World.vy=my-POLY_cam_y;
	pp.World.vz=mz-POLY_cam_z;

	gte_RotTransPers(&pp.World,&pp.SYSX,&pp.Flag,&pp.P,&pp.Z);

	r_in = (RADIUS_IN*scale) / (420+pp.Z);
	r_out = (RADIUS_OUT*scale) / (420+pp.Z);

	sat_acc=accuracy>>1;
	SATURATE(sat_acc,0,128)

	col =  sat_acc<<8;
	col |= (128-sat_acc)<<16;

	cangle=+angle;

	dx1=((COS(cangle&2047)*r_out)>>16);
	dy1=((SIN(cangle&2047)*r_out)>>16);
	dx2=((COS(cangle&2047)*r_in)>>16);
	dy2=((SIN(cangle&2047)*r_in)>>16);

	AENG_draw_gun_point(pp.Word.SX,pp.Word.SY,dx1,dy1,dx2,dy2,col,pp.Z);
	AENG_draw_gun_point(pp.Word.SX,pp.Word.SY,-dx1,dy1,-dx2,dy2,col,pp.Z);
	AENG_draw_gun_point(pp.Word.SX,pp.Word.SY,dx1,-dy1,dx2,-dy2,col,pp.Z);
	AENG_draw_gun_point(pp.Word.SX,pp.Word.SY,-dx1,-dy1,-dx2,-dy2,col,pp.Z);

	r_in=10+(sat_acc>>3);
	r_out=(r_in+80)>>1;

	r_in = (r_in*scale) / (420+pp.Z);
	r_out = (r_out*scale) / (420+pp.Z);

	AENG_draw_gun_line(pp.Word.SX,pp.Word.SY,r_in,0,r_out,0,col,pp.Z);
	AENG_draw_gun_line(pp.Word.SX,pp.Word.SY,-r_in,0,-r_out,0,col,pp.Z);
	AENG_draw_gun_line(pp.Word.SX,pp.Word.SY,0,r_in,0,r_out,col,pp.Z);
	AENG_draw_gun_line(pp.Word.SX,pp.Word.SY,0,-r_in,0,-r_out,col,pp.Z);
}


#if 0
#define COMPASS_MID_X	(512-48)
#define COMPASS_MID_Y	(24)

void	PANEL_draw_compass_north()
{
	POLY_F3 *p;
	SLONG dx,dy;

	ALLOCPRIM(p,POLY_F3);

	dx=-rsin(AENG_cam_yaw*2);
	dy=rcos(AENG_cam_yaw*2);

	setPolyF3(p);
	setXY3(p,COMPASS_MID_X-(dy>>10),COMPASS_MID_Y+(dx>>10),COMPASS_MID_X+(dy>>10),COMPASS_MID_Y-(dx>>10),COMPASS_MID_X+(dx>>8),COMPASS_MID_Y+(dy>>8));
	setRGB0(p,255,0,0);
	DOPRIM(PANEL_OTZ,p);
}
#endif
/*
void	PANEL_draw_compass_to(SLONG x,SLONG z)
{
	POLY_F3 *p;
	SLONG dx,dy,ang;

	p=(POLY_F3*)the_display.CurrentPrim;

	ang=Arctan(AENG_cam_x-x,AENG_cam_z-z)-AENG_cam_yaw;

	dx=-rsin(ang*2);
	dy=rcos(ang*2);

	setPolyF3(p);
	setXY3(p,COMPASS_MID_X-(dy>>10),COMPASS_MID_Y+(dx>>10),COMPASS_MID_X+(dy>>10),COMPASS_MID_Y-(dx>>10),COMPASS_MID_X+(dx>>8),COMPASS_MID_Y+(dy>>8));
	setRGB0(p,255,0,0);
	DOPRIM(PANEL_OTZ,p);
	p++;
	the_display.CurrentPrim=(UBYTE*)p;
	
}
*/

void	PANEL_start(void)
{
}

void	PANEL_finish(void)
{
}

#if 0
void	AENG_draw_col_tri(SLONG x0,SLONG y0,SLONG col0,SLONG x1,SLONG y1,SLONG col1,SLONG x2,SLONG y2,SLONG col2,SLONG layer)
{
}
#endif
/*
#if 0
char vspf_str[80];
#endif

void MSG_add(CBYTE *fmt, ...)
{
#if 0
	va_list arg;
	va_start(arg,fmt);
	vsprintf(vspf_str,fmt,arg);
	printf("%s\n",vspf_str);
	va_end(arg);
#endif
}

void Debug_Text(CBYTE *fmt, ...)
{
}
*/
// New font, that I'm not convinced will work with NTSC
SBYTE f_width[]={
	5,1,3,7,5,8,6,2,
	3,3,5,5,1,5,2,4,
	5,2,5,5,5,5,5,5,
	5,5,1,1,3,4,3,4,
	7,5,5,5,5,4,4,6,
	5,1,4,5,4,7,5,7,
	5,7,5,5,5,5,5,7,
	5,5,4,2,4,2,5,7,
	2,4,4,4,4,4,4,4,
	4,1,2,4,1,7,4,4,
	4,4,3,4,3,4,5,7,
	5,5,4,3,1,3,5,5,
	0,8,0,0,0,0,0,0,
	0,0,0,0,0,8,8,8,
	8,0,1,0,0,0,0,0,
	0,0,0,0,0,8,8,0,
	8,1,0,0,0,0,0,0,
	0,8,0,0,0,0,8,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,4,
	5,5,5,5,5,5,7,5,
	4,4,4,4,2,2,2,2,
	6,5,7,7,7,7,7,5,
	7,5,5,5,5,5,4,5,
	4,4,4,5,4,4,7,4,
	4,4,4,4,2,2,2,2,
	5,5,4,4,4,5,4,5,
	5,4,4,4,4,5,4,5
};

SBYTE f_descend[]={
	0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,1,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,2,
	0,0,1,0,0,0,0,0,
	2,2,0,0,0,0,0,0,
	0,2,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,3,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,2,0,2
};


//char text_table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,!\":;'#$*-()[]\\/?^¬@_";
#ifdef VERSION_KANJI
/*
CBYTE *Kanji_message[8];
SLONG Kanji_mess_x[8];
SLONG Kanji_mess_y[8];
SLONG Kanji_mess_col[8];


SLONG Kanji_next=0;
TIM_IMAGE Kanji_Tim;
ULONG Kanji_old_sp;


SLONG Kanji_AddString(SLONG x,SLONG y,CBYTE *message,SLONG colour)
{
	SLONG Kanji_found=Kanji_next;
	SLONG i;

	printf("%s\n",message);
	for(i=0;i<8;i++)
	{
		if (Kanji_message[i]==message)
		{
			Kanji_found=i;
			break;
		}
	}
	if (Kanji_found==Kanji_next)
	{
		Kanji_message[Kanji_next]=message;
		Kanji_mess_x[Kanji_next]=x;
		Kanji_mess_y[Kanji_next]=y;
		Kanji_mess_col[Kanji_next]=colour;
		Kanji_next=(Kanji_next+1)&7;
	}
	return Kanji_found;
}

SLONG Kanji_str[8];

void Kanji_Render()
{
	SLONG i,cc;

	for(i=0;i<8;i++)
	{
		if (Kanji_message[i])
		{
			Kanji_str[i]=KanjiFntOpen(Kanji_mess_x[i],Kanji_mess_y[i],8*strlen(Kanji_message[i]),16,0,240,0,496,1,32);
			KanjiFntPrint(Kanji_str[i],Kanji_message[i]);
		}
	}
	for(i=0;i<8;i++)
		if (Kanji_message[i])
		{
			KanjiFntFlush(Kanji_str[i]);
			Kanji_message[i]=0;
		}
	KanjiFntClose();
	Kanji_next=0;
}

*/
#endif

void draw_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id)
{
#ifndef VERSION_KANJI
	SPRT_8 *p;
	DR_TPAGE *tp;
	UBYTE* m=(UBYTE*)message;
	SLONG x0=x,y0=y;
	SLONG c;

	ALLOCPRIM(tp,DR_TPAGE);
	p=(SPRT_8*)the_display.CurrentPrim;
	check_prim_ptr((void**)&p);
	setDrawTPage(tp,0,1,getPSXTPageT(0));
	while (*m)
	{
		switch(*m)
		{
		case '\t':
			x0=(x0+64)&0xfc0;
			if (x0>512)
			{
				x0=x;
				y0+=12;
			}
			break;
		case 10:
		case 13:
			x0=x;
			y0+=8;
			break;
		case 32:
			x0+=5;
			break;
		default:
			c=*m-32;

			setSprt8(p);
			setXY0(p,x0,y0+f_descend[c]);
//			p->w=f_width[c];
//			p->h=12;
			setUV0(p,(c&7)*8,(c&0xf8));
			if (font_id&&((c<96)||(c>111)))
			{
				if (font_id&0xff000000)
					setSemiTrans(p,1);
				setRGB0(p,(font_id>>16)&0xff,(font_id>>8)&0xff,font_id&0xff);
			}
			else
				setRGB0(p,128,128,128);
			p->clut=getPSXClutT(0);
			DOPRIM(PANEL_OTZ,p);
			p++;
			check_prim_ptr((void**)&p);
			x0+=f_width[c]+1;
			break;
		}
		m++;
	}
	DOPRIM(PANEL_OTZ,tp);
	the_display.CurrentPrim=(UBYTE*)p;
#else
#if 0
	POLY_FT4 *p;
	SLONG Kanji_found=Kanji_next;

	Kanji_found=Kanji_FindString(message);

	// This may seem strange, but all valid addresses are negative except those
	// on the scratch pad, and since scratch pad strings are sprintf'd in functions
	// these are unlikely to be the same next time around.

	if ((SLONG)message>0)
		Kanji_JunkString(Kanji_found);

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	// Cheat to fucking buggery here, since were SJIS double byte character string
	// and each character is 16 pixels wide we
	setXYWH(p,x,y,8*strlen(message),16);
	setUVWH(p,0,16*Kanji_found,8*strlen(message)-1,15);
	setRGB0(p,(font_id>>16)&0xff,(font_id>>8)&0xff,font_id&0xff);
	p->tpage=getTPage(0,0,Kanji_buffer_x,Kanji_buffer_y);
	p->tpage=getClut(Kanji_clut_x,Kanji_clut_y);
	DOPRIM(PANEL_OTZ,p);
#else
	Kanji_string(x,y,(UWORD*)message,font_id,256);
#endif
#endif
}

SLONG	text_width(CBYTE *message,SLONG font_id,SLONG *char_count)
{
#ifndef VERSION_KANJI
	char *p=message;
	int width=0;

	while(*p)
	{
		if (*p>31)
			width+=f_width[*p-32]+1;
		p++;
		*char_count++;
	}
	return(width);
#else
	return(strlen(message)*8);
#endif
}

SLONG	text_height(CBYTE *message,SLONG font_id,SLONG *char_count)
{
#ifndef VERSION_KANJI
	*char_count=0;//strlen(message);
	return(8);
#else
	*char_count=0;
	return(16);
#endif
}

void	draw_centre_text_at(SLONG x, SLONG y,CBYTE *message,SLONG font_id,SLONG flag)
{
	SLONG c;
	draw_text_at(x-(text_width(message,font_id,&c)/2),y-(text_height(message,font_id,&c)/2),message,font_id);
}

void CONSOLE_text(CBYTE *text, SLONG delay) 
{
	PANEL_new_text(NULL,delay,text);
}

#if 0
void CONSOLE_text_at(
		SLONG  x,
		SLONG  y,
		SLONG  delay,
		CBYTE *fmt, ...)
{
}

void	CONSOLE_clear(void)
{ 
}


void	show_text(void)
{
}

typedef struct 
{
	SLONG	x1,y1,z1;
	SLONG	x2,y2,z2;
	SLONG	width1,width2;
	SLONG   c1;
//	SLONG	c2;
	SLONG	sort;
} WorldLine;

#define MAX_WORLD_LINE	8

int world_line_upto=0;

WorldLine world_line[MAX_WORLD_LINE];


void AENG_world_line(
		SLONG x1, SLONG y1, SLONG z1, SLONG width1, ULONG colour1, 
		SLONG x2, SLONG y2, SLONG z2, SLONG width2, ULONG colour2,
		SLONG sort_to_front)
{
	WorldLine *wl;

	if (world_line_upto==MAX_WORLD_LINE)
		return;

	wl=&world_line[world_line_upto++];
	wl->x1=x1;
	wl->x2=x2;
	wl->y1=y1;
	wl->y2=y2;
	wl->z1=z1;
	wl->z2=z2;
	wl->width1=width1;
	wl->width2=width2;
	wl->c1=colour1;
//	wl->c2=colour2;
	wl->sort=sort_to_front;
}

void AENG_world_line_render()
{
	PSX_POLY_Point *pp=perm_pp_array;
	POLY_F4	*p;
	int w1,w2;
	int i,z;
	WorldLine *wl=world_line;

	for(i=0;i<world_line_upto;i++)
	{
		pp[0].World.vx=wl->x1-AENG_cam_x;
		pp[0].World.vy=wl->y1-AENG_cam_y;
		pp[0].World.vz=wl->z1-AENG_cam_z;

		pp[1].World.vx=wl->x2-AENG_cam_x;
		pp[1].World.vy=wl->y2-AENG_cam_y;
		pp[1].World.vz=wl->z2-AENG_cam_z;

		gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

		if (!(pp[0].Flag&pp[1].Flag&(1<<31)))
		{
	
			w1=(wl->width1<<7)/(420+pp[0].Z);
			w2=(wl->width2<<7)/(420+pp[1].Z);

			z=get_z_sort(max(pp[0].Z,pp[1].Z));

			ALLOCPRIM(p,POLY_F4);
			setPolyF4(p);
			setXY4(p,pp[0].Word.SX-w1,pp[0].Word.SY,
					 pp[0].Word.SX+w1,pp[0].Word.SY,
					 pp[1].Word.SX-w2,pp[1].Word.SY,
					 pp[1].Word.SX+w2,pp[1].Word.SY);
			setRGB0(p,wl->c1>>16,(wl->c1>>8)&255,(wl->c1)&255);

			if (wl->sort)
				DOPRIM(PANEL_OTZ,p)
			else
				DOPRIM(z,p)
		}
		wl++;
	}
	world_line_upto=0;
}
#endif

void AENG_set_camera(
		SLONG wx,
		SLONG wy,
		SLONG wz,
		SLONG y,		    
		SLONG p,
		SLONG r)
{
	SLONG radians_yaw   = ((2*y * PI16) / 2048);
	SLONG radians_pitch = ((2*p * PI16) / 2048);
	SLONG radians_roll  = ((2*r * PI16) / 2048);

	AENG_set_camera_radians(
		wx,
		wy,
		wz,
		radians_yaw,
		radians_pitch,
		radians_roll);
}

void AENG_set_camera_radians(
		SLONG wx,
		SLONG wy,
		SLONG wz,
		SLONG y,
		SLONG p,
		SLONG r)
{
	AENG_cam_x = SLONG(wx);
	AENG_cam_y = SLONG(wy);
	AENG_cam_z = SLONG(wz);

	AENG_cam_yaw   = y;
	AENG_cam_pitch = p;
	AENG_cam_roll  = r;

	POLY_camera_set(
		AENG_cam_x,
		AENG_cam_y,
		AENG_cam_z,
		AENG_cam_yaw,
		AENG_cam_pitch,
		AENG_cam_roll,
		SLONG(AENG_DRAW_DIST) * 256,
		AENG_LENS);

	FMATRIX_vector(AENG_cam_vec,y*2048,p*2048);
//	POLY_frame_init(FALSE);
}

/*
UWORD pers_lookup[]={
	0,38,39,15,31+64,30+64,32,33,40,48,49,32,34,35
};
*/

void AENG_create_dx_prim_points()
{
#ifdef	DONE_ON_PC_NOW
	int i,x,y;
	PrimFace4 *p=prim_faces4;
	PrimFace3 *p2=prim_faces3;
	int u,v,page,cpage;

	for(i=0;i<next_prim_face4;i++)
	{
		page=p->TexturePage;//|((p->UV[0][0]&0xc0)<<2)+(8<<6);
		if (!(p->FaceFlags & FACE_FLAG_WALKABLE))
			p->TexturePage+=(7<<6);
		cpage=page>>6;

//		if ((cpage==18))
//			p->Bright[0]=page=(8<<6)+(page&63);
//		if (cpage==19)
//			p->Bright[0]=page=(8<<6)+(pers_lookup[page&63]);

//		if (page==(8<<6)+95) page=(8<<6)+98;
		p->UV[0][0]&=0x3f;
		u=getPSXU(page);
		v=getPSXV(page);

		for(x=0;x<4;x++)
		{
		 	if (p->UV[x][0]==32) p->UV[x][0]=31;
			 	p->UV[x][0]+=u;
			if (p->UV[x][1]==32) p->UV[x][1]=31;
				p->UV[x][1]+=v;
		}
		p++;
	}

	for(i=0;i<next_prim_face3;i++)
	{
		page=p2->TexturePage;//|((p2->UV[0][0]&0xc0)<<2)+(8<<6);
		p2->TexturePage+=(7<<6);
		cpage=page>>6;

//		if (page==(8<<6)+95) page=(8<<6)+98;
//		if (cpage==18)
//			p2->Col=page=(8<<6)+(page&63);
//		if (cpage==19)
//			p2->Col=page=(8<<6)+(pers_lookup[page&63]);


		p2->UV[0][0]&=0x3f;

		u=getPSXU(page);
		v=getPSXV(page);

		for(x=0;x<3;x++)
		{
		 	if (p2->UV[x][0]==32) p2->UV[x][0]=31;
			 	p2->UV[x][0]+=u;
			if (p2->UV[x][1]==32) p2->UV[x][1]=31;
				p2->UV[x][1]+=v;
		}
		p2++;
	}
#endif
}

#if 0
void AENG_draw_scanner(
	    SLONG screen_x1,
		SLONG screen_y1,
		SLONG screen_x2,
		SLONG screen_y2,
		SLONG map_x,
		SLONG map_z,
		SLONG map_zoom,
		SLONG map_angle)
{
}

void ANEG_draw_messages()
{
}


void AENG_e_draw_3d_line(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
	LINE_F2	*p_line_f2;


}
void AENG_e_draw_3d_line_dir(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2)
{
}
void AENG_e_draw_3d_line_col(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{
}
void AENG_e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b)
{
}
void AENG_e_draw_3d_mapwho(SLONG x1, SLONG z1)
{
}
#endif

SWORD	sync_count=0;

void AENG_blit()
{
	AENG_flip();

}
void AENG_unlock()
{
}
SLONG AENG_lock()
{
}
void AENG_clear_screen()
{
}

/*
void	build_tri(SLONG x,SLONG y)
{
	POLY_F3	*p;

	if(the_display.CurrentPrim+sizeof(POLY_F3)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
		return;

	p=(POLY_F3 *)the_display.CurrentPrim;
	setPolyF3(p);
	setRGB0(p,x,y,128);
	setXY3(p,x,y,x+10,y+10,x,y+10);
	
	DOPRIM(0,p);
	the_display.CurrentPrim+=sizeof(POLY_F3);

}
*/
#define	SET_TX_TY(p,x1,y1,x2,y2,x3,y3,x4,y4)	setUV4(p,x1,y1,x2,y2,x3,y3,x4,y4)

#define	QSET_TX_TY(p,x1,y1,x2,y2,x3,y3,x4,y4,c,pal)	((ULONG*)p)[3]=((x1)<<0)|((y1)<<8)|(c<<16);((ULONG*)p)[6]=((x2)<<0)|((y2)<<8)|(pal<<16);((ULONG*)p)[9]=((x3)<<0)|((y3)<<8);((ULONG*)p)[12]=((x4)<<0)|((y4)<<8);

inline void set_floor_texture(POLY_GT4 *p,UWORD texture)
{
	SLONG tpage;
	SLONG trot;
	SLONG tflip;

	SLONG num;

	SLONG u;
	SLONG v;
	SLONG s;
	UWORD	clut,page;

//	if(texture)
	{
		UWORD	cpage;
		tpage = (texture >> 0x6) & 0xf;
		trot  = (texture >> 0xa) & 0x3;
		tflip = (texture >> 0xc) & 0x3;
		s = 31; //(texture >> 0xe) & 0x3;



//		cpage=tpage*64+u+(v<<3);
		cpage=texture&0x3ff;
 
		clut=getPSXClut(cpage);
		//cluty=256+(cpage>>2);
		//clutx=960+((cpage&3)<<4);
		//p->clut=getClut(clutx,cluty);


		u=getPSXU(cpage);
		v=getPSXV(cpage);
		

	}
/*
	else
	{
		u    = 0;
		v    = 0;
		tpage = 1;
		trot  = 0;
		tflip = 0;
		s = 31; //(texture >> 0xe) & 0x3;
		clut  = getClut(960,256);
	}
*/


	page = psx_tpages[tpage];
//	setUV4(p,u,v,u+s,v,u,v+s,u+s,v+s);

	//
	// The texture coordinates depend of the rotation.
	//


	switch(trot)
	{

		case	0:		
			QSET_TX_TY(p,u,v,u+s,v,u,v+s,u+s,v+s,clut,page);
			break;
		case	1:		
			QSET_TX_TY(p,u+s,v,u+s,v+s,u,v,u,v+s,clut,page);
			break;
		case	2:	
			QSET_TX_TY(p,u+s,v+s,u,v+s,u+s,v,u,v,clut,page);
			break;
		case	3:	
		default:
			QSET_TX_TY(p,u,v+s,u,v,u+s,v+s,u+s,v,clut,page);
			break;

	}

}

#ifdef INSIDES_EXIST
void set_inside_texture(POLY_FT4 *p,UWORD texture)
{
	SLONG tpage;
	SLONG trot;
	SLONG tflip;

	SLONG num;

	SLONG u;
	SLONG v;
	SLONG s;

	if(texture)
	{
		UWORD	cpage;
		SLONG	clutx,cluty;
		u    = (texture >> 0x0) & 0x7;
		v    = (texture >> 0x3) & 0x7;
		tpage = (texture >> 0x6) & 0xf;
		trot  = (texture >> 0xa) & 0x3;
		tflip = (texture >> 0xc) & 0x3;
		s = 31; //(texture >> 0xe) & 0x3;



//		cpage=tpage*64+u+(v<<3);
		cpage=texture&0x3ff;

		cluty=256+(cpage>>2);
		clutx=960+((cpage&3)<<4);
		p->clut=getClut(clutx,cluty);


		u=u<<5;
		v=v<<5;
		

	}
	else
	{
		u    = 0;
		v    = 0;
		tpage = 1;
		trot  = 0;
		tflip = 0;
		s = 31; //(texture >> 0xe) & 0x3;
		p->clut  = getClut(960,256);
	}


	p->tpage = psx_tpages[tpage];
//	setUV4(p,u,v,u+s,v,u,v+s,u+s,v+s);

	//
	// The texture coordinates depend of the rotation.
	//


	switch(trot)
	{

		case	0:		
			SET_TX_TY(p,u,v,u+s,v,u,v+s,u+s,v+s);
			break;
		case	1:		
			SET_TX_TY(p,u+s,v,u+s,v+s,u,v,u,v+s);
			break;
		case	2:	
			SET_TX_TY(p,u+s,v+s,u,v+s,u+s,v,u,v);
			break;
		case	3:	
		default:
			SET_TX_TY(p,u,v+s,u,v,u+s,v+s,u+s,v);
			break;

	}

}
#endif

#define	SET_TXTY(p,x1,y1,x2,y2,x3,y3,x4,y4)	tx[0]=x1;ty[0]=y1;tx[1]=x2;ty[1]=y2;tx[2]=x3;ty[2]=y3;tx[3]=x4;ty[3]=y4;
/*
void set_floor_texture_special_tri(POLY_GT3 *p,UWORD texture,UWORD a,UWORD b,UWORD c)
{
	SLONG tpage;
	SLONG trot;
	SLONG tflip;

	SLONG num;

	SLONG u;
	SLONG v;
	SLONG s;
	SLONG	tx[4],ty[4];


	if(texture)
	{
		u    = (texture >> 0x0) & 0x7;
		v    = (texture >> 0x3) & 0x7;
		tpage = (texture >> 0x6) & 0xf;
		trot  = (texture >> 0xa) & 0x3;
		tflip = (texture >> 0xc) & 0x3;
		s = 31; //(texture >> 0xe) & 0x3;
		u=u<<5;
		v=v<<5;
		
		

	}
	else
	{
		u    = 0;
		v    = 0;
		tpage = 1;
		trot  = 0;
		tflip = 0;
		s = 31; //(texture >> 0xe) & 0x3;
	}


	p->tpage = psx_tpages[tpage];
//	p->clut  = psx_tpages_clut[tpage];
//	setUV4(p,u,v,u+s,v,u,v+s,u+s,v+s);

	//
	// The texture coordinates depend of the rotation.
	//

	switch(trot)
	{

		case	0:		
			SET_TXTY(p,u,v,u+s,v,u,v+s,u+s,v+s);
			break;
		case	1:		
			SET_TXTY(p,u+s,v,u+s,v+s,u,v,u,v+s);
			break;
		case	2:	
			SET_TXTY(p,u+s,v+s,u,v+s,u+s,v,u,v);
			break;
		case	3:	
		default:
			SET_TXTY(p,u,v+s,u,v,u+s,v+s,u+s,v);
			break;

	}
	setUV3(p,tx[a],ty[a],tx[b],ty[b],tx[c],ty[c]);
}
*/
SLONG	calc_light(POLY_Point *pp)
{
	SLONG	s;
	SLONG	x,y,z;

//	z=abs(pp->Z)<<8;
//	z=((AENG_DRAW_DIST<<16)-z)/AENG_DRAW_DIST;
//	s=z>>8;
	s=getPSXFade(pp->Z);
	SATURATE(s,0,255);
	return(s);


/*
	x=pp->x>>3;
	y=pp->y>>3;
	z=pp->z>>3;
	x=(x+2048)&2047;
	y=(y+2048)&2047;
	z=(z+2048)&2047;

	s=(COS(x)>>10)+(COS(y)>>10)+(SIN(x)>>9)+128;
	if(s<0)
		s=0;
	else
		if(s>255)
			s=255;

	return(s);
*/
}

void	do_4_lights(POLY_GT4 *p,POLY_Point *quad)
{
	SLONG	s0,s1,s2,s3;

	s0=calc_light(&quad[0]);
	setRGB0(p,s0,s0,s0);

	s1=calc_light(&quad[1]);
	setRGB1(p,s1,s1,s1);

	s2=calc_light(&quad[2]);
	setRGB2(p,s2,s2,s2);

	s3=calc_light(&quad[3]);
	setRGB3(p,s3,s3,s3);

//	if ((s0!=128)||(s1!=128)||(s2!=128)||(s3|=128))
//		setSemiTrans(p,1);
}
/*
void	do_4_lights_shadow(POLY_GT4 *p,POLY_Point *quad)
{
	SLONG	s;
	s=calc_light(&quad[0])>>1;
	setRGB0(p,s,s,s);

	s=calc_light(&quad[1])>>1;
	setRGB1(p,s,s,s);

	s=calc_light(&quad[2])>>1;
	setRGB2(p,s,s,s);

	s=calc_light(&quad[3])>>1;
	setRGB3(p,s,s,s);
}

void	do_3_lights(POLY_GT3 *p,POLY_Point *quad)
{
	SLONG	s;
	s=calc_light(&quad[0]);
	setRGB0(p,s,s,s);

	s=calc_light(&quad[1]);
	setRGB1(p,s,s,s);

	s=calc_light(&quad[2]);
	setRGB2(p,s,s,s);

}

void	do_3_lights_shadow(POLY_GT3 *p,POLY_Point *quad)
{
	SLONG	s;
	s=calc_light(&quad[0])>>1;
	setRGB0(p,s,s,s);

	s=calc_light(&quad[1])>>1;
	setRGB1(p,s,s,s);

	s=calc_light(&quad[2])>>1;
	setRGB2(p,s,s,s);

}
*/

void	add_quad(POLY_Point *quad,PAP_Hi *ph)
{
	SLONG	z;
	SLONG	shadow;
#if 0
	if(the_display.CurrentPrim>&(the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM-100]))
	{
		printf(" buckets full \n");
		return;
	}
#endif

	//
	// find furthest z
	//
	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	z=get_z_sort(z);
//	ASSERT(z>=0&&z<=4095);

	//
	// no shadow for now
	//
	{
		POLY_GT4	*p;
		SLONG	s;

		ALLOCPRIM(p,POLY_GT4);

		set_floor_texture(p,ph->Texture);
		setPolyGT4(p);
		do_4_lights(p,quad);

		setXY4(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y,quad[3].X,quad[3].Y);
		DOPRIM(z,p);

	}
/*
	shadow=ph->Flags&(FLOOR_SHADOW_1|FLOOR_SHADOW_2);
	switch(shadow)
	{
		case	0:
			{
				POLY_GT4	*p;
				SLONG	s;

				p=(POLY_GT4 *)the_display.CurrentPrim;

				set_floor_texture(p,me->Texture);
				setPolyGT4(p);
				do_4_lights(p,quad);



				setXY4(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y,quad[3].X,quad[3].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT4);
				addPrim(&(the_display.CurrentDisplayBuffer->ot[z]),p);

			}

		case	(FLOOR_SHADOW_1|FLOOR_SHADOW_2): // all shadow
			{

				POLY_GT4	*p;
				p=(POLY_GT4 *)the_display.CurrentPrim;

				set_floor_texture(p,me->Texture);
				setPolyGT4(p);
				do_4_lights_shadow(p,quad);
//				setRGB0(p,128-64,128-64,128-64);
				setXY4(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y,quad[3].X,quad[3].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT4);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);


			}
			break;
		case	FLOOR_SHADOW_2:
			//   .
			//  ..
			// ...
			{

				POLY_GT3	*p;
				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,1,2);
				setPolyGT3(p);
				do_3_lights(p,quad);
				setXY3(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,1,3,2);
				setPolyGT3(p);
				do_3_lights_shadow(p,quad); //error
				setXY3(p,quad[1].X,quad[1].Y,quad[3].X,quad[3].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
			}
		case	FLOOR_SHADOW_1:
			// ... 
			// ..
			// .
			{

				POLY_GT3	*p;
				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,1,2);
				setPolyGT3(p);
				do_3_lights_shadow(p,quad);
				setXY3(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,1,3,2);
				setPolyGT3(p);
				do_3_lights(p,quad); //error
				setXY3(p,quad[1].X,quad[1].Y,quad[3].X,quad[3].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
			}

		case	93:
			// . 
			// ..
			// ...
			{

				POLY_GT3	*p;
				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,3,2);
				setPolyGT3(p);
				do_3_lights_shadow(p,quad);
				setXY3(p,quad[0].X,quad[0].Y,quad[3].X,quad[3].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,1,3);
				setPolyGT3(p);
				do_3_lights(p,quad); //error
				setXY3(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[3].X,quad[3].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
			}
		case	94:
			// ... 
			// ..
			// .
			{

				POLY_GT3	*p;
				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,1,2);
				setPolyGT3(p);
				do_3_lights_shadow(p,quad);
				setXY3(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,1,3,2);
				setPolyGT3(p);
				do_3_lights(p,quad); //error
				setXY3(p,quad[1].X,quad[1].Y,quad[3].X,quad[3].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
			}
		case	95:
			// ... 
			//  ..
			//   .
			{

				POLY_GT3	*p;
				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,3,2);
				setPolyGT3(p);
				do_3_lights(p,quad);
				setXY3(p,quad[0].X,quad[0].Y,quad[3].X,quad[3].Y,quad[2].X,quad[2].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);

				p=(POLY_GT3 *)the_display.CurrentPrim;
				set_floor_texture_special_tri(p,me->Texture,0,1,3);
				setPolyGT3(p);
				do_3_lights_shadow(p,quad); //error
				setXY3(p,quad[0].X,quad[0].Y,quad[1].X,quad[1].Y,quad[3].X,quad[3].Y);
				the_display.CurrentPrim+=sizeof(POLY_GT3);
				addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
			}
			break;
	}
	*/
}
/*
inline void	add_inside_quad(PSX_POLY_Point *quad,UWORD text,UBYTE **cp,UWORD col0,UBYTE fade)
{
	SLONG	z;
	SLONG	shadow;
	ULONG	padd;
	SLONG	two_pass=0;

	//
	// find furthest z
	//
	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	z=get_z_sort(z);
	ASSERT(z>=0&&z<=4095);

	//
	// no shadow for now
	//
	{
		SLONG	b0;
		POLY_FT4	*p;
//		SLONG	s;

		p=(POLY_FT4 *)*cp; //the_display.CurrentPrim;

		set_inside_texture(p,text);
		setPolyFT4(p);
//		do_4_lights(p,quad);

		if (fade)
		{
			b0=128-(fade>>1);
			setSemiTrans(p,1);
		}
		else
			b0=128-((quad[0].P)>>5);//128+((4096-quad[0].P)>>5);

//
// forget the fadeout
//

		if(b0<128)
			setRGB0(p,(((col0>>10)&0x3f)*b0)>>5,(((col0>>5)&0x1f)*b0)>>4,(((col0)&0x1f)*b0)>>4);
		else
			setRGB0(p,(((col0>>10)&0x3f)<<2),(((col0>>5)&0x1f)<<3),(((col0)&0x1f)<<3));

		setXY4(p,quad[0].Word.SX,quad[0].Word.SY,
				quad[1].Word.SX,quad[1].Word.SY,
				quad[2].Word.SX,quad[2].Word.SY,
				quad[3].Word.SX,quad[3].Word.SY);

		*cp+=sizeof(POLY_FT4);
		//the_display.CurrentPrim+=sizeof(POLY_GT4);

		DOPRIM(z,p);
	}
}
*/
//UBYTE shadow_map[8][4]={{0,0,0,0},{2,1,1,0},{2,1,2,1},{1,1,2,1},{1,1,2,2},{2,1,2,2},{2,1,2,1},{0,1,1,2}};
//UBYTE shadow_map[8][4]={{0,0,0,0},{3,0,3,0},{3,0,3,0},{0,0,3,0},{0,0,3,3},{3,0,3,3},{3,0,3,0},{0,0,3,3}};
UBYTE shadow_map[8][4]={{0,0,0,0},{0,0,0,0},{3,0,3,0},{0,0,3,0},{0,0,3,3},{3,0,3,3},{3,0,3,0},{0,0,0,0}};

#define	SHADOW_SUB	64
//UBYTE shadow_map2[8][4]={{0,0,0,0},{SHADOW_SUB,0,SHADOW_SUB,0},{SHADOW_SUB,0,SHADOW_SUB,0},{0,0,SHADOW_SUB,0},{0,0,SHADOW_SUB,SHADOW_SUB},{SHADOW_SUB,0,SHADOW_SUB,SHADOW_SUB},{SHADOW_SUB,0,SHADOW_SUB,0},{0,0,SHADOW_SUB,SHADOW_SUB}};

//  0    n1     1
//
//	n4	 n0		n3
//
//	2    n2     3


void	split_quad_all(PSX_POLY_Point *quad,POLY_GT4	*p,SLONG z)
{

	POLY_GT4	*pa;
	PSX_POLY_Point	pp[5];
	SLONG	r1,g1,b1,r2,g2,b2;
	SLONG	u0,v0,u1,v1;

	pp[0].World.vx=quad[0].World.vx+128;
	pp[0].World.vz=quad[0].World.vz+128;
	pp[0].World.vy=(quad[0].World.vy+quad[1].World.vy+quad[2].World.vy+quad[3].World.vy)>>2;

	pp[1].World.vx=quad[0].World.vx+128;
	pp[1].World.vz=quad[0].World.vz;
	pp[1].World.vy=(quad[0].World.vy+quad[1].World.vy)>>1;

	pp[2].World.vx=quad[2].World.vx+128;
	pp[2].World.vz=quad[2].World.vz;
	pp[2].World.vy=(quad[2].World.vy+quad[3].World.vy)>>1;

	pp[3].World.vx=quad[1].World.vx;
	pp[3].World.vz=quad[1].World.vz+128;
	pp[3].World.vy=(quad[1].World.vy+quad[3].World.vy)>>1;

	pp[4].World.vx=quad[0].World.vx;
	pp[4].World.vz=quad[0].World.vz+128;
	pp[4].World.vy=(quad[0].World.vy+quad[2].World.vy)>>1;

	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);

	if(pp[0].Flag&(1<<31))
	{
		//
		// If you can't transform the middle point then your fucked
		//
		return;
	}

	if(((quad[0].Flag|quad[1].Flag)&(1<<31))==0)
	{
		gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
	}
	else
		pp[1].Flag=1<<31;

	if(((quad[0].Flag|quad[2].Flag)&(1<<31))==0)
	{
		gte_RotTransPers(&pp[4].World,&pp[4].SYSX,&pp[4].P,&pp[4].Flag,&pp[4].Z);
	}
	else
		pp[4].Flag=1<<31;

	if(((quad[2].Flag|quad[3].Flag)&(1<<31))==0)
	{
		gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
	}
	else
		pp[2].Flag=1<<31;

	if(((quad[1].Flag|quad[3].Flag)&(1<<31))==0)
	{
		gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
	}
	else
		pp[3].Flag=1<<31;


	r1=(p->r0+p->r1+p->r2+p->r3)>>2;
	g1=(p->g0+p->g1+p->g2+p->g3)>>2;
	b1=(p->b0+p->b1+p->b2+p->b3)>>2;

	u1=(p->u0+p->u1+p->u2+p->u3)>>2;
	v1=(p->v0+p->v1+p->v2+p->v3)>>2;
/*
								if(0)
								{
									LINE_F2	*line;

									ALLOCPRIM(line,LINE_F2);
									setLineF2(line);
									setXY2(line,80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2),80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2));
									setRGB0(line,255,255,255);
									DOPRIM(PANEL_OTZ,line);
								
									ALLOCPRIM(line,LINE_F2);
									setLineF2(line);
									setXY2(line,80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2),80+(pp[4].Word.SX>>2),60+(pp[4].Word.SY>>2));
										setRGB0(line,128,128,128);
									DOPRIM(PANEL_OTZ,line);

									ALLOCPRIM(line,LINE_F2);
									setLineF2(line);
									setXY2(line,80+(pp[4].Word.SX>>2),60+(pp[4].Word.SY>>2),80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2));
									setRGB0(line,128,128,128);
									DOPRIM(PANEL_OTZ,line);

									ALLOCPRIM(line,LINE_F2);
									setLineF2(line);
									setXY2(line,80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2),80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2));
									setRGB0(line,128,128,128);
									DOPRIM(PANEL_OTZ,line);

									ALLOCPRIM(line,LINE_F2);
									setLineF2(line);
									setXY2(line,80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2),80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2));
									setRGB0(line,128,128,128);
									DOPRIM(PANEL_OTZ,line);


								}
*/


/*
	pp[1].Word.SY-=1;
	pp[2].Word.SY-=1;
	pp[3].Word.SY-=1;
	pp[4].Word.SY-=1;
*/


//  0    n1     1
//	   X
//	n4	 n0		n3
//
//	2    n2     3
	if(((quad[0].Flag|pp[0].Flag|pp[1].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,quad[0].Word.SX,quad[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,pp[4].Word.SX,pp[4].Word.SY,pp[0].Word.SX,pp[0].Word.SY);

		setRGB0(pa,p->r0,p->g0,p->b0);
		setRGB1(pa,(p->r0+p->r1)>>1,(p->g0+p->g1)>>1,(p->b0+p->b1)>>1);
		setRGB2(pa,(p->r0+p->r2)>>1,(p->g0+p->g2)>>1,(p->b0+p->b2)>>1);
		setRGB3(pa,r1,g1,b1);

		QSET_TX_TY(pa,p->u0,p->v0,
				(p->u0+p->u1)>>1,(p->v0+p->v1)>>1,
				(p->u0+p->u2)>>1,(p->v0+p->v2)>>1,
				u1,v1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);



	}

//  0    n1     1
//			 X
//	n4	 n0		n3
//
//	2    n2     3

	if(((quad[0].Flag|pp[0].Flag|pp[1].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,pp[1].Word.SX,pp[1].Word.SY,quad[1].Word.SX,quad[1].Word.SY,pp[0].Word.SX,pp[0].Word.SY,pp[3].Word.SX,pp[3].Word.SY);

		setRGB0(pa,(p->r0+p->r1)>>1,(p->g0+p->g1)>>1,(p->b0+p->b1)>>1);
		setRGB1(pa,p->r1,p->g1,p->b1);
		setRGB2(pa,r1,g1,b1);
		setRGB3(pa,(p->r3+p->r1)>>1,(p->g3+p->g1)>>1,(p->b3+p->b1)>>1);

		QSET_TX_TY(pa,(p->u0+p->u1)>>1,(p->v0+p->v1)>>1,
				p->u1,p->v1,
				u1,v1,
				(p->u1+p->u3)>>1,(p->v1+p->v3)>>1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}


//  0    n1     1
//
//	n4	 n0		n3
//	  X
//	2    n2     3

	if(((quad[2].Flag|pp[0].Flag|pp[2].Flag|pp[4].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);



		setXY4(pa,pp[4].Word.SX,pp[4].Word.SY,pp[0].Word.SX,pp[0].Word.SY,quad[2].Word.SX,quad[2].Word.SY,pp[2].Word.SX,pp[2].Word.SY);

		setRGB0(pa,(p->r0+p->r2)>>1,(p->g0+p->g2)>>1,(p->b0+p->b2)>>1);
		setRGB1(pa,r1,g1,b1);
		setRGB2(pa,p->r2,p->g2,p->b2);
		setRGB3(pa,(p->r2+p->r3)>>1,(p->g2+p->g3)>>1,(p->b2+p->b3)>>1);

		QSET_TX_TY(pa,
				(p->u0+p->u2)>>1,(p->v0+p->v2)>>1,
				u1,v1,
				p->u2,p->v2,
				(p->u3+p->u2)>>1,(p->v3+p->v2)>>1,p->clut,p->tpage);


//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}
//  0    n1     1
//
//	n4	 n0		n3
//			 X
//	2    n2     3

	if(((quad[3].Flag|pp[0].Flag|pp[2].Flag|pp[3].Flag)&(1<<31))==0)
	{
		ALLOCPRIM(pa,POLY_GT4);

		setPolyGT4(pa);

		setXY4(pa,pp[0].Word.SX,pp[0].Word.SY,pp[3].Word.SX,pp[3].Word.SY,pp[2].Word.SX,pp[2].Word.SY,quad[3].Word.SX,quad[3].Word.SY);

		setRGB0(pa,r1,g1,b1);
		setRGB1(pa,(p->r3+p->r1)>>1,(p->g3+p->g1)>>1,(p->b3+p->b1)>>1);
		setRGB2(pa,(p->r3+p->r2)>>1,(p->g3+p->g2)>>1,(p->b3+p->b2)>>1);
		setRGB3(pa,p->r3,p->g3,p->b3);
//		setRGB3(pa,255,0,0);

		QSET_TX_TY(pa,u1,v1,
				(p->u3+p->u1)>>1,(p->v3+p->v1)>>1,
				(p->u3+p->u2)>>1,(p->v3+p->v2)>>1,
				p->u3,p->v3,p->clut,p->tpage);

//		pa->tpage=p->tpage;
//		pa->clut=p->clut;
		DOPRIM(z,pa);

	}



}



//UBYTE shadow_map[8][4]={{0,0,0,0},{128,192,192,0},{128,192,128,192},{192,192,192,192},
//						{192,192,192,192},{192,192,192,192},{192,192,192,192},{0,192,192,192}};

//  quad  0     1      p= 0     1	    split     0  a  1
//		  
//		  2	    3	 	  2	    3				  2  b  3
#ifdef	OLD_SPLIT
void	split_quad_a(PSX_POLY_Point *quad,POLY_GT4	*p,SLONG z)
{

	POLY_GT4	*pa;
	SLONG	r1,g1,b1,r2,g2,b2;
	SLONG	u0,v0,u1,v1;

	//
	// rhs (is a whole new GT4
	//
	ALLOCPRIM(pa,POLY_GT4);

	setPolyGT4(pa);


	quad[0].World.vx+=128;
	quad[0].World.vy=(quad[0].World.vy+quad[1].World.vy)>>1;
	gte_RotTransPers(&quad[0].World,&quad[0].SYSX,&quad[0].P,&quad[0].Flag,&quad[0].Z);

	quad[2].World.vx+=128;
	quad[2].World.vy=(quad[2].World.vy+quad[3].World.vy)>>1;
	gte_RotTransPers(&quad[2].World,&quad[2].SYSX,&quad[2].P,&quad[2].Flag,&quad[2].Z);

	r1=(p->r0+p->r1)>>1;
	g1=(p->g0+p->g1)>>1;
	b1=(p->b0+p->b1)>>1;

	r2=(p->r2+p->r3)>>1;
	g2=(p->g2+p->g3)>>1;
	b2=(p->b2+p->b3)>>1;

	setXY4(pa,quad[0].Word.SX,quad[0].Word.SY,
			p->x1,p->y1,
			quad[2].Word.SX,quad[2].Word.SY,
			p->x3,p->y3);

	setRGB0(pa,r1,g1,b1);
	setRGB1(pa,p->r1,p->g1,p->b1);
	setRGB2(pa,r2,g2,b2);
	setRGB3(pa,p->r3,p->g3,p->b3);

	u0=(p->u0+p->u1)>>1;
	v0=(p->v0+p->v1)>>1;

	u1=(p->u2+p->u3)>>1;
	v1=(p->v2+p->v3)>>1;

	setUV4(pa,u0,v0,p->u1,p->v1,u1,v1,p->u3,p->v3);

	pa->tpage=p->tpage;
	pa->clut=p->clut;
	DOPRIM(z,pa);

	//
	// LHS   writes over gt4 being used
	//

	p->x1=quad[0].Word.SX;
	p->y1=quad[0].Word.SY;

	p->x3=quad[2].Word.SX;
	p->y3=quad[2].Word.SY;

	p->u1=u0;
	p->v1=v0;

	p->u3=u1;
	p->v3=v1;

	setRGB1(p,r1,g1,b1);
	setRGB3(p,r2,g2,b2);
	DOPRIM(z,p);

}


//   0	  1
//	 a	  b
//	 2	  3
void	split_quad_b(PSX_POLY_Point *quad,POLY_GT4	*p,SLONG z)
{

	POLY_GT4	*pa;
	SLONG	r1,g1,b1,r2,g2,b2;
	SLONG	u0,v0,u1,v1;

	//
	// rhs (is a whole new GT4
	//
	ALLOCPRIM(pa,POLY_GT4);

	setPolyGT4(pa);


	quad[0].World.vz+=128;
	quad[0].World.vy=(quad[0].World.vy+quad[2].World.vy)>>1;
	gte_RotTransPers(&quad[0].World,&quad[0].SYSX,&quad[0].P,&quad[0].Flag,&quad[0].Z);

	quad[1].World.vz+=128;
	quad[1].World.vy=(quad[1].World.vy+quad[3].World.vy)>>1;
	gte_RotTransPers(&quad[1].World,&quad[1].SYSX,&quad[1].P,&quad[1].Flag,&quad[1].Z);

	r1=(p->r0+p->r2)>>1;
	g1=(p->g0+p->g2)>>1;
	b1=(p->b0+p->b2)>>1;

	r2=(p->r1+p->r3)>>1;
	g2=(p->g1+p->g3)>>1;
	b2=(p->b1+p->b3)>>1;

	setXY4(pa,quad[0].Word.SX,quad[0].Word.SY,
			quad[1].Word.SX,quad[1].Word.SY,
			p->x2,p->y2,
			p->x3,p->y3);

	setRGB0(pa,r1,g1,b1);
	setRGB1(pa,r2,g2,b2);
	setRGB2(pa,p->r2,p->g2,p->b2);
	setRGB3(pa,p->r3,p->g3,p->b3);

	u0=(p->u0+p->u2)>>1;
	v0=(p->v0+p->v2)>>1;

	u1=(p->u1+p->u3)>>1;
	v1=(p->v1+p->v3)>>1;

	setUV4(pa,u0,v0,u1,v1,p->u2,p->v2,p->u3,p->v3);

	pa->tpage=p->tpage;
	pa->clut=p->clut;
	DOPRIM(z,pa);

	//
	// LHS   writes over gt4 being used
	//

	p->x2=quad[0].Word.SX;
	p->y2=quad[0].Word.SY;

	p->x3=quad[1].Word.SX;
	p->y3=quad[1].Word.SY;

	p->u2=u0;
	p->v2=v0;

	p->u3=u1;
	p->v3=v1;

	setRGB2(p,r1,g1,b1);
	setRGB3(p,r2,g2,b2);
	DOPRIM(z,p);

}
#endif
//inline void	add_floor_quad(PSX_POLY_Point *quad,PAP_Hi *ph,UWORD col0,UWORD col1,UWORD col2,UWORD col3,SLONG shadow,ULONG lum,SLONG split_all,POLY_GT4 *p,ULONG day_flag)
#ifdef	OLD_POO
inline	void	add_floor_quad(PSX_POLY_Point *quad,PAP_Hi *ph,UWORD col0,UWORD col1,UWORD col2,UWORD col3,SLONG shadow,ULONG lum,SLONG split_all,POLY_GT4 *p,ULONG day_flag)
{
	SLONG	z;
	SLONG	two_pass=0;
	UBYTE	*s_map;
	UWORD	texture;

	s_map=&shadow_map[shadow&0xff][0];

/*
	if(PadKeyIsPressed(&PAD_Input1,PAD_FLT))
		two_pass=1;

	if(the_display.CurrentPrim>&(the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM-100]))
	{
		return;
	}
*/
	//
	// find furthest z
	//
	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	z=get_z_sort(z);

	//
	// no shadow for now
	//
	{
		SLONG	b0,b1,b2,b3;
//		POLY_GT4	*p;
		SLONG	s;

//		ALLOCPRIM(p,POLY_GT4);

		texture=ph->Texture;
		set_floor_texture(p,texture);
		setPolyGT4(p);
//		do_4_lights(p,quad);
		//
		// was doing 4 duplicate reads of NIGHT_flag    saves 12 ticks ish
		//
		if(day_flag)
		{
			b0=getPSXFade_day(quad[0].P);
			b1=getPSXFade_day(quad[1].P);
			b2=getPSXFade_day(quad[2].P);
			b3=getPSXFade_day(quad[3].P);
		}
		else
		{
			b0=getPSXFade_night(quad[0].P);
			b1=getPSXFade_night(quad[1].P);
			b2=getPSXFade_night(quad[2].P);
			b3=getPSXFade_night(quad[3].P);
		}

		if ((b0==0)||(b0==255))
			return;

extern void AENG_add_semi_fade(PSX_POLY_Point *pp,UWORD b0,UWORD b1,UWORD b2,UWORD b3,SLONG z);

//		if((b0!=128)||(b1!=128)||(b2!=128)||(b3!=128))
		if(1)
		{
			UBYTE	shift[4];

			if(shadow)
			{
				shift[0]=s_map[0];
				shift[1]=s_map[1];
				shift[2]=s_map[2];
				shift[3]=s_map[3];
			}
			else
			{
				shift[0]=0;
				shift[1]=0;
				shift[2]=0;
				shift[3]=0;

			}
//			if ((b0!=128)&&(b1!=128)&&(b2!=128)&&(b3!=128))
//				setSemiTrans(p,1);
//			else
//				AENG_add_semi_fade(quad,128-b0,128-b1,128-b2,128-b3,z);

			//
			// in fadeout region
			//

			b0>>=1;
			b1>>=1;
			b2>>=1;
			b3>>=1;

			setRGB0(p,MAKELUMI((((col0>>10)&0x3f)*b0)>>(4+shift[0]),lum&0xff),
				      MAKELUMI((((col0>>5)&0x1f)*b0)>>(3+shift[0]),lum&0xff),
					  MAKELUMI((((col0)&0x1f)*b0)>>(3+shift[0]),(lum>>1)&0x7f));

			setRGB1(p,MAKELUMI((((col1>>10)&0x3f)*b1)>>(4+shift[1]),(lum>>8)&0xff),
				      MAKELUMI((((col1>>5)&0x1f)*b1)>>(3+shift[1]),(lum>>8)&0xff),
					  MAKELUMI((((col1)&0x1f)*b1)>>(3+shift[1]),(lum>>9)&0x7f));

			setRGB2(p,MAKELUMI((((col2>>10)&0x3f)*b2)>>(4+shift[2]),(lum>>16)&0xff),
				      MAKELUMI((((col2>>5)&0x1f)*b2)>>(3+shift[2]),(lum>>16)&0xff),
					  MAKELUMI((((col2)&0x1f)*b2)>>(3+shift[2]),(lum>>17)&0x7f));

			setRGB3(p,MAKELUMI((((col3>>10)&0x3f)*b3)>>(4+shift[3]),lum>>24),
				      MAKELUMI((((col3>>5)&0x1f)*b3)>>(3+shift[3]),lum>>24),
					  MAKELUMI(((col3&0x1f)*b3)>>(3+shift[3]),lum>>25));
		}
		else
/*
		{
			UBYTE	shift[4];
			//
			// not in fadeout
			//
			if(shadow)
			{
				//
				//could do this with one long read
				//
				shift[0]=s_map[0];
				shift[1]=s_map[1];
				shift[2]=s_map[2];
				shift[3]=s_map[3];
			}
			else
			{
				//
				// mostly no shadow, so save ourselves 4 memory reads (maybe 12 if optimiser is rubish)
				//
				shift[0]=0;
				shift[1]=0;
				shift[2]=0;
				shift[3]=0;

			}

			setRGB0(p,MAKELUMI((((col0>>10)&0x3f)<<(2-shift[0])),lum&0xff),
					  MAKELUMI((((col0>>5)&0x1f)<<(3-shift[0])),lum&0xff),
					  MAKELUMI((((col0)&0x1f)<<(3-shift[0])),(lum>>1)&0x7f));

			setRGB1(p,MAKELUMI((((col1>>10)&0x3f)<<(2-shift[1])),(lum>>8)&0xff),
					  MAKELUMI((((col1>>5)&0x1f)<<(3-shift[1])),(lum>>8)&0xff),
					  MAKELUMI((col1&0x1f)<<(3-shift[1]),(lum>>9)&0x7f));

			setRGB2(p,MAKELUMI((((col2>>10)&0x3f)<<(2-shift[2])),(lum>>16)&0xff),
					  MAKELUMI((((col2>>5)&0x1f)<<(3-shift[2])),(lum>>16)&0xff),
					  MAKELUMI((((col2)&0x1f)<<(3-shift[2])),(lum>>17)&0x7f));

			setRGB3(p,MAKELUMI((((col3>>10)&0x3f)<<(2-shift[3])),lum>>24),
					  MAKELUMI((((col3>>5)&0x1f)<<(3-shift[3])),lum>>24),
					  MAKELUMI((((col3)&0x1f)<<(3-shift[3])),lum>>25));
		}
*/

//		setXY4(p,quad[0].Word.SX,quad[0].Word.SY,
//				quad[1].Word.SX,quad[1].Word.SY,
//				quad[2].Word.SX,quad[2].Word.SY,
//				quad[3].Word.SX,quad[3].Word.SY);

		//
		// saves 4 memory writes  == 16 ticks
		//

//		if(z<=FLOOR_SPLIT_DIST)
		{
/*
			quad[0].SYSX+=30<<16;
			quad[1].SYSX+=30<<16;
			quad[2].SYSX+=30<<16;
			quad[3].SYSX+=30<<16;
*/

		}
		if(z<=FLOOR_SPLIT_DIST)
		{
			quad[1].SYSX+=1<<16;
			quad[2].SYSX+=1<<16;
			quad[3].SYSX+=1<<16;
			quad[0].SYSX+=1<<16;
		}

		((SLONG *)p)[2]=quad[0].SYSX;
		((SLONG *)p)[5]=quad[1].SYSX;
		((SLONG *)p)[8]=quad[2].SYSX;
		((SLONG *)p)[11]=quad[3].SYSX;


//		if(split_all || z<10)
/*
		if(split_all || z>FLOOR_SPLIT_DIST)
		{
			split_quad_all(quad,p,z);
			return;

		}
		else
*/
//		if(floor_split_a)
/*
		if(texture&0xc000)
		{
			//return;
			if(texture&(1<<15))
			{
				split_quad_a(quad,p,z);
				return;
			}
			
			if(texture&(1<<14))
			{
				split_quad_b(quad,p,z);
				return;

			}
		}
*/
		DOPRIM(z,p);

#ifdef WHEN_DO_I_WANT_TO_TWO_PASS
		if(two_pass)
		{
			memcpy((UBYTE*)*cp,(UBYTE*)p,sizeof(POLY_GT4));
			setSemiTrans(p,1);
			p->tpage&=~(3<<5);
		}
		

		{
			if(two_pass)
			{
				p=(POLY_GT4 *)*cp; //the_display.CurrentPrim;
				*cp+=sizeof(POLY_GT4);
				p->x0+=2;
				p->x1+=2;
				p->x2+=2;
				p->x3+=2;
				DOPRIM(z,p);
			}
		}
#endif

	}
}
#endif
inline	void	add_floor_quad_quick(PSX_POLY_Point *quad,PAP_Hi *ph,SLONG no_split,POLY_GT4 *p)
{
	SLONG	z;
	UWORD	texture;



	//
	// find furthest z
	//

	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	if(!(no_split&2))
	{
		//
		//not a roof face
		//
		z+=5;
	}
	if(in_ware)
		z+=20;
	z=get_z_sort(z);
//	ASSERT(z>=0&&z<=2048);

	{
		SLONG	s;


		texture=ph->Texture;
		set_floor_texture(p,texture);
		setPolyGT4(p);


		if(z<=FLOOR_SPLIT_DIST)
		{
			quad[1].SYSX+=1<<16;
			quad[2].SYSX+=1<<16;
			quad[3].SYSX+=1<<16;
			quad[0].SYSX+=1<<16;
		}


		((SLONG *)p)[2]=quad[0].SYSX;
		((SLONG *)p)[5]=quad[1].SYSX;
		((SLONG *)p)[8]=quad[2].SYSX;
		((SLONG *)p)[11]=quad[3].SYSX;

//#ifdef MIKE
		if((no_split&1)==0 && z>FLOOR_SPLIT_DIST)
		{
			split_quad_all(quad,p,z);

		}
		else
//#endif
		{
			DOPRIM(z,p);
			return;
		}


	}
}
void	add_floor_quad_quick_ni(PSX_POLY_Point *quad,PAP_Hi *ph,POLY_GT4 *p)
{
	SLONG	z;
	UWORD	texture;

	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;



	z+=10;
	z=get_z_sort(z);
		if(z<=FLOOR_SPLIT_DIST)
		{
			quad[1].SYSX+=1<<16;
			quad[2].SYSX+=1<<16;
			quad[3].SYSX+=1<<16;
			quad[0].SYSX+=1<<16;
		}

	{


//		setPolyGT4(p);

		((SLONG *)p)[2]=quad[0].SYSX;
		((SLONG *)p)[5]=quad[1].SYSX;
		((SLONG *)p)[8]=quad[2].SYSX;
//		ASSERT(0);
		((SLONG *)p)[11]=quad[3].SYSX;
		texture=ph->Texture;
		set_floor_texture(p,texture);

	}
}
#ifdef	DOG_POO
void	add_floor_quad_ni(PSX_POLY_Point *quad,PAP_Hi *ph,UWORD col0,UWORD col1,UWORD col2,UWORD col3,SLONG shadow,ULONG lum,SLONG split_all,POLY_GT4 *p)
{
	SLONG	z;
	SLONG	two_pass=0;
	UBYTE	*s_map;
	UWORD	texture;

	s_map=&shadow_map[shadow&0xff][0];


	if(PadKeyIsPressed(&PAD_Input1,PAD_FLT))
		two_pass=1;
/*
	if(the_display.CurrentPrim>&(the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM-100]))
	{
		return;
	}
*/
	//
	// find furthest z
	//
	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	z=get_z_sort(z);
	ASSERT(z>=0&&z<=2048);

	//
	// no shadow for now
	//
	{
		SLONG	b0,b1,b2,b3;
//		POLY_GT4	*p;
		SLONG	s;

//		ALLOCPRIM(p,POLY_GT4);

		texture=ph->Texture;
		set_floor_texture(p,texture);
		setPolyGT4(p);
//		do_4_lights(p,quad);
		//
		// was doing 4 duplicate reads of NIGHT_flag    saves 12 ticks ish
		//
		if((NIGHT_flag & NIGHT_FLAG_DAYTIME))
		{
			b0=getPSXFade_day(quad[0].P);
			b1=getPSXFade_day(quad[1].P);
			b2=getPSXFade_day(quad[2].P);
			b3=getPSXFade_day(quad[3].P);
		}
		else
		{
			b0=getPSXFade_night(quad[0].P);
			b1=getPSXFade_night(quad[1].P);
			b2=getPSXFade_night(quad[2].P);
			b3=getPSXFade_night(quad[3].P);
		}

		if ((b0==0)||(b0==255))
			return;

extern void AENG_add_semi_fade(PSX_POLY_Point *pp,UWORD b0,UWORD b1,UWORD b2,UWORD b3,SLONG z);

//		if((b0!=128)||(b1!=128)||(b2!=128)||(b3!=128))
		if(1)
		{
			UBYTE	shift[4];

			if(shadow)
			{
				shift[0]=s_map[0];
				shift[1]=s_map[1];
				shift[2]=s_map[2];
				shift[3]=s_map[3];
			}
			else
			{
				shift[0]=0;
				shift[1]=0;
				shift[2]=0;
				shift[3]=0;

			}

			b0>>=1;
			b1>>=1;
			b2>>=1;
			b3>>=1;

			setRGB0(p,MAKELUMI((((col0>>10)&0x3f)*b0)>>(4+shift[0]),lum&0xff),
				      MAKELUMI((((col0>>5)&0x1f)*b0)>>(3+shift[0]),lum&0xff),
					  MAKELUMI((((col0)&0x1f)*b0)>>(3+shift[0]),(lum>>1)&0x7f));

			setRGB1(p,MAKELUMI((((col1>>10)&0x3f)*b1)>>(4+shift[1]),(lum>>8)&0xff),
				      MAKELUMI((((col1>>5)&0x1f)*b1)>>(3+shift[1]),(lum>>8)&0xff),
					  MAKELUMI((((col1)&0x1f)*b1)>>(3+shift[1]),(lum>>9)&0x7f));

			setRGB2(p,MAKELUMI((((col2>>10)&0x3f)*b2)>>(4+shift[2]),(lum>>16)&0xff),
				      MAKELUMI((((col2>>5)&0x1f)*b2)>>(3+shift[2]),(lum>>16)&0xff),
					  MAKELUMI((((col2)&0x1f)*b2)>>(3+shift[2]),(lum>>17)&0x7f));

			setRGB3(p,MAKELUMI((((col3>>10)&0x3f)*b3)>>(4+shift[3]),lum>>24),
				      MAKELUMI((((col3>>5)&0x1f)*b3)>>(3+shift[3]),lum>>24),
					  MAKELUMI(((col3&0x1f)*b3)>>(3+shift[3]),lum>>25));
		}

		if(z<=FLOOR_SPLIT_DIST)
		{
			quad[0].Word.SY++;
			quad[1].Word.SY++;
			quad[2].Word.SY++;
			quad[3].Word.SY++;

		}

		((SLONG *)p)[2]=quad[0].SYSX;
		((SLONG *)p)[5]=quad[1].SYSX;
		((SLONG *)p)[8]=quad[2].SYSX;
		((SLONG *)p)[11]=quad[3].SYSX;




	}
}
#endif

void	add_floor_2tri(PSX_POLY_Point *quad,PAP_Hi *ph,SLONG shadow,POLY_GT4 *p)
{
	SLONG	z;
	SLONG	two_pass=0;
	UBYTE	*s_map;
	UWORD	texture;

//	if(shadow==1)
//		return;
/*
	if(p->x0<0)
		p->x0=0;
	if(p->x1<0)
		p->x1=0;
	if(p->x2<0)
		p->x2=0;
	if(p->x3<0)
		p->x3=0;

	if(p->x0>320)
		p->x0=319;
	if(p->x1>320)
		p->x1=319;
	if(p->x2>320)
		p->x2=319;
	if(p->x3>320)
		p->x3=319;

	if(p->y0<0)
		p->y0=0;
	if(p->y1<0)
		p->y1=0;
	if(p->y2<0)
		p->y2=0;
	if(p->y3<0)
		p->y3=0;

	if(p->y0>256)
		p->y0=255;
	if(p->y1>256)
		p->y1=255;
	if(p->y2>256)
		p->y2=255;
	if(p->y3>256)
		p->y3=255;
*/
/*
	POLY_GT3	*p1,*p2;
	ALLOCPRIM(p1,POLY_GT3);
	setPolyGT3(p1);

	ASSERT(p->x1>-512 && p->x1 <900);
	ASSERT(p->x2>-512 && p->x2 <900);
	ASSERT(p->x3>-112 && p->x3 <400);

	ASSERT(p->y1>-512 && p->y1 <900);
	ASSERT(p->y2>-512 && p->y2 <900);
	ASSERT(p->y3>-512 && p->y3 <900);
		p1->x0=p->x1;	  	
		p1->y0=p->y1;	  	
							 
		p1->x1=p->x3;	  	
		p1->y1=p->y3;	  	
							 
		p1->x2=p->x2;	  	
		p1->y2=p->y2;	  	
	setXY3(p1,10,10,150,20,20,100);




	setRGB0(p1,255,128,128);
	setRGB1(p1,255,128,128);
	setRGB2(p1,255,128,128);

	p1->clut=p->clut;
	p1->tpage=p->tpage;

	setUV3(p1,0,0,128,128,64,0);

	DOPRIM(1,p1);
	return;
*/

	
	
	
	s_map=&shadow_map[shadow&0xff][0];




	//
	// find furthest z
	//
	z=quad[0].Z;
	if(z<quad[1].Z)
		z=quad[1].Z;
	if(z<quad[2].Z)
		z=quad[2].Z;
	if(z<quad[3].Z)
		z=quad[3].Z;
//	z>>=1;


	z=get_z_sort(z);
	ASSERT(z>=0&&z<=1024);

	{
		SLONG	b0,b1,b2,b3;
		POLY_GT3	*p1,*p2;
		SLONG	s;

//		ASSERT(0);
		ALLOCPRIM(p1,POLY_GT3);
		ALLOCPRIM(p2,POLY_GT3);

		setPolyGT3(p1);
		setPolyGT3(p2);



//		if((b0!=128)||(b1!=128)||(b2!=128)||(b3!=128))
		{
			UBYTE	shift[4];


			p1->x0=p->x1;	  		p2->x0=p->x0;    
			p1->y0=p->y1;	  		p2->y0=p->y0;    
							                           
			p1->x1=p->x3;	  		p2->x1=p->x1;    
			p1->y1=p->y3;	  		p2->y1=p->y1;    
							                           
			p1->x2=p->x2;	  		p2->x2=p->x2;    
			p1->y2=p->y2;	  		p2->y2=p->y2;    


			p1->u0=p->u1;			p2->u0=p->u0; 
			p1->v0=p->v1;			p2->v0=p->v0; 
									                  
			p1->u1=p->u3;			p2->u1=p->u1; 
			p1->v1=p->v3;			p2->v1=p->v1; 
									                  
			p1->u2=p->u2;			p2->u2=p->u2; 
			p1->v2=p->v2;			p2->v2=p->v2; 

			p1->clut=p2->clut=p->clut;
			p1->tpage=p2->tpage=p->tpage;

			if(shadow==7 )
			{
				
				//    0  1				   p2(0)....p2(1)    p1(0)
				//		/x					  .....		   ....
				//	   /xx					  ...		.......
				//	  2xx3				   p2(2)	p1(2)....p1(1)

				p1->r2=MAX(p->r2-64,0);
				p1->g2=MAX(p->g2-64,0);
				p1->b2=MAX(p->b2-64,0);

				p1->r1=MAX(p->r3-64,0);
				p1->g1=MAX(p->g3-64,0);
				p1->b1=MAX(p->b3-64,0);

				p1->r0=p->r1;
				p1->g0=p->g1;
				p1->b0=p->b1;


				p2->r0=p->r0;
				p2->g0=p->g0;
				p2->b0=p->b0;

				p2->r1=p->r1;
				p2->g1=p->g1;
				p2->b1=p->b1;

				p2->r2=p->r2;
				p2->g2=p->g2;
				p2->b2=p->b2;
/*
				setRGB0(p2,MAKELUMI((((col0>>10)&0x3f)*b0)>>(4),lum&0xff),
						  MAKELUMI((((col0>>5)&0x1f)*b0)>>(3),lum&0xff),
						  MAKELUMI((((col0)&0x1f)*b0)>>(3),(lum>>1)&0x7f));

				setRGB1(p2,MAKELUMI((((col1>>10)&0x3f)*b1)>>(4),(lum>>8)&0xff),
						  MAKELUMI((((col1>>5)&0x1f)*b1)>>(3),(lum>>8)&0xff),
						  MAKELUMI((((col1)&0x1f)*b1)>>(3),(lum>>9)&0x7f));

				setRGB2(p2,MAKELUMI((((col2>>10)&0x3f)*b2)>>(4),(lum>>16)&0xff),
						  MAKELUMI((((col2>>5)&0x1f)*b2)>>(3),(lum>>16)&0xff),
						  MAKELUMI((((col2)&0x1f)*b2)>>(3),(lum>>17)&0x7f));
*/

			}
			else
			{
				//    0xx1				   p2(0)....p2(1)    p1(0)
				//	  xx/ 					  .....		   ....
				//	  x/  					  ...		.......
				//	  2  3				   p2(2)	p1(2)....p1(1)

				p2->r2=MAX(p->r2-64,0);
				p2->g2=MAX(p->g2-64,0);
				p2->b2=MAX(p->b2-64,0);

				p2->r0=MAX(p->r0-64,0);
				p2->g0=MAX(p->g0-64,0);
				p2->b0=MAX(p->b0-64,0);

				p2->r1=p->r1;
				p2->g1=p->g1;
				p2->b1=p->b1;


				p1->r0=p->r1;
				p1->g0=p->g1;
				p1->b0=p->b1;

				p1->r1=p->r3;
				p1->g1=p->g3;
				p1->b1=p->b3;

				p1->r2=p->r2;
				p1->g2=p->g2;
				p1->b2=p->b2;
/*
				setRGB0(p1,MAKELUMI((((col1>>10)&0x3f)*b1)>>(4),(lum>>8)&0xff),
						  MAKELUMI((((col1>>5)&0x1f)*b1)>>(3),(lum>>8)&0xff),
						  MAKELUMI((((col1)&0x1f)*b1)>>(3),(lum>>9)&0x7f));

				setRGB1(p1,MAKELUMI((((col3>>10)&0x3f)*b3)>>(4+shift[3]),lum>>24),
						  MAKELUMI((((col3>>5)&0x1f)*b3)>>(3+shift[3]),lum>>24),
						  MAKELUMI(((col3&0x1f)*b3)>>(3+shift[3]),lum>>25));

				setRGB2(p1,MAKELUMI((((col2>>10)&0x3f)*b2)>>(4),(lum>>16)&0xff),
						  MAKELUMI((((col2>>5)&0x1f)*b2)>>(3),(lum>>16)&0xff),
						  MAKELUMI((((col2)&0x1f)*b2)>>(3),(lum>>17)&0x7f));
*/


			}

		}

//	setXY3(p1,10,10,150,20,20,100);
//	setXY3(p2,10,10,150,20,20,100);
//	if ((MF_NormalClip(((SLONG*)p1)[2],((SLONG*)p1)[5],((SLONG*)p1)[8]))>0)
		DOPRIM(z,p1);
		DOPRIM(z,p2);


	}
}

void add_kerb(PSX_POLY_Point *pp0,PSX_POLY_Point *pp1)
{
	POLY_F4 *p;
	PSX_POLY_Point pp2,pp3;
	SLONG z;
/*
	pp2.World.vx=pp0->World.vx;
	pp3.World.vx=pp1->World.vx;
	pp2.World.vy=pp0->World.vy+32;
	pp3.World.vy=pp1->World.vy+32;
	pp2.World.vz=pp0->World.vz;
	pp3.World.vz=pp1->World.vz;
*/
	
	pp0->World.vy+=32;
	gte_RotTransPers(&pp0->World,&pp2.SYSX,&pp2.P,&pp2.Flag,&pp2.Z);
	pp0->World.vy-=32;
	pp1->World.vy+=32;
	gte_RotTransPers(&pp1->World,&pp3.SYSX,&pp3.P,&pp3.Flag,&pp3.Z);
	pp1->World.vy-=32;

	if (MF_NormalClip(pp2.SYSX,pp3.SYSX,pp0->SYSX)>0)
	{
		ALLOCPRIM(p,POLY_F4);
		setlen(p, 5);//,  setcode(p, 0x28)  ===  setPolyF4(p);
		((ULONG*)p)[1]=0x28<<24;  //set RGB and code in one long write :)
		//setRGB0(p,0,0,0);
		((SLONG*)p)[2]=pp2.SYSX; 
		((SLONG*)p)[3]=pp3.SYSX; 
		((SLONG*)p)[4]=pp0->SYSX; 
		((SLONG*)p)[5]=pp1->SYSX; 

//		setXY4(p,pp2.Word.SX,pp2.Word.SY,pp3.Word.SX,pp3.Word.SY,
//				 pp0->Word.SX,pp0->Word.SY,pp1->Word.SX,pp1->Word.SY);
		z=MAX4(pp0->Z,pp1->Z,pp2.Z,pp3.Z);
		//z=(pp0->Z-150)>>1;
		z=get_z_sort(z);
		DOPRIM(z,p);
	}
}



struct	FloorStore
{
	UBYTE	R,G,B;
	UBYTE	Flag;
};

#define	MAX_WIDTH	27

extern	void	fuck_z(PSX_POLY_Point *pp);
void	fuck_floor_z(PSX_POLY_Point *pp)
{
	SLONG	dx,dy,dz;
	SLONG	t;
	VECTOR	out1;
	ULONG	flag;

	extern	SLONG	POLY_cam_yaw;
extern	SLONG AENG_cam_yaw;
/*
	if (PadKeyIsPressed(&PAD_Input1,PAD_RL))
		return;
*/

	pp->Flag|=(1<<17);
	gte_RotTrans(&pp->World,&out1,&flag);
	if(out1.vz<210 && out1.vz>-150)
	{
		t=214-out1.vz;

		dx=(PSX_view_matrix.m[2][0]*t)>>12;
		dy=(PSX_view_matrix.m[2][1]*t)>>12;
		dz=(PSX_view_matrix.m[2][2]*t)>>12;

//		if(dx<296 && dz<296)
		{

			pp->World.vx+=dx;
			pp->World.vy+=dy;
			pp->World.vz+=dz;
/*
			RotTrans(&pp->World,&out1,&flag);
			if(out1.vz<210)
			{
				ASSERT(0);
				if(t>1000000)
					if(abs(dx)>1000)
					if(abs(dz)>1000)
						ASSERT(0);
			}
*/


			gte_RotTransPers(&pp->World,&pp->SYSX,&pp->P,&pp->Flag,&pp->Z);

			pp->World.vx-=dx;
			pp->World.vy-=dy;
			pp->World.vz-=dz;
		}
	}
}

#define	SHADOW_OVERLAP		8

void AENG_draw_floor(void)
{
	struct	FloorStore	row[MAX_WIDTH*2];
	struct	FloorStore	*p1,*p2;

	SLONG world_x=POLY_cam_x;
	SLONG world_y=POLY_cam_y;
	SLONG world_z=POLY_cam_z;
	SWORD	x,z;
	SLONG	flag;
	SLONG	kerb=0;

	PSX_POLY_Point *pp=perm_pp_array;
	PAP_Hi *ph;
	UWORD	*col;
	ULONG	flag_or,flag_and;
	ULONG	near_clipped;
	SLONG	max_y=POLY_cam_y;
	SLONG	roof_y;
	SLONG	xmax;
	ULONG	day_flag;
	SLONG	lum_off_x_local,lum_off_z_local;
	SLONG	skip_floor=0;


	if(roper_pickup==1)
		skip_floor=1;

	lum_off_x_local=lum_off_x;
	lum_off_z_local=lum_off_z;

	day_flag=(NIGHT_flag & NIGHT_FLAG_DAYTIME);

	//
	// clear the floor lighting vertex cache
	//
	{
		ULONG	*p;
		p=(ULONG*)&row[0];
		for(z=0;z<MAX_WIDTH*2;z++)
		{
			*p++=0;
		}
	}
//	ASSERT(	NGAMUT_point_zmin<0);
//	ASSERT(	NGAMUT_point_zmax<=127);

	if(NGAMUT_point_zmax>126)
		NGAMUT_point_zmax=126;


	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		SLONG	xcount;

		xmax=NGAMUT_point_gamut[z].xmax;

		xcount=NGAMUT_point_gamut[z].xmin-(NGAMUT_xmin);
		ASSERT(xcount>=0 && xcount<MAX_WIDTH);


		if(z&1)
		{
			p1=&row[0+xcount];
			p2=&row[MAX_WIDTH+xcount];
		}
		else
		{
			p1=&row[MAX_WIDTH+xcount];
			p2=&row[0+xcount];
		}

		xcount=xmax-NGAMUT_point_gamut[z].xmin;
		if(xmax>126)
			xmax=126;
		ASSERT(xcount<MAX_WIDTH);
		{
			ULONG	*p;
			p=(ULONG*)p2;

			do
			{
				*p++=0;
			}
			while(--xcount>=0);
		}
		
		ASSERT(xmax<=127)
		for (x = NGAMUT_point_gamut[z].xmin,xcount=0; x <=xmax ; x++,xcount++,p1++,p2++)
		{

			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

			ph = &PAP_2HI(x,z);

			col=&floor_psx_col[x][z];
			near_clipped=0;

//			if (!(ph->Flags & FLOOR_HIDDEN))
			{
				SLONG	shadow;

				if(ph->Flags&PAP_FLAG_SINK_SQUARE)
				{
					kerb=1;
					world_y+=32;
				}
				//
				// The upper point.
				//
				roof_y=1;
				if ((ph->Flags & PAP_FLAG_ROOF_EXISTS))
				{
					
					roof_y=(MAVHEIGHT(x,z)<<6);
					if(roof_y>=max_y)
						goto	end_loop;
//						continue;

					pp[0].World.vy=	pp[1].World.vy=pp[2].World.vy=pp[3].World.vy=roof_y-world_y;



				}
				else
				if (ph->Flags & FLOOR_HIDDEN)
				{
					goto	end_loop;
					//continue;
				}
				else
				{
					if(skip_floor)
						goto	end_loop;
					//	continue;
					pp[0].World.vy=	((ph)->Alt * (1 << ALT_SHIFT))-world_y;
					pp[1].World.vy=	((ph+MAP_WIDTH)->Alt * (1 << ALT_SHIFT))-world_y;
					pp[2].World.vy=	((ph+1)->Alt * (1 << ALT_SHIFT))-world_y;
					pp[3].World.vy=	((ph+1+MAP_WIDTH)->Alt * (1 << ALT_SHIFT))-world_y;

//			 		if (ph->Flags & PAP_FLAG_NOGO)
			 		if (ph->Texture&(1<<14))// & PAP_FLAG_NOGO)
					{
						pp[0].World.vy+=(COS(((x<<5)+(sea_offset))&2047)+SIN(((z<<4)+(sea_offset)+700)&2047))>>13;
						
					}
//			 		if ((ph+MAP_WIDTH)->Flags & PAP_FLAG_NOGO)
			 		if ((ph+MAP_WIDTH)->Texture&(1<<14))// & PAP_FLAG_NOGO)
					{						  
						pp[1].World.vy+=(COS((((x+1)<<5)+(sea_offset))&2047)+SIN(((z<<4)+(sea_offset)+700)&2047))>>13;
						
					}
//			 		if ((ph+1)->Flags & PAP_FLAG_NOGO)
			 		if ((ph+1)->Texture&(1<<14))// & PAP_FLAG_NOGO)
					{
						pp[2].World.vy+=(COS(((x<<5)+(sea_offset))&2047)+SIN((((z+1)<<4)+(sea_offset)+700)&2047))>>13;
					}
//			 		if ((ph+1+MAP_WIDTH)->Flags & PAP_FLAG_NOGO)
			 		if ((ph+1+MAP_WIDTH)->Texture&(1<<14))// & PAP_FLAG_NOGO)
					{
						pp[3].World.vy+=(COS((((x+1)<<5)+(sea_offset))&2047)+SIN((((z+1)<<4)+(sea_offset)+700)&2047))>>13;
					}


				}
				shadow=ph->Flags&(PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3);
				if(shadow==7 || shadow==1)
					shadow=-shadow;

				pp[0].World.vx=	(x<<ELE_SHIFT)-world_x;
				pp[0].World.vz=	(z<<ELE_SHIFT)-world_z;
				if(shadow<0)
				{
					pp[0].World.vx-=SHADOW_OVERLAP;
					pp[0].World.vz-=SHADOW_OVERLAP;
				}


//				gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
//				#define gte_RotTransPers(r1,r2,r3,r4,r5)   			
				{
					gte_ldv0(&pp[0].World);   	
					gte_rtps();     	

					// 14 ticks for your delight
					pp[1].World.vx=	((x+1)<<ELE_SHIFT)-world_x;
					pp[1].World.vz=	((z)<<ELE_SHIFT)-world_z;
					if(shadow<0)
					{
						pp[1].World.vx+=SHADOW_OVERLAP;
						pp[1].World.vz-=SHADOW_OVERLAP;
					}

					gte_stflg(&pp[0].Flag);		
					if(pp[0].Flag&(3<<17))
//					if(0)
					{
						fuck_z(&pp[0]);
						near_clipped=1;
					}
					else
					{

						gte_stsxy(&pp[0].SYSX); 		
						gte_stdp(&pp[0].P);   	
						gte_stszotz(&pp[0].Z);	
					}
				}

//				gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
				{
					gte_ldv0(&pp[1].World);   	
					gte_rtps();     	

					pp[2].World.vx=	((x)<<ELE_SHIFT)-world_x;
					pp[2].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
					if(shadow<0)
					{
						pp[2].World.vx-=SHADOW_OVERLAP;
						pp[2].World.vz+=SHADOW_OVERLAP;
					}

					gte_stflg(&pp[1].Flag);		

					if(pp[1].Flag&(3<<17))
//					if(0)
					{
						fuck_z(&pp[1]);
						near_clipped=1;
					}
					else
					{
						gte_stsxy(&pp[1].SYSX); 		
						gte_stdp(&pp[1].P);   	
						gte_stszotz(&pp[1].Z);	
					}
				}


//				pp[2].World.vx=	((x)<<ELE_SHIFT)-world_x;
//				pp[2].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
//				gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
				{
					gte_ldv0(&pp[2].World);   	
					gte_rtps();     	

					pp[3].World.vx=	((x+1)<<ELE_SHIFT)-world_x;
					pp[3].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
					if(shadow<0)
					{
						pp[3].World.vx+=SHADOW_OVERLAP;
						pp[3].World.vz+=SHADOW_OVERLAP;
					}

					gte_stflg(&pp[2].Flag);		
					if(pp[2].Flag&(3<<17))
//					if(0)
					{
						fuck_z(&pp[2]);
						near_clipped=1;
					}
					else
					{
						gte_stsxy(&pp[2].SYSX); 		
						gte_stdp(&pp[2].P);   	
						gte_stszotz(&pp[2].Z);	
					}
				}


//				pp[3].World.vx=	((x+1)<<ELE_SHIFT)-world_x;
//				pp[3].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
//				gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
				{
					gte_ldv0(&pp[3].World);   	
					gte_rtps();     	

					flag_or=(pp[0].Flag|pp[1].Flag|pp[2].Flag);//|pp[3].Flag);
					flag_and=(pp[0].Flag&pp[1].Flag&pp[2].Flag);//&pp[3].Flag);

					gte_stflg(&pp[3].Flag);		

					if(pp[3].Flag&(3<<17))
//					if(0)
					{
						fuck_z(&pp[3]);
						near_clipped=1;
					}
					else
					{
						gte_stsxy(&pp[3].SYSX); 		
						gte_stdp(&pp[3].P);   	
						gte_stszotz(&pp[3].Z);	
					}
				}

				flag_or|=pp[3].Flag;
				flag_and&=pp[3].Flag;


//				flag_or=(pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag);
//				flag_and=(pp[0].Flag&pp[1].Flag&pp[2].Flag&pp[3].Flag);

				//
				//     If all points valid   || at least one valid    && at least one near clipped    && not backwards
				//
/*				
				if( (flag_and&(1<<31))==0 && (flag_or&(1<<17)))
				{
					near_clipped=1;
				}
				else
					near_clipped=0;
*/

				if((roof_y&1)==0)
				{
					near_clipped|=2;
					if((MF_NormalClip(pp[0].SYSX,pp[1].SYSX,pp[2].SYSX))<=0)
					{
						//
						// roof face and back face clipped
						//
						//max_y=MIN(max_y,roof_y);
						goto	end_loop;
//						continue;
					}
				}
				if (   ((flag_or&(1<<31))==0))// || near_clipped))
				{
					SLONG	tflag,c0;
					flag=0xffffffff;

//								if(0)


					for(c0=0;c0<4;c0++)
					{
						tflag=0;
						if(pp[c0].Word.SX<0)
						{
							tflag|=POLY_CLIP_LEFT;
//							if(pp[c0].Word.SX<-100)
//								pp[c0].Word.SX=-100;
						}
						if(pp[c0].Word.SX>=320)
							tflag|=POLY_CLIP_RIGHT;
						if(pp[c0].Word.SY<0)
							tflag|=POLY_CLIP_TOP;
						if(pp[c0].Word.SY>=SCREEN_HEIGHT)
						{
//							if(pp[c0].Word.SY>=400)
//								pp[c0].Word.SY=400;

							tflag|=POLY_CLIP_BOTTOM;
						}
						flag&=tflag;
					}
					if(flag==0)// && !near_clipped)
					{
						#define LUMI(x,z) floor_lum[(x)-lum_off_x][(z)-lum_off_z]

						UBYTE	lum;//=LUMI(x,z)+(LUMI(x+1,z)<<8)+(LUMI(x,z+1)<<16)+(LUMI(x+1,z+1)<<24);
						//SLONG	shadow;
						UBYTE	*lum_p;
						UWORD	col0,b0;



						//
						// optimise the global read of lum_off_x,z to a local stack read  4 ticks * 8 * loop_count becomes 1 tick*8*loop_count
						// then use a pointer to avoid the address recalc (probably doesnt help much)
						lum_p=&floor_lum[(x)-lum_off_x_local][(z)-lum_off_z_local];

						//lum=(*lum_p)+((*(lum_p+32))<<8)+((*(lum_p+1))<<16)+((*(lum_p+33))<<24);

						if(p1->Flag)
						{


						}
						else
						{
							//
							// calc lighting for this point
							//
							lum=*lum_p;
							b0=getPSXFade_night(pp[0].P);
							b0>>=1;
							col0=*col;
							
							p1->R=MAKELUMI((((col0>>10)&0x3f)*b0)>>(4),lum);
							p1->G=MAKELUMI((((col0>>5)&0x1f)*b0)>>(3),lum);
							p1->B=MAKELUMI((((col0)&0x1f)*b0)>>(3),(lum>>1));
							p1->Flag=1;
						}
						p1++;
						if(p1->Flag)
						{


						}
						else
						{
							//
							// calc lighting for this point
							//
							lum=*(lum_p+32);
							b0=getPSXFade_night(pp[1].P);
							b0>>=1;
							col0=*(col+PAP_SIZE_HI);
							
							p1->R=MAKELUMI((((col0>>10)&0x3f)*b0)>>(4),lum);
							p1->G=MAKELUMI((((col0>>5)&0x1f)*b0)>>(3),lum);
							p1->B=MAKELUMI((((col0)&0x1f)*b0)>>(3),(lum>>1));
							p1->Flag=1;
						}
						p1--;
						if(p2->Flag)
						{


						}
						else
						{
							//
							// calc lighting for this point
							//
							lum=*(lum_p+1);
							b0=getPSXFade_night(pp[2].P);
							b0>>=1;
							col0=*(col+1);
							
							p2->R=MAKELUMI((((col0>>10)&0x3f)*b0)>>(4),lum);
							p2->G=MAKELUMI((((col0>>5)&0x1f)*b0)>>(3),lum);
							p2->B=MAKELUMI((((col0)&0x1f)*b0)>>(3),(lum>>1));
							p2->Flag=1;
						}
						p2++;
						if(p2->Flag)
						{


						}
						else
						{
							//
							// calc lighting for this point
							//
							lum=*(lum_p+33);
							b0=getPSXFade_night(pp[3].P);
							b0>>=1;
							col0=*(col+PAP_SIZE_HI+1);
							
							p2->R=MAKELUMI((((col0>>10)&0x3f)*b0)>>(4),lum);
							p2->G=MAKELUMI((((col0>>5)&0x1f)*b0)>>(3),lum);
							p2->B=MAKELUMI((((col0)&0x1f)*b0)>>(3),(lum>>1));
							p2->Flag=1;
						}
						p2--;



//						if(near_clipped)
//						{
//							shadow=0;
//						}
//						else
						{
//							shadow=ph->Flags&(PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3);
							if(shadow<0)
								shadow=-shadow;
						}


						{
							if(pp[0].Z>800)
							{
								if(kerb)
								{
									kerb=0;
									world_y-=32;
								}
								shadow=0;
							}
						}

						if (kerb)
						{
							if (!(PAP_2HI(x-1,z).Flags&PAP_FLAG_SINK_SQUARE))
								add_kerb(&pp[2],&pp[0]);
							else if (!(PAP_2HI(x+1,z).Flags&PAP_FLAG_SINK_SQUARE))
								add_kerb(&pp[1],&pp[3]);

							if (!(PAP_2HI(x,z-1).Flags&PAP_FLAG_SINK_SQUARE))
								add_kerb(&pp[0],&pp[1]);
							else if (!(PAP_2HI(x,z+1).Flags&PAP_FLAG_SINK_SQUARE))
								add_kerb(&pp[3],&pp[2]);
						}

/*
						if(shadow==7 || shadow==1)
						{
							POLY_GT4	p;
							add_floor_quad_quick_ni(&pp[0],ph,&p);
							add_floor_2tri(&pp[0],ph,shadow,&p);
						}
						else
*/

						{
							POLY_GT4	*p;
							POLY_GT4	p_temp;

							if(shadow==7 || shadow==1)
							{
								p=&p_temp;
//								ASSERT(0);
							}
							else
							{
								ALLOCPRIM(p,POLY_GT4);
							}

							if(shadow)
							{
								UBYTE	*s_map;
								s_map=&shadow_map[shadow&0xff][0];


								if(s_map[0])
								{
									setRGB0(p,MAX(0,p1->R-SHADOW_SUB),max(0,p1->G-SHADOW_SUB),max(0,p1->B-SHADOW_SUB));
								}
								else
								{
									//setRGB0(p,p1->R,p1->G,p1->B);
									((ULONG*)p)[1]=*(ULONG*)p1;
								}
								p1++;
								if(s_map[1])
								{
									setRGB1(p,MAX(0,p1->R-SHADOW_SUB),max(0,p1->G-SHADOW_SUB),max(0,p1->B-SHADOW_SUB));
								}
								else
								{
//									setRGB1(p,p1->R,p1->G,p1->B);
									((ULONG*)p)[4]=*(ULONG*)p1;
								}
								p1--;

								if(s_map[2])
								{
									setRGB2(p,MAX(0,p2->R-SHADOW_SUB),max(0,p2->G-SHADOW_SUB),max(0,p2->B-SHADOW_SUB));
								}
								else
								{
									//setRGB2(p,p2->R,p2->G,p2->B);
									((ULONG*)p)[7]=*(ULONG*)p2;
								}
								p2++;
								if(s_map[3])
								{
									setRGB3(p,MAX(0,p2->R-SHADOW_SUB),max(0,p2->G-SHADOW_SUB),max(0,p2->B-SHADOW_SUB));
								}
								else
								{
									//setRGB3(p,p2->R,p2->G,p2->B);
									((ULONG*)p)[10]=*(ULONG*)p2;
								}
								p2--;



							}

							else
							{
								((ULONG*)p)[1]=*(ULONG*)p1;
								((ULONG*)p)[4]=((ULONG*)p1)[1];
								((ULONG*)p)[7]=*(ULONG*)p2;
								((ULONG*)p)[10]=((ULONG*)p2)[1];
							}


							if((shadow==7 || shadow==1) )//&& !near_clipped)
							{
								add_floor_quad_quick_ni(&pp[0],ph,p);
								add_floor_2tri(&pp[0],ph,shadow,p);
							}
							else

							{
								add_floor_quad_quick(&pp[0],ph,near_clipped,p);

							}
						}
					}
				}
				else
				{
						if(0)
						if(near_clipped)
						{
							LINE_F2	*line;
							SLONG	r=128;
//									if(near_clipped)
//										r=255;

							ALLOCPRIM(line,LINE_F2);
							setLineF2(line);
							setXY2(line,80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2),80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2));
							setRGB0(line,r,128,128);
							DOPRIM(1000,line);
						
							ALLOCPRIM(line,LINE_F2);
							setLineF2(line);
							setXY2(line,80+(pp[1].Word.SX>>2),60+(pp[1].Word.SY>>2),80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2));
							setRGB0(line,r,128,128);
							DOPRIM(1000,line);

							ALLOCPRIM(line,LINE_F2);
							setLineF2(line);
							setXY2(line,80+(pp[3].Word.SX>>2),60+(pp[3].Word.SY>>2),80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2));
							setRGB0(line,r,128,128);
							DOPRIM(1000,line);

							ALLOCPRIM(line,LINE_F2);
							setLineF2(line);
							setXY2(line,80+(pp[2].Word.SX>>2),60+(pp[2].Word.SY>>2),80+(pp[0].Word.SX>>2),60+(pp[0].Word.SY>>2));
							setRGB0(line,r,128,128);
							DOPRIM(1000,line);



						}

				}
end_loop:;	
				if(kerb)
				{
					kerb=0;
					world_y-=32;
				}

				//
				// steam texture code deleted 4th jan 2000
				//
		}



			
/*
			POLY_transform(world_x, world_y, world_z, &pp[0]);

			world_x = (x+1)       <<ELE_SHIFT;
			world_y = (ph+MAP_WIDTH)->Alt * (1 << ALT_SHIFT);
			world_z = z       <<ELE_SHIFT;

			POLY_transform(world_x, world_y, world_z, &pp[1]);

			world_x = (x)       <<ELE_SHIFT;
			world_y = (ph+1)->Alt * (1 << ALT_SHIFT);
			world_z = (z+1)       <<ELE_SHIFT;

			POLY_transform(world_x, world_y, world_z, &pp[2]);

			world_x = (x+1)       <<ELE_SHIFT;
			world_y = (ph+1+MAP_WIDTH)->Alt * (1 << ALT_SHIFT);
			world_z = (z+1)       <<ELE_SHIFT;


			POLY_transform(world_x, world_y, world_z, &pp[3]);

			if (!(ph->Flags & FLOOR_HIDDEN))
			{
				if (POLY_valid_quad(&pp[0])) //inline this
				{
					add_quad(&pp[0],ph);

				}
			}
			else
			{
//				printf("hidden\n");
			}
*/
		}
	}
//	the_display.CurrentPrim=current_prim;

}

void AENG_draw_ware_floor(void)
{

	SLONG world_x=POLY_cam_x;
	SLONG world_y=POLY_cam_y;
	SLONG world_z=POLY_cam_z;
	SWORD	x,z;
	SLONG	flag;
	SLONG	kerb=0;

	PSX_POLY_Point *pp=perm_pp_array;
	PAP_Hi *ph;
	UWORD	*col;
	ULONG	day_flag;
	


//	UBYTE	*current_prim;
//	current_prim=the_display.CurrentPrim;
	day_flag=(NIGHT_flag & NIGHT_FLAG_DAYTIME);

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		for (x = NGAMUT_point_gamut[z].xmin; x <= NGAMUT_point_gamut[z].xmax; x++)
		{

			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

//extern	UBYTE	player_visited[16][128];
//			player_visited[x>>3][z]|=1<<(x&7);

//			me = &MAP[MAP_INDEX(x, z)];
			ph = &PAP_2HI(x,z);

			col=&floor_psx_col[x][z];

			if ((ph->Flags & FLOOR_HIDDEN) && !(ph->Flags & PAP_FLAG_ROOF_EXISTS))
			{
				SLONG	near=0;
/*
				if(ph->Flags&PAP_FLAG_SINK_SQUARE)
				{
					kerb=1;
					world_y+=32;
				}
*/
				//
				// The upper point.
				//

				pp[0].World.vx=	(x<<ELE_SHIFT)-world_x;
				pp[0].World.vy=	((ph)->Alt * (1 << ALT_SHIFT))-world_y;
				pp[0].World.vz=	(z<<ELE_SHIFT)-world_z;
				gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
				if(pp[0].Flag&(3<<17))
				{
					fuck_z(&pp[0]);
					near=1;
				}


				pp[1].World.vx=	((x+1)<<ELE_SHIFT)-world_x;
				pp[1].World.vy=	((ph+MAP_WIDTH)->Alt * (1 << ALT_SHIFT))-world_y;
				pp[1].World.vz=	((z)<<ELE_SHIFT)-world_z;
				gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);
				if(pp[1].Flag&(3<<17))
				{
					fuck_z(&pp[1]);
					near=1;
				}

				pp[2].World.vx=	((x)<<ELE_SHIFT)-world_x;
				pp[2].World.vy=	((ph+1)->Alt * (1 << ALT_SHIFT))-world_y;
				pp[2].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
				gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);
				if(pp[2].Flag&(3<<17))
				{
					fuck_z(&pp[2]);
					near=1;
				}

				pp[3].World.vx=	((x+1)<<ELE_SHIFT)-world_x;
				pp[3].World.vy=	((ph+1+MAP_WIDTH)->Alt * (1 << ALT_SHIFT))-world_y;
				pp[3].World.vz=	((z+1)<<ELE_SHIFT)-world_z;
				gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);
				if(pp[3].Flag&(3<<17))
				{
					fuck_z(&pp[3]);
					near=1;
				}

//				if (POLY_valid_quad(&pp[0])) //inline this
				if(((pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag)&(1<<31))==0)
				{
					SLONG	tflag,c0;
					flag=0xffffffff;
					for(c0=0;c0<4;c0++)
					{
						tflag=0;
						if(pp[c0].Word.SX<0)
							tflag|=POLY_CLIP_LEFT;
						if(pp[c0].Word.SX>=320)//DISPLAYWIDTH)
							tflag|=POLY_CLIP_RIGHT;
						if(pp[c0].Word.SY<0)
							tflag|=POLY_CLIP_TOP;
						if(pp[c0].Word.SY>=SCREEN_HEIGHT)
							tflag|=POLY_CLIP_BOTTOM;
						flag&=tflag;
					}
					if(flag==0)
					{
						POLY_GT4	*p;

						ALLOCPRIM(p,POLY_GT4);
						((ULONG*)p)[1]=0x3f3f3f;
						((ULONG*)p)[4]=0x3f3f3f;
						((ULONG*)p)[7]=0x3f3f3f;
						((ULONG*)p)[10]=0x3f3f3f;

						//
						// if near clipped dont split poly
						//
						add_floor_quad_quick(&pp[0],ph,near,p);
						


//						add_floor_quad(&pp[0],ph,0x7f7f7f,0x7f7f7f,0x7f7f7f,0x7f7f7f,0,0,0,p,day_flag);//ph->Flags&(PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3));
					}

				}
				if(kerb)
				{
					kerb=0;
					world_y-=32;
				}
			}
		}
	}
//	the_display.CurrentPrim=current_prim;

}

struct	BIGVECTOR
{
	SLONG	X,Y,Z;
};

struct	COLOUR
{
	UBYTE R,G,B;
};

struct COLOUR	leaf_col[]=
{
	{198,128,128},
	{128,198,128},
	{158,158,128},
	{128,168,100}
};

void SHAPE_droplet(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG dx,
		SLONG dy,
		SLONG dz,
		ULONG colour,
		SLONG page)
{
	PSX_POLY_Point *pp=perm_pp_array;
	POLY_FT3*	p;
	SLONG			dpx,dpy;
	SLONG			size,mul,len;
	SLONG			u,v;

	pp[0].World.vx=x-POLY_cam_x;
	pp[0].World.vy=y-POLY_cam_y;
	pp[0].World.vz=z-POLY_cam_z;
	pp[1].World.vx=pp[0].World.vx+dx;
	pp[1].World.vy=pp[0].World.vy+dy;
	pp[1].World.vz=pp[0].World.vz+dz;

	gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
	gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[0].P,&pp[1].Flag,&pp[1].Z);

	if (((pp[0].Flag|pp[1].Flag)&(1<<31))||(pp[0].Z>4095))
		return;
	if ((min(pp[0].Word.SX,pp[1].Word.SX)>DISPLAYWIDTH)||(max(pp[0].Word.SX,pp[1].Word.SX)<0)||
		(min(pp[0].Word.SY,pp[1].Word.SY)>256)||(max(pp[0].Word.SY,pp[1].Word.SY)<0))
		return;

//	p=(POLY_FT3*)the_display.CurrentPrim;
//	the_display.CurrentPrim+=sizeof(POLY_FT3);

	ALLOCPRIM(p,POLY_FT3);

	setPolyFT3(p);

	dpx = pp[0].Word.SX - pp[1].Word.SX;
	dpy = pp[0].Word.SY - pp[1].Word.SY;

	if (abs(dpx) > abs(dpy))
	{
		len = abs(dpx) + (abs(dpy)>>1);
	}
	else
	{
		len = abs(dpy) + (abs(dpx)>>1);
	}	

	size=(10<<9)/(420+(pp[0].Z));//(world_size<<12)/(420+(pp->Z>>2));///pp->Z;

	if (len)
	{
		dpx = (dpx * size) / len;
		dpy = (dpy * size) / len;
	}
	else
		dpx = dpy =0;

	setXY3(p,pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX+dpy,pp[1].Word.SY-dpx,pp[1].Word.SX-dpy,pp[1].Word.SY-dpx);
//	printf("(%d,%d)-(%d,%d) %d / %d (%d,%d)\n",pp[0].Word.SX,pp[0].Word.SY,pp[1].Word.SX,pp[1].Word.SY,size,len,dpx,dpy);
	u=getPSXU(page);
	v=getPSXV(page);
	setUV3(p,u+31,v+15,u,v,u,v+31);
//	setRGB0(p,128,128,128);
	setRGB0(p,colour&0xff,(colour>>8)&0xff,colour>>16);
	p->tpage=getPSXTPageE(page)&~(3<<5);
	p->clut=getPSXClutE(page);
	setSemiTrans(p,1);

	pp[0].Z=get_z_sort(pp[0].Z);
	DOPRIM(pp[0].Z,p);
//	DOPRIM(PANEL_OTZ,p);
}

extern SLONG matrix[9];

void SHAPE_rubbish(DIRT_Info *di)
{

}

void AENG_draw_dirt(void)
{
	SLONG i;

	#define LEAF_PAGE		(0)
	#define LEAF_CENTRE_U	(getPSXU(POLY_PAGE_LEAF)+16)
	#define LEAF_CENTRE_V	(getPSXV(POLY_PAGE_LEAF)+16)
	#define LEAF_RADIUS		(15)
	#define LEAF_U(a)		(LEAF_CENTRE_U + ((LEAF_RADIUS * SIN(a))>>16) )
	#define LEAF_V(a)		(LEAF_CENTRE_V + ((LEAF_RADIUS * COS(a))>>16) )
	#define LEAF_UP			8
	#define LEAF_SIZE       (20+(i&15))

	SLONG j,falling;

	DIRT_Info di;

	SLONG yaw;
	SLONG pitch;
	SLONG roll;

	SLONG angle;
	SVECTOR temp[3];
	PSX_POLY_Point *pp=perm_pp_array;
//	PSX_POLY_Point *tri[3];
	SLONG	flag,tflag,c0,p,page,clut,z;
	SLONG	world_x=POLY_cam_x;
	SLONG	world_y=POLY_cam_y;
	SLONG	world_z=POLY_cam_z;
	UBYTE	*cp;

	if(roper_pickup==1)
		return;

	cp=the_display.CurrentPrim;
	check_prim_ptr((void**)&cp);

//	ULONG leaf_colour_choice_rgb[4] =
//	{
//		0x332d1d,
//		0x243224,
//		0x123320,
//		0x332f07
//	};

	ULONG col;
//	ULONG leaf_colour;
//	ULONG leaf_specular;

//	tri[0] = &pp[0];
//	tri[1] = &pp[1];
//	tri[2] = &pp[2];

	page = getPSXTPageE(POLY_PAGE_LEAF);
	clut = getPSXClutE(POLY_PAGE_LEAF);

//	clut = psx_tpages_clut[LEAF_PAGE];

	//
	// Draw the dirt.
	//

	for (i = 0; i < DIRT_MAX_DIRT; i++)
	{
		falling = FALSE;
		if(DIRT_get_info(i,&di)==0)
		{
//			goto	do_next_dirt;
			continue;
		}

		
		switch(di.type)
		{
			
			case DIRT_INFO_TYPE_WATER:
				
//				the_display.CurrentPrim=cp;
				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
					0x00552211,
					POLY_PAGE_DRIP);
//				cp=the_display.CurrentPrim;
				break;

			case DIRT_INFO_TYPE_SNOW:
				col = 0xff000000+(di.morph1>>2)+((di.morph1>>2)<<8)+((di.morph1>>2)<<16);
				SPRITE_draw(di.x,di.y,di.z,32,col,0xff000000,POLY_PAGE_BLOOM1,1);
				break;

			case DIRT_INFO_TYPE_LEAF:

				if ((i & 0x7)==0)
				{
					if ((di.yaw | di.pitch | di.roll) == 0)
					{
						//
						// This happens often... so we optimise it out.
						//

						temp[0].vx = (di.x -LEAF_SIZE)-world_x;
						temp[0].vy = (di.y + LEAF_UP)-world_y;
						temp[0].vz = (di.z )-world_z;

						temp[1].vx = (di.x )-world_x;
						temp[1].vy = (di.y + LEAF_UP)-world_y;
						temp[1].vz = (di.z -LEAF_SIZE)-world_z;

						temp[2].vx = (di.x )-world_x;
						temp[2].vy = (di.y + LEAF_UP)-world_y;
						temp[2].vz = (di.z +LEAF_SIZE)-world_z;

						temp[3].vx = (di.x +LEAF_SIZE)-world_x;
						temp[3].vy = (di.y + LEAF_UP)-world_y;
						temp[3].vz = (di.z )-world_z;

					}
					else
					{
						//
						// The rotation matrix of this bit of dirt.
						//

						yaw   = (di.yaw);  //   * (PI / 1024.0F);
						pitch = (di.pitch);// * (PI / 1024.0F);
						roll  = (di.roll); //  * (PI / 1024.0F);

						MATRIX_calc(matrix, yaw, pitch, roll);

						//
						// Work out the position of the points.																													 				//
						//

						for (j = 0; j < 4; j++)
						{

							temp[j].vx  = (di.x)-world_x;
							temp[j].vy  = (di.y)+(LEAF_UP)-world_y;
							temp[j].vz  = (di.z)-world_z;

						}

						temp[3].vx += (matrix[6] * LEAF_SIZE)>>16;
						temp[3].vy += (matrix[7] * LEAF_SIZE)>>16;
						temp[3].vz += (matrix[8] * LEAF_SIZE)>>16;

						temp[0].vx -= (matrix[6] * LEAF_SIZE)>>16;
						temp[0].vy -= (matrix[7] * LEAF_SIZE)>>16;
						temp[0].vz -= (matrix[8] * LEAF_SIZE)>>16;

						temp[2].vx += (matrix[0] * LEAF_SIZE)>>16;
						temp[2].vy += (matrix[1] * LEAF_SIZE)>>16;
						temp[2].vz += (matrix[2] * LEAF_SIZE)>>16;

						temp[1].vx -= (matrix[0] * LEAF_SIZE)>>16;
						temp[1].vy -= (matrix[1] * LEAF_SIZE)>>16;
						temp[1].vz -= (matrix[2] * LEAF_SIZE)>>16;

						falling = TRUE;
					}

					//
					// Transform the points.
					//


					gte_RotTransPers(&temp[3],&pp[3].SYSX,&p,&flag,&z);
					gte_RotTransPers3(&temp[0],&temp[1],&temp[2],
									  &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,
										&p,&flag,&z);




						if (flag<0)
						{
							//
							// Tell the DIRT module that the leaf is off-screen.
							//

							DIRT_mark_as_offscreen(i);

							//
							// Don't bother transforming the other points.
							//

							goto do_next_dirt;
						}
						else
						{

							if(pp[0].Word.SX>=0 && pp[0].Word.SX<DISPLAYWIDTH && pp[0].Word.SY>=0 &&pp[0].Word.SY<SCREEN_HEIGHT)
							{
								UBYTE	rubbish=(7*8)+((i>>3)&3);
								UBYTE   u,v;

								u=getPSXU(rubbish);
								v=getPSXV(rubbish);


								SWORD	floor_red,floor_green,floor_blue;

	//							angle = i*100;
								POLY_FT4	*p;

	//							if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
	//								return;

								ALLOCPRIM(p,POLY_FT4);

								LONG_set_BBW(p,3,u,v,getPSXClutP(rubbish));
								LONG_set_BBW(p,5,u+31,v,getPSXTPageP(rubbish));
								LONG_set_BBW(p,7,u,v+31,0);
								LONG_set_BBW(p,9,u+31,v+31,0);


								//setRGB0(p,floor_red,floor_green,floor_blue); //leaf_col[col].R,leaf_col[col].G,leaf_col[col].B);
								((ULONG *)p)[1]=(0x2c<<24)|0x7f7f7f; //FT3
								((P_TAG *)p)->len=9;  //FT3


								((SLONG *)p)[2]=pp[0].SYSX;
								((SLONG *)p)[4]=pp[1].SYSX;
								((SLONG *)p)[6]=pp[2].SYSX;
								((SLONG *)p)[8]=pp[3].SYSX;


	//							z=(z);

								z=get_z_sort(z);

//								p->clut = clut;
//								p->tpage = page;

								DOPRIM(z,p);
								cp+=sizeof(POLY_FT3);
							}
							else
							{

								DIRT_mark_as_offscreen(i);
							}

						}
			 		break;
			 	}
				if ((di.yaw | di.pitch | di.roll) == 0)
				{
					//
					// This happens often... so we optimise it out.
					//

					temp[0].vx = (di.x)-world_x;
					temp[0].vy = (di.y + LEAF_UP)-world_y;
					temp[0].vz = (di.z + LEAF_SIZE)-world_z;

					temp[1].vx = (di.x + LEAF_SIZE)-world_x;
					temp[1].vy = (di.y + LEAF_UP)-world_y;
					temp[1].vz = (di.z - LEAF_SIZE)-world_z;

					temp[2].vx = (di.x - LEAF_SIZE)-world_x;
					temp[2].vy = (di.y + LEAF_UP)-world_y;
					temp[2].vz = (di.z - LEAF_SIZE)-world_z;
				}
				else
				{
					//
					// The rotation matrix of this bit of dirt.
					//

					yaw   = (di.yaw);  //   * (PI / 1024.0F);
					pitch = (di.pitch);// * (PI / 1024.0F);
					roll  = (di.roll); //  * (PI / 1024.0F);

					MATRIX_calc(matrix, yaw, pitch, roll);

					//
					// Work out the position of the points.																													 				//

					for (j = 0; j < 3; j++)
					{

						temp[j].vx  = (di.x)-world_x;
						temp[j].vy  = (di.y)+(LEAF_UP)-world_y;
						temp[j].vz  = (di.z)-world_z;

					}


					temp[0].vx += (matrix[6] * LEAF_SIZE)>>16;
					temp[0].vy += (matrix[7] * LEAF_SIZE)>>16;
					temp[0].vz += (matrix[8] * LEAF_SIZE)>>16;

					temp[1].vx -= (matrix[6] * LEAF_SIZE)>>16;
					temp[1].vy -= (matrix[7] * LEAF_SIZE)>>16;
					temp[1].vz -= (matrix[8] * LEAF_SIZE)>>16;

					temp[2].vx -= (matrix[6] * LEAF_SIZE)>>16;
					temp[2].vy -= (matrix[7] * LEAF_SIZE)>>16;
					temp[2].vz -= (matrix[8] * LEAF_SIZE)>>16;

					temp[1].vx += (matrix[0] * LEAF_SIZE)>>16;
					temp[1].vy += (matrix[1] * LEAF_SIZE)>>16;
					temp[1].vz += (matrix[2] * LEAF_SIZE)>>16;

					temp[2].vx -= (matrix[0] * LEAF_SIZE)>>16;
					temp[2].vy -= (matrix[1] * LEAF_SIZE)>>16;
					temp[2].vz -= (matrix[2] * LEAF_SIZE)>>16;

					falling = TRUE;
				}

				//
				// Transform the points.
				//


				gte_RotTransPers3(&temp[0],&temp[1],&temp[2],
								  &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,
									&p,&flag,&z);




					if (flag<0)
					{
						//
						// Tell the DIRT module that the leaf is off-screen.
						//

						DIRT_mark_as_offscreen(i);

						//
						// Don't bother transforming the other points.
						//

						goto do_next_dirt;
					}
					else
					{
/*
// only a bloody leaf lets bodge the clipping
						flag=0xffffffff;
						for(c0=0;c0<3;c0++)
						{
							tflag=0;
							if(pp[c0].Word.SX<0)
								tflag|=POLY_CLIP_LEFT;

							if(pp[c0].Word.SX>=SCREEN_WIDTH)
								tflag|=POLY_CLIP_RIGHT;

							if(pp[c0].Word.SY<0)
								tflag|=POLY_CLIP_TOP;

							if(pp[c0].Word.SY>=SCREEN_HEIGHT)
								tflag|=POLY_CLIP_BOTTOM;

							flag&=tflag;
						}
*/

//						if(flag==0) 
						if(pp[0].Word.SX>=0 && pp[0].Word.SX<DISPLAYWIDTH && pp[0].Word.SY>=0 &&pp[0].Word.SY<SCREEN_HEIGHT)
						{
							UBYTE	col=i&3;
							SWORD	floor_red,floor_green,floor_blue;
//							angle = i*100;
							POLY_FT3	*p;


//							if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
//								return;

//							p=(POLY_FT3 *)cp; //the_display.CurrentPrim;
							ALLOCPRIM(p,POLY_FT3);

							//setPolyFT3(p);
//							setUV3(p,LEAF_U(i&2047),LEAF_V((i&2047)),
//									LEAF_U((i+680)&2047),LEAF_V((i+680)&2047),
//									LEAF_U((i+1350)&2047),LEAF_V((i+1350)&2047));

							LONG_set_BBW(p,3,LEAF_U(i&2047),LEAF_V((i&2047)),clut);
							LONG_set_BBW(p,5,LEAF_U((i+680)&2047),LEAF_V((i+680)&2047),page);
							LONG_set_BBW(p,7,LEAF_U((i+1350)&2047),LEAF_V((i+1350)&2047),0);


							floor_red  =(((di.tween>>10)<<2))>>0; //*leaf_col[col].R)>>8;
							floor_green=(((di.tween>>5)<<3))>>0; //*leaf_col[col].G)>>8;
							floor_blue =(((di.tween)<<3))>>0; //*leaf_col[col].B )>>8;

							floor_red+=leaf_col[col].R>>1;
							floor_green+=leaf_col[col].G>>1;
							floor_blue+=leaf_col[col].B>>1;

							SATURATE(floor_red,0,128);
							SATURATE(floor_green,0,128);
							SATURATE(floor_blue,0,128);

							//setRGB0(p,floor_red,floor_green,floor_blue); //leaf_col[col].R,leaf_col[col].G,leaf_col[col].B);
							((ULONG *)p)[1]=(0x24<<24)|(floor_blue<<16)|(floor_green<<8)|floor_red; //FT3
							((P_TAG *)p)->len=7;  //FT3


							((SLONG *)p)[2]=pp[0].SYSX;
							((SLONG *)p)[4]=pp[1].SYSX;
							((SLONG *)p)[6]=pp[2].SYSX;
//							setXY3(p,pp[0].Word.SX,pp[0].Word.SY,
//									pp[1].Word.SX,pp[1].Word.SY,
//									pp[2].Word.SX,pp[2].Word.SY);



//							z=(z);

							z=get_z_sort(z);

//							p->clut = clut;
//							p->tpage = page;

							DOPRIM(z,p);
							cp+=sizeof(POLY_FT3);
						}
						else
						{

							DIRT_mark_as_offscreen(i);
						}



					}

				break;

			case DIRT_INFO_TYPE_PRIM:

//				printf("DIRT - PRIM (%d)\n",di.prim);
				{
					SLONG	dist,dx,dz;
					dx=(di.x)-POLY_cam_x;
					dz=(di.z)-POLY_cam_z;
					dist=dx*dx+dz*dz;

					if((dist<256*12*256*12))
					{
//						the_display.CurrentPrim=cp;
						MESH_draw_poly(
							di.prim,
							di.x,
							di.y,
							di.z,
							di.yaw,
							di.pitch,
							di.roll,
							NULL,0);
//						cp=the_display.CurrentPrim;
					}
				}

				break;
			case DIRT_INFO_TYPE_BLOOD:
//				the_display.CurrentPrim=cp;
				SHAPE_droplet(
					di.x,
					di.y,
					di.z,
					di.dx * 4,
					di.dy * 4,
					di.dz * 4,
					0x0000007f,
					POLY_PAGE_DRIP);
//				cp=the_display.CurrentPrim;
				break;

			case DIRT_INFO_TYPE_MORPH:
				/*

				MESH_draw_morph(
					di.prim,
					di.morph1,
					di.morph2,
					di.tween,
					di.x,
					di.y,
					di.z,
					di.yaw,
					di.pitch,
					di.roll,
					NULL);
					*/

				break;
			case DIRT_INFO_TYPE_SPARKS:
				break;
				

			default:
				ASSERT(0);
				break;
		}

		do_next_dirt:;
	}
//	printf("Done objects\n");
//	the_display.CurrentPrim=cp;
//	ASSERT(the_display.CurrentPrim < &the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM]);

}

void	draw_finished(void)
{
//	draw_state=0;
	draw_state--;
//	the_game.Packets[15]=0;
}


DB	*old_buffer;
void	*last_used_bucket_ram=0,*first_used_bucket_ram=0;

//
// just to ensure draw_state is set to 1 immediatly prior to DrawOtag
//
void	do_draw(void)
{
//	while(draw_state);

}

void AENG_flip_init(void)
{
#ifdef	OLD_FLIP
#else
	last_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem+200;
	first_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem;
#endif
	the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem+200;


	DrawSyncCallback(draw_finished);

	DrawSync(0);

	//
	// danger point is end of 
	//
	last_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem;
	the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem;
	first_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem;
	danger_point=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-200; // 200 hundred from end
	danger_point_type=0;

	ReadInputDevice();
}

UBYTE Ive_Not_Been_Flipped;
SLONG	geom=420;

void AENG_flip()
{
	static	SLONG	prev_sync;
	SLONG	temp;


	//
	// draw people at last second, so we know how much bucket ram we have for them
	//

#ifdef	MIKE
	{
		CBYTE	str[100];
		sprintf(str,"%d %d",available_bucket_ram()/1024,my_draw_dist>>8);
		FONT2D_DrawString(str,100,100);
	}
#endif


/*
			if (PadKeyIsPressed(&PAD_Input1,PAD_RL))
				geom+=10;
	if (PadKeyIsPressed(&PAD_Input1,PAD_RU))
		geom-=10;
	SetGeomScreen(geom);
*/



#ifdef	OLD_FLIP
//	DrawSync(0);		/* wait for hardware to finish drawing */
#else
//	ASSERT(0);
	first_used_bucket_ram=last_used_bucket_ram;
	last_used_bucket_ram=the_display.CurrentPrim;		
#endif
	ReadInputDevice();
	VSync(0);		/* wait for V-BLNK (1/60) */

	temp=VSync(-1);
	sync_count=temp-prev_sync;
	if(sync_count>4)
		sync_count=4;

	prev_sync=temp;

//	sync_count=2;

	ASSERT(Ive_Not_Been_Flipped<2);

	Ive_Not_Been_Flipped=0;

	temp=the_display.CurrentPrim-the_display.CurrentDisplayBuffer->PrimMem;
					    
	ASSERT(temp<BUCKET_MEM);

	old_buffer=the_display.CurrentDisplayBuffer;
	if(the_display.CurrentDisplayBuffer==&the_display.DisplayBuffers[0])
   		the_display.CurrentDisplayBuffer=&the_display.DisplayBuffers[1];
	else
   		the_display.CurrentDisplayBuffer=&the_display.DisplayBuffers[0];
	draw_state=1;
	PutDispEnv(&the_display.CurrentDisplayBuffer->Disp); 
	PutDrawEnv(&the_display.CurrentDisplayBuffer->Draw); 
	DrawOTag(old_buffer->ot);	//draw the bucketlist

//	if(PadKeyIsPressed(&PAD_Input1,PAD_FLT))
//		VSync(50);		/* wait for V-BLNK (1/60) */

//	draw_state=1;


//	the_game.Packets[15]=1;
//	do_draw();
//	DrawSync(0);

	if (temp>the_display.Max_Used)
		the_display.Max_Used=temp;
//#define MIKE	1
#ifdef MIKE
/*
	if(temp<((BUCKET_MEM>>1)-4000)&&double_ot!=1)
	{
		//
		// enough memory to double buffer the drawlist
		//
		the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem+(BUCKET_MEM>>1);
		double_ot=1;
	}
	else
	if(double_ot==1)
	{
		//
		//
		//if prev frame was double buffered (using end of ram) then do it again using start of ram
		double_ot=2;
		the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem;
	}
	else
*/
#endif

#ifdef	OLD_FLIP
	{
		double_ot=0;
		the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem;
	}
#endif

	//if(danger_point>first_used_bucket_ram && danger_point_type==1)
/*
	if(danger_point_type==1)
	{
		//
		// danger point is start of used prims
		//

		//
		// used prims point has just changed 
		//
		if(first_used_bucket_ram-200>danger_point)//the_display.CurrentPrim)
		{
			danger_point=first_used_bucket_ram-200;
			// same type
		}
		else
		{
			ASSESRT(0);

			if(the_display.CurrentPrim<first_used_bucket_ram-200)
			{
				danger_point=first_used_bucket_ram-200;

			}
			else
			{
				danger_point=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-200; // 200 hundred from end
				danger_point_type=0;
			}
		}

	}
*/

//	ASSERT(0);
	if(first_used_bucket_ram-200>the_display.CurrentPrim)
	{
		//
		// If theres some data between me and the end of the file then make that data point the danger zone
		//
		danger_point=first_used_bucket_ram-200;
		danger_point_type=1;
	}
	else
	{
		//
		// otherwise we are fine to the end of the file
		//
		danger_point=the_display.CurrentDisplayBuffer->PrimMem+BUCKET_MEM-200; // 200 hundred from end
		danger_point_type=0;
	}
	
	


	check_prim_ptr((void**)&the_display.CurrentPrim);

//	ASSERT(danger_point>the_display.CurrentPrim);

//	FntFlush(-1);

}

extern UBYTE Wadmenu_PadType;

void AENG_flip2(ULONG *back_image)
{
	DB	*old_buffer;

	ReadInputDevice();
	DrawSync(0);
	VSync(2);		/* wait for V-BLNK (2/60) */

	Wadmenu_PadType=PadInfoMode(0,InfoModeCurID,0);

	old_buffer=the_display.CurrentDisplayBuffer;
	if(the_display.CurrentDisplayBuffer==&the_display.DisplayBuffers[0])
   		the_display.CurrentDisplayBuffer=&the_display.DisplayBuffers[1];
	else
   		the_display.CurrentDisplayBuffer=&the_display.DisplayBuffers[0];
	PutDispEnv(&the_display.CurrentDisplayBuffer->Disp); 
	PutDrawEnv(&the_display.CurrentDisplayBuffer->Draw); 

	LoadImage(&the_display.CurrentDisplayBuffer->Draw.clip,back_image);
	DrawSync(0);
	DrawOTag(old_buffer->ot);	//draw the bucketlist
	DrawSync(0);

	ClearOTag(old_buffer->ot,OTSIZE);

	the_display.CurrentPrim=the_display.CurrentDisplayBuffer->PrimMem;
	last_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem-200;
	first_used_bucket_ram=the_display.CurrentDisplayBuffer->PrimMem-200;

//	FntFlush(-1);

}

extern void SHAPE_sparky_line(
		SLONG num_points,
		SLONG px[],
		SLONG py[],
		SLONG pz[],
		ULONG colour,
		SLONG width);

void AENG_draw_sparks()
{
	SLONG z;
	

	SPARK_Info   *si;
//	GLITTER_Info *gi;

	for (z = NGAMUT_point_zmin; z <= NGAMUT_point_zmax; z++)
	{
		SPARK_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);

		while(si = SPARK_get_next())
		{
			SHAPE_sparky_line(
				si->num_points,
				si->x,
				si->y,
				si->z,
				si->colour,
				si->size);
		}
/*
		GLITTER_get_start(
			NGAMUT_point_gamut[z].xmin,
			NGAMUT_point_gamut[z].xmax,
			z);
		
		while(gi = GLITTER_get_next())
		{
			SHAPE_glitter(
				gi->x1,
				gi->y1,
				gi->z1,
				gi->x2,
				gi->y2,
				gi->z2,
				gi->colour);
		}
*/
	}

}

void	AENG_draw_inside_floor(UWORD inside_index,UWORD inside_room,UBYTE fade);

/*
#define SKY_MAX_STARS 256

typedef struct {
	SVECTOR pos;
	CVECTOR col;
} STAR_Data;

STAR_Data Star[SKY_MAX_STARS];
*/

SVECTOR sky_range[64][9];

void SKY_init(CBYTE *star_file)
{
	UWORD i,j;
	SVECTOR pos;
	VECTOR res;
	MATRIX m;
	SVECTOR rot;
	pos.vx=0;
	pos.vy=0;
	pos.vz=32767;

	for(i=0;i<64;i++)
	{
		for(j=0;j<9;j++)
		{
			rot.vx=(-j)<<6;
			rot.vy=2048-(i<<6);
			rot.vz=0;
			SetRotMatrix(RotMatrixYXZ(&rot,&m));
			ApplyRotMatrix(&pos,&res);
			sky_range[i][j].vx=res.vx;
			sky_range[i][j].vy=res.vy-2048;
			sky_range[i][j].vz=res.vz;
		}
	}
	/*
	Star[0].pos.vx=rcos(0)<<3;
	Star[0].pos.vz=rsin(0)<<3;
	Star[0].pos.vy=rcos(768)<<3;
	for(i=1;i<SKY_MAX_STARS;i++)
	{
		Star[i].pos.vx=rcos(rand()%4096)<<3;
		Star[i].pos.vz=rsin(rand()%4096)<<3;
		Star[i].pos.vy=rcos((rand()%2048)-1024)<<3;
		Star[i].col.r=Star[i].col.g=Star[i].col.b=128+(rand()%128);
	}
	*/
}

void SKY_draw_bowl(void)
{
	int angle;
	PSX_POLY_Point *pp=perm_pp_array;
	POLY_FT4* p;
	VECTOR v;
	MATRIX m;
//	SLONG draw_from=0;
	SWORD	and_flag_y,and_flag_x;
	int i,j;

	angle=((AENG_cam_yaw&2016)-192)&2047;
	angle/=32;
	PushMatrix();
	v.vx=0;
	v.vy=AENG_cam_y<<1;
	v.vz=0;
	
//	if (AENG_cam_y<768)
//		draw_from=2;

	SetTransMatrix(TransMatrix(&m,&v));
	for(i=0;i<12;i++)
	{
		for(j=0;j<8;j++)
		{
			gte_RotTransPers3(&sky_range[angle][j+1],&sky_range[(angle+1)&63][j+1],&sky_range[angle][j],
							  &pp[0].SYSX,&pp[1].SYSX,&pp[2].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);
			gte_RotTransPers(&sky_range[(angle+1)&63][j],&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);


			//
			// could do this in one SLONG using SYSX
			//
			and_flag_y=pp[0].Word.SY&pp[1].Word.SY&pp[2].Word.SY&pp[3].Word.SY;

			if(!(and_flag_y&((1<<15))))
			{

				and_flag_x=pp[0].Word.SX&pp[1].Word.SX&pp[2].Word.SX&pp[3].Word.SX;
				if(!(and_flag_x&((1<<15)|(1<<9))))
				{
					ALLOCPRIM(p,POLY_FT4);
					setPolyFT4(p);
		//			setSemiTrans(p,1);
					setXY4(p,pp[0].Word.SX,pp[0].Word.SY,
							 pp[1].Word.SX,pp[1].Word.SY,
							 pp[2].Word.SX,pp[2].Word.SY,
							 pp[3].Word.SX,pp[3].Word.SY);
					p->tpage=getPSXTPageS(0);
					p->clut=getPSXClutS((angle&7)+((7-j)<<3));
					setUVWH(p,((angle&7)<<5),224-(j<<5),31,31);
  
					setRGB0(p,128,128,128);
		//			setRGB0(p,64,64,64);
		//			setRGB0(p,NIGHT_sky_colour.red<<1,NIGHT_sky_colour.green<<1,NIGHT_sky_colour.blue<<1);
					DOPRIM(0,p);
				}
			}
		}
		angle=(angle+1)&63;
	}
	PopMatrix();
}

/*
void SKY_draw_stars(void)
{
	SLONG i,pf,flag,z;
	DVECTOR xy;
	LINE_F2 *p;
	POLY_FT4 *p2;

	p2=(POLY_FT4*)the_display.CurrentPrim;
	gte_RotTransPers(&Star[0].pos,&xy,&pf,&flag,&z)
	if ((z>0)&&(xy.vx>-32)&&(xy.vx<544)&&(xy.vy>=-32)&&(xy.vy<288))
	{
		setPolyFT4(p2);
		setXYWH(p2,xy.vx-32,xy.vy-32,102,64);
		setUVWH(p2,0,0,64,64);
		p2->tpage=getPSXTPage(POLY_PAGE_MOON);
		p2->clut=getPSXClut(POLY_PAGE_MOON);
		setRGB0(p2,128,128,128);
		DOPRIM(0,p2);
		p2++;
	}
	p=(LINE_F2*)p2;
	for(i=1;i<SKY_MAX_STARS;i++)
	{
		gte_RotTransPers(&Star[i].pos,&xy,&pf,&flag,&z);
		if ((z>0)&&(xy.vx>=0)&&(xy.vx<512)&&(xy.vy>=0)&&(xy.vy<256))
		{
			setLineF2(p);
			setXY2(p,xy.vx,xy.vy,xy.vx,xy.vy);
			setRGB0(p,Star[i].col.r,Star[i].col.g,Star[i].col.b);
			DOPRIM(0,p);
			p++;
		}
	}
	the_display.CurrentPrim=(UBYTE *)p;
}


void AENG_draw_bike(Thing *p_thing)
{
	ASSERT(p_thing->Class == CLASS_BIKE);

	{	
		//
		// Nasty eh! But I can't be arsed to create a new drawtype.
		// 
		
		BIKE_Drawinfo bdi = BIKE_get_drawinfo(p_thing);

		//
		// Draw the frame of the bike.
		//

		ANIM_obj_draw(p_thing, p_thing->Draw.Tweened);

		//
		// If the bike is parked or being mounted then the wheels are
		// included in the animating object.
		//

		if (p_thing->Genus.Bike->mode == BIKE_MODE_DRIVING)
		{
		//	AENG_set_bike_wheel_rotation(bdi.front_rot);

			MESH_draw_poly(
					PRIM_OBJ_BIKE_BWHEEL,
					bdi.front_x,
					bdi.front_y,
					bdi.front_z,
					bdi.steer,
					bdi.pitch,
					bdi.roll,
					NULL,0);

		//	AENG_set_bike_wheel_rotation(bdi.back_rot);

			MESH_draw_poly(
					PRIM_OBJ_BIKE_BWHEEL,
					bdi.back_x,
					bdi.back_y,
					bdi.back_z,
					bdi.yaw,
					0,
					bdi.roll,
					NULL,0);
		}

		// Now some bike fx... first the exhaust
		PARTICLE_Exhaust2(p_thing, 5, 16);

		if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
		{
			SLONG matrix[9], vector[3], dx,dy,dz;
			FMATRIX_calc(matrix, bdi.steer, bdi.pitch, bdi.roll);
			FMATRIX_TRANSPOSE(matrix);
			vector[2]=-255; vector[1]=0; vector[0]=0; 
			FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
			dx=vector[0]; dy=vector[1]; dz=vector[2];
			vector[2]=25; vector[1]=80; vector[0]=0; 
			FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
			BLOOM_draw(bdi.front_x+vector[0],bdi.front_y+vector[1],bdi.front_z+vector[2],dx,dy,dz,0x606040,BLOOM_LENSFLARE|BLOOM_BEAM);

			FMATRIX_calc(matrix, bdi.yaw, bdi.pitch, bdi.roll);
			FMATRIX_TRANSPOSE(matrix);
			vector[2]=255; vector[1]=0; vector[0]=0;
			FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);
			dx=vector[0]; dy=vector[1]; dz=vector[2];
			vector[2]=70; vector[1]=75; vector[0]=0;
			FMATRIX_MUL(matrix,vector[0],vector[1],vector[2]);

			BLOOM_draw(
				(p_thing->WorldPos.X >> 8)+vector[0],
				(p_thing->WorldPos.Y >> 8)+vector[1],
				(p_thing->WorldPos.Z >> 8)+vector[2],
				dx,dy,dz,0x800000,0);

		}
	}

}

void AENG_ride_bike(Thing *p_thing)
{
	Thing *p_bike = TO_THING(p_thing->Genus.Person->InCar);

	ASSERT(p_thing->Genus.Person->Flags & FLAG_PERSON_BIKING);
	ASSERT(p_thing->Genus.Person->InCar);

	BIKE_Drawinfo bdi = BIKE_get_drawinfo(p_bike);

	//
	// Move to the same position above the bike.
	//

	GameCoord newpos = p_bike->WorldPos;

	p_thing->Draw.Tweened->Angle = bdi.yaw;
	p_thing->Draw.Tweened->Tilt  = bdi.pitch;
	p_thing->Draw.Tweened->Roll  = bdi.roll;

	{
		SLONG roll = bdi.roll;

		if (roll > 1024)
		{
			roll -= 2048;
		}

		roll /= 2;
		roll &= 2047;

		p_thing->Draw.Tweened->Roll = roll;
	}

	{
		BIKE_Control bc;
		DrawTween   *dt = p_thing->Draw.Tweened;

		bc = BIKE_control_get(p_bike);

		if (bc.steer == 0)
		{
			dt->CurrentFrame = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
			dt->NextFrame    = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
		}
		else
		if (bc.steer < 0)
		{
			dt->CurrentFrame =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
			dt->NextFrame    =  global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_RIGHT];
			dt->AnimTween    = -bc.steer << 3;
		}
		else
		{
			dt->CurrentFrame = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN];
			dt->NextFrame    = global_anim_array[p_thing->Genus.Person->AnimType][ANIM_BIKE_LEAN_LEFT];
			dt->AnimTween    = bc.steer << 3;
		}
	}

	{
		GameCoord oldpos = p_thing->WorldPos;

		p_thing->WorldPos = newpos;

		FIGURE   _draw(p_thing);

		p_thing->WorldPos = oldpos;
	}
}
*/

void	AENG_draw_tripwires()
{
	SLONG map_x1;
	SLONG map_z1;

	SLONG map_x2;
	SLONG map_z2;

	TRIP_Info *ti;

	TRIP_get_start();

	while(ti = TRIP_get_next())
	{
		//
		// Check whether this tripwire is on the map.
		//

		map_x1 = ti->x1 >> 8;
		map_z1 = ti->z1 >> 8;

		map_x2 = ti->x2 >> 8;
		map_z2 = ti->z2 >> 8;

		if ((WITHIN(map_z1, NGAMUT_zmin-2, NGAMUT_zmax+2) && WITHIN(map_x1, NGAMUT_gamut[map_z1].xmin-2, NGAMUT_gamut[map_z1].xmax+2)) ||
 			(WITHIN(map_z2, NGAMUT_zmin-2, NGAMUT_zmax+2) && WITHIN(map_x2, NGAMUT_gamut[map_z2].xmin-2, NGAMUT_gamut[map_z2].xmax+2)))
		{
			//
			// Draw the bugger.
			//

			#define AENG_TRIPWIRE_WIDTH 0x3

			SHAPE_tripwire(
				ti->x1,
				ti->y,
				ti->z1,
				ti->x2,
				ti->y,
				ti->z2,
				AENG_TRIPWIRE_WIDTH,
				0x00660000,
				ti->counter,
				ti->along);
		}
	}
}

void AENG_dfcache_clean()
{
	SLONG dfcache;
	SLONG next;
//	UBYTE lastturn = GAME_TURN - 1;

	NIGHT_Dfcache *ndf;

	for (dfcache = NIGHT_dfcache_used; dfcache; dfcache = next)
	{
		ASSERT(WITHIN(dfcache, 1, NIGHT_MAX_DFCACHES - 1));

		ndf  = &NIGHT_dfcache[dfcache];
		next = 	ndf->next;

		//
		// Was this facet drawn last gameturn? If it wasn't then
		// free up the cached lighting info for it.
		//

		ASSERT(WITHIN(ndf->dfacet, 1, next_dfacet - 1));
		ASSERT(dfacets[ndf->dfacet].Dfcache == dfcache);

		if (dfacets[ndf->dfacet].Counter[0] != SUPERMAP_counter[0])
		{
			//
			// Free up the lighting info.
			//

			dfacets[ndf->dfacet].Dfcache = 0;

			NIGHT_dfcache_destroy(dfcache);
		}
	}
}


void	fiddle_draw_distance(void)
{
extern	SLONG	tick_tock_unclipped;
	if(tick_tock_unclipped)
	{
		if((1000/tick_tock_unclipped)<13)
		{
			my_draw_dist-=64;
			remove_dead_people=1;
		}
		else
		if((1000/tick_tock_unclipped)>17)
		{
			my_draw_dist+=64;
		}
		SATURATE(my_draw_dist,(10<<8)+128,(20<<8)+128);
	}
}

void AENG_draw_warehouse(SLONG info)
{
	SLONG i;

	SLONG x;
	SLONG z;

//	MapElement *me;

	THING_INDEX t_index;
	Thing      *p_thing;
	SLONG	temp_draw_dist;

	//
	// Create the gamut
	//

	fiddle_draw_distance();

	
	temp_draw_dist=my_draw_dist-256;
	SATURATE(temp_draw_dist,(10<<8)+128,(16<<8)+128);

	SetFogNearFar(temp_draw_dist-(temp_draw_dist>>2),temp_draw_dist,420);
	AENG_calc_gamut(temp_draw_dist>>8);
/*
	{
//		static SLONG near=810,far=22000;
		static SLONG near=3400,far=4200;

		SetFogNearFar(near,far,420);
	}
*/
	AENG_calc_gamut(temp_draw_dist>>8);//AENG_DRAW_DIST-4);

	remove_dead_people=0;

	// Since we only have one buffer we must wait for it to finish drawing.

#ifdef	OLD_FLIP
	DrawSync(0);
#endif
	// And only one ordering table, so clear it now.
	ClearOTag(the_display.CurrentDisplayBuffer->ot, OTSIZE);

	AENG_draw_ware_floor();
//	AENG_draw_ware_roof();
//	AENG_draw_dirt();
	DrawGrenades();
	AENG_draw_sparks();
	AENG_draw_pows();
	//
	// Points out of the ambient light.
	//
	SUPERMAP_counter_increase(0);

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)

		{
			OB_Info *oi;
				//
				// The objects on this mapsquare.
				//

				oi = OB_find(x,z);

				while(oi->prim)
				{
 					if (oi->flags & OB_FLAG_WAREHOUSE)
					{

							SLONG	dist,dx,dz;
							dx=(oi->x)-POLY_cam_x;
							dz=(oi->z)-POLY_cam_z;
							dist=dx*dx+dz*dz;

							if((dist<256*12*256*12))
							{
								
								MESH_draw_poly(
									oi->prim,
									oi->x,
									oi->y,
									oi->z,
									oi->yaw,
									oi->pitch,
									oi->roll,
									0,
									0);
							}


//						if ((oi->prim=133)||(prim_objects[oi->prim].flag & PRIM_FLAG_ITEM))
//						if (prim_objects[oi->prim].flag & PRIM_FLAG_ITEM)
//						{
//							oi->yaw+=1;
//							OB_ob[oi->index].yaw += 1;
//						}
					}
					oi += 1;
				}

//			me = &MAP[MAP_INDEX(x, z)];
							//
				// Look at the colvects on this square.
				//

				{
					SLONG f_list;
					SLONG facet;
					SLONG build;
					SLONG exit = FALSE;

					f_list = PAP_2LO(x,z).ColVectHead;


					if (f_list)
					{
						ASSERT(f_list>0 && f_list<next_facet_link);
						while(!exit)
						{
							facet=facet_links[f_list];

							DFacet *df;

							ASSERT(facet);

							if (facet < 0)
							{
								//
								// The last facet in the list for each square
								// is negative.
								// 

								facet = -facet;
								exit  =  TRUE;
							}
		 					ASSERT(facet>0 && facet<next_dfacet);


							//
							// Has this facet's building been processed this
							// gameturn yet?
							// 

							df = &dfacets[facet];

							if (df->Counter[0] != SUPERMAP_counter[0])
							{
								build = df->Building;

								if (dbuildings[build].Type == BUILDING_TYPE_WAREHOUSE ||
									dbuildings[build].Type == BUILDING_TYPE_CRATE_IN)
								{
									//
									// Draw the facet.
									// 

									FACET_draw(facet,0);
									debug_count[0]++;

									if (df->FacetType == STOREY_TYPE_NORMAL)
									{
										build = df->Building;

										if (build)
										{
											if (dbuildings[build].Counter[0] != SUPERMAP_counter[0])
											{
												//
												// Draw all the walkable faces for this building.
												//

												FACET_draw_walkable(build);
												//debug_count[1]++;

												//
												// Mark the buiding as procesed this gameturn.
												//

												dbuildings[build].Counter[0] = SUPERMAP_counter[0];
											}
										}
									}
								}

								//
								// Mark this facet as drawn this gameturn already.
								//

								dfacets[facet].Counter[0] = SUPERMAP_counter[0];
							}


							f_list++;
						}
					}
				}
			

			t_index = PAP_2LO(x,z).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);
				p_thing->Flags|=FLAGS_IN_VIEW;

 extern	ULONG	MESH_colour_and;

					MESH_colour_and=(t_index+1) & 7;

					switch(p_thing->DrawType)
					{
						case DT_NONE:
							break;

						case DT_BUILDING:

							break;

						case DT_PRIM:
							break;

						case DT_ROT_MULTI:
															
							ASSERT(p_thing->Class == CLASS_PERSON);

							//
							// If this person is riding the bike...
							//

							if (!p_thing->Genus.Person->Ware)
								break;
/*					
							if (p_thing->SubState == SUB_STATE_RIDING_BIKE)
							{
								AENG_ride_bike(p_thing);
							}
							else
*/
							{
								SLONG	dist,dx,dz;
								dx=(p_thing->WorldPos.X>>8)-POLY_cam_x;
								dz=(p_thing->WorldPos.Z>>8)-POLY_cam_z;
								dist=dx*dx+dz*dz;

								if(dist<256*8*256*8)
								{
//									FIGURE_draw_queued(p_thing,dist);
									FIGURE_draw_queued(p_thing,dist);
//									if(dist<256*7*256*7)
//  									AENG_draw_shadow(p_thing);
								}

							}

							if (p_thing->State == STATE_DEAD)
								{
									if (p_thing->Genus.Person->Timer1 > 10)
									{
										if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
											p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
											p_thing->Genus.Person->PersonType == PERSON_MIB3)
										{
											//
											// Dead MIB self destruct!
											//
											DRAWXTRA_MIB_destruct(p_thing);
										}
									}
								}
#ifdef WERE_GOING_TO_STUPIDLY_STICK_THE_FINAL_BANE_INSIDE
								if (p_thing->Genus.Person->pcom_ai == PCOM_AI_BANE)
								{
									DRAWXTRA_final_glow(
										p_thing->WorldPos.X          >> 8,
										p_thing->WorldPos.Y + 0x8000 >> 8,
										p_thing->WorldPos.Z          >> 8,
									   -p_thing->Draw.Tweened->Tilt);
								}
#endif

							break;

						case DT_EFFECT:
							break;

						case DT_MESH:
							// a,b,c
							// a,c,b
							// c,a,b //
							// c,b,a
							// b,a,c
							// b,c,a

							if (p_thing->Class == CLASS_SPECIAL) DRAWXTRA_Special(p_thing);
							MESH_draw_poly(
									p_thing->Draw.Mesh->ObjectId,
									p_thing->WorldPos.X >> 8 ,
									p_thing->WorldPos.Y >> 8 ,
									p_thing->WorldPos.Z >> 8 ,
									p_thing->Draw.Mesh->Angle,
									p_thing->Draw.Mesh->Tilt,
									p_thing->Draw.Mesh->Roll,
									NULL,0);
														
							break;
//						case DT_BIKE:
//							AENG_draw_bike(p_thing);
//								break;

						case DT_VEHICLE:
							break;
						case DT_CHOPPER:
							break;
						case DT_ANIM_PRIM:
							{
//								SLONG	dist,dx,dz;
//								dx=(p_thing->WorldPos.X>>8)-POLY_cam_x;
//								dz=(p_thing->WorldPos.Z>>8)-POLY_cam_z;
//								dist=dx*dx+dz*dz;

//								if(dist<256*8*256*8)
									ANIM_obj_draw(p_thing,p_thing->Draw.Tweened);
							}
							break;
						case DT_PYRO:
							PYRO_draw_pyro(p_thing);
							break;
						case DT_ANIMAL_PRIM:
							break;
						case DT_TRACK:
							TRACKS_DrawTrack(p_thing);
							break;

						default:
//							printf("Drawing object %d\n",p_thing->DrawType);
							ASSERT(0);
							break;
					}

				t_index = p_thing->Child;
			}

		}

	}
	PARTICLE_Draw();
	AENG_draw_tripwires();
//	DoFigureDraw();
	DrawSync(0);

	AENG_dfcache_clean();
 	//
	// Draw the tripwires.
	//

//	PANEL_render_timer();

}

VECTOR temp;
DVECTOR sysx;


void	debug_lines(void)
{
	return;
#ifndef	NDEBUG
#ifdef	MIKE
	SLONG	x1,x2;

	// debug stuff

	x1=last_used_bucket_ram-the_display.CurrentDisplayBuffer->PrimMem;; //started filling this gameturn
	x2=the_display.CurrentPrim-the_display.CurrentDisplayBuffer->PrimMem; //finished this gameturn

	x1=10+((x1*300)/BUCKET_MEM);
	x2=10+((x2*300)/BUCKET_MEM);

void	quick_rect(SLONG x,SLONG y,SLONG w,SLONG h,SLONG r,SLONG g,SLONG b);


//
// red for this gameturn
//
	if(x1<x2)
	{
		quick_rect(x1,10,x2,18,255,0,0);
	}
	else
	{
		quick_rect(x1,10,310,18,255,0,0);
		quick_rect(10,10,x2,18,255,0,0);
	}


	//
	// show previous game turns stuff in green
	//
	x2=last_used_bucket_ram-the_display.CurrentDisplayBuffer->PrimMem;; //started filling this gameturn
	x1=first_used_bucket_ram-the_display.CurrentDisplayBuffer->PrimMem; //finished this gameturn

	x1=10+((x1*300)/BUCKET_MEM);
	x2=10+((x2*300)/BUCKET_MEM);



	if(x1==x2)
	{
		quick_rect(0,10,320,18,0,0,255);
	}
	else
	if(x1<x2)
	{
		quick_rect(x1,10,x2,18,0,255,0);
	}
	else
	{
		quick_rect(x1,10,310,18,0,255,0);
		quick_rect(10,10,x2,18,0,255,0);
	}

	if(sync_point)
	{
		x1=sync_point-the_display.CurrentDisplayBuffer->PrimMem; //finished this gameturn

		x1=10+((x1*300)/BUCKET_MEM);

		quick_rect(x1,10,x1+3,58,255,255,255);
	}

		x1=danger_point-the_display.CurrentDisplayBuffer->PrimMem; //finished this gameturn

		x1=10+((x1*300)/BUCKET_MEM);

		
		quick_rect(x1,10,x1+3,158,255,255,danger_point_type==0?0:255);

	

	sync_point=0;
#endif
#endif
}


void AENG_draw_city(SLONG info)
{
//	SLONG i;

	SLONG x;
	SLONG z;

//	MapElement *me;

	THING_INDEX t_index;
	Thing      *p_thing;

//	SLONG p,flag,Z,r;

	PrimInfo *pi;
	SLONG	supermap_counter;
	SLONG	max_x;
	SLONG	max_y=NGAMUT_Ymax;
	ULONG	incar=0;

	remove_dead_people=0;

	fiddle_draw_distance();
	if (NET_PERSON(0)->Genus.Person->InCar)
	{
//		ASSERT(0);
		incar=1;
	}
//	if (NET_PERSON(0)->Genus.Person->Mode==PERSON_MODE_FIGHT)
//		AENG_DRAW_DIST=10;
//	else
#ifdef	DRAW_FLOOR_FURTHER
	AENG_DRAW_DIST=22;//15;
#else
	AENG_DRAW_DIST=18;
#endif
	if(0)
	{
		SLONG near,far;
		far=(AENG_DRAW_DIST+2)*256;
		near=far-(AENG_DRAW_DIST<<6);

//		far=20000;
//		near=10000;

		SetFogNearFar(near,far,420);
		AENG_calc_gamut(AENG_DRAW_DIST+2);
	}

	SetFogNearFar(my_draw_dist-(my_draw_dist>>2),my_draw_dist,420);
	AENG_calc_gamut(my_draw_dist>>8);

extern	void NIGHT_lum_init();
extern void NIGHT_dlight_render();

	NIGHT_dlight_render();  

	// Since we only have one buffer we must wait for it to finish drawing.
#ifdef	OLD_FLIP

	if(double_ot==0)
	{
		DrawSync(0);
	}
#endif
		// And only one ordering table, so clear it now.
	ClearOTag(the_display.CurrentDisplayBuffer->ot, OTSIZE);
//	AENG_calc_gamut(AENG_DRAW_DIST);//+4);

	AENG_draw_floor();
/*
	{
		SLONG near,far;
		far=(AENG_DRAW_DIST)*256;
		near=far-(AENG_DRAW_DIST<<6);

//		far=20000;
//		near=10000;

		SetFogNearFar(near,far,420);
	}
*/

//	debug_lines();
//	AENG_draw_floor();
//	AENG_draw_floor();
//	AENG_draw_floor();
//	AENG_draw_floor();
//	debug_lines();
//	AENG_dfcache_clean();
//	debug_lines();
//	return;


//	AENG_calc_gamut(AENG_DRAW_DIST+2);

#ifdef	DRAW_FLOOR_FURTHER
	{
		AENG_DRAW_DIST=13;

		{
			SLONG near,far;
			far=AENG_DRAW_DIST*256;
			near=far-(AENG_DRAW_DIST<<6);

			SetFogNearFar(near,far,420);
		}
		AENG_calc_gamut(AENG_DRAW_DIST);
	}

#endif

	AENG_draw_dirt();
	DrawGrenades();

	AENG_draw_sparks();
	AENG_draw_pows();

	//
	// Points out of the ambient light.
	//

	SUPERMAP_counter_increase(0);

	supermap_counter=SUPERMAP_counter[0];


#if 0
	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		TILE *p;
		ALLOCPRIM(p,TILE);
		setTile(p);
		setSemiTrans(p,1);
		setRGB0(p,0,128,0);
		setXY0(p,NGAMUT_lo_gamut[z].xmin<<3,z<<3);
		p->w=((NGAMUT_lo_gamut[z].xmax-NGAMUT_lo_gamut[z].xmin)+1)<<3;
		p->h=8;
		DOPRIM(PANEL_OTZ,p);
	}
#endif

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		max_x=NGAMUT_lo_gamut[z].xmax;
		for (x = NGAMUT_lo_gamut[z].xmin; x <= max_x; x++)

		{
			

				OB_Info *oi;

							//
				// The objects on this mapsquare.
				//

				oi = OB_find(x,z);
//#ifdef	WRAP				

//				if(0) //cb
				while(oi->prim)
				{
 					if (!(oi->flags & OB_FLAG_WAREHOUSE))
					{
//						if (POLY_sphere_visible(oi->x,oi->y,oi->z,get_prim_info(oi->prim)->radius))
						{
							//if(0)
							SLONG	dist,dx,dz;
							dx=(oi->x)-POLY_cam_x;
							dz=(oi->z)-POLY_cam_z;
							dist=dx*dx+dz*dz;

							if((dist<256*12*256*12)||incar)
							{
								
								MESH_draw_poly(
									oi->prim|(incar<<16),
									oi->x,
									oi->y,
									oi->z,
									oi->yaw,
									oi->pitch,
									oi->roll,
									0,
									0);
							}

//								if ((oi->prim=133)||(prim_objects[oi->prim].flag & PRIM_FLAG_ITEM))
//								{
//									oi->yaw += 1;
//									OB_ob[oi->index].yaw += 1;
//								}
							
						
	//							SHAPE_prim_shadow(oi);

								if (prim_objects[oi->prim].flag & PRIM_FLAG_GLARE)
								{
									SLONG	c8;
									//for(c8=0;c8<100;c8++);
									if(oi->prim==230)
										BLOOM_draw(oi->x,oi->y+48,oi->z, 0,0,0,0x808080,0);
									else
										BLOOM_draw(oi->x,oi->y,oi->z, 0,0,0,0x808080,0);
								}
/*
								else
								if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME)) 
								{
		
									switch (oi->prim) 
									{
									case 2:
										BLOOM_draw(oi->x+270,oi->y+350,oi->z, 0,-255,0,0x7f6500,BLOOM_BEAM);
										BLOOM_draw(oi->x-270,oi->y+350,oi->z, 0,-255,0,0x7f6500,BLOOM_BEAM);
										BLOOM_draw(oi->x,oi->y+350,oi->z+270, 0,-255,0,0x7f6500,BLOOM_BEAM);
										BLOOM_draw(oi->x,oi->y+350,oi->z-270, 0,-255,0,0x7f6500,BLOOM_BEAM);
										break;
									case 190:
										BLOOM_draw(oi->x,oi->y,oi->z, 0,0,0,0x808080,0);
									break;
									}
								}
*/
						}
					}
					oi += 1;
				}
//#endif

//	  			me = &MAP[MAP_INDEX(x, z)];

//				Rain effect, taken out 29/09/99

//				if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
//					SHAPE_droplet((x<<10)+(Random()&0x3ff),Random()&0x7ff,(z<<10)+(Random()&0x3ff),0,-64,0,0xff3f1f00,POLY_PAGE_DRIP);


				//
				// Look at the colvects on this square.
				//

				{
					SLONG f_list;
					SLONG facet;
					SLONG build;
					SLONG exit = FALSE;

					f_list = PAP_2LO(x,z).ColVectHead;


					if (f_list)
					{
						
//						ASSERT(f_list>0 && f_list<next_facet_link);
						while(!exit)
						{
							facet=facet_links[f_list];

							ASSERT(facet);

							if (facet < 0)
							{
								//
								// The last facet in the list for each square
								// is negative.
								// 

								facet = -facet;
								exit  =  TRUE;
							}
//		 					ASSERT(facet>0 && facet<next_dfacet);


							 		//
									// Has this facet's building been processed this
									// gameturn yet?
									// 

					
							if(dfacets[facet].Counter[0]!=supermap_counter)
							{
								//
								// Draw the facet.
								// 

								dfacets[facet].Counter[0] = supermap_counter;

								if(dfacets[facet].Y[0]>max_y && dfacets[facet].FacetType!=STOREY_TYPE_CABLE)
								{
									debug_count[6]++;
									dfacets[facet].FacetFlags|=FACET_FLAG_INVISIBLE;
								}
								else


								{


									if(dfacets[facet].FacetType==STOREY_TYPE_NORMAL)
										build=dfacets[facet].Building;
									else
										build=0;

 									if (build && dbuildings[build].Type == BUILDING_TYPE_CRATE_IN || (dfacets[facet].FacetFlags&FACET_FLAG_INSIDE))
									{
										//
										// Don't draw inside buildings outside.
										//
									}
									else
									{



	//									if(0)
										if(dfacets[facet].FacetFlags&FACET_FLAG_INVISIBLE)
										{
											if(EWAY_cam_jumped)
											{
												dfacets[facet].FacetFlags&=~FACET_FLAG_INVISIBLE;
												FACET_draw(facet,0);

											}
											else
											{

												if(GAME_TURN&1)
													dfacets[facet].FacetFlags&=~FACET_FLAG_INVISIBLE;
											}
	//										ASSERT(0);
										}
										else	
										{
											FACET_draw(facet,0);
//											FACET_draw(facet,0);
										}

										debug_count[2]++;



			
										if (build)
										{
											struct	DBuilding *p_building;


											p_building=&dbuildings[build];
											if (p_building->Counter[0] != supermap_counter)
											{
												//
												// Draw all the walkable faces for this building.
												//

												if(p_building->Walkable)
												{
													FACET_draw_walkable(build);
													debug_count[3]++;
												}

												dbuildings[build].Counter[0] = supermap_counter;
											}
										}
									}
								}

							}

							f_list++;
						}
					}
				}
			t_index = PAP_2LO(x,z).MapWho;
			while(t_index)
			{
				p_thing = TO_THING(t_index);
				p_thing->Flags|=FLAGS_IN_VIEW;

				if (p_thing->Flags & FLAGS_IN_BUILDING)
				{
					//
					// Dont draw things inside buildings when we are outdoors.
					//
				}
				else
				{
 extern	ULONG	MESH_colour_and;

					MESH_colour_and=(THING_NUMBER(p_thing)+1) & 7;

					switch(p_thing->DrawType)
					{
						case DT_NONE:
							break;

						case DT_BUILDING:

							//
							// Draw cables from their mapwho link. All other buildings are
							// drawn from their colvects.
							//
/*
							if (p_thing->Flags & FLAGS_CABLE_BUILDING)
							{
								BUILD_draw(p_thing);
							}
*/
							break;

						case DT_PRIM:
							break;

						case DT_ROT_MULTI:
							ASSERT(p_thing->Class == CLASS_PERSON);

							//
							// If this person is riding the bike...
							//
							if (p_thing->Genus.Person->Ware)
								break;
/*
							if (p_thing->SubState == SUB_STATE_RIDING_BIKE)
							{
								AENG_ride_bike(p_thing);
							}
*/
							{
								SLONG	dist,dx,dz;
								dx=(p_thing->WorldPos.X>>8)-POLY_cam_x;
								dz=(p_thing->WorldPos.Z>>8)-POLY_cam_z;
								dist=dx*dx+dz*dz;

								if(p_thing->Genus.Person->PlayerID||(dist<128*23*128*23))
								{
									FIGURE_draw_queued(p_thing,dist);
									if(dist<256*7*256*7)
										AENG_draw_shadow(p_thing);
								}
							}
							
							if (p_thing->State == STATE_DEAD)
							{
								if (p_thing->Genus.Person->Timer1 > 10)
								{
									if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
										p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
										p_thing->Genus.Person->PersonType == PERSON_MIB3)
									{
										//
										// Dead MIB self destruct!
										//
										DRAWXTRA_MIB_destruct(p_thing);
									}
								}
							}

							if (p_thing->Genus.Person->pcom_ai == PCOM_AI_BANE)
							{
								DRAWXTRA_final_glow(
									p_thing->WorldPos.X          >> 8,
									p_thing->WorldPos.Y + 0x8000 >> 8,
									p_thing->WorldPos.Z          >> 8,
								   -p_thing->Draw.Tweened->Tilt);
							}

							break;

						case DT_EFFECT:
							break;

						case DT_MESH:
							// a,b,c
							// a,c,b
							// c,a,b //
							// c,b,a
							// b,a,c
							// b,c,a
								if (p_thing->Class == CLASS_SPECIAL) 
									DRAWXTRA_Special(p_thing);
								
									//if(0)//cb
								MESH_draw_poly(
									p_thing->Draw.Mesh->ObjectId|(incar<<16),
									p_thing->WorldPos.X >> 8 ,
									p_thing->WorldPos.Y >> 8 ,
									p_thing->WorldPos.Z >> 8 ,
									p_thing->Draw.Mesh->Angle,
									p_thing->Draw.Mesh->Tilt,
									p_thing->Draw.Mesh->Roll,
									NULL,0);
							break;
//						case DT_BIKE:
//							AENG_draw_bike(p_thing);
//								break;

						case DT_VEHICLE:
extern	void	draw_car(Thing *p_car);
							if(p_thing->Class==CLASS_VEHICLE)
								{
									if(p_thing->Genus.Vehicle->Driver)
									{
										TO_THING(p_thing->Genus.Vehicle->Driver)->Flags|=FLAGS_IN_VIEW;

									}
								}
							draw_car(p_thing);
							break;
						case DT_CHOPPER:
							CHOPPER_draw_chopper(p_thing);
							break;
						case DT_ANIM_PRIM:
			
							{
								SLONG	dist,dx,dz;
								dx=(p_thing->WorldPos.X>>8)-POLY_cam_x;
								dz=(p_thing->WorldPos.Z>>8)-POLY_cam_z;
								dist=dx*dx+dz*dz;

//								if(dist<256*8*256*8)
									ANIM_obj_draw(p_thing,p_thing->Draw.Tweened);
							}
							break;
						case DT_PYRO:
							PYRO_draw_pyro(p_thing);
							break;
						case DT_ANIMAL_PRIM:
							break;
						case DT_TRACK:
							//if(!INDOORS_INDEX)
								TRACKS_DrawTrack(p_thing);
							break;
						default:
							ASSERT(0);
							break;
					}
				}

				t_index = p_thing->Child;
			}

		}

	}
//	PARTICLE_Draw(); //crashes!!!!
//	AENG_dfcache_clean();
//	debug_lines();



#ifdef	INSIDES_EXIST
extern	void	draw_insides(SLONG indoor_index,SLONG room,UBYTE fade);
	if(INDOORS_INDEX_NEXT)
	{
		draw_insides(INDOORS_INDEX_NEXT,INDOORS_INDEX_NEXT,INDOORS_INDEX_FADE);
		AENG_draw_inside_floor(INDOORS_INDEX_NEXT,INDOORS_ROOM_NEXT,INDOORS_INDEX_FADE);
	}
	if(INDOORS_INDEX)
	{
		draw_insides(INDOORS_INDEX,INDOORS_ROOM,255-INDOORS_INDEX_FADE);
		AENG_draw_inside_floor(INDOORS_INDEX,INDOORS_ROOM,255-INDOORS_INDEX_FADE);
	}
#endif
//	if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME))
//	{
		//
		// Draw the stars...
		//
	
		SKY_draw_bowl();

//	}

 	//
	// Draw the tripwires.
	//

	AENG_draw_tripwires();
#if 0
	AENG_world_line_render();
#endif

	PARTICLE_Draw(); //crashes!!!!
//	DoFigureDraw();
	DrawSync(0);
//	PANEL_render_timer();


#if 0

	AENG_ambient_editor(0);

#endif

//#define	DRAW_BLACK_FACETS	1
#ifdef	DRAW_BLACK_FACETS	

	AENG_calc_gamut(AENG_DRAW_DIST*3);   // just how far do we want to go with these black facets

	for (z = NGAMUT_lo_zmin; z <= NGAMUT_lo_zmax; z++)
	{
		for (x = NGAMUT_lo_gamut[z].xmin; x <= NGAMUT_lo_gamut[z].xmax; x++)
		{
			SLONG f_list;
			SLONG facet;
			SLONG build;
			SLONG exit = FALSE;

			f_list = PAP_2LO(x,z).ColVectHead;

			if (f_list)
			{
				ASSERT(f_list>0 && f_list<next_facet_link);
				while(!exit)
				{
					facet=facet_links[f_list];

					ASSERT(facet);

					if (facet < 0)
					{
						//
						// The last facet in the list for each square
						// is negative.
						// 

						facet = -facet;
						exit  =  TRUE;
					}


					//
					// Has this facet's building been processed this
					// gameturn yet?
					// 

			
					if(dfacets[facet].Counter[0]!=supermap_counter)
					{
						//
						// Draw the facet.
						// 
							
						dfacets[facet].Counter[0] = supermap_counter;


extern	void FACET_draw_quick(SLONG facet);

						FACET_draw_quick(facet);
						debug_count[4]++;

					}

					f_list++;
				}
			}
		}
	}
#endif

	AENG_dfcache_clean();
#ifndef	NDEBUG
	debug_lines();
#endif

	return;

}

#ifdef MIKE

void	quick_rect(SLONG x,SLONG y,SLONG x2,SLONG y2,SLONG r,SLONG g,SLONG b)
{
void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb);

	DRAW2D_Box_Page(x,y,x2,y2,(128<<24)+(r<<16)+(g<<8)+b);


}

void	show_debug_info(void)
{
	CBYTE	str[100];
	ULONG	size;
/*
	static	SLONG	pers=420;
		if (PadKeyIsPressed(&PAD_Input2,PAD_LU))
		{
			pers+=5;
		}
		if (PadKeyIsPressed(&PAD_Input2,PAD_LD))
		{
			pers-=5;
		}
		if(pers<10)
			pers=10;
	SetGeomScreen(pers);	
*/


extern	SLONG	pers_off;

	size=the_display.CurrentPrim-&(the_display.CurrentDisplayBuffer->PrimMem[0]);
//	sprintf(str,"%d %d FC %d",global_debug,draw_state,FC_cam[0].pitch>>8);
//	sprintf(str,"%d %d",global_debug,global_debug2);
//	FONT2D_DrawString(str,200,30);

	global_debug=0;
	global_debug2=0;
/*
	sprintf(str,"WFS %d DF %d F %d SF %d FQ %d I %d RFY %d",debug_count[0],debug_count[1],debug_count[2],debug_count[3],debug_count[4],debug_count[5],debug_count[6]);
	FONT2D_DrawString(str,10,10);
	sprintf(str,"mem %d ",size);
	FONT2D_DrawString(str,100,30);
	*/

/*
extern	SWORD	store_z[15];
	sprintf(str,"po %d P%d LF%d LT%d LF%d T%d ",pers_off,store_z[0],store_z[1],store_z[2],store_z[3],store_z[4]);
	FONT2D_DrawString(str,10,10);

	sprintf(str,"RH%d RR%d RH%d LH%d LR%d ",store_z[5],store_z[6],store_z[7],store_z[8],store_z[9]);
	FONT2D_DrawString(str,10,25);

	sprintf(str,"LH%d SK%d RF%d RT%d RF%d ",store_z[10],store_z[11],store_z[12],store_z[13],store_z[14]);
	FONT2D_DrawString(str,10,40);

	//FONT2D_DrawString(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
//	FONT2D_DrawString(str,10,10);
	//, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
*/

}
#endif


void AENG_draw(SLONG info)
{
	PSX_POLY_Point	holdpp[MAX_PP_ARRAY];
	perm_pp_array=holdpp;

extern	SLONG	tick_tock_unclipped;
	sea_offset+=(tick_tock_unclipped);

	Ive_Not_Been_Flipped++;

	memset(debug_count,0,20);

extern SLONG	psx_camera(void);
	// This should fix all those fucking useless warehouse camera problems in one fell swoop.
	// It sure does.
	if (psx_camera())
	{
		in_ware=1;
		AENG_draw_warehouse(info);
	}
	else
	{
		in_ware=0;
		AENG_draw_city(info);
	}
#ifdef MIKE
	show_debug_info();
#endif


	if(EWAY_cam_jumped)
		EWAY_cam_jumped--;


}

#ifdef INSIDES_EXIST
void	AENG_draw_inside_floor(UWORD inside_index,UWORD inside_room,UBYTE fade)
{
	SLONG	x,z;
//	SLONG page;

//	float world_x;
//	float world_y;
	SLONG floor_y;
	SLONG roof_y;
//	float world_z;

	PSX_POLY_Point *pp=perm_pp_array;
	MapElement *me;

//	PAP_Lo *pl;
	PAP_Hi *ph;

//	PSX_POLY_Point *quad[4];

	struct	InsideStorey	*p_inside;
	SLONG	in_width;
	UBYTE	*in_block;
	SLONG	min_z,max_z;
//	SLONG	c0;
	SLONG	floor_type;
	UBYTE   *current_prim;


	//
	// draw the internal walls
	//
//extern	void	draw_insides(SLONG indoor_index,SLONG room,UBYTE fade);
//	draw_insides(inside_index,inside_room,fade);
	
	p_inside=&inside_storeys[inside_index];

	floor_type=p_inside->TexType;

//	MSG_add("in room %d\n",INDOORS_ROOM);

	current_prim=the_display.CurrentPrim;

	floor_y=p_inside->StoreyY;
	roof_y=floor_y+256;

	min_z=MAX(NGAMUT_point_zmin,p_inside->MinZ);
	max_z=MIN(NGAMUT_point_zmax,p_inside->MaxZ);

	in_width=p_inside->MaxX-p_inside->MinX;

	in_block=&inside_block[p_inside->InsideBlock];

	for (z = min_z; z <= max_z; z++)
	{
		SLONG	min_x,max_x;
		SLONG	face_y;
		SLONG	col;
		min_x=MAX(NGAMUT_point_gamut[z].xmin,p_inside->MinX);
		max_x=MIN(NGAMUT_point_gamut[z].xmax,p_inside->MaxX);

		for (x = min_x;x<=max_x;x++)
		{
			ASSERT(WITHIN(x, 0, MAP_WIDTH  - 1));
			ASSERT(WITHIN(z, 0, MAP_HEIGHT - 1));

			me = &MAP[MAP_INDEX(x, z)];
			ph = &PAP_2HI(x,z);

			if ((PAP_2HI(x,z).Flags & (PAP_FLAG_HIDDEN)))
			{
				SLONG	room_id;

				room_id=in_block[(x-p_inside->MinX)+(z-p_inside->MinZ)*in_width]&(0xf|0x80|0x40);

				if(!(room_id&0xc0))
				{
					if(1||(room_id&0xf)==inside_room)
					{
						face_y=floor_y;
						col=0xffffff;
					}
					else
					{
						face_y=roof_y;
						col=0;

					}

//					col=col|( (fade&255)<<24);
					pp[0].World.vx=	(x<<ELE_SHIFT)-POLY_cam_x;
					pp[0].World.vy=	floor_y-POLY_cam_y;
					pp[0].World.vz=	(z<<ELE_SHIFT)-POLY_cam_z;
					gte_RotTransPers(&pp[0].World,&pp[0].SYSX,&pp[0].P,&pp[0].Flag,&pp[0].Z);

					pp[1].World.vx=	((x+1)<<ELE_SHIFT)-POLY_cam_x;
					pp[1].World.vy=	floor_y -POLY_cam_y;
					pp[1].World.vz=	((z)<<ELE_SHIFT)-POLY_cam_z;
					gte_RotTransPers(&pp[1].World,&pp[1].SYSX,&pp[1].P,&pp[1].Flag,&pp[1].Z);

					pp[2].World.vx=	((x)<<ELE_SHIFT)-POLY_cam_x;
					pp[2].World.vy=	floor_y -POLY_cam_y;
					pp[2].World.vz=	((z+1)<<ELE_SHIFT)-POLY_cam_z;
					gte_RotTransPers(&pp[2].World,&pp[2].SYSX,&pp[2].P,&pp[2].Flag,&pp[2].Z);

					pp[3].World.vx=	((x+1)<<ELE_SHIFT)-POLY_cam_x;
					pp[3].World.vy=	floor_y -POLY_cam_y;
					pp[3].World.vz=	((z+1)<<ELE_SHIFT)-POLY_cam_z;
					gte_RotTransPers(&pp[3].World,&pp[3].SYSX,&pp[3].P,&pp[3].Flag,&pp[3].Z);

					if(((pp[0].Flag|pp[1].Flag|pp[2].Flag|pp[3].Flag)&(1<<31))==0)
					{
						SLONG	flag,tflag,c0;
						flag=0xffffffff;
						for(c0=0;c0<4;c0++)
						{
							tflag=0;
							if(pp[c0].Word.SX<0)
								tflag|=POLY_CLIP_LEFT;
							if(pp[c0].Word.SX>=SCREEN_WIDTH)
								tflag|=POLY_CLIP_RIGHT;
							if(pp[c0].Word.SY<0)
								tflag|=POLY_CLIP_TOP;
							if(pp[c0].Word.SY>=SCREEN_HEIGHT)
								tflag|=POLY_CLIP_BOTTOM;
							flag&=tflag;
						}
						if(flag==0)
							add_inside_quad(&pp[0],inside_tex[floor_type][room_id-1]+START_PAGE_FOR_FLOOR*64,&current_prim,col,fade);
					}
				}
			}
		}
	}
	the_display.CurrentPrim=current_prim;

}
#endif


SLONG FONT_draw(SLONG x, SLONG y, CBYTE *fmt, ...)
{
}

void AENG_init(void)
{
	SKY_init(NULL);
}



//
//
//
//
//
//
//
//
//      .(4)



void AENG_calc_gamut(SLONG dist)
{
	SLONG width;
	SLONG height;
	SLONG depth;
	SLONG aspect;
	SLONG matrix[9];

	struct
	{
		SLONG	x;
		SLONG	y;
		SLONG	z;

	} cone[5];


					   
	MATRIX_calc(
		matrix,
		-AENG_cam_yaw,
		AENG_cam_pitch,
		0);

	//
	// The dimensions of the view pyramid.
	//

	width  = dist;
	height = dist<<16;
	depth  = dist;

#ifdef	VERSION_LORES
	width *= ((10*DisplayWidth)>>3);
#else
	width *= ((3*DisplayWidth)>>2);
#endif
	width /= DisplayHeight;

	width<<=16;
	width  /= AENG_LENS;
	height /= AENG_LENS;

	// width & height are not <<16


	//
	// Finds the points of the cone in world space
	//

	cone[3].x = cone[4].x = (AENG_cam_x<<8); // 256;
	cone[3].y = cone[4].y = (AENG_cam_y<<8); // 256;
	cone[3].z = cone[4].z = (AENG_cam_z<<8);// 256;

	cone[3].x += depth * matrix[6];
	cone[3].y += depth * matrix[7];
	cone[3].z += depth * matrix[8];

	//
	// cone[0] is the top left corner...
	//

	cone[0].x = cone[3].x + height * matrix[3];
	cone[0].y = cone[3].y + height * matrix[4];
	cone[0].z = cone[3].z + height * matrix[5];

	cone[0].x = cone[0].x - width *  matrix[0];
	cone[0].y = cone[0].y - width *  matrix[1];
	cone[0].z = cone[0].z - width *  matrix[2];

	//
	// cone[1] is the top right corner...
	//

	cone[1].x = cone[3].x + height * matrix[3];
	cone[1].y = cone[3].y + height * matrix[4];
	cone[1].z = cone[3].z + height * matrix[5];

	cone[1].x = cone[1].x + width *  matrix[0];
	cone[1].y = cone[1].y + width *  matrix[1];
	cone[1].z = cone[1].z + width *  matrix[2];

	//
	// cone[2] is the bottom right corner...
	//

	cone[2].x = cone[3].x - height * matrix[3];
	cone[2].y = cone[3].y - height * matrix[4];
	cone[2].z = cone[3].z - height * matrix[5];

	cone[2].x = cone[2].x + width *  matrix[0];
	cone[2].y = cone[2].y + width *  matrix[1];
	cone[2].z = cone[2].z + width *  matrix[2];

	//
	// cone[3] is the bottom left corner...
	//

	cone[3].x = cone[3].x - height * matrix[3];
	cone[3].y = cone[3].y - height * matrix[4];
	cone[3].z = cone[3].z - height * matrix[5];

	cone[3].x = cone[3].x - width *  matrix[0];
	cone[3].y = cone[3].y - width *  matrix[1];
	cone[3].z = cone[3].z - width *  matrix[2];

	//
	// Create the gamut.
	//

	NGAMUT_init();

	NGAMUT_add_line(cone[4].x>>16, cone[4].z>>16, cone[0].x>>16, cone[0].z>>16);
	NGAMUT_add_line(cone[4].x>>16, cone[4].z>>16, cone[1].x>>16, cone[1].z>>16);
	NGAMUT_add_line(cone[4].x>>16, cone[4].z>>16, cone[2].x>>16, cone[2].z>>16);
	NGAMUT_add_line(cone[4].x>>16, cone[4].z>>16, cone[3].x>>16, cone[3].z>>16);
											    		       			      
	NGAMUT_add_line(cone[0].x>>16, cone[0].z>>16, cone[1].x>>16, cone[1].z>>16);
	NGAMUT_add_line(cone[1].x>>16, cone[1].z>>16, cone[2].x>>16, cone[2].z>>16);
	NGAMUT_add_line(cone[2].x>>16, cone[2].z>>16, cone[3].x>>16, cone[3].z>>16);
	NGAMUT_add_line(cone[3].x>>16, cone[3].z>>16, cone[0].x>>16, cone[0].z>>16);
#ifdef	MIKE_INFO
	if(0)
	{

		POLY_F3	*p;
		POLY_F4	*p4;
		LINE_F2	*line;

				ALLOCPRIM(line,LINE_F2);
				setLineF2(line);
				setXY2(line,80+(0>>2),60+(0>>2),80+(320>>2),60+(0>>2));
				setRGB0(line,255,255,255);
				DOPRIM(700,line);
				ALLOCPRIM(line,LINE_F2);
				setLineF2(line);
				setXY2(line,80+(320>>2),60+(0>>2),80+(320>>2),60+(240>>2));
				setRGB0(line,255,255,255);
				DOPRIM(700,line);
				ALLOCPRIM(line,LINE_F2);
				setLineF2(line);
				setXY2(line,80+(320>>2),60+(240>>2),80+(0>>2),60+(240>>2));
				setRGB0(line,255,255,255);
				DOPRIM(700,line);
				ALLOCPRIM(line,LINE_F2);
				setLineF2(line);
				setXY2(line,80+(0>>2),60+(240>>2),80+(0>>2),60+(0>>2));
				setRGB0(line,255,255,255);
				DOPRIM(700,line);



//		p=(POLY_F3 *)the_display.CurrentPrim;
		ALLOCPRIM(p,POLY_F3);

		setPolyF3(p);
		setRGB0(p,0,32,0);
		setXY3(p,cone[4].x>>14,cone[4].z>>15,cone[3].x>>14,cone[3].z>>15,cone[2].x>>14,cone[2].z>>15);
//		the_display.CurrentPrim+=sizeof(POLY_F3);
		addPrim(&the_display.CurrentDisplayBuffer->ot[700],p);

//		p=(POLY_F3 *)the_display.CurrentPrim;
		ALLOCPRIM(p,POLY_F3);

		setPolyF3(p);
		setRGB0(p,0,48,0);
		setXY3(p,cone[4].x>>14,cone[4].z>>15,cone[1].x>>14,cone[1].z>>15,cone[0].x>>14,cone[0].z>>15);
//		the_display.CurrentPrim+=sizeof(POLY_F3);
		addPrim(&the_display.CurrentDisplayBuffer->ot[700],p);





//		p4=(POLY_F4 *)the_display.CurrentPrim;
		ALLOCPRIM(p4,POLY_F4);

		setPolyF4(p4);
		setRGB0(p4,64,64,64);
		setXYWH(p4,NGAMUT_xmin<<2,NGAMUT_zmin<<1,(NGAMUT_xmax-NGAMUT_xmin)<<2,(NGAMUT_zmax-NGAMUT_zmin)<<1);
//		the_display.CurrentPrim+=sizeof(POLY_F4);
		addPrim(&the_display.CurrentDisplayBuffer->ot[700],p4);
		if(0)
		{
			SLONG	x;
			for(x=0;x<128;x++)
			{
				if(NGAMUT_gamut2[x].zmin!=255)
				{
					ALLOCPRIM(line,LINE_F2);
					setLineF2(line);
					setXY2(line,((x<<2)),((NGAMUT_gamut2[x].zmax<<1)),((x<<2)),((NGAMUT_gamut2[x].zmin<<1)));
					setRGB0(line,255,255,255);
					DOPRIM(1010,line);
				}
				
			}
		}

	}
#endif
	{

		SLONG	c0;
		NGAMUT_Ymin=cone[0].y;
		NGAMUT_Ymax=NGAMUT_Ymin;

		for(c0=1;c0<5;c0++)
		{
			if(cone[c0].y>NGAMUT_Ymax)
				NGAMUT_Ymax=cone[c0].y;
			if(cone[c0].y<NGAMUT_Ymin)
				NGAMUT_Ymin=cone[c0].y;
		}
	}
	NGAMUT_Ymax>>=8;
	NGAMUT_Ymin>>=8;

	NGAMUT_calculate_point_gamut();
	NGAMUT_calculate_lo_gamut();
}

/*
void AENG_loadbar(SLONG percent)
{
	PANEL_draw_health_bar(406,228,percent);
	DrawSync(0);
	DrawOTag(the_display.CurrentDisplayBuffer->ot);
	AENG_flip();
}
*/


#ifndef FS_ISO9660

char scrn_mem[512*256*2];

void AENG_screen_shot(SLONG width)
{
	char *mem;
	int handle;
	char fname[32];
	static scrn_counter=0;

	mem=scrn_mem;
	ASSERT(mem!=0);
	StoreImage(&the_display.CurrentDisplayBuffer->Disp.disp,(unsigned long *)mem);
	sprintf(fname,"screenshots\\scrn%03d.raw",scrn_counter++);
	handle=PCcreat(fname,0);
	PCwrite(handle,mem,width*DisplayHeight*2);
	PCclose(handle);
}
#endif

#if 0
void AENG_world_line_infinite(
		SLONG ix1, SLONG iy1, SLONG iz1, SLONG iwidth1, ULONG colour1, 
		SLONG ix2, SLONG iy2, SLONG iz2, SLONG iwidth2, ULONG colour2,
		SLONG sort_to_front)
{
	/*
	float x1 = float(ix1);
	float y1 = float(iy1);
	float z1 = float(iz1);
	float w1 = float(iwidth1);
	float r1 = float((colour1 >> 16) & 0xff);
	float g1 = float((colour1 >>  8) & 0xff);
	float b1 = float((colour1 >>  0) & 0xff);

	float x2 = float(ix2);
	float y2 = float(iy2);
	float z2 = float(iz2);
	float w2 = float(iwidth2);
	float r2 = float((colour2 >> 16) & 0xff);
	float g2 = float((colour2 >>  8) & 0xff);
	float b2 = float((colour2 >>  0) & 0xff);

	float dx = x2 - x1;
	float dy = y2 - y1;
	float dz = z2 - z1;
	float dr = r2 - r1;
	float dg = g2 - g1;
	float db = b2 - b1;
	float dw = w2 - w1;

	float dist      = sqrt(dx*dx + dy*dy + dz*dz);
	float steps     = floor(dist * (1.0F / 1024.0F)) + 1.0F;
	float oversteps = 1.0F / steps;

	float x = x1;
	float y = y1;
	float z = z1;
	float w = w1;
	float r = r1;
	float g = g1;
	float b = b1;

	dx *= oversteps;
	dy *= oversteps;
	dz *= oversteps;
	dw *= oversteps;
	dr *= oversteps;
	dg *= oversteps;
	db *= oversteps;
	
	float f;

	for (f = 0.0F; f < steps; f += 1.0F)
	{
		colour1 = (SLONG(r     ) << 16) | (SLONG(g     ) << 8) | (SLONG(b     ) << 0);
		colour2 = (SLONG(r + dr) << 16) | (SLONG(g + dg) << 8) | (SLONG(b + db) << 0);

		AENG_world_line(
			SLONG(x),
			SLONG(y),
			SLONG(z),
			SLONG(w),
			colour1,
			SLONG(x + dx),
			SLONG(y + dy),
			SLONG(z + dz),
			SLONG(w + dw),
			colour2,
			sort_to_front);

		x += dx;
		y += dy;
		z += dz;
		w += dw;
		r += dr;
		g += dg;
		b += db;
	}
	*/
}
#endif

#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif
	FONT2D_DrawString(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0)
{
#ifndef VERSION_KANJI
	POLY_FT4 *p;
	CBYTE *m=chr;
	SLONG x0=x,y0=y;
	SLONG c,dec;

	SLONG cl=getPSXClutT(0);//POLY_PAGE_FONT2D);
	SLONG tp=getPSXTPageT(0);//POLY_PAGE_FONT2D);

//	if (the_display.CurrentPrim>&GDisp_Bucket[BUCKET_MEM-3120])
//		return;
//	p=(POLY_FT4*)the_display.CurrentPrim;
	ALLOCPRIM(p,POLY_FT4);
//	check_prim_ptr((void**)&p);
		//(POLY_FT4*)the_display.CurrentPrim;   

	while (*m)
	{
		switch(*m)
		{
		case '\t':
			x0=(x0+16)&0xff0;
			break;
		case 10:
		case 13:
			x0=x;
			y0+=(scale*10)>>8;
			break;
		case 32:
			x0+=(scale*6)>>8;
			break;
		default:
			c=*m-32;

			setPolyFT4(p);
			dec=(((SLONG)f_descend[c])*(scale+1))>>8;
			setXYWH(p,x0,y0+dec,(scale>>5),(scale>>5));
			setUVWH(p,(c&7)<<3,(c&0xf8),8,8);
			if (rgb&&((c<96)||(c>111)))
			{
				if (rgb&0xff000000)
					setSemiTrans(p,1);
				setRGB0(p,(rgb>>16)&0xff,(rgb>>8)&0xff,rgb&0xff);
			}
			else
				setRGB0(p,128,128,128);
			p->clut=cl;//getPSXClut(POLY_PAGE_FONT2D);
			p->tpage=tp;//getPSXTPage(POLY_PAGE_FONT2D);
			DOPRIM(PANEL_OTZ,p);
			p++;
			check_prim_ptr((void**)&p);
			x0+=((f_width[c]*scale)>>8)+1;
			break;
		}
/*		if (x0>=DISPLAYWIDTH)
		{
			x0=x;
			y0+=(8*scale)>>8;
		}*/
		m++;
	}
	the_display.CurrentPrim=(UBYTE*)p;
#else
#if 0
	SLONG Kanji_found=Kanji_FindString(chr);
	POLY_FT4 *p;
	SLONG width,ch,height;

	if ((SLONG)chr>0)
		Kanji_JunkString(Kanji_found);

	width=text_width(chr,0,&ch);
	height=text_height(chr,0,&ch);
	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setXYWH(p,x,y,(width*scale)>>8,(height*scale)>>8);
	setUVWH(p,0,16*Kanji_found,width-1,height-1);
	if (rgb&0xff000000)
		setSemiTrans(p,1);
	setRGB0(p,(rgb>>16)&0xff,(rgb>>8)&0xff,rgb&0xff);
	p->clut=getClut(Kanji_clut_x,Kanji_clut_y);
	p->tpage=getTPage(0,0,Kanji_buffer_x,Kanji_buffer_y);
	DOPRIM(PANEL_OTZ,p);
#else
	Kanji_string(x,y,(UWORD*)chr,rgb,scale);
#endif
#endif
	ASSERT(the_display.CurrentPrim-the_display.CurrentDisplayBuffer->PrimMem<BUCKET_MEM);
}

char FONT2D_WrapStr[128];

SLONG AENG_text_wrappoint=308;

SLONG FONT2D_DrawStringWrap(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0)
{
	int	w;
	int h=0;
	unsigned char *p,*p2;

	p2=(unsigned char*)chr;
	while(*p2)
	{
		w=x;
		p=(unsigned char*)FONT2D_WrapStr;

		while(*p2&&(*p2!='|')&&(w<=AENG_text_wrappoint))
		{
			if (*p2>31)
			{
				w+=f_width[*p2-32]+1;
//				printf("%d",f_width[*p2-32]+1);
			}
			*p++=*p2++;
		}

		if (*p2)
		{
			if (*p2!='|')
				while(*p2!=' ')
				{
					p--;
					p2--;
				}
			p2++;
		}
		*p=0;
		FONT2D_DrawString(FONT2D_WrapStr,x,y+h,rgb,scale,page,fade);
		h+=9;
	}
	return h+4;


}

FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade)
{
	PSX_POLY_Point pp;
	SLONG size;

	pp.World.vx=world_x-POLY_cam_x;
	pp.World.vy=world_y-POLY_cam_y;
	pp.World.vz=world_z-POLY_cam_z;

	gte_RotTransPers(&pp.World,&pp.SYSX,&pp.P,&pp.Flag,&pp.Z);

	size=(text_size<<9)/(420+pp.Z);

	FONT2D_DrawString(str,pp.Word.SX,pp.Word.SY,rgb,size,POLY_PAGE_FONT2D,fade);
}

void FONT2D_DrawStringCentred(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0)
{
	SLONG width,c;

	width=(text_width(chr,0,&c)*scale)>>8;
	FONT2D_DrawString(chr,x-(width>>1),y-6,rgb,scale,0,0);
}


void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb)
{
	POLY_F4 *p;


	ALLOCPRIM(p,POLY_F4);

	setPolyF4(p);

	setRGB0(p,(rgb>>16)&0xff,(rgb>>8)&0xff,rgb&0xff);
	setXY4(p,x,y,ox,y,x,oy,ox,oy);

	DOPRIM(PANEL_OTZ,p);

	if (rgb&0xff000000)
	{
		setSemiTrans(p,1);

		if (rgb>0)	// Using &7f instead of &ff, means that it's half&half.
		{
			DR_TPAGE *p2;
			ALLOCPRIM(p2,DR_TPAGE);
			setDrawTPage(p2,0,0,getTPage(0,2,0,0));
			DOPRIM(PANEL_OTZ,p2);
		}

	}

}

void CONSOLE_draw()
{
//	PANEL_new_text_process();
//	PANEL_new_text_draw();
}


void CONSOLE_font(CBYTE *fontpath, SLONG scale)
{
}

#define MAP_MAX_BEACON_COLOURS 6

extern ULONG MAP_beacon_colour[MAP_MAX_BEACON_COLOURS];

extern SLONG	analogue;

ImageInfo poly2d_image[]={
	{0,0,0,136,16},
	{0,0,16,196,16}
};

void POLY2D_TextImage(SLONG image,SLONG x,SLONG y,SLONG col)
{
	POLY_FT4 *p;

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setXYWH(p,x,y,poly2d_image[image].w,poly2d_image[image].h);
	setRGB0(p,(col>>16)&0xff,(col>>8)&0xff,col&0xff);
	if (col&0xff000000)
		setSemiTrans(p,1);
	setUVWH(p,poly2d_image[image].x,poly2d_image[image].y,poly2d_image[image].w,poly2d_image[image].h);
	p->tpage=getPSXTPageP(poly2d_image[image].page);
	p->clut=getPSXClutP(poly2d_image[image].page);
	DOPRIM(PANEL_OTZ,p);
}

void AENG_loading_bar_draw(SLONG percent)
{
	SLONG i;
	percent/=5;

	for(i=0;i<percent;i++)
	{
		DRAW2D_Box_Page((i*16)+1,217,(i*16)+15,232,(0x0d0d0d*i));
	}
}

void AENG_loading_bar(SLONG percent)
{
	AENG_loading_bar_draw(percent);
	AENG_flip();
	AENG_loading_bar_draw(percent);
	AENG_flip();
}