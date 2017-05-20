//
// playcuts.cpp
// matthew rosenfeld 30 june 99
//
// plays back cutscenes made with cutscene.cpp in gedit
//

#include "playcuts.h"
#include "Game.h"

#include "person.h"
#include "psystem.h"
#include "ribbon.h"
#include "dirt.h"
#include "wmove.h"
#include "balloon.h"
#include "pow.h"
#include "puddle.h"
#include "drip.h"
#include "mfx.h"
#include "fc.h"
#include "poly.h"
#include "sound.h"
#include "grenade.h"
#ifndef PSX
#include "polypage.h"
#include "drawxtra.h"
#include "font2d.h"
#endif

//----------------------------------------------------------------------------
// DEFINES
//

// misc
#define SLOMO_RATE			(10)
#define LERP_SHIFT			(8)

// Channel Types
#define CT_CHAR				(1)
#define CT_CAM				(2)
#define CT_WAVE				(3)
//#define CT_FX				(4) // not used
#define CT_TEXT				(5)

// Packet Types
#define PT_ANIM				(1)
//#define PT_ACTION			(2) // not used
#define PT_WAVE 			(3)
#define PT_CAM				(4)
#define PT_TEXT				(5)

// Packet Flags
#define PF_BACKWARDS		(1) // for anims
#define PF_SECURICAM		(1) // for cameras
#define PF_INTERPOLATE_MOVE	(2) // for both
#define PF_SMOOTH_MOVE_IN	(4)
#define PF_SMOOTH_MOVE_OUT	(8)
#define PF_SMOOTH_MOVE_BOTH (PF_SMOOTH_MOVE_IN|PF_SMOOTH_MOVE_OUT)
#define PF_INTERPOLATE_ROT	(16)
#define PF_SMOOTH_ROT_IN	(32)
#define PF_SMOOTH_ROT_OUT	(64)
#define PF_SMOOTH_ROT_BOTH	(PF_SMOOTH_ROT_IN|PF_SMOOTH_ROT_OUT)
#define PF_SLOMO			(128)



//----------------------------------------------------------------------------
// EXTERNAL REFERENCES
//

extern	SLONG	hardware_input_continue(void);
inline	void	screen_flip(void)
{

#ifdef PSX
	//
	//	sCREENSHOT just before screen_flip
	//
extern	void	DoFigureDraw(void);
	DoFigureDraw();

#ifndef FS_ISO9660
extern	void	AENG_screen_shot(SLONG width);
			if (Keys[KB_S])
			{
				AENG_screen_shot(320);
				Keys[KB_S]=0;
			}
#endif
#endif

	//
	// Always flip on the PSX.
	//

	AENG_flip();
}
extern	void	lock_frame_rate(SLONG fps);
extern	SLONG	person_normal_animate(Thing *p_person);


typedef Thing* ThingPtr;


//----------------------------------------------------------------------------
// GLOBALS
//

CBYTE *text_disp=0;
UBYTE PLAYCUTS_fade_level=255;
BOOL  PLAYCUTS_slomo=0;
BOOL  PLAYCUTS_playing=0;
UBYTE PLAYCUTS_slomo_ctr=0;
UBYTE no_more_packets=0;


//----------------------------------------------------------------------------
// MAJOR CONSTRUCTS
//

#ifndef PSX
CBYTE		PLAYCUTS_text_data[MAX_CUTSCENE_TEXT];
CBYTE		*PLAYCUTS_text_ptr=PLAYCUTS_text_data;
CPData		PLAYCUTS_cutscenes[MAX_CUTSCENES];
CPPacket	PLAYCUTS_packets[MAX_CUTSCENE_PACKETS];
CPChannel	PLAYCUTS_tracks[MAX_CUTSCENE_TRACKS];
#else
CBYTE		*PLAYCUTS_text_data;
CPData		*PLAYCUTS_cutscenes;
CPPacket	*PLAYCUTS_packets;
CPChannel	*PLAYCUTS_tracks;
#endif
UWORD	PLAYCUTS_cutscene_ctr=0;
UWORD	PLAYCUTS_packet_ctr=0;
UWORD	PLAYCUTS_track_ctr=0;
UWORD	PLAYCUTS_text_ctr=0;

//----------------------------------------------------------------------------
// MISC USEFUL FUNCTIONS
//

inline SLONG LERPValue(SLONG a, SLONG b, SLONG m) {
	return a+(((b-a)*m)>>LERP_SHIFT);
}

inline int  LERPAngle(SLONG a, SLONG b, SLONG m) {
	if ((a-b)>1024) b+=2048;
	if ((b-a)>1024) a+=2048;
	return LERPValue(a,b,m);
}

#ifndef PSX

//----------------------------------------------------------------------------
// CUTSCENE SUPPORT CODE
//

// Reading cutscenes and the bits that make them up from disk

void PLAYCUTS_Read_Packet(MFFileHandle handle, CPPacket *packet) {

	FileRead(handle, packet, sizeof(CPPacket));

	switch(packet->type) {
	case PT_TEXT:
		if (packet->pos.X) {
			SLONG l;
			FileRead(handle,&l,sizeof(l));
			FileRead(handle,PLAYCUTS_text_ptr,l);
			packet->pos.X=(SLONG)PLAYCUTS_text_ptr;
			PLAYCUTS_text_ptr+=l;
			*PLAYCUTS_text_ptr=0; PLAYCUTS_text_ptr++;
			PLAYCUTS_text_ctr+=(l+1);
		}
		break;
	}

}

void PLAYCUTS_Read_Channel(MFFileHandle handle, CPChannel *channel) {
	SLONG packnum;

	FileRead(handle, channel, sizeof(CPChannel));
//	channel->packets = new CPPacket[channel->packetcount];
	channel->packets = PLAYCUTS_packets+PLAYCUTS_packet_ctr;
	PLAYCUTS_packet_ctr+=channel->packetcount;

	for (packnum=0;packnum<channel->packetcount;packnum++) 
		PLAYCUTS_Read_Packet(handle, channel->packets+packnum);

}

CPData* PLAYCUTS_Read(MFFileHandle handle) {
	UBYTE channum;
	CPData* cutscene;

//	cutscene = new CPData;
	cutscene = PLAYCUTS_cutscenes+PLAYCUTS_cutscene_ctr;
	PLAYCUTS_cutscene_ctr++;
	cutscene->channels=0;
	FileRead(handle,&cutscene->version,1);
	FileRead(handle,&cutscene->channelcount,1);
//	cutscene->channels = new CPChannel[cutscene->channelcount];
	cutscene->channels = PLAYCUTS_tracks + PLAYCUTS_track_ctr;
	PLAYCUTS_track_ctr+=cutscene->channelcount;
	for (channum=0;channum<cutscene->channelcount;channum++) 
		PLAYCUTS_Read_Channel(handle,cutscene->channels+channum);
	return cutscene;
}

// Freeing finished-with cutscenes

void PLAYCUTS_Free_Chan(CPChannel *chan) {
	// we could free stuff on a packet by packet basis...
	// ... but right now, there's nothing to free except text messages...
	// ... and they get dealt with seperately
	
	//we don't even do this now we have them pseudostatically allocated...
	//delete [] chan->packets;	
}

void PLAYCUTS_Free(CPData *cutscene) {
	int chan;
//	for (chan=0;chan<cutscene->channelcount;chan++) PLAYCUTS_Free_Chan(cutscene->channels+chan);
//	delete [] cutscene->channels;
//	delete cutscene;
}

void PLAYCUTS_Reset() {
	PLAYCUTS_cutscene_ctr=0;
	PLAYCUTS_track_ctr=0;
	PLAYCUTS_packet_ctr=0;
}
#endif

// finds the packet that matches a cell

CPPacket *PLAYCUTS_Get_Packet(CPChannel* chan, SLONG cell) {
	CPPacket* pkt=chan->packets;
	SLONG ctr=1;
	if ((cell<0)||(cell>2000)) return 0;
	while (pkt&&(pkt->start!=cell)) {
		pkt++;
		ctr++;
		if (ctr>chan->packetcount) pkt=0;
	}
	return pkt;
}

// finds the packets to the left and right of a cell

BOOL PLAYCUTS_find_surrounding_packets(CPChannel *chan, SLONG cell, SLONG* left, SLONG* right) {
	SLONG leftmax=-1, rightmin=2001, packctr=0;
	CPPacket *pack;
	*left=-1; *right=2001;
	pack=chan->packets;
	for (packctr=0;packctr<chan->packetcount;packctr++,pack++) {
		if ((pack->start<=cell)&&(pack->start>leftmax)) {
			leftmax=pack->start; *left=packctr;
		}
		if ((pack->start>=cell)&&(pack->start<rightmin)) {
			rightmin=pack->start; *right=packctr;
		}
	}
	return ((leftmax>-1)&&(rightmin<2000));
}

//----------------------------------------------------------------------------
// PLAYING THE DAMN THINGS
//

void LERPAnim(CPChannel *chan, Thing *person, SLONG cell, SLONG sub_ctr) {
	SLONG left,right,lastpos,pos,dist, a, mult, smoothmult, smoothin, smoothout;
	int x,y,z;
	CPPacket *pktA, *pktB;
	GameCoord posn;

	PLAYCUTS_find_surrounding_packets(chan,cell,&left,&right);
	if (right==2001) no_more_packets++;
	if (left<0) return;
	pktA=chan->packets+left;
	pos=cell-pktA->start;

	// interpolation
	if ((right<2001)&&(right!=left)) {
		pktB=chan->packets+right;
		dist=pktB->start-pktA->start;
		mult=(pos<<LERP_SHIFT);
		if (PLAYCUTS_slomo) {
//			mult+=((SLOMO_RATE-CUTSCENE_slomo_ctr)<<LERP_SHIFT)/SLOMO_RATE;
			mult+=(PLAYCUTS_slomo_ctr<<LERP_SHIFT)/SLOMO_RATE;
		}
		mult/=dist;


		if (pktA->flags&(PF_SMOOTH_MOVE_BOTH|PF_SMOOTH_ROT_BOTH)) {
			// this little fiddle should smooth in/out the edges
			smoothmult=(mult-128)*4;
			smoothmult=(SIN(smoothmult&2047)>>8)+256;
			smoothmult>>=1;
			smoothin=(mult-256)*2;
			smoothin=256+(SIN(smoothin&2047)>>8);
			smoothout=SIN(mult*2)>>8;
		}		

		// motion:
		if (pktA->flags&PF_INTERPOLATE_MOVE) {
			if ((pktA->flags&PF_SMOOTH_MOVE_BOTH)==PF_SMOOTH_MOVE_BOTH) {
				x=LERPValue(pktA->pos.X,pktB->pos.X,smoothmult);
				y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothmult);
				z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothmult);
			} else
				if (pktA->flags&PF_SMOOTH_MOVE_IN) {
					x=LERPValue(pktA->pos.X,pktB->pos.X,smoothin);
					y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothin);
					z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothin);
				} else
					if (pktA->flags&PF_SMOOTH_MOVE_OUT) {
						x=LERPValue(pktA->pos.X,pktB->pos.X,smoothout);
						y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothout);
						z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothout);
					} else { // linear
						x=LERPValue(pktA->pos.X,pktB->pos.X,mult);
						y=LERPValue(pktA->pos.Y,pktB->pos.Y,mult);
						z=LERPValue(pktA->pos.Z,pktB->pos.Z,mult);
					}
		} else {
			x=pktA->pos.X;
			y=pktA->pos.Y;
			z=pktA->pos.Z;
		}
		
		// er... later.

		// rotation:
		if (pktA->flags&PF_INTERPOLATE_ROT) {
			if ((pktA->flags&PF_SMOOTH_ROT_BOTH)==PF_SMOOTH_ROT_BOTH) {
				mult=smoothmult;
			} else
				if (pktA->flags&PF_SMOOTH_ROT_IN) {
					mult=smoothin;
				} else
					if (pktA->flags&PF_SMOOTH_ROT_OUT) {
						mult=smoothout;
					} // else mult is already correct for linear
			a=LERPAngle(pktA->angle,pktB->angle,mult);
		} else {
			a=pktA->angle;
		}
		person->Draw.Tweened->Angle=a;
		posn.X=x; posn.Y=y; posn.Z=z;
		move_thing_on_map(person,&posn);

	}


	set_anim(person,pktA->index);
	if (pktA->flags&1) // reverse
		pos=(pktA->length-pos)-1;
	if (pos<=0) return;
	TICK_RATIO=pos<<TICK_SHIFT;
	if (PLAYCUTS_slomo) {
//		TICK_RATIO+=((SLOMO_RATE-CUTSCENE_slomo_ctr)<<TICK_SHIFT)/SLOMO_RATE;
		TICK_RATIO+=(PLAYCUTS_slomo_ctr<<TICK_SHIFT)/SLOMO_RATE;
	}
	person_normal_animate(person);


}

void LERPCamera(CPChannel *chan, SLONG cell, SLONG sub_ctr) {
	SLONG x,y,z,p,a, left, right, dist, pos, mult, smoothin, smoothout, smoothmult,lens;
	CPPacket *pktA,*pktB;

	if (!PLAYCUTS_find_surrounding_packets(chan, cell,&left,&right)) {
		if (right==2001) no_more_packets++;
		return;
	}

	pktA=chan->packets+left;
	pktB=chan->packets+right;
	dist=pktB->start-pktA->start;
	pos=cell-pktA->start;

	mult=(pos*256);
	if (PLAYCUTS_slomo) {
	//	mult+=((SLOMO_RATE-CUTSCENE_slomo_ctr)<<LERP_SHIFT)/SLOMO_RATE;
		mult+=(PLAYCUTS_slomo_ctr<<LERP_SHIFT)/SLOMO_RATE;
	}
	mult/=dist;

	if (pktA->flags&(PF_SMOOTH_MOVE_BOTH|PF_SMOOTH_ROT_BOTH)) {
		// this little fiddle should smooth in/out the edges
		smoothmult=(mult-128)*4;
		smoothmult=(SIN(smoothmult&2047)>>8)+256;
		smoothmult>>=1;
		smoothin=(mult-256)*2;
		smoothin=256+(SIN(smoothin&2047)>>8);
		smoothout=SIN(mult*2)>>8;
	}
	
	if (pktA->flags&PF_INTERPOLATE_MOVE) {
		if ((pktA->flags&PF_SMOOTH_MOVE_BOTH)==PF_SMOOTH_MOVE_BOTH) {
			x=LERPValue(pktA->pos.X,pktB->pos.X,smoothmult);
			y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothmult);
			z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothmult);
			lens=LERPValue(pktA->length&0xff,pktB->length&0xff,smoothmult);
		} else
			if (pktA->flags&PF_SMOOTH_MOVE_IN) {
				x=LERPValue(pktA->pos.X,pktB->pos.X,smoothin);
				y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothin);
				z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothin);
				lens=LERPValue(pktA->length&0xff,pktB->length&0xff,smoothin);
			} else
				if (pktA->flags&PF_SMOOTH_MOVE_OUT) {
					x=LERPValue(pktA->pos.X,pktB->pos.X,smoothout);
					y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothout);
					z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothout);
					lens=LERPValue(pktA->length&0xff,pktB->length&0xff,smoothout);
				} else { // linear
					x=LERPValue(pktA->pos.X,pktB->pos.X,mult);
					y=LERPValue(pktA->pos.Y,pktB->pos.Y,mult);
					z=LERPValue(pktA->pos.Z,pktB->pos.Z,mult);
					lens=LERPValue(pktA->length&0xff,pktB->length&0xff,mult);
				}
	} else { // snap-to
		x=pktA->pos.X;
		y=pktA->pos.Y;
		z=pktA->pos.Z;
		lens=pktA->length&0xff;
	}
	lens=((lens-0x7f)*1.5f)+0xff;
	FC_cam[0].lens=(lens*0x24000)>>8;

	if (pktA->flags&PF_INTERPOLATE_ROT) {
		if ((pktA->flags&PF_SMOOTH_ROT_BOTH)==PF_SMOOTH_ROT_BOTH) {
			mult=smoothmult;
		} else
			if (pktA->flags&PF_SMOOTH_ROT_IN) {
				mult=smoothin;
			} else
				if (pktA->flags&PF_SMOOTH_ROT_OUT) {
					mult=smoothout;
				} // else mult is already correct for linear
		p=LERPAngle(pktA->pitch,pktB->pitch,mult);
		a=LERPAngle(pktA->angle,pktB->angle,mult);
	} else {
		p=pktA->pitch;
		a=pktA->angle;
	}

//	AENG_set_camera(x,y,z,(-p)&2047,(-a)&2047,0);
	AENG_set_camera(x,y,z,(a)&2047,(p)&2047,0);
	PLAYCUTS_fade_level=LERPValue(pktA->length>>8,pktB->length>>8,mult);
}




// update an individual actor

void PLAYCUTS_Update(CPChannel *chan, Thing *thing, SLONG read_head, SLONG sub_ctr) {
	CPPacket  *pkt;
	int index,lens;
	SLONG l,r;

	pkt=PLAYCUTS_Get_Packet(chan,read_head);
	if (!pkt) {
		switch(chan->type) {
		case CT_CHAR:
			LERPAnim(chan,thing,read_head, sub_ctr);
			break;
		case CT_CAM:
			LERPCamera(chan,read_head,sub_ctr);
			break;
		default:
			// check for end-of-track
			PLAYCUTS_find_surrounding_packets(chan, read_head, &l, &r);
			if (r==2001) no_more_packets++;
			break;
		}
		return;
	}
	switch(pkt->type) {
		case PT_ANIM:
			index=pkt->index;
			while (index>1023) { index-=1024; }
			if (thing) {
				move_thing_on_map(thing,&pkt->pos);
				thing->Draw.Tweened->Angle=pkt->angle;
				set_anim(thing,index);
				if (pkt->flags&1) LERPAnim(chan,thing,read_head,sub_ctr); // should handle the reversing
			}
			break;

		case PT_CAM:
//			AENG_set_camera(pkt->pos.X,pkt->pos.Y,pkt->pos.Z,pkt->pitch&2047,pkt->angle&2047,0);
			AENG_set_camera(pkt->pos.X,pkt->pos.Y,pkt->pos.Z,pkt->angle&2047,pkt->pitch&2047,0);
			lens=((pkt->length&0xff)-0x7f);
			lens=(lens*1.5f)+0xff;
			FC_cam[0].lens=(lens*0x24000)>>8;
#ifndef PSX
			PolyPage::SetGreenScreen(pkt->flags&PF_SECURICAM);
#endif
			PLAYCUTS_slomo=pkt->flags&PF_SLOMO;
			PLAYCUTS_fade_level=pkt->length>>8;
			break;

		case PT_WAVE:
			  MFX_play_xyz(MUSIC_REF+10,pkt->index-2048,0,pkt->pos.X<<8,pkt->pos.Y<<8,pkt->pos.Z<<8);
			break;

		case PT_TEXT:
			if (pkt->pos.X) 
				text_disp=(CBYTE*)pkt->pos.X;
			else
				text_disp=0;
			break;
	}
}

// main loop

void PLAYCUTS_Play(CPData *cutscene) {
	UBYTE env_frame_rate = 20;
	SLONG read_head=0, sub_ctr=0;
	UBYTE channum;
	ThingPtr *cs_things;
	Thing* darci=NET_PERSON(0);
	CPChannel *chan;

	// sensible defaults
	text_disp=0;
	PLAYCUTS_fade_level=255;
	PLAYCUTS_slomo=0;
	PLAYCUTS_slomo_ctr=0;
	PLAYCUTS_playing=1;

	cs_things = new ThingPtr[cutscene->channelcount];
	for (channum=0,chan=cutscene->channels;channum<cutscene->channelcount;channum++,chan++) 
	{
		if (chan->type==CT_CHAR) {
			cs_things[channum]=TO_THING(
				create_person(
					chan->index-1,
					0,
					0,
					0,
					0
				)
			);
		} else {
			cs_things[channum]=0;
		}
	}

//	already_warned_about_leaving_map = GetTickCount();

	Keys[KB_SPACE]=0;
	LastKey=0;
	no_more_packets=0;
	
	remove_thing_from_map(darci);

	while(SHELL_ACTIVE&&(!Keys[KB_SPACE])&&(!no_more_packets)&&!hardware_input_continue())
	{
		// process some generic stuff. not all of it (yet...)
		PARTICLE_Run();
		//OB_process();
		//TRIP_process();
		//DOOR_process();
		//EWAY_process();
#ifndef PSX
		RIBBON_process();
		DIRT_process();
		ProcessGrenades();
#ifndef TARGET_DC
		WMOVE_draw();
		BALLOON_process();
#endif
		//MAP_process();
		POW_process();
#ifndef TARGET_DC
		PUDDLE_process();
#endif
		DRIP_process();
#endif
		//FC_process(); // camera is overriden by funky one


		// Update the cutscene's 'actors'
		for (channum=0;channum<cutscene->channelcount;channum++) 
			{
				chan=cutscene->channels+channum;
				PLAYCUTS_Update(chan,cs_things[channum],read_head,sub_ctr);
			}
		if (no_more_packets<channum) no_more_packets=0;

		MFX_set_listener(FC_cam[0].x,FC_cam[0].y,FC_cam[0].z,-(FC_cam[0].yaw>>8),-(1024),-(FC_cam[0].pitch>>8));

		AENG_draw(FALSE);

		if (text_disp) {
#ifndef PSX
			POLY_frame_init(FALSE, FALSE);
#endif
			FONT2D_DrawStringCentred(text_disp,320,400,0x7fffffff,256,POLY_PAGE_FONT2D);
#ifndef PSX
			POLY_frame_draw(FALSE, FALSE);
#endif
		}

		if (PLAYCUTS_fade_level<255) {
#ifndef PSX
			POLY_frame_init(FALSE, FALSE);
			DRAW2D_Box(0, 0, 640, 480, (255-PLAYCUTS_fade_level)<<24, 1, 255);
			POLY_frame_draw(FALSE, FALSE);
#endif
		}

		
		//draw_screen();

		//
		// Draw panel and other exciting things
		//
		// OVERLAY_handle(); prolly don't wanna do that for cutscenes

		MFX_render();

		screen_flip();

		//
		// Lock frame-rate to 30 FPS
		//
		lock_frame_rate(env_frame_rate);


		GAME_TURN++;

		if (PLAYCUTS_slomo) {
			PLAYCUTS_slomo_ctr++;
			if (PLAYCUTS_slomo_ctr==SLOMO_RATE) {
				read_head++;
				PLAYCUTS_slomo_ctr=0;
			}
		} else {
			read_head++;
		}
	}

	add_thing_to_map(darci);

	Keys[KB_SPACE]=0;

	// clean up afterwards
	for (channum=0,chan=cutscene->channels;channum<cutscene->channelcount;channum++,chan++) {
		if (cs_things[channum]) THING_kill(cs_things[channum]);
	}
	delete [] cs_things;
	PLAYCUTS_playing=0;
}


//