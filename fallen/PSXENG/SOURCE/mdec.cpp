/*
** MDEC.CPP
**
** Full motion video playback for PSX.
*/
				  
#include "game.h"
#include "psxeng.h"

#include <libpress.h>
#include <libcd.h>

#include <ctrller.h>

#if 0
typedef struct {
	SWORD frame;
	UBYTE fast;
	UBYTE slow;
} MDEC_vibra;

#define VIBRA_NONE(f)		{f,0,0}
#define VIBRA_GUNSHOT(f)	{f,1,64}
#define VIBRA_RICOCHET(f)	{f,1,0}
#define VIBRA_SHOTGUN(f)	{f,1,128}
#define VIBRA_SMASH(f)		{f,0,192}
#define VIBRA_CRACK(f)		{f,1,128}
#define VIBRA_WINDOW(f)		{f,0,128}
#define VIBRA_BULLET(f)		{f,1,96}
#define VIBRA_EXPLODE(f)	{f,0,192}
#define VIBRA_MINISTART(f)	{f,255,0}
#define VIBRA_MINIEND(f)	{f,255,0}
#define VIBRA_COVER(f)		{f,0,64}
#define VIBRA_FINALE(f)		{f,0,255}
#define VIBRA_FINALEEND(f)	{f,0,255}


MDEC_vibra	vib_none[]={VIBRA_NONE(32767)};
MDEC_vibra	vib_intro[]={
	VIBRA_GUNSHOT(1671),
	VIBRA_GUNSHOT(1679),
	VIBRA_SHOTGUN(1814),
	VIBRA_GUNSHOT(1825),
	VIBRA_GUNSHOT(1837),
	VIBRA_SHOTGUN(1839),
	VIBRA_GUNSHOT(1854),
	VIBRA_RICOCHET(1859),
	VIBRA_RICOCHET(1867),
	VIBRA_GUNSHOT(1872),
	VIBRA_GUNSHOT(1884),
	VIBRA_SHOTGUN(1886),
	VIBRA_GUNSHOT(1892),
	VIBRA_RICOCHET(1896),
	VIBRA_GUNSHOT(1899),
	VIBRA_RICOCHET(1907),
	VIBRA_SHOTGUN(1914),
	VIBRA_GUNSHOT(1924),
	VIBRA_SMASH(2083),
	VIBRA_CRACK(2492),
	VIBRA_SHOTGUN(2539),
	VIBRA_SHOTGUN(2559),
	VIBRA_WINDOW(2606),
	VIBRA_SMASH(2860),
	VIBRA_SMASH(2997),
	VIBRA_GUNSHOT(3001),
	VIBRA_GUNSHOT(3012),
	VIBRA_BULLET(3031),
	VIBRA_BULLET(3037),
	VIBRA_GUNSHOT(3053),
	VIBRA_BULLET(3064),
	VIBRA_SMASH(3142),
	VIBRA_EXPLODE(3152),
	VIBRA_MINISTART(3942),
	VIBRA_GUNSHOT(4177),
	VIBRA_GUNSHOT(4184),
	VIBRA_COVER(4207),
	VIBRA_MINIEND(4210),
	VIBRA_COVER(4574),
	VIBRA_FINALE(4634),
	VIBRA_FINALE(4735),
	VIBRA_NONE(32767)
};

#define VIBRA_RUMBLE(f) {f,0,96}
#define VIBRA_BOLT(f)	{f,1,64}
#define VIBRA_LAMP(f)	{f,1,128}

MDEC_vibra vib_endgame[]={
	VIBRA_RUMBLE(246),
	VIBRA_RUMBLE(250),
	VIBRA_RUMBLE(254),
	VIBRA_RUMBLE(258),
	VIBRA_RUMBLE(262),
	VIBRA_BOLT(296),
	VIBRA_BOLT(303),
	VIBRA_BOLT(312),
	VIBRA_LAMP(364),
	VIBRA_BOLT(416),
	VIBRA_BOLT(427),
	VIBRA_WINDOW(437),
	VIBRA_WINDOW(441),
	VIBRA_EXPLODE(453),
	VIBRA_EXPLODE(455),
	VIBRA_EXPLODE(489),
	VIBRA_FINALE(512),
	VIBRA_FINALE(540)
};

MDEC_vibra *vibra[]={vib_none,vib_none,vib_intro,vib_none,vib_endgame,vib_none};
#else
typedef struct {
	UBYTE	fast;
	UBYTE	slow;
} VibraData;

VibraData *MDEC_vibra;

#endif

extern ControllerPacket	PAD_Input1,PAD_Input2;

DECDCTTAB *MDEC_VLCTable;
ULONG *MDEC_input;
ULONG *MDEC_slice;
ULONG *MDEC_output[2];
ULONG MDEC_width;
long MDEC_endframe;
UBYTE MDEC_frame;
RECT MDEC_rect;
volatile UBYTE MDEC_done;
extern int screen_x;
extern int screen_y;
int MDEC_height;

static void MDEC_Callback()
{
#ifndef VERSION_REVIEW
	if (MDEC_rect.x<MDEC_width)
	{
		// not last frame
		DecDCTout(MDEC_output[1-MDEC_frame],MDEC_height*8);
	} else
		MDEC_done=1;

	LoadImage(&MDEC_rect,MDEC_output[MDEC_frame]);
	MDEC_frame=1-MDEC_frame;
	MDEC_rect.x+=16;
#endif
}

static CdlATV MDEC_mix;

int MDEC_Init(char *fname,int len)
{
#ifndef VERSION_REVIEW
	CdlFILE file;
	CdlFILTER filter;
	char str[20];

	int i;
	
	DecDCTReset(0);
	MDEC_VLCTable=(DECDCTTAB*)MemAlloc(sizeof(DECDCTTAB));
	DecDCTvlcBuild(*MDEC_VLCTable);
	MDEC_input=(ULONG*)MemAlloc(24*2048);
	MDEC_slice=(ULONG*)MemAlloc(320*240);
	MDEC_output[0]=(ULONG*)MemAlloc(30*256);
	MDEC_output[1]=(ULONG*)MemAlloc(30*256);
	MDEC_vibra=(VibraData *)MemAlloc(10240);

	DecDCToutCallback(MDEC_Callback);

	filter.chan=0;
	filter.file=1;

	CdControl(CdlSetfilter,(UBYTE*)&filter,0);

	MDEC_mix.val0=MDEC_mix.val2=127;
	MDEC_mix.val1=MDEC_mix.val3=0;

	CdMix(&MDEC_mix);

	if (CdSearchFile(&file, fname)==0)
		return 0;


	memset((void*)MDEC_input,0,24*2048);
	memset((void*)MDEC_slice,0,320*240);
	memset((void*)MDEC_output[0],0,30*256);
	memset((void*)MDEC_output[1],0,30*256);
	memset((void*)MDEC_vibra,0,10240);

	strcpy(str,fname);
	strcpy(strrchr(str,'.'),".PVD");

	PCReadFile(&str[1],(UBYTE*)MDEC_vibra,10240);

	StSetRing(MDEC_input,24);
	StSetStream(0,0,0xffffffff,0,0);
	
	MDEC_endframe=len;
	do
	{
		UBYTE param=CdlModeSpeed;
		while (CdControl(CdlSetloc,(UBYTE*)&file.pos,0)==0);
		while (CdControl(CdlSetmode,&param,0)==0);
		VSync(3);
	}
	while(CdRead2(CdlModeStream|CdlModeSpeed|CdlModeRT)==0);
#endif
}

SLONG MDEC_CurrentFrame;


int MDEC_Render(void)
{
#ifndef VERSION_REVIEW
	RECT rect;
	ULONG *addr;
	StHEADER *sector;
	SLONG decode,b;
	SLONG pos;
	SLONG timer=0x80000;

	// Setup the strip rectangle for one column of display based on the current display buffer's
	// draw location.

	while((timer)&&(StGetNext(&addr,(ULONG **)&sector)!=0)) timer--;

	// Okay if we timeout on this operation then skip the FMV

	if (timer==0)
		return 1;

	MDEC_rect.x=160-(sector->width>>1);
	MDEC_rect.y=the_display.CurrentDisplayBuffer->Draw.clip.y+ (120-(sector->height>>1));
	MDEC_rect.w=16;
	MDEC_rect.h=sector->height;
	// find nearest 16 below the width of the video (normally width-16)
	ASSERT((sector->width&15)==0);
	ASSERT((sector->height&15)==0);
	MDEC_width=(sector->width-1)&0xff0;
	MDEC_height=sector->height;


	DecDCTvlcSize2(0);
	decode=DecDCTvlc2(addr,MDEC_slice,*MDEC_VLCTable);
	StFreeRing(addr);
	DecDCTin(MDEC_slice,0);

	MDEC_frame=0;
	MDEC_done=0;
	DecDCTout(MDEC_output[0],MDEC_height*8);

	while(MDEC_done==0);

	DrawSync(0);
	MDEC_CurrentFrame=sector->frameCount;
	if ((sector->frameCount>=MDEC_endframe))
		return 1;
	if ((sector->frameCount>5)&&(PadKeyIsPressed(&PAD_Input1,PAD_RD)||PadKeyIsPressed(&PAD_Input1,PAD_START)))
		return 1;
#endif
	return 0;
}

int MDEC_Close(void)
{
#ifndef VERSION_REVIEW
	CdPause();
	DecDCToutCallback(0);
//	StUnSetRing();
	MemFree((void*)MDEC_input);
	MemFree((void*)MDEC_slice);
	MemFree((void*)MDEC_output[0]);
	MemFree((void*)MDEC_output[1]);
	MemFree((void*)MDEC_VLCTable);
	MemFree((void*)MDEC_vibra);
	PSX_SetShock(0,0);
#endif
}

void MDEC_VideoSet(int width,int height)
{
	SetDefDrawEnv(&the_display.DisplayBuffers[0].Draw, 0,   0, width, height);	
	SetDefDrawEnv(&the_display.DisplayBuffers[1].Draw, 0, 256, width, height);
	SetDefDispEnv(&the_display.DisplayBuffers[0].Disp, 0, 256, width, height);
	SetDefDispEnv(&the_display.DisplayBuffers[1].Disp, 0,   0, width, height);

	the_display.DisplayBuffers[0].Draw.isbg=1;
	the_display.DisplayBuffers[0].Draw.r0=0;
	the_display.DisplayBuffers[0].Draw.g0=0;
	the_display.DisplayBuffers[0].Draw.b0=0;

	the_display.DisplayBuffers[1].Draw.isbg=1;
	the_display.DisplayBuffers[1].Draw.r0=0;
	the_display.DisplayBuffers[1].Draw.g0=0;
	the_display.DisplayBuffers[1].Draw.b0=0;

	the_display.DisplayBuffers[0].Disp.screen.x=screen_x;
	the_display.DisplayBuffers[1].Disp.screen.x=screen_x;
	the_display.DisplayBuffers[0].Disp.screen.y=screen_y;
	the_display.DisplayBuffers[1].Disp.screen.y=screen_y;
	
	SetGeomOffset(width>>1, height>>1);	/* set geometry origin as (160, 120) */

}

extern SLONG MFX_Seek_delay;

extern UBYTE psx_motor[];

MDEC_Shock(int frame)
{
	psx_motor[0]=MDEC_vibra[frame].fast;
	psx_motor[1]=MDEC_vibra[frame].slow;
	PadSetAct(0x00,psx_motor,2);
//	PadSetAct(0x00,&MDEC_vibra[frame],2);
}

SLONG MDEC_Play(char *fname,int len,int lang)
{
#ifndef VERSION_REVIEW
extern int vibra_mode;

	ClearOTag(the_display.DisplayBuffers[0].ot,OTSIZE);
	ClearOTag(the_display.DisplayBuffers[1].ot,OTSIZE);

	MFX_Seek_delay=INFINITY;
	MDEC_VideoSet(320,240);
	MDEC_Init(fname,len);
	while(MDEC_Render()==0)
	{
		if (vibra_mode)
			MDEC_Shock(MDEC_CurrentFrame);
		AENG_flip();
	}
	MDEC_Close();
	if (PadKeyIsPressed(&PAD_Input1,PAD_START))
	{
		return 1;
	}
	MFX_Seek_delay=20;
//	MDEC_VideoSet(512,240);
#endif
	return 0;
}

    