#include "game.h"
#include "dclowlevel.h"
#include "sound_id.h"
#include "matrix.h"
#include "mfx.h"




#ifdef DEBUG
#define SHARON TRACE
#else
#define SHARON sizeof
#endif


// So that we can disable sounds during pause.
extern SLONG GAMEMENU_menu_type;
#define GAMEMENU_MENU_TYPE_PAUSE 1


//
// All our sounds. 'copy_of' is the index of the MFX_DC_Sound that
// 'own's' the DCLL_Sound *ds (the one that allocated it first)
//

typedef struct
{
	Thing      *thing;
	UWORD       channel_id;
	UWORD       copy_of;		// copy_of == the index of this sound if it's not a copy of another sound.
	DCLL_Sound *ds;

} MFX_DC_Sound;

#define MFX_DC_MAX_SOUNDS 1024

MFX_DC_Sound MFX_DC_sound[MFX_DC_MAX_SOUNDS];


//
// The amount of sound memory needed.
//

SLONG MFX_DC_sound_memory;
SLONG MFX_DC_11khz_saving;
SLONG MFX_DC_num_duplicates;
SLONG MFX_DC_duplicate_saving;
SLONG MFX_DC_large_file_saving;


extern SLONG DCLL_bytes_of_sound_memory_used;


void MFX_DC_init(void)
{
	SLONG i;

	SLONG biggest_sample_index = 0;
	SLONG biggest_sample_size  = 0;

	CBYTE fname[256];

	SHARON ( "Entered MFX_DC_init\n" );


	//
	// Initialise the DC sound system.
	//

	DCLL_init();
	
	//
	// Load all the sound fx.
	//


#define GUESS_HOW_MANY_SOUNDS_TO_LOAD 548
#define HOW_MUCH_COMPLETION_BAR_WE_GET 160
#define HOW_MANY_UPDATES 20
	int iNumWavsLoaded = 0;
	int iNumWavsToDoNextChunk = GUESS_HOW_MANY_SOUNDS_TO_LOAD / HOW_MANY_UPDATES;
	int iCurChunkVal = 0;


	MFX_DC_sound_memory = 0;

	for (i = 0; sound_list[i][0] != '!'; i++)
	{
		ASSERT(WITHIN(i, 0, MFX_DC_MAX_SOUNDS - 1));

		iNumWavsLoaded++;
		if ( iNumWavsLoaded > iNumWavsToDoNextChunk )
		{
			// Do an update of the status bar.
			iNumWavsToDoNextChunk += GUESS_HOW_MANY_SOUNDS_TO_LOAD / HOW_MANY_UPDATES;
			iCurChunkVal += HOW_MUCH_COMPLETION_BAR_WE_GET / HOW_MANY_UPDATES;
extern void ATTRACT_loadscreen_draw(SLONG completion);
			ATTRACT_loadscreen_draw ( iCurChunkVal );
		}


		if (i > 0)
		{
			//
			// If this name is too similar to the previous filename, assume
			// the sounds are similar enough so that we can just use the
			// previous sound...
			//

			CBYTE *ch1 = sound_list[i - 0];
			CBYTE *ch2 = sound_list[i - 1];

			SLONG differences = 0;

			while(1)
			{
				if (*ch1 == '.' && *ch2 == '.')
				{
					break;
				}

				if (*ch1 != *ch2)
				{
					differences += 1;

					if (differences > 2)
					{
						break;
					}
				}

				if (*ch1 != '.') ch1++;
				if (*ch2 != '.') ch2++;
			}

			if (differences <= 2)
			{
				//
				// Similar enough...
				//

				MFX_DC_sound[i].ds      = MFX_DC_sound[i - 1].ds;
				MFX_DC_sound[i].copy_of = MFX_DC_sound[i - 1].copy_of;

				MFX_DC_num_duplicates   += 1;
				MFX_DC_duplicate_saving += DCLL_bytes_of_sound_memory_used;

				continue;
			}
		}

		#ifdef TARGET_DC	
		sprintf(fname, "Data\\Sfx\\1622DC\\%s", sound_list[i]);
		#else
		sprintf(fname, "Data\\Sfx\\1622\\%s", sound_list[i]);
		#endif

		SHARON ( "Loading sound <%s>\n", fname );

		MFX_DC_sound[i].ds      = DCLL_load_sound(fname);
		MFX_DC_sound[i].copy_of = i;

		if (DCLL_bytes_of_sound_memory_used > 128 * 1024)
		{
			//
			// This sound won't work on the DC!
			//

			SHARON("WONT WORK ON DC! - \"%s\"\n", fname);
		}

		if (DCLL_bytes_of_sound_memory_used > 300 * 1024)
		{
			ASSERT(0);

			//
			// Dont load any samples that are too big...
			//

			DCLL_free_sound(MFX_DC_sound[i].ds);

			MFX_DC_sound[i].ds = NULL;

			{
				CBYTE str[500];

				sprintf(str, "Not loading huge sound file (%4d k) \"%s\"\n", DCLL_bytes_of_sound_memory_used >> 10, sound_list[i]);
				ASSERT ( FALSE );
				SHARON ( str );
			}

			MFX_DC_large_file_saving       += DCLL_bytes_of_sound_memory_used;
			DCLL_bytes_of_sound_memory_used = 0;
		}
		else
		{
			SLONG j;

			CBYTE *words_for_11khz[] =
			{
				"Suspension",
				"Scrape",
				"Balrog",
				"Footstep",
				"PAIN",
				"TAUNT",
				"darci\\",
				"roper\\"
				"bthug1\\",
				"wthug1\\",
				"wthug2\\",
				"cop\\COP",
				"misc\\",
				"Barrel",
				"Ambience",
				"CarX",
				"!"
			};

			//
			// Shall we pretend this file is 11025 khz?
			//

			for (j = 0; words_for_11khz[j][0] != '!'; j++)
			{
				if (strstr(sound_list[i], words_for_11khz[j]))
				{
					DCLL_bytes_of_sound_memory_used >>= 1;
					MFX_DC_11khz_saving              += DCLL_bytes_of_sound_memory_used;

					break;
				}
			}

			MFX_DC_sound_memory += DCLL_bytes_of_sound_memory_used;
		}

		if (DCLL_bytes_of_sound_memory_used > biggest_sample_size)
		{
			biggest_sample_size  = DCLL_bytes_of_sound_memory_used;
			biggest_sample_index = i;
		}
	}

	SHARON ( "Done MFX_DC_init\n" );

}

//
// Stops all sounds with the given channel_id (apart from the given wave)
//

void MFX_stop_channel_id(UWORD channel_id, ULONG wave)
{
	SLONG i;

	MFX_DC_Sound *ms;

	if (channel_id == 0)
	{
		return;
	}

	for (i = 0; i < MFX_DC_MAX_SOUNDS; i++)
	{
		ms = &MFX_DC_sound[i];

		if (ms->channel_id == channel_id && (ULONG)i != wave)
		{
			ms->thing      = NULL;
			ms->channel_id = 0;

			DCLL_stop_sound(ms->ds);
		}
	}
}

void MFX_free_sound(SLONG wave)
{
	MFX_DC_Sound *ms;

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	if (ms->ds)
	{	
		DCLL_free_sound(ms->ds);

		ms->ds         = NULL;
		ms->channel_id = 0;
		ms->thing      = 0;

		//
		// If any sounds are copies of this sound, we must free them too.
		//

		SLONG i;
		
		for (i = 0; sound_list[i][0] != '!'; i++)
		{
			ASSERT(WITHIN(i, 0, MFX_DC_MAX_SOUNDS - 1));

			if (i == wave)
			{
				continue;
			}

			if (MFX_DC_sound[i].copy_of == wave)
			{
				MFX_DC_sound[i].ds = NULL;
			}
		}
	}
}

void MFX_load_sound(SLONG wave)
{
	MFX_DC_Sound *ms;

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	if (ms->ds)
	{
		//
		// Already loaded!
		//

		return;
	}
	else
	{
		//
		// If this sound is a copy of another one, just use the copy...
		//

		if (ms->copy_of && ms->copy_of != wave)
		{
			ASSERT(WITHIN(ms->copy_of, 0, MFX_DC_MAX_SOUNDS - 1));

			MFX_DC_Sound *ms_copy = &MFX_DC_sound[ms->copy_of];

			if (ms_copy->ds)
			{
				ms->ds = ms_copy->ds;

				return;
			}
		}

		CBYTE fname[256];

		#ifdef TARGET_DC	
		sprintf(fname, "Data\\Sfx\\1622DC\\%s", sound_list[wave]);
		#else
		sprintf(fname, "Data\\Sfx\\1622\\%s", sound_list[wave]);
		#endif

		ms->ds = DCLL_load_sound(fname);

		if (ms->copy_of && ms->copy_of != wave)
		{
			//
			// Make sure the 'owner' of this sound has a copy of the sound too...
			//

			ASSERT(WITHIN(ms->copy_of, 0, MFX_DC_MAX_SOUNDS - 1));

			MFX_DC_Sound *ms_copy = &MFX_DC_sound[ms->copy_of];

			ASSERT(ms_copy->ds == NULL);

			ms_copy->ds = ms->ds;
		}
	}
}



// ========================================================
//
// IMPLEMENT THE MUCKYFOOT SOUND SYSTEM
//
// ========================================================

SLONG MFX_DC_vol_fx  = 127;	// From 0 to 127
SLONG MFX_DC_vol_amb = 127;	// From 0 to 127
SLONG MFX_DC_vol_mus = 127;	// From 0 to 127

SLONG MFX_DC_listener_x;
SLONG MFX_DC_listener_y;
SLONG MFX_DC_listener_z;

void MFX_get_volumes(SLONG *fx, SLONG *amb, SLONG *mus)
{
	*fx  = MFX_DC_vol_fx;
	*amb = MFX_DC_vol_amb;
	*mus = MFX_DC_vol_mus;
}

void MFX_set_volumes(SLONG fx, SLONG amb, SLONG mus)
{
	SATURATE(fx,  0, 127);
	SATURATE(amb, 0, 127);
	SATURATE(mus, 0, 127);

	MFX_DC_vol_fx  = fx;
	MFX_DC_vol_amb = amb;
	MFX_DC_vol_mus = mus;
	
	DCLL_stream_set_volume_range(float(mus) * (1.0F / 127.0F));

	//
	// Set the volumes of the various DCLL_sounds...?
	//

	// ...
}

extern	SLONG	DCLL_still_playing(DCLL_Sound *ds);

void MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z)
{
	MFX_DC_Sound *ms;

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return;
	}

	SLONG dx = abs(x - MFX_DC_listener_x);
	SLONG dy = abs(y - MFX_DC_listener_y);
	SLONG dz = abs(z - MFX_DC_listener_z);

	SLONG dist = dx + dy + dz >> 8;
	SLONG vol  = 256 - (256 * dist >> 13);

	if (vol <= 0)
	{
//		if(flags & MFX_LOOPED)
//			vol=1;   //play it quiet because we might be going to fade it in etc
//		else
			return;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	if(ms->channel_id==channel_id) //MikeD
	{
		if (!(flags & MFX_REPLACE))
		{
			//
			// not replace so dont allow this sound
			//
			if(DCLL_still_playing(ms->ds))
				return;
		}
	}

	ms->thing      = NULL;
	ms->channel_id = channel_id;


	DCLL_set_volume(ms->ds, float(MFX_DC_vol_fx * vol >> 8) * (1.0F / 127.0F));
	DCLL_2d_play_sound(ms->ds, ((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));

	/*

	DCLL_3d_play_sound(
		ms->ds,
		float(x >> 8),
		float(y >> 8),
		float(z >> 8),
		((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));

	*/
}




void MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p)
{
	MFX_DC_Sound *ms;

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return;
	}

	SLONG dx = abs(p->WorldPos.X - MFX_DC_listener_x);
	SLONG dy = abs(p->WorldPos.Y - MFX_DC_listener_y);
	SLONG dz = abs(p->WorldPos.Z - MFX_DC_listener_z);

	SLONG dist = dx + dy + dz >> 8;
	SLONG vol  = 256 - (256 * dist >> 13);

	if (vol <= 0  )
	{
//		if(flags & MFX_LOOPED)
//			vol=1;   //play it quiet because we might be going to fade it in etc
//		else
			return;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	DCLL_set_volume(ms->ds, float(MFX_DC_vol_fx * vol >> 8) * (1.0F / 127.0F));
	if(ms->channel_id==channel_id && ms->thing==p) //MikeD
	{
		if (!(flags & MFX_REPLACE))
		{
			//
			// not replace so dont allow this sound
			//
			if(DCLL_still_playing(ms->ds))
				return;
		}
	}


	ms->thing      = p;
	ms->channel_id = channel_id;


	DCLL_2d_play_sound(ms->ds, ((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));

	/*

	DCLL_3d_play_sound(
		ms->ds,
		float(p->WorldPos.X >> 8),
		float(p->WorldPos.Y >> 8),
		float(p->WorldPos.Z >> 8),
		((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));

	*/
}

void MFX_play_at_stream_volume(UWORD channel_id, ULONG wave, ULONG flags)
{
	MFX_DC_Sound *ms;

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	ms->thing      = NULL;
	ms->channel_id = channel_id;

	//
	// Ambient => 2D...
	//

	DCLL_set_volume(ms->ds, float(MFX_DC_vol_mus) * (1.0F / 127.0F));

	DCLL_2d_play_sound(ms->ds, ((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));
}



void MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags)
{
	MFX_DC_Sound *ms;

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	ms->thing      = NULL;
	ms->channel_id = channel_id;

	//
	// Ambient => 2D...
	//

	DCLL_set_volume(ms->ds, float(MFX_DC_vol_amb) * (1.0F / 127.0F));

	DCLL_2d_play_sound(ms->ds, ((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));
}

UBYTE MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags)
{
	//
	// Stereo => Same as ambient but at FX volume????
	//

	MFX_DC_Sound *ms;

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return 0;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	ms->thing      = NULL;
	ms->channel_id = channel_id;

	//
	// Ambient => 2D...
	//

	DCLL_set_volume(ms->ds, float(MFX_DC_vol_fx) * (1.0F / 127.0F));

	DCLL_2d_play_sound(ms->ds, ((flags & MFX_QUEUED) ? 0 : DCLL_FLAG_INTERRUPT) | ((flags & MFX_LOOPED) ? DCLL_FLAG_LOOP : 0));

	return 0;
}

void MFX_stop(SLONG channel_id, ULONG wave)
{
	SLONG i;

	MFX_DC_Sound *ms;

	if ( ( wave == MFX_WAVE_ALL ) || ( wave == MFX_WAVE_ALMOST_ALL ) )
	{
		for (i = 0; i < MFX_DC_MAX_SOUNDS - 1; i++)
		{
			ms = &MFX_DC_sound[i];

			ms->thing      = NULL;
			ms->channel_id = 0;

			DCLL_stop_sound(ms->ds);
		}

		// And stop the streaming sound.
		DCLL_stream_stop();

		if ( wave != MFX_WAVE_ALMOST_ALL )
		{
			DCLL_memstream_stop();
		}

		return;
	}

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	ms->thing = NULL;

	DCLL_stop_sound(ms->ds);
}

void MFX_stop_attached(Thing *p)
{
	SLONG i;

	MFX_DC_Sound *ms;

	//
	// Look for the sound attached to this thing.
	//

	for (i = 0; i < MFX_DC_MAX_SOUNDS; i++)
	{
		ms = &MFX_DC_sound[i];

		if (ms->thing == p)
		{
			ms->thing      = NULL;
			ms->channel_id = 0;

			DCLL_stop_sound(ms->ds);

			return;
		}
	}
}


//
// Ignore these for now!
//

void MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend)
{
}

void MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain)
{
	MFX_DC_Sound *ms;

	ASSERT(WITHIN(wave, 0, MFX_DC_MAX_SOUNDS - 1));

	ms = &MFX_DC_sound[wave];

	DCLL_set_volume(ms->ds, float(gain) * (1.0F / 255.0F));	
}

void MFX_set_queue_gain(UWORD channel_id, ULONG wave, UBYTE gain)
{
}


void MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch)
{
	float matrix[9];
	float rad_yaw;
	float rad_pitch;
	float rad_roll;

	MFX_DC_listener_x = x;
	MFX_DC_listener_y = y;
	MFX_DC_listener_z = z;

	rad_yaw   = float(heading) * 2.0F * PI / 2048.0F;
	rad_pitch = float(roll   ) * 2.0F * PI / 2048.0F;
	rad_roll  = float(pitch  ) * 2.0F * PI / 2048.0F;

	MATRIX_calc(
		matrix,
		rad_yaw,
		rad_pitch,
		rad_roll);

	DCLL_3d_set_listener(
		float(x >> 8),
		float(y >> 8),
		float(z >> 8),
		matrix);
}


void MFX_load_wave_list(CBYTE *names[])
{
}

void MFX_free_wave_list()
{
}


UWORD MFX_get_wave(UWORD channel_id, UBYTE index)
{
	return 0;
}

void MFX_render()
{
}


//
// Streaming sound stuff...
//

SLONG MFX_QUICK_play_id = 1;
SLONG last_started=0;

SLONG MFX_QUICK_play(CBYTE *str, SLONG loop, SLONG x, SLONG y, SLONG z)
{

	if ( GAMEMENU_menu_type == GAMEMENU_MENU_TYPE_PAUSE )
	{
		// Don't play sounds during pause.
		return NULL;
	}

	if (DCLL_stream_play(str, loop))
	{
		last_started=timeGetTime();

		if (!loop)
		{
			// Assume it's speech.
			DCLL_stream_volume( (float)MFX_DC_vol_fx * ( 1.0f / 127.0f ) );
		}
		else
		{
			// Assume it's music.
			DCLL_stream_volume( (float)MFX_DC_vol_mus * ( 1.0f / 127.0f ) );
		}

		return ++MFX_QUICK_play_id;
	}
	else
	{
		return NULL;
	}
}

void MFX_QUICK_wait()
{
	DCLL_stream_wait();
}

void MFX_QUICK_stop ( bool bAllowMemstream )
{
	DCLL_stream_stop();
	if ( !bAllowMemstream )
	{
		DCLL_memstream_stop();
	}
//	ASSERT(!DCLL_stream_is_playing());
}

SLONG MFX_QUICK_still_playing()
{
	SLONG	ret;

	ret=DCLL_stream_is_playing();
	if(ret==0)
	{
		//
		// It's stopped or maybe it never started
		//
		if((timeGetTime()-last_started)<8000) //8 seconds
		{
			ret=1;
		}
	}
	else
		last_started=0; //it has definatly started so dont let the 8 second delay thing happen
	return(ret);
}

