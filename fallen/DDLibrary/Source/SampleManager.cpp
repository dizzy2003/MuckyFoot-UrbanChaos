// SampleManager.cpp
// Guy Simmons, 24th February 1998.

#include	"DDLib.h"

// Windows multimedia stuff, used for WAV's.
#include	<mmsystem.h>
#pragma comment(lib, "winmm.lib")

#ifndef VERIFY
#ifdef NDEBUG
#define VERIFY(x) x
#else
#define VERIFY(x) {ASSERT(x);}
#endif
#endif

SampleManager		samples;

//---------------------------------------------------------------
//
//	User functions.
//
//---------------------------------------------------------------

void	LoadSampleList(CBYTE *sample_file)
{
	samples.LoadSamples(sample_file);
}

//---------------------------------------------------------------

void	PlaySample(SLONG ref,SWORD sample_no,SLONG vol,SLONG pan,SLONG freq,SLONG pri)
{
	// Fudged for now.
	SLONG			c0;
	SampleInfo		*current_sample;


	if(sample_no)
	{
		current_sample	=	samples.SampleList;
		for(c0=1;c0<=sample_no;c0++)
		{
			if(c0==sample_no)
			{
				if(current_sample)
					current_sample->Play(ref,vol,pan,freq,pri);
			}
			if(current_sample)
				current_sample	=	current_sample->Next;
			else
				break;
		}
	}
}

//---------------------------------------------------------------

void	StopSample(SLONG user_ref)
{

}

//---------------------------------------------------------------
//
//	CLASS	:	SampleInfo
//
//---------------------------------------------------------------

SampleInfo::SampleInfo()
{
	SampleFlags	=	0;
	hmmFile		=	NULL;

	Next		=	NULL;
	Prev		=	NULL;
}

//---------------------------------------------------------------

SampleInfo::~SampleInfo()
{
	if(hmmFile)
	{
		mmioClose(hmmFile,0);
		hmmFile	=	NULL;
	}
//	Destroy();
}

//---------------------------------------------------------------

HRESULT	SampleInfo::Create(CBYTE *file_name,UBYTE buffer_count)
{
	int				mm_error;
	SWORD			extra_bytes	=	0;	
	MMCKINFO		mm_temp_chunk;
	PCMWAVEFORMAT	temp_wave_format;


	if(IsValid())
	{
		// Programmer error.
		return	FALSE;
	}

	// open the file.
	hmmFile	=	mmioOpenA(file_name,NULL,MMIO_READ);
	if(!hmmFile)
	{
		// It's not a valid file so exit.
		goto	error;
	}

	// Read the RIFF chunk.
	mm_error	=	mmioDescend(hmmFile,&RIFFChunk,NULL,0);
	if(mm_error!=0)
	{
		// Error reading file.
		goto	error;
	}

	// Check the chunk to see if it's a WAV.
	if((RIFFChunk.ckid != FOURCC_RIFF) || (RIFFChunk.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
	{
		// It's not a WAV.
		goto	error;
	}

	// Search for the 'fmt' chunk.
    mm_temp_chunk.ckid	=	mmioFOURCC('f', 'm', 't', ' ');
	mm_error	=	mmioDescend(hmmFile,&mm_temp_chunk,&RIFFChunk,MMIO_FINDCHUNK);
	if(mm_error!=0)
	{
		// Error reading file.
		goto	error;
	}

	// The 'fmt' chunk should be >= sizeof(PCMWAVEFORMAT)
	if(mm_temp_chunk.cksize<sizeof(PCMWAVEFORMAT))
	{
		// It lied, it can't possibly be a WAV.
		goto	error;
	}

	// Read the 'fmt' into PCMWAVEFORMAT.
	mm_error	=	mmioRead(hmmFile,(HPSTR)&temp_wave_format,sizeof(PCMWAVEFORMAT));
	if(mm_error!=sizeof(PCMWAVEFORMAT))
	{
		// Error reading file.
		goto	error;
	}

	// We have to allow for the fact that the 'fmt' might be bigger than
	// sizeof(PCMWAVEFORMAT), so we need to find out how many extra bytes
	// there are.
	if(temp_wave_format.wf.wFormatTag!=WAVE_FORMAT_PCM)
	{
		mm_error	=	mmioRead(hmmFile,(LPSTR)&extra_bytes,sizeof(extra_bytes));
		if(mm_error!=sizeof(extra_bytes))
		{
			// Error reading file.
			goto	error;
		}
	}

	// Copy the pcm structure into the WaveFormatEx structure.
	memcpy(&WaveFormat,&temp_wave_format,sizeof(PCMWAVEFORMAT));
	WaveFormat.cbSize	=	extra_bytes;

	// Now if there are extra bytes we need to read them in.
	if(extra_bytes)
	{
		mm_error	=	mmioRead(hmmFile,(LPSTR)(((BYTE*)&(WaveFormat.cbSize))+sizeof(extra_bytes)),extra_bytes);
		if(mm_error!=extra_bytes)
		{
			// Error reading file.
			goto	error;
		}
	}
	
	// Ascend the input file out of the 'fmt' chunk.
	mm_error	=	mmioAscend(hmmFile,&mm_temp_chunk,0);
	if(mm_error!=0)
	{
		// Error reading file.
		goto	error;
	}


	// Initialise member vars.
	if(buffer_count<1)
		BufferCount	=	1;
	else if(buffer_count>MAX_DUP_BUFFERS)
		BufferCount	=	MAX_DUP_BUFFERS;
	else
		BufferCount	=	buffer_count;

	ZeroMemory(&Priority,sizeof(Priority));
	ZeroMemory(&UserRef,sizeof(UserRef));


	// All is well.
	ValidOn();

	// Success.
	return	DS_OK;

error:
	// Failed.
	if(hmmFile)
	{
		mmioClose(hmmFile,0);
		hmmFile	=	NULL;
	}
	return	DSERR_GENERIC;
}

//---------------------------------------------------------------

HRESULT	SampleInfo::LoadSampleData(void)
{
	int				mm_error;
	UBYTE			*data_ptr1,
					*data_ptr2;
	ULONG			c0,
					data_count1,
					data_count2;
	DSBUFFERDESC	ds_bd;
	HRESULT			result;
	MMCKINFO		mm_temp_chunk;


	// Seek to the end of the RIFF chunk.
	mm_error	=	mmioSeek(hmmFile,RIFFChunk.dwDataOffset+sizeof(FOURCC),SEEK_SET);
	if(mm_error<0)
	{
		// Seek error.
		return	DSERR_GENERIC;
	}

	// Now find the 'data' chunk.
	mm_temp_chunk.ckid	=	mmioFOURCC('d', 'a', 't', 'a');
	mm_error	=	mmioDescend(hmmFile,&mm_temp_chunk,&RIFFChunk,MMIO_FINDCHUNK);
	if(mm_error!=0)
	{
		// Can't find data chunk.
		return	DSERR_GENERIC;
	}

	// Now extract the size of the sample.
	SampleSize		=	mm_temp_chunk.cksize;

	// Extract the base frequency.
	BaseFrequency	=	WaveFormat.nSamplesPerSec;

	// If the loaded flag is set then the buffer has already been created, we just need to reload the data.
	if(!IsLoaded())
	{
		// Set up the buffer description
		InitStruct(ds_bd);


		// Please decide what DSBCAPS_flags you want to use - CTRLDEFAULT is legacy.
		ASSERT ( FALSE );
		//ds_bd.dwFlags		=	DSBCAPS_CTRLDEFAULT;
		ds_bd.dwFlags		=0;




		ds_bd.lpwfxFormat	=	&WaveFormat;

		if(IsStreamed())
		{
			ds_bd.dwFlags		|=	DSBCAPS_CTRLPOSITIONNOTIFY;
			ds_bd.dwBufferBytes	=	3*WaveFormat.nAvgBytesPerSec;
		}
		else
		{
			ds_bd.dwFlags		|=	DSBCAPS_STATIC;
			ds_bd.dwBufferBytes	=	SampleSize;
		}

		// Create the buffer.
		result	=	the_sound_manager.CreateSoundBuffer(&ds_bd,&lp_DS_Buffer[0],NULL);
		if(FAILED(result))
		{
			// Unable to create sound buffer.
			return	result;
		}
	}

	// Now lock the buffer & copy the sample in.
	result	=	lp_DS_Buffer[0]->Lock	(
											0,SampleSize,
											(LPVOID*)&data_ptr1,&data_count1,
											(LPVOID*)&data_ptr2,&data_count2,
											DSBLOCK_ENTIREBUFFER
										);
	if(result==DSERR_BUFFERLOST)
	{
		// Restore the lost buffer.
	}
	else if(FAILED(result))
	{
		// Unable to lock buffer.
		return	result;
	}

	if(IsStreamed())
	{
		// Set up the streaming events.
		SetUpNotifications();
	}
	else
	{
		// Read the sample data in.
		mm_error	=	mmioRead(hmmFile,(HPSTR)data_ptr1,data_count1);
		if(mm_error!=(int)data_count1)
		{
			// Unable to read data, so unlock the buffer & exit.
			lp_DS_Buffer[0]->Unlock(data_ptr1,data_count1,data_ptr2,data_count2);
			return	DSERR_GENERIC;
		}
		if(data_count2)
		{
			mm_error	=	mmioRead(hmmFile,(HPSTR)data_ptr2,data_count2);
			if(mm_error!=(int)data_count2)
			{
				// Unable to read data, so unlock the buffer & exit.
				lp_DS_Buffer[0]->Unlock(data_ptr1,data_count1,data_ptr2,data_count2);
				return	DSERR_GENERIC;
			}
		}

		// Now unlock the buffer.
		result	=	lp_DS_Buffer[0]->Unlock(data_ptr1,data_count1,data_ptr2,data_count2);
	}

	// Again we only need to do this if the LOADED flag is not set.
	if(!IsLoaded())
	{
		// Create the duplicate buffers.
		for(c0=1;c0<BufferCount;c0++)
		{
			the_sound_manager.lp_DS->DuplicateSoundBuffer(lp_DS_Buffer[0],&lp_DS_Buffer[c0]);
		}
	}

	// We now have the main sample data loaded in.
	LoadedOn();

	// Success.
	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	SampleInfo::FreeSampleData(void)
{
	return	DS_OK;
}

//---------------------------------------------------------------

void	*SampleInfo::Play(SLONG user_ref,SLONG vol,SLONG pan,SLONG freq,SLONG priority)
{
	ULONG				c0,
						status;
	SLONG				buffer_index	=	0,
						flags=0,
						lowest_pri		=	0x7fffffff;
	HRESULT				result;
	LPDIRECTSOUNDBUFFER	buffer_ref		=	NULL;


	if(IsValid())
	{
		if(!IsLoaded())
		{
			LoadSampleData();
		}

		if(IsLoaded())
		{
			// Scan for the user reference to see if we're already playing this sample.
			for(c0=0;c0<BufferCount;c0++)
			{
				if(UserRef[c0]==user_ref)
				{
					VERIFY(lp_DS_Buffer[c0]->GetStatus(&status)==DS_OK);
					if(status&DSBSTATUS_PLAYING)
					{
						// Yes we are, so just set the control stuff & return.
						VERIFY(Control(lp_DS_Buffer[c0],vol,pan,freq)==DS_OK);
						return	lp_DS_Buffer[c0];
					}
				}
			}

			// Find a buffer that isn't currently playing.
			for(c0=0;c0<BufferCount;c0++)
			{
				VERIFY(lp_DS_Buffer[c0]->GetStatus(&status)==DS_OK);
				if(!status&DSBSTATUS_PLAYING)
				{
					// Got one.
					buffer_ref		=	lp_DS_Buffer[c0];
					buffer_index	=	c0;
					break;
				}
				else if(Priority[c0]<lowest_pri)
				{
					// Make a note of the buffer that has been playing the longest.
					lowest_pri		=	Priority[c0];
					buffer_index	=	c0;
				}
			}

			// If all buffers are playing then trash the one with the lowest priority.
			if(!buffer_ref && priority>=lowest_pri)
			{
				buffer_ref	=	lp_DS_Buffer[buffer_index];
			}
		}
	}

	if(buffer_ref)
	{
		// Ensure that the play position is at the start of the data.
		VERIFY(buffer_ref->SetCurrentPosition(0)==DS_OK);

		// Set the sound buffer controls.
		VERIFY(Control(buffer_ref,vol,pan,freq)==DS_OK);

		// Set the priority & the user reference.
		Priority[buffer_index]	=	priority;
		UserRef[buffer_index]	=	user_ref;

		// Set up the play flags.
		if(IsLooped())
			flags	=	DSBPLAY_LOOPING;

		// Start the sample.
		result	=	buffer_ref->Play(0,0,flags);
		if(result==DSERR_BUFFERLOST)
		{
			Restore(buffer_ref);
			result	=	buffer_ref->Play(0,0,flags);
		}
		if(FAILED(result))
			buffer_ref	=	NULL;
	}

	return	buffer_ref;
}

//---------------------------------------------------------------

HRESULT	SampleInfo::Control(LPDIRECTSOUNDBUFFER the_buf,SLONG vol,SLONG pan,SLONG freq)
{
	SLONG		new_freq;


	new_freq	=	BaseFrequency+freq;
	in_range(new_freq,DSBFREQUENCY_MIN,DSBFREQUENCY_MAX);
	VERIFY(the_buf->SetFrequency(new_freq)==DS_OK);

	in_range(pan,DSBPAN_LEFT,DSBPAN_RIGHT);
	VERIFY(the_buf->SetPan(pan)==DS_OK);

	in_range(vol,DSBVOLUME_MIN,DSBVOLUME_MAX);
	VERIFY(the_buf->SetVolume(vol)==DS_OK);

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	SampleInfo::Restore(LPDIRECTSOUNDBUFFER the_buf)
{
	HRESULT		result;


	result	=	the_buf->Restore();
	if(SUCCEEDED(result))
	{
		// If the buffer was lost then the sample data will have been too,
		// so force a reload of the data.
		if(IsLoaded())
			result	=	LoadSampleData();
	}
	else
	{
		// If an error occured at this stage then the sample is probably lost for good.
		// We probably need to do a tidy up of the DS buffer here.
	}

	return	result;
}

//---------------------------------------------------------------

#define	NUM_PLAY_NOTIFICATIONS		4

HRESULT	SampleInfo::SetUpNotifications(void)
{
	//DSBPOSITIONNOTIFY dsbPosNotify[NUM_PLAY_NOTIFICATIONS +1 ];	

	
	// Create the 2 events. One for Play one for Stop.
	hNotifyEvent[0]	=	CreateEvent(NULL, FALSE, FALSE, NULL);
	hNotifyEvent[1]	=	CreateEvent(NULL, FALSE, FALSE, NULL);

	// setup the first one.
//	dsbPosNotify[0].dwOffset		=	dwSize;
//	dsbPosNotify[0].hEventNotify	=	hNotifyEvent[0];

	return	DS_OK;
}

//---------------------------------------------------------------
//
//	CLASS	:	SampleManager
//
//---------------------------------------------------------------

SampleManager::SampleManager()
{
	ManagerFlags	=	0;

	SampleCount		=	0;
	SampleList		=	NULL;
	SampleListEnd	=	NULL;
}

//---------------------------------------------------------------

SampleManager::~SampleManager()
{
	DestroySamples();
}

//---------------------------------------------------------------

HRESULT	SampleManager::LoadSamples(CBYTE *script_name)
{
	CBYTE			sample_name[MAX_PATH];
	UBYTE			looped,
					rep_count,
					sample_3d,
					streamed;
	FILE			*script_handle;
	HRESULT			result	=	DSERR_GENERIC;
	SampleInfo		*new_sample;


	// Get rid of any existing samples.
	DestroySamples();

	// Open the script file.
	script_handle	=	MF_Fopen(script_name,"r");
	if(script_handle)
	{
		while(fscanf(script_handle,"%s %d %d %d %d",sample_name,&rep_count,&looped,&streamed,&sample_3d)>0)
		{
			new_sample	=	MFnew<SampleInfo>();
			if(new_sample)
			{
				// Initialise the sample.
				result	=	new_sample->Create(sample_name,rep_count);
				if(FAILED(result))
				{
					MFdelete(new_sample);
					return	result;
				}

				// Set up attributes.
				if(looped)
					new_sample->LoopedOn();
				if(streamed)
					new_sample->StreamedOn();
				if(sample_3d)
					new_sample->Sample3DOn();

				// Add the sample to the list.
				result	=	AddSample(new_sample);
				if(FAILED(result))
				{
					MFdelete(new_sample);
					return	result;
				}
			}
		}

		// Finished with the script.
		MF_Fclose(script_handle);
	}

	return	DSERR_GENERIC;
}

//---------------------------------------------------------------

HRESULT	SampleManager::DestroySamples(void)
{
	SampleInfo		*current_sample,
					*next_sample;


	current_sample	=	SampleList;
	while(current_sample)
	{
		next_sample		=	current_sample->Next;

		MFdelete(current_sample);

		current_sample	=	next_sample;
	}

	SampleCount		=	0;
	SampleList		=	NULL;
	SampleListEnd	=	NULL;

	// Success.
	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	SampleManager::AddSample(SampleInfo *the_sample)
{
	if(!the_sample)
	{
		// Error, Invalid parameters
		return	DSERR_GENERIC;
	}

	// Add sample to list.
	the_sample->Prev	=	SampleListEnd;
	the_sample->Next	=	NULL;

	// Update list end.
	if(SampleListEnd)
		SampleListEnd->Next	=	the_sample;
	SampleListEnd	=	the_sample;

	// Update list.
	if(!SampleList)
		SampleList			=	the_sample;

	SampleCount++;

	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	SampleManager::DeleteSample(SampleInfo *the_sample)
{
	return	DS_OK;
}

//---------------------------------------------------------------
