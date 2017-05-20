#ifndef		LIGHTG_H
#define		LIGHTG_H

//
// Do we want coloured lights?
//

#define LIGHT_COLOURED 1


//
// A structure that describes a light.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	UBYTE red;
	UBYTE green;
	UBYTE blue;
	UBYTE range;
	UBYTE type;
	UBYTE param;
	UWORD dummy;

} LIGHT_Save;



//
// A light colour.
//

#if LIGHT_COLOURED

typedef struct
{
	UBYTE red;
	UBYTE green;
	UBYTE blue;

} LIGHT_Colour;

#else

typedef UBYTE LIGHT_Colour;

#endif

//
// UBYTE values of lights go from 0 to LIGHT_MAX_BRIGHT, all values above
// LIGHT_MAX_BRIGHT are at maximum brightness.
//

#define LIGHT_MAX_BRIGHT 64

//
// The maximum range of the light in world coords.
//

#define LIGHT_MAX_RANGE 0x600


//
// So the light module can work with any sort of height field.
// The LIGHT_Map you pass can be on the stack because the
// function takes a copy of it.
//

typedef struct
{
	SLONG width;
	SLONG height;

	SLONG        (*get_height)(SLONG x, SLONG z);	// 0 <= x < width, 0 <= z < height
	LIGHT_Colour (*get_light) (SLONG x, SLONG z);
	void         (*set_light) (SLONG x, SLONG z, LIGHT_Colour light);

} LIGHT_Map;

void LIGHT_set_hf(LIGHT_Map *map);


//
// Sets the ambient light used. The normal of the light should have
// a length of 255.
//

extern LIGHT_Colour LIGHT_amb_colour; // DONT SET THESE VALUES, USE THE FUNCTION BELOW!
extern SLONG        LIGHT_amb_norm_x;
extern SLONG        LIGHT_amb_norm_y;
extern SLONG        LIGHT_amb_norm_z;

void LIGHT_set_ambient(
		LIGHT_Colour amb_colour,
		SLONG        amb_norm_x,
		SLONG        amb_norm_y,
		SLONG        amb_norm_z);

//
// Removes all lights and removes all cached light info.
//

void LIGHT_init(void);


//
// Where building-point lighting goes.
//

extern LIGHT_Colour LIGHT_building_point[RMAX_PRIM_POINTS];


//
// Recalcs all lighting on the current height-field and puts values in the
// LIGHT_building_point[] array for the lights on the building prim points.
//

void LIGHT_recalc_hf(void);




//
// Lights...
//

typedef UWORD LIGHT_Index;


//
// Creates a new light. The meaning of 'param' depends on
// the type of light.
//

#define LIGHT_TYPE_NORMAL		1
#define LIGHT_TYPE_BROKEN		2	// param is how often the light flickers off.
#define LIGHT_TYPE_PULSE		3	// param is the speed of flashing.
#define LIGHT_TYPE_FADE			4	// param is how long the light lasts.

LIGHT_Index LIGHT_create(
				GameCoord    where,
				LIGHT_Colour colour,
				UBYTE        range,
				UBYTE        type,
				UBYTE        param);

void LIGHT_destroy(LIGHT_Index l_index);

//
// Gets/sets a light's position.
//

GameCoord LIGHT_pos_get(LIGHT_Index l_index);
void      LIGHT_pos_set(LIGHT_Index l_index, GameCoord pos);


//
// Processes all the lights.
//

void LIGHT_process(void);



//
// Returns a value that identifies the lighting context of the given thing.
// If the lighting context changes then that thing needs to be re-lit. The
// position and orientation of the actual thing is not taken into account
// in the context. You have to check for that yourself.
//

SLONG LIGHT_get_context(THING_INDEX t_index);


//
// Returns the colour of light at the given point. It only uses light found
// in the last call to LIGHT_get_context.
//

LIGHT_Colour LIGHT_get_point(SLONG x, SLONG y, SLONG z);


//
// Lights the given prim. It puts the light values in the LIGHT_point_colour array.
// Uses the prim_normal array... make sure you've called calc_prim_normals();
//

void LIGHT_prim            (THING_INDEX t_index);
void LIGHT_prim_use_normals(THING_INDEX t_index);

//
// Lights a building. It puts the light values in the LIGHT_point_colour array.
//

void LIGHT_building_use_normals(THING_INDEX t_index);

//
// The array in which the colours of the points will be returned.
//

#define LIGHT_MAX_POINTS 2560

extern LIGHT_Colour LIGHT_point_colour[LIGHT_MAX_POINTS];
extern SLONG        LIGHT_point_colour_upto;



//
// Returns a D3D colour from a LIGHT_Colour.
//

#if LIGHT_COLOURED
inline void LIGHT_get_d3d_colour(LIGHT_Colour col, ULONG *colour, ULONG *specular)
{
	SLONG red   = col.red;
	SLONG green = col.green;
	SLONG blue  = col.blue;

	SLONG wred   = 0;
	SLONG wgreen = 0;
	SLONG wblue  = 0;

	red   *= (256 / LIGHT_MAX_BRIGHT);
	green *= (256 / LIGHT_MAX_BRIGHT);
	blue  *= (256 / LIGHT_MAX_BRIGHT);

	if (red   > 255) {wred   = red   - 255 >> 1; red   = 255; if (wred   > 255) {wred   = 255;}}
	if (green > 255) {wgreen = green - 255 >> 1; green = 255; if (wgreen > 255) {wgreen = 255;}}
	if (blue  > 255) {wblue  = blue  - 255 >> 1; blue  = 255; if (wblue  > 255) {wblue  = 255;}}

   *colour   = (red  << 16) | (green  << 8) | (blue  << 0);
   *specular = (wred << 16) | (wgreen << 8) | (wblue << 0);
}
inline ULONG LIGHT_get_glide_colour(LIGHT_Colour col)
{
	SLONG red   = col.red;
	SLONG green = col.green;
	SLONG blue  = col.blue;
	SLONG alpha = 0;

	red   *= (256 / LIGHT_MAX_BRIGHT);
	green *= (256 / LIGHT_MAX_BRIGHT);
	blue  *= (256 / LIGHT_MAX_BRIGHT);

	if (red   > 255) {alpha += red   - 255; red   = 255;}
	if (green > 255) {alpha += green - 255; green = 255;}
	if (blue  > 255) {alpha += blue  - 255; blue  = 255;}
	if (alpha > 255) {alpha = 255;}

    return (alpha << 24) | (red  << 16) | (green  << 8) | (blue  << 0);
}

#else
inline void LIGHT_get_d3d_colour(LIGHT_Colour col, ULONG *colour, ULONG *specular)
{
	SLONG whiteout;
	SLONG bright;

	bright = col * (256 / LIGHT_MAX_BRIGHT);

	//
	// Once we go over fully-bright, we get into saturated colour.
	//

	if (bright > 255)
	{
		whiteout = bright - 255 >> 1;
		bright   = 255;

		if (whiteout > 255) {whiteout = 255;}
	}
	else
	{
		whiteout = 0;
	}

   *colour   = (bright   << 0) | (bright   << 8) | (bright   << 16);
   *specular = (whiteout << 0) | (whiteout << 8) | (whiteout << 16);
}
#endif


#endif









