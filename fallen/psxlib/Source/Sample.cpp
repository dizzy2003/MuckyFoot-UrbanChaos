// SampleManager.cpp
// Guy Simmons, 24th February 1998.


// Windows multimedia stuff, used for WAV's.

//---------------------------------------------------------------
//
//	User functions.
//
//---------------------------------------------------------------
#include	<MFStdLib.h>

void	LoadSampleList(CBYTE *sample_file)
{
}

//---------------------------------------------------------------

void	PlaySample(SLONG ref,SWORD sample_no,SLONG vol,SLONG pan,SLONG freq,SLONG pri)
{
}

//---------------------------------------------------------------

void	StopSample(SLONG user_ref)
{

}



void	PlayWave(SLONG ref,SLONG wave_id,SLONG play_type,WaveParams *the_params)
{
//	the_qs_sound_manager.PlayWave(ref,wave_id,play_type,the_params);
}

//---------------------------------------------------------------

void	StopWave(SLONG ref,SLONG wave_id)
{
//	the_qs_sound_manager.StopWave(ref,wave_id);
}

void	LoadWaveList(CBYTE *wave_path,CBYTE *wave_list)
{
}

//---------------------------------------------------------------

void	LoadWave(CBYTE *wave_name)
{
}

//---------------------------------------------------------------

void	FreeWaveList(void)
{
}

void	SetListenerPosition(SLONG x,SLONG y,SLONG z,SLONG scale)
{
}

void	SetListenerOrientation(SLONG angle,SLONG roll,SLONG tilt)
{
}
