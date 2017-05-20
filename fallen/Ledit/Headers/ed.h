//
// Light editor.
//

#ifndef _ED_
#define _ED_

#define	LIGHT_FLAGS_INSIDE	(1)

typedef struct
{
	UBYTE range;
	SBYTE red;
	SBYTE green;
	SBYTE blue;
	UBYTE next;
	UBYTE used;
	UBYTE flags;
	UBYTE padding;
//	UWORD padding;
	SLONG x;
	SLONG y;
	SLONG z;
	
} ED_Light;

#define ED_MAX_LIGHTS 256

extern ED_Light ED_light[ED_MAX_LIGHTS];
extern SLONG    ED_light_free;


//
// Clears all the light info.
//

void ED_init(void);


//
// Creates a new light.
//
// Range goes from 1 - 255
// (R,G,B) are signed values (+/- 127)
//

SLONG ED_create(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue);

//
// Deletes one light or all the lights.
// 

void ED_delete    (SLONG light);
void ED_delete_all(void);

//
// Updates the given light.
//

void ED_light_move(
		SLONG light,
		SLONG x,
		SLONG y,
		SLONG z);

void ED_light_change(
		SLONG light,
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue);

//
// Get/Set the ambient light colour.  Values go from 0 - 255
//

void ED_amb_get(
		SLONG *red,
		SLONG *green,
		SLONG *blue);

void ED_amb_set(
		SLONG red,
		SLONG green,
		SLONG blue);

//
// Do we have light under the lamposts? Is it nightime?
// 

SLONG ED_lampost_on_get(void);
void  ED_lampost_on_set(SLONG lamposts_are_on);

SLONG ED_night_get(void);
void  ED_night_set(SLONG its_night_time);

SLONG ED_darken_bottoms_on_get(void);
void  ED_darken_bottoms_on_set(SLONG darken_bottoms_on);


//
// Get/Set the lights under lamposts. 
//
// Range goes from 1 - 255
// (R,G,B) are signed values (+/- 127)
//
//

void ED_lampost_get(
		SLONG *range,
		SLONG *red,
		SLONG *green,
		SLONG *blue);

void ED_lampost_set(
		SLONG range,
		SLONG red,
		SLONG green,
		SLONG blue);

//
// Get/Set the sky light colour.  Values go from 0 - 255
//

void ED_sky_get(
		SLONG *red,
		SLONG *green,
		SLONG *blue);

void ED_sky_set(
		SLONG red,
		SLONG green,
		SLONG blue);



//
// Undo / Redo functions.
//

void ED_undo_store(void);	// Remember the current state.

void ED_undo_undo(void);
void ED_undo_redo(void);

SLONG ED_undo_undo_valid(void);
SLONG ED_undo_redo_valid(void);



//
// Loading and saving. Return TRUE on success.
//

SLONG ED_load(CBYTE *name);
SLONG ED_save(CBYTE *name);

//
// The format that everything is saved out in.
//
//  SLONG        sizeof(ED_Light)
//  SLONG        ED_MAX_LIGHTS
//  SLONG        sizeof(NIGHT_Colour)
//
//	ED_Light     ed_light[ED_MAX_LIGHTS];
//	SLONG        ed_light_free;
//	ULONG        night_flag;
//	ULONG        night_amb_d3d_colour;
//	ULONG        night_amb_d3d_specular;
//	SLONG        night_amb_red;
//	SLONG        night_amb_green;
//	SLONG        night_amb_blue;
//  SBYTE        night_lampost_red;
//  SBYTE        night_lampost_green;
//  SBYTE        night_lampost_blue;
//  UBYTE        padding;
//  SLONG        night_lampost_radius;
//	NIGHT_Colour night_sky_colour;
//



#endif





