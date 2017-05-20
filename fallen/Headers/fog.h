//
// Fog!
//

#ifndef FOG_H
#define FOG_H




//
// Gets rid of all the fog info and starts afresh.
//

void FOG_init(void);


//
// Sets the area of focus.  Fog only exists within the area
// of focus.
//

void FOG_set_focus(
		SLONG x,
		SLONG z,
		SLONG radius);


//
// Makes the fog react to a gust of wind. The gust happens
// at (x1,z1).  (x2,z2) gives strength and direction.
//

void FOG_gust(
		SLONG x1, SLONG z1,
		SLONG x2, SLONG z2);


//
// Process the fog.
//

void FOG_process(void);


//
// The fog module tells you how to draw each bit of fog.
//

#define FOG_TYPE_TRANS1	 0	// Very transparent.
#define FOG_TYPE_TRANS2	 1
#define FOG_TYPE_TRANS3	 2
#define FOG_TYPE_TRANS4	 3	// Very opaque.
#define FOG_TYPE_NO_MORE 4	// There is no more fog to draw.
#define FOG_TYPE_UNUSED  4

typedef struct
{
	UBYTE type;
	UBYTE trans;	// Transparency. 0 => more transparent.
	UWORD size;		// Radius.
	UWORD yaw;
	UWORD shit;
	SLONG x;
	SLONG y;
	SLONG z;

} FOG_Info;

void     FOG_get_start(void);
FOG_Info FOG_get_info (void);


#endif
