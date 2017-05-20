//
// Sofware renderer hacked in!
//

#ifndef _SW_
#define _SW_


//
// The maximum software screen dimensions.
//

#define SW_MAX_WIDTH  640
#define SW_MAX_HEIGHT 480

extern ULONG *SW_buffer;

#ifndef TARGET_DC

//
// Reloads all textures. Looks at the D3D Texture system and
// loads a copy of each texture.
//

void SW_reload_textures(void);


//
// Initialises a new frame and clears the SW_buffer.
//

void SW_init(
		SLONG width,
		SLONG height);


//
// Sets the way each page is drawn.
//

#define SW_PAGE_IGNORE   0	// This page is not drawn.
#define SW_PAGE_NORMAL   1
#define SW_PAGE_MASKED   2
#define SW_PAGE_ALPHA    3
#define SW_PAGE_ADDITIVE 4

void SW_set_page(SLONG page, SLONG type);




//
// Adds a triangle. (x,y) are given in 8-bit fixed point and z
// is a value from 1 to 65535.
//

void SW_add_triangle(
		SLONG x1, SLONG y1, SLONG z1, SLONG r1, SLONG g1, SLONG b1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG z2, SLONG r2, SLONG g2, SLONG b2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG z3, SLONG r3, SLONG g3, SLONG b3, SLONG u3, SLONG v3,
		SLONG page,
		SLONG alpha = 255);	// Only for certain pages...

//
// Copies the SW_buffer onto the back buffer.
//

void SW_copy_to_bb(void);


#else //#ifndef TARGET_DC

// Not used - it just shuts the compiler up.
#define SW_PAGE_IGNORE   0	// This page is not drawn.
#define SW_PAGE_NORMAL   1
#define SW_PAGE_MASKED   2
#define SW_PAGE_ALPHA    3
#define SW_PAGE_ADDITIVE 4

#endif //#else //#ifndef TARGET_DC



#endif
