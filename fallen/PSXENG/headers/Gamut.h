//
// Gamut calculations of polygons.
//

#ifndef _NGAMUT_
#define _NGAMUT_


//
// The size of the gamut mapwho.
//

#define NGAMUT_SIZE 128
#define NGAMUT_SIZE_LO	32


//
// The gamut for squares.
//

typedef struct
{
	UBYTE xmin;
	UBYTE xmax;

} NGAMUT_Gamut;

typedef struct
{
	UBYTE zmin;
	UBYTE zmax;

} NGAMUT_Gamut2;

extern	NGAMUT_Gamut2	NGAMUT_gamut2[NGAMUT_SIZE];
extern NGAMUT_Gamut NGAMUT_gamut[NGAMUT_SIZE];
extern SLONG        NGAMUT_zmin;
extern SLONG        NGAMUT_zmax;
extern SLONG        NGAMUT_Ymin;
extern SLONG        NGAMUT_Ymax;
extern SLONG			NGAMUT_xmin,NGAMUT_xmax;

//
// Function to work out a gamut.
//
// Initialises the gamut.
// Pushes out the gamut along the line. The line can go off the gamut mapwho, but
// the gamut always stays within the gamut mapwho.
//

void NGAMUT_init    (void);
void NGAMUT_add_line(
		SLONG px1,
		SLONG pz1,
		SLONG px2,
		SLONG pz2);

//
// Works out a square gamut of the given radius centered at (x,z)
//

void NGAMUT_view_square(SLONG mid_x, SLONG mid_z, SLONG radius);


//
// Once you've worked out the gamut for squares, this function works
// out the gamut for points.
//

extern NGAMUT_Gamut NGAMUT_point_gamut[NGAMUT_SIZE];
extern SLONG        NGAMUT_point_zmin;
extern SLONG        NGAMUT_point_zmax;

void NGAMUT_calculate_point_gamut(void);

									 //
// This is a lower-res gamut. It is guaranteed to encompass all the
// squares in the point gamut.  It is calculated from the out gamut, so
// make sure you have calculated that one already.
//

extern NGAMUT_Gamut NGAMUT_lo_gamut[NGAMUT_SIZE_LO];
extern SLONG        NGAMUT_lo_zmin;
extern SLONG        NGAMUT_lo_zmax;

void NGAMUT_calculate_lo_gamut(void);


#endif
