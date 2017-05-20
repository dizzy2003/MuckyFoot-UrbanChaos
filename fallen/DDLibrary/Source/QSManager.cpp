//	QSManager.cpp
//	Guy Simmons, 6th May 1998.


#ifndef TARGET_DC

#include	"DDLib.h"
#include	"C:\fallen\DDEngine\Headers\Matrix.h"

//---------------------------------------------------------------
//
//	User Functions.
//
//---------------------------------------------------------------

void	LoadWaveList(CBYTE *wave_path,CBYTE *wave_list)
{
	the_qs_sound_manager.LoadWaves(wave_path,wave_list);
}

//---------------------------------------------------------------

void	LoadWave(CBYTE *wave_name)
{
	the_qs_sound_manager.LoadWave(wave_name);
}

//---------------------------------------------------------------

void	FreeWaveList(void)
{
	the_qs_sound_manager.FreeWaves();
}

//---------------------------------------------------------------

void	PlayWave(SLONG ref,SLONG wave_id,SLONG play_type,WaveParams *the_params)
{
	the_qs_sound_manager.PlayWave(ref,wave_id-1,play_type,the_params);
}

//---------------------------------------------------------------

void	StopWave(SLONG ref,SLONG wave_id)
{
	the_qs_sound_manager.StopWave(ref,wave_id-1);
}

//---------------------------------------------------------------

void	SetListenerPosition(SLONG x,SLONG y,SLONG z,SLONG scale)
{
	float		f_scale;
	QMIX_RESULT	r;
	QSVECTOR	position;
	TCHAR		error_text[256];


	f_scale	=	1.0f/(float)scale;
	position.x	=	(float)x*f_scale;
	position.y	=	(float)y*f_scale;
	position.z	=	(float)z*f_scale;
	
	r	=	QS(SetListenerPosition(the_qs_sound_manager.GetHQMixer(),&position,0));
	if(r)
	{
		QS(GetErrorText(r,error_text,256));
//		DebugText("%s\n",error_text);
	}
}

//---------------------------------------------------------------

void	SetListenerOrientation(SLONG angle,SLONG roll,SLONG tilt)
{
	QMIX_RESULT	r;
	QSVECTOR	direction,
				up;
	TCHAR		error_text[256];
/*	float		dirvec[3];

	MATRIX_vector(dirvec,angle,tilt);
	direction.x=dirvec[0];
	direction.y=dirvec[1];
	direction.z=dirvec[2];
	*/

	direction.x	=	0.0f;
	direction.y	=	0.0f;
	direction.z	=	1.0f;
	up.x		=	0.0f;
	up.y		=	1.0f;
	up.z		=	0.0f;
	r	=	QS(SetListenerOrientation(the_qs_sound_manager.GetHQMixer(),&direction,&up,0));
	if(r)
	{
		QS(GetErrorText(r,error_text,256));
//		DebugText("%s\n",error_text);
	}

	//	Fudge the velocity.
	up.x		=	0.0f;
	up.y		=	0.0f;
	up.z		=	0.0f;
	r	=	QS(SetListenerVelocity(the_qs_sound_manager.GetHQMixer(),&up,0));
	if(r)
	{
		QS(GetErrorText(r,error_text,256));
//		DebugText("%s\n",error_text);
	}
}

//---------------------------------------------------------------
//
//	class	QSManager
//
//---------------------------------------------------------------

QSManager		the_qs_sound_manager;

//---------------------------------------------------------------

QSManager::QSManager()
{
	ManagerFlags	=	0;
	WaveList		=	WaveListEnd	=	NULL;
	WaveCount		=	0;
}

//---------------------------------------------------------------

QSManager::~QSManager()
{
	Fini();
}

//---------------------------------------------------------------

HRESULT	QSManager::Init(void)
{
	if(!IsInitialised())
	{
		//	Set up the config struct & initialise QSound.
		InitStruct(Config);
		Config.dwFlags			=	QMIX_CONFIG_LEFTCOORDS;
		Config.dwSamplingRate	=	22050;
		Config.iChannels		=	MAX_CHANNELS;
		Config.hwnd				=	hDDLibWindow;
		HQMixer					=	QS(InitEx(&Config));

		//	If all is well active QSound.
		if(HQMixer)
		{
			SLONG	r;
			CBYTE	error_text[200];
			ActivateSound();

			//	Open all the channels.
			r=QS(OpenChannel(HQMixer,0,QMIX_OPENALL));
			if(r)
			{
				QS(GetErrorText(r,error_text,256));
//				DebugText("PlayWave: %s\n",error_text);
			}

			InitOn();
		}

	}
	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	QSManager::Fini(void)
{
	if(IsInitialised())
	{
		QS(CloseSession(HQMixer));

		InitOff();
	}
	return	DS_OK;
}

//---------------------------------------------------------------

void	QSManager::ActivateSound(void)
{
	//	Activate QSound.
	if(!IsActive())
	{
		if(!QS(Activate(HQMixer,TRUE)))
			ActiveOn();
	}
}

//---------------------------------------------------------------

void	QSManager::DeactivateSound(void)
{
	//	Deactivate QSound.
	if(IsActive())
	{
		if(!QS(Activate(HQMixer,FALSE)))
			ActiveOff();
	}
}

//---------------------------------------------------------------

HRESULT	QSManager::LoadWaves(CBYTE *wave_path,CBYTE *script_name)
{
	CBYTE			wave_name[MAX_PATH],
					wave_file[MAX_PATH];
	ULONG			streamed;
	FILE			*script_handle;
	HRESULT			result	=	DSERR_GENERIC;
	Wave			*new_wave;

	
	// Get rid of any existing samples.
	FreeWaves();
//	LogText
	// Open the script file.
	script_handle	=	MF_Fopen(script_name,"r");
	if(script_handle)
	{
		while(fscanf(script_handle,"%s %d",wave_name,&streamed)>0)
		{
//			LogText(" wave name %s streamed %d \n",wave_name,streamed);
			new_wave	=	new Wave;
			if(new_wave)
			{
				// Set up attributes.
				if(streamed)
					new_wave->StreamedOn();

				// Initialise the wave.
				strcpy(wave_file,wave_path);
				strcat(wave_file,wave_name);
				result	=	new_wave->Init(wave_file,HQMixer);
				if(FAILED(result))
				{
					delete	new_wave;
					return	result;
				}

				AddWave(new_wave);

			}
		}

		// Finished with the script.
		MF_Fclose(script_handle);
	}

	return	result;
}

//---------------------------------------------------------------

HRESULT	QSManager::LoadWave(CBYTE *wave_name)
{
	ULONG			streamed=0; //temp?
	HRESULT			result	=	DSERR_GENERIC;
	Wave			*new_wave;

	
	new_wave	=	new Wave;
	if(new_wave)
	{
		// Set up attributes.
		if(streamed)
			new_wave->StreamedOn();

		// Initialise the wave.
		result	=	new_wave->Init(wave_name,HQMixer);
		if(FAILED(result))
		{
			delete	new_wave;
			return	result;
		}

		AddWave(new_wave);

	}
	return	result;
}

//---------------------------------------------------------------

HRESULT	QSManager::FreeWaves(void)
{
	Wave		*current_wave,
				*next_wave;


	current_wave	=	WaveList;
	while(current_wave)
	{
		next_wave	=	current_wave->Next;
		current_wave->Fini(HQMixer);
		delete	current_wave;

		current_wave	=	next_wave;
	}

	//	Initialise wave list.
	WaveList	=	WaveListEnd	=	NULL;
	WaveCount	=	0;

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	QSManager::AddWave(Wave *the_wave)
{
	if(!the_wave)
	{
		// Error, Invalid parameters
		return	DSERR_GENERIC;
	}

	// Add sample to list.
	the_wave->Prev	=	WaveListEnd;
	the_wave->Next	=	NULL;

	// Update list end.
	if(WaveListEnd)
		WaveListEnd->Next	=	the_wave;
	WaveListEnd	=	the_wave;

	// Update list.
	if(!WaveList)
		WaveList			=	the_wave;

	WaveCount++;

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	QSManager::DeleteWave(Wave *the_wave)
{
	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	QSManager::PlayWave(SLONG wave_ref,SLONG wave_id,SLONG play_type,WaveParams *the_params)
{
	float			f_scale;
	int				channel;
	BOOL			channel_done	=	FALSE,
					has_channel		=	FALSE;
	SLONG			c0,
					play_flags	=	QMIX_QUEUEWAVE,
					play_loop	=	0;
	LPMIXWAVE		the_wave;
	Wave			*current_wave;
	QMIX_DISTANCES	distances;
	QMIXPLAYPARAMS	play_params;
	QMIX_RESULT		r;
	QSVECTOR		wave_position;
	TCHAR			error_text[256];


	//	Get a pointer to the requested wave.
	current_wave	=	WaveList;
	for(c0=0;c0<wave_id;c0++)
	{
		current_wave	=	current_wave->Next;
		if(!current_wave)
			return	DSERR_GENERIC;
	}
	the_wave	=	current_wave->GetMixWave();

	//	Check the wave.
	if(!the_wave)
		return	DSERR_GENERIC;

	//	Try & match the wave ref & channel.
	for(c0=0;c0<MAX_CHANNELS;c0++)
	{
		if(Channels[c0].GetUserRef()==wave_ref)
		{
			has_channel	=	TRUE;
			break;
		}
	}

	if(has_channel)
	{
		//	Set the channel.
		channel	=	c0;
		Channels[channel].SetWaveID(wave_id);
	}
	else
	{
		//	Get a channel.
//		channel	=	QS(FindChannel(HQMixer,QMIX_FINDCHANNEL_MRA,0));
		channel	=	QS(FindChannel(HQMixer,QMIX_FINDCHANNEL_LRU,0));
//		DebugText("Got new channel - %ld\n",channel);

		//	Check to see we have a channel, otherwise try & grab one with a lower priority.
		

		//	Set up the channel info.
		Channels[channel].SetUserRef(wave_ref);
		Channels[channel].SetPriority(0);
		Channels[channel].SetWaveID(wave_id);
	}

	if(channel>=0)
	{
		if(the_params)
		{
			//	Set the channel parameters according to the user params.
			switch(the_params->Flags&WAVE_TYPE_MASK)
			{
				case	WAVE_STEREO:
					QS(SetVolume(HQMixer,channel,QMIX_USEONCE,32767));
					break;
				case	WAVE_POLAR:

					break;		
				case	WAVE_CARTESIAN:
					f_scale	=	1.0f/(float)the_params->Mode.Cartesian.Scale;
					wave_position.x	=	(float)the_params->Mode.Cartesian.X*f_scale;
					wave_position.y	=	(float)the_params->Mode.Cartesian.Y*f_scale;
					wave_position.z	=	(float)the_params->Mode.Cartesian.Z*f_scale;
					r	=	QS(SetSourcePosition(HQMixer,channel,0,&wave_position));

					//	Report any errors.
					if(r)
					{
						QS(GetErrorText(r,error_text,256));
//						DebugText("PlayWave: %s\n",error_text);
					}

					if(the_params->Flags&WAVE_DISTANCE_MAPPING)
					{
						distances.cbSize		=	sizeof(QMIX_DISTANCES);
						distances.minDistance	=	40.0f;
						distances.maxDistance	=	56.0f;
						distances.scale			=	8.0f;
						QS(SetDistanceMapping(HQMixer,channel,QMIX_USEONCE,&distances));
					}
					else
					{
						distances.cbSize		=	sizeof(QMIX_DISTANCES);
						distances.minDistance	=	20.0f;
						distances.maxDistance	=	100.0;
						distances.scale			=	1.0f;
						QS(SetDistanceMapping(HQMixer,channel,QMIX_USEONCE,&distances));
					}
                	break;
			}
		}

		//	Loop status.
		if(the_params->Flags&WAVE_PAN_RATE)
			QS(SetPanRate(HQMixer,channel,QMIX_USEONCE,1000));

		//	Pan rate status.
		if(the_params->Flags&WAVE_LOOP)
			play_loop	=	-1;

		//	Set the play mode.
		if(play_type==WAVE_PLAY_QUEUE)
		{
			play_flags	=	QMIX_QUEUEWAVE;
		}
		else
		{
			play_flags	=	QMIX_CLEARQUEUE;

			//	Get channel status.
			channel_done	=	QS(IsChannelDone(HQMixer,channel));

			//	If we don't want to interupt the current wave then exit.
			if(!channel_done && play_type==WAVE_PLAY_NO_INTERUPT)
				return	DS_OK;
			//	If we want to play the wave lots then get any old channel.
			else if(!channel_done && play_type==WAVE_PLAY_OVERLAP)
			{
				channel	=	QS(FindChannel(HQMixer,QMIX_FINDCHANNEL_LRU,0));
				if(channel<0)
					return	DSERR_GENERIC;
			}
		}

		//	Play the wave.
		if(the_params->Flags&WAVE_SET_LOOP_POINTS)
		{
			InitStruct(play_params);
			play_params.lStartLoop	=	the_params->LoopStart;
			play_params.lEndLoop	=	the_params->LoopEnd;
//			play_params.lEnd		=	the_params->LoopEnd;

			r	=	QS(PlayEx(HQMixer,channel,play_flags,the_wave,play_loop,&play_params));
		}
		else
		{
			r	=	QS(PlayEx(HQMixer,channel,play_flags,the_wave,play_loop,0));
		}

		//	Report any errors.
		if(r)
		{
			QS(GetErrorText(r,error_text,256));
//			DebugText("PlayWave: %s\n",error_text);
		}
		
		//	Free up the channel if the wave is not playing.
		if(QS(IsChannelDone(HQMixer,channel)))
			Channels[channel].SetUserRef(-1);	
	}

	//	return	success.
	return	DS_OK;
}
 
//---------------------------------------------------------------

HRESULT	QSManager::StopWave(SLONG wave_ref,SLONG wave_id)
{
	BOOL			has_channel	=	FALSE;
	SLONG			c0;
	TCHAR			error_text[256];
	QMIX_RESULT		r;

	
	//	Try & match the wave ref & channel.
	for(c0=0;c0<MAX_CHANNELS;c0++)
	{
		if(Channels[c0].GetUserRef()==wave_ref)
		{
			has_channel	=	TRUE;
			break;
		}
	}

	if(has_channel)
	{
		if(Channels[c0].GetWaveID()==wave_id)
		{
			r	=	QS(FlushChannel(HQMixer,c0,0));

			//	Report any errors.
			if(r)
			{
				QS(GetErrorText(r,error_text,256));
//				DebugText("PlayWave: %s\n",error_text);
			}

			Channels[c0].SetUserRef(-1);
			Channels[c0].SetWaveID(0);
		}
	}

	return	DS_OK;
}

//---------------------------------------------------------------
//
//	class	Wave
//
//---------------------------------------------------------------

Wave::Wave()
{
	WaveFlags	=	0;
	MixWave		=	NULL;
	Next		=	NULL;
	Prev		=	NULL;
}

//---------------------------------------------------------------

Wave::~Wave()
{
}

//---------------------------------------------------------------

HRESULT	Wave::Init(CBYTE *file_name,HQMIXER the_mixer)
{
	QMIXWAVEPARAMS		wave_params;

	wave_params.FileName	=	file_name;
	if(IsStreamed())
	{
		MixWave	=	QS(OpenWaveEx(the_mixer,&wave_params,QMIX_FILESTREAM));
	}
	else
	{
		MixWave	=	QS(OpenWaveEx(the_mixer,&wave_params,QMIX_FILE));
	}

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	Wave::Fini(HQMIXER the_mixer)
{
	QS(FreeWave(the_mixer,MixWave));

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	Wave::Load(void)
{

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	Wave::Free(void)
{

	return	DS_OK;
}

//---------------------------------------------------------------
//
//	class	Channel
//
//---------------------------------------------------------------

Channel::Channel()
{
	ChannelFlags	=	0;
	IChannel		=	0;
	ChannelPriority	=	0;
	ChannelUserRef	=	-1;
}

//---------------------------------------------------------------

Channel::~Channel()
{
}

//---------------------------------------------------------------

void	Channel::Open(void)
{
	
}

//---------------------------------------------------------------

void	Channel::Close(void)
{
	
}

//---------------------------------------------------------------


#endif //#ifndef TARGET_DC

