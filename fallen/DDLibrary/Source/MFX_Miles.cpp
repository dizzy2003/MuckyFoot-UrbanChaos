

#if 0

// MFX_Miles.cpp
//
// Miles Sound System

// Making it asynchronous:
//
// 1. Keep everything the same.  vptr->smp points to sample, sptr->dptr points to data, only "pending" is marked for sample
// 2. When IO completes, go through voices and trigger/modify according to data in vptr

#define	ASYNC_FILE_IO	1
#define TALK_3D			0

#include "snd_type.h"
#include	"c:\fallen\headers\demo.h"

#include "MFX.h"
#include "MFX_Miles.h"

#include "c:\fallen\headers\fc.h"
#include "c:\fallen\headers\env.h"
#include "c:\fallen\headers\xlat_str.h"
#include "c:\fallen\headers\demo.h"
#include "c:\fallen\ddengine\headers\poly.h"
#include "resource.h"
#ifndef TARGET_DC
#include <cmath>
#endif

#include "drive.h"

#if ASYNC_FILE_IO
#include "asyncfile2.h"
#endif

#ifdef DODGYPSXIFY
BOOL dodgy_psx_mode=0;
#endif





#ifdef NO_SOUND
//
// GET NO_SOUND COMPILING!
//

SLONG MFX_QUICK_play(char *c, SLONG a, SLONG b, SLONG d)
{
	return 0;
}

void MilesTerm(void)
{
	return;
}


#endif




#ifdef M_SOUND

#define		COORDINATE_UNITS	float(1.0 / 256.0)	// 1 tile ~= 1 meter

HDIGDRIVER		drv = NULL;

static MFX_Voice	Voices[MAX_VOICE];
static MFX_QWave	QWaves[MAX_QWAVE];
static MFX_QWave*	QFree;		// first free queue elt. (NEVER NULL - we waste one element)
static MFX_QWave*	QFreeLast;	// last free queue elt.

static MFX_Sample	Samples[MAX_SAMPLE];
static MFX_Sample	TalkSample;		// for talk
static int			NumSamples;
static MFX_Sample	LRU;			// sentinel for LRU dllist

static MFX_3D		Providers[MAX_PROVIDER];
static MFX_3D		FallbackProvider;
static int			CurProvider;
static int			IsEAX;
static int			NumProviders;
static HPROVIDER	Provider = NULL;
static H3DPOBJECT	Listener = NULL;
static H3DPOBJECT	FallbackListener = NULL;
static UBYTE		DoMilesDialog = 1;
static const float	MinDist = 512;			// min distance for 3D provider
static const float	MaxDist = (64 << 8);	// max distance for 3D provider
static float		Gain2D = 1.0;			// gain for 2D voices
static float		LX,LY,LZ;				// listener coords

static float		Volumes[3];				// volumes for each SampleType

static int			Num3DVoices = 0;
static int			Max3DVoices = 0;
static int			AllocatedRAM = 0;

static char*		PathName(char *fname);
static void			DumpInfo();
static void			InitVoices();
static void			LoadWaveFile(MFX_Sample* sptr);
static void			LoadTalkFile(char* filename);
#if ASYNC_FILE_IO
static void			LoadWaveFileAsync(MFX_Sample* sptr);
#endif
static void			LoadWaveFileFinished(MFX_Sample* sptr);
static void			UnloadWaveFile(MFX_Sample* sptr);
static void			UnloadTalkFile();
static void			MilesDialog(HINSTANCE hInst, HWND hWnd);
static void			FinishLoading(MFX_Voice* vptr);

static void			PlayVoice(MFX_Voice* vptr);
static void			MoveVoice(MFX_Voice* vptr);
static void			SetVoiceRate(MFX_Voice* vptr, float mult);
static void			SetVoiceGain(MFX_Voice* vptr, float gain);

extern CBYTE*		sound_list[];


//----------------------------------------------------------------------------
// DODGY PSXIFIER (sorry eddie :} )
//
// Yeah, sorry is right - why does this mess have to be in my lovely file?
// Check out File/New on the VC menu <g>
//

#ifdef DODGYPSXIFY

BOOL dodgy_psx_info_loaded=0;
BOOL dodgy_psx_info[1024];

void		LoadDodgyPSXInfo() {
	SLONG i=0;
	dodgy_psx_info_loaded=1;
	dodgy_psx_mode=ENV_get_value_number("dodgy_psx_sound",0,"Audio");
	if (!dodgy_psx_mode) return;
	ZeroMemory(dodgy_psx_info,sizeof(dodgy_psx_info));
	while (strcmp(sound_list[i],"!")) {
		dodgy_psx_info[i]=GetPrivateProfileInt("PSXSound",sound_list[i],0,"c:\\fallen\\psxsound.ini");
		i++;
	}
}

inline BOOL AvailableOnPSX(SLONG i) {
	if (!dodgy_psx_info_loaded) LoadDodgyPSXInfo();
	if (!dodgy_psx_mode) return 1;
	return dodgy_psx_info[i];
}

#define PSXCheck(i) { if (!AvailableOnPSX(i)) return; }
#define PSXCheck2(i) { if (!AvailableOnPSX(i)) return 0; }

#else

#define PSXCheck(i) ;
#define PSXCheck2(i) ;

#endif

//----------------------------------------------------------------------------


// MilesInit
//
// init the Miles crap

void MilesInit(HINSTANCE hInst, HWND hWnd)
{
	if (drv)	return;

	AIL_set_preference(DIG_MIXER_CHANNELS, MAX_VOICE);
	AIL_set_preference(DIG_DEFAULT_VOLUME, 127);
	if (!AIL_quick_startup(1,0,44100,16,2))	return;
	AIL_quick_handles(&drv, NULL, NULL);

#if ASYNC_FILE_IO
	InitAsyncFile();
#endif

	InitVoices();

	// initialize Samples[]
	LRU.prev_lru = LRU.next_lru = &LRU;
	AllocatedRAM = 0;

	NumSamples = 0;
	CBYTE		buf[_MAX_PATH];
	CBYTE**		names = sound_list;
	MFX_Sample*	sptr = Samples;

	while (strcmp(names[0], "!"))
	{
		sptr->prev_lru = NULL;
		sptr->next_lru = NULL;
		sptr->fname = NULL;
		sptr->dptr = NULL;
		sptr->is3D = true;
		sptr->rate = 0;
		sptr->stream = false;
		sptr->size = 0;
		sptr->type = SMP_Effect;
		sptr->linscale = 1.0;
		sptr->loading = false;

		if (stricmp("null.wav", names[0]))
		{
			FILE*	fd = MF_Fopen(PathName(names[0]), "rb");
			if (fd)
			{
				sptr->fname = names[0];
				sptr->dptr = NULL;
				if (((!strnicmp(names[0], "music",5))||(!strnicmp(names[0], "generalmusic",12)))&&(!strstr(names[0],"Club1"))&&(!strstr(names[0],"Acid")))
				{
					sptr->is3D = false;
					sptr->type = SMP_Music;
				}
				sptr->rate = 0;
				sptr->stream = false;

				fseek(fd, 0, SEEK_END);
				sptr->size = ftell(fd);
				MF_Fclose(fd);

				if (sptr->size > MIN_STREAM_SIZE)
				{
					sptr->stream = true;
					sptr->is3D = false;
				}

//				TRACE("FOUND: %s (%s)\n", sptr->fname, PathName(sptr->fname));
			}
			else
			{
				TRACE("NOT FOUND: %s (%s)\n", names[0], PathName(names[0]));
			}
		}

		NumSamples++;

		if (sptr->fname)
		{
			int gain = GetPrivateProfileInt("PowerLevels", sptr->fname, 0, "data\\sfx\\powerlvl.ini");
			if (gain)
			{
				gain *= 4;
				SetPower(sptr-Samples, float(gain));
				TRACE("Setting %s gain to %d dB\n",sptr->fname,gain);
			}
		}

		sptr++;
		names++;
	}

	ASSERT(NumSamples <= MAX_SAMPLE);

	sptr = &TalkSample;

	sptr->prev_lru = NULL;
	sptr->next_lru = NULL;
	sptr->fname = NULL;
	sptr->dptr = NULL;
	sptr->is3D = TALK_3D ? true : false;
	sptr->rate = 0;
	sptr->stream = false;
	sptr->size = 0;
	sptr->type = SMP_Effect;
	sptr->linscale = 1.0;
	sptr->loading = false;

	// get 3D providers list
	HPROENUM	next = HPROENUM_FIRST;
	MFX_3D*		prov = &Providers[0];
	int			best = -1;
	int			sbest = 0;

	FallbackProvider.hnd = NULL;

	while ((NumProviders < MAX_PROVIDER) && AIL_enumerate_3D_providers(&next, &prov->hnd, &prov->name))
	{
		// try and open this
		M3DRESULT	res = AIL_open_3D_provider(prov->hnd);
		if (res == M3D_NOERR)
		{
			TRACE("Found provider: %s\n", prov->name);
			AIL_close_3D_provider(prov->hnd);

			int	score;
			if (!strcmp(prov->name, "Aureal A3D Interactive(TM)"))									score = 30;
			else if (!strcmp(prov->name, "Microsoft DirectSound3D with Creative Labs EAX(TM)"))		score = 100;
			else if (!strcmp(prov->name, "Microsoft DirectSound3D hardware support"))				score = 50;
			else if (!strcmp(prov->name, "Microsoft DirectSound3D software emulation"))				score = 20;
			else if (!strcmp(prov->name, "RSX 3D Audio from RAD Game Tools"))						score = 10;
			else																					score = 5;

			if (score > sbest)
			{
				best = NumProviders;
				sbest = score;
			}

			if (!strcmp(prov->name, "Miles Fast 2D Positional Audio"))
			{
				// store fallback 2D provider
				FallbackProvider = *prov;
			}

			prov++;
			NumProviders++;
		}
		else
		{
			TRACE("Can't use provider: %s\n", prov->name);
		}
	}

	CurProvider = best;

	// create fallback 3D provider
	if (FallbackProvider.hnd)
	{
		if (AIL_open_3D_provider(FallbackProvider.hnd) == M3D_NOERR)
		{
			TRACE("Opened fallback provider %s\n", FallbackProvider.name);
			FallbackListener = AIL_open_3D_listener(FallbackProvider.hnd);
		}
		else
		{
			TRACE("Couldn't open fallback provider\n");
			FallbackProvider.hnd = NULL;
		}
	}
	else
	{
		TRACE("No fallback provider\n");
	}

	// read config file
	DoMilesDialog = ENV_get_value_number("run_sound_dialog", 1, "Audio");
	char*	str = ENV_get_value_string("3D_sound_driver", "Audio");

	if (str)
	{
		for (int ii = 0; ii < NumProviders; ii++)
		{
			if (!stricmp(Providers[ii].name, str))	CurProvider = ii;
		}
	}

	Volumes[SMP_Ambient]	= float(ENV_get_value_number("ambient_volume", 127, "Audio")) / 127;
	Volumes[SMP_Music]		= float(ENV_get_value_number("music_volume", 127, "Audio")) / 127;
	Volumes[SMP_Effect]		= float(ENV_get_value_number("fx_volume", 127, "Audio")) / 127;

	/*

	//
	// WAIT FOR THE GRAPHICS DIALOG BOX TO FINISH OFF INITIALISING US!
	// 
	//  It calls init_my_dialog() and my_dialog_over()...
	//
	//

	// do dialog
	if (DoMilesDialog)
	{
		MilesDialog(hInst, hWnd);
		ENV_set_value_number("run_sound_dialog", DoMilesDialog, "Audio");
	}

	// set 3D provider
	Set3DProvider(CurProvider);

	*/
}

// MilesTerm
//
// term the Miles crap

void MilesTerm()
{
	if (!drv)	return;

	DumpInfo();

	MFX_free_wave_list();

	// free waves
	for (int ii = 0; ii < NumSamples; ii++)
	{
		UnloadWaveFile(&Samples[ii]);
	}
	NumSamples = 0;

	UnloadTalkFile();

	// free providers & listener
	Set3DProvider(-1);
	if (FallbackListener)
	{
		AIL_close_3D_listener(FallbackListener);
		FallbackListener = NULL;
	}
	if (FallbackProvider.hnd)
	{
		AIL_close_3D_provider(FallbackProvider.hnd);
		FallbackProvider.hnd = NULL;
	}

	AIL_quick_shutdown();
	drv = NULL;

#if ASYNC_FILE_IO
	TermAsyncFile();
#endif
}

// GetMilesDriver
//
// BinkClient calls this to find the Miles driver for BINK

HDIGDRIVER GetMilesDriver()
{
	return drv;
}

// PathName
//
// get a sample's full pathname

static char* PathName(char* fname)
{
	CBYTE	buf[MAX_PATH];
	static CBYTE	pathname[MAX_PATH];

	if (strchr(fname,'-'))  // usefully, all the taunts etc have a - in them, and none of the other sounds do... bonus!
	{
		CHAR *ptr = strrchr(fname,'\\')+1;
		sprintf(buf,"talk2\\misc\\%s",ptr);
		strcpy(pathname, GetSFXPath());
		strcat(pathname, buf);
		return pathname;
	}
	else
	{
		sprintf(buf, "data\\sfx\\1622\\%s", fname);
		if (!strnicmp(fname, "music", 5))
		{
			if (!MUSIC_WORLD)	MUSIC_WORLD = 1;
#ifdef VERSION_DEMO
			MUSIC_WORLD = 1;
#endif
			buf[19] = '0' + (MUSIC_WORLD / 10);
			buf[20] = '0' + (MUSIC_WORLD % 10);
		}
	}

	strcpy(pathname, GetSFXPath());
	strcat(pathname, buf);

	return pathname;
}

// SetLinScale
//
// set a sample's linear scale

void SetLinScale(SLONG wave, float linscale)
{
	if ((wave >= 0) && (wave < NumSamples))
	{
		Samples[wave].linscale = linscale;
	}
}

// SetPower
//
// set a sample's power in dB

void SetPower(SLONG wave, float dB)
{
	SetLinScale(wave, float(exp(log(10) * dB / 20)));
}

// DumpInfo
//
// dump info about loaded sounds

static void DumpInfo()
{
#ifdef _DEBUG
	FILE*	fd = MF_Fopen("c:\\soundinfo.txt", "w");
	if (fd)
	{
		int	loaded = 0;
		for (int ii = 0; ii < NumSamples; ii++)
		{
			fprintf(fd, "%d : %s : %d K : %s\n", ii, Samples[ii].fname, Samples[ii].size >> 10, Samples[ii].dptr ? "LOADED" : "unloaded");
			if (Samples[ii].dptr)	loaded++;
		}
		fprintf(fd, "-------\nTotal %d samples, (%d loaded = %d K)\n\n", NumSamples, loaded, AllocatedRAM >> 10);
		fprintf(fd, "LRU queue:\n\n");
		for (MFX_Sample* sptr = LRU.next_lru; sptr != &LRU; sptr = sptr->next_lru)
		{
			fprintf(fd, "%s (%d)\n", sptr->fname, sptr->usecount);
		}
		MF_Fclose(fd);
	}
#endif
}

// Get3DProviderList
//
// get the list of providers

int Get3DProviderList(MFX_3D** prov)
{
	*prov = &Providers[0];
	return NumProviders;
}

// Get3DProvider
//
// get the 3D provider

int Get3DProvider()
{
	return CurProvider;
}

// Set3DProvider
//
// set a provider

void Set3DProvider(int ix)
{
	if (!drv)	return;

	if (Listener)
	{
		AIL_close_3D_listener(Listener);
	}
	Listener = NULL;

	if (Provider)
	{
		MFX_stop(MFX_CHANNEL_ALL, MFX_WAVE_ALL);
		AIL_close_3D_provider(Provider);
	}
	Provider = NULL;

	if ((ix >= 0) && (ix < NumProviders))
	{
		CurProvider = ix;

		if (Providers[ix].hnd == FallbackProvider.hnd)
		{
			// only use fallback provider
			IsEAX = false;
		}
		else if (AIL_open_3D_provider(Providers[ix].hnd) == M3D_NOERR)
		{
			Provider = Providers[ix].hnd;
			TRACE("Selected 3D provider: %s\n", Providers[ix].name);
			Listener = AIL_open_3D_listener(Provider);

			IsEAX = !strcmp(Providers[ix].name, "Microsoft DirectSound3D with Creative Labs EAX(TM)");
		}

		if (IsEAX)
		{
			float	level = 0.0;
			AIL_set_3D_provider_preference(Provider, "EAX effect volume", &level);
			Gain2D = 0.7f;
		}
		else
		{
			Gain2D = 1.0;
		}

		// write to config
		ENV_set_value_string("3D_sound_driver", Providers[ix].name, "Audio");
	}
}

// InitVoices
//
// initialize the voices and queue

static void InitVoices()
{
	int	ii;

	for (ii = 0; ii < MAX_VOICE; ii++)
	{
		Voices[ii].id = 0;
		Voices[ii].wave = 0;
		Voices[ii].flags = 0;
		Voices[ii].x = 0;
		Voices[ii].y = 0;
		Voices[ii].z = 0;
		Voices[ii].thing = NULL;
		Voices[ii].queue = NULL;
		Voices[ii].queuesz = 0;
		Voices[ii].smp = NULL;
		Voices[ii].h2D = NULL;
		Voices[ii].h3D = NULL;
	}

	for (ii = 0; ii < MAX_QWAVE; ii++)
	{
		QWaves[ii].next = &QWaves[ii+1];
	}
	QWaves[ii-1].next = NULL;
	QFree = &QWaves[0];
	QFreeLast = &QWaves[ii-1];

	LX = LY = LZ = 0;
}

// Hash
//
// hash a channel ID to a voice ID

static inline int Hash(UWORD channel_id)
{
	return (channel_id * 37) & VOICE_MSK;
}

// FindVoice
//
// find an active voice with the given channel ID and wave #

static MFX_Voice* FindVoice(UWORD channel_id, ULONG wave)
{
	int	offset = Hash(channel_id);

	for (int ii = 0; ii < MAX_VOICE; ii++)
	{
		int	vn = (ii + offset) & VOICE_MSK;
		if ((Voices[vn].id == channel_id) && (Voices[vn].wave == wave))	return &Voices[vn];
	}

	return NULL;
}

// FindFirst
//
// find the first active voice with the given channel ID

static MFX_Voice* FindFirst(UWORD channel_id)
{
	int	offset = Hash(channel_id);

	for (int ii = 0; ii < MAX_VOICE; ii++)
	{
		int	vn = (ii + offset) & VOICE_MSK;
		if (Voices[vn].id == channel_id)	return &Voices[vn];
	}

	return NULL;
}

// FindNext
//
// find the next active voice with the same channel ID

static MFX_Voice* FindNext(MFX_Voice* vptr)
{
	int	offset = Hash(vptr->id);

	int	ii = ((vptr - Voices) - offset) & VOICE_MSK;

	for (ii++; ii < MAX_VOICE; ii++)
	{
		int	vn = (ii + offset) & VOICE_MSK;
		if (Voices[vn].id == vptr->id)		return &Voices[vn];
	}

	return NULL;
}

// FindFree
//
// find a free voice

static MFX_Voice* FindFree(UWORD channel_id)
{
	int	offset = Hash(channel_id);

	for (int ii = 0; ii < MAX_VOICE; ii++)
	{
		int	vn = (ii + offset) & VOICE_MSK;
		if (!Voices[vn].smp)		return &Voices[vn];
	}

	return NULL;
}

// FreeVoiceSource
//
// remove a voice's source

static void FreeVoiceSource(MFX_Voice* vptr)
{
	if (vptr->h2D)
	{
		vptr->smp->usecount--;
		AIL_release_sample_handle(vptr->h2D);
		vptr->h2D = NULL;
	}
	if (vptr->h3D)
	{
		vptr->smp->usecount--;
		AIL_release_3D_sample_handle(vptr->h3D);
		vptr->h3D = NULL;
		Num3DVoices--;
	}
	if (vptr->hStream)
	{
		AIL_close_stream(vptr->hStream);
		vptr->hStream = NULL;
	}
}

// FreeVoice
//
// free a voice up

static void FreeVoice(MFX_Voice* vptr)
{
	if (!vptr)	return;

	// remove queue
	QFreeLast->next = vptr->queue;
	while (QFreeLast->next)	QFreeLast = QFreeLast->next;
	vptr->queue = NULL;
	vptr->queuesz = 0;

	// reset data
	FreeVoiceSource(vptr);

	if (vptr->thing)
	{
		vptr->thing->Flags &= ~FLAGS_HAS_ATTACHED_SOUND;
	}
	vptr->thing = NULL;
	vptr->flags = 0;
	vptr->id = 0;
	vptr->wave = 0;
	vptr->smp = NULL;
}

// GetVoiceForWave
//
// find a voice slot for the wave

static MFX_Voice* GetVoiceForWave(UWORD channel_id, ULONG wave, ULONG flags)
{
	// just return a new voice if overlapped
	if (flags & MFX_OVERLAP)	return FindFree(channel_id);

	MFX_Voice*	vptr;

	if (flags & (MFX_QUEUED | MFX_NEVER_OVERLAP))
	{
		// find first voice on this channel, if any
		vptr = FindFirst(channel_id);
	}
	else
	{
		// find voice playing this sample, if any
		vptr = FindVoice(channel_id, wave);
	}

	if (!vptr)
	{
		vptr = FindFree(channel_id);
	}
	else
	{
		// found a voice - return NULL if not queued but never overlapped (else queue)
		if ((flags & (MFX_NEVER_OVERLAP | MFX_QUEUED)) == MFX_NEVER_OVERLAP)	return NULL;
	}

	return vptr;
}

// SetupVoiceTalk
//
// setup a talking voice

static SLONG SetupVoiceTalk(MFX_Voice* vptr, char* filename)
{
	vptr->id = 0;
	vptr->wave = NumSamples;
	vptr->flags = 0;
	vptr->thing = NULL;
	vptr->queue = NULL;
	vptr->queuesz = 0;
	vptr->smp = NULL;
	vptr->queuesz = 0;
	vptr->smp = NULL;
	vptr->playing = false;
	vptr->ratemult = 1.0;
	vptr->gain = 1.0;

	if (!Volumes[SMP_Effect])	return FALSE;

	LoadTalkFile(filename);
	if (!TalkSample.dptr)	return FALSE;

	vptr->smp = &TalkSample;
	FinishLoading(vptr);

	return TRUE;
}

// SetupVoice
//
// setup a voice for playback

static void SetupVoice(MFX_Voice* vptr, UWORD channel_id, ULONG wave, ULONG flags)
{
	vptr->id = channel_id;
	vptr->wave = wave;
	vptr->flags = flags;
	vptr->thing = NULL;
	vptr->queue = NULL;
	vptr->queuesz = 0;
	vptr->smp = NULL;
	vptr->playing = false;
	vptr->ratemult = 1.0;
	vptr->gain = 1.0;

	if (wave >= NumSamples)	return;

	MFX_Sample*	sptr = &Samples[wave];

	if ((sptr->type!=SMP_Music)&&(GAME_STATE & (GS_LEVEL_LOST|GS_LEVEL_WON))) return ; // once the level's won or lost, no more sounds


	float	level = Volumes[sptr->type];

	if (!level)	return;

	if (sptr->stream)
	{
		if (!sptr->fname)	return;
		vptr->hStream = AIL_open_stream(drv, PathName(sptr->fname), 0);
		AIL_set_stream_volume(vptr->hStream, S32(Gain2D * level * 127));
		TRACE("Opened stream %s\n", sptr->fname);
		vptr->smp = sptr;
	}
	else
	{
		// load the sample
		if (!sptr->dptr)
		{
#if ASYNC_FILE_IO
			LoadWaveFileAsync(sptr);
#else
			LoadWaveFile(sptr);
#endif
			if (!sptr->dptr)	return;
		}

		// unlink from LRU queue
		if (sptr->prev_lru)
		{
			sptr->prev_lru->next_lru = sptr->next_lru;
			sptr->next_lru->prev_lru = sptr->prev_lru;
		}

		// free some stuff if we've got too many loaded
		if (AllocatedRAM > MAX_SAMPLE_MEM)
		{
			MFX_Sample*	sptr = LRU.next_lru;
			while (sptr != &LRU)
			{
				MFX_Sample*	next = sptr->next_lru;
				if (!sptr->usecount)
				{
					UnloadWaveFile(sptr);
					if (AllocatedRAM <= MAX_SAMPLE_MEM)	break;
				}
				sptr = next;
			}
		}

		// link in at front
		sptr->next_lru = &LRU;
		sptr->prev_lru = LRU.prev_lru;
		sptr->next_lru->prev_lru = sptr;
		sptr->prev_lru->next_lru = sptr;

		vptr->smp = sptr;

		if (!sptr->loading)
		{
			FinishLoading(vptr);
		}
	}
}

// FinishLoading
//
// set up voice after sample has loaded

static void FinishLoading(MFX_Voice* vptr)
{
	MFX_Sample*	sptr = vptr->smp;

	if (sptr->is3D)
	{
		if (Provider)
		{
			vptr->h3D = AIL_allocate_3D_sample_handle(Provider);
		}

		if (!vptr->h3D && FallbackProvider.hnd)
		{
			vptr->h3D = AIL_allocate_3D_sample_handle(FallbackProvider.hnd);
		}

		if (vptr->h3D)
		{
			sptr->usecount++;
			AIL_set_3D_sample_file(vptr->h3D, sptr->dptr);
			AIL_set_3D_sample_distances(vptr->h3D,	MaxDist * COORDINATE_UNITS * sptr->linscale, MinDist * COORDINATE_UNITS * sptr->linscale, 
													MaxDist * COORDINATE_UNITS * sptr->linscale, MinDist * COORDINATE_UNITS * sptr->linscale);
			AIL_set_3D_sample_volume(vptr->h3D, S32(Volumes[sptr->type] * 127));
			if (IsEAX)
			{
				float	level = 0.0;
				AIL_set_3D_sample_preference(vptr->h3D, "EAX sample reverb mix", &level);
			}
			Num3DVoices++;
			if (Num3DVoices > Max3DVoices)	Max3DVoices = Num3DVoices;
//			TRACE("Setup 3D voice %d for %s - %d in use, max %d\n", vptr - Voices, sptr->fname, Num3DVoices, Max3DVoices);
		}
	}

	if (!vptr->h3D)
	{
		// get 2D handle
		vptr->h2D = AIL_allocate_file_sample(drv, sptr->dptr, 0);
//		TRACE("Setup 2D voice %d for %s\n", vptr - Voices, sptr->fname);
		if (vptr->h2D)
		{
			sptr->usecount++;
			AIL_set_sample_volume(vptr->h2D, S32(Gain2D * Volumes[sptr->type] * 127));
		}
	}

	MoveVoice(vptr);
	if (vptr->ratemult != 1.0)		SetVoiceRate(vptr, vptr->ratemult);
	if (vptr->gain != 1.0)			SetVoiceGain(vptr, vptr->gain);
	if (vptr->playing)				PlayVoice(vptr);
}

// PlayVoice
//
// play the voice

static void PlayVoice(MFX_Voice* vptr)
{
	if (vptr->h2D)
	{
		if (vptr->flags & MFX_LOOPED)
		{
			AIL_set_sample_loop_count(vptr->h2D, 0);
		}
		AIL_start_sample(vptr->h2D);
	}
	if (vptr->h3D)
	{
		if (vptr->flags & MFX_LOOPED)
		{
			AIL_set_3D_sample_loop_count(vptr->h3D, 0);
		}
		AIL_start_3D_sample(vptr->h3D);
	}
	if (vptr->hStream)
	{
		if (vptr->flags & MFX_LOOPED)
		{
			AIL_set_stream_loop_count(vptr->hStream, 0);
		}
		AIL_start_stream(vptr->hStream);
	}
	vptr->playing = true;
}

// MoveVoice
//
// set position of voice source from voice x,y,z

static void MoveVoice(MFX_Voice* vptr)
{
	if (vptr->h3D)
	{
		float	x = vptr->x * COORDINATE_UNITS;
		float	y = vptr->y * COORDINATE_UNITS;
		float	z = vptr->z * COORDINATE_UNITS;
/*
#ifndef	FINAL
void	AENG_draw_rectr(SLONG x,SLONG y,SLONG w,SLONG h,SLONG col,SLONG layer,SLONG page);
		AENG_draw_rectr(vptr->x>>6,vptr->z>>7,2,2,0xffff,1,POLY_PAGE_COLOUR);
#endif				   
*/



		if ((fabs(x - LX) < 0.5) && (fabs(y - LY) < 0.5) && (fabs(z - LZ) < 0.5))
		{
			// set exactly at the listener if within epsilon
			AIL_set_3D_position(vptr->h3D, LX, LY, LZ);
		}
		else
		{
			AIL_set_3D_position(vptr->h3D, x, y, z);
		}
	}
}

// SetVoiceRate
//
// set rate for voice

static void SetVoiceRate(MFX_Voice* vptr, float mult)
{
	if (vptr->h2D)
	{
		AIL_set_sample_playback_rate(vptr->h2D, S32(mult * vptr->smp->rate));
	}
	if (vptr->h3D)
	{
		AIL_set_3D_sample_playback_rate(vptr->h3D, S32(mult * vptr->smp->rate));
	}
	if (vptr->hStream)
	{
		AIL_set_stream_playback_rate(vptr->hStream, S32(mult * vptr->smp->rate));
	}
	vptr->ratemult = mult;
}

// SetVoiceGain
//
// set gain for voice

static void SetVoiceGain(MFX_Voice* vptr, float gain)
{
	if (vptr->smp == NULL)
	{
		return;
	}

	gain *= Volumes[vptr->smp->type];

	if (vptr->h2D)
	{
		AIL_set_sample_volume(vptr->h2D, S32(Gain2D * gain * 127));
	}
	if (vptr->h3D)
	{
		AIL_set_3D_sample_volume(vptr->h3D, S32(gain * 127));
	}
	if (vptr->hStream)
	{
		AIL_set_stream_volume(vptr->hStream, S32(Gain2D * gain * 127));
	}
	if (vptr->queue)
		vptr->queue->gain=gain;
	vptr->gain = gain;
}

// IsVoiceDone
//
// check if voice is done

static bool IsVoiceDone(MFX_Voice* vptr)
{
	if (vptr->flags & MFX_LOOPED)	return false;

	U32		status;
	U32		posn;

	if (vptr->h2D)
	{
		status = AIL_sample_status(vptr->h2D);
		posn = AIL_sample_position(vptr->h2D);
	}
	else if (vptr->h3D)
	{
		status = AIL_3D_sample_status(vptr->h3D);
		posn = AIL_3D_sample_offset(vptr->h3D);
	}
	else if (vptr->hStream)
	{
		status = AIL_stream_status(vptr->hStream);
		posn = AIL_stream_position(vptr->hStream);
	}
	else if (vptr->smp && vptr->smp->loading)
	{
		return false;
	}
	else
	{
		return true;
	}

	if (vptr->flags & MFX_EARLY_OUT)
	{
		return (vptr->smp->size - (signed)posn < 440*2);
	}

	return (status != SMP_PLAYING);
}

// QueueWave
//
// queue a wave

static void QueueWave(MFX_Voice* vptr, UWORD wave, ULONG flags, SLONG x, SLONG y, SLONG z)
{
	if ((flags & MFX_SHORT_QUEUE) && vptr->queue)
	{
		// short queue - just blat it over the queued one
		vptr->queue->flags = flags;
		vptr->queue->wave = wave;
		vptr->queue->x = x;
		vptr->queue->y = y;
		vptr->queue->z = z;
		return;
	}

	if (vptr->queuesz > MAX_QVOICE)		return;		// too many queued voices
	if (QFree == QFreeLast)				return;		// no free slots

	// allocate a queue element
	MFX_QWave*	qptr = vptr->queue;

	if (qptr)
	{
		while (qptr->next)	qptr = qptr->next;
		qptr->next = QFree;
		QFree = QFree->next;
		qptr = qptr->next;
	}
	else
	{
		vptr->queue = QFree;
		QFree = QFree->next;
		qptr = vptr->queue;
	}

	qptr->next = NULL;
	qptr->flags = flags;
	qptr->wave = wave;
	qptr->x = x;
	qptr->y = y;
	qptr->z = z;
	qptr->gain = 1.0;
}

// TriggerPairedVoice
//
// trigger a paired voice

static void TriggerPairedVoice(UWORD channel_id)
{
	MFX_Voice*	vptr = FindFirst(channel_id);

	if (!vptr || !vptr->smp)	return;

	vptr->flags &= ~MFX_PAIRED_TRK2;
	PlayVoice(vptr);
}

// PlayWave
//
// play a sound

static UBYTE PlayWave(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z, Thing* thing)
{
	MFX_Voice*	vptr = GetVoiceForWave(channel_id, wave, flags);

	if (!vptr)	return 0;

	if (thing)
	{
		vptr->x = (thing->WorldPos.X >> 8);
		vptr->y = (thing->WorldPos.Y >> 8);
		vptr->z = (thing->WorldPos.Z >> 8);
	}
	else
	{
		vptr->x = x;
		vptr->y = y;
		vptr->z = z;
	}

	if (vptr->smp)
	{
		if ((vptr->smp->type!=SMP_Music)&&(GAME_STATE & (GS_LEVEL_LOST|GS_LEVEL_WON))) return 0; // once the level's won or lost, no more sounds
		if (flags & MFX_QUEUED)
		{
			QueueWave(vptr, wave, flags, vptr->x, vptr->y, vptr->z);
			return 2;
		}
		if ((vptr->wave == wave) && !(flags & MFX_REPLACE))
		{
			MoveVoice(vptr);
			return 0;
		}
		FreeVoice(vptr);
	}

	SetupVoice(vptr, channel_id, wave, flags);
	MoveVoice(vptr);
	if (!(flags & MFX_PAIRED_TRK2))
	{
		PlayVoice(vptr);
	}
	if (thing)
	{
		vptr->thing = thing;
		thing->Flags |= FLAGS_HAS_ATTACHED_SOUND;
	}
	if (flags & MFX_PAIRED_TRK1)
	{
		TriggerPairedVoice(channel_id + 1);
	}

	return 1;
}

// PlayTalk
//
// play a speech

static UBYTE PlayTalk(char* filename, SLONG x, SLONG y, SLONG z)
{
	MFX_Voice*	vptr = GetVoiceForWave(0, NumSamples, 0);
	if (!vptr)	return 0;

	if (vptr->smp)
	{
		FreeVoice(vptr);
	}

	if (x | y | z)		TalkSample.is3D = TALK_3D ? true : false;
	else				TalkSample.is3D = false;

	vptr->x = x;
	vptr->y = y;
	vptr->z = z;

	if (!SetupVoiceTalk(vptr, filename))
	{
		return FALSE;
	}

	MoveVoice(vptr);
	PlayVoice(vptr);

	return 1;
}

//----- volume functions

// MFX_get_volumes
//
// get the current volumes, all 0 to 127

void MFX_get_volumes(SLONG* fx, SLONG* amb, SLONG* mus)
{
	*fx = SLONG(127 * Volumes[SMP_Effect]);
	*amb = SLONG(127 * Volumes[SMP_Ambient]);
	*mus = SLONG(127 * Volumes[SMP_Music]);
}

// MFX_set_volumes
//
// set the current volumes, all 0 to 127

void MFX_set_volumes(SLONG fx, SLONG amb, SLONG mus)
{
	if (fx < 0)				fx = 0;
	else if (fx > 127)		fx = 127;
	if (amb < 0)			amb = 0;
	else if (amb > 127)		amb = 127;
	if (mus < 0)			mus = 0;
	else if (mus > 127)		mus = 127;

	Volumes[SMP_Effect]		= float(fx) / 127;
	Volumes[SMP_Ambient]	= float(amb) / 127;
	Volumes[SMP_Music]		= float(mus) / 127;

	ENV_set_value_number("ambient_volume", amb, "Audio");
	ENV_set_value_number("music_volume", mus, "Audio");
	ENV_set_value_number("fx_volume", fx, "Audio");
}

//----- transport functions -----

// MFX_play_xyz
//
// play a sound at the given location

void MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z)
{
	if (!drv)	return;
	PSXCheck(wave);
	PlayWave(channel_id, wave, flags, x >> 8, y >> 8, z >> 8, NULL);
}

// MFX_play_thing
//
// play a sound from a thing

void MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p)
{
	if (!drv)	return;
	PSXCheck(wave);
	PlayWave(channel_id, wave, flags, 0,0,0, p);
}

// MFX_play_ambient
//
// play an ambient sound

void MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags)
{
	if (!drv)	return;

	PSXCheck(wave);
	if (wave < NumSamples)
	{
		Samples[wave].is3D = false;			// save 3D channels for non-ambient sounds
		if (Samples[wave].type == SMP_Effect)
		{
			Samples[wave].type = SMP_Ambient;	// use this volume setting
		}
	}
	PlayWave(channel_id, wave, flags, FC_cam[0].x, FC_cam[0].y, FC_cam[0].z, NULL);
}

// MFX_play_stereo
//
// play a stereo sound

UBYTE MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags)
{
	if (!drv)	return 0;

	PSXCheck2(wave);
	return PlayWave(channel_id, wave, flags, 0, 0, 0, NULL);
}

// MFX_stop
//
// stop a sound

void MFX_stop(SLONG channel_id, ULONG wave)
{
	if (!drv)	return;

	if (channel_id == MFX_CHANNEL_ALL)
	{
		for (int ii = 0; ii < MAX_VOICE; ii++)
		{
			FreeVoice(&Voices[ii]);
		}
	}
	else
	{
		if (wave == MFX_WAVE_ALL)
		{
			MFX_Voice*	vptr = FindFirst(channel_id);
			while (vptr)
			{
				FreeVoice(vptr);
				vptr = FindNext(vptr);
			}
		}
		else
		{
			FreeVoice(FindVoice(channel_id, wave));
		}
	}
}

// MFX_stop_attached
//
// stop all sounds attached to a thing

void MFX_stop_attached(Thing *p)
{
	if (!drv)	return;

	for (int ii = 0; ii < MAX_VOICE; ii++)
	{
		if (Voices[ii].thing == p)	FreeVoice(&Voices[ii]);
	}
}

//----- audio processing functions -----

void MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend)
{
	if (!drv)	return;

	MFX_Voice*	vptr = FindVoice(channel_id, wave);
	if (!vptr || !vptr->smp)	return;

	float	pitch = float(pitchbend + 256) / 256;
	SetVoiceRate(vptr, pitch);
}

void MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain)
{
	if (!drv)	return;

	MFX_Voice*	vptr = FindVoice(channel_id, wave);
	if (!vptr || !vptr->smp)	return;

	float	fgain = float(gain) / 255;
	SetVoiceGain(vptr, fgain);
}
/*
void MFX_set_loop_count(UWORD channel_id, ULONG wave, UBYTE count)
{
	if (!drv)	return;

	MFX_Voice*	vptr = FindVoice(channel_id, wave);
	if (!vptr || !vptr->smp)	return;

	float	fgain = float(gain) / 255;
	SetVoiceGain(vptr, fgain);

	AIL_set_sample_loop_count();
}
*/
void MFX_set_queue_gain(UWORD channel_id, ULONG wave, UBYTE gain)
{
	if (!drv)	return;

	float	fgain = float(gain) / 256;

	MFX_Voice*	vptr = FindFirst(channel_id);
	while (vptr)
	{
		if (!vptr->queue && (vptr->wave == wave))
		{
			SetVoiceGain(vptr, fgain);
		}

		for (MFX_QWave* qptr = vptr->queue; qptr; qptr = qptr->next)
		{
			if (qptr->wave == wave)	qptr->gain = fgain;
		}

		vptr = FindNext(vptr);
	}
}

//----- sound library functions -----

static void LoadWaveFile(MFX_Sample* sptr)
{
	if (!sptr->fname)	return;	// no file
	if (sptr->dptr)		return;	// already loaded

	sptr->size = AIL_file_size(PathName(sptr->fname));

	sptr->dptr = AIL_file_read(PathName(sptr->fname), NULL);
	if (!sptr->dptr)	return;

	AllocatedRAM += sptr->size;

	LoadWaveFileFinished(sptr);
}

static void LoadTalkFile(char* filename)
{
	if (TalkSample.dptr)
	{
		AIL_mem_free_lock(TalkSample.dptr);
		AllocatedRAM -= TalkSample.size;
	}

	TalkSample.dptr = AIL_file_read(filename, NULL);
	if (!TalkSample.dptr)	return;

	TalkSample.size = AIL_file_size(filename);
	AllocatedRAM += TalkSample.size;

	LoadWaveFileFinished(&TalkSample);
}

#if ASYNC_FILE_IO

static void LoadWaveFileAsync(MFX_Sample* sptr)
{
	if (!sptr->fname)	return;	// no file
	if (sptr->dptr)		return;	// loaded

	sptr->size = AIL_file_size(PathName(sptr->fname));

	sptr->dptr = AIL_mem_alloc_lock(sptr->size);
	if (!sptr->dptr)	return;

	AllocatedRAM += sptr->size;

	if (!LoadAsyncFile(PathName(sptr->fname), sptr->dptr, sptr->size, sptr))
	{
		AIL_mem_free_lock(sptr->dptr);
		AllocatedRAM -= sptr->size;
		sptr->dptr = NULL;

		// fall back to synchronous loading
		LoadWaveFile(sptr);
		return;
	}

	sptr->loading = true;
}

#endif

static void LoadWaveFileFinished(MFX_Sample* sptr)
{
	AILSOUNDINFO	info;

	sptr->loading = false;

	if (!AIL_WAV_info(sptr->dptr, &info))
	{
		TRACE("sample = %s - INVALID\n", sptr->fname);
		AIL_mem_free_lock(sptr->dptr);
		AllocatedRAM -= sptr->size;
		sptr->dptr = NULL;
		return;
	}
	else
	{
		sptr->rate = info.rate;

		if (sptr->is3D)
		{
			if (info.channels > 1)				
			{
				sptr->is3D = false;
			}
			if (info.format != WAVE_FORMAT_PCM)
			{
				if (sptr->is3D)
				{
					// decompress
					void*	wav;
					U32		size;
					if (!AIL_decompress_ADPCM(&info, &wav, &size))
					{
						sptr->is3D = false;
					}
					else
					{
						AIL_mem_free_lock(sptr->dptr);
						AllocatedRAM -= sptr->size;
						sptr->dptr = wav;
						sptr->size = size;
						AllocatedRAM += sptr->size;
					}
				}
			}
		}
	}
}

static void UnloadWaveFile(MFX_Sample* sptr)
{
	if (!sptr->dptr)	return;
	if (sptr->usecount)	return;

	// unlink
	if (sptr->prev_lru)
	{
		sptr->prev_lru->next_lru = sptr->next_lru;
		sptr->next_lru->prev_lru = sptr->prev_lru;
		sptr->next_lru = sptr->prev_lru = NULL;
	}

	// cancel pending IO
#if ASYNC_FILE_IO
	if (sptr->loading)
	{
		CancelAsyncFile(sptr);
		sptr->loading = false;
	}
#endif

	// free
	AIL_mem_free_lock(sptr->dptr);
	sptr->dptr = NULL;

	AllocatedRAM -= sptr->size;
}

static void UnloadTalkFile()
{
	if (!TalkSample.dptr)		return;
	AIL_mem_free_lock(TalkSample.dptr);
	TalkSample.dptr = NULL;

	AllocatedRAM -= TalkSample.size;
}

void MFX_load_wave_list(CBYTE *names[])
{
	if (!drv)	return;

	MFX_free_wave_list();

	// free waves
	for (int ii = 0; ii < NumSamples; ii++)
	{
		if (Samples[ii].type==SMP_Music) UnloadWaveFile(&Samples[ii]);
	}
	UnloadWaveFile(&TalkSample);
}

void MFX_free_wave_list()
{

	// reset the music system
extern void MUSIC_reset(); // yeah yeah i know ugly
	MUSIC_reset();

	if (!drv)	return;

	MFX_stop(MFX_CHANNEL_ALL, MFX_WAVE_ALL);

	InitVoices();

}

//----- listener & environment -----

void MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch)
{
	if (!drv)	return;

	if (Listener || FallbackListener)
	{
		x >>= 8;
		y >>= 8;
		z >>= 8;

		LX = x * COORDINATE_UNITS;
		LY = y * COORDINATE_UNITS;
		LZ = z * COORDINATE_UNITS;

		heading += 0x200;
		heading &= 0x7FF;

		float	xorient = float(COS(heading)) / 65536;
		float	zorient = float(SIN(heading)) / 65536;

		if (Listener)
		{
			AIL_set_3D_position(Listener, LX, LY, LZ);
			AIL_set_3D_orientation(Listener, xorient, 0, zorient, 0, 1, 0);
		}

		if (FallbackListener)
		{
			AIL_set_3D_position(FallbackListener, LX, LY, LZ);
			AIL_set_3D_orientation(FallbackListener, xorient, 0, zorient, 0, 1, 0);
		}

		// move voices so the epsilon checks
		// get made
		for (int ii = 0; ii < MAX_VOICE; ii++)
		{
			if (Voices[ii].h3D)		MoveVoice(&Voices[ii]);
		}
	}
}

//----- general system stuff -----

// MFX_render
//
// update the parameters of the sounds

void MFX_render()
{
	if (!drv)	return;

#if ASYNC_FILE_IO
	// check for async completions
	MFX_Sample*	sptr;

	while (sptr = (MFX_Sample*)GetNextCompletedAsyncFile())
	{
		// set up sample now it's in RAM
		LoadWaveFileFinished(sptr);

		// and trigger any voices that were waiting for it
		for (int ii = 0; ii < MAX_VOICE; ii++)
		{
			if (Voices[ii].smp == sptr)
			{
				FinishLoading(&Voices[ii]);				
			}
		}
	}
#endif

	for (int ii = 0; ii < MAX_VOICE; ii++)
	{
		MFX_Voice*	vptr = &Voices[ii];

		if (vptr->flags & MFX_PAIRED_TRK2)	continue;
		if (!vptr->smp)						continue;

		if (IsVoiceDone(vptr))
		{
			if (!vptr->queue)
			{
				FreeVoice(vptr);				
			}
			else
			{
				// get next wave from queue
				MFX_QWave*	qptr = vptr->queue;
				vptr->queue = qptr->next;

				if (qptr->flags & MFX_PAIRED_TRK1)	TriggerPairedVoice(Voices[ii].id + 1);

				// free the old sample and set up the new one
				Thing*	thing = vptr->thing;
				FreeVoiceSource(vptr);
				SetupVoice(vptr, vptr->id, qptr->wave, qptr->flags & ~MFX_PAIRED_TRK2);
				vptr->thing = thing;

				// set the position
				if ((vptr->flags & MFX_MOVING) && vptr->thing)
				{
					vptr->x = vptr->thing->WorldPos.X >> 8;
					vptr->y = vptr->thing->WorldPos.Y >> 8;
					vptr->z = vptr->thing->WorldPos.Z >> 8;
				}
				else
				{
					vptr->x = qptr->x;
					vptr->y = qptr->y;
					vptr->z = qptr->z;
				}

				// relocate and play
				MoveVoice(vptr);
				PlayVoice(vptr);
				SetVoiceGain(vptr, qptr->gain);

				// release queue element
				qptr->next = QFree;
				QFree = qptr;
			}
		}
		else
		{
			if (vptr->flags & MFX_CAMERA)
			{
				vptr->x = FC_cam[0].x >> 8;
				vptr->z = FC_cam[0].z >> 8;
				if (!(vptr->flags & MFX_LOCKY))	vptr->y = FC_cam[0].y >> 8;
				MoveVoice(vptr);
			}
			if ((vptr->flags & MFX_MOVING) && vptr->thing)
			{
				vptr->x = vptr->thing->WorldPos.X >> 8;
				vptr->y = vptr->thing->WorldPos.Y >> 8;
				vptr->z = vptr->thing->WorldPos.Z >> 8;
				MoveVoice(vptr);
			}
		}
	}
}

//----- querying information back -----

UWORD MFX_get_wave(UWORD channel_id, UBYTE index)
{
	if (!drv)	return 0;

	MFX_Voice*	vptr = FindFirst(channel_id);

	while (index--)
	{
		vptr = FindNext(vptr);
	}

	return vptr ? vptr->wave : 0;
}

// dlgproc
//
// dialog box procedure

static BOOL CALLBACK dlgproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	ctrl;
	int		ii;

	ctrl = GetDlgItem(hWnd, IDC_SOUND_DROPDOWN);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			RECT	winsize;
			RECT	scrsize;

			GetWindowRect(hWnd, &winsize);
			GetClientRect(GetDesktopWindow(), &scrsize);

			// localise this bastard
			CBYTE *lang=ENV_get_value_string("language");
			
			if (!lang) lang="text\\lang_english.txt";
			XLAT_load(lang);
			XLAT_init();
			SetWindowText(hWnd,XLAT_str(X_MILES_SETUP));
			SetDlgItemText(hWnd,IDC_STATIC_SELECT,XLAT_str(X_SELECT_SOUND));
			SetDlgItemText(hWnd,IDC_NOSHOW,XLAT_str(X_DO_NOT_SHOW));
			SetDlgItemText(hWnd,IDOK,XLAT_str(X_OKAY));

			int		xoff = ((scrsize.right - scrsize.left) - (winsize.right - winsize.left)) / 2;
			int		yoff = ((scrsize.bottom - scrsize.top) - (winsize.bottom - winsize.top)) / 2;

		//	SetWindowPos(hWnd, NULL, xoff, yoff, 0,0, SWP_NOZORDER | SWP_NOSIZE);

			for (ii = 0; ii < NumProviders; ii++)
			{
				SendMessage(ctrl, CB_INSERTSTRING, -1, (LPARAM)Providers[ii].name);
			}

			SendMessage(ctrl, CB_SETCURSEL, CurProvider, 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			CurProvider = SendMessage(ctrl, CB_GETCURSEL, 0, 0);
			DoMilesDialog = SendMessage(GetDlgItem(hWnd, IDC_NOSHOW), BM_GETCHECK, 0, 0) ? 0 : 1;

		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

// MilesDialog
//
// do the dialog box

static void MilesDialog(HINSTANCE hInst, HWND hWnd)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_MILESDLG), hWnd, (DLGPROC)dlgproc);
}

// MFX_QUICK_play
//
// play a speech

SLONG MFX_QUICK_play(CBYTE* str, SLONG x, SLONG y, SLONG z)
{
	return PlayTalk(str, x,y,z);
}

SLONG MFX_QUICK_still_playing()
{
	MFX_Voice*	vptr = GetVoiceForWave(0, NumSamples, 0);
	if (!vptr)	return 0;

	return IsVoiceDone(vptr) ? 0 : 1;
}

void MFX_QUICK_stop()
{
	MFX_stop(0, NumSamples);
}

void MFX_QUICK_wait()
{
	while (MFX_QUICK_still_playing());
	MFX_QUICK_stop();
}

#if 0

static HAUDIO	quick_wav=NULL;

SLONG	MFX_QUICK_play(CBYTE *str)
{
	char	filename[256];
	strcpy(filename, GetSpeechPath());
	strcat(filename, str);

	quick_wav=AIL_quick_load(filename);
	if(quick_wav)
	{
		AIL_quick_play(quick_wav,1);
		return(1);
	}
	else
	{
		return(0);
	}
}

void	MFX_QUICK_wait(void)
{
	if(quick_wav)
	{
		while(AIL_quick_status(quick_wav)==QSTAT_PLAYING)
		{
		}

		AIL_quick_unload(quick_wav);
		quick_wav=0;


	}
}
SLONG	MFX_QUICK_still_playing(void)
{
	if(quick_wav)
	{
		if(AIL_quick_status(quick_wav)==QSTAT_PLAYING)
		{
			return(1);
		}

	}

	return(0);
}

void	MFX_QUICK_stop(void)
{
	if(quick_wav)
	{

		//
		// The big white manual says unloading a playing sample will stop it, so there!
		//
		AIL_quick_unload(quick_wav);
		quick_wav=0;


	}
}

#endif

#endif	// M_SOUND


#ifndef M_SOUND

#ifndef TARGET_DC
SLONG MFX_QUICK_play(CBYTE* fname)
{
	return 1;
}

void MFX_QUICK_wait(void)
{
}

SLONG MFX_QUICK_still_playing(void)
{
	return 0;
}

void MFX_QUICK_stop()
{
}
#endif

#endif











#ifndef TARGET_DC

void init_my_dialog (HWND hWnd)
{
	HWND	ctrl;
	int		ii;


	#ifdef NO_SOUND
	return;
	#else

	ctrl = GetDlgItem(hWnd, IDC_SOUND_DROPDOWN);

	{
		RECT	winsize;
		RECT	scrsize;

		GetWindowRect(hWnd, &winsize);
		GetClientRect(GetDesktopWindow(), &scrsize);

		// localise this bastard
		CBYTE *lang=ENV_get_value_string("language");
		
		if (!lang) lang="text\\lang_english.txt";
		XLAT_load(lang);
		XLAT_init();
		SetDlgItemText(hWnd,IDC_SOUND_OPTIONS,XLAT_str(X_MILES_SETUP));
		SetDlgItemText(hWnd,IDC_STATIC_SELECT,XLAT_str(X_SELECT_SOUND));
		SetDlgItemText(hWnd,IDC_NOSHOW,XLAT_str(X_DO_NOT_SHOW));
		SetDlgItemText(hWnd,IDOK,XLAT_str(X_OKAY));

		int		xoff = ((scrsize.right - scrsize.left) - (winsize.right - winsize.left)) / 2;
		int		yoff = ((scrsize.bottom - scrsize.top) - (winsize.bottom - winsize.top)) / 2;

		SetWindowPos(hWnd, NULL, xoff, yoff, 0,0, SWP_NOZORDER | SWP_NOSIZE);

		SendMessage(ctrl, CB_RESETCONTENT, 0,0);

		for (ii = 0; ii < NumProviders; ii++)
		{
			SendMessage(ctrl, CB_INSERTSTRING, -1, (LPARAM)Providers[ii].name);
		}

		SendMessage(ctrl, CB_SETCURSEL, CurProvider, 0);

	}
	#endif
}

void my_dialogs_over(HWND hWnd)
{
	HWND	ctrl;

	#ifdef NO_SOUND
	return;
	#else


	ctrl = GetDlgItem(hWnd, IDC_SOUND_DROPDOWN);

	CurProvider = SendMessage(ctrl, CB_GETCURSEL, 0, 0);

	Set3DProvider(CurProvider);

	#endif
}


#endif //#ifndef TARGET_DC

#endif
