// MFX_Miles.h
//
// Miles Sound System


#ifndef TARGET_DC

#ifndef _MFX_MILES_H_
#define _MFX_MILES_H_

#include "c:\fallen\miles\mss.h" // i tried changing the project settings but it didn't care

// MFX_Sample
//
// sample control block

enum SampleType
{
	SMP_Ambient = 0,
	SMP_Music,
	SMP_Effect
};

struct MFX_Sample
{
	MFX_Sample*	prev_lru;		// prev LRU entry (i.e. sample used before this one)
	MFX_Sample*	next_lru;		// next LRU entry (i.e. sample used after this one)
	char*		fname;			// sample filename
	void*		dptr;			// data pointer for sample, else NULL
	bool		is3D;			// is 3D?
	bool		stream;			// sample is to be streamed?
	int			rate;			// sample rate (initialized when sample is loaded)
	int			size;			// size of sample, in bytes
	int			usecount;		// sample is in use (count)
	int			type;			// SampleType of sample - used to get volume
	float		linscale;		// linear scaling for sample volume - used to set 3D distances
	bool		loading;		// sample is loading
};

#define MAX_SAMPLE		552				// max. number of samples
#define MIN_STREAM_SIZE	1024*1024		// samples bigger than this are streamed
#define MAX_SAMPLE_MEM	16*1024*1024	// max. amount of sample mem

// MFX_QWave
//
// control block for a queued wave

struct MFX_QWave
{
	MFX_QWave*	next;		// next voice to be queued, or NULL
	ULONG		wave;		// sound sample to be played
	ULONG		flags;		// flags for the sample
	SLONG		x,y,z;		// coordinates of the voice
	float		gain;		// gain of voice
};

#define MAX_QWAVE		32	// number of queued wave slots
#define MAX_QVOICE		5	// maximum waves queued per voice

// MFX_Voice
//
// voice control block

struct Thing;

struct MFX_Voice
{
	UWORD		id;			// channel_id for this voice
	ULONG		wave;		// sound sample playing on this voice
	ULONG		flags;		// flags for this voice (see mfx.h)
	SLONG		x,y,z;		// coordinates of this voice
	Thing*		thing;		// thing this voice belongs to
	MFX_QWave*	queue;		// queue of samples to play
	SLONG		queuesz;	// number of queued samples
	MFX_Sample*	smp;		// sample being played
	HSAMPLE		h2D;		// 2D sound handle
	H3DSAMPLE	h3D;		// 3D sound handle
	HSTREAM		hStream;	// stream handle
	bool		playing;	// sample is playing?
	float		ratemult;	// rate multiplier
	float		gain;		// gain
};

#define MAX_VOICE	64		// number of voices
#define	VOICE_MSK	63		// mask for voice indices

// MFX_3D
//
// a 3D provider

struct MFX_3D
{
	HPROVIDER	hnd;
	char*		name;
};

#define MAX_PROVIDER	16	// max. number of providers

// Miles-specific exports

HDIGDRIVER	GetMilesDriver();

int		Get3DProviderList(MFX_3D** prov);
void	Set3DProvider(int ix);
int		Get3DProvider();

void	SetLinScale(SLONG wave, float linscale);	// set volume scaling for sample (1.0 = normal)
void	SetPower(SLONG wave, float dB);				// set power for sample in dB (0 = normal)



void init_my_dialog (HWND hWnd);
void my_dialogs_over(HWND hWnd);




#endif


#endif //#ifndef TARGET_DC
