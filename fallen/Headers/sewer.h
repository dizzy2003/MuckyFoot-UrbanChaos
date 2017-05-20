//
// Sewers.. yuck!
//

#ifndef _SEWER_
#define _SEWER_


//
// The size of the sewage system.
// How deep the sewers are.
//

#define SEWER_SIZE	128
#define SEWER_DEPTH 0x100


//
// How to create the sewage system.
//

#define SEWER_EDGE_XS	0
#define SEWER_EDGE_XL	1
#define SEWER_EDGE_ZS	2
#define SEWER_EDGE_ZL	3

void SEWER_init     (void);
void SEWER_square_on(SLONG x, SLONG z);
void SEWER_ladder_on(SLONG x, SLONG z, SLONG edge);
void SEWER_pillar_on(SLONG x, SLONG z);
void SEWER_precalc  (void);
void SEWER_save     (CBYTE *filename);
void SEWER_load     (CBYTE *filename);


//
// Returns true if the given square is an entrance into the
// sewage system.
//

SLONG SEWER_can_i_enter(UBYTE x, UBYTE z);


//
// Inserts colvects aroung the sewer system.
// Removes sewer colvects from the map.
//

void SEWER_colvects_insert(void);
void SEWER_colvects_remove(void);


//
// Returns the height of the sewers at (x,z).
//

SLONG SEWER_calc_height_at(SLONG x, SLONG z);



//
// How to draw a square of the sewage system.
//

#define SEWER_PAGE_FLOOR	0
#define SEWER_PAGE_WALL		1
#define SEWER_PAGE_PILLAR	2
#define SEWER_PAGE_WATER	3
#define SEWER_PAGE_NUMBER	4

typedef struct
{
	SLONG x[4];
	SLONG y[4];
	SLONG z[4];
	SLONG u[4];	// 16-bit fixed point from 0.0 to 256.0
	SLONG v[4];	// 16-bit fixed point from 0.0 to 256.0
	ULONG c[4];

	UBYTE page;	// The sewer page... NOT the texturepage!
	
} SEWER_Face;

void        SEWER_get_start(SLONG x, SLONG z);
SEWER_Face *SEWER_get_next (void);	// NULL => there are no more faces.
SEWER_Face *SEWER_get_water(void);	// Returns the water faces...


#endif
