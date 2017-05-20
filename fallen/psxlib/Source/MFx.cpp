//
// MFx.cpp
//
// Muckyfoot sound fx api for A3D / PSX
//
// (same header, different cpp)
//
#include "MFStdLib.h"
#include "MFx.h"

#include <libspu.h>
#include <libgte.h>
#include <libcd.h>

#include <libpad.h>

#include "ctrller.h"

#include "c:\fallen\psxeng\headers\psxeng.h"

#include "c:\fallen\headers\sound_id.h"
#include "c:\fallen\headers\music.h"
#include "c:\fallen\headers\statedef.h"
#include "c:\fallen\headers\animate.h"

#include "c:\fallen\psxlib\headers\mfxmusic.h"

#define MFX_MAX_CHANNELS	24
#define MFX_MAX_WAVES		547

#define MFX_FLAG_AMBIENT	(32768)

#ifdef NTSC
#define MFX_BYTES_PER_FRAME	35
#else
#define MFX_BYTES_PER_FRAME 45
#endif

#define S_PITCH 0x400		// 8000

CdlFILE MFX_audio_start;
extern char *PANEL_wide_cont;
extern char PANEL_wide_text[];

typedef struct {
	SWORD	waypoint;
	UBYTE	conversation;
	UBYTE	conv_off;
	UBYTE	channel;
	UBYTE   pack;
	SWORD	length;
	SWORD	offset;
} AudioOffset;


SLONG MFX_Speech_files;
SLONG MFX_Cd_Position;
SLONG MFX_Seek_delay=0;
SLONG MFX_music_stop=0;
SLONG MFX_sound_frame;

AudioOffset Audio_data[128];
CdlFILE MFX_Speech_Start;
SLONG	MFX_Speech_End;
#if 1
extern char *GDisp_Bucket;
#else
extern char GDisp_Bucket[];
#endif
SLONG MFX_Conv_playing=0;

extern SWORD music_current_level;
extern UBYTE music_current_mode;
SLONG MFX_music_end;
SLONG MFX_music_queued;
SLONG MFX_music_gain;
SLONG MFX_music_q_flag;
SLONG MFX_music_pending;
SLONG MFX_music_world;

typedef struct {
	GameCoord *position;
	GameCoord poshold;
	UWORD	  channel_id;
	UBYTE	  channel;
	UBYTE	  in_use;
	ULONG	  wave;
	ULONG	  end_frame;
	ULONG	  flags;
	SpuVoiceAttr voice;
} MFX_Sound;

typedef struct {
	GameCoord position;
	SVECTOR	  orientation;
	SLONG	  environment;
} MFX_Listener;

typedef struct {
	SLONG	  address;
	SLONG	  length;
} MFX_Wave;

MFX_Sound	MFX_channel[MFX_MAX_CHANNELS];
MFX_Listener MFX_listener;
MFX_Wave	MFX_wave[MFX_MAX_WAVES];

SLONG		MFX_WavesLoaded;
SLONG		MFX_WaveFree;
SLONG		MFX_music_wave;
SLONG		MFX_music_int;

SLONG MFX_OnKey,MFX_OffKey;

void MFX_Conv_stop();

//----- transport functions -----

inline ULONG MFX_FindPSXChannel(ULONG channel_id,ULONG wave,ULONG flags) 
{
	int i=MFX_MAX_CHANNELS,c0;
	channel_id&=0xffff;
	c0=channel_id%MFX_MAX_CHANNELS;

	while((i--)&&(MFX_channel[c0].in_use))
	{
		if ((MFX_channel[c0].in_use)&&(MFX_channel[c0].channel_id==channel_id)&&(MFX_channel[c0].wave==wave))
		{
			if (flags&MFX_REPLACE)
				return c0;
			if (!(flags&MFX_OVERLAP))
				return 0xffff;
		}
		c0=(c0+1)%MFX_MAX_CHANNELS;
	}
	// We have no channels free, play on the original intended channel no matter what.
	return c0;
}

inline ULONG MFX_SearchPSXChannel(ULONG channel_id,ULONG wave) 
{
	int i=MFX_MAX_CHANNELS,c0;
	channel_id&=0xffff;
	c0=channel_id%MFX_MAX_CHANNELS;

	for(i=0;i<MFX_MAX_CHANNELS;i++)
	{
		if (MFX_channel[c0].in_use)
		{
			if (MFX_channel[c0].channel_id==channel_id)
			{
				if (wave==MFX_WAVE_ALL)
					return c0;
				if (MFX_channel[c0].wave==wave)
					return c0;
			}
		}
		c0=(c0+1)%MFX_MAX_CHANNELS;
	}
	return 0xffff;
}

void MFX_StartWave(MFX_Sound *channel,ULONG wave,ULONG flags)
{
	if (MFX_wave[wave].length)
	{
		channel->wave=wave;
		channel->in_use=1;
		channel->flags=flags;
		channel->voice.voice=1<<channel->channel;
//		printf("channel: %d\n",channel->channel_id);
		channel->voice.addr=MFX_wave[wave].address;
		channel->voice.volume.left=0x3fff;
		channel->voice.volume.right=0x3fff;
		channel->voice.pitch=S_PITCH;
		channel->voice.mask|=SPU_VOICE_WDSA|SPU_VOICE_VOLL|SPU_VOICE_VOLR|SPU_VOICE_PITCH;
		if (flags&MFX_LOOPED)
			channel->end_frame=INFINITY;
		else
			channel->end_frame=MFX_sound_frame+(MFX_wave[wave].length/MFX_BYTES_PER_FRAME)+1;

		MFX_OnKey|=1<<channel->channel;
	}	  

	/*
	if ((wave>=S_PISTOL_SHOT)&&(wave<=S_PUNCH_END))
		channel->flags|=MFX_SHOT;
	if ((wave>=S_EXPLODE_START)&&(wave<=S_EXPLODE_END))
		channel->flags|=MFX_EXPLODE;
	if ((wave==S_SHOTGUN_SHOT)||(wave==S_DARCI_HIT_START)||(wave==S_DARCI_HIT_END))
		channel->flags|=MFX_SHOT;
	*/
}

void MFX_StopWave(MFX_Sound *channel,ULONG wave)
{
	// Check we are playing the wave we expected
	if (channel->wave!=wave)
		return;

	channel->in_use=0;
	MFX_OffKey|=1<<channel->channel;
}

void	MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z)
{
	UWORD	psx_channel=MFX_FindPSXChannel(channel_id,wave,flags);
	if (psx_channel==0xffff)
		return;
//	ASSERT(!(flags & MFX_FLAG_SEARCHER));

	MFX_channel[psx_channel].poshold.X=x;
	MFX_channel[psx_channel].poshold.Y=y;
	MFX_channel[psx_channel].poshold.Z=z;
	MFX_channel[psx_channel].position=&MFX_channel[psx_channel].poshold;
	MFX_channel[psx_channel].channel_id=channel_id&0xffff;
	MFX_StartWave(&MFX_channel[psx_channel],wave,flags);
}

void	MFX_play_pos(UWORD channel_id, ULONG wave, ULONG flags, GameCoord* position)
{
	UWORD	psx_channel=MFX_FindPSXChannel(channel_id,wave,flags);
	if (psx_channel==0xffff)
		return;
//	ASSERT(!(flags & MFX_FLAG_SEARCHER));
	MFX_channel[psx_channel].position=position;
	MFX_channel[psx_channel].channel_id=channel_id&0xffff;
	MFX_StartWave(&MFX_channel[psx_channel],wave,flags);
}

void	MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p)
{
	UWORD	psx_channel=MFX_FindPSXChannel(channel_id,wave,flags);
	if (psx_channel==0xffff)
		return;

//	if(p->Class==CLASS_VEHICLE)
//		ASSERT(p->Genus.Vehicle->Driver);
//	ASSERT(!(flags & MFX_FLAG_SEARCHER));
	MFX_channel[psx_channel].position=&p->WorldPos;
	MFX_channel[psx_channel].channel_id=channel_id&0xffff;
	if (p->Genus.Person->PlayerID)
		flags|=MFX_PLAYER;
	// Bodge in a check to see if we're playing a search or slide sound.

//	if (flags & (MFX_FLAG_SLIDER|MFX_FLAG_SEARCHER)) //just store it all the time, the instructions wont be in the cache so a read and a write is probably no worse than reading the extra instructions
		MFX_channel[psx_channel].poshold.X=(SLONG)p;

	MFX_StartWave(&MFX_channel[psx_channel],wave,flags);
}

void	MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags)
{
	UWORD	psx_channel=MFX_FindPSXChannel(channel_id,wave,flags);
	if (psx_channel==0xffff)
		return;
	MFX_channel[psx_channel].channel_id=channel_id&0xffff;
//	ASSERT(!(flags & MFX_FLAG_SEARCHER));
	if (flags & MFX_FLAG_SEARCHER)
	{
		MFX_channel[psx_channel].poshold.X=(SLONG)NET_PERSON(0);
	}
	MFX_StartWave(&MFX_channel[psx_channel],wave,flags|MFX_FLAG_AMBIENT);
}

void	MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags)
{
}

void	MFX_stop(SLONG channel_id, ULONG wave)
{
	UWORD	psx_channel=MFX_SearchPSXChannel(channel_id,wave);
	if (psx_channel==0xffff)
		return;

	if (wave==MFX_WAVE_ALL)
	{
		while(psx_channel!=0xffff)
		{
			printf("Channel: %d\n",psx_channel);
//			ASSERT(!(MFX_channel[psx_channel].flags & MFX_FLAG_SLIDER));
			MFX_channel[psx_channel].in_use=0;
			MFX_OffKey|=1<<MFX_channel[psx_channel].channel;
			psx_channel=MFX_SearchPSXChannel(channel_id,wave);
		}
		printf("Done.\n");
	}
	else
		MFX_StopWave(&MFX_channel[psx_channel],wave);
}

void	MFX_stop_attached(Thing *p)
{
}

//----- audio processing functions -----

void	MFX_SetPitch(MFX_Sound *channel,ULONG wave, SLONG pitchbend)
{
	if (channel->wave!=wave)
		return;
	channel->voice.pitch=(S_PITCH*(pitchbend+256))>>8;
	channel->voice.mask|=SPU_VOICE_PITCH;
}

void	MFX_SetWave(MFX_Sound *channel,ULONG wave, SLONG new_wave)
{
	if (channel->wave!=wave)
		return;
}

void	MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend)
{
	UWORD	psx_channel=MFX_SearchPSXChannel(channel_id,wave);
	if (psx_channel==0xffff)
		return;
	MFX_SetPitch(&MFX_channel[psx_channel],wave,pitchbend);
}

void	MFX_set_wave(UWORD channel_id, ULONG wave, ULONG new_wave)
{
	UWORD	psx_channel=MFX_SearchPSXChannel(channel_id,wave);
	if (psx_channel==0xffff)
		return;
	MFX_SetWave(&MFX_channel[psx_channel],wave,new_wave);
}

void	MFX_set_xyz(UWORD channel_id, ULONG wave, SLONG x, SLONG y, SLONG z)
{
	UWORD	psx_channel=MFX_SearchPSXChannel(channel_id,wave);
	if (psx_channel==0xffff)
		return;
	MFX_channel[channel_id].poshold.X=x;
	MFX_channel[channel_id].poshold.Y=y;
	MFX_channel[channel_id].poshold.Z=z;
	MFX_channel[channel_id].position=&MFX_channel[channel_id].poshold;
}

void	MFX_set_pos(UWORD channel_id, ULONG wave, GameCoord* position)
{
	UWORD	psx_channel=MFX_SearchPSXChannel(channel_id,wave);
	if (psx_channel==0xffff)
		return;
	MFX_channel[channel_id].position=position;
}

//----- listener & environment -----

void	MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch)
{
	MFX_listener.position.X=x;
	MFX_listener.position.Y=y;
	MFX_listener.position.Z=z;
	MFX_listener.orientation.vx=pitch;
	MFX_listener.orientation.vy=heading;
	MFX_listener.orientation.vz=roll;
}

void	MFX_set_environment(SLONG env_type)
{
	MFX_listener.environment=env_type;
}

//----- sound library functions -----

extern CBYTE *sound_list[];

void	MFX_load_wave_list(CBYTE *path,CBYTE *script_file)
{
}
#if 0
void	MFX_load_wave_file(CBYTE *wave_file)
{
	int wave=MFX_WavesLoaded++;
	int size,handle;
	char *mem;

	handle=SpecialOpen(wave_file);
	if (handle>=0)
	{
		size=SpecialSize(handle);
		mem=(char *)MemAlloc(size);
		ASSERT(mem!=0);
//		printf("Loading VAG: %d %s (%d @ %08x -> %08x)\n",wave,wave_file,size,mem,MFX_WaveFree);
		SpecialRead(handle,mem,size);
		SpecialClose(handle);
		SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
		SpuSetTransferStartAddr(MFX_WaveFree);
		SpuWrite(mem+48,size-48);
		SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

		MFX_wave[wave].address=MFX_WaveFree;
		MFX_wave[wave].length=size-48;

		MFX_WaveFree+=size-48;
		MemFree((void *)mem);
	} 
	else
	{
		if (wave>0)
		{
			MFX_wave[wave].address=MFX_wave[wave-1].address;
			MFX_wave[wave].length=MFX_wave[wave-1].length;
		} 
		else
			MFX_wave[wave].address=MFX_wave[wave].length=0;
	}
}
#endif

void	MFX_load_wave_list(CBYTE *names[]=0)				// load list from array
{
#if 0
	char **ptr;
	char buff[128];

	if (names==0) names=sound_list;

	ptr=names;

	while(**ptr!='!')
	{
		sprintf(buff,"DATA\\SFX\\1622\\%s",*ptr);
		*strrchr(buff,'.')=0;
		strcat(buff,".vag");
		MFX_load_wave_file(buff);
	 	ptr++;
	}
	printf("Done.\n");
#else

#ifdef	MIKE
//	return;
#endif
	SLONG *buffer=(SLONG*)MemAlloc(530*1024);
	SLONG *p=buffer;
	SLONG i;


	ASSERT(buffer!=0);

#ifndef VERSION_DEMO
	PCReadFile("DATA\\PSXSOUND.VAG",(UBYTE*)buffer,512*1025);
#else
	PCReadFile("URBAN\\PSXSOUND.VAG",(UBYTE*)buffer,512*1025);
#endif
	MFX_WavesLoaded=*p++;
	MFX_WaveFree=0x1010;

	printf("MFX_WavesLoaded = %d\n",MFX_WavesLoaded);

	for(i=0;i<MFX_WavesLoaded;i++)
	{
		MFX_wave[i].address=MFX_WaveFree+(SLONG)*p++;
		MFX_wave[i].length=(MFX_WaveFree+(SLONG)*p)-MFX_wave[i].address;
		if ((MFX_wave[i].length==0)&&(i>0))
			MFX_wave[i].length=MFX_wave[i-1].length;
	}
	p++;

	SpuSetTransferMode(SPU_TRANSFER_BY_IO);
	SpuSetTransferStartAddr(MFX_WaveFree);
	SpuWrite((char*)p,(0x80000-MFX_WaveFree));
//	SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

	MemFree((void*)buffer);
#endif
}

void	MFX_free_wave_list()
{
	MFX_WavesLoaded=0;
//	MFX_WaveFree=0x1010;
}

//----- Callback with one function only -----
/*
void MFX_Callback_CdRead(UBYTE status,UBYTE *result)
{
	CdlLOC buffer[8];

	if (status == CdlDataReady)
	{
        CdGetSector((u_long*)buffer,2);
		MFX_Cd_Position=CdPosToInt(buffer);
	}
}
*/

//----- general system stuff -----

extern int sfx_volume;
extern int music_volume;
extern int sound_mode;
extern int speech_volume;

inline SLONG MFX_Volume(SLONG invol)
{
	return (invol*sfx_volume)>>8;
}

void	MFX_render()
{
	int i;
	MATRIX m;
	MFX_Sound *ch;
	CdlATV mix;
	static CdlLOC sector;

	RotMatrix(&MFX_listener.orientation,&m);
	m.t[0]=-MFX_listener.position.X >> 10;//8;
	m.t[1]=-MFX_listener.position.Y >> 10;//8;
	m.t[2]=-MFX_listener.position.Z >> 10;//8;

	SetRotMatrix(&m);
	SetTransMatrix(&m);

	ch=&MFX_channel[0];

	for(i=0;i<MFX_MAX_CHANNELS;i++)
	{
		// Process each channel
		if (ch->in_use)
		{

			if(!(ch->flags&MFX_FLAG_AMBIENT))
			{
				// Process any non-ambient sounds
				VECTOR v;
				VECTOR lv;
				SLONG flags,dist;

				GameCoord *p=ch->position;
				v.vx=p->X>>10;
				v.vy=p->Y>>10;
				v.vz=p->Z>>10;
				TransRot_32(&v,&lv,&flags);

				dist=(lv.vx*lv.vx)+(lv.vy*lv.vy)+(lv.vz*lv.vz);

				if (dist<0x3fffff)
				{
					// Calculate Left/Right panning
					if (sound_mode)
					{
						ch->voice.mask|=SPU_VOICE_VOLL|SPU_VOICE_VOLR;
						ch->voice.volume.left=MFX_Volume(0x3fff-(dist>>((lv.vx<0)?8:7)));
						ch->voice.volume.right=MFX_Volume(0x3fff-(dist>>((lv.vx<0)?7:8)));
					} else
					{
						ch->voice.mask|=SPU_VOICE_VOLL|SPU_VOICE_VOLR;
						ch->voice.volume.left=ch->voice.volume.right=MFX_Volume(0x3fff-(dist>>7));
					}

					SATURATE(ch->voice.volume.left,0,0x3fff);
					SATURATE(ch->voice.volume.right,0,0x3fff);


				} else
				{
					ch->voice.mask|=SPU_VOICE_VOLL|SPU_VOICE_VOLR;
					ch->voice.volume.left=ch->voice.volume.right=0;
				}
			} else
			{
				ch->voice.mask|=SPU_VOICE_VOLL|SPU_VOICE_VOLR;
				ch->voice.volume.left=ch->voice.volume.right=MFX_Volume(0x3fff);
			}

			if (ch->flags & MFX_FLAG_SLIDER)
			{
				Thing *p=(Thing *)ch->poshold.X;
				if (p->SubState!=SUB_STATE_RUNNING_SKID_STOP ||p->Draw.Tweened->CurrentAnim==ANIM_SLIDER_END)
				{
					MFX_OffKey|=(1<<i);
					ch->in_use=0;
//					ASSERT(0);
				}
			}

			if (ch->flags & MFX_FLAG_SEARCHER)
			{
				Thing *p=(Thing *)ch->poshold.X;
				if (p->State!=STATE_SEARCH)
				{
					MFX_OffKey|=(1<<i);
					ch->in_use=0;
				}
			}


			if (ch->end_frame<=MFX_sound_frame)
			{
				// Turn off all expired channels
				MFX_OffKey|=(1<<i);
				ch->in_use=0;
//				ASSERT(!(ch->flags & MFX_FLAG_SLIDER));
			}

			if (ch->voice.mask)
			{
				SpuSetVoiceAttr(&ch->voice);
				ch->voice.mask=0;
			}
		}
		ch++;
	}
	// If we turn off first, we can catch channels that are supposed to restart again the
	// same frame (well thats the theory)
	if (MFX_OnKey)
	{
		SpuSetKey(SpuOn,MFX_OnKey);
	}
	// Set all the keys changed this frame, including those shutdown by the above loop.
	if (MFX_OffKey)
	{
		SpuSetKey(SpuOff,MFX_OffKey);
	}

	// If we're playing a conversation, check it see if it's ended.

	if (MFX_Conv_playing)
		mix.val0=mix.val1=(128*speech_volume)>>7;
	else
		mix.val0=mix.val1=(music_current_level*music_volume)>>7;

	if (!MFX_Seek_delay)
	{
		CdControl(CdlGetlocL,0,(UBYTE*)&sector);
		MFX_Cd_Position=CdPosToInt(&sector);
	} else
	{
		// If we're seeking then turn the volume off.
		mix.val0=mix.val1=0;
		MFX_Cd_Position=0;
		CdControl(CdlNop,0,0);
		if (CdStatus() & CdlStatRead)
			MFX_Seek_delay=0;
	}

	if (MFX_Conv_playing&&!MFX_Seek_delay)
	{
		CdlLOC	sector;
		SLONG pos;
#ifndef	MIKE

		if ((MFX_Cd_Position>MFX_Speech_End)&&(MFX_Cd_Position<MFX_music_int))
		{
			if (PANEL_wide_cont)
				PANEL_wide_cont=0;
			MFX_Conv_stop();
			mix.val0=mix.val1=0;
			music_current_level=0;
		}
#endif
	}


	if (MFX_music_wave!=-1)
	{
		if (MFX_Cd_Position>=MFX_music_end)
		{
			MUSIC_stop(0);
			mix.val0=mix.val1=0;
		}
	}

   	mix.val2=mix.val3=0;
	CdMix(&mix);


	// Clear the keys and mask

	MFX_OnKey=0;
	MFX_OffKey=0;
}

void MFX_init(void)
{
	SpuCommonAttr attr;
	SpuVoiceAttr s_attr;
	int i;

	SpuInit();

	MFX_free_wave_list();

	attr.mask=(SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
	attr.mvol.left=0x1fff;
	attr.mvol.right=0x1fff;
	attr.cd.volume.left=0x3fff;
	attr.cd.volume.right=0x3fff;
	attr.cd.mix=SPU_ON;

	SpuSetCommonAttr(&attr);

	s_attr.mask = (SPU_VOICE_VOLL |
				   SPU_VOICE_VOLR |
				   SPU_VOICE_VOLMODEL |
				   SPU_VOICE_VOLMODER |
				   SPU_VOICE_PITCH |
				   SPU_VOICE_ADSR_AMODE |
				   SPU_VOICE_ADSR_SMODE |
				   SPU_VOICE_ADSR_RMODE |
				   SPU_VOICE_ADSR_AR |
				   SPU_VOICE_ADSR_DR |
				   SPU_VOICE_ADSR_SR |
				   SPU_VOICE_ADSR_RR |
				   SPU_VOICE_ADSR_SL
		   );

    /* set the attributes with all voice */
    s_attr.voice = SPU_ALLCH;

    /* value of each voice attribute */
    s_attr.volume.left  = 0x1fff;		/* Left volume */
    s_attr.volume.right = 0x1fff;		/* Right volume */
	s_attr.volmode.left = SPU_VOICE_DIRECT;
	s_attr.volmode.right= SPU_VOICE_DIRECT;
    s_attr.pitch        = S_PITCH;		/* Pitch */
    s_attr.a_mode       = SPU_VOICE_LINEARIncN;	/* Attack curve */
    s_attr.s_mode       = SPU_VOICE_LINEARIncN;	/* Sustain curve */
    s_attr.r_mode       = SPU_VOICE_LINEARDecN;	/* Release curve */
    s_attr.ar           = 0x0;			/* Attack rate value */
    s_attr.dr           = 0x0;			/* Decay rate value */
    s_attr.sr           = 0x0;			/* Sustain rate value */
    s_attr.rr           = 0x0;			/* Release rate value */
    s_attr.sl           = 0xf;			/* Sustain level value */

    SpuSetVoiceAttr (&s_attr);

	for(i=0;i<MFX_MAX_CHANNELS;i++)
		MFX_channel[i].channel=i;

	MFX_OnKey=0;
	MFX_OffKey=0;

	MFX_music_wave=-1;
	MFX_sound_frame=0;
	MFX_Conv_playing=0;
}

#ifdef VERSION_NTSC
#define SECOND(x) ((x)*60)
#else
#define SECOND(x) ((x)*50)
#endif

CdlFILTER MFX_filter;

UBYTE MFX_music[]={0,1,2,3,4,1,4,3,5,6,7,0,0,6,4};

extern UBYTE MUSIC_bodge_code;

UBYTE MUSIC_play(UWORD wave,UBYTE flags)
{
#ifndef MIKE
	CBYTE param[8];
	SLONG wv;

	// Dont even think of stopping the speech when it's going.

	if (MFX_Conv_playing)
		return -1;
/*
	if (MUSIC_bodge_code)
	{
		if (MFX_music_wave!=MUSIC_bodge_code)
		{
			printf("Bodging music to : %d\n",MUSIC_bodge_code);
			MFX_music_wave=-1;
			wave=MUSIC_bodge_code;
			flags=0;
//			MFX_music_pending=MUSIC_bodge_code;
			music_current_level=127;
		} else
		{
			flags=0;
		}
	}
*/
	MFX_filter.file=1;

	MFX_filter.chan=MFX_music[wave&0x7f];

//	printf("Bodged music wave: %d (%d)\n",wave,MFX_filter.chan);

	if (MFX_music_wave==-1)
	{
//		printf("Playing: %d (%D - %d)\n",wave,MFX_music_int,MFX_Music_len[MFX_music_world][MFX_filter.chan]);
		MFX_music_pending=wave;
		if (flags&(MUSIC_FLAG_FADE_IN|MUSIC_FLAG_FADE_OUT))
		{
			music_current_level=0;
		}
		MFX_music_wave=wave;
		MFX_music_queued=-1;
		MFX_music_end=MFX_music_int+(MFX_Music_len[MFX_music_world][MFX_filter.chan]<<3);

		CdControlF(CdlSetfilter,(UBYTE*)&MFX_filter);
		CdControlF(CdlReadS,(UBYTE*)&MFX_audio_start.pos);
		MFX_Seek_delay=10;
	} 
	else
	{
//		if (flags&MUSIC_FLAG_FADE_OUT)
//			MFX_music_gain=-2;

		if (flags&MUSIC_FLAG_QUEUED)
		{
			MFX_music_queued=wave;
			MFX_music_q_flag=flags;
		}
	}
	return MFX_music_wave;
#else
	return -1;
#endif
}

void MUSIC_stop(BOOL fade)
{
#ifndef MIKE
	CBYTE param[8];

	if (fade)
	{
		if (MFX_music_gain==0)
			MFX_music_wave=-1;
		else
			MFX_music_gain=-2;
	} else
	{
		// Pause the CD System
//		CdControlF(CdlPause,param);
		// Then seek to the start of the music stuff again.
//		CdControlF(CdlSetloc,(char*)&MFX_audio_start.pos);
		MFX_music_wave=-1;
	}
#endif
}

UWORD MUSIC_wave()
{
	return MFX_music_wave;
}

void	MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain) 
{
	return;
}

#define MUSIC_PHY	0x81
#define MUSIC_COM	0x82
#define MUSIC_DRI	0x83
#define MUSIC_DAN	0x84
#define MUSIC_FIN	0x88

UBYTE MFX_music_worlds[]={MUSIC_COM,MUSIC_PHY,MUSIC_DRI,MUSIC_COM,1,
						  MUSIC_DRI,MUSIC_COM,7,MUSIC_DRI,6,
						  7,6,1,1,3,
						  6,6,6,8,8,
						  6,8,9,5,5,
						  9,4,5,2,5,
						  0,9,MUSIC_FIN,6};

void MUSIC_init_level(SLONG world)
{
#ifndef MIKE
	CBYTE str[32];

	SLONG new_world=world;

	// Cludge for martin
#ifndef VERSION_DEMO
	if (world)
		new_world=MFX_music_worlds[wad_level-1];
#else
	new_world=6;
#endif

	printf("New world: %d (%d)\n",new_world,world);

	if (new_world&0x80)
	{
		MUSIC_bodge_code=new_world&0x7f;
//		printf("MUSIC_bodge_code=%d\n",MUSIC_bodge_code);
		new_world=0;
	} else
		MUSIC_bodge_code=0;

#ifndef VERSION_DEMO
	sprintf(str,"\\XA\\IMUSIC%d.XA;1",new_world);
#else
	sprintf(str,"\\URBAN\\IMUSIC%d.XA;1",new_world);
#endif
	CdSearchFile(&MFX_audio_start,str);
	
	MFX_music_int=CdPosToInt(&MFX_audio_start.pos);
//	printf("Position: %d\n",MFX_music_int);
	MFX_music_stop=0;
	MFX_music_world=new_world;
	MFX_OffKey=-1;
	MUSIC_stop(0);
//	printf("MFX_music_world=%d\n",MFX_music_world);
	MFX_Mute(0);
#endif
}

void MFX_Init_Speech(SLONG level)
{
	char str[32];
#ifndef	MIKE
#ifndef VERSION_DEMO
	sprintf(str,"LEVELS%d\\LEVEL%02d\\CONVERSE.OFF",level/10,level);
#else
	sprintf(str,"URBAN\\LEVEL%02d\\CONVERSE.OFF",level);
#endif
	PCReadFile(str,(unsigned char*)&GDisp_Bucket[BUCKET_MEM-2048],2048);

	MFX_Speech_files=*(SLONG*)&GDisp_Bucket[BUCKET_MEM-2048];
	memcpy((void*)Audio_data,(void*)&GDisp_Bucket[BUCKET_MEM-2044],MFX_Speech_files*sizeof(AudioOffset));

#ifndef VERSION_DEMO
	sprintf(str,"\\LEVELS%d\\LEVEL%02d\\CONVERSE.XA;1",level/10,level);
#else
	sprintf(str,"\\URBAN\\LEVEL%02d\\CONVERSE.XA;1",level);
#endif
	CdSearchFile(&MFX_Speech_Start,str);
	str[0]=CdlModeRT|CdlModeSF|CdlModeSize1;
	CdControlB(CdlSetmode,(UBYTE*)str,0);
#endif
	MFX_Conv_playing=0;
	MFX_Mute(0);
//	CdReadyCallback(MFX_Callback_CdRead);
}

AudioOffset *MFX_Find_Speech(SLONG waypoint,SLONG conv,SLONG conv_off)
{
	SLONG found=-1;
	SLONG i;

	for(i=0;(i<MFX_Speech_files)&&(found==-1);i++)
	{
		if ((Audio_data[i].waypoint==waypoint)&&
			(Audio_data[i].conversation==conv)&&
			(Audio_data[i].conv_off==conv_off))
				found=i;
	}
	if (found!=-1)
		return &Audio_data[found];
	else
	{
		printf("Cannot find: %d (%d,%d)\n",waypoint,conv,conv_off);
		return 0;
	}
}

void MFX_Mute(SLONG mute)
{
	SpuSetMute(mute?SPU_ON:SPU_OFF);
}

void MFX_Conv_stop()
{
	MFX_Conv_playing=0;
#ifndef	MIKE
//	CdControl(CdlPause,0,0);
#endif

	music_current_level=0;
	music_current_mode=-1;
	MFX_music_wave=-1;
}

extern ControllerPacket	PAD_Input1,PAD_Input2;

void MFX_Conv_wait()
{
	int awaiting=0;

	while(MFX_Conv_playing || awaiting)
	{
		VSync(0);

		if (PadKeyIsPressed(&PAD_Input1,PAD_RD))
		{
			if (awaiting)
				continue;
			if (PANEL_wide_cont)
			{
				strcpy(PANEL_wide_text,PANEL_wide_cont);
				PANEL_wide_cont=0;
				awaiting=1;
			}
			else
			{
				MFX_Conv_stop();
				awaiting=1;
			}
		}
		else
			awaiting=0;

		MFX_render();
	}
}

SLONG MFX_Conv_play(SLONG waypoint,SLONG conv,SLONG conv_off)
{
#ifndef	MIKE
	AudioOffset *voice=MFX_Find_Speech(waypoint,conv,conv_off);

	SLONG pos;
	static CdlLOC sector;

	// We should never be calling a sound without any audio track, but since we may
	// this is the surefire way of making sure it dont crash.

	if (voice==0) 
	{
//		printf("Cannot find voice: %d - %d,%d\n",waypoint,conv,conv_off);
		return 0;
	}

	MFX_filter.file=1;
	MFX_filter.chan=voice->channel;

	pos=CdPosToInt(&MFX_Speech_Start.pos);
	pos+=(voice->offset<<4)+voice->channel-11;
	CdIntToPos(pos,&sector);
#ifndef VERSION_GERMAN
	MFX_Speech_End=pos+((voice->length)<<4)+9;
#else
	MFX_Speech_End=pos+((voice->length-1)<<4)+9;
#endif

	CdControlF(CdlSetfilter,(UBYTE*)&MFX_filter);
	CdControlF(CdlReadS,(char*)&sector);
	MFX_Seek_delay=10;

	music_current_level=127;
	MFX_Conv_playing=1;
	return 1;
#endif
}					   
#if 0
SLONG	MFX_QUICK_still_playing(void)
{
	return();
}

void	MFX_QUICK_stop(void)
{
}
#endif