//
// Dreamcast low-level functions, ripped from the example docs.
//

#ifndef _DCLL_
#define _DCLL_




//
// A dreamcast sound buffer...
//

typedef struct dcll_sound DCLL_Sound;



//
// Initialises the DC sound system.
//

void DCLL_init(void);




// ========================================================
//
// PLAYING SFX
//
// ========================================================

//
// Loads a sound from a file.
//

DCLL_Sound *DCLL_load_sound(CBYTE *fname);


// Call this when a bunch of sounds have finished loading.
// Can be called whenever you like really.
void DCLL_ProbablyDoneMostOfMySoundLoadingForAWhile ( void );


//
// Sets the volume of the sample. 0.0F <= volume <= 1.0F
//

void DCLL_set_volume(DCLL_Sound *ds, float volume);



//
// Plays a 2D sound.
//

#define DCLL_FLAG_INTERRUPT (1 << 0)
#define DCLL_FLAG_LOOP      (1 << 1)

void DCLL_2d_play_sound(DCLL_Sound *ds, SLONG flag = 0);


//
// Plays a 3D sound.
//

void DCLL_3d_play_sound(DCLL_Sound *ds, float x, float y, float z, SLONG flag = 0);


//
// The 'head' for 3d sounds.
//

void DCLL_3d_set_listener(
		float x,
		float y,
		float z,
		float matrix[9]);

//
// Stop the sound from playing (2D or 3D)
// 

void DCLL_stop_sound(DCLL_Sound *ds);


//
// Frees up the given sound.
//

void DCLL_free_sound(DCLL_Sound *ds);


//
// Shuts down the sound system.
//

void DCLL_fini(void);



// ========================================================
//
// STREAMING FILES FROM CD
//
// ========================================================

//
// Sets the range of DCLL_stream_volume.
//

void DCLL_stream_set_volume_range(float max_vol);	// 0.0F to 1.0F


SLONG DCLL_stream_play(CBYTE *fname, SLONG loop = FALSE);	// Play the file streaming off CD. Looping samples have a lower priority. Returns FALSE if it doesn't issue the play.
void  DCLL_stream_wait(void);								// Wait until the streaming file has finished playing
void  DCLL_stream_stop(void);								// Stop the streaming sound.
SLONG DCLL_stream_is_playing(void);							// Returns TRUE if the streaming sound is still playing.
void  DCLL_stream_volume(float volume);						// 0.0F <= volume <= 1.0F


// ========================================================
//
// STREAMING FILES FROM MEMORY
//
// ========================================================

void DCLL_memstream_load  (CBYTE *fname);					// Loads the file.
void DCLL_memstream_volume(float volume);					// 0.0F <= volume <= 1.0F
void DCLL_memstream_play  (void);							// Loops and plays the file
void DCLL_memstream_stop  (void);							// Stops playing the file
void DCLL_memstream_unload(void);							// Frees memory.




#ifdef DEBUG

void DumpTracies ( void );

#else

static void DumpTracies ( void ){}

#endif


#endif
