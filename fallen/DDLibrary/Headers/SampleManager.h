// SampleManager.h
// Guy Simmons, 24th February 1998.

#ifndef	SAMPLEMANAGER_H
#define	SAMPLEMANAGER_H

//---------------------------------------------------------------

#define	SM_MANAGER_INIT		(1<<0)

#define	SM_SAMPLE_VALID		(1<<0)
#define	SM_SAMPLE_LOADED	(1<<1)
#define	SM_SAMPLE_LOOPED	(1<<2)
#define	SM_SAMPLE_STREAMED	(1<<3)
#define	SM_SAMPLE_3D		(1<<4)

#define	MAX_DUP_BUFFERS		10

//---------------------------------------------------------------

class	SampleInfo
{
	private:
	protected:
	public:
		ULONG				BufferCount,
							SampleFlags,
							SampleSize;
		SLONG				BaseFrequency,
							Priority[MAX_DUP_BUFFERS],
							UserRef[MAX_DUP_BUFFERS];
		HANDLE				hNotifyEvent[2];
		HMMIO				hmmFile;
		LPDIRECTSOUNDBUFFER	lp_DS_Buffer[MAX_DUP_BUFFERS];
		LPDIRECTSOUNDNOTIFY	lp_DS_Notify;
		MMCKINFO			RIFFChunk;
		WAVEFORMATEX		WaveFormat;

		SampleInfo			*Next,
							*Prev;

							SampleInfo();
							~SampleInfo();

		// Methods.
		HRESULT				Create(CBYTE *file_name,UBYTE buffer_count);
		HRESULT				LoadSampleData(void);
		HRESULT				FreeSampleData(void);
		void				*Play(SLONG user_ref,SLONG vol,SLONG pan,SLONG freq,SLONG priority);
		HRESULT				Control(LPDIRECTSOUNDBUFFER the_buf,SLONG vol,SLONG pan,SLONG freq);
		HRESULT				Restore(LPDIRECTSOUNDBUFFER the_buf);


		HRESULT				SetUpNotifications(void);

		inline	BOOL		IsValid(void)				{	return	SampleFlags&SM_SAMPLE_VALID;		}
		inline	void		ValidOn(void)				{	SampleFlags		|=	SM_SAMPLE_VALID;		}
		inline	void		ValidOff(void)				{	SampleFlags		&=	~SM_SAMPLE_VALID;		}

		inline	BOOL		IsLoaded(void)				{	return	SampleFlags&SM_SAMPLE_LOADED;		}
		inline	void		LoadedOn(void)				{	SampleFlags		|=	SM_SAMPLE_LOADED;		}
		inline	void		LoadedOff(void)				{	SampleFlags		&=	~SM_SAMPLE_LOADED;		}

		inline	BOOL		IsLooped(void)				{	return	SampleFlags&SM_SAMPLE_LOOPED;		}
		inline	void		LoopedOn(void)				{	SampleFlags		|=	SM_SAMPLE_LOOPED;		}
		inline	void		LoopedOff(void)				{	SampleFlags		&=	~SM_SAMPLE_LOOPED;		}

		inline	BOOL		IsStreamed(void)			{	return	SampleFlags&SM_SAMPLE_STREAMED;		}
		inline	void		StreamedOn(void)			{	SampleFlags		|=	SM_SAMPLE_STREAMED;		}
		inline	void		StreamedOff(void)			{	SampleFlags		&=	~SM_SAMPLE_STREAMED;	}

		inline	BOOL		IsSample3D(void)			{	return	SampleFlags&SM_SAMPLE_3D;			}
		inline	void		Sample3DOn(void)			{	SampleFlags		|=	SM_SAMPLE_3D;			}
		inline	void		Sample3DOff(void)			{	SampleFlags		&=	~SM_SAMPLE_3D;			}
};

//---------------------------------------------------------------

class	SampleManager
{
	private:
	protected:
	public:
		ULONG			ManagerFlags;

		// Samples Info
		ULONG			SampleCount;					// Count of Modes.
		SampleInfo		*SampleList,					// List of Modes.
						*SampleListEnd;


						SampleManager();
						~SampleManager();

		// Methods.
		HRESULT			LoadSamples(CBYTE *script_name);
		HRESULT			DestroySamples(void);

		HRESULT			AddSample(SampleInfo *the_sample);
		HRESULT			DeleteSample(SampleInfo *the_sample);
};

//---------------------------------------------------------------

extern	SampleManager		samples;

//---------------------------------------------------------------
#endif

