/*
** File: Panel.cpp
**
** Author: James Watson
**
** Copyright: 1999 Mucky Foot Productions Ltd
*/

#include "game.h"
#include "libpad.h"
#include "libapi.h"

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
#include "c:\fallen\headers\pcom.h"


#ifdef VERSION_USA
#define STR_CONT		STR_CROSS" Continue"
#define	STR_RETURN		STR_TRI" Return to Menu"
#define STR_REPLAY		STR_CROSS" Restart Level"
#endif
#ifdef VERSION_GERMAN
#define STR_CONT		STR_CROSS" Fortfahren"
#define	STR_RETURN		STR_TRI" Zurück zum Menü"
#define STR_REPLAY		STR_CROSS" Level neu starten"
#endif
#ifdef VERSION_FRENCH
#define STR_CONT		STR_CROSS" Continue"
#define	STR_RETURN		STR_TRI" Retourner au menu"
#define STR_REPLAY		STR_CROSS" Recommencer niveau"
#endif
#ifdef VERSION_ITALIAN
#define STR_CONT		STR_CROSS" Continua"
#define	STR_RETURN		STR_TRI" Ritorna al menu"
#define STR_REPLAY		STR_CROSS" Ricomincia livello"
#endif
#ifdef VERSION_SPANISH
#define STR_CONT		STR_CROSS" Seguir"
#define	STR_RETURN		STR_TRI" Volver al menú"
#define STR_REPLAY		STR_CROSS" Repetir nivel"
#endif
#ifdef VERSION_JAPAN
#define STR_CONT		STR_CROSS" Continue"
#define	STR_RETURN		STR_TRI" Return to Menu"
#define STR_REPLAY		STR_CROSS" Replay Level"
#endif
#ifdef VERSION_KOREA
#define STR_CONT		STR_CROSS" Continue"
#define	STR_RETURN		STR_TRI" Return to Menu"
#define STR_REPLAY		STR_CROSS" Replay Level"
#endif

#ifdef VERSION_ENGLISH
#define STR_CONT		STR_CROSS" Continue"
#define	STR_RETURN		STR_TRI" Return to Menu"
#define STR_REPLAY		STR_CROSS" Replay Level"
#endif

extern PSX_POLY_Point *perm_pp_array;

void PANEL_render_thugs();

#include "psxeng.h"

#include "panel.h"
#include "xlat_str.h"

extern SLONG FONT2D_DrawStringWrap(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=256, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);

char PANEL_wide_text[320];
char *PANEL_wide_cont;

SWORD PANEL_wide_top_person;
SWORD PANEL_wide_bot_person;
SWORD PANEL_wide_top_is_talking;


//
// The positions of all the icons on the IC page.
//

#define PANEL_IC_BACKDROP		0
#define PANEL_HEALTH_PLAYER		1
#define PANEL_HEALTH_CAR		2
#define PANEL_IC_AK47			3
#define PANEL_IC_GRENADE		4
#define PANEL_IC_PISTOL			5
#define PANEL_IC_SHOTGUN		6
#define PANEL_IC_KNIFE			7
#define PANEL_IC_BASEBALLBAT	8
#define PANEL_IC_EXPLOSIVES		9
#define PANEL_IC_HAND_DARCI		10
#define PANEL_IC_GEAR_LOW		11
#define PANEL_IC_GEAR_HIGH		12
#define PANEL_IC_RADAR_DOT		13
#define PANEL_IC_SEARCHING		14
#define PANEL_IC_COMPLETED		15
#define PANEL_IC_LEVEL			16
#define PANEL_IC_LOST			17
#define PANEL_IC_INV_LEFT		18
#define PANEL_IC_INV_MIDDLE		19
#define PANEL_IC_INV_RIGHT		20

typedef struct
{
	UBYTE u1;
	UBYTE v1;
	UBYTE u2;
	UBYTE v2;
	UWORD page;
	
} PANEL_Ic;


PANEL_Ic PANEL_ic[] =
{
	{0,32,103,113,8		},
	{64,128,127,191,34	},
	{0,128,63,191,32	},
	{224,32,255,54,15	},
	{128,64,160,86,20	},
	{160,32,192,54,13	},
	{192,32,224,54,14	},
	{224,64,255,86,23	},
	{192,64,224,86,22	},
	{160,64,192,86,21	},
	{128,32,160,54,12	},
	{128,96,160,118,28	},
	{160,96,192,118,29	},
	{128,56,130,58,12   },
	{64,16,140,28,0		},
	{64,0,186,16,0		},
	{0,0,62,16,0		},
	{0,16,52,32,0		},
	{96,128,99,161,35	},
	{99,128,133,161,35	},
	{133,128,136,161,35	}
};

extern void DRAW2D_Box_Page(SLONG x,SLONG y,SLONG ox,SLONG oy,SLONG rgb);
extern SLONG AENG_cam_yaw;
extern SLONG	analogue;
extern SLONG AENG_text_wrappoint;
extern void PANEL_draw_stats(Thing *who);
extern void PANEL_ScoresDraw();

SLONG PANEL_sign_time;
SLONG PANEL_sign_which;

#define MAP_MAX_BEACON_COLOURS 6

extern ULONG MAP_beacon_colour[MAP_MAX_BEACON_COLOURS];
void AENG_draw_arrow(SLONG x,SLONG y,SLONG angle,SLONG colour)
{
	POLY_FT4 *p;
	UBYTE u,v;
	SLONG cx,cy;

	cx=COS(512-angle)>>14;
	cy=SIN(512-angle)>>14;

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,colour&0xff);
	setXY4(p,x+cx-(cy),y+cy+(cx),
			 x+cx+(cy),y+cy-(cx),
			 x-cx-(cy),y-cy+(cx),
			 x-cx+(cy),y-cy-(cx));
	u=getPSXU(POLY_PAGE_ARROW);
	v=getPSXV(POLY_PAGE_ARROW);
	setUV4(p,u,v,u+31,v,u,v+31,u+31,v+31);
	p->tpage=getPSXTPageE(POLY_PAGE_ARROW);
	p->clut=getPSXClutE(POLY_PAGE_ARROW);

	DOPRIM(PANEL_OTZ,p);
}

void PANEL_draw_quad(SLONG left,SLONG top,SLONG width,SLONG height,SLONG page,ULONG colour,UBYTE u1,UBYTE v1,UBYTE u2,UBYTE v2)
{
	POLY_FT4 *p;
	UBYTE u,v;

	u=getPSXU(page);
	v=getPSXV(page);

	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setXYWH(p,left,top,width,height);
	setUV4(p,u1,v1,u2,v1,u1,v2,u2,v2);
	setRGB0(p,(colour>>16)&0xff,(colour>>8)&0xff,(colour)&0xff);
	p->tpage=getPSXTPageP(page);
	if (colour&0xff000000)
	{
		setSemiTrans(p,1);
		p->tpage&=~(3<<5);
	}
	p->clut=getPSXClutP(page);
	DOPRIM(PANEL_OTZ,p);
}

void PANEL_funky_quad(
		SLONG which,
		SLONG x,
		SLONG y,
		ULONG colour)
{
	SLONG right;
	SLONG bottom;

//	ASSERT(WITHIN(which, 0, PANEL_IC_R - 1));

	PANEL_Ic *pi = &PANEL_ic[which];

	right  =  (pi->u2 - pi->u1) + 1;
	bottom =  (pi->v2 - pi->v1) + 1;

 	PANEL_draw_quad(x,y,right,bottom,pi->page,colour,pi->u1,pi->v1,pi->u2,pi->v2);
}

void PANEL_flip_quad(SLONG which,SLONG x,SLONG y, ULONG colour)
{
	SLONG right;
	SLONG bottom;

	PANEL_Ic *pi = &PANEL_ic[which];

	right  =  (pi->u2 - pi->u1) + 1;
	bottom =  (pi->v2 - pi->v1) + 1;

 	PANEL_draw_quad(x,y,right,bottom,pi->page,colour,pi->u2,pi->v1,pi->u1,pi->v2);
}

#define IC_X(x) (PANEL_BACK_X+(x))
#define IC_Y(y) (PANEL_BACK_Y+(y))

#ifdef VERSION_PAL

// PANEL settings for the PAL version
// ==================================
//
// setup to run on the left of the screen

#define PANEL_BACK_X		(12)
#define PANEL_BACK_Y		(12)

#define PANEL_AMMO_X		IC_X(102)
#define PANEL_AMMO_Y		IC_Y(39)

#define PANEL_HEALTH_X		IC_X(61)
#define PANEL_HEALTH_Y		IC_Y(16)

#define PANEL_WAYPOINT_TX	IC_X(6)
#define PANEL_WAYPOINT_TY	IC_Y(5)

#define PANEL_WAYPOINT_CX	IC_X(29)
#define PANEL_WAYPOINT_CY	IC_Y(44)

#define PANEL_MFD_X			IC_X(71)
#define PANEL_MFD_Y			IC_Y(51)

#define PANEL_IC_BBX IC_X(20)
#define PANEL_IC_BBY IC_Y(10)

#define PANEL_IC_PX IC_X(10)
#define PANEL_IC_PY 172

#define PANEL_IC_GX IC_X(70)
#define PANEL_IC_GY IC_Y(17)

#define PANEL_IC_TX	IC_X(4)
#define PANEL_IC_TY IC_Y(68)

#define PANEL_IC_MESX IC_X(106)
#define PANEL_IC_MESY IC_Y(2)

#define PANEL_IC_DMX IC_X(460)
#define PANEL_IC_DMY 220

#define PANEL_IC_CRX IC_X(24)
#define PANEL_IC_CRY 227

#define PANEL_IC_SCANX IC_X(64)
#define PANEL_IC_SCANY 204

#define PANEL_IC_BTX IC_X(48)
#define PANEL_IC_BTY 172

#define PANEL_SIGN_X 144
#define PANEL_SIGN_Y 32

#define PANEL_TUTOR_X 48
#define PANEL_TUTOR_Y 96

#define PANEL_BUTTON_X 232
#define PANEL_BUTTON_Y 216

#define PANEL_INV_X IC_X(106)
#define PANEL_INV_Y IC_Y(15)

#else

// PANEL settings for the NTSC version
// ===================================
//
// setup to run on the right of the screen


#define PANEL_BACK_X		(205)
#define PANEL_BACK_Y		(12)

#define PANEL_AMMO_X		IC_X(35)
#define PANEL_AMMO_Y		IC_Y(39)

#define PANEL_HEALTH_X		IC_X(37)
#define PANEL_HEALTH_Y		IC_Y(16)

#define PANEL_WAYPOINT_TX	IC_X(6)
#define PANEL_WAYPOINT_TY	IC_Y(5)

#define PANEL_WAYPOINT_CX	IC_X(77)
#define PANEL_WAYPOINT_CY	IC_Y(44)

#define PANEL_MFD_X			IC_X(5)
#define PANEL_MFD_Y			IC_Y(51)

#define PANEL_IC_BBX IC_X(20)
#define PANEL_IC_BBY IC_Y(10)

#define PANEL_IC_PX IC_X(10)
#define PANEL_IC_PY 172

#define PANEL_IC_GX IC_X(3)
#define PANEL_IC_GY IC_Y(17)

#define PANEL_IC_TX	IC_X(4)
#define PANEL_IC_TY IC_Y(68)

#define PANEL_IC_MESX 16
#define PANEL_IC_MESY IC_Y(2)

#define PANEL_IC_DMX IC_X(460)
#define PANEL_IC_DMY 220

#define PANEL_IC_CRX IC_X(24)
#define PANEL_IC_CRY 227

#define PANEL_IC_SCANX IC_X(64)
#define PANEL_IC_SCANY 204

#define PANEL_IC_BTX IC_X(48)
#define PANEL_IC_BTY 172

#define PANEL_SIGN_X 144
#define PANEL_SIGN_Y 32

#define PANEL_TUTOR_X 48
#define PANEL_TUTOR_Y 96

#define PANEL_BUTTON_X 232
#define PANEL_BUTTON_Y 216

#define PANEL_INV_X IC_X(-8)
#define PANEL_INV_Y IC_Y(15)


#endif
SBYTE IconSpecial[]={
	PANEL_IC_HAND_DARCI,	// None
	-1,						// Key
	PANEL_IC_PISTOL,		// Gun
	-1,						// Health
	-1,						// Bomb
	PANEL_IC_SHOTGUN,		// Shotgun
	PANEL_IC_KNIFE,			// Knife
	PANEL_IC_EXPLOSIVES,	// Explosives
	PANEL_IC_GRENADE,		// Grenade
	PANEL_IC_AK47,			// AK47
	-1,						// Mine
	-1,						// Thermodroid
	PANEL_IC_BASEBALLBAT,	// Base ball bat
	-1,						// Pistol Ammo
	-1,						// Shotgun Ammo
	-1,						// AK47 Ammo
	-1,						// Keycard
	-1,						// File
	-1,						// Floppy disk
	-1,						// Crowbar
	-1,						// Gasmask
	-1,						// Wrench
	-1,						// Video
	-1,						// Gloves
	-1,						// Weedaway
	-1,						// Treasure
	-1,						// Red Car Key
	-1,						// Blue Car Key
	-1,						// Green Car Key
	-1,						// Black Car Key
	-1,						// White Car Key
};

// Ammo structures, 0 means no ammo, -1 is a number/counter

struct ammo_data {
	SBYTE ammo;
	SBYTE clips;
} Ammo[]={
	{0,0},{0,0},{0,0},			// Panel back graphics
	{1,1},{1,0},{1,1},{1,1},	// Projectile weapons + Grenade
	{0,0},{0,0},{1,0},{0,0}		// Concussion weapons + Explosives + Hand
};

ULONG danger_col[]={0,0xff0000,0xffdd00,0x00dd00};

extern SLONG	text_width(CBYTE *message,SLONG font_id,SLONG *char_count);

void PANEL_draw_special(Thing* darci)
{
	SLONG specialtype,ammo, which_gun,clips;
	Thing *p_special;
	char cbuf[8];

	if (darci->Genus.Person->SpecialUse)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialUse);

		specialtype = p_special->Genus.Special->SpecialType;
		ammo        = p_special->Genus.Special->ammo;
		switch(specialtype)
		{
		case SPECIAL_AK47:
			clips=darci->Genus.Person->ammo_packs_ak47/30;
			break;
		case SPECIAL_SHOTGUN:
			clips=darci->Genus.Person->ammo_packs_shotgun>>3;
			break;
		default:
			clips=0;
			break;
		}
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
		clips		= darci->Genus.Person->ammo_packs_pistol/15;
	}


	if (IconSpecial[specialtype]!=-1)
	{
		if (specialtype==SPECIAL_GRENADE)
		{
			if (p_special->SubState==SPECIAL_SUBSTATE_ACTIVATED)
			{
				sprintf(cbuf,"%d",(p_special->Genus.Special->timer/(16*20))+1);

				if (p_special->Genus.Special->timer<640)
					draw_text_at(PANEL_IC_TX,PANEL_IC_TY,cbuf,0xff0000);
				else
					draw_text_at(PANEL_IC_TX,PANEL_IC_TY,cbuf,0xffffff);
			}
		}
		
		which_gun=IconSpecial[specialtype];
		
		PANEL_funky_quad(which_gun,PANEL_IC_GX,	PANEL_IC_GY,0xff7f7f7f);

		if (Ammo[which_gun].ammo)
		{
			if (Ammo[which_gun].clips)
				sprintf(cbuf,"%2d/%d",ammo,clips);
			else
				sprintf(cbuf,"%2d",ammo);
			draw_text_at(PANEL_AMMO_X-text_width(cbuf,0,&clips),PANEL_AMMO_Y,cbuf,0x7f7f7f);
		}
	}
}

void PANEL_render_stamina(Thing *thing)
{
	POLY_FT4 *p;
	UBYTE u,v;

	SLONG stamina=thing->Genus.Person->Stamina;
	SATURATE(stamina,0,128);

	SLONG stam=(stamina*45)>>7;

	ALLOCPRIM(p,POLY_FT4);
	u=getPSXU(POLY_PAGE_ARROW);
	v=getPSXV(POLY_PAGE_ARROW);

	setPolyFT4(p);

	setXYWH(p,PANEL_HEALTH_X-3,PANEL_HEALTH_Y+42-stam,7,7);
	setUV4(p,u,v+16,u,v,u+31,v+16,u+31,v);
	setRGB0(p,128-stamina,stamina,0);
	p->tpage=getPSXTPageE(POLY_PAGE_ARROW);
	p->clut=getPSXClutE(POLY_PAGE_ARROW);

	DOPRIM(PANEL_OTZ,p);
}

void PANEL_render_health(SLONG type,Thing *thing)
{
#if 0
	SLONG health;
	SLONG angle;
	SWORD bx,by,tx,ty;
	UBYTE u,v;
	POLY_FT3 *p;

	if (type)
		health=thing->Genus.Vehicle->Health;
	else
		health=thing->Genus.Person->Health;

	SATURATE(health,0,200);

	if (health==0)
		return;

	if (health<100)
		angle=(1024*health)/200;
	else
		angle=512;

	u=getPSXU(34-(2*type));
	v=getPSXV(34-(2*type));

	ALLOCPRIM(p,POLY_FT3);
	setPolyFT3(p);

	bx=(SIN(angle+256))>>10;
	by=(COS(angle+256))>>10;

	by*=(34<<6)/bx;
	by>>=6;

	setXY3(p,PANEL_WAYPOINT_CX,PANEL_WAYPOINT_CY,PANEL_WAYPOINT_CX-34,PANEL_WAYPOINT_CY+34,
			 PANEL_WAYPOINT_CX-34,PANEL_WAYPOINT_CY+by);
	setUV3(p,u+34,v+34,u,v+68,u,v+34+by);
	p->tpage=getPSXTPageP(34-(2*type));
	p->clut=getPSXClutP(34-(2*type));
	setRGB0(p,128,128,128);

	DOPRIM(PANEL_OTZ,p);

	if (health<101)
		return;

	ALLOCPRIM(p,POLY_FT3);
	setPolyFT3(p);

	angle=(1024*health)/200;

	bx=(SIN(angle+256))>>10;
	by=(COS(angle+256))>>10;

	bx*=(34<<6)/by;
	bx>>=6;

	setXY3(p,PANEL_WAYPOINT_CX,PANEL_WAYPOINT_CY,PANEL_WAYPOINT_CX-34,PANEL_WAYPOINT_CY-34,
			 PANEL_WAYPOINT_CX+bx,PANEL_WAYPOINT_CY-34);
	setUV3(p,u+34,v+34,u,v,u+34+bx,v);
	p->tpage=getPSXTPageP(34-(2*type));
	p->clut=getPSXClutP(34-(2*type));
	setRGB0(p,128,128,128);

	DOPRIM(PANEL_OTZ,p);
#else
	SLONG health;
	UBYTE u,v;
	POLY_FT4 *p;

	if (type)
	{
		//car
		health=thing->Genus.Vehicle->Health;

		health=(health*47)/300;
	}
	else
	{
		health=thing->Genus.Person->Health;
		if(thing->Genus.Person->PersonType==PERSON_ROPER)
			health=(health*47)/400;
		else
			health=(health*47)/200;
	}


	if (health==0)
		return;

	SATURATE(health,0,47);

	u=getPSXU(34-(2*type));
	v=getPSXV(34-(2*type));


	ALLOCPRIM(p,POLY_FT4);
	setPolyFT4(p);
	setXYWH(p,PANEL_HEALTH_X,PANEL_HEALTH_Y+47-health,7,health);
	setUVWH(p,u,v+47-health,7,health);
	setRGB0(p,128,128,128);
	p->tpage=getPSXTPageP(34-(2*type));
	p->clut=getPSXClutP(34-(2*type));

	DOPRIM(PANEL_OTZ,p);
#endif
}

extern SLONG timert;
extern CBYTE timerstr[];

void PANEL_draw_mfd()
{
	CBYTE cbuf[8];

	if (timert)
	{
		draw_text_at(PANEL_MFD_X,PANEL_MFD_Y,timerstr,0x7f7f7f);
		timert--;
	}
	else if (wad_level==22)
	{
	 	sprintf(cbuf, "%d%%", CRIME_RATE);

		draw_text_at(
			PANEL_MFD_X,
			PANEL_MFD_Y,
			cbuf,
			0x7f7f7f);
	}

}
extern CBYTE *EWAY_tutorial_string;

void PANEL_draw_tutorial(CBYTE *tutor)
{
	POLY_F4 *p;
	DR_TPAGE *pp;
	SLONG h;
	AENG_text_wrappoint=DisplayWidth-PANEL_TUTOR_X;
	h=FONT2D_DrawStringWrap(tutor,PANEL_TUTOR_X,PANEL_TUTOR_Y,0x7f7f7f);
	AENG_text_wrappoint=DisplayWidth-12;
	ALLOCPRIM(p,POLY_F4);
	setPolyF4(p);
	setXYWH(p,PANEL_TUTOR_X-4,PANEL_TUTOR_Y-4,DisplayWidth-((PANEL_TUTOR_X<<1)-8),h+4);
	setRGB0(p,64,64,64);
	setSemiTrans(p,1);
	DOPRIM(PANEL_OTZ,p);
	ALLOCPRIM(pp,DR_TPAGE);
	setDrawTPage(pp,0,0,getTPage(0,2,0,0));
	DOPRIM(PANEL_OTZ,pp);

	if (GAME_TURN&24)
		draw_text_at(PANEL_BUTTON_X,PANEL_BUTTON_Y,STR_CONT,0x7f7f7f);
}

void PANEL_draw_search(SLONG timer)
{
	SLONG percent = timer>>8;
	DR_TPAGE *p;

	SATURATE(percent,0,100);

#ifndef SEARCHING
	if (percent&12)
		PANEL_funky_quad(PANEL_IC_SEARCHING,122,115,0x7f7f00);
#else
	if (percent&12)
		FONT2D_DrawStringCentred(SEARCHING,160,123,0x7f7f00,256);
#endif

	if (percent>0)
		DRAW2D_Box_Page(110,112,110+percent,130,0xff3f0000);
	if (percent<100)
		DRAW2D_Box_Page(110+percent,112,210,130,0xff000000);
	ALLOCPRIM(p,DR_TPAGE);
	setDrawTPage(p,0,0,getTPage(0,0,0,0));
	DOPRIM(PANEL_OTZ,p);
}

extern SLONG PSX_inv_open;
extern int PSX_inv_focus;
extern int PSX_inv_count;
SLONG PANEL_icon_time;

void PANEL_new_funky()
{
	SLONG health, stamina;

	SLONG which_gun;
	Thing* p_special,*darci;

	darci=NET_PERSON(0);

	if (darci == NULL)
	{
		return;
	}

#if 0
	if (MFX_Conv_playing)
		FONT2D_DrawString("AUDIO PLAYING",96,116,0x7f7f7f,256);
#endif
	if (GAME_FLAGS & GF_PAUSED)
		return;

	if (GAME_STATE & (GS_LEVEL_WON|GS_LEVEL_LOST))
	{	 
		PANEL_new_text_process();
		PANEL_new_text_draw();
		return;
	}


	if (EWAY_tutorial_string)
	{
		PANEL_draw_tutorial(EWAY_tutorial_string);
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


	if (GAME_FLAGS & GF_PAUSED)
		return;


	PANEL_draw_mfd();

	PANEL_draw_beacons();
	PANEL_render_thugs();

	if (PSX_inv_open)
	{
		PANEL_inventory(darci);
	}
	else if (darci->Genus.Person->Flags&FLAG_PERSON_DRIVING)
	{
		Thing *car=TO_THING(darci->Genus.Person->InCar);
		PANEL_render_health(1,car);
//		if (car->Genus.Vehicle->Gear)
//			PANEL_funky_quad(PANEL_IC_GEAR_HIGH,PANEL_IC_GX,PANEL_IC_GY,0x7f7f7f);
//		else
			PANEL_funky_quad(PANEL_IC_GEAR_LOW,PANEL_IC_GX,PANEL_IC_GY,0xff7f7f7f);
	}
	else
	{
		PANEL_render_stamina(darci);
		PANEL_render_health(0,darci);
		PANEL_draw_special(darci);
	}

#ifdef VERSION_PAL
	PANEL_funky_quad(PANEL_IC_BACKDROP,PANEL_BACK_X,PANEL_BACK_Y,0xff7f7f7f);
#else
	PANEL_flip_quad(PANEL_IC_BACKDROP,PANEL_BACK_X,PANEL_BACK_Y,0xff7f7f7f);
#endif

	SLONG dtime = GetTickCount() - PANEL_sign_time;

	if (dtime < 5000 && (GAME_TURN&24))
		PANEL_DrawSign(PANEL_SIGN_X,PANEL_SIGN_Y,PANEL_sign_which);

	PANEL_new_text_process();
	PANEL_new_text_draw();

	if (PANEL_icon_time)
	{
		PANEL_draw_stats(NET_PLAYER(0));
		PANEL_icon_time--;
	}
}
/*
void PANEL_draw_treasure(SLONG count)
{
	SLONG i,x;

	if (count)
	{
		FONT2D_DrawStringCentred("Treasure:",256,144,0x00ff00,384);
		x=256-(16*count);
		for(i=0;i<count;i++,x+=32)
			PANEL_funky_quad(PANEL_IC_SHIELD,x,160,0x7f7f7f);
	}
}

#define		HEALTH_BAR_WIDTH	104
#define		HEALTH_BAR_HEIGHT	10
#define		HEALTH_BAR_VERT_GAP	2
#define		HEALTH_BAR_HORIZ_GAP	2

void	PANEL_draw_health_bar(SLONG x,SLONG y,SLONG percentage)
{
	POLY_F4 *p;

	ALLOCPRIM(p,POLY_F4);

	if (percentage>0)
	{
		setPolyF4(p);
		setXYWH(p,x+HEALTH_BAR_VERT_GAP,y+HEALTH_BAR_HORIZ_GAP,percentage,6);
		setRGB0(p,200-(percentage<<1),(percentage<<1),0);
		DOPRIM(PANEL_OTZ,p);
	}
	p++;
	setPolyF4(p);
	setXYWH(p,x,y,HEALTH_BAR_WIDTH,HEALTH_BAR_HEIGHT);
	setRGB0(p,0,0,0);
	DOPRIM(PANEL_OTZ,p);
	p++;
}


#define B0 (1 << 0)
#define B1 (1 << 1)
#define B2 (1 << 2)
#define B3 (1 << 3)
#define B4 (1 << 4)
#define B5 (1 << 5)
#define B6 (1 << 6)

#define PANEL_SEG_L	(16)
#define PANEL_SEG_W ( 4)

UBYTE SEG_number[10] =
{
	B0|B1|B2|B4|B5|B6,
	B1|B4,
	B0|B2|B3|B4|B6,
	B0|B2|B3|B5|B6,
	B1|B2|B3|B5,
	B0|B1|B3|B5|B6,
	B0|B1|B3|B4|B5|B6,
	B0|B2|B5,
	B0|B1|B2|B3|B4|B5|B6,
	B0|B1|B2|B3|B5
};
*/

void PANEL_inv_weapon(SLONG x, SLONG y, SLONG item, SLONG alpha) {
	SLONG which_gun;

	//
	// Which gun shall we draw.
	//
	switch(item)
	{
		default:
			which_gun = PANEL_IC_HAND_DARCI;
			break;

		case SPECIAL_GUN:
			which_gun  = PANEL_IC_PISTOL;
			break;

		case SPECIAL_AK47:
			which_gun  =  PANEL_IC_AK47;
			break;

		case SPECIAL_BASEBALLBAT:
			which_gun  =  PANEL_IC_BASEBALLBAT;
			break;

		case SPECIAL_KNIFE:
			which_gun  = PANEL_IC_KNIFE;
			break;

		case SPECIAL_SHOTGUN:
			which_gun  =  PANEL_IC_SHOTGUN;
			break;

		case SPECIAL_GRENADE:
			which_gun  =  PANEL_IC_GRENADE;
			break;

		case SPECIAL_EXPLOSIVES:
			which_gun  =  PANEL_IC_EXPLOSIVES;
			break;
	}

	PANEL_funky_quad(
		which_gun,
		x ,
		y ,
		alpha);
}


#define PANEL_ADDWEAPON(item) draw_list[draw_count++]=item
#define ITEM_SEPERATION (35)

void PANEL_inventory(Thing *darci) 
{
	DR_TPAGE *p;
	CBYTE draw_list[10];
	UBYTE draw_count=0;
	Thing *p_special = NULL;
	SLONG x,c0,i,y;
	UBYTE current_item = 0;

	Thing *player=NET_PERSON(0);

	PANEL_ADDWEAPON(0);

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special) 
		{
			ASSERT(p_special->Class == CLASS_SPECIAL);
			if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
				SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES)
			{
			  if (THING_NUMBER(p_special)==darci->Genus.Person->SpecialUse) current_item=draw_count;
			  PANEL_ADDWEAPON(p_special->Genus.Special->SpecialType);
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN) {
		if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) current_item=draw_count;
		PANEL_ADDWEAPON(SPECIAL_GUN);
	}

	current_item = PSX_inv_focus;

	if (draw_count==1) 
	{
		PSX_inv_open=0;
		return;
	}
extern void	CONTROLS_set_inventory(Thing *darci, Thing *player,SLONG count);
	if (draw_count==2)
	{
		PSX_inv_focus=1-PSX_inv_focus;
		PSX_inv_open=0;
		CONTROLS_set_inventory(darci,player,PSX_inv_focus);
		return;
	}
	if (current_item==-1) return;

#ifdef VERSION_NTSC
	x = PANEL_INV_X-((draw_count-1)*ITEM_SEPERATION);
#else
	x = PANEL_INV_X;
#endif
	y = PANEL_INV_Y;

	PANEL_funky_quad(PANEL_IC_INV_LEFT,x,y,0xff7f7f7f);
	x+=4;

	i = current_item+1;

	if (i==draw_count)
		i=0;

	for (c0=0;c0<draw_count-1;c0++)
	{
		PANEL_inv_weapon(x+1,y+2,draw_list[i],0x3f3f3f);
		PANEL_funky_quad(PANEL_IC_INV_MIDDLE,x,y,0xff7f7f7f);
		x+=ITEM_SEPERATION;
		i=i+1;
		if (i==draw_count) i=0;
	}
	PANEL_inv_weapon(PANEL_IC_GX,PANEL_IC_GY,draw_list[i],0x7f7f7f);

	PANEL_funky_quad(PANEL_IC_INV_RIGHT,x,y,0xff7f7f7f);

	PANEL_draw_stats(NET_PLAYER(0));

}
typedef struct
{
	Thing *who;			// Who is saying the message. NULL => computer message
	CBYTE  text[320];
	SLONG  delay;		// 0 => unused.
	SLONG  turns;		// The number of turns this message has been alive for.

} PANEL_Text;

#define PANEL_MAX_TEXTS 4	// Power of 2 please!

PANEL_Text PANEL_text[PANEL_MAX_TEXTS];
SLONG      PANEL_text_head;	// Always acces PANEL_text[PANEL_text_head & (PANEL_MAX_TEXTS - 1)]
SLONG      PANEL_text_tail;	// Always acces PANEL_text[PANEL_text_tail & (PANEL_MAX_TEXTS - 1)]
SLONG      PANEL_text_tick;

CBYTE PANEL_msg[320];

void PANEL_new_text_init(void)
{
	int i;

	for(i=0;i<PANEL_MAX_TEXTS;i++)
	{
		PANEL_text[i].who=NULL;
		PANEL_text[i].delay=0;
		PANEL_text[i].text[0]=0;
	}

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

	va_list	ap;

	va_start(ap, fmt);
	vsprintf(PANEL_msg, fmt, ap);
	va_end  (ap);

	//
	// Make sure the message isn't too long.
	//

	//
	// Create a new message at the tail of the queue.
	//

	pt = &PANEL_text[PANEL_text_tail & (PANEL_MAX_TEXTS - 1)];

	pt->who   = who;
	pt->delay = delay+100;	// Added delay because we dont display for 3 frames incase it's snatched by PANEL_new_widescreen
	pt->turns = 0;

	strcpy(pt->text, PANEL_msg);

	//
	// Process the message so we put in vertical '|'s 
	//

	PANEL_text_tail += 1;
}

void PANEL_new_text_process()
{
	SLONG i;

	PANEL_Text *pt;

	//
	// Process 20 times a second but no more than 4 times a frame.
	//

	for (i = 0; i < PANEL_MAX_TEXTS; i++)
	{
		pt = &PANEL_text[i];

		if (pt->delay)
		{
			pt->delay -= 30;

			if (pt->delay < 0)
			{
				pt->delay = 0;
			}

		}
		pt->turns += 1;
	}
}

CBYTE PANEL_help_message[64];
SLONG PANEL_help_timer;

CBYTE *PANEL_info_message=PANEL_help_message;
SLONG PANEL_info_time;

void PANEL_new_info_message(CBYTE *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(PANEL_info_message, fmt, ap);
	va_end  (ap);
	
	PANEL_info_time = GetTickCount();
}


void PANEL_new_help_message(CBYTE *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(PANEL_help_message, fmt, ap);
	va_end  (ap);

	//
	// How long the message lasts for.
	//

	PANEL_help_timer = 160 * 20 * 5;	// 5 seconds....
}

void PANEL_new_text_draw()
{
	SLONG i;
	SLONG ybase;
	SLONG y = PANEL_IC_MESY;
	SLONG height;

	PANEL_Text *pt;

#ifdef VERSION_NTSC
	AENG_text_wrappoint=200;
#else
	AENG_text_wrappoint=304;
#endif

	// This is a very clunky bodge, in coversations multiple lines
	// of text are added but none are removed from the tail, so when
	// we come back the difference between head and tail can be over
	// the maximum buffer length, which will then wrap around and display
	// the same message multiple times, therefore if we have more than 
	// PANEL_MAX_TEXTS messages we need to shift our head forwards.to
	// remove those bogus messages.

	if ((PANEL_text_tail-PANEL_text_head)>PANEL_MAX_TEXTS)
	{
		PANEL_text_head=PANEL_text_tail-4;
	}

	for (i = PANEL_text_head; i < PANEL_text_tail; i++)
	{
		pt = &PANEL_text[i & (PANEL_MAX_TEXTS - 1)];

		if (i == PANEL_text_head)
		{
			if (pt->delay == 0)
			{
				PANEL_text_head += 1;
				continue;
			}
		}

		if ((pt->delay != 0)&&(pt->turns>2))
		{
			//
			// Draw this message. Start off with the face.
			//

			ybase = y;

/*			PANEL_new_face(
				pt->who,
				PANEL_IC_MESX,
				ybase - 3,
				PANEL_FACE_SMALL);
*/
			//
			// The text....
			//

			height = FONT2D_DrawStringWrap(pt->text, PANEL_IC_MESX +2, y, 0x00ff00);

			{
				POLY_F4 *p;
				DR_TPAGE *p2;
				ALLOCPRIM(p,POLY_F4);
				ALLOCPRIM(p2,DR_TPAGE);
				setPolyF4(p);
				setSemiTrans(p,1);
#ifdef VERSION_PAL
				setXYWH(p,PANEL_IC_MESX ,y - 1,DISPLAYWIDTH - (PANEL_IC_MESX+12),height-2);
#else
				setXYWH(p,PANEL_IC_MESX ,y - 1,PANEL_BACK_X - (PANEL_IC_MESX), height-2);
#endif
				setRGB0(p,64,64,64);
				DOPRIM(PANEL_OTZ,p);
				setDrawTPage(p2,0,0,getTPage(0,2,0,0));
				DOPRIM(PANEL_OTZ,p2);
			}

			if (height < 16)
			{
				height = 16;
			}

			y += height;
		}
	}
#ifdef VERSION_NTSC
	AENG_text_wrappoint=308;//DISPLAYWIDTH-12;
#endif
}

void PANEL_draw_beacons(void)
{
	SLONG i;

	SLONG dx;
	SLONG dz;
	SLONG dist;
	SLONG dangle;
	SLONG score;
	SLONG angle;


	SLONG ax;
	SLONG ay;

	SLONG bx;
	SLONG by;

	SLONG cx;
	SLONG cy;

	PSX_POLY_Point *pp=perm_pp_array;

	MAP_Beacon *mb;

	ULONG colour;

	Thing *darci = NET_PERSON(0);

	if (analogue)
		angle=-AENG_cam_yaw;
	else
		angle=darci->Draw.Tweened->Angle;


	SLONG best_beacon = NULL;
	SLONG best_score  = INFINITY;

	for (i = 1; i < MAP_MAX_BEACONS; i++)
	{
		mb = &MAP_beacon[i];

		if (!mb->used)
		{
			continue;
		}

		colour = MAP_beacon_colour[i % MAP_MAX_BEACON_COLOURS] | (0xff000000);

		//
		// Work out the distance and relative angle of this beacon from darci.
		//

		if (mb->track_thing)
		{
			Thing *p=TO_THING(mb->track_thing);
			dx = (p->WorldPos.X - darci->WorldPos.X) >> 8;
			dz = (p->WorldPos.Z - darci->WorldPos.Z) >> 8;
			extern SLONG is_person_dead(Thing *p_person);

			if (p->Class == CLASS_PERSON)
			{
				if (p->State == STATE_DEAD)
				{

					//
					// Don't draw beacons to dead people.
					//

					if (p->SubState == SUB_STATE_DEAD_INJURED)
					{
						//
						// Except if they're only injured...
						//

						if (p->Genus.Person->pcom_ai == PCOM_AI_FIGHT_TEST)
						{
							//
							// Except if they're fight test dummies!
							//

							mb->used         = 0;
							continue;
						}
					}
					else
					{
						mb->used         = 0;
						continue;
					}
				}
			}

		}
		else
		{
			dx = (mb->wx - (darci->WorldPos.X >> 8));
			dz = (mb->wz - (darci->WorldPos.Z >> 8));
		}

		dist = (dx*dx) + (dz*dz);

		if (dist < (1<<22))
		{
			score  = dist ;
			score *= 2 * 1024 * 22 / 360;

			cx=((-COS(angle)*dx)>>16)+((SIN(angle)*dz)>>16);
			cy=((SIN(angle)*dx)>>16)+((COS(angle)*dz)>>16);

			PANEL_funky_quad(PANEL_IC_RADAR_DOT,PANEL_WAYPOINT_CX-(cx>>6),PANEL_WAYPOINT_CY+(cy>>6),colour);
		}
		else
		{
			dangle= (ratan2(dx,dz)>>1) - angle;

			score = (dangle % (2047))-1024;

			if (score > 1024) {score -= 2048;}
			if (score < -1024) {score += 2048;}

			//
			// The arrow positions...
			//

			cx = (SIN(dangle) * 24) >>16;
			cy = (COS(dangle) * 24) >>16;

			//
			// Draw an arrow in this direction.
			//

			AENG_draw_arrow(PANEL_WAYPOINT_CX+cx,PANEL_WAYPOINT_CY+cy,dangle,colour);
		}

		if (abs(score) < best_score)
		{
			best_score  = abs(score);
			best_beacon = i;
		}
	}

	if (PANEL_info_time > (GetTickCount() - 2000))
	{
//		FONT2D_DrawString(PANEL_info_message,PANEL_WAYPOINT_TX,PANEL_WAYPOINT_TY,0x7f7f7f,256);
		draw_text_at(PANEL_WAYPOINT_TX,PANEL_WAYPOINT_TY,PANEL_info_message,0x7f7f7f);
	}
	else if (best_beacon)
	{
		ASSERT(WITHIN(best_beacon, 1, MAP_MAX_BEACONS - 1));

		mb = &MAP_beacon[best_beacon];
extern	SLONG EWAY_mess_upto;
		ASSERT(mb->index>=0 &&mb->index<EWAY_mess_upto);

		extern CBYTE *EWAY_get_mess(SLONG index);

		FONT2D_DrawString(
			EWAY_get_mess(mb->index),
			PANEL_WAYPOINT_TX,
			PANEL_WAYPOINT_TY,
			MAP_beacon_colour[best_beacon % MAP_MAX_BEACON_COLOURS],
			256);
			
	}
}

void PANEL_render_thugs()
{

	Thing *p_found,*darci;
	SLONG i;
	SLONG dx,dz,cx,cy;
	SLONG colour,dist;
	BOOL display;
	SLONG angle;

	darci=NET_PERSON(0);

	if (analogue)
		angle=-AENG_cam_yaw;
	else
		angle=darci->Draw.Tweened->Angle;

	
	SLONG num_found = THING_find_sphere(
						darci->WorldPos.X >> 8,
						darci->WorldPos.Y >> 8,
						darci->WorldPos.Z >> 8,
						0x1000,
						THING_array,
						THING_ARRAY_SIZE,
						1 << CLASS_PERSON);


	for(i=0;i<num_found; i++)
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
			case PERSON_MIB1:
			case PERSON_MIB2:
			case PERSON_MIB3:
				display = TRUE;
				colour=0x7f0000;
				break;

			default:
				break;
		}

		if (PCOM_person_wants_to_kill(p_found) == THING_NUMBER(darci))
		{
			display=TRUE;
			colour = GAME_TURN&6?0x7f7f7f:0x7f0000;
		}

		if (display)
		{
			dx = (p_found->WorldPos.X - darci->WorldPos.X) >> 8;
			dz = (p_found->WorldPos.Z - darci->WorldPos.Z) >> 8;

			dist  = (dx*dx) + (dz*dz);

			if (dist < (1<<22))
			{
				cx=((-COS(angle)*dx)>>16)+((SIN(angle)*dz)>>16);
				cy=((SIN(angle)*dx)>>16)+((COS(angle)*dz)>>16);

				PANEL_funky_quad(PANEL_IC_RADAR_DOT,PANEL_WAYPOINT_CX-(cx>>6),PANEL_WAYPOINT_CY+(cy>>6),colour);
			}
		}
	}
}

void PANEL_new_widescreen_init()
{
	PANEL_wide_top_person     = NULL;
	PANEL_wide_bot_person     = NULL;
	PANEL_wide_top_is_talking = FALSE;
	PANEL_wide_text[0]        = '\000';
	PANEL_wide_cont			  = 0;
}

extern SBYTE f_width[];

void PANEL_wide_copy(char *str)
{
	int width=0;
	int lines=0;
	char *d=PANEL_wide_text;
	char *s=str;

	while(*s && (lines<3))
	{
		switch(*s)
		{
		case '|':
			width=0;
			lines++;
			break;
		default:
			width+=f_width[*s-32]+1;
			break;
		}
		// Add to string.
		*d++=*s++;

		if (width>258)
		{
			*d=*s;
			while (*s!=32)
			{
				s--;
				d--;
			}
			s++;
			d++;
			lines++;
			width=0;
		}
	}
	if (*s)
		d[-1]=0;
	else
		*d=0;

	if (*s)
		PANEL_wide_cont=s;
	else
		PANEL_wide_cont=0;
}

void PANEL_new_widescreen()
{
	//
	// We have to stop the clipping getting rid of whatever we are doing!
	//

	//
	// Clear the widescreen borders to black.
	//

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
	else
	{
		PANEL_wide_top_person = NULL;
		PANEL_wide_bot_person = NULL;
	}

	//
	// Steal new_text messages!
	// 

	SLONG i;

	PANEL_Text *pt;

	for (i = 0; i < PANEL_MAX_TEXTS; i++)
	{
		pt = &PANEL_text[i];

		if (pt->delay && pt->turns <= 5)
		{
			if (pt->who == NULL)
			{
				//
				// This is a message that goes on the bottom.
				// 

				PANEL_wide_copy(pt->text);

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

					PANEL_wide_copy(pt->text);

					PANEL_wide_top_is_talking = TRUE;

					pt->delay = 0;
				}
				else
				{
					//
					// This is a message that goes on the bottom.
					// 

					PANEL_wide_copy(pt->text);

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
		PANEL_new_face(TO_THING(PANEL_wide_top_person),280,8);
//		PANEL_new_face(
//			TO_THING(PANEL_wide_top_person),
//			float(DisplayWidth) - 72.0F,
//			8.0F,
//			PANEL_FACE_LARGE);
	}


	//
	// Draw the text for who is speaking.
	//

	if (PANEL_wide_text[0])
	{
		PANEL_new_face(PANEL_wide_bot_person ?  TO_THING(PANEL_wide_bot_person) : NULL, 8, 200);
		if (PANEL_wide_top_is_talking)
		{
			AENG_text_wrappoint = 276;
			FONT2D_DrawStringWrap(
				PANEL_wide_text,
				16,
				16,
				0x00ff00);
			if (PANEL_wide_cont && (GAME_TURN&8))
				FONT2D_DrawString(STR_CROSS,272,38,0xffffff);
			AENG_text_wrappoint = 308;
		}
		else
		{
			AENG_text_wrappoint = 308;

			FONT2D_DrawStringWrap(
				PANEL_wide_text,
				52,
				200,
				0x00ff00);

			if (PANEL_wide_cont && (GAME_TURN&8))
				FONT2D_DrawString(STR_CROSS,308,214,0xffffff);

		}
	}

	DRAW2D_Box_Page(0,0,DisplayWidth,48,0x000000);
	DRAW2D_Box_Page(0,192,DisplayWidth,240,0x000000);
}

typedef struct {
	UBYTE	icon;
	UBYTE	flip;
} SignData;

#define PANEL(x,y) (((y)*8)+(x))

SignData PANEL_sign[]={
	{PANEL(2,6),0},
	{PANEL(1,6),0},
	{PANEL(0,6),0},
	{PANEL(3,6),0},
	{PANEL(2,6),1},
	{PANEL(1,6),1},
	{PANEL(0,6),1},
	{PANEL(3,6),0}
};

SBYTE new_faces[]=
{
	EXTRA(4,0),
	EXTRA(5,0),
	EXTRA(4,2),
	-1,			// Force civs to use their own mesh info
	EXTRA(6,0),
	EXTRA(4,4),
	EXTRA(7,3),
	EXTRA(4,3),
	EXTRA(5,3),
	EXTRA(5,2),
	EXTRA(5,4),
	EXTRA(4,5),
	EXTRA(4,1),
	EXTRA(5,1),
	EXTRA(4,1)
};

SBYTE civ_faces[]=
{
	EXTRA(6,3),
	EXTRA(6,4),
	EXTRA(6,1),
	EXTRA(6,4),
};


void PANEL_new_face(Thing *who,SLONG x,SLONG y)
{
	POLY_FT4 *p;
	SLONG u,v,face;

	if (who==NULL || who->Class != CLASS_PERSON)
		face=EXTRA(6,2);
	else
	{
		face=new_faces[who->Genus.Person->PersonType];
		if (face==-1)
		{
			switch(who->Draw.Tweened->MeshID)
			{
			case 7:
				face=civ_faces[who->Draw.Tweened->PersonID-6];
				break;
			case 8:
				face=EXTRA(6,4);
				break;
			case 9:
				face=EXTRA(7,4);
				break;
			}
		}
	}

	ALLOCPRIM(p,POLY_FT4);
	
	setPolyFT4(p);
	setXYWH(p,x,y,32,32);
	p->tpage=getTPage(0,1,384,256);//getPSXTPageT(face);
	p->clut=getPSXClutT(face);
	u=getPSXU(face);
	v=getPSXV(face);
	setUVWH(p,u,v,31,31);
	setRGB0(p,128,128,128);
	
	DOPRIM(PANEL_OTZ,p);
}

void PANEL_DrawSign(SLONG x,SLONG y,SLONG type)
{
	POLY_FT4 *p;
	SLONG u,v;

	ALLOCPRIM(p,POLY_FT4);

	setPolyFT4(p);
	setXYWH(p,x,y,32,32);
	p->tpage=getPSXTPageP(PANEL_sign[type].icon);
	p->clut=getPSXClutP(PANEL_sign[type].icon);
	u=getPSXU(PANEL_sign[type].icon);
	v=getPSXV(PANEL_sign[type].icon);
	if (PANEL_sign[type].flip)
	{
		setUV4(p,u+31,v,u,v,u+31,v+31,u,v+31);
	}
	else
	{
		setUVWH(p,u,v,31,31);
	}
	setRGB0(p,128,128,128);
	DOPRIM(PANEL_OTZ,p);
}


#ifdef VERSION_GERMAN
#define STAT_STR "Stärke:\t\t\t%d\nKonstitution:\t%d\nReflexe:\t\t%d\nAusdauer:\t\t%d\n"
#endif
#ifdef VERSION_FRENCH
#define STAT_STR "Force:\t\t\t%d\nConstitution:\t%d\nRéflexes:\t\t%d\nRésistance:\t%d\n"
#endif
#ifdef VERSION_SPANISH
#define STAT_STR "Fuerza:\t\t\t%d\nConstitución:\t%d\nReflejos:\t\t%d\nResistencia:\t%d\n"
#endif
#ifdef VERSION_ITALIAN
#define STAT_STR "Forza:\t\t\t%d\nTempra:\t\t%d\nRiflessi:\t\t%d\nResistenza:\t%d\n"
#endif
#ifdef VERSION_USA
#define STAT_STR "Strength:\t\t%d\nConstitution:\t%d\nReflexes:\t\t%d\nStamina:\t\t%d\n"
#endif
#ifdef VERSION_JAPAN
#define STAT_STR "Strength:\t\t%d\nConstitution:\t%d\nReflexes:\t\t%d\nStamina:\t\t%d\n"
#endif
#ifdef VERSION_KOREA
#define STAT_STR "Strength:\t\t%d\nConstitution:\t%d\nReflexes:\t\t%d\nStamina:\t\t%d\n"
#endif
#ifdef VERSION_ENGLISH
#define STAT_STR "Strength:\t\t%d\nConstitution:\t%d\nReflexes:\t\t%d\nStamina:\t\t%d\n"
#endif


void PANEL_draw_stats(Thing *who)
{
	CBYTE str[64];
	sprintf(str,STAT_STR,
					who->Genus.Player->Strength,who->Genus.Player->Constitution,
					who->Genus.Player->Skill,who->Genus.Player->Stamina);
	FONT2D_DrawString(str,16,160,0x7f7f7f,256);
}

void PANEL_flash_sign(SLONG sign, SLONG flip)
{
	PANEL_sign_time  = GetTickCount();
	PANEL_sign_which = sign+(flip<<2);
}



void PANEL_draw_eog(SLONG win)
{
	if (win)
	{
#ifndef	LEVEL_WON
		PANEL_funky_quad(PANEL_IC_LEVEL,62,112,0x007f00);
		PANEL_funky_quad(PANEL_IC_COMPLETED,136,112,0x007f00);
#else
		FONT2D_DrawStringCentred(LEVEL_WON,160,120,0x007f00,511);
#endif

extern	SLONG	playing_real_mission(void);

		if (playing_real_mission())
		{
			if (GAME_TURN&128)
				PANEL_draw_stats(NET_PLAYER(0));
			else
				PANEL_ScoresDraw();
		} else
			PANEL_draw_stats(NET_PLAYER(0));
	} else
	{
#ifndef LEVEL_LOST
		PANEL_funky_quad(PANEL_IC_LEVEL,97,112,0x007f00);
		PANEL_funky_quad(PANEL_IC_LOST,171,112,0x007f00);
#else
		FONT2D_DrawStringCentred(LEVEL_LOST,160,120,0x007f00,511);
#endif
	}

	if (GAME_TURN&24)
	{
		if (win)
			draw_text_at(DISPLAYWIDTH-128,212,STR_CONT,0x00ffff);
		else
		{
			draw_text_at(DISPLAYWIDTH-144,200,STR_RETURN,0x00ffff);
			draw_text_at(DISPLAYWIDTH-128,212,STR_REPLAY,0x00ffff);
		}
	}
}

extern	SLONG	stat_killed_thug;
extern	SLONG	stat_killed_innocent;
extern	SLONG	stat_arrested_thug;
extern	SLONG	stat_arrested_innocent;
extern	SLONG	stat_count_bonus;
extern	SLONG	stat_start_time,stat_game_time;

typedef struct 
{
	SLONG str;
	SLONG *value;
} ScoreData;

extern SLONG Wadmenu_MuckyTime;
extern CBYTE Wadmenu_MuckyName[];

void PANEL_ScoresDraw()
{
	SLONG c0,count,count_bonus_left;
	SLONG y=150;
	SLONG ticks=stat_game_time;
	SLONG h,m,s;
	CBYTE str[52];

#define MAX_STATS 7

	static ScoreData PANEL_scores[]={
		{X_WON_KILLED,&stat_killed_thug},
		{X_WON_ARRESTED,&stat_arrested_thug},
		{X_WON_AT_LARGE,&count},
		{X_WON_BONUS_FOUND,&stat_count_bonus},
		{X_WON_BONUS_MISSED,&count_bonus_left},
		{X_WON_MUCKYTIME,1},
		{X_WON_TIMETAKEN,0}
	};


   	h=ticks/(1000*60*60);
	ticks-=h*1000*60*60;
	m=ticks/(1000*60);
	ticks-=m*1000*60;
	s=ticks/(1000);

	count=count_bonus_left=0;

	for(c0=0;c0<MAX_THINGS;c0++)
	{
		Thing	*p_thing;

		p_thing=TO_THING(c0);

		if(p_thing->Class==CLASS_PERSON)
		{
			if(p_thing->State!=STATE_DEAD)
			{
				switch(p_thing->Genus.Person->PersonType)
				{
 					case PERSON_THUG_RASTA:
					case PERSON_THUG_GREY:
					case PERSON_THUG_RED:
					case PERSON_MIB1:
					case PERSON_MIB2:
					case PERSON_MIB3:
						count++;
						break;


				}
			}

		}
		if(p_thing->Class==CLASS_SPECIAL)
		{
			if(p_thing->Genus.Special->SpecialType==SPECIAL_TREASURE)
			{
				count_bonus_left++;
			}
		}

	}

	for(c0=0;c0<MAX_STATS;c0++)
	{
		switch((SLONG)PANEL_scores[c0].value)
		{
		case 0:
			sprintf(str,"%s:\t(%d:%02d:%02d)",XLAT_str(PANEL_scores[c0].str),h,m,s);
			break;
		case 1:
			if (Wadmenu_MuckyTime)
				sprintf(str,"%s:\t(%d:%02d:%02d) %s",XLAT_str(PANEL_scores[c0].str),Wadmenu_MuckyTime/3600,(Wadmenu_MuckyTime/60)%60,(Wadmenu_MuckyTime%60),Wadmenu_MuckyName);
			else
				str[0]=0;
			break;
		default:
			sprintf(str,"%s:\t%d",XLAT_str(PANEL_scores[c0].str),*PANEL_scores[c0].value);
			break;
		}
		draw_text_at(32,y,str,0x7f7f7f);
		y+=10;
	}

}

void PANEL_draw_local_health(SLONG mx,SLONG my,SLONG mz,SLONG percentage,SLONG radius = 60)
{
	SVECTOR pos;
	DVECTOR scrn;
	SLONG scale;
	SLONG flag,p,z;
	SLONG tx,ty,width;

	pos.vx=mx-POLY_cam_x;
	pos.vy=my-POLY_cam_y;
	pos.vz=mz-POLY_cam_z;

	gte_RotTransPers(&pos,&scrn,&flag,&p,&z);

	scale=radius>>1;

	tx=scrn.vx-scale;
	ty=scrn.vy;
	width=scale*2;
	p=(radius*percentage)/100;
	if (p>=radius)
		DRAW2D_Box_Page(tx,ty,tx+radius,ty+1,0x00ff00);
	else
		DRAW2D_Box_Page(tx,ty,tx+p,ty+1,0xff0000);
	DRAW2D_Box_Page(tx-1,ty-1,tx+width+1,ty+2,0x000000);
}

