//
// Drawing polygons with D3D
//

#ifndef _POLY_
#define _POLY_


#ifdef TARGET_DC
// Fade the diffuse and specular, rather than using D3D fog, which doesn't work on DC yet.
// It does now!!!!!
//#define FAKE_UP_VERTEX_FOG_PLEASE_BOB defined

// If using D3D fog, use W-based table fog, rather than vertex fogging.
// (which is faked up by the driver on DC). Doesn't work yet,
#define USE_W_FOG_PLEASE_BOB defined

#endif




#if defined(TARGET_DC)

static inline int ftol(float f)
{
	return ( (int)f );
}


#else //#if defined(TARGET_DC)

#ifndef PSX
static inline int ftol(float f)
{
	int ans;

	__asm
	{
		mov		eax,f
		fld		f
		fistp	ans
	}

	return ans;
}
#endif

#endif //#else //#if defined(TARGET_DC)


//
// Only points further than 1/64'th of the draw range are drawn.
//

#define POLY_ZCLIP_PLANE	(1.0F / 64.0F)

//
// Call once at the start of the whole program.
//

void POLY_init(void);


#ifndef TARGET_DC
void	calc_global_cloud(SLONG x,SLONG y,SLONG z);
void	apply_cloud(SLONG x,SLONG y,SLONG z,ULONG *col);
void	use_global_cloud(ULONG *col);
#endif


//
// Clears all the texture flags.
// Loads texture flags from the given file. The 'offset' is added onto the
// page numbers in the file.
//

void POLY_init_texture_flags(void);
void POLY_load_texture_flags(CBYTE *fname, SLONG offset = 0);

// reset renderstates

void POLY_reset_render_states();


// ========================================================
//
// TRANSFORMING POINTS
//
// ========================================================

//
// Sets the position of the camera.  You can clip all the polygons
// to the top or bottom half of the screen.
//

#define POLY_SPLITSCREEN_NONE   0
#define POLY_SPLITSCREEN_TOP    1
#define POLY_SPLITSCREEN_BOTTOM	2

extern float POLY_cam_x;
extern float POLY_cam_y;
extern float POLY_cam_z;
extern float POLY_cam_matrix[9];

void POLY_camera_set(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float view_dist,	// The maximum distance a point should be from the camera.
		float lens,			// Normally around 1.5F... the higher it is the more zoom you get.
		SLONG splitscreen = POLY_SPLITSCREEN_NONE);

//
// These are set by calling camera_set()
//

extern float POLY_screen_width;
extern float POLY_screen_height;

//
// Given three points in world space, this function fills in
// the 3d view space coordinates of the point.  If the point
// is not 'behind' the camera, then it is transformed.  If it
// is transformed then the 2d points and clipping flags are
// calculated.
//

#define POLY_CLIP_LEFT			(1 << 0)	// point is to left of screen, implies POLY_CLIP_TRANSFORMED
#define POLY_CLIP_RIGHT			(1 << 1)	// point is to right of screen, implies POLY_CLIP_TRANSFORMED
#define POLY_CLIP_TOP			(1 << 2)	// point is above screen, implies POLY_CLIP_TRANSFORMED
#define POLY_CLIP_BOTTOM		(1 << 3)	// point is below screen, implies POLY_CLIP_TRANSFORMED
#define POLY_CLIP_TRANSFORMED	(1 << 4)	// point has been rotated into camera coordinates and projected onto the screen
#define POLY_CLIP_FAR			(1 << 5)	// point is behind farplane, implies !POLY_CLIP_TRANSFORMED
#define POLY_CLIP_NEAR			(1 << 6)	// point is in front of nearplane, implies !POLY_CLIP_TRANSFORMED

// any one of these flags set => point is off screen
#define POLY_CLIP_OFFSCREEN     (POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM | POLY_CLIP_FAR | POLY_CLIP_NEAR)

typedef struct
{
	float x;	//              
	float y;	// 3D points... 
	float z;	//              

	float X;	//             
	float Y;	// 2D points...
	float Z;	//             

	UWORD clip;
	UWORD user;	// For your use!

	float u;
	float v;
	ULONG colour;		// xxRRGGBB
	ULONG specular;		// xxRRGGBB

	// IsValid - point has been rotated & projected; screen coords OK
	inline bool	IsValid()		{ return ((clip & POLY_CLIP_TRANSFORMED) != 0); }
	// MaybeValid - point has been rotated, but may be in front of the nearplane; screen coords not necessarily OK
	inline bool	MaybeValid()	{ return ((clip & (POLY_CLIP_TRANSFORMED | POLY_CLIP_NEAR)) != 0); }
	// NearClip - point is in front of the nearplane
	inline bool	NearClip()		{ return ((clip & POLY_CLIP_NEAR) != 0); }
	
} POLY_Point;


#ifdef TARGET_DC

void POLY_flush_local_rot ( void );

void POLY_transform_c(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bResetTheFTRV = FALSE);

#define POLY_transform POLY_transform_c

#else //#ifdef TARGET_DC

// Does nothing on PC.
inline void POLY_flush_local_rot ( void )
{
}

void POLY_transform(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bUnused = FALSE);

void POLY_transform_c(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bUnused = FALSE);


#endif //#else //#ifdef TARGET_DC


void POLY_set_local_rotation_none ( void );

void POLY_transform_abs(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt);

void POLY_transform_c_saturate_z(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt);



//
// Transformation function that takes points already rotated
// into view space (whose x,y,z,u,v,colour,specular have valid
// values and calculates the X,Y,Z and clip values.
//

void POLY_transform_from_view_space(POLY_Point *pt);

//
// Returns the screen coordinates if the point is not
// zclipped. Returns TRUE on success.  This transformation
// function does not care about the z-range.
//

SLONG POLY_get_screen_pos(
		float  world_x,
		float  world_y,
		float  world_z,
		float *screen_x,
		float *screen_y);

//
// Applies the perspective transform to a POLY_Point already
// in viewspace. Viewspace x and y go from -1 to +1 and viewspace
// z goes from POLY_ZCLIP_PLANE to 1
//

void POLY_perspective(POLY_Point *pt, UBYTE wibble_key = 0);

//
// sets clipping flags for a point - DON'T just set pt->clip to TRANSFORMED!!!
//

void POLY_setclip(POLY_Point* pt);

//
// Converts a length in world space to a width on the screen.
//

float POLY_world_length_to_screen(float world_length);

//
// Get approximate length of a 2D line
//

float POLY_approx_len(float dx, float dy);

//
// Gives the screen position and approximate screen radius of
// the circle corresponding to the given sphere in world space.
// Returns FALSE if the sphere is behind you.
//

SLONG POLY_get_sphere_circle(
		float  world_x,
		float  world_y,
		float  world_z,
		float  world_radius,
		SLONG *screen_x,
		SLONG *screen_y,
		SLONG *screen_radius);

//
// Sets the wibble values to use.
// 

void POLY_set_wibble(
		UBYTE wibble_y1,
		UBYTE wibble_y2,
		UBYTE wibble_g1,
		UBYTE wibble_g2,
		UBYTE wibble_s1,
		UBYTE wibble_s2);

//
// Sets an additional matrix to be applied to the world point before
// it is transformed.
//

void POLY_set_local_rotation(
		float off_x,
		float off_y,
		float off_z,
		float matrix[9]);


#ifdef TARGET_DC

void POLY_transform_using_local_rotation_c(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt);

#define POLY_transform_using_local_rotation POLY_transform_using_local_rotation_c

#else //#ifdef TARGET_DC

void POLY_transform_using_local_rotation(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt);

#endif //#else //#ifdef TARGET_DC


void POLY_transform_using_local_rotation_and_wibble(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt,
		UBYTE       wibble_key);

//
// Returns true if the given sphere in world space is visible
// on screen. 'radius' should be given as a fraction of the viewdist passed
// to POLY_camera_set.  radius = world_radius / view_dist.
//

SLONG POLY_sphere_visible(
		float world_x,
		float world_y,
		float world_z,
		float radius);

//
// Returns TRUE if the given texture page is drawn two-pass with the
// next higher-numbered texture page over the top of it.
//

SLONG POLY_page_is_masked_self_illuminating(SLONG page);

//
// Handy buffers for rotating objects
//

#ifdef TARGET_DC
// Gatecrasher seems to give the biggest high-water-mark, during the intro cutscene.
#define POLY_BUFFER_SIZE 768
#define POLY_SHADOW_SIZE 512
#else
#define POLY_BUFFER_SIZE 8192
#define POLY_SHADOW_SIZE 8192
#endif

extern POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
extern SLONG      POLY_buffer_upto;

extern POLY_Point POLY_shadow[POLY_SHADOW_SIZE];
extern SLONG      POLY_shadow_upto;


// ========================================================
//
// GLOBALS EFFECTING POLY DRAW AND POINT FADEOUT.
//
// ========================================================

//
// All colours are ANDed with ~POLY_colour_restrict.
//

extern ULONG POLY_colour_restrict;		// NOTted then ANDed with all colours
extern ULONG POLY_force_additive_alpha;	// Everything is drawn with additive alpha.

//
// The fade-out range of the points.
//

#define POLY_FADEOUT_START	(0.60F)
#define POLY_FADEOUT_END	(0.95F)


//#define POLY_FADEOUT_START	(0.60F/3.0F)
//#define POLY_FADEOUT_END	(0.95F/3.0F)

//
// Applies fade out to the given point.
// Applies fade out to all the points in the POLY_buffer array.
//

SLONG fade_point_more(POLY_Point *pp);

static void inline POLY_fadeout_point(POLY_Point *pp)
{

#ifdef USE_W_FOG_PLEASE_BOB
	// Not much to do apart from colour masking.
	pp->colour   &= ~POLY_colour_restrict;
	pp->specular &= ~POLY_colour_restrict;
#else //#ifdef USE_W_FOG_PLEASE_BOB


	// Mark, you can't just rip out the fog and replace it with nothing else ya twonk,
	// even if you are on a Quest For Speed (but gave up after a week, having not found any).
	//return;

//	if (pp->z > POLY_FADEOUT_START)
	{
		SLONG multi = 255 - ftol((pp->z - POLY_FADEOUT_START) * (256.0F / (POLY_FADEOUT_END - POLY_FADEOUT_START)));
#ifdef TARGET_DC
		//multi = 255 - multi;
#endif

//		multi=fade_point_more(pp);


		if(multi>255)
		{
			multi=255;
		}

		if (multi < 0)
		{
			multi = 0;
		}

		pp->specular &= 0x00ffffff;
#ifndef FAKE_UP_VERTEX_FOG_PLEASE_BOB

		// Use D3D fog.
		pp->specular |= multi << 24;

#else

		SLONG red;
		SLONG green;
		SLONG blue;

		{
			red   = ((pp->colour >> 16) & 0xff) * multi >> 8;
			green = ((pp->colour >>  8) & 0xff) * multi >> 8;
			blue  = ((pp->colour >>  0) & 0xff) * multi >> 8;

#ifdef TARGET_DC
			// Force the alpha channel on.
			//pp->colour = 0xff000000;
			pp->colour = multi << 24;
			//pp->colour &= 0xff000000;
#else
			pp->colour &= 0xff000000;
#endif
			pp->colour |= red   << 16;
			pp->colour |= green << 8;
			pp->colour |= blue  << 0;

			if (pp->specular)
			{
				red   = ((pp->specular >> 16) & 0xff) * multi >> 8;
				green = ((pp->specular >>  8) & 0xff) * multi >> 8;
				blue  = ((pp->specular >>  0) & 0xff) * multi >> 8;

				pp->specular = 0;
				pp->specular |= red   << 16;
				pp->specular |= green << 8;
				pp->specular |= blue  << 0;
			}
		}
#endif

	}
/*
	else
	{
		pp->specular |= 0xff000000;
	}
*/

	pp->colour   &= ~POLY_colour_restrict;
	pp->specular &= ~POLY_colour_restrict;

#endif //#else //#ifdef USE_W_FOG_PLEASE_BOB

}

void POLY_fadeout_buffer(void);

// ========================================================
//
// ADDING TRIANGLES / QUADS	/ LINES
//
// ========================================================

//
// NB There is no vertex sharing except in a quad.
//
// While adding triangles to a page, if there are enough triangles using the same
// texture, then they are drawn immediately. The exeption to this is the shadow
// page which is only draw with a call to POLY_frame_draw(TRUE).
//

//
// The 'page' argument should either be one of the TEXTURE module's
// texture pages or one of these defines.
//

#ifdef TARGET_DC
#define POLY_NUM_PAGES (22*64+124)
#else
#define POLY_NUM_PAGES (22*64+111)	// 1508
#endif

#ifdef TARGET_DC
#define POLY_PAGE_BACKGROUND_IMAGE	(POLY_NUM_PAGES - 124)
#define POLY_PAGE_BACKGROUND_IMAGE2	(POLY_NUM_PAGES - 123)

#define POLY_PAGE_JOYPAD_A			(POLY_NUM_PAGES - 122)
#define POLY_PAGE_JOYPAD_B			(POLY_NUM_PAGES - 121)
#define POLY_PAGE_JOYPAD_C			(POLY_NUM_PAGES - 120)
#define POLY_PAGE_JOYPAD_X			(POLY_NUM_PAGES - 119)
#define POLY_PAGE_JOYPAD_Y			(POLY_NUM_PAGES - 118)
#define POLY_PAGE_JOYPAD_Z			(POLY_NUM_PAGES - 117)
#define POLY_PAGE_JOYPAD_L			(POLY_NUM_PAGES - 116)
#define POLY_PAGE_JOYPAD_R			(POLY_NUM_PAGES - 115)
#define POLY_PAGE_JOYPAD_PAD_L		(POLY_NUM_PAGES - 114)
#define POLY_PAGE_JOYPAD_PAD_R		(POLY_NUM_PAGES - 113)
#define POLY_PAGE_JOYPAD_PAD_D		(POLY_NUM_PAGES - 112)
#define POLY_PAGE_JOYPAD_PAD_U		(POLY_NUM_PAGES - 111)
#endif

#define POLY_PAGE_FADE_MF			(POLY_NUM_PAGES - 110)
#define POLY_PAGE_SNOWFLAKE			(POLY_NUM_PAGES - 109)
#define POLY_PAGE_SPLASH			(POLY_NUM_PAGES - 108)
#define POLY_PAGE_METEOR	      	(POLY_NUM_PAGES - 107)	
#define POLY_PAGE_LITE_BOLT      	(POLY_NUM_PAGES - 106)	
#define POLY_PAGE_LADSHAD			(POLY_NUM_PAGES - 105)
#define POLY_PAGE_LASTPANEL_ALPHA	(POLY_NUM_PAGES - 104)	
#define POLY_PAGE_LASTPANEL_SUB     (POLY_NUM_PAGES - 103)
#define POLY_PAGE_LASTPANEL2_SUB    (POLY_NUM_PAGES - 102)	
#define POLY_PAGE_LASTPANEL2_ADDALPHA  (POLY_NUM_PAGES - 101)	
#define POLY_PAGE_LASTPANEL_ADDALPHA   (POLY_NUM_PAGES - 100)	
#define POLY_PAGE_LASTPANEL2_ALPHA  (POLY_NUM_PAGES - 99)	
#define POLY_PAGE_LASTPANEL2_ADD    (POLY_NUM_PAGES - 98)
#define POLY_PAGE_EXPLODE2_ADDITIVE (POLY_NUM_PAGES - 97)
#define POLY_PAGE_EXPLODE1_ADDITIVE	(POLY_NUM_PAGES - 96)
#define POLY_PAGE_SHADOW_SQUARE		(POLY_NUM_PAGES - 95)
#define POLY_PAGE_PCFLAMER          (POLY_NUM_PAGES - 94)
#define POLY_PAGE_SIGN              (POLY_NUM_PAGES - 93)
#define POLY_PAGE_LASTPANEL_ADD     (POLY_NUM_PAGES - 92)
#define POLY_PAGE_LEAF		    	(POLY_NUM_PAGES - 90)
#define POLY_PAGE_RUBBISH	    	(POLY_NUM_PAGES - 89)
#define POLY_PAGE_FADECAT			(POLY_NUM_PAGES - 88)
#define POLY_PAGE_LADDER			(POLY_NUM_PAGES - 87)
#define POLY_PAGE_SMOKECLOUD2		(POLY_NUM_PAGES - 86)
#define POLY_PAGE_SMOKECLOUD		(POLY_NUM_PAGES - 85) 
#define POLY_PAGE_TINY_BUTTONS		(POLY_NUM_PAGES - 84)
#define POLY_PAGE_FINALGLOW			(POLY_NUM_PAGES - 83)
#define POLY_PAGE_BIG_LEAF			(POLY_NUM_PAGES - 82)
#define POLY_PAGE_BIG_RAIN			(POLY_NUM_PAGES - 81)
#define POLY_PAGE_BIG_BUTTONS		(POLY_NUM_PAGES - 80)
#define POLY_PAGE_COLOUR_WITH_FOG  	(POLY_NUM_PAGES - 79)
#define POLY_PAGE_ALPHA_OVERLAY 	(POLY_NUM_PAGES - 78)
#define POLY_PAGE_TYRESKID 	    	(POLY_NUM_PAGES - 77)
#define POLY_PAGE_COLOUR	    	(POLY_NUM_PAGES - 76)
#define POLY_PAGE_POLAROID  		(POLY_NUM_PAGES - 75)
#define POLY_PAGE_MENULOGO  		(POLY_NUM_PAGES - 74)
#define POLY_PAGE_SUBTRACTIVEALPHA	(POLY_NUM_PAGES - 73)
#define POLY_PAGE_NEWFONT_INVERSE	(POLY_NUM_PAGES - 72)
#define POLY_PAGE_IC_NORMAL			(POLY_NUM_PAGES - 71)
#define POLY_PAGE_IC2_NORMAL		(POLY_NUM_PAGES - 70)
#define POLY_PAGE_IC_ALPHA			(POLY_NUM_PAGES - 69)
#define POLY_PAGE_IC2_ALPHA			(POLY_NUM_PAGES - 68)
#define POLY_PAGE_IC_ADDITIVE		(POLY_NUM_PAGES - 67)
#define POLY_PAGE_IC2_ADDITIVE		(POLY_NUM_PAGES - 66)
#define POLY_PAGE_IC_ALPHA_END		(POLY_NUM_PAGES - 65)
#define POLY_PAGE_IC2_ALPHA_END		(POLY_NUM_PAGES - 64)
#define POLY_PAGE_PRESS1			(POLY_NUM_PAGES - 63)
#define POLY_PAGE_PRESS2			(POLY_NUM_PAGES - 62)
#define POLY_PAGE_NEWFONT			(POLY_NUM_PAGES - 61)
#define POLY_PAGE_TARGET			(POLY_NUM_PAGES - 60)
#define POLY_PAGE_SMOKER			(POLY_NUM_PAGES - 59)
#define POLY_PAGE_DEVIL				(POLY_NUM_PAGES - 58)
#define POLY_PAGE_ANGEL				(POLY_NUM_PAGES - 57)
#define POLY_PAGE_ADDITIVEALPHA 	(POLY_NUM_PAGES - 56)
#define POLY_PAGE_TYRETRACK	    	(POLY_NUM_PAGES - 55)
#define POLY_PAGE_WINMAP	    	(POLY_NUM_PAGES - 54)
#define POLY_PAGE_LENSFLARE     	(POLY_NUM_PAGES - 53)
#define POLY_PAGE_HITSPANG      	(POLY_NUM_PAGES - 52)
#define POLY_PAGE_BLOOM2        	(POLY_NUM_PAGES - 51)
#define POLY_PAGE_BLOOM1        	(POLY_NUM_PAGES - 50)
#define POLY_PAGE_BLOODSPLAT    	(POLY_NUM_PAGES - 49)
#define POLY_PAGE_FLAMES3       	(POLY_NUM_PAGES - 48)
#define POLY_PAGE_DUSTWAVE      	(POLY_NUM_PAGES - 47)
#define POLY_PAGE_BIGBANG       	(POLY_NUM_PAGES - 46)
#define POLY_PAGE_FACE1		    	(POLY_NUM_PAGES - 45)
#define POLY_PAGE_FACE2		    	(POLY_NUM_PAGES - 44)
#define POLY_PAGE_FACE3		    	(POLY_NUM_PAGES - 43)
#define POLY_PAGE_FACE4		    	(POLY_NUM_PAGES - 42)
#define POLY_PAGE_FACE5		    	(POLY_NUM_PAGES - 41)
#define POLY_PAGE_FACE6		    	(POLY_NUM_PAGES - 40)
#define POLY_PAGE_FONT2D        	(POLY_NUM_PAGES - 39)
#define POLY_PAGE_FOOTPRINT     	(POLY_NUM_PAGES - 38)
#define POLY_PAGE_BARBWIRE      	(POLY_NUM_PAGES - 37)
#define POLY_PAGE_MENUPASS      	(POLY_NUM_PAGES - 36)
#define POLY_PAGE_MENUTEXT      	(POLY_NUM_PAGES - 35)
#define POLY_PAGE_MENUFLAME     	(POLY_NUM_PAGES - 34)
#define POLY_PAGE_FLAMES2	    	(POLY_NUM_PAGES - 33)
#define POLY_PAGE_SMOKE  	    	(POLY_NUM_PAGES - 32)	// 1476	// this is OK to have a "bad alpha blend mode" message
#define POLY_PAGE_FLAMES 	    	(POLY_NUM_PAGES - 31)
#define POLY_PAGE_SEWATER	    	(POLY_NUM_PAGES - 28)
#define POLY_PAGE_SKY		    	(POLY_NUM_PAGES - 27)
#define POLY_PAGE_SHADOW	    	(POLY_NUM_PAGES - 26)	// 1482	// this is OK to have a "bad alpha blend mode" message
#define POLY_PAGE_SHADOW_OVAL		(POLY_NUM_PAGES - 25)
#define POLY_PAGE_PUDDLE	    	(POLY_NUM_PAGES - 24)
#define POLY_PAGE_CLOUDS	    	(POLY_NUM_PAGES - 23)
#define POLY_PAGE_ALPHA		    	(POLY_NUM_PAGES - 22)
#define POLY_PAGE_ADDITIVE	    	(POLY_NUM_PAGES - 21)
#define POLY_PAGE_MOON		    	(POLY_NUM_PAGES - 20)
#define POLY_PAGE_MANONMOON     	(POLY_NUM_PAGES - 19)
#define POLY_PAGE_MASKED	    	(POLY_NUM_PAGES - 18)
#define POLY_PAGE_ENVMAP	    	(POLY_NUM_PAGES - 17)
#define POLY_PAGE_WATER		    	(POLY_NUM_PAGES - 16)
#define POLY_PAGE_DRIP          	(POLY_NUM_PAGES - 15)
#define POLY_PAGE_FOG		    	(POLY_NUM_PAGES - 14)
#define POLY_PAGE_STEAM		    	(POLY_NUM_PAGES - 13)
#define POLY_PAGE_BANG		    	(POLY_NUM_PAGES - 11)
#define	POLY_PAGE_TEXT		    	(POLY_NUM_PAGES - 10)
#define POLY_PAGE_LOGO		    	(POLY_NUM_PAGES -  9)
#define POLY_PAGE_DROPLET	    	(POLY_NUM_PAGES -  8)
#define POLY_PAGE_RAINDROP	    	(POLY_NUM_PAGES -  7)
#define POLY_PAGE_SPARKLE	    	(POLY_NUM_PAGES -  6)
#define POLY_PAGE_EXPLODE2			(POLY_NUM_PAGES -  5)
#define POLY_PAGE_EXPLODE1			(POLY_NUM_PAGES -  4)
#define POLY_PAGE_COLOUR_ALPHA    	(POLY_NUM_PAGES -  1)

#define	POLY_PAGE_TEST_SHADOWMAP	(POLY_NUM_PAGES - 2)

//
// Clears all buffers, ready for a new frame.
// Checks the clipflags and backface culling of the triangle and returns TRUE if it should be drawn.
// Adds a triangle and a quad.
// Adds a line. The widths are given in world-space sizes. if (sort_to_front) then lines will be drawn last with no z-buffer.
// Sets the box against which clip-lines are clipped.
// Adds a line clipped against the clip box.
// Draws all the triangles and quads.
//

void  POLY_frame_init    (SLONG keep_shadow_page, SLONG keep_text_page);	// TRUE => doesn't delete the shadow polygons.
SLONG POLY_valid_triangle(POLY_Point *p[3]);
SLONG POLY_valid_quad    (POLY_Point *p[4]);
SLONG POLY_valid_line    (POLY_Point *p1, POLY_Point *p2);
void  POLY_add_poly      (POLY_Point** poly, SLONG poly_points, SLONG page);
void  POLY_add_triangle  (POLY_Point *p[3], SLONG page, SLONG shall_i_backface_cull, SLONG generate_clip_flags = FALSE);
void  POLY_add_quad      (POLY_Point *p[4], SLONG page, SLONG shall_i_backface_cull, SLONG generate_clip_flags = FALSE);
void  POLY_add_quad_split2(POLY_Point *pp[4], SLONG page, SLONG backface_cull);
void  POLY_create_cylinder_points(POLY_Point* p1, POLY_Point* p2, float width, POLY_Point* pout);
void  POLY_add_line      (POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front);
void  POLY_add_line_tex  (POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front);
void  POLY_add_line_2d   (float sx1, float sy1, float sx2, float sy2, ULONG colour);
void  POLY_clip_line_box (float sx1, float sy1, float sx2, float sy2);
void  POLY_clip_line_add (float sx1, float sy1, float sx2, float sy2, ULONG colour);
void  POLY_frame_draw    (SLONG draw_shadow_page, SLONG draw_text_page);	// FALSE => Doens't draw the shadow polygons.

void  POLY_sort_sewater_page(void);		// Sorts the sewater page polys in order of distance from the eye.

void  POLY_frame_draw_odd(void);		// Only draws normal textures with blend mode MODULATEALHPA
void  POLY_frame_draw_puddles(void);	// Only draws the puddle page.
void  POLY_frame_draw_sewater(void);	// Only draws the sewer water page.

ULONG POLY_interpolate_colour(float v, ULONG colour1, ULONG colour2);

//
// Draws the frame focused on the focal point.
// 

void  POLY_frame_draw_focused(float focus);

//
// To share points among a group of faces all using the same texture
// page. Calling any other add functions on that page will cock this up.
//

void POLY_add_shared_start(SLONG page);
void POLY_add_shared_point(POLY_Point *pp);
void POLY_add_shared_tri  (UWORD p1, UWORD p2, UWORD p3);	// 0 => The first shared point added.

//
// Returns TRUE if the given screen coordinate is inside the quad.
// If it is, then it returns the amount along the two vectors 0-1, 0-2
// that the point lies.
//
// ASSUMES that the QUAD is a PARALELLAGRAM
//

SLONG POLY_inside_quad(
		float       screen_x,
		float       screen_y,
		POLY_Point *quad[3],
		float      *along_01,
		float      *along_02);

//
// Flags for each standard texture page.
//

#define POLY_PAGE_FLAG_TRANSPARENT	(1 << 0)
#define POLY_PAGE_FLAG_WRAP			(1 << 1)
#define POLY_PAGE_FLAG_ADD_ALPHA	(1 << 2)
#define POLY_PAGE_FLAG_2PASS		(1 << 3)
#define POLY_PAGE_FLAG_SELF_ILLUM	(1 << 4)
#define POLY_PAGE_FLAG_NO_FOG		(1 << 5)
#define POLY_PAGE_FLAG_WINDOW		(1 << 6)
#define POLY_PAGE_FLAG_WINDOW_2ND	(1 << 7)
#define POLY_PAGE_FLAG_ALPHA		(1 << 8)

extern SLONG		draw_3d;
extern UWORD		POLY_page_flag[POLY_NUM_PAGES];

extern void POLY_init_render_states();

#endif
