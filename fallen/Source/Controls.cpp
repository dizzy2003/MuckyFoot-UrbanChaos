// Control.cpp
// Guy Simmons, 4th November 1997.

#include	"Game.h"
#include	"enter.h"
#include	"id.h"
#include	"dirt.h"
#include	"fog.h"
#include	"mist.h"
#include	"water.h"
#include	"hm.h"
#include	"light.h"
#include	"shadow.h"
#include	"puddle.h"
#include	"pow.h"
#ifndef PSX
#include	"panel.h"
#endif
#include	"drip.h"
#include	"cam.h"
#include	"sewer.h"
#include	"bang.h"
#include	"mav.h"
#include	"animtmap.h"
#include	"spark.h"
#include	"animate.h"
#include	"glitter.h"
#include	"sound.h"
#include	"ob.h"
#include	"trip.h"
#include	"ob.h"
#include	"pap.h"
#include	"pcom.h"
#include	"night.h"
#include	"hook.h"
//#include	"sm.h"
#include	"ns.h"
#include	"cloth.h"
#include	"eway.h"
#include    "animal.h"
#include    "chopper.h"
#include	"interfac.h"
#include	"statedef.h"
#include	"combat.h"
#include	"door.h"
#ifndef	PSX
#include    "fallen/DDEngine/Headers/console.h"
#endif
#include	"psystem.h"
#include	"ribbon.h"
#include	"fallen/DDEngine/Headers/poly.h"
#include	"wmove.h"
#include	"balloon.h"
#include	"wand.h"
#include	"snipe.h"
#include	"barrel.h"
#include	"road.h"
#include	"fc.h"
#include	"memory.h"
#include	"ware.h"
#include	"MFx.h"
#include	"cnet.h"
#include	"widget.h"
#include	"save.h"
#include	"dike.h"
#include	"music.h"
#include	"night.h"
#include	"grenade.h"
#include	"demo.h"

#ifndef		PSX
#include	<DDLib.h>
#include	"vertexbuffer.h"
#include	"polypoint.h"
#include	"renderstate.h"
#include	"polypage.h"
#include	"font2d.h"
#ifndef TARGET_DC
#include	"FFManager.h"
#endif
#include	"fallen/ddlibrary/headers/ddlib.h"
#include	"fallen/ddengine/headers/texture.h"
#else
#include	"LIBETC.h"

// For Object Creation we need the pad input crap here.

#include	"ctrller.h"
extern ControllerPacket PAD_Input1,PAD_Input2;

// ****************************************************

#endif

extern	SLONG	am_i_a_thug(Thing *p_person);
extern	void	drop_current_gun(Thing *p_person,SLONG change_anim);
extern	SLONG	analogue;

#ifndef TARGET_DC
SLONG NIGHT_specular_enable = FALSE;
#endif

SLONG draw_3d;
extern	SLONG	mouse_input;

//CBYTE	tab_map_name[200];

#define AMB_NUM_CHOICES 3

LIGHT_Colour amb_choice[AMB_NUM_CHOICES] =
{
	{32, 32, 32},
	{38, 32, 30},
	{30, 32, 38}
};

LIGHT_Colour amb_colour;
SLONG        amb_choice_cur;

UWORD		controls;


// this allows darci to go undetected
UBYTE stealth_debug = 0;
SWORD CONTROLS_inventory_mode = 0;


BOOL dodgy_psx_mode=0;

#ifndef EDITOR
//---------------------------------------------------------------

UBYTE InkeyToAscii[]=
{
	/*   0 - 9   */		0,		0,		'1',	'2',	'3',	'4',	'5',	'6',	'7',	'8',
	/*  10 - 19  */		'9',	'0',	'-',	'=',	'\b',	'\t',	'q',	'w',	'e',	'r',
	/*  20 - 29  */		't',	'y',	'u',	'i',	'o',	'p',	'[',	']',	0,		0,
	/*  30 - 39  */ 	'a', 	's',	'd',	'f',	'g',	'h',	'j',	'k',	'l',	';',
	/*  40 - 49  */		'\'',	'`',	0,		'#',	'z',	'x',	'c',	'v',	'b',	'n',
	/*  50 - 59  */		'm',	',',	'.',	'/',	0,		'*',	0,		' ',	0,		0,
	/*  60 - 69  */		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
	/*  70 - 79  */		0,		0,		0,		0,		'-',	0,		0,		0,		'+',	0,
	/*  80 - 89  */		0,		0,		0,		0,		0,		0,		'\\',	0,		0,		0,
	/*  90 - 99  */		0,		0,		0,		0,		0,		0,		'/',	0,		0,		'(',
	/* 100 - 109 */		')',	'/',	'*',	0,		0,		0,		0,		0,		0,		0,
	/* 110 - 119 */		0,		0,		0,		'.',	0,		0,		0,		0,		0,		0,
	/* 120 - 127 */		0,		0,		0,		0,		0,		0,		0,		0
};

UBYTE InkeyToAsciiShift[]=
{
	/*   0 - 9   */		0,		0,		'!',	'"',	'�',	'$',	'%',	'^',	'&',	'*',
	/*  10 - 19  */		'(',	')',	'_',	'+',	'\b',	'\t',	'Q',	'W',	'E',	'R',
	/*  20 - 29  */		'T',	'Y',	'U',	'I',	'O',	'P',	'{',	'}',	0,		0,
	/*  30 - 39  */ 	'A', 	'S',	'D',	'F',	'G',	'H',	'J',	'K',	'L',	':',
	/*  40 - 49  */		'@',	'~',	0,		'~',	'Z',	'X',	'C',	'V',	'B',	'N',
	/*  50 - 59  */		'M',	'<',	'>',	'?',	0,		'*',	0,		' ',	0,		0,
	/*  60 - 69  */		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,
	/*  70 - 79  */		0,		0,		0,		0,		'-',	0,		0,		0,		'+',	0,
	/*  80 - 89  */		0,		0,		0,		0,		0,		0,		'|',	0,		0,		0,
	/*  90 - 99  */		0,		0,		0,		0,		0,		0,		'/',	0,		0,		'(',
	/* 100 - 109 */		')',	'/',	'*',	0,		0,		0,		0,		0,		0,		0,
	/* 110 - 119 */		0,		0,		0,		'.',	0,		0,		0,		0,		0,		0,
	/* 120 - 127 */		0,		0,		0,		0,		0,		0,		0,		0
};

#endif


//---------------------------------------------------------------

#ifndef PSX

CBYTE *cmd_list[] = {"cam", "echo", "tels", "telr", "telw", "break", "wpt", "vtx", "alpha", "gamma", "bangunsnotgames", "cctv", "win", "lose","s","l","restart","ambient","analogue","world","fade","roper", "darci", "crinkles","bangunsnotgames", "boo", NULL};

EWAY_Way* eway_find(SLONG id)
{
	SLONG i;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
		if (ew->id == id) return ew;
	}

	return NULL;
}

EWAY_Way* eway_find_near(GameCoord pos)
{
	ULONG i, d=512, d2;
	SLONG dx,dy,dz, r=-1;
	EWAY_Way *ew;

	pos.X>>=8; pos.Y>>=8; pos.Z>>=8;

	d*=d;

	for (i = 1; i < (unsigned)EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];
		dx=ew->x-pos.X;
		dy=ew->y-pos.Y;
		dz=ew->z-pos.Z;
		d2=(dx*dx)+(dy*dy)+(dz*dz);
		if (d2<d) {
			d=d2;
			r=i;
		}
	}

	if (r>0) return &EWAY_way[r];

	return NULL;
}


#ifdef TARGET_DC
BOOL allow_debug_keys=0;

#else

#ifndef NDEBUG
BOOL allow_debug_keys=1;
#else
#ifdef FAST_EDDIE
BOOL allow_debug_keys=1;
#else
BOOL allow_debug_keys=0;
#endif
#endif

#endif

BOOL dkeys_have_been_used;

void parse_console(CBYTE *str) {
  CBYTE cmd[20];
  CBYTE *ptr=str, *pt2=cmd;
  UWORD i;
  Thing	*darci=NET_PERSON(0);

  // special stuff to do with the commands themselves
  static GameCoord stored_pos = { -1,-1,-1 };
  EWAY_Way *way;

  memset(cmd,0,20);
  while ((*ptr)&&(*ptr!=32)&&(ptr-str<20)) {
	  *pt2++=*ptr++;
  }
  if (ptr-str==20) return; // far too sillyily long for a cmd name
  ptr++;

  for (i=0; cmd_list[i]; i++) {
	  if (!stricmp(cmd,cmd_list[i])) {
		  switch(i) {
			case 0: // cam -- nothing yet

				break;
			case 1: // echo -- for testing
				CONSOLE_text(ptr,5000);
				break;
			case 2: stored_pos=NET_PERSON(0)->WorldPos; CONSOLE_text("stored.",3000); break;
			case 3: // store & restore teleport positions
				if ((stored_pos.X==-1)&&(stored_pos.Y==-1)&&(stored_pos.Z==-1)) {
					CONSOLE_text("no tel stored.",5000); break;
				}
				set_person_idle(NET_PERSON(0));
				move_thing_on_map(NET_PERSON(0), &stored_pos);
				FC_force_camera_behind(0);
				SLONG plant_feet(Thing *p_person);
				plant_feet(NET_PERSON(0));
				NET_PERSON(0)->Genus.Person->Flags&=~(FLAG_PERSON_KO|FLAG_PERSON_HELPLESS);
				CONSOLE_text("restored.",5000);
				break;
			case 4: // telw -- teleport to numbered waypoint
				i=atoi(ptr);
				way=eway_find(i);
				if (way) {
					GameCoord pos= {way->x<<8,way->y<<8,way->z<<8};
					set_person_idle(NET_PERSON(0));
					move_thing_on_map(NET_PERSON(0), &pos);
					FC_force_camera_behind(0);
					SLONG plant_feet(Thing *p_person);
					plant_feet(NET_PERSON(0));
					NET_PERSON(0)->Genus.Person->Flags&=~(FLAG_PERSON_KO|FLAG_PERSON_HELPLESS);
					CONSOLE_text("z-z-zap.",5000);
				} else CONSOLE_text("wpt not found");
				break;
			case 5: // break
				ASSERT(0);
				break;
			case 6: // wpt? -- find nearest wpt
				way=eway_find_near(NET_PERSON(0)->WorldPos);
				if (way) {
					itoa(way->id,cmd,10);
					CONSOLE_text(cmd);
				} else CONSOLE_text("wpt not found");
				break;

			case 7:	// vtx - dump vertex buffer information
				{
#ifdef TARGET_DC
				CONSOLE_text("Shan't dump VB info - you can't make me.");
#else
					FILE*	fd = MF_Fopen("C:\\VertexBufferInfo.txt", "w");
					if (fd)
					{
						TheVPool->DumpInfo(fd);
						MF_Fclose(fd);
						CONSOLE_text("Info dumped at C:\\VertexBufferInfo.txt");
					}
					else CONSOLE_text("Can't open C:\\VertexBufferInfo.txt");
#endif
				}
				break;

			case 8:	// alpha - set alpha sort type
				if (ptr[0] == '0')
				{
					PolyPage::DisableAlphaSort();
					CONSOLE_text("Alpha sorting OFF");
				}
				else if (ptr[0] == '1')
				{
					PolyPage::EnableAlphaSort();
					CONSOLE_text("Alpha sorting ON");
				}
				else
				{
					CONSOLE_text(PolyPage::AlphaSortEnabled() ? "Alpha sorting is ON" : "Alpha sorting is OFF");
				}
				break;

			case 9:	// gamma - set gamma level
				if ((ptr[0] >= '0') && (ptr[0] <= '7'))
				{
					int	val = 12*(ptr[0] - '0');
					the_display.SetGamma(val, 256);
				}
				else
				{
					CONSOLE_text("Gamma 0-7");
				}
				break;

			case 10: // dkeys -- debug keys en/disable
				allow_debug_keys^=1;
				if (allow_debug_keys)
					CONSOLE_text("debug mode on");
				else
					CONSOLE_text("debug mode off");

				dkeys_have_been_used = TRUE;

				break;

			case 11:	// cctv
				if (ptr[0] == '0')	PolyPage::SetGreenScreen(false);
				else				PolyPage::SetGreenScreen();
				break;

			case 12:	// win
				if(allow_debug_keys)
					GAME_STATE = GS_LEVEL_WON;
				break;

			case 13:	// lose
					GAME_STATE = GS_LEVEL_LOST;
				break;
			case 14:
//				SAVE_ingame("");
//				CONSOLE_text("GAME SAVED");
				break;
			case 15:
//				LOAD_ingame("");
//				CONSOLE_text("GAME LOADED");
				break;
			case 16:
void	reload_level(void);
				if(allow_debug_keys)
					reload_level();
				break;
			case 17:
				//
				// ingame ambient editor
				//
				{
					CBYTE	str[100];
					SLONG r,g,b;
					sscanf(ptr,"%d %d %d",&r,&g,&b);
					CONSOLE_text(ptr);
					sprintf(str," red %d green %d blue %d \n",r,g,b);
					CONSOLE_text(str);

					NIGHT_amb_red=r;
					NIGHT_amb_green=g;
					NIGHT_amb_blue=b;
					NIGHT_cache_recalc();
					NIGHT_dfcache_recalc();
					NIGHT_generate_walkable_lighting();


				}
				break;
			case 18:
				analogue^=1;
				break;

			case 19:
				i=atoi(ptr);
				sprintf(str,"loading music world %d...",i);
				CONSOLE_text(str);
				MUSIC_WORLD=i;
				MFX_load_wave_list();
				break;
			case 20:
extern	UWORD	fade_black;
				i=atoi(ptr);
				if(i==0)
					fade_black=1;
				else
					fade_black=0;
				break;
			case 21:
				if(allow_debug_keys)
				{
					darci->Genus.Person->PersonType =  PERSON_ROPER;
					darci->Genus.Person->AnimType =  ANIM_TYPE_ROPER;
					darci->Draw.Tweened->TheChunk	= &game_chunk[ANIM_TYPE_ROPER];
					darci->Draw.Tweened->MeshID	= 0;
					darci->Draw.Tweened->PersonID=0;
					set_person_idle(darci);
				}
				break;
			case 22:
				if(allow_debug_keys)
				{
					darci->Genus.Person->PersonType =  PERSON_DARCI;
					darci->Genus.Person->AnimType =  ANIM_TYPE_DARCI;
					darci->Draw.Tweened->TheChunk	= &game_chunk[ANIM_TYPE_DARCI];
					darci->Draw.Tweened->MeshID	= 0;
					darci->Draw.Tweened->PersonID=0;
					set_person_idle(darci);
				}
				break;
			case 23:
extern int AENG_detail_crinkles;
				AENG_detail_crinkles ^= 1;
				if (AENG_detail_crinkles)
					CONSOLE_text("crinkles on");
				else
					CONSOLE_text("crinkles off");
				break;

			case 24:
//				if(allow_debug_keys)
//				VIOLENCE=1;
				break;

			case 25:
				PYRO_create(darci->WorldPos, PYRO_GAMEOVER);
				break;

		  }
		  return;
	  }
  }
  CONSOLE_text("huh?",3000);

}

#endif


//
// Takes a screen shot of the city from above.
//


#if !defined(PSX) && !defined(TARGET_DC)

#include "tga.h"

TGA_Pixel tga[480 ][640 ];

void	tga_dump(void)
{
	SLONG i;
	SLONG x;
	SLONG y;

	UBYTE red;
	UBYTE green;
	UBYTE blue;

	CBYTE fname[32];

	FILE *handle;


	//
	// Find the first available file.
	//

	for (i = 0; i < 10000; i++)
	{
		sprintf(fname, "c:\\tmp\\shot%03d.tga", i);

		handle = MF_Fopen(fname, "rb");

		if (handle)
		{
			//
			// This file already exists...
			//

			MF_Fclose(handle);
		}
		else
		{
			goto found_file;
		}
	}

	//
	// All thousand filenames are used up!
	//

	return;

  found_file:;

	//
	// Save out the raw.
	//

	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < 640; x++)
		{
			the_display.GetPixel(
				x,
				y,
			   &red,
			   &green,
			   &blue);

			tga[y][x].red=red;
			tga[y][x].green=green;
			tga[y][x].blue=blue;
		}
	}

	TGA_save(
		fname,
		640 ,
		480 ,
	   &tga[0][0],
		FALSE);

}

void plan_view_shot()
{
	SLONG i;

	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG px1;
	SLONG py1;

	SLONG px2;
	SLONG py2;

	SLONG mx;
	SLONG my;
	SLONG mz;
	SLONG height;

	UBYTE red;
	UBYTE green;
	UBYTE blue;

	SLONG shadow;

	RoofFace4 *rf;
	DFacet    *df;

	UBYTE shad[8][9] =
	{
		{4,4,4,4,4,4,4,4,4},

		{4,4,1,		// 1
		 4,2,1,
		 3,2,1},

		{3,2,1,		// 2
		 3,2,1,
		 3,2,1},

		{3,2,1,		// 3
		 3,2,2,
		 4,3,3},

		{1,1,1,		// 4
		 2,2,2,
		 3,3,3},

		{1,1,1,		// 5
		 2,2,1,
		 3,2,1},

		{3,2,1,		// 6
		 3,2,1,
		 3,2,1},

		{1,1,1,		// 7
		 2,2,4,
		 3,4,4}
	};

	//
	// The floor first.
	//

	for (px = 0; px < 128 * 3; px++)
	for (py = 0; py < 128 * 3; py++)
	{
		mx = (px << 8) / 3;
		mz = (py << 8) / 3;

		height = PAP_calc_height_at(mx,mz);

		if (ROAD_is_road(mx>>8,mz>>8))
		{
			red    = 64 + (height >> 3);

			SATURATE(red, 0, 255);

			green = red;
			blue  = red;
		}
		else
		{
			red   = 128 + (height >> 4);

			SATURATE(red, 4, 255);

			green = red - 4;
			blue  = red - 4;
		}

		tga[px][py].red   = red;
		tga[px][py].green = green;
		tga[px][py].blue  = blue;
	}

	//
	// Shadow the floor.
	//

	for (mx = 0; mx < 128; mx++)
	for (mz = 0; mz < 128; mz++)
	{
		px = mx * 3;
		py = mz * 3;

		shadow = PAP_2HI(mx,mz).Flags & 0x7;

		for (dx = 0; dx < 3; dx++)
		for (dy = 0; dy < 3; dy++)
		{
			tga[px + dx][py + dy].red   = tga[px + dx][py + dy].red   * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
			tga[px + dx][py + dy].green = tga[px + dx][py + dy].green * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
			tga[px + dx][py + dy].blue  = tga[px + dx][py + dy].blue  * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
		}

	}

	//
	// The roof faces.
	//

	for (i = 1; i < next_roof_face4; i++)
	{
		rf = &roof_faces4[i];

		height = rf->Y;

		red   = 100 + (height >> 5);
		green = red - 4;
		blue  = red + 4;

		px = (rf->RX&127) * 3;
		py = (rf->RZ&127) * 3;

		shadow = rf->DrawFlags & 0x7;

		for (dx = 0; dx < 3; dx++)
		for (dy = 0; dy < 3; dy++)
		{
			tga[px + dx][py + dy].red   = red   * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
			tga[px + dx][py + dy].green = green * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
			tga[px + dx][py + dy].blue  = blue  * shad[shadow][(2 - dx) + (2 - dy) * 3] >> 2;
		}
	}

	//
	// Fences
	//

	for (i = 1; i < next_dfacet; i++)
	{
		df = &dfacets[i];

		if (df->FacetType == STOREY_TYPE_FENCE      ||
			df->FacetType == STOREY_TYPE_FENCE_FLAT ||
			df->FacetType == STOREY_TYPE_FENCE_BRICK)
		{
			px1 = df->x[0] * 3;
			py1 = df->z[0] * 3;

			px2 = df->x[1] * 3;
			py2 = df->z[1] * 3;

			dx = SIGN(px2 - px1);
			dy = SIGN(py2 - py1);

			if (dx && dy)
			{
				//
				// Ignore diagonal facets.
				//

				continue;
			}

			x = px1;
			y = py1;

			while(1)
			{
				tga[x][y].red   = 50;
				tga[x][y].green = 200;
				tga[x][y].blue  = 10;

				x += dx;
				y += dy;

				if (x == px2 && y == py2)
				{
					break;
				}
			}
		}
	}

	//
	// Waypoints.
	//

	UBYTE dot_do;
	UBYTE dot_size;

	EWAY_Way *ew;

	for (i = 1; i < EWAY_way_upto; i++)
	{
		ew = &EWAY_way[i];

		dot_size = 1;

		switch(ew->ed.type)
		{
			case EWAY_DO_CREATE_PLAYER:
				dot_do   = TRUE;
				red      = 0;
				green    = 0;
				blue     = 0;
				dot_size = 2;
				break;

			case EWAY_DO_CREATE_ITEM:
				dot_do = TRUE;
				red    = 210;
				green  = 210;
				blue   = 40;
				break;

			case EWAY_DO_CREATE_ENEMY:

				switch(ew->ed.subtype)
				{
					case PERSON_DARCI:
					case PERSON_ROPER:

						dot_do = TRUE;
						red    = 55;
						green  = 255;
						blue   = 55;

						break;

					case PERSON_CIV:
					case PERSON_SLAG_TART:
					case PERSON_SLAG_FATUGLY:
					case PERSON_HOSTAGE:
					case PERSON_MECHANIC:
					case PERSON_TRAMP:

						//
						// Don't show wandering civs...
						//

						EWAY_Edef *ee;

						ASSERT(WITHIN(ew->index, 1, EWAY_edef_upto - 1));

						ee = &EWAY_edef[ew->index];

						if (ee->pcom_move != PCOM_MOVE_WANDER)
						{
							dot_do = TRUE;
						}
						else
						{
							dot_do = FALSE;
						}

						red   = 255;
						green = 255;
						blue  = 255;

						break;


					case PERSON_COP:

						dot_do = TRUE;
						red    = 55;
						green  = 55;
						blue   = 255;

						break;

					default:
					case PERSON_THUG_RASTA:
					case PERSON_THUG_GREY:
					case PERSON_THUG_RED:
					case PERSON_MIB1:
					case PERSON_MIB2:
					case PERSON_MIB3:

						dot_do = TRUE;

						red    = 255;
						green  = 55;
						blue   = 55;

						break;
				}

				break;

			default:
				dot_do = FALSE;
				break;
		}

		if (dot_do)
		{
			SLONG alpha;

			mx = ew->x * 3 >> 8;
			my = ew->z * 3 >> 8;

			for (dx = -dot_size; dx <= dot_size; dx++)
			for (dy = -dot_size; dy <= dot_size; dy++)
			{
				px = mx + dx;
				py = my + dy;

				if (WITHIN(px, 0, 128 * 3 - 1) &&
					WITHIN(py, 0, 128 * 3 - 1))
				{
					alpha = abs(dx) + abs(dy);

					tga[px][py].red   = red   * (4 - alpha) + tga[px][py].red   * alpha >> 2;
					tga[px][py].green = green * (4 - alpha) + tga[px][py].green * alpha >> 2;
					tga[px][py].blue  = blue  * (4 - alpha) + tga[px][py].blue  * alpha >> 2;
				}
			}
		}
	}

	extern CBYTE ELEV_fname_level[];
	extern CBYTE ELEV_fname_map  [];

	{
		CBYTE  fname[256];
		CBYTE *mapname;
		CBYTE *ch;


		if (ELEV_fname_level[0])
		{
			mapname = ELEV_fname_level;
		}
		else
		if (ELEV_fname_map[0])
		{
			mapname = ELEV_fname_map;
		}
		else
		{
			mapname = NULL;
		}

		if (mapname)
		{

			for (ch = mapname; *ch; ch++);

			while(ch > mapname)
			{
				if (*ch == '\\')
				{
					ch += 1;

					break;
				}

				ch--;
			}

			mapname = ch;

		}
		else
		{
			mapname = "shot.tga";
		}

		sprintf(fname, "c:\\shot\\%s", mapname);

		for (ch = fname; *ch; ch++);

		ch[-3] = 't';
		ch[-2] = 'g';
		ch[-1] = 'a';

		TGA_save(
			fname,
			128 * 3,
			128 * 3,
		   &tga[0][0],
			FALSE);
	}
}

#else	// PSX or TARGET_DC

void plan_view_shot()
{
}

#endif

//---------------------------------------------------------------



SLONG	yomp_speed=40;
SLONG	sprint_speed=70;

extern UBYTE aeng_draw_cloud_flag;
SLONG can_i_draw_this_special(Thing *p_special)
{
	if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
		SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
		p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES ||
		p_special->Genus.Special->SpecialType                     == SPECIAL_WIRE_CUTTER)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void	CONTROLS_set_inventory(Thing *darci, Thing *player,SLONG count)
{
	Thing *p_special = NULL;
//	SBYTE count;


	if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		// Put away your gun.
//		set_person_gun_away(darci);
		darci->Genus.Person->SpecialUse = NULL;
		darci->Draw.Tweened->PersonID&=  ~0xe0;
		darci->Genus.Person->Flags&=~FLAG_PERSON_GUN_OUT;
	}
//	if (player->Genus.Player->PopupFade<64) CONTROLS_rot_inventory(darci,player,1);   //not needed on PC anymore
//	count=player->Genus.Player->ItemFocus;

	if (!count)
	{
		set_person_item_away(darci);
		return; // using fists
	}

	// Make Darci use an item.

	if (darci->Genus.Person->SpecialList!=0)
		p_special = TO_THING(darci->Genus.Person->SpecialList);
	else
		p_special = 0;

	if (p_special && can_i_draw_this_special(p_special))
		count--;

	while (count) {
		if (!p_special) break; // no specials -- gun maybe?
		ASSERT(p_special->Class == CLASS_SPECIAL);

		if (p_special->Genus.Special->NextSpecial)
			p_special = TO_THING(p_special->Genus.Special->NextSpecial);
		else
			p_special = NULL;

		if (p_special && can_i_draw_this_special(p_special))
		{
			count--;
		}
	}

	if (p_special && can_i_draw_this_special(p_special))
	{
		set_person_draw_item(darci, p_special->Genus.Special->SpecialType);
	}
	else
	{
		darci->Genus.Person->SpecialUse = NULL;
		darci->Draw.Tweened->PersonID   = 0;

		//
		// If Darci has a gun then get it out.
		//

		if (darci->Flags & FLAGS_HAS_GUN)
		{
			set_person_draw_gun(darci);
		}
		else
		{
			set_person_idle(darci);
		}
	}


}

#ifndef PSX

Form   *test_form;
Widget *widget_text;
Widget *widget_ok;

BOOL form_proc(Form *form, Widget *widget, SLONG message)
{
	if (widget && widget->methods == &BUTTON_Methods && message == WBN_PUSH)
	{
		form->returncode = TRUE;

		return TRUE;	// Exit
	}
	else
	{
		return FALSE;	// Don't exit
	}
}


#define INVENTORY_FADE_SPEED	(32)

SBYTE CONTROLS_get_selected_item(Thing *darci, Thing *player) {
	SBYTE count=1; // 0 is fist
	Thing *p_special = NULL;
	SBYTE current_item = 0;

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special) {
			ASSERT(p_special->Class == CLASS_SPECIAL);
/*			if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
				SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES||
				p_special->Genus.Special->SpecialType                     == SPECIAL_WIRE_CUTTER)*/
			if (can_i_draw_this_special(p_special))
			{
			  if (THING_NUMBER(p_special)==darci->Genus.Person->SpecialUse) current_item=count;
			  count++;
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN) {
		if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) current_item=count;
		count++;
	}
	player->Genus.Player->ItemCount = count;

	return current_item;

}


//
// ak47 / shotgun / pistol
//
#define	AK47_SCORE		6
#define	SHOTGUN_SCORE	5
#define	PISTOL_SCORE	4
#define	KNIFE_SCORE		3
#define	BAT_SCORE		2
#define	GRENADE_SCORE	1

SBYTE CONTROLS_get_best_item(Thing *darci, Thing *player)
{
	SBYTE count=1; // 0 is fist
	Thing *p_special = NULL;
	SBYTE current_item = 0,current_score=0;

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special)
		{
			ASSERT(p_special->Class == CLASS_SPECIAL);
			if (can_i_draw_this_special(p_special))
			{
				switch(p_special->Genus.Special->SpecialType)
				{
					case SPECIAL_SHOTGUN:
						if(p_special->Genus.Special->ammo||darci->Genus.Person->ammo_packs_shotgun)
						if(current_score<SHOTGUN_SCORE)
						{
							current_item=count;
							current_score=SHOTGUN_SCORE;
						}
						break;

					case SPECIAL_AK47:
						if(p_special->Genus.Special->ammo||darci->Genus.Person->ammo_packs_ak47)
						if(current_score<AK47_SCORE)
						{
							current_item=count;
							current_score=AK47_SCORE;
						}
						break;
					case SPECIAL_GRENADE:
						if(p_special->Genus.Special->ammo)
						if(current_score<GRENADE_SCORE)
						{
							current_item=count;
							current_score=GRENADE_SCORE;
						}
						break;

					case SPECIAL_BASEBALLBAT:
						if(current_score<BAT_SCORE)
						{
							current_item=count;
							current_score=BAT_SCORE;
						}
						break;
					case SPECIAL_KNIFE:
						if(current_score<KNIFE_SCORE)
						{
							current_item=count;
							current_score=KNIFE_SCORE;
						}
						break;
				}
			  count++;
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN)
	{
		if(darci->Genus.Person->ammo_packs_pistol || darci->Genus.Person->Ammo)
		{
			if(current_score<PISTOL_SCORE)
			{
   				current_item=count;
				current_score=PISTOL_SCORE;
			}

		}

		count++;
	}

	return(current_item);

}


//
// does the panel fade in, and if no item is focus it finds if you have a current weapon and sets that as focus
//
SLONG	CONTROLS_new_inventory(Thing *darci, Thing *player)
{
	UWORD temp = player->Genus.Player->PopupFade;
	if (!temp)
		player->Genus.Player->ItemFocus=-1;

	temp+=INVENTORY_FADE_SPEED;

	if (temp<256)
		player->Genus.Player->PopupFade=temp;

	if (player->Genus.Player->ItemFocus==-1)
	{
/*
		if(darci->Genus.Person->SpecialUse==0 &&  !(darci->Flags & FLAGS_HAS_GUN) )
		{
			//
			// currently unarmed so pick best weapon
			//

			player->Genus.Player->ItemFocus = CONTROLS_get_best_item(darci, player);
			if(player->Genus.Player->ItemFocus)
			{
				CONTROLS_set_inventory(darci,player,player->Genus.Player->ItemFocus);
				return(1);
			}
		}
		else
*/

		{
			player->Genus.Player->ItemFocus = CONTROLS_get_selected_item(darci, player);
		}
	}
	return(0);
}


//void	CONTROLS_set_inventory(Thing *darci, Thing *player);

void	CONTROLS_rot_inventory(Thing *darci, Thing *player, SBYTE dir,SLONG pull_it_out_ooooerrr)
{
	player->Genus.Player->ItemFocus+=dir;
	if (player->Genus.Player->ItemFocus==-1)
		player->Genus.Player->ItemFocus=player->Genus.Player->ItemCount-1;

	if (player->Genus.Player->ItemFocus>=player->Genus.Player->ItemCount)
		player->Genus.Player->ItemFocus=0;

	if(pull_it_out_ooooerrr)
		CONTROLS_set_inventory(darci,player,player->Genus.Player->ItemFocus);
}

/*
void	CONTROLS_set_inventory(Thing *darci, Thing *player) {
	Thing *p_special = NULL;
	SBYTE count;


	if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		// Put away your gun.
//		set_person_gun_away(darci);
		darci->Genus.Person->SpecialUse = NULL;
		darci->Draw.Tweened->PersonID&=  ~0xe0;
		darci->Genus.Person->Flags&=~FLAG_PERSON_GUN_OUT;
	}
//	if (player->Genus.Player->PopupFade<64) CONTROLS_rot_inventory(darci,player,1);   //not needed on PC anymore
	count=player->Genus.Player->ItemFocus;

	if (!count) {
		set_person_item_away(darci);
		return; // using fists
	}

	// Make Darci use an item.

	if (darci->Genus.Person->SpecialList!=0)
		p_special = TO_THING(darci->Genus.Person->SpecialList);
	else
		p_special = 0;

	if (p_special && can_i_draw_this_special(p_special))
		count--;

	while (count) {
		if (!p_special) break; // no specials -- gun maybe?
		ASSERT(p_special->Class == CLASS_SPECIAL);

		if (p_special->Genus.Special->NextSpecial)
			p_special = TO_THING(p_special->Genus.Special->NextSpecial);
		else
			p_special = NULL;

		if (p_special && can_i_draw_this_special(p_special))
		{
			count--;
		}
	}

	if (p_special && can_i_draw_this_special(p_special))
	{
		set_person_draw_item(darci, p_special->Genus.Special->SpecialType);
	}
	else
	{
		darci->Genus.Person->SpecialUse = NULL;
		darci->Draw.Tweened->PersonID   = 0;

		//
		// If Darci has a gun then get it out.
		//

		if (darci->Flags & FLAGS_HAS_GUN)
		{
			set_person_draw_gun(darci);
		}
		else
		{
			set_person_idle(darci);
		}
	}


}
*/

#endif
//
// PC VERSION
//
extern	UWORD	count_gang(Thing *p_target);

// new cleaner version
void	context_music(void)
{
	UBYTE	mode = 0;
	Thing	*darci;
	static	SLONG danger=0;
	static enum Waves danger_lookup[] = { S_NULL, S_TUNE_DANGER_RED, S_NULL, S_TUNE_DANGER_GREEN };
	UBYTE	new_danger;

	if (!NET_PLAYER(PLAYER_ID))
	{
		MUSIC_mode(0);
		return;
	}
	else
		new_danger=NET_PLAYER(PLAYER_ID)->Genus.Player->Danger;

	// the danger 'tunes' are more sfx than music, and can overlap other tunes without clashing.

	if(new_danger!=danger)
	{
		danger=danger_lookup[new_danger];
		if (danger) MFX_play_ambient(MUSIC_REF+1,danger,MFX_OVERLAP);
		danger=new_danger;
	}

	// fade in and out the appropriate music

	darci=NET_PERSON(PLAYER_ID);

	if(darci->Genus.Person->Flags&FLAG_PERSON_DRIVING)
		mode = MUSIC_MODE_DRIVING;

	if(darci->SubState==SUB_STATE_CRAWLING)
		mode = MUSIC_MODE_CRAWLING;

	if(darci->Genus.Person->Mode==PERSON_MODE_FIGHT)
		mode = MUSIC_MODE_FIGHTING;

	if(darci->Genus.Person->Mode==PERSON_MODE_SPRINT)
		mode = MUSIC_MODE_SPRINTING;

#ifndef VERSION_DEMO
	// hi ho, hi ho, a bodging we shall go
	switch(MUSIC_bodge_code)
	{
	case 1:
		mode=MUSIC_MODE_TRAIN_COMBAT;
		break;
	case 2:
		mode=MUSIC_MODE_TRAIN_ASSAULT;
		break;
	case 3:
		mode=MUSIC_MODE_TRAIN_DRIVING;
		break;
	case 4:
		mode=MUSIC_MODE_FINAL_RECKONING;
		break;
	}
#endif

	if (WARE_ware[darci->Genus.Person->Ware].ambience==4)
		return;
//		mode=0; // we're inside the nightclub, so don't play context music.

// Just for dreamcast MikeD aug 2000
//	if (WARE_ware[darci->Genus.Person->Ware].ambience)
//		mode=13+WARE_ware[darci->Genus.Person->Ware].ambience;
		//return; // we're inside the nightclub, so don't play context music.

#ifndef PSX
	MUSIC_mode(mode);
#else
	MUSIC_mode(mode|MUSIC_MODE_FORCE);
#endif

}

// old skanky version
#if 0

void	context_music(void)
{
	SLONG	music=0;
	Thing	*darci;
	static	UBYTE danger_music=0,drive=0;
	UBYTE	new_danger_music;
	SLONG	wave;

	UBYTE	mode = 0;

	new_danger_music=NET_PLAYER(PLAYER_ID)->Genus.Player->Danger;

	if(new_danger_music==danger_music)
	{
		new_danger_music=0;
	}
	else
		danger_music=new_danger_music;



	darci=NET_PERSON(PLAYER_ID);

	if(darci->Genus.Person->Flags&FLAG_PERSON_DRIVING)
	{
		mode = MUSIC_MODE_DRIVING;
/*		if(drive)
		{
//			CONSOLE_text("DRIVING MUSIC");
			MUSIC_play(S_TUNE_DRIVING+(GAME_TURN&1),MUSIC_FLAG_QUEUED);
//			play_music(S_TUNE_DRIVING+(GAME_TURN&1),1);
		}
		else
		{
			MUSIC_stop(1);
			MUSIC_play(S_TUNE_DRIVING_START,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_IN);
//			play_music(S_TUNE_DRIVING_START,1);
//			CONSOLE_text("DRIVING START MUSIC");
			drive=1;
		}


		return;

	}
	else
	{
		if(drive)
		{
			MUSIC_stop(1);
		}
		drive=0;*/
	}


	if(darci->SubState==SUB_STATE_CRAWLING)
	{
		/*
//			CONSOLE_text("CRAWL MUSIC");
		if(MUSIC_wave()!=S_TUNE_CRAWL)
			MUSIC_play(S_TUNE_CRAWL,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_IN);
		else
			MUSIC_play(S_TUNE_CRAWL,MUSIC_FLAG_QUEUED);
//		play_music(S_TUNE_CRAWL,1);
		return;
	}
	else
	{
		if(MUSIC_wave()==S_TUNE_CRAWL)
			MUSIC_stop(1);
			*/
		mode = MUSIC_MODE_CRAWLING;

	}

	if(darci->Genus.Person->Mode==PERSON_MODE_FIGHT)
	{
		//
		// Fighting has priority
		//
/*		if(count_gang(darci))
		{
//			CONSOLE_text("FIGHT MUSIC");
//			play_music(S_TUNE_FIGHT+(GAME_TURN&1),1);
			wave=MUSIC_wave();
			if(wave==S_TUNE_FIGHT || wave==S_TUNE_FIGHT+1)
				MUSIC_play(S_TUNE_FIGHT+(GAME_TURN&1),MUSIC_FLAG_QUEUED);
			else
				MUSIC_play(S_TUNE_FIGHT+(GAME_TURN&1),MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT|MUSIC_FLAG_FADE_IN);
			return;
		}*/

		mode = MUSIC_MODE_FIGHTING;

	}

	if(darci->Genus.Person->Mode==PERSON_MODE_SPRINT)
	{
/*		SLONG	wave;
		wave=MUSIC_wave();
		if(wave==S_TUNE_SPRINT || wave==S_TUNE_SPRINT+1)
		{
			MUSIC_play(S_TUNE_SPRINT+(GAME_TURN&1),(MUSIC_FLAG_QUEUED));
		}
		else
		{
			MUSIC_play(S_TUNE_SPRINT+(GAME_TURN&1),(MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT));
		}

		return;*/
		mode = MUSIC_MODE_SPRINTING;
	}

	MUSIC_mode(mode);

#if 0
//	switch (NET_PLAYER(PLAYER_ID)->Genus.Player->Danger)
	if(new_danger_music)
	{
		//
		// Only play green and yellow if it is new music, ie don't loop it
		//
		switch(new_danger_music)
		{
			case 0: // no danger warning
				break;

			case 3: // green, mild danger
//			CONSOLE_text("D3 MUSIC");
//				play_music(S_TUNE_DANGER_GREEN,1);
/*
#ifndef PSX
				MFX_play_ambient(0, S_TUNE_DANGER_GREEN, 0);
#else
				MUSIC_stop(1);
				MUSIC_play(S_TUNE_DANGER_GREEN,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT);
#endif
*/

				break; // no music

			case 2: // yellow
//			CONSOLE_text("D2 MUSIC");
/*
#ifndef PSX
				MFX_play_ambient(0, S_TUNE_DANGER_YELLOW, 0);
#else
				MUSIC_stop(1);
				MUSIC_play(S_TUNE_DANGER_YELLOW,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT);
#endif
*/
//				play_music(S_TUNE_DANGER_YELLOW,1);
				break;

			case 1: // red
#ifndef PSX
				MFX_play_ambient(0, S_TUNE_DANGER_RED, 0);
#else
				MUSIC_stop(1);
				MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT);
#endif
//			CONSOLE_text("D1 MUSIC");
				//play_music(S_TUNE_DANGER_RED,1);
/*
				if(MUSIC_wave()==S_TUNE_DANGER_RED)
				{
					MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED); //queue it all the time
				}
				else
				{

					MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT); // fade it in if its not playing
				}
*/
				break;
		}
	}
#endif

/*
	else
	{
		if(NET_PLAYER(PLAYER_ID)->Genus.Player->Danger==1)
		{
			//
			// red is looped
			//
//			CONSOLE_text("D1 MUSIC");
				if(MUSIC_wave()==S_TUNE_DANGER_RED)
				{
					MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED); //queue it all the time
				}
				else
				{

					MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED|MUSIC_FLAG_FADE_OUT); // fade it in if its not playing
				}
//			MUSIC_play(S_TUNE_DANGER_RED,MUSIC_FLAG_QUEUED); //queue it all the time
		}
	}
*/
/*	{

		SLONG	wave;

		wave=MUSIC_wave();
		switch(wave)
		{
			case	 S_TUNE_FIGHT:
			case	 S_TUNE_FIGHT+1:
			case	 S_TUNE_SPRINT:
			case	 S_TUNE_SPRINT+1:
				MUSIC_stop(1);
		}
	}*/
}

#endif // old skanky version of context_music

void	set_danger_level()
{
	SLONG num_found;
	SLONG	i,j;

	SLONG dist;
	SLONG best_dist   = INFINITY;
	SLONG best_person = NULL;

	Thing *p_found;

	//
	// Work out how far each player is from danger.
	//

	for (i = 0; i < NO_PLAYERS; i++)
	{
		Thing *p_person = NET_PERSON(i);
		Thing *p_player = NET_PLAYER(i);

		if (p_person == NULL)
		{
			continue;
		}

/*		if (p_person->Genus.Person->Mode==PERSON_MODE_FIGHT && count_gang(p_person))
		{
			p_player->Genus.Player->Danger = 0;
		}
		else*/
		{
			//
			// Fighting has priority
			//
				num_found = THING_find_sphere(
								p_person->WorldPos.X >> 8,
								p_person->WorldPos.Y >> 8,
								p_person->WorldPos.Z >> 8,
								0xc00,
								THING_array,
								THING_ARRAY_SIZE,
								1 << CLASS_PERSON);

				for (j = 0; j < num_found; j++)
				{
					p_found = TO_THING(THING_array[j]);

					if (p_found == p_person)
					{
						continue;
					}

					if (p_found->State == STATE_DEAD)
					{
						continue;
					}

					//
					// Is this person dangerous to the player?
					//
/*
					if (p_found->Genus.Person->pcom_ai != PCOM_AI_CIV		&&
						p_found->Genus.Person->pcom_ai != PCOM_AI_NONE		&&
						p_found->Genus.Person->pcom_ai != PCOM_AI_BDISPOSER &&
						p_found->Genus.Person->pcom_ai != PCOM_AI_DRIVER	&&
						p_found->Genus.Person->pcom_ai != PCOM_AI_COP		&&
						p_found->Genus.Person->pcom_ai != PCOM_AI_COP_DRIVER)
*/
					if(am_i_a_thug(p_found))
					{

						SLONG dx = abs(p_found->WorldPos.X - p_person->WorldPos.X >> 8);
						SLONG dy = abs(p_found->WorldPos.Y - p_person->WorldPos.Y >> 8);
						SLONG dz = abs(p_found->WorldPos.Z - p_person->WorldPos.Z >> 8);

						//
						// Nasty person.
						//

						dist = QDIST3(dx,dy,dz);

						if (dist < 0xc00)
						{
							if (best_dist > dist)
							{
								best_dist   = dist;
								best_person = THING_array[i];
							}
						}
					}
					else
					{
						//
						// These are safe people...
						//
					}
				}

				if (best_person)
				{
					p_player->Genus.Player->Danger = best_dist / 0x400 + 1;

					SATURATE(p_player->Genus.Player->Danger, 1, 3);
				}
				else
				{
					p_player->Genus.Player->Danger = 0;
				}
		}
	}
}

#ifndef PSX
void	process_controls(void)
{
	SLONG i;
	SLONG j;
	SLONG x;
	SLONG z;

	Thing *darci = NET_PERSON(0);

	/*

	if (Keys[KB_R])
	{
		Keys[KB_R] = 0;

		void FACET_facetinfo_trace(void);

		FACET_facetinfo_trace();
	}

	*/


	/*
	{
		static werwer = FALSE;

		if (!werwer)
		{
			werwer = TRUE;

			extern void DCLL_looping_sample_conversion(void);

			DCLL_looping_sample_conversion();

		}
	}
	*/

//	if (Keys[KB_D])

	if (GAME_TURN <= 1)
	{
		Keys[KB_D] = 0;

		//
		// Find an MIB...
		//

		SLONG  list;
		SLONG  num_mibs = 0;
		Thing *p_thing;

		for (list = thing_class_head[CLASS_PERSON]; list; list = p_thing->NextLink)
		{
			p_thing = TO_THING(list);

			ASSERT(p_thing->Class == CLASS_PERSON);

			if (p_thing->Genus.Person->PersonType == PERSON_MIB1 ||
				p_thing->Genus.Person->PersonType == PERSON_MIB2 ||
				p_thing->Genus.Person->PersonType == PERSON_MIB3)
			{
				//
				// Found an mib...
				//

				num_mibs += 1;
			}
		}

		/*

		extern void convert_pointers_to_index(void);
		extern void convert_index_to_pointers(void);

		convert_pointers_to_index();
		convert_index_to_pointers();

		*/

	}

#ifndef TARGET_DC
	if (Keys[KB_D])
	{
		Keys[KB_D] = 0;

		SLONG is_there_room_behind_person(Thing *p_person, SLONG hit_from_behind);

		if (is_there_room_behind_person(darci, FALSE))
		{
			PANEL_new_text(NULL, 400, "There is room behind Darci");
		}

		//set_person_recoil(darci, ANIM_HIT_FRONT_HI, 0);
		//set_person_dead(darci, NULL, PERSON_DEATH_TYPE_LEG_SWEEP, 0, 0);
	}
#endif //#ifndef TARGET_DC


//	PANEL_new_text(NULL, 2000, "abcdefghijk lmnopqr stuvwxyz ABCDEFG HIJKLMNO PQRSTUVWXYZ 0123456789 !\"�$%^ &*(){} []<>\\/:;'@ #~?-=+.,");
//	PANEL_new_text(NULL, 2000, "a-b-c-d-e-f-g  h-i-j-k-l-m-n");

	#if THIS_IS_A_LOS_TEST

	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		x1 = (darci->WorldPos.X >> 8);
		y1 = (darci->WorldPos.Y >> 8) + 0x60;
		z1 = (darci->WorldPos.Z >> 8);

		x2 = (darci->WorldPos.X >> 8);
		y2 = (darci->WorldPos.Y >> 8) + 0x60;
		z2 = (darci->WorldPos.Z >> 8);

		x2 -= SIN(darci->Draw.Tweened->Angle) >> 6;
		z2 -= COS(darci->Draw.Tweened->Angle) >> 6;

		SLONG colour;

		if (there_is_a_los(
				x1, y1, z1,
				x2, y2, z2,
				0))
		{
			colour = 0xffffff;
		}
		else
		{
			colour = 0xbb0000;
		}

		AENG_world_line(
			x1, y1, z1,
			32,
			colour || 0x111111,
			x2, y2, z2,
			16,
			colour,
			FALSE);
	}

	#endif

#ifndef TARGET_DC
	if (allow_debug_keys)
	{
		/*

		if (Keys[KB_B])
		{
			Keys[KB_B] = 0;

			extern UWORD last_fudge_camera;

			for (i = 0; i < EWAY_way_upto; i++)
			{
				last_fudge_camera += 1;

				if (last_fudge_camera >= EWAY_way_upto)
				{
					last_fudge_camera = 1;
				}

				if (EWAY_way[last_fudge_camera].ed.type == EWAY_DO_CAMERA_CREATE)
				{
					extern void EWAY_set_active(EWAY_Way *ew);

					EWAY_set_active(&EWAY_way[last_fudge_camera]);

					break;
				}
			}
		}

		if (Keys[KB_M] && !ShiftFlag)
		{
			Keys[KB_M] = 0;

			extern UWORD last_fudge_message;

			SLONG old_flag;

			for (i = 0; i < EWAY_way_upto; i++)
			{
				last_fudge_message += 1;

				if (last_fudge_message >= EWAY_way_upto)
				{
					last_fudge_message = 1;
				}

				if (EWAY_way[last_fudge_message].ed.type == EWAY_DO_MESSAGE ||
					EWAY_way[last_fudge_message].ed.type == EWAY_DO_CONVERSATION)
				{
					extern void EWAY_set_active(EWAY_Way *ew);

					old_flag = EWAY_way[last_fudge_message].flag;

					EWAY_set_active(&EWAY_way[last_fudge_message]);

					EWAY_way[last_fudge_message].flag = old_flag;

					break;
				}
			}
		}

		*/
	}
#endif //#ifndef TARGET_DC


#if 0
	if ((GAME_TURN & 0x7f) == 16)
	{
		PANEL_new_text(
			NULL,
			8000,
			"abcdefghijklmnopqrstuvw "
			"xyzABCDEFGHIJKLMNOPQRS "
			"TUVWXYZ0123456789!\"#$ "
			"%^&*(){}[]<>\\/:;'@#_?-=+., "
			"�� ABC ����� Abc ���� "
			"������ abc ��� abc ����� "
			"��� abc ���� abc ���� "
 			"�� ABC ����� abc ����");
	}
#endif

	if ((GAME_TURN & 0x0f)==0)
		set_danger_level();
	context_music();

	if(!CNET_network_game)
	if (darci->State == STATE_DEAD)
	{
		if (darci->Genus.Person->Flags & FLAG_PERSON_ARRESTED)
		{
			//
			// Wait till she is actually arrested!
			//

			if (darci->SubState == SUB_STATE_DEAD_ARRESTED)
			{
				GAME_STATE = GS_LEVEL_LOST;
			}
		}
		else
		{
			GAME_STATE = GS_LEVEL_LOST;
		}
	}
#ifndef TARGET_DC
	if (allow_debug_keys)
	{
		static	SLONG index_cam=0;
		Thing	*p_thing;


		if (Keys[KB_RBRACE])
		{
			Keys[KB_RBRACE]=0;
			while(1)
			{
				index_cam++;
				if(index_cam>=MAX_THINGS)
					index_cam=0;
				p_thing=TO_THING(index_cam);
				if(p_thing->Class==CLASS_PERSON && (p_thing->Genus.Person->PersonType!=PERSON_CIV))
				{
					FC_look_at(1, index_cam);
					FC_setup_initial_camera(1);
					break;

				}
			}

		}
		if (Keys[KB_LBRACE])
		{
			Keys[KB_LBRACE]=0;
			while(1)
			{
				index_cam--;
				if(index_cam<0)
					index_cam=MAX_THINGS-1;
				p_thing=TO_THING(index_cam);
				if(p_thing->Class==CLASS_PERSON && (p_thing->Genus.Person->PersonType!=PERSON_CIV))
				{
					FC_look_at(1, index_cam);
					FC_setup_initial_camera(1);
					break;

				}
			}

		}

		if(Keys[KB_P])
		{
			Keys[KB_P]=0;

			if(FC_cam[1].focus)
			{
				FC_cam[1].focus=NULL;
			}
			else
			{

				p_thing=TO_THING(index_cam);
				if(p_thing->Class==CLASS_PERSON && (p_thing->Genus.Person->PersonType!=PERSON_CIV))
				{
					FC_look_at(1, index_cam);
					FC_setup_initial_camera(1);
				}

			}
		}
#ifndef	NDEBUG
		SLONG is_there_room_behind_person(Thing *p_person, SLONG hit_from_behind);

		if (Keys[KB_U])
		{
			Keys[KB_U] = 0;

			if (is_there_room_behind_person(darci, FALSE))
			{
				PANEL_new_text(darci, 1000, "Enough room behind me.");
			}
			else
			{
				PANEL_new_text(darci, 1000, "No room behind me.");
			}
		}
#endif
	}
#endif //#ifndef TARGET_DC


	#if WE_NEED_THE_NUMBER_KEYS

	if (Keys[KB_7])
	{
		Keys[KB_7] = 0;

		PARTICLE_Add(
			darci->WorldPos.X,
			darci->WorldPos.Y + 0x5000,
			darci->WorldPos.Z,
		   -SIN(darci->Draw.Tweened->Angle) >> 2,
			0,
		   -COS(darci->Draw.Tweened->Angle) >> 2,
			POLY_PAGE_FLAMES2,
			2 + ((Random() & 0x3) << 2),
			0xffffffff,
			PFLAG_SPRITEANI | PFLAG_SPRITELOOP  | PFLAG_EXPLODE_ON_IMPACT | PFLAG_GRAVITY | PFLAG_LEAVE_TRAIL,
			100,
			160,
			1,
			1,
			1);

		/*

		static pow_size = POW_CREATE_MEDIUM;

		pow_size += 1;
		pow_size %= 3;

		POW_create(
			pow_size,
			darci->WorldPos.X - SIN(darci->Draw.Tweened->Angle),
			darci->WorldPos.Y,// + 0x8000,
			darci->WorldPos.Z - COS(darci->Draw.Tweened->Angle));

		*/

		/*

		BAT_create(
			BAT_TYPE_BAT,
			darci->WorldPos.X >> 8,
			darci->WorldPos.Z >> 8);

		*/

		/*

		{
			SLONG	index;
			index=BAT_create(
				BAT_TYPE_GARGOYLE,
				darci->WorldPos.X >> 8,
				darci->WorldPos.Z >> 8,0);
			TO_THING(index)->Genus.Bat->state=8;
		}

		*/

//		ROAD_calc_mapsquare_type();
	}

	#endif

	/*

	if ((GAME_TURN & 0xffff) == 0)
	{
		SLONG look = ROAD_get_mapsquare_type(darci->WorldPos.X >> 16, darci->WorldPos.Z >> 16);

		CBYTE *look_names[] =
		{
			"stone",
			"grass",
			"dirt",
			"slippery",
		};

		PANEL_new_text(darci, 250, "Texture feels like %s", look_names[look]);
	}

	*/

/*
	static DIKE_Dike *dd = NULL;

	if (dd == NULL || Keys[KB_7])
	{
		Keys[KB_7] = 0;

		DIKE_init();

		dd = DIKE_create(
				darci->WorldPos.X >> 8,
				darci->WorldPos.Z >> 8,
				darci->Draw.Tweened->Angle);
	}

	if (dd)
	{
		dd->control = 0;

		if (Keys[KB_PSLASH  ]) {dd->control |= DIKE_CONTROL_LEFT;}
		if (Keys[KB_ASTERISK]) {dd->control |= DIKE_CONTROL_RIGHT;}
		if (Keys[KB_P8      ]) {dd->control |= DIKE_CONTROL_ACCEL;}

		DIKE_process(dd);
		DIKE_draw   (dd);
	}
*/
	/*

	if (Keys[KB_7])
	{
		Keys[KB_7] = 0;

		remove_thing_from_map(darci);
	}

	*/
/*
SLONG is_person_crouching(Thing *p_person);

	if (is_person_crouching(darci))
	{
		AENG_world_line(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8,
			32,
			0xffffff,
			darci->WorldPos.X >> 8,
			(darci->WorldPos.Y >> 8) + 0x80,
			darci->WorldPos.Z >> 8,
			0,
			0x111111,
			TRUE);
	}
*/
	// this stuff shouldn't even _be_ in process_controls.

#ifdef	PSX
	DIRT_set_focus(darci->WorldPos.X>>8, darci->WorldPos.Z>>8, 0x800);
#else
	DIRT_set_focus(darci->WorldPos.X>>8, darci->WorldPos.Z>>8, 0xc00);
#endif
	MIST_process();
//	WATER_process();
//	BANG_process();
	SPARK_process();
	GLITTER_process();
//	LIGHT_process();
	HOOK_process();
//	SM_process();
#ifdef	OLD_CAM
	CAM_process();
#endif
#ifndef TARGET_DC
	SNIPE_process();
#endif

	GameCoord position;
	Thing	 *t_thing;



	// console processing


	static BOOL  is_inputing=0;
	extern UBYTE InkeyToAscii[];
	extern UBYTE InkeyToAsciiShift[];
	extern void CONSOLE_status(CBYTE *msg);

	if (is_inputing) {
		static CBYTE input_text[MAX_PATH]="] ";

		if ((Keys[KB_ESC])||(Keys[KB_ENTER])) {
			if (Keys[KB_ENTER]) parse_console(input_text+2); // +2 to skip the "] "
			Keys[KB_ESC]=0;
			Keys[KB_ENTER]=0;
			is_inputing=0;
			CONSOLE_status("");
			strcpy(input_text,"] ");
		} else {

			POLY_frame_init(FALSE, FALSE);

			if (LastKey) {
				UWORD len=strlen(input_text);
				CBYTE key;
			    key= (Keys[KB_LSHIFT]||Keys[KB_RSHIFT]) ?  InkeyToAsciiShift[LastKey] : InkeyToAscii[LastKey] ;
				if (key==8) {
					if (len>2) input_text[len-1]=0;
				} else {
					if (((key>='A')&&(key<='Z'))||((key>='a')&&(key<='z'))||((key>='0')&&(key<='9'))||(key==' ')) {
						input_text[len]=key;
						input_text[len+1]=0;
					}
				}
				LastKey=0;
			}

			CONSOLE_status(input_text);
			POLY_frame_draw(FALSE, FALSE);

		}
		return;
	} else {
#ifndef VERSION_DEMO
	  if (Keys[KB_F9]) is_inputing=1;
#endif
	}


	// normal key processing

	//
	// Do keyboard inventory.
	//

	extern Form *form_leave_map;
	extern SLONG can_darci_change_weapon(Thing *p_person);

	if ((!(GAME_FLAGS & GF_PAUSED) && !form_leave_map) && can_darci_change_weapon(darci))
	{
		Thing *the_player = NET_PLAYER(0);


//		if (can_darci_change_weapon(darci))
		{
/*
			if (NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_SELECT) {
				CONTROLS_inventory_mode^=1;
			}
*/

			if (NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_SELECT)
			{
				CONTROLS_new_inventory(darci, the_player);

				if(CONTROLS_inventory_mode==0 && darci->Genus.Person->SpecialUse==0 &&  !(darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) && the_player->Genus.Player->ItemFocus==0)
				{
					//
					// currently unarmed so pick best weapon
					//

					the_player->Genus.Player->ItemFocus = CONTROLS_get_best_item(darci, the_player);
					if(the_player->Genus.Player->ItemFocus)
					{
						CONTROLS_set_inventory(darci,the_player,the_player->Genus.Player->ItemFocus);
					}
					else
						CONTROLS_rot_inventory(darci,the_player, 1,1);
				}
				else
					CONTROLS_rot_inventory(darci,the_player, 1,1);
				CONTROLS_inventory_mode=3000;
			}

			if (CONTROLS_inventory_mode)
			{
//				Keys[KB_ENTER] = 0;

				//
				// tick down the panel display
				//
				CONTROLS_inventory_mode-=TICK_TOCK;
				if(CONTROLS_inventory_mode<0)
					CONTROLS_inventory_mode=0;


				Thing *p_special = NULL;

				if (!ShiftFlag)
				{

					//
					// does the fade in
					//
					CONTROLS_new_inventory(darci, the_player);
/*
					if ((NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_LEFT)||(NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_FORWARDS))
					{
//						NET_PLAYER(0)->Genus.Player->Pressed &= ~(INPUT_MASK_LEFT|INPUT_MASK_FORWARDS);
						CONTROLS_rot_inventory(darci,the_player,-1,0);
						CONTROLS_inventory_mode=500;
					}
					if ((NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_RIGHT)||(NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_BACKWARDS))
					{
//						NET_PLAYER(0)->Genus.Player->Pressed &= ~(INPUT_MASK_RIGHT|INPUT_MASK_BACKWARDS);
						CONTROLS_rot_inventory(darci,the_player, 1,0);
						CONTROLS_inventory_mode=500;
					}
*/

				}
				else
				{
					//
					// Shift + Return == Drop current weapon.
					//
					set_person_gun_away(darci);
					drop_current_gun(darci,1);
				}
			}
		}
/*		else
		{
			CONTROLS_inventory_mode=0;
		}*/

//		if (the_player->Genus.Player->PopupFade&&!(NET_PLAYER(0)->Genus.Player->ThisInput & INPUT_MASK_SELECT)) {

		if (the_player->Genus.Player->PopupFade&&!CONTROLS_inventory_mode)
		{
			the_player->Genus.Player->PopupFade-=INVENTORY_FADE_SPEED;
			if (the_player->Genus.Player->ItemFocus>-1)
			{
//				CONTROLS_set_inventory(darci,the_player);
				the_player->Genus.Player->ItemFocus = -1;
			}
		}


	}

#ifndef TARGET_DC
	if(Keys[KB_F12]&&ShiftFlag)
	{
		Keys[KB_F12]=0;

extern	UBYTE	cheat;

		if(cheat)
		{
			cheat=0;
		}
		else
		{
			cheat=2;
		}


	}
#endif //#ifndef TARGET_DC


	if (!allow_debug_keys) return;

#ifndef TARGET_DC

	if(mouse_input)
	{
		//
		// put the mouse in the center of the screen so we can always get a mousedx,mousedy
		//
		RecenterMouse();
	}

	if(Keys[KB_F3])
	{
void	save_whole_game(CBYTE	*gamename);
void	load_whole_game(CBYTE	*gamename);

		Keys[KB_F3]=0;
		if(ShiftFlag)
		{
			save_whole_game("poo.sav");
		}
		else
		{
			load_whole_game("poo.sav");
		}
	}

	if (Keys[KB_F11]) {
		Keys[KB_F11]=0;
		if (aeng_draw_cloud_flag) {
			aeng_draw_cloud_flag=0;
			CONSOLE_text("clouds off");
		} else {
			aeng_draw_cloud_flag=1;
			CONSOLE_text("clouds on");
		}
	}

	WARE_debug();

	if(ControlFlag&&ShiftFlag)
	{
		/*

		MSG_add(" yomp %d sprint %d \n",yomp_speed,sprint_speed);

		if(Keys[KB_1])
			yomp_speed--;
		if(Keys[KB_2])
			yomp_speed++;
		if(Keys[KB_3])
			sprint_speed--;
		if(Keys[KB_4])
			sprint_speed++;

		*/

		if(yomp_speed<10)
			yomp_speed=10;

		if(sprint_speed>120)
			sprint_speed=120;

		if(yomp_speed>120)
			yomp_speed=120;


		if(yomp_speed>sprint_speed)
			sprint_speed=yomp_speed;
		return;
	}

	if(Keys[KB_TILD])
	{
		Keys[KB_TILD]=0;

		if(DETAIL_LEVEL)
			DETAIL_LEVEL=0;
		else
			DETAIL_LEVEL=0xffff;

	}

	//
	// Mark's stuff.
	//

//	Thing *darci = NET_PERSON(0);

	/*

	//
	// Is darci still?
	//

	if (darci->State == STATE_IDLE)
	{
		if (darci->Draw.Tweened->CurrentAnim != ANIM_ROTATE_L &&
			darci->Draw.Tweened->CurrentAnim != ANIM_ROTATE_R)
		{
			AENG_world_line(
				darci->WorldPos.X >> 8,
				darci->WorldPos.Y >> 8,
				darci->WorldPos.Z >> 8,
				32, 0xffffff,
				darci->WorldPos.X          >> 8,
				darci->WorldPos.Y + 0x8000 >> 8,
				darci->WorldPos.Z          >> 8,
				0, 0xffffff,
				TRUE);
		}
	}

	if (darci->SubState == SUB_STATE_CRAWLING)
	{
		AENG_world_line(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8,
			32, 0xff0000,
			darci->WorldPos.X          >> 8,
			darci->WorldPos.Y + 0x8000 >> 8,
			darci->WorldPos.Z          >> 8,
			0, 0xff0000,
			TRUE);
	}

	*/


void	set_person_idle(Thing *p_person);

	if(Keys[KB_P])
	{
void	save_whole_game(CBYTE	*gamename);
		save_whole_game("save.me");

	}

	/*

	if(Keys[KB_1])
	{
extern	UBYTE	anim_type[];

		darci->Genus.Person->PersonType =  PERSON_DARCI;
		darci->Genus.Person->AnimType =  ANIM_TYPE_DARCI;
		darci->Draw.Tweened->TheChunk   = &game_chunk[ANIM_TYPE_DARCI];
		set_person_idle(darci);
		darci->Draw.Tweened->PersonID=0;
		darci->Draw.Tweened->MeshID	= 0;
//		if(darci->Draw.Tweened->PersonID>19)
//			darci->Draw.Tweened->PersonID=0;


//darci->Genus.Person->Flags&=~FLAG_PERSON_OTHERHAND;
void FC_look_at(SLONG cam, UWORD thing_index);


		FC_look_at(0,THING_NUMBER(darci));

//		yomp_speed=50;
//		sprint_speed=80;
	}

	*/


	/*

	if(Keys[KB_2])
	{
		darci->Genus.Person->PersonType =  PERSON_ROPER;
		darci->Genus.Person->AnimType =  ANIM_TYPE_ROPER;
		darci->Draw.Tweened->TheChunk	= &game_chunk[ANIM_TYPE_ROPER];
		darci->Draw.Tweened->MeshID	= 0;
		darci->Draw.Tweened->PersonID=0;
		set_person_idle(darci);
//darci->Genus.Person->Flags|=FLAG_PERSON_OTHERHAND;
		FC_look_at(0,THING_NUMBER(darci));
//		yomp_speed=36;
//		sprint_speed=56;
	}

	*/

	static UBYTE playing=0;

/*	if (playing&&(Keys[KB_3]||Keys[KB_4]||Keys[KB_5])) {
		MFX_stop(THING_NUMBER(darci),MFX_WAVE_ALL);
		playing=0;
		Keys[KB_3]=Keys[KB_4]=Keys[KB_5]=0;
	}*/

/*	if (Keys[KB_3]) play_music(S_TEST_MUSIC_START,0);
	if (Keys[KB_4]) play_music(S_TEST_MUSIC_START+1,0);*/

	/*
	if (Keys[KB_5]) {
		Keys[KB_5]=0;
		MUSIC_play(S_TEST_MUSIC_START,MUSIC_FLAG_QUEUED);
	}

	*/

/*	if (Keys[KB_3])
	{
		Keys[KB_3] = 0;

//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_CHAOS,MFX_REPLACE|MFX_LOOPED,darci);
//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_DARCI_BASS,MFX_REPLACE|MFX_LOOPED,darci);
		play_music(S_TUNE_DARCI_BASS,1);
		playing=1;
	}

	if (Keys[KB_4])
	{
		Keys[KB_4] = 0;

//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_CLUB,MFX_REPLACE|MFX_LOOPED,darci);
//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_DARCI_CHASE,MFX_REPLACE|MFX_LOOPED,darci);
		play_music(S_TUNE_DARCI_CHASE,0);
		playing=1;
	}

	if (Keys[KB_5])
	{
		Keys[KB_5] = 0;

//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_DANGER,MFX_REPLACE|MFX_LOOPED,darci);
//		MFX_play_thing(THING_NUMBER(darci),S_TUNE_DARCI_TROUBLE,MFX_REPLACE|MFX_LOOPED,darci);
		play_music(S_TUNE_DARCI_TROUBLE,0);
		playing=1;
	}*/
/*
	// auto music scheduling test...
	switch (NET_PLAYER(0)->Genus.Player->Danger) {
	case 0: // no danger warning
		break;
	case 3: // green, mild danger
		play_music(S_TUNE_DARCI_BASS,1);
		break; // no music
	case 2: // yellow
//		play_music(S_TUNE_DARCI_BASS,1);
		play_music(S_TUNE_DARCI_BASS,1);
		play_music(S_TUNE_DARCI_CHASE,0);
		break;
	case 1: // red
		play_music(S_TUNE_DARCI_BASS,1);
		play_music(S_TUNE_DARCI_TROUBLE,0);
		break;
	}
	*/
/*
	switch (NET_PLAYER(0)->Genus.Player->Danger)
	{
		case 0: // no danger warning
			break;
		case 3: // green, mild danger
			play_music(S_TUNE_DARCI_BASS,1);
			break; // no music
		case 2: // yellow
			play_music(S_TUNE_DARCI_SLOW,1);
			break;
		case 1: // red
			play_music(S_TUNE_DARCI_SPOOKY,1);
			break;
	}
*/




//	TRACE("danger numbers: %d\n",(NET_PLAYER(0)->Genus.Player->Danger));

	if (!ShiftFlag)
	{
		if (Keys[KB_J])
		{
			SLONG mx = darci->WorldPos.X >> 16;
			SLONG mz = darci->WorldPos.Z >> 16;

			MAV_draw(
				mx - 5, mz - 5,
				mx + 5, mz + 5);
		}

		if (Keys[KB_I])
		{
			SLONG mx = darci->WorldPos.X >> 16;
			SLONG mz = darci->WorldPos.Z >> 16;

			WAND_draw(mx,mz);
		}
/*
		if (Keys[KB_LBRACE])
		{
			Keys[KB_LBRACE] = 0;

			ERROR_MSG(FALSE, NULL);
		}
*/
	}

	/*

	//
	// Hook control.
	//

	if (HOOK_get_state() == HOOK_STATE_SPINNING)
	{
		static SLONG spin_speed = 0;

		if (ControlFlag) {spin_speed += 15;}

		spin_speed -= 5;

		SATURATE(spin_speed, 64, 154);

		SLONG px;
		SLONG py;
		SLONG pz;

		calc_sub_objects_position(
			darci,
			darci->Draw.Tweened->AnimTween,
			SUB_OBJECT_RIGHT_HAND,
		   &px,
		   &py,
		   &pz);

		px += darci->WorldPos.X >> 8;
		py += darci->WorldPos.Y >> 8;
		pz += darci->WorldPos.Z >> 8;

		HOOK_spin(
			px,
			py,
			pz,
			darci->Draw.Tweened->Angle,
			spin_speed);

		if (Keys[KB_F])
		{
			Keys[KB_F] = 0;

			HOOK_release();
		}
	}

	*/

	/*

	if(!(GAME_FLAGS & GF_SHOW_MAP))
	{
		if(Keys[KB_TAB])
		{
			Keys[KB_TAB]=0;
//			InitBackImage(tab_map_name);
			InitBackImage("handhold.tga");
			GAME_FLAGS|=GF_SHOW_MAP;

		}
	}

	*/

	if(Keys[KB_E])
	{
		SLONG y;
		SLONG index;

		static UBYTE type = 0;

		Keys[KB_E] = 0;

		y     = PAP_calc_height_at(darci->WorldPos.X>>8,darci->WorldPos.Z>>8);
		index = VEH_create((darci->WorldPos.X)+(400<<8),(y+65)<<8,(darci->WorldPos.Z),0,0,0,type,0,0);

		WMOVE_create(TO_THING(index));

		type += 1;

		if (type == VEH_TYPE_NUMBER)
		{
			type = 0;
		}
	}

	/*

	if (Keys[KB_B])
	{
		Keys[KB_B] = 0;

			extern SLONG NIGHT_specular_enable;
			NIGHT_specular_enable ^= TRUE;
	}

	*/

#ifdef	PSX
		{
			static	create_chopper=0;
//			if(PadKeyIsPressed(&PAD_Input2,PAD_FRT)&&(create_chopper==0))
//				create_chopper=1;
//			else
				create_chopper=0;
			if(create_chopper==1)
			{
				Thing *chopper;
				GameCoord posn;

				Keys[KB_O] = 0;

				posn.X=darci->WorldPos.X;
				posn.Z=darci->WorldPos.Z;
	//			posn.Y=darci->WorldPos.Y+(265<<8)+(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8)<<8);
				posn.Y=darci->WorldPos.Y+(265<<8)+(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8));
				chopper=CHOPPER_create(posn,CHOPPER_CIVILIAN);
				chopper->Draw.Mesh->Angle=darci->Draw.Tweened->Angle;
				create_chopper=2;
			}

		}
#endif

#ifndef	PSX
//	CLOTH_process();
#endif

	//
	// Enter and leave the sewers if Darci does.
	//
/*	if (darci->Flags & FLAGS_IN_SEWERS)
	{
		if (!(GAME_FLAGS & GF_SEWERS))
		{
			GAME_FLAGS |= GF_SEWERS;

			DIRT_init(0, 0, 0, INFINITY, INFINITY, INFINITY, INFINITY);
			NIGHT_destroy_all_cached_info();
			NS_cache_init();
		}
	}
	else
	{
		if (GAME_FLAGS & GF_SEWERS)
		{
			GAME_FLAGS &= ~GF_SEWERS;

			NS_cache_fini();
			DIRT_init(100, 3, 3, INFINITY, INFINITY, INFINITY, INFINITY);

			//
			// Reposition the camera at a decent place to watch Darci exit the sewers.
			//
#ifdef	OLD_CAM
			CAM_set_to_leave_sewers_position(darci);
#endif
		}
	}*/

	if(Keys[KB_W])
	{
		SLONG px = darci->WorldPos.X >> 8;
		SLONG py = darci->WorldPos.Y >> 8;
		SLONG pz = darci->WorldPos.Z >> 8;

		px += 0x80;
		py += 0x10;
		pz += 0x80;

		if (ControlFlag)
		{
			DIRT_new_water(px,py,pz, 1+(SIN(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12),14,   (COS(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12));
			DIRT_new_water(px,py,pz, 1+(SIN(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12),14,-1+(COS(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12));
			DIRT_new_water(px,py,pz, 1+(SIN(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12),14, 1+(COS(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12));
			DIRT_new_water(px,py,pz,-1+(SIN(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12),14,   (COS(darci->Draw.Tweened->Angle+((GAME_TURN*7)&15))>>12));
		}
		else
		{
			DIRT_new_water(px + 2, py, pz,     -1, 28,  0);
			DIRT_new_water(px    , py, pz + 2,  0, 29, -1);
			DIRT_new_water(px    , py, pz - 2,  0, 28, +1);
			DIRT_new_water(px - 2, py, pz,     +1, 29,  0);
		}
	}

	static UBYTE hm_1 = 255;
	static UBYTE hm_2 = 255;

	/*
	{
		SLONG process;

		//
		// How many times to process?
		//

		#define AT_TWENTY ((ShiftFlag) ? 4 : 8)

		process = AT_TWENTY * TICK_RATIO >> TICK_SHIFT;

		for (SLONG i = 0; i < process; i++)
		{
			HM_process();
			SM_process();
			//HM_collide_all();

		 	//if (hm_1 != 255 &&
			//	hm_2 != 255)
			//{
			//	HM_collide(hm_2, hm_1);
			//}
		}
	}
	*/

	#if WE_WANNA_TEST_THE_LOS_STUFF

	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;
#ifdef	OLD_CAM
		CAM_get_pos(&x1, &y1, &z1);
#endif

		calc_sub_objects_position(
			darci,
			darci->Draw.Tweened->AnimTween,
			SUB_OBJECT_HEAD,
		   &x2,
		   &y2,
		   &z2);

		x2 += darci->WorldPos.X >> 8;
		y2 += darci->WorldPos.Y >> 8;
		z2 += darci->WorldPos.Z >> 8;

		if (there_is_a_los(x2, y2, z2, x1, y1, z1))
		{
			AENG_world_line(
				x1, 0, z1, 16, 0x00ffffff,
				x2, 0, z2, 16, 0x00dddddd,
				TRUE);
		}
		else
		{
			AENG_world_line(
				x1, 0, z1, 16, 0x00eeee00,
				x2, 0, z2, 16, 0x00dddd00,
				TRUE);

			AENG_world_line(
				x2, 12, z2, 8, 0x00866dd,
				los_failure_x, 12, los_failure_z, 8, 0x00ff22ee,
				TRUE);

		}
	}

	#endif

	/*

	if (Keys[KB_4]) {HM_draw();}

	*/

	if (GAME_FLAGS & GF_INDOORS)
	{
		/*

		//
		// Is darci ready to go upstairs?
		//

		SLONG dfloor;
		SLONG new_floor;

		SLONG new_x;
		SLONG new_z;
		SLONG new_storey;

		dfloor = ID_change_floor(
					darci->WorldPos.X >> 8,
					darci->WorldPos.Z >> 8,
				   &new_x,
				   &new_z,
				   &new_storey);

		if (dfloor)
		{
			//
			// Delete the old furniture and colvects.
			//

			ID_remove_inside_things();
			ID_wall_colvects_remove();

			//
			// Enter the building on a new floor.
			//

			if (ENTER_setup(INDOORS_BUILDING, new_storey, TRUE))
			{
				GameCoord newpos;

				//
				// Move Darci to the new position.
				//

				newpos.X = new_x<<8;
				newpos.Y = INDOORS_HEIGHT_FLOOR<<8;
				newpos.Z = new_z<<8;

				move_thing_on_map(darci, &newpos);

				TRACE("Moving floor %s\n", (dfloor > 0) ? "up" : "down");
			}
		}

		*/
	}

	static THING_INDEX fti = 0;

	//
	// Tell the ID module where the player is, so that
	// it can draw the correct rooms.
	//

//	ID_this_is_where_i_am((darci->WorldPos.X>>8) >> ELE_SHIFT, (darci->WorldPos.Z>>8) >> ELE_SHIFT);

	static GameCoord enter_pos = {0,0,0};	// Where Darci entered the building.

	#if WE_WANT_WIND
	{
		static SLONG wind_counter = 100;
		static SLONG wind_updown  = 0;
		static SLONG wind_speed = 0;
		static SLONG wind_max = 0;
		static SLONG wind_sign_dx = 0;
		static SLONG wind_sign_dz = 0;

		switch(wind_updown)
		{
			case 0:
				if (wind_counter > 0)
				{
					wind_counter -= 1;
				}
				else
				{
					wind_counter = rand() & 0xff;
					wind_max     = rand() & 0x7f;
					wind_updown  = 1;
					wind_speed   = 0;
/*					wind_sign_dx = (rand() & 0x1) ? +1 : -1;
					wind_sign_dz = (rand() & 0x1) ? +1 : -1;*/
					wind_sign_dx=wind_sign_dz=1;
				}
				break;

			case 1:
				if (wind_speed < wind_max)
				{
					wind_speed += 1;
				}

				if (wind_counter > 0)
				{
					wind_counter -= 1;
				}
				else
				{
					wind_counter = 0;
					wind_updown  = 2;
				}
				break;

			case 2:
				if (wind_speed == 0)
				{
					wind_counter  = rand() & 0x1ff;
					wind_counter += 100;
					wind_updown   = 0;
				}
				else
				{
					wind_speed -= 1;
				}
				break;

			default:
				wind_updown  = 0;
				wind_counter = 0xff;
				wind_speed   = 0;
		}

		SLONG wind_dx = (wind_speed >> 3) * wind_sign_dx;
		SLONG wind_dz = (wind_speed >> 4) * wind_sign_dz;

		DIRT_gale(
			wind_dx,
			wind_dz);

#ifdef WE_WANT_SOME_BOLLOCKS_OLD_WIND_SOUND_FX_THATS_BEEN_OBSOLETE_FOR_OVER_A_YEAR
		if (wind_speed > 32)
		{
			SLONG			wind_wave;
			static SLONG	wind_ref=0;
/*			WaveParams		wind;

			wind_wave					=	((Random()*(S_WIND_END-S_WIND_START))>>16)+S_WIND_START;
			wind.Flags					=	WAVE_CARTESIAN|WAVE_DISTANCE_MAPPING|WAVE_PAN_RATE;
			wind.Priority				=	0;
			wind.Mode.Cartesian.Scale	=	(128<<8);
			wind.Mode.Cartesian.X		=	NET_PERSON(0)->WorldPos.X;
			wind.Mode.Cartesian.Y		=	NET_PERSON(0)->WorldPos.Y;
			wind.Mode.Cartesian.Z		=	NET_PERSON(0)->WorldPos.Z;

			PlayWave(WIND_REF,wind_wave,WAVE_PLAY_NO_INTERUPT,&wind);*/

			// PlayWave is QSound specific, whereas play_quick[...] adjust to
			// QSound or A3D automatically
			wind_ref=play_quick_wave_xyz(
				NET_PERSON(0)->WorldPos.X,
				NET_PERSON(0)->WorldPos.Y,
				NET_PERSON(0)->WorldPos.Z,
				wind_wave,
				wind_ref,WAVE_PLAY_NO_INTERRUPT);
		}
#endif
	}

	#ifdef WE_WANT_MANUAL_WIND_ASWELL
	{
		//
		// Wind...
		//

		static SLONG wind_dx = 0;
		static SLONG wind_dz = 0;

		if (Keys[KB_LBRACE]) {Keys[KB_LBRACE] = 0; wind_dx -= 2; wind_dz -= 1;}
		if (Keys[KB_RBRACE]) {Keys[KB_RBRACE] = 0; wind_dx += 2; wind_dz += 1;}

		DIRT_gale(
			wind_dx,
			wind_dz);

		SLONG eow = abs(wind_dx) + abs(wind_dz);

		if (eow > 3)
		{
			if ((GAME_TURN & 0x7) == 5)
			{
				SPARK_in_sphere(
					darci->WorldPos.X >> 8,
					darci->WorldPos.Y >> 8,
					darci->WorldPos.Z >> 8,
					0x400,
					eow,
					eow >> 1);
			}
#ifdef	OLD_CAM
			CAM_set_shake(eow >> 1);
#endif
		}
		else
		{
#ifdef	OLD_CAM
			CAM_set_shake(0);
#endif
		}
	}
	#endif
	#endif

#ifdef WE_WANT_SHITTY_PANTS_WIND

	// NEW WIND -- only for snow level, to blow snow around. Blizzard code!
	if (world_type==WORLD_TYPE_SNOW)
	{

		static SLONG wind_speed = 0;
		static SLONG wind_counter=0;

//		if (--wind_counter<0)
		{

/*			wind_speed = (GAME_TURN & 2047);

			wind_speed=(SIN(wind_speed)>>1)+127;*/

			wind_speed=0x2f+(Random()&0x3f);

			SLONG wind_dx = (wind_speed >> 3) ;
			SLONG wind_dz = (wind_speed >> 4) ;

			wind_counter=(Random()&0x3f);

/*			DIRT_gale(
				wind_dx,
				wind_dz);*/
void DIRT_gale_height(SLONG dx,SLONG dy,SLONG dz);

			DIRT_gale_height(
				wind_dx,
				0xff,
				wind_dz);

		}

	}
#endif

#ifndef	PSX
#ifndef TARGET_DC

/*

	if (Keys[KB_P4])
	{
		static SLONG startx = 0;
		static SLONG startz = 0;
		static SLONG startvalid = 0;

		static SLONG endx = 0;
		static SLONG endz = 0;
		static SLONG endvalid = 0;

		Keys[KB_P4] = 0;

		if (!startvalid)
		{
			startx     = darci->WorldPos.X >> 8;
			startz     = darci->WorldPos.Z >> 8;
			startvalid = TRUE;
		}
		else
		if (!endvalid)
		{
			endx     = darci->WorldPos.X >> 8;
			endz     = darci->WorldPos.Z >> 8;
			endvalid = TRUE;
		}

		if (startvalid && endvalid)
		{
			startvalid = FALSE;
			endvalid   = FALSE;

			TRIP_create(
				darci->WorldPos.Y + 0x4000 >> 8,
				startx,
				startz,
				endx,
				endz);
		}
	}

	*/

	// pyrotest
	{
	static SLONG ribbon_id=-1;
	static UBYTE which_pyro=0;
	static SLONG line=0;
	GameCoord posn;

	if (Keys[KB_P7]) {
		CBYTE *names[] = { "flicker", "ribbon", "explosion", "sparklies", "bonfire", "immolate", "testrib", "firewall", "new sploje", "new dome", "whoomph" };
		Keys[KB_P7]=0;
		which_pyro++;
		if (which_pyro==(sizeof(names)>>2)) which_pyro=0;
		CONSOLE_text(names[which_pyro],1500);
	}

	if (Keys[KB_P5]) {
		static UBYTE line = 0;
		static GameCoord oldposn = {0,0,0};
		Thing *pyro;

		Keys[KB_P5]=0;
		posn.X=darci->WorldPos.X;
		posn.Z=darci->WorldPos.Z;
		posn.Y=(PAP_calc_height_at(posn.X>>8,posn.Z>>8)*256);

		switch(which_pyro) {
		case 0:
			posn.X-=32000;
			PYRO_create(posn,PYRO_FLICKER);
			break;
		case 1:
			if (ribbon_id==-1) ribbon_id=RIBBON_alloc(RIBBON_FLAG_FADE|RIBBON_FLAG_SLIDE|RIBBON_FLAG_CONVECT,20,POLY_PAGE_FLAMES3,-1,6,4);
			break;
		case 2:
			PYRO_construct(posn,14,256);
			PYRO_construct(posn,1,400);
			posn.X+=rand()>>1;
			posn.Z+=rand()>>1;
			PYRO_construct(posn,2,96+(rand()&0x3f));
			posn.X=darci->WorldPos.X;
			posn.Z=darci->WorldPos.Z;
			posn.X-=rand()>>1;
			posn.Z-=rand()>>1;
			PYRO_construct(posn,2,96+(rand()&0x3f));


			PCOM_oscillate_tympanum(
				PCOM_SOUND_BANG,
				darci,
				posn.X >> 8,
				posn.Y >> 8,
				posn.Z >> 8);

			break;
		case 3:
			PYRO_construct(posn,1,400);
			break;
		case 4:
			posn.X-=32000;
			PYRO_create(posn,PYRO_BONFIRE);
			break;
		case 5:
			pyro=PYRO_create(posn,PYRO_IMMOLATE);
			pyro->Genus.Pyro->victim=darci;
			pyro->Genus.Pyro->Flags=PYRO_FLAGS_FLICKER;
			darci->Genus.Person->BurnIndex=PYRO_NUMBER(pyro->Genus.Pyro)+1;
			break;
		case 6:
			posn.X-=32000;
			pyro=PYRO_create(posn,PYRO_FLICKER);
extern void PYRO_fn_init(Thing *thing);
			PYRO_fn_init(pyro); // heh heh heh
			pyro->Genus.Pyro->radii[0]=128;
			pyro->Genus.Pyro->radii[1]=400;
			pyro->Genus.Pyro->radii[2]=256;
			break;
		case 7:
			if (line) {
				pyro=PYRO_create(oldposn,PYRO_FIREWALL);
				pyro->Genus.Pyro->target=posn;
				line=0;
			} else {
				oldposn=posn;
				line=1;
			}
			break;

		case 8:
			//pyro=PYRO_create(posn,PYRO_SPLATTERY);
			pyro=PYRO_create(posn,PYRO_FIREBOMB);
			break;

		case 9:
			pyro=PYRO_create(posn,PYRO_NEWDOME);
			if (pyro)
			{
				pyro->Genus.Pyro->scale=400;
			}
			break;

		case 10:
			posn.X-=32000;
			PYRO_create(posn,PYRO_WHOOMPH);
			break;

		}


/*		Keys[KB_P5]=0;
		if (line) {
			posn.X=darci->WorldPos.X;
			posn.Z=darci->WorldPos.Z;
			posn.Y=(PAP_calc_height_at(posn.X>>8,posn.Z>>8)<<8)+0x3000;
			pyro=PYRO_create(oldposn,PYRO_FIREWALL);
			pyro->Genus.Pyro->target=posn;
			line=0;
		} else {
			oldposn.X=darci->WorldPos.X;
			oldposn.Z=darci->WorldPos.Z;
			oldposn.Y=(PAP_calc_height_at(oldposn.X>>8,oldposn.Z>>8)<<8)+0x3000;
			line=1;

		}*/
	}
	if (ribbon_id!=-1) {
		static ribbon_tick=0;
		ribbon_tick++;
		if (ribbon_tick&1) {
			SLONG dx,dz,ang;
			ang=(-darci->Draw.Tweened->Angle)&2047;
			dx=COS(ang)/8;
			dz=SIN(ang)/8;
			posn.X=darci->WorldPos.X;
			posn.Z=darci->WorldPos.Z;
//			posn.Y=(PAP_calc_height_at(posn.X>>8,posn.Z>>8)*256)+0x3000;
			posn.Y=darci->WorldPos.Y+0x3000;
			posn.X-=dx; posn.Z-=dz;
			RIBBON_extend(ribbon_id,posn.X>>8,posn.Y/256,posn.Z>>8);
			posn.X+=dx*2; posn.Z+=dz*2;
			RIBBON_extend(ribbon_id,posn.X>>8,posn.Y/256,posn.Z>>8);
		}
	}
	}


/*
	if (Keys[KB_P8])
	{
		Keys[KB_P8] = 0;
#ifdef	OLD_CAM
		switch(CAM_get_mode())
		{
			case CAM_MODE_NORMAL:       CAM_set_mode(CAM_MODE_THIRD_PERSON); break;
			case CAM_MODE_THIRD_PERSON: CAM_set_mode(CAM_MODE_NORMAL); CAM_set_zoom(0x320); break;
			default:
				CAM_set_mode(CAM_MODE_NORMAL);
				break;
		}
#endif
	}

*/

	/*

	//
	// Simon's idea! Make the camera look at whoever darci is targeting.
	//

	if (darci->State    == STATE_GUN &&
		darci->SubState == SUB_STATE_AIM_GUN)
	{
		if (darci->Genus.Person->Target)
		{
			CAM_set_focus(TO_THING(darci->Genus.Person->Target));
		}
	}
	else
	{
		CAM_set_focus(darci);
	}

	*/
#ifdef	OLD_CAM
	if (CAM_get_mode() == CAM_MODE_THIRD_PERSON)
	{
		//
		// Set the camera angles depending on the mouse.
		//

		SLONG yaw;
		SLONG pitch;
		SLONG roll;

		yaw   = MouseX - (DisplayWidth  / 2) << 3;
		pitch = MouseY - (DisplayHeight / 2) << 3;
		roll  = 0;

		CAM_set_angle(
			yaw,
			pitch,
			roll);

		SLONG zoom = CAM_get_zoom();

		#define CAM_DZOOM 0x10

		if (Keys[KB_P9]) {zoom -= CAM_DZOOM;}
		if (Keys[KB_P6]) {zoom += CAM_DZOOM;}

		CAM_set_zoom(zoom);
	}
#endif
#endif
#endif
/*
	if (Keys[KB_8])
	{
		Keys[KB_8] = 0;

		SPARK_in_sphere(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8,
			0x400,
			rand() & 0x3f,
			1);
	}

	if (Keys[KB_9])
	{
		Keys[KB_9] = 0;

		SPARK_Pinfo p1;
		SPARK_Pinfo p2;

		p1.type   = SPARK_TYPE_LIMB;
		p1.flag   = 0;
		p1.person = THING_NUMBER(darci);
		p1.limb   = SUB_OBJECT_RIGHT_HAND;


		p2.type   = SPARK_TYPE_GROUND;
		p2.flag   = SPARK_FLAG_STILL;
		p2.x      = (darci->WorldPos.X >> 8) + 31;
		p2.z      = (darci->WorldPos.Z >> 8) + 64;

		SPARK_create(
			&p1,
			&p2,
			 40);
	}
*/

	/*

	if (Keys[KB_L])
	{
		DRIP_create(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8);

		PUDDLE_splash(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8);
	}

	*/

	{
		static dlight = NULL;

		if (Keys[KB_L])
		{
			Keys[KB_L] = 0;

			if (dlight)
			{
				NIGHT_dlight_destroy(dlight);

				dlight = NULL;
			}
			else
			{
				dlight = NIGHT_dlight_create(
							(darci->WorldPos.X >> 8),
							(darci->WorldPos.Y >> 8) + 0x80,
							(darci->WorldPos.Z >> 8),
							200,
							40,
							30,
							20);
			}
		}

		if (dlight)
		{
			NIGHT_dlight_move(
				dlight,
				(darci->WorldPos.X >> 8),
				(darci->WorldPos.Y >> 8) + 0x80,
				(darci->WorldPos.Z >> 8));

			if (GAME_TURN & 0x20)
			{
				NIGHT_dlight_colour(
					dlight,
					60, 20, 20);
			}
			else
			{
				NIGHT_dlight_colour(
					dlight,
					20, 20, 60);
			}
		}
	}

	if (Keys[KB_P2])
	{
		// this is to test immolation of a thing
		Keys[KB_P2] = 0;
		if (TO_CHOPPER(1)->ChopperType!=CHOPPER_NONE) {
			Thing *pyro;
			GameCoord anyoldposn; // dont care, updated by thing

			pyro=PYRO_create(anyoldposn,PYRO_IMMOLATE);
			pyro->Genus.Pyro->victim=TO_CHOPPER(1)->thing;
//			pyro->Genus.Pyro->Flags=PYRO_FLAGS_FACES; // immolate faces
			pyro->Genus.Pyro->Flags=PYRO_FLAGS_FLICKER;
		}

	}
	if (Keys[KB_P3])
	{
		static UBYTE line = 0;
		static GameCoord oldposn={0,0,0};
		Thing *pyro;
		GameCoord posn;

		Keys[KB_P3] = 0;
		if (line) {
			posn.X=darci->WorldPos.X;
			posn.Z=darci->WorldPos.Z;
//			posn.Y=(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8)<<8)+0x3000;
			posn.Y=(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8))+0x3000;
			pyro=PYRO_create(oldposn,PYRO_FIREPOOL);
			pyro->Genus.Pyro->target=posn;
			posn.X-=oldposn.X;
			posn.Z-=oldposn.Z;
			posn.X>>=8; posn.Z>>=9;
			pyro->Genus.Pyro->radius=Root(posn.X*posn.X+posn.Z*posn.Z);
//			pyro->Genus.Pyro->Flags|=PYRO_FLAGS_TOUCHPOINT;
			line=0;
		} else {
			oldposn.X=darci->WorldPos.X;
			oldposn.Z=darci->WorldPos.Z;
//			oldposn.Y=(PAP_calc_map_height_at(oldposn.X>>8,posn.Z>>8)<<8)+0x3000;
			oldposn.Y=(PAP_calc_map_height_at(oldposn.X>>8,oldposn.Z>>8))+0x3000;
			line=1;
		}

	}
	static UBYTE smokin=0;
#ifndef	PSX
/*	if (Keys[KB_FORESLASH]) {

		Keys[KB_FORESLASH]=0;

		if (!ShiftFlag) {
			smokin=!smokin;
		} else {
			the_ff_manager->Test();
		}
	}*/
#endif
	if (Keys[KB_FORESLASH]) {
		Keys[KB_FORESLASH]=0;

		stealth_debug=!stealth_debug;
		if (stealth_debug)
			CONSOLE_text("STEALTH DEBUG MODE ON");
		else
			CONSOLE_text("STEALTH DEBUG MODE OFF");
	}

	/*

	static SWORD steamypos = 0;
	if (Keys[KB_POINT]) {
		Keys[KB_POINT]=0;
		if (ShiftFlag)
		  steamypos=-120;
		else
		  steamypos=120;
	}
	if (steamypos>0) {
	  PARTICLE_Steam(darci->WorldPos.X,darci->WorldPos.Y+0x5800,darci->WorldPos.Z,0,800,2,steamypos);
	  steamypos--;
	}
	if (steamypos<0) {
	  if (!PARTICLE_SGrenade(darci,-steamypos))
	    TRACE("particle add failed (sgrenade)\n");
	  steamypos++;
	}

	*/

	if (Keys[KB_POINT])
	{
		PARTICLE_Add(
			darci->WorldPos.X,
			darci->WorldPos.Y + 0x4000,
			darci->WorldPos.Z,
			256 + (Random() & 0xff),
			256 + (Random() & 0xff),
			256 + (Random() & 0xff),
			POLY_PAGE_SMOKECLOUD2,
			2,
			0x7fffffff,
			PFLAG_SPRITEANI | PFLAG_SPRITELOOP | PFLAG_FADE2 | PFLAG_RESIZE,
			100,
			40,
			1,
			1,
			1);
	}


//	if (smokin) psystem.Add(darci->WorldPos.X,darci->WorldPos.Y+0x100,darci->WorldPos.Z,0,0,0,POLY_PAGE_FOG,0x7FFFFFFF,PFLAGS_SMOKE,128,2,1);
	if (smokin) {
//		if (!psystem.Add(darci->WorldPos.X,darci->WorldPos.Y+0x100,darci->WorldPos.Z,0,0,0,POLY_PAGE_STEAM,0,0x7FFFFFFF,PFLAGS_SMOKE|PFLAG_WANDER,128,4,1,1,3))
//		if (!psystem.Add(darci->WorldPos.X,darci->WorldPos.Y+0x300,darci->WorldPos.Z,0,0,0,POLY_PAGE_STEAM,1+((rand()&3)<<2),0x7FFFFFFF,PFLAGS_SMOKE|PFLAG_WANDER,128,4,1,2,3))
//		if (!psystem.Exhaust(darci->WorldPos.X,darci->WorldPos.Y+0x300,darci->WorldPos.Z,2,10))
		if (!PARTICLE_Exhaust2(darci,2,10))
			TRACE("particle add failed (exhaust)\n");
	}




	static SLONG nav_x = 0;
	static SLONG nav_z = 0;

	static SLONG      ma_valid = 0;
	static MAV_Action ma = {0,0,0,0};
#ifndef	PSX
#ifndef TARGET_DC
	{
		//e_draw_3d_mapwho_y(nav_x, MAV_height[nav_x][nav_z] << 6, nav_z);

		if (ma_valid)
		{
			SLONG x1 = darci->WorldPos.X >> 8;
			SLONG y1 = darci->WorldPos.Y >> 8;
			SLONG z1 = darci->WorldPos.Z >> 8;

			SLONG x2 = (ma.dest_x << 8) + 0x80;
			SLONG y2 = MAVHEIGHT(ma.dest_x,ma.dest_z)<<6;//LAV_height[ma.dest_x][ma.dest_z] << 6;
			SLONG z2 = (ma.dest_z << 8) + 0x80;

			AENG_world_line(
				x1, y1, z1, 32, 0x00ffff00,
				x2, y2, z2, 0,  0x000000ff,
				TRUE);
		}
	}
#endif
#endif

#ifndef TARGET_DC
	if (Keys[KB_O]&&!ShiftFlag)
	{
		Keys[KB_O] = 0;

		OB_create(
			darci->WorldPos.X >> 8,
			darci->WorldPos.Y >> 8,
			darci->WorldPos.Z >> 8,
			darci->Draw.Tweened->Angle,
			0,
			0,
			5,0,0,0);
	}

	if (ShiftFlag)
	{
		static	skill=0;
		if(Keys[KB_A])
		{
			UWORD	index;
			Keys[KB_A]=0;

			//
			// Create a fight test dummy.
			//

			index=PCOM_create_person(
				PERSON_THUG_RED,//(GAME_TURN&1)?PERSON_THUG:PERSON_SOLDIER,
				0,
				0,
				PCOM_AI_GANG,
				0,
				skill % 16,
				PCOM_MOVE_WANDER,
				0,
				PCOM_BENT_FIGHT_BACK,
				0,
				0,
				0,
				(darci->WorldPos.X)-(128<<8)+((Random()&0xff)<<11),
				darci->WorldPos.Y,
				darci->WorldPos.Z-(128<<8)+((Random()&0xff)<<11),
				0,				0);

extern	void PCOM_set_person_ai_kill_person(Thing *p_person, Thing *p_target, SLONG alert_gang);
			PCOM_set_person_ai_kill_person(TO_THING(index),NET_PERSON(0),0);

			skill += 2;
		}

		if (Keys[KB_I])
		{
			Keys[KB_I] = 0;

			//
			// Create a bodyguard. First we must create a DUD waypoint that
			// pretends it created Darci!
			//

			SLONG waypoint = EWAY_find_or_create_waypoint_that_created_person(darci);

			PCOM_create_person(
				PERSON_HOSTAGE,
				0,
				0,
				PCOM_AI_BODYGUARD,
				waypoint,
				Random() % 16,
				PCOM_MOVE_FOLLOW,
				waypoint,
				0,
				0,
				0,
				0,
				darci->WorldPos.X + 0x4000,
				darci->WorldPos.Y,
				darci->WorldPos.Z,
				0,
				0,0,0);//FLAG2_PERSON_FAKE_WANDER);

			// remove_thing_from_map(darci);
		}

		if (Keys[KB_J])
		{
			Keys[KB_J] = 0;

			MAV_precalculate();

			switch(Random() % 3)
			{
				case 0: set_person_do_a_simple_anim(darci, ANIM_DANCE_BOOGIE);   break;
				case 1: set_person_do_a_simple_anim(darci, ANIM_DANCE_WOOGIE);   break;
				case 2: set_person_do_a_simple_anim(darci, ANIM_DANCE_HEADBANG); break;
			}

			darci->Genus.Person->Flags |= FLAG_PERSON_NO_RETURN_TO_NORMAL;
			darci->Genus.Person->Action = ACTION_SIT_BENCH;
		}

/*
		if (Keys[KB_Z])
		{
			Keys[KB_Z] = 0;

			nav_x = darci->WorldPos.X >> 16;
			nav_z = darci->WorldPos.Z >> 16;

			ma_valid = FALSE;
		}

		if (Keys[KB_X])
		{
			Keys[KB_X] = 0;

void	set_person_mav_to_xz(Thing *p_person,SLONG x,SLONG z);
			set_person_mav_to_xz(darci,nav_x<<8,nav_z<<8);

			ma_valid = TRUE;

		}
*/
		/*

		if (Keys[KB_R])
		{
			static GameCoord before_enter_pos = {0x400000, 0, 0x400000};

			Keys[KB_R] = 0;

			darci->Flags                  ^= FLAGS_IN_SEWERS;
			darci->Genus.Person->sewerbits = 0;

			if (darci->Flags & FLAGS_IN_SEWERS)
			{
				//
				// Look for a good square in the sewers near Darci.
				//

				SLONG i;
				SLONG dx;
				SLONG dz;
				SLONG mx = darci->WorldPos.X >> 16;
				SLONG mz = darci->WorldPos.Z >> 16;
				SLONG sx;
				SLONG sz;

				before_enter_pos = darci->WorldPos;

				for (i = 0; i < (128 << 3); i++)
				{
					dx = (rand() % ((i >> 3) + 1)) - (i >> 4);
					dz = (rand() % ((i >> 3) + 1)) - (i >> 4);

					sx = mx + dx;
					sz = mz + dz;

					sx = 0x40;
					sz = 0x3e;

					if (PAP_on_map_hi(sx,sz))
					{
						if (NS_HI_TYPE(&NS_hi[sx][sz]) == NS_HI_TYPE_STONE ||
							NS_HI_TYPE(&NS_hi[sx][sz]) == NS_HI_TYPE_SEWER)
						{
							//
							// Move Darci here.
							//

							GameCoord new_position = darci->WorldPos;

							new_position.X = (sx << 16) + 0x8000;
							new_position.Z = (sz << 16) + 0x8000;

							new_position.Y = NS_calc_height_at(
												new_position.X >> 8,
												new_position.Z >> 8) << 8;

							move_thing_on_map(darci, &new_position);

							break;
						}
					}
				}
			}
			else
			{
				//
				// Move Darci back.
				//

				move_thing_on_map(darci, &before_enter_pos);
			}
		}


		if (Keys[KB_B])
		{
			Keys[KB_B] = 0;

			PUDDLE_create(
				 darci->WorldPos.X >> 8,
				(darci->WorldPos.Y >> 8) + 0x4,
				 darci->WorldPos.Z >> 8);
		}

		if (Keys[KB_F])
		{
			Keys[KB_F] = 0;

			SLONG px;
			SLONG py;
			SLONG pz;

			calc_sub_objects_position(
				darci,
				darci->Draw.Tweened->AnimTween,
				SUB_OBJECT_RIGHT_HAND,
			   &px,
			   &py,
			   &pz);

			px += darci->WorldPos.X >> 8;
			py += darci->WorldPos.Y >> 8;
			pz += darci->WorldPos.Z >> 8;

			HOOK_spin(
				px,
				py,

				pz,
				darci->Draw.Tweened->Angle,
				50);
		}
		*/


		if (Keys[KB_Y])
		{
			COLLIDE_debug_fastnav(
				darci->WorldPos.X >> 8,
				darci->WorldPos.Z >> 8);
		}


		/*
		if (Keys[KB_T])
		{
			Keys[KB_T] = 0;

#ifndef	PSX
			UBYTE cloth = CLOTH_create(
								CLOTH_TYPE_FLAG,
								(darci->WorldPos.X >> 8) + 0x80,
								(darci->WorldPos.Y >> 8) + 0x80,
								(darci->WorldPos.Z >> 8) + 0x80,
								16, 0,  0,
								0, 16,  0,
								16,
								0x00ff8888);

			if (cloth && ControlFlag)
			{
				static choose = 0;

				if (choose++ & 1)
				{
					CLOTH_point_lock(cloth, 0, 1);
					CLOTH_point_lock(cloth, 0, 3);
				}
				else
				{
					CLOTH_point_lock(cloth, 0, 0);
					CLOTH_point_lock(cloth, 0, 4);
				}
			}
#endif
		}
		*/

		/*
		if (Keys[KB_X] && fti)
		{
			Keys[KB_X] = 0;

			GameCoord p1 = darci->WorldPos;
			GameCoord p2;

			p1.X>>=8;
			p1.Y>>=8;
			p1.Z>>=8;

			p2 = p1;


			p2.X -= (SIN(darci->Draw.Tweened->Angle)*32)>>16;
			p2.Z -=	(COS(darci->Draw.Tweened->Angle)*32)>>16;

			FURN_push(
				fti,
				p1.X, p1.Y, p1.Z,
				p2.X, p2.Y, p2.Z);
		}
		*/

		/*

		if (Keys[KB_M])
		{
			Keys[KB_M] = 0;

			static SLONG last_detail = 17;
			static SLONG last_height = 84;

			#define MIST_SIZE 0x800

			SLONG x1 = (darci->WorldPos.X>>8) - MIST_SIZE;
			SLONG z1 = (darci->WorldPos.Z>>8) - MIST_SIZE;
			SLONG x2 = (darci->WorldPos.X>>8) + MIST_SIZE;
			SLONG z2 = (darci->WorldPos.Z>>8) + MIST_SIZE;

			MIST_create(
				last_detail,
				last_height,
				x1, z1,
				x2, z2);

			last_height += (last_height & 0x1) ? -25 : +25;
		}

		if (Keys[KB_M])
		{
			Keys[KB_M] = 0;

			static SLONG last_detail = 17;
			static SLONG last_height = 84;

			#define MIST_SIZE 0x800

			SLONG x1 = (darci->WorldPos.X>>8) - MIST_SIZE;
			SLONG z1 = (darci->WorldPos.Z>>8) - MIST_SIZE;
			SLONG x2 = (darci->WorldPos.X>>8) + MIST_SIZE;
			SLONG z2 = (darci->WorldPos.Z>>8) + MIST_SIZE;

			MIST_create(
				last_detail,
				last_height,
				x1, z1,
				x2, z2);

			last_height += (last_height & 0x1) ? -25 : +25;
		}

		*/

		if (Keys[KB_D] && !ShiftFlag)
		{
			Keys[KB_D] = 0;

			NIGHT_slight_create(
				(darci->WorldPos.X >> 8),
				(darci->WorldPos.Y >> 8) + 0x80,
				(darci->WorldPos.Z >> 8),
				200,
				30,
				40,
				50);

			NIGHT_cache_recalc();
			NIGHT_dfcache_recalc();
			NIGHT_generate_walkable_lighting();
		}

		if (Keys[KB_D] && ShiftFlag)
		{
			Keys[KB_D]=0;
			//
			// shift D to leap into the debugger
			//
			ASSERT(2+2==5);

		}


		if (Keys[KB_G])
		{

			Keys[KB_G] = 0;
/*
			NIGHT_slight_create(
				(darci->WorldPos.X >> 8),
				(darci->WorldPos.Y >> 8) + 0x80,
				(darci->WorldPos.Z >> 8),
				200,
			   -30,
			   -30,
			   -30);

			NIGHT_cache_recalc();
			NIGHT_dfcache_recalc();
			NIGHT_generate_walkable_lighting();
*/
			darci->Flags |= FLAGS_HAS_GUN;

		}

		if (Keys[KB_H])
		{
			Keys[KB_H] = 0;

			plan_view_shot();

			/*

			SM_init();

			SM_create_cube(
				(darci->WorldPos.X >> 8) + 0x000,
				(darci->WorldPos.Y >> 8) + 0x080,
				(darci->WorldPos.Z >> 8) + 0x000,
				(darci->WorldPos.X >> 8) + 0x000 + 0x80,
				(darci->WorldPos.Y >> 8) + 0x080 + 0x80,
				(darci->WorldPos.Z >> 8) + 0x000 + 0x80,
				3,
				0x10000,
				0x10000);

			*/

			/*

			if (fti)
			{
				static float force = 20.0F;

				FURN_hypermatterise(fti);

				if (TO_THING(fti)->Draw.Mesh->Hm != 255)
				{
					HM_shockwave(
						TO_THING(fti)->Draw.Mesh->Hm,
						float( darci->WorldPos.X >> 8),
						float((darci->WorldPos.Y >> 8) - 32),
						float( darci->WorldPos.Z >> 8),
						400.0F,
						force);
				}
			}
			else
			{
				HM_Primgrid *hpg = HM_get_primgrid(6);

				UBYTE hm = HM_create(
								6,

								(darci->WorldPos.X>>8),
								(darci->WorldPos.Y>>8) + 512,
								(darci->WorldPos.Z>>8),

								0, 0, 0,
								0, 0, 0,

								hpg->x_res,
								hpg->y_res,
								hpg->z_res,
								hpg->x_point,
								hpg->y_point,
								hpg->z_point,

								hpg->x_dgrav,
								hpg->y_dgrav,
								hpg->z_dgrav,

								0.050F,		// elasticity
								0.900F,		// bounciness
								0.000F,		// friction
								0.999F);	// damping

				if (hm_1 == 255) {hm_1 = hm;} else {hm_2 = hm;}
			}

			*/
		}

/*
		if (Keys[KB_E])
		{
			Keys[KB_E] = 0;

			if (GAME_FLAGS & GF_INDOORS)
			{
				//
				// Toggle out of this mode.
				//

				GAME_FLAGS   &= ~GF_INDOORS;
				darci->Flags &= ~FLAGS_IN_BUILDING;

				//
				// Move Darci to where she entered the building.
				//

				move_thing_on_map(darci, &enter_pos);

				//
				// Get rid of all the things created to be
				// inside the building.
				//

				//ID_remove_inside_things();
				//ID_wall_colvects_remove();

			}
			else
			{
				//
				// Find and enter a nearby building.
				//

				ENTER_Okay eo;

				eo = ENTER_can_i(THING_NUMBER(NET_PERSON(0)));

				if (eo.dbuilding)
				{
					//
					// Ground level of the building.
					//

					SLONG top;
					SLONG bot;

					ENTER_get_extents(
						eo.dbuilding,
					   &bot,
					   &top);

					if (WITHIN(darci->WorldPos.Y >> 8, bot - 0x40, bot + 0x40))
					{
						if (ENTER_setup(eo.dbuilding, bot, TRUE, FALSE))
						{
							//
							// Remember where Darci is.
							//

							enter_pos = darci->WorldPos;

							//
							// Move darci to inside the building.
							//

							SLONG x1;
							SLONG z1;
							SLONG x2;
							SLONG z2;

							GameCoord pos;

							ID_get_floorplan_bounding_box(
								&x1,
								&z1,
								&x2,
								&z2);

							TRACE("Bounding box = (%d,%d)-(%d,%d)\n", x1, z1, x2, z2);
							TRACE("Building on square (%d,%d)\n", eo.map_x, eo.map_z);

							pos.X = (x1 + x2) << (ELE_SHIFT + 7);
							pos.Z = (z1 + z2) << (ELE_SHIFT + 7);

							pos.Y = 0;

							//
							// Don't go to the centre of the building, use the coordinates
							// give to use by the ENTER module.
							//

							pos.X  = eo.map_x << 16;
							pos.Z  = eo.map_z << 16;

							pos.X += 0x8000;
							pos.Z += 0x8000;

							move_thing_on_map(darci, &pos);

							//
							// Mark Darci as being inside the building.
							//

							darci->Flags     |= FLAGS_IN_BUILDING;
							darci->WorldPos.Y = INDOORS_HEIGHT_FLOOR;

							//
							// Go into 'inside' mode.
							//

							GAME_FLAGS ^= GF_INDOORS;
						}
					}
				}
			}
		}
*/

/*
		if (Keys[KB_N])
		{
			GameCoord posn;

			Keys[KB_N] = 0;

			posn.X=darci->WorldPos.X;
			posn.Y=darci->WorldPos.Y;
			posn.Z=darci->WorldPos.Z;
			ANIMAL_create(posn,ANIMAL_CANID);
			TRACE("Animal created\n");

		}
*/
		if (Keys[KB_O])
		{
			Thing *chopper;
			GameCoord posn;

			Keys[KB_O] = 0;

			posn.X=darci->WorldPos.X;
			posn.Z=darci->WorldPos.Z;
//			posn.Y=darci->WorldPos.Y+(265<<8)+(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8)<<8);
			posn.Y=darci->WorldPos.Y+(265<<8)+(PAP_calc_map_height_at(posn.X>>8,posn.Z>>8));
			chopper=CHOPPER_create(posn,CHOPPER_CIVILIAN);
			chopper->Draw.Mesh->Angle=darci->Draw.Tweened->Angle;
			TRACE("Chopper created\n");

		}

		/*

		if (TO_CHOPPER(1)->ChopperType!=CHOPPER_NONE)
		{
			if (Keys[KB_5]) {
				Keys[KB_5] = 0;
				CHOPPER_init_state(TO_CHOPPER(1)->thing, CHOPPER_substate_takeoff);
			}

			if (Keys[KB_6]) {
				Keys[KB_6] = 0;
				CHOPPER_init_state(TO_CHOPPER(1)->thing, CHOPPER_substate_landing);
			}


			if (Keys[KB_7]) {
				Keys[KB_7] = 0;
				if (TO_CHOPPER(1)->substate==CHOPPER_substate_tracking) {
					CHOPPER_init_state(TO_CHOPPER(1)->thing, CHOPPER_substate_patrolling);
				} else {
					TO_CHOPPER(1)->target=darci;
					CHOPPER_init_state(TO_CHOPPER(1)->thing, CHOPPER_substate_tracking);
				}
			}
		}

		*/

		/*
extern	SLONG	FC_cam_dist;
extern	SLONG	FC_cam_height;
		if(Keys[KB_7])
		{
			CBYTE	str[50];
			sprintf(str,"cam_dist %d cam_height %d",FC_cam_dist,FC_cam_height>>8);
			CONSOLE_text(str,500);
			FC_cam_dist-=16;
		}
		if(Keys[KB_8])
		{
			CBYTE	str[50];
			sprintf(str,"cam_dist %d cam_height %d",FC_cam_dist,FC_cam_height>>8);
			CONSOLE_text(str,500);
			FC_cam_dist+=16;
		}
		if(Keys[KB_9])
		{
			CBYTE	str[50];
			sprintf(str,"cam_dist %d cam_height %d",FC_cam_dist,FC_cam_height>>8);
			CONSOLE_text(str,500);
			FC_cam_height-=0x1000;
		}
		if(Keys[KB_0])
		{
			CBYTE	str[50];
			sprintf(str,"cam_dist %d cam_height %d",FC_cam_dist,FC_cam_height>>8);
			CONSOLE_text(str,500);
			FC_cam_height+=0x1000;
		}
		*/
/*
		if(Keys[KB_F12])
		{
			mouse_input^=1;

			Keys[KB_F12]=0;
			if(mouse_input)
			{

				SetCapture(hDDLibWindow);

				ShowCursor(NULL);
			}
			else
			{
				ReleaseCapture();
				ShowCursor(1);
			}
				//SetCursor(LoadCursor(NULL,IDC_ARROW));



		}
*/



#ifndef TARGET_DC
		if (Keys[KB_M])
		{
			Keys[KB_M] = 0;

			//
			// Create a mine at the mouse.
			//

			SLONG world_x;
			SLONG world_y;
			SLONG world_z;

			AENG_raytraced_position(
				MouseX,
				MouseY,
			   &world_x,
			   &world_y,
			   &world_z);

			if (WITHIN(world_x, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1) &&
				WITHIN(world_z, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1))
			{
				world_y = PAP_calc_map_height_at(world_x, world_z);

				alloc_special(
					SPECIAL_MINE,
					SPECIAL_SUBSTATE_NONE,
					world_x,
					world_y,
					world_z,
					NULL);
			}
		}
#endif //#ifndef TARGET_DC






	}
	else
	{
		//
		// Shift not held down.
		//

		if(Keys[KB_F12])
		{
			Keys[KB_F12] = 0;

			/*

			alloc_special(
				SPECIAL_EXPLOSIVES,
				SPECIAL_SUBSTATE_NONE,
				darci->WorldPos.X + 0x8000 >> 8,
				darci->WorldPos.Y + 0x2000 >> 8,
				darci->WorldPos.Z + 0x8000 >> 8,
				NULL);

			alloc_special(
				SPECIAL_SHOTGUN,
				SPECIAL_SUBSTATE_NONE,
				darci->WorldPos.X + 0x8000 >> 8,
				darci->WorldPos.Y + 0x2000 >> 8,
				darci->WorldPos.Z + 0x8000 >> 8,
				NULL);
			*/

			SLONG	wx,wy,wz,dx,dz;
			SLONG	angle;

			wx=darci->WorldPos.X>>8;
			wy=darci->WorldPos.Y>>8;
			wz=darci->WorldPos.Z>>8;

			darci->Genus.Person->Health=9999;

			wy+=0x20;

			for(angle=0;angle<7;angle++)
			{
				dx=COS(angle*(2047/7))>>8;
				dz=SIN(angle*(2047/7))>>8;
				switch(angle)
				{
					case 0:
						alloc_special(SPECIAL_HEALTH     , SPECIAL_SUBSTATE_NONE, wx+dx, wy + 0x10, wz+dz, 0);
						break;
					case 1:
						alloc_special(SPECIAL_BASEBALLBAT        , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;
					case 2:
						alloc_special(SPECIAL_KNIFE        , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;
					case 3:
						alloc_special(SPECIAL_SHOTGUN    , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;
					case 4:
						alloc_special(SPECIAL_GRENADE    , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;
					case 5:
						alloc_special(SPECIAL_AK47       , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;
					case 6:
						alloc_special(SPECIAL_MINE       , SPECIAL_SUBSTATE_NONE, wx+dx, wy,        wz+dz, 0);
						break;

					default:
						ASSERT(0);
						break;
				}
			}

			/*
			{
				SLONG	c0,index1,index2,index3;
				for(c0=0;c0<4;c0++)
				{
					SLONG	index;
					index=PCOM_create_person(1,0,0,PCOM_AI_FIGHT_TEST,0,c0*5,PCOM_MOVE_STILL,0,PCOM_BENT_FIGHT_BACK,0,(wx+(c0+2)*300)<<8,wy<<8,wz<<8,0);
				}

				index1=PCOM_create_person(1,0,0,PCOM_AI_FIGHT_TEST,0,15,PCOM_MOVE_STILL,0,PCOM_BENT_FIGHT_BACK,0,(wx-(2)*300)<<8,wy<<8,wz<<8,0);
				index2=PCOM_create_person(1,0,0,PCOM_AI_FIGHT_TEST,0,7,PCOM_MOVE_STILL,0,PCOM_BENT_FIGHT_BACK,0,(wx-(3)*300)<<8,wy<<8,wz<<8,0);
				index3=PCOM_create_person(1,0,0,PCOM_AI_FIGHT_TEST,0,7,PCOM_MOVE_STILL,0,PCOM_BENT_FIGHT_BACK,0,(wx-(3)*300)<<8,wy<<8,(wz-200)<<8,0);

				PCOM_attack_happened(TO_THING(index1),TO_THING(index2));
				PCOM_attack_happened(TO_THING(index2),TO_THING(index1));
				PCOM_attack_happened(TO_THING(index3),TO_THING(index1));

			}
			*/
		}

#ifndef PSX
#ifdef  BIKE
		if (Keys[KB_K])
		{
			Keys[KB_K] = 0;

			//
			// Create a bike at the mouse.
			//

			SLONG world_x;
			SLONG world_y;
			SLONG world_z;

			AENG_raytraced_position(
				MouseX,
				MouseY,
			   &world_x,
			   &world_y,
			   &world_z);

			if (WITHIN(world_x, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1) &&
				WITHIN(world_z, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1))
			{
				BIKE_create(
					world_x,
					world_z,
					0);
			}

		}
#endif
#endif

#if !defined(PSX) && !defined(TARGET_DC)
		if (Keys[KB_G])
		{
			Keys[KB_G] = 0;

			SLONG world_x;
			SLONG world_y;
			SLONG world_z;

			//
			// Teleport Darci to where the mouse is.
			//

			RECT	client;

			GetClientRect(hDDLibWindow, &client);

			float	hitx = float(MouseX) * float(DisplayWidth) / float(client.right - client.left);
			float	hity = float(MouseY) * float(DisplayHeight) / float(client.bottom - client.top);

			AENG_raytraced_position(
				SLONG(hitx + 0.5f),
				SLONG(hity + 0.5f),
//				MouseX,
//				MouseY,
			   &world_x,
			   &world_y,
			   &world_z);

			if (WITHIN(world_x, 600, (PAP_SIZE_HI << PAP_SHIFT_HI) - 601) &&
				WITHIN(world_z, 600, (PAP_SIZE_HI << PAP_SHIFT_HI) - 601))
			{
				world_y += 0x10;

				GameCoord teleport;

				teleport.X = world_x << 8;
				teleport.Y = world_y << 8;
				teleport.Z = world_z << 8;

				set_person_idle(darci);
				move_thing_on_map(darci, &teleport);

				SLONG plant_feet(Thing *p_person);
				plant_feet(darci);
				darci->Genus.Person->Flags&=~(FLAG_PERSON_KO|FLAG_PERSON_HELPLESS);

			}
		}

#ifndef NDEBUG
		// debug - find which car the mouse points at
		extern void LookForSelectedThing();
		LookForSelectedThing();
#endif
#endif
/*
		if (Keys[KB_N])
		{
			Keys[KB_N] = 0;

			//
			// Toggle in/out of sniper mode.
			//

			if (SNIPE_on)
			{
				SNIPE_mode_off();
			}
			else
			{
				if (darci->State == STATE_IDLE)
				{
					SLONG head_x;
					SLONG head_y;
					SLONG head_z;

					//
					// Where is Darci's head?
					//

					calc_sub_objects_position(
						darci,
						darci->Draw.Tweened->AnimTween,
						SUB_OBJECT_HEAD,
					   &head_x,
					   &head_y,
					   &head_z);

					head_x += darci->WorldPos.X >> 8;
					head_y += darci->WorldPos.Y >> 8;
					head_z += darci->WorldPos.Z >> 8;

					SNIPE_mode_on(
						head_x,
						head_y,
						head_z,
						darci->Draw.Tweened->Angle + 1024);
				}
			}
		}

		if (Keys[KB_B])
		{
			Keys[KB_B] = 0;

			SNIPE_shoot();
		}
*/

/*
#ifndef TARGET_DC
#if !defined(PSX)
		if (Keys[KB_R])
		{
			Keys[KB_R] = 0;

			SLONG world_x;
			SLONG world_y;
			SLONG world_z;

			//
			// Place a barrel where the mouse is.
			//

			AENG_raytraced_position(
				MouseX,
				MouseY,
			   &world_x,
			   &world_y,
			   &world_z);

			if (WITHIN(world_x, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1) &&
				WITHIN(world_z, 0, (PAP_SIZE_HI << PAP_SHIFT_HI) - 1))
			{
				//
				// Place a barrel where Darci is.
				//

				BARREL_alloc(
					BARREL_TYPE_NORMAL,
					PRIM_OBJ_BARREL,
					world_x,
					world_z,
					NULL);
			}
		}
#else
		{
			if(PadKeyIsPressed(&PAD_Input2,PAD_FRB))
				BARREL_alloc(
					BARREL_TYPE_NORMAL,
					141,
					(darci->WorldPos.X>>8)+200,
					darci->WorldPos.Z>>8,
					NULL);
		}

#endif
#endif
*/

		if (Keys[KB_U])
		{
			BARREL_hit_with_sphere(
				darci->WorldPos.X >> 8,
				darci->WorldPos.Y >> 8,
				darci->WorldPos.Z >> 8,
				0x80);
		}

		if (Keys[KB_Q])
		{
			ROAD_debug();
		}
	}

	//
	// Animate the animating texture structure
	//
#ifndef PSX
	animate_texture_maps();
#endif
	//	Get user game input.

#endif //#ifndef TARGET_DC
#endif //#ifndef TARGET_DC
}

#else
SLONG	 PSX_inv_open;
int PSX_inv_focus;
int PSX_inv_count;
int PSX_inv_select;
int PSX_inv_timer=0;


#define INVENTORY_FADE_SPEED	(16)

SBYTE CONTROLS_get_selected_item(Thing *darci, Thing *player) {
	SBYTE count=1; // 0 is fist
	Thing *p_special = NULL;
	SBYTE current_item = 0;

	if (darci->Genus.Person->SpecialList)
	{
		p_special = TO_THING(darci->Genus.Person->SpecialList);

		while(p_special) {
			ASSERT(p_special->Class == CLASS_SPECIAL);
			if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
				SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
				p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES||
				p_special->Genus.Special->SpecialType                     == SPECIAL_WIRE_CUTTER)
			{
			  if (THING_NUMBER(p_special)==darci->Genus.Person->SpecialUse) current_item=count;
			  count++;
			}
			if (p_special->Genus.Special->NextSpecial)
				p_special = TO_THING(p_special->Genus.Special->NextSpecial);
			else
				p_special = NULL;
		}
	}

	if (darci->Flags & FLAGS_HAS_GUN) {
		if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT) current_item=count;
		count++;
	}
	PSX_inv_count=count;

	return current_item;

}


void	CONTROLS_new_inventory(Thing *darci, Thing *player)
{
	PSX_inv_open=1;
	PSX_inv_focus = CONTROLS_get_selected_item(darci,player);
/*
	if (!PSX_inv_fade)
	{
		PSX_inv_select=-1;
		PSX_inv_focus=-1;
	}
	PSX_inv_fade+=INVENTORY_FADE_SPEED;
	if (PSX_inv_fade>255)
		PSX_inv_fade=255;

	if (PSX_inv_focus==-1)
		PSX_inv_focus = CONTROLS_get_selected_item(darci, player);
*/
}

void	CONTROLS_rot_inventory(Thing *darci, Thing *player, SBYTE dir)
{
	PSX_inv_timer=3;
	PSX_inv_focus+=dir;
	if (PSX_inv_focus<0)
		PSX_inv_focus=PSX_inv_count-1;
	if (PSX_inv_focus>=PSX_inv_count)
		PSX_inv_focus=0;
}
/*
void	CONTROLS_set_inventory(Thing *darci, Thing *player) {
	Thing *p_special = NULL;
	SBYTE count;


	if (darci->Genus.Person->Flags & FLAG_PERSON_GUN_OUT)
	{
		// Put away your gun.
		set_person_gun_away(darci);
	}

	count=PSX_inv_focus;

	if (!count) {
		set_person_item_away(darci);
		return; // using fists
	}

	// Make Darci use an item.

	if (darci->Genus.Person->SpecialList!=0)
		p_special = TO_THING(darci->Genus.Person->SpecialList);
	else
		p_special = 0;

	count--;

	while (count) {
		if (!p_special) break; // no specials -- gun maybe?
		ASSERT(p_special->Class == CLASS_SPECIAL);

		if (SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_ONEHANDED_WEAPON ||
			SPECIAL_info[p_special->Genus.Special->SpecialType].group == SPECIAL_GROUP_TWOHANDED_WEAPON ||
			p_special->Genus.Special->SpecialType                     == SPECIAL_EXPLOSIVES||
			p_special->Genus.Special->SpecialType                     == SPECIAL_WIRE_CUTTER)
		{
			count--;
		}

		if (p_special->Genus.Special->NextSpecial)
			p_special = TO_THING(p_special->Genus.Special->NextSpecial);
		else
			p_special = NULL;
	}

	if (p_special)
	{
		set_person_draw_item(darci, p_special->Genus.Special->SpecialType);
	}
	else
	{
		darci->Genus.Person->SpecialUse = NULL;
		darci->Draw.Tweened->PersonID   = 0;

		//
		// If Darci has a gun then get it out.
		//

		if (darci->Flags & FLAGS_HAS_GUN)
		{
			set_person_draw_gun(darci);
		}
		else
		{
			set_person_idle(darci);
		}
	}


}
*/
//
// PSX VERSION
//

void	process_controls(void)
{
	SLONG i;
	SLONG x;
	SLONG z;

	GameCoord position;
	Thing	 *t_thing;

	//
	// Mark's stuff.
	//

	Thing *darci = NET_PERSON(0);

	if ((GAME_TURN & 0x0f)==0)
		set_danger_level();

	context_music();

#ifndef FS_ISO9660
#ifndef	MIKE
	if (PadKeyIsPressed(&PAD_Input2,PAD_FRT))
		GAME_STATE = GS_LEVEL_WON;
#endif
#endif

	if (darci->State == STATE_DEAD)
	{
		if (darci->Genus.Person->Flags & FLAG_PERSON_ARRESTED)
		{
			//
			// Wait till she is actually arrested!
			//

			if (darci->SubState == SUB_STATE_DEAD_ARRESTED)
			{
				GAME_STATE = GS_LEVEL_LOST;
			}
		}
		else
		{
			GAME_STATE = GS_LEVEL_LOST;
		}
	}


	DIRT_set_focus(darci->WorldPos.X>>8, darci->WorldPos.Z>>8, 0x800);
	DIRT_process();
//	ProcessGrenades();
//	MIST_process();
//	WATER_process();
//	DRIP_process();
//	PUDDLE_process();
//	BANG_process();
	SPARK_process();
//	GLITTER_process();
//	LIGHT_process();
//	HOOK_process();
//	SM_process();
//	FC_process();
//	BALLOON_process();
//	SNIPE_process();

	if (!(GAME_FLAGS & GF_PAUSED))
	{
		Thing *the_player = NET_PLAYER(0);
//		if (NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_SELECT)
		if ((PACKET_DATA(0) & INPUT_MASK_SELECT)&&!EWAY_stop_player_moving())
		{
			if ((darci->SubState==SUB_STATE_WALKING)||(darci->SubState==SUB_STATE_RUNNING)||(darci->SubState==SUB_STATE_WALKING_BACKWARDS)||(darci->State == STATE_IDLE) ||(darci->State == STATE_FIGHT) || (darci->State == STATE_GUN && darci->SubState != SUB_STATE_SHOOT_GUN))
			{
				if(!PSX_inv_open)
				{
					CONTROLS_new_inventory(darci, the_player);
				}
				else
				{
					PSX_inv_open=0;
					CONTROLS_set_inventory(darci,the_player,PSX_inv_focus);
				}

			}
		}
		if (PSX_inv_open)
		{
			if(PSX_inv_timer)
			{
				PSX_inv_timer--;

			}
			else
			{
//				if (PACKET_DATA(0) & (INPUT_MASK_PUNCH|INPUT_MASK_ACTION|INPUT_MASK_KICK|INPUT_MASK_JUMP))
				if (PadKeyIsPressed(&PAD_Input1,PAD_RU)||PadKeyIsPressed(&PAD_Input1,PAD_RD)||PadKeyIsPressed(&PAD_Input1,PAD_RL)||PadKeyIsPressed(&PAD_Input1,PAD_RR))
				{
					NET_PLAYER(0)->Genus.Player->InputDone|=(INPUT_MASK_PUNCH|INPUT_MASK_ACTION|INPUT_MASK_KICK|INPUT_MASK_JUMP);
					PSX_inv_open=0;
					CONTROLS_set_inventory(darci,the_player,PSX_inv_focus);
				}
				if ((PACKET_DATA(0) & INPUT_MASK_LEFT)||(PACKET_DATA(0) & INPUT_MASK_FORWARDS))
					CONTROLS_rot_inventory(darci,the_player,-1);
				//if ((NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_RIGHT)||(NET_PLAYER(0)->Genus.Player->Pressed & INPUT_MASK_BACKWARDS))
				if ((PACKET_DATA(0) & INPUT_MASK_RIGHT)||(PACKET_DATA(0) & INPUT_MASK_BACKWARDS))
					CONTROLS_rot_inventory(darci,the_player, 1);
			}

		}

	}

}


#endif

//---------------------------------------------------------------
