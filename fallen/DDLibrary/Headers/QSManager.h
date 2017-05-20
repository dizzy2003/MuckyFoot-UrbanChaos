//	QSManager.h
//	Guy Simmons, 6th May 1998.

#ifndef	QSMANAGER_H
#define	QSMANAGER_H

//---------------------------------------------------------------

// #include "qmixer.h"
#include "qmdx.h"

#if defined(QMDX_H)
	#define QS(name) QMDX_ ## name
#ifndef TARGET_DC
	#pragma comment(lib, "qmdx.lib")
#endif
#else
	#define QS(name) QSWaveMix ## name
#ifndef TARGET_DC
	#pragma comment(lib, "Qmixer.lib")
#endif
#endif

//---------------------------------------------------------------

#define	QS_WAVE_VALID			(1<<0)
#define	QS_WAVE_LOADED			(1<<1)
#define	QS_WAVE_STREAMED		(1<<2)

//---------------------------------------------------------------

class	Wave
{
	private:
		ULONG				WaveFlags;
		LPMIXWAVE			MixWave;

	public:
		Wave				*Next,
							*Prev;

							Wave();
							~Wave();

		//	Methods.
		HRESULT				Init(CBYTE *file_name,HQMIXER the_mixer);
		HRESULT				Fini(HQMIXER the_mixer);
		HRESULT				Load(void);
		HRESULT				Free(void);

		inline	BOOL		IsValid(void)				{	return	WaveFlags&QS_WAVE_VALID;		}
		inline	void		ValidOn(void)				{	WaveFlags		|=	QS_WAVE_VALID;		}
		inline	void		ValidOff(void)				{	WaveFlags		&=	~QS_WAVE_VALID;		}

		inline	BOOL		IsLoaded(void)				{	return	WaveFlags&QS_WAVE_LOADED;		}
		inline	void		LoadedOn(void)				{	WaveFlags		|=	QS_WAVE_LOADED;		}
		inline	void		LoadedOff(void)				{	WaveFlags		&=	~QS_WAVE_LOADED;	}

		inline	BOOL		IsStreamed(void)			{	return	WaveFlags&QS_WAVE_STREAMED;	}
		inline	void		StreamedOn(void)			{	WaveFlags		|=	QS_WAVE_STREAMED;	}
		inline	void		StreamedOff(void)			{	WaveFlags		&=	~QS_WAVE_STREAMED;	}

		inline	LPMIXWAVE	GetMixWave(void)			{	return	MixWave;						}
};

//---------------------------------------------------------------

#define	QS_CHANNEL_OPEN			(1<<0)

//---------------------------------------------------------------

class	Channel
{
	private:
		int					IChannel;
		ULONG				ChannelFlags,
							ChannelPriority;
		SLONG				ChannelUserRef,
							ChannelWaveID;

	public:
							Channel();
							~Channel();

		//	Methods.
		void				Open(void);
		void				Close(void);

		inline SLONG		GetUserRef(void)			{	return	ChannelUserRef;						}
		inline void			SetUserRef(SLONG ref)		{	ChannelUserRef	=	ref;					}
		inline SLONG		GetPriority(void)			{	return	ChannelPriority;					}
		inline void			SetPriority(SLONG pri)		{	ChannelPriority	=	pri;					}
		inline SLONG		GetWaveID(void)				{	return	ChannelWaveID;						}
		inline void			SetWaveID(SLONG id)			{	ChannelWaveID	=	id;						}
};

//---------------------------------------------------------------

#define	QS_MANAGER_INIT			(1<<0)
#define	QS_MANAGER_ACTIVE		(1<<1)

#define	MAX_CHANNELS			16

//---------------------------------------------------------------

class	QSManager
{
	private:
		ULONG				ManagerFlags,
							WaveCount;
		Channel				Channels[MAX_CHANNELS];
		HQMIXER				HQMixer;
		QMIXCONFIG			Config;
		Wave				*WaveList,
							*WaveListEnd;


	public:
							QSManager();
							~QSManager();

		// Methods.
		HRESULT				Init(void);
		HRESULT				Fini(void);
		void				ActivateSound(void);
		void				DeactivateSound(void);

		HRESULT				LoadWaves(CBYTE *wave_path,CBYTE *script_name);
		HRESULT				LoadWave(CBYTE *wave_name);
		HRESULT				FreeWaves(void);
		HRESULT				AddWave(Wave *the_wave);
		HRESULT				DeleteWave(Wave *the_wave);

		HRESULT				PlayWave(SLONG wave_ref,SLONG wave_id,SLONG play_type,WaveParams *the_params);
		HRESULT				StopWave(SLONG wave_ref,SLONG wave_id);

		inline	BOOL		IsInitialised(void)			{	return	ManagerFlags&QS_MANAGER_INIT;		}
		inline	void		InitOn(void)				{	ManagerFlags	|=	QS_MANAGER_INIT;		}
		inline	void		InitOff(void)				{	ManagerFlags	&=	~QS_MANAGER_INIT;		}

		inline	BOOL		IsActive(void)				{	return	ManagerFlags&QS_MANAGER_ACTIVE;		}
		inline	void		ActiveOn(void)				{	ManagerFlags	|=	QS_MANAGER_ACTIVE;		}
		inline	void		ActiveOff(void)				{	ManagerFlags	&=	~QS_MANAGER_ACTIVE;		}

		inline	HQMIXER		GetHQMixer(void)			{	return	HQMixer;							}
};

//---------------------------------------------------------------

extern QSManager		the_qs_sound_manager;

//---------------------------------------------------------------

#endif
