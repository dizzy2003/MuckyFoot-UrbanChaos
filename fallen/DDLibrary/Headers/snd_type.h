// snd_type.h

#undef NO_SOUND
#undef A3D_SOUND
#undef Q_SOUND
#undef M_SOUND
#undef DS_SOUND

#ifdef TARGET_DC
// No sound for the moment
//#define NO_SOUND	1
#define DC_SOUND 1
#else
// My sound doesn't seem to be working. Must be linking with the wron libs or something.
#define	NO_SOUND	1
//#define	A3D_SOUND	1
//#define	Q_SOUND		1
//#define	M_SOUND		1
#endif
