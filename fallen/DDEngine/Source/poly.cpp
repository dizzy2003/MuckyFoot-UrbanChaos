//
// Drawing polygons with D3D
//

#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include	"Game.h"		//	Guy	-	4 DEMO
#include "matrix.h"
#include "poly.h"
#include "texture.h"
#include "message.h"
#include "night.h"
#include "vertexbuffer.h"
#include "polypoint.h"
#include "renderstate.h"
#include "polypage.h"
#include "eway.h"
#include "font2d.h"
#include "crinkle.h"
#include "night.h"
#include "BreakTimer.h"
#include "sw.h"
#include "superfacet.h"


#ifndef TARGET_DC

#define LOG_ENTER(x) {}
#define LOG_EXIT(x)  {}

#endif



extern D3DTexture TEXTURE_texture[];


int iPolyNumPagesRender = 0;


#define COMBO_FALSE 0
#define COMBO_TRUE 1
#define COMBO_DIRTY 2
int m_iCurrentCombo = COMBO_DIRTY;

//
// position of near clipping plane
//

#define POLY_Z_NEARPLANE	POLY_ZCLIP_PLANE

static UBYTE	s_ClipMask;	// the clip bits we care about

#if USE_D3D_VBUF
#define STD_CLIPMASK	(POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM | POLY_CLIP_NEAR)
#else
#define STD_CLIPMASK	(POLY_CLIP_NEAR)
#endif

#ifdef TARGET_DC
// DC clips sideways and up and down anyway.
#define NO_CLIPPING_TO_THE_SIDES_PLEASE_BOB 1
// Can't enable this - problems. :-(
#define NO_BACKFACE_CULL_PLEASE_BOB 0
#else
// PC should clip to sides fo screen
#define NO_CLIPPING_TO_THE_SIDES_PLEASE_BOB 0
#define NO_BACKFACE_CULL_PLEASE_BOB 0
#endif

#ifdef	EDITOR
// poly has ugly key stuff which makes a mess in the editors so cut it out awright?
extern HWND CUTSCENE_edit_wnd;
#endif

//
// Flags for each standard texture page.
//

UWORD POLY_page_flag[POLY_NUM_PAGES];

//
// some extern from somewhere which someone should put in a header file
//

extern SLONG draw_3d;

//
// The handy buffer
//

POLY_Point POLY_buffer[POLY_BUFFER_SIZE];
SLONG      POLY_buffer_upto;

POLY_Point POLY_shadow[POLY_SHADOW_SIZE];
SLONG      POLY_shadow_upto;

//
// The vertex buffers for each texture page.
//

RenderState	DefRenderState;
PolyPage	POLY_Page[POLY_NUM_PAGES];

// utility

#define POLY_SWAP(pp1,pp2) {POLY_Point *pp_spare; pp_spare = (pp1); (pp1) = (pp2); (pp2) = pp_spare;}

//
// The camera and the screen.
//

float POLY_cam_x;
float POLY_cam_y;
float POLY_cam_z;
float POLY_cam_aspect;
float POLY_cam_lens;
float POLY_cam_view_dist;
float POLY_cam_over_view_dist;
float POLY_cam_matrix[9];

float POLY_screen_width;
float POLY_screen_height;
float POLY_screen_mid_x;
float POLY_screen_mid_y;
float POLY_screen_mul_x;
float POLY_screen_mul_y;

float POLY_screen_clip_left=0; // these default values
float POLY_screen_clip_right=640;
float POLY_screen_clip_bottom=480;
float POLY_screen_clip_top=0;

SLONG POLY_splitscreen;
ULONG POLY_colour_restrict;
ULONG POLY_force_additive_alpha;

SLONG fade_point_more(POLY_Point *pp)
{
	SLONG	fade;

	fade=(((SLONG)(pp->y))>>0);

	//fade=2000;
//	fade=-fade;

	return(fade);

}


// POLY_init
//
// init engine

void POLY_init(void)
{
#ifdef TEX_EMBED
#if !USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
	SLONG i;

	for (int ii = 0; ii < 512; ii++)
	{
		float	u_off = (ii & 3) * 0.25f;
		float	v_off = ((ii >> 2) & 3) * 0.25f;
		POLY_Page[ii].SetTexEmbed(0.25f, u_off, 0.25f, v_off);
	}
#endif
#endif
}


// Clears all poly pages.
void POLY_ClearAllPages ( void )
{
	for ( int i = 0; i < POLY_NUM_PAGES; i++ )
	{
		POLY_Page[i].Clear();
	}
}









SLONG POLY_wibble_y1;
SLONG POLY_wibble_y2;
SLONG POLY_wibble_g1;
SLONG POLY_wibble_g2;
SLONG POLY_wibble_s1;
SLONG POLY_wibble_s2;
SLONG POLY_wibble_turn;
SLONG POLY_wibble_dangle1;
SLONG POLY_wibble_dangle2;

void POLY_set_wibble(
		UBYTE wibble_y1,
		UBYTE wibble_y2,
		UBYTE wibble_g1,
		UBYTE wibble_g2,
		UBYTE wibble_s1,
		UBYTE wibble_s2)
{
	POLY_wibble_y1 = wibble_y1;
	POLY_wibble_y2 = wibble_y2;
	POLY_wibble_g1 = wibble_g1;
	POLY_wibble_g2 = wibble_g2;
	POLY_wibble_s1 = wibble_s1;
	POLY_wibble_s2 = wibble_s2;

	POLY_wibble_turn += 256 * TICK_RATIO >> TICK_SHIFT;

	POLY_wibble_dangle1 = POLY_wibble_turn * POLY_wibble_g1 >> 9;
	POLY_wibble_dangle2 = POLY_wibble_turn * POLY_wibble_g2 >> 9;
}


SLONG POLY_page_is_masked_self_illuminating(SLONG page)
{
	if (WITHIN(page, 0, POLY_NUM_PAGES - 1) &&
		(POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#if USE_TOMS_ENGINE_PLEASE_BOB
D3DMATRIX g_matProjection;
D3DVIEWPORT2 g_viewData;
// Used to hack in letterbox mode.
DWORD g_dw3DStuffHeight;
DWORD g_dw3DStuffY;
#endif





float POLY_cam_matrix_comb[9];
float POLY_cam_off_x;
float POLY_cam_off_y;
float POLY_cam_off_z;



#ifdef TARGET_DC


_inline void EnsureFTRVMatrix ( int iCombo )
{
	// If this fails, you need to put POLY_flush_local_rot() just before it somewhere.
	ASSERT ( m_iCurrentCombo == iCombo );
}


_inline void SetupFTRVMatrix ( int iCombo )
{
	D3DMATRIX matTemp;
	if ( m_iCurrentCombo == iCombo )
	{
		// Already done.
		return;
	}
	else
	{
		m_iCurrentCombo = iCombo;
		if ( iCombo != COMBO_FALSE )
		{
			matTemp._11 = POLY_cam_matrix_comb[0];
			matTemp._12 = POLY_cam_matrix_comb[1];
			matTemp._13 = POLY_cam_matrix_comb[2];
			matTemp._14 = POLY_cam_off_x;
			matTemp._21 = POLY_cam_matrix_comb[3];
			matTemp._22 = POLY_cam_matrix_comb[4];
			matTemp._23 = POLY_cam_matrix_comb[5];
			matTemp._24 = POLY_cam_off_y;
			matTemp._31 = POLY_cam_matrix_comb[6];
			matTemp._32 = POLY_cam_matrix_comb[7];
			matTemp._33 = POLY_cam_matrix_comb[8];
			matTemp._34 = POLY_cam_off_z;
			matTemp._41 = 0.0f;
			matTemp._42 = 0.0f;
			matTemp._43 = 0.0f;
			matTemp._44 = 1.0f;
			_LoadMatrix( &(matTemp._11) );
		}
		else
		{
			matTemp._11 = POLY_cam_matrix[0];
			matTemp._12 = POLY_cam_matrix[1];
			matTemp._13 = POLY_cam_matrix[2];
			matTemp._14 = 0.0f;
			matTemp._21 = POLY_cam_matrix[3];
			matTemp._22 = POLY_cam_matrix[4];
			matTemp._23 = POLY_cam_matrix[5];
			matTemp._24 = 0.0f;
			matTemp._31 = POLY_cam_matrix[6];
			matTemp._32 = POLY_cam_matrix[7];
			matTemp._33 = POLY_cam_matrix[8];
			matTemp._34 = 0.0f;
			matTemp._41 = 0.0f;
			matTemp._42 = 0.0f;
			matTemp._43 = 0.0f;
			matTemp._44 = 1.0f;
			_LoadMatrix( &(matTemp._11) );
		}
	}
}


#endif //#ifdef TARGET_DC




void POLY_camera_set(
		float x,
		float y,
		float z,
		float yaw,
		float pitch,
		float roll,
		float view_dist,
		float lens,
		SLONG splitscreen)
{


#ifdef TARGET_DC
	// DC has plenty of Z-buffer precision, and the view_dist stuff breaks the fogging.
	// So set it to a typical constant.
	view_dist = 6000.0f;
#endif


	POLY_splitscreen = splitscreen;

	POLY_screen_width		= float(DisplayWidth);
	POLY_screen_clip_left	= 0.0;
	POLY_screen_clip_right	= POLY_screen_width-00.0;
	POLY_screen_mid_x		= POLY_screen_width  * 0.5F;
	POLY_screen_mul_x		= POLY_screen_width  * 0.5F / POLY_ZCLIP_PLANE;

	switch(POLY_splitscreen)
	{
		case POLY_SPLITSCREEN_NONE:
			POLY_screen_height		= float(DisplayHeight);

			POLY_screen_clip_top	= 00.0;
			POLY_screen_clip_bottom	= POLY_screen_height-00.0;

			{
				static float wideify = 0.0f;

				/*

				if (EWAY_stop_player_moving())
				{
					wideify += (80.0F - wideify) * 0.125F;
				}
				else
				{
					wideify += (0.0F - wideify) * 0.125F;
				}

				if (wideify < 1)	wideify = 0;

				*/

				if (EWAY_stop_player_moving())
				{
					wideify = 80.0F;
				}
				else
				{
					wideify = 0.0F;
				}

				POLY_screen_clip_top    += wideify;
				POLY_screen_clip_bottom -= wideify;

				POLY_screen_height -= wideify * 2.0F;
			}

			POLY_screen_mid_y		= POLY_screen_height * 0.5F + POLY_screen_clip_top;
			POLY_screen_mul_y		= POLY_screen_height * 0.5F / POLY_ZCLIP_PLANE;

			s_ClipMask = STD_CLIPMASK;
			break;

		case POLY_SPLITSCREEN_TOP:
			POLY_screen_height		= float(DisplayHeight >> 1);

			POLY_screen_clip_top	= 0.0F;
			POLY_screen_clip_bottom	= POLY_screen_height;

			POLY_screen_mid_y		= POLY_screen_height * 0.50F;
			POLY_screen_mul_y		= POLY_screen_height * 0.50F / POLY_ZCLIP_PLANE;

			s_ClipMask = STD_CLIPMASK | POLY_CLIP_BOTTOM;
			break;

		case POLY_SPLITSCREEN_BOTTOM:
			POLY_screen_height		= float(DisplayHeight >> 1);

			POLY_screen_clip_top	= POLY_screen_height;
			POLY_screen_clip_bottom	= float(DisplayHeight);
			
			POLY_screen_mid_y		= POLY_screen_height * 1.50F;
			POLY_screen_mul_y		= POLY_screen_height * 0.50F / POLY_ZCLIP_PLANE;

			s_ClipMask = STD_CLIPMASK | POLY_CLIP_TOP;
			break;

		default:
			ASSERT(0);
			break;
	}

//	POLY_screen_clip_left += 32;
//	POLY_screen_clip_right -= 32;
//	POLY_screen_clip_top += 32;
//	POLY_screen_clip_bottom -= 32;

	POLY_cam_x = x;
	POLY_cam_y = y;
	POLY_cam_z = z;

	POLY_cam_lens           = lens;
	POLY_cam_view_dist      = view_dist;
	POLY_cam_over_view_dist = 1.0F / view_dist;
	POLY_cam_aspect         = POLY_screen_height / POLY_screen_width;

	MATRIX_calc(
		POLY_cam_matrix,
		yaw,
		pitch,
		roll);


	{
		//
		// Tell the crinkle code about the view-space light vector.
		//

		float dx = float(NIGHT_amb_norm_x) * (1.0F / 256.0F);
		float dy = float(NIGHT_amb_norm_y) * (1.0F / 256.0F);
		float dz = float(NIGHT_amb_norm_z) * (1.0F / 256.0F);

		CRINKLE_light(dx,dy,dz);
	}

	{
		//
		// Tell the crinkle module about the view space skewing.
		//

		CRINKLE_skew(
			POLY_cam_aspect,
			POLY_cam_lens);
	}

	MATRIX_skew(
		POLY_cam_matrix,
		POLY_cam_aspect,
		POLY_cam_lens,
		POLY_cam_over_view_dist);	// Shrink the matrix down so the furthest point has a view distance z of 1.0F


#if USE_TOMS_ENGINE_PLEASE_BOB

	HRESULT hres;

	// View matrix is just unit - we concatenate everything
	// into the world matrix.
	D3DMATRIX matTemp;
	matTemp._11 = 1.0f;
	matTemp._21 = 0.0f;
	matTemp._31 = 0.0f;
	matTemp._41 = 0.0f;			
	matTemp._12 = 0.0f;
	matTemp._22 = 1.0f;
	matTemp._32 = 0.0f;
	matTemp._42 = 0.0f;			
	matTemp._13 = 0.0f;
	matTemp._23 = 0.0f;
	matTemp._33 = 1.0f;
	matTemp._43 = 0.0f;			
	matTemp._14 = 0.0f;
	matTemp._24 = 0.0f;
	matTemp._34 = 0.0f;
	matTemp._44 = 1.0f;
	hres = (the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_VIEW, &matTemp );


	// Set up the projection matrix - should be done infrequently enough not to matter.
#ifdef TARGET_DC
	const float fOneOverPZP = 1.0f / POLY_ZCLIP_PLANE;

	g_matProjection._11 = -fOneOverPZP;
	g_matProjection._21 = 0.0f;
	g_matProjection._31 = 0.0f;
	g_matProjection._41 = 0.0f;
	g_matProjection._12 = 0.0f;
	g_matProjection._22 = fOneOverPZP;
	g_matProjection._32 = 0.0f;
	g_matProjection._42 = 0.0f;
	g_matProjection._13 = 0.0f;
	g_matProjection._23 = 0.0f;
	g_matProjection._33 = fOneOverPZP;
	g_matProjection._43 = -1.0f;
	g_matProjection._14 = 0.0f;
	g_matProjection._24 = 0.0f;
	g_matProjection._34 = fOneOverPZP;
	g_matProjection._44 = 0.0f;
#else
	g_matProjection._11 = -1.0f;
	g_matProjection._21 = 0.0f;
	g_matProjection._31 = 0.0f;
	g_matProjection._41 = 0.0f;
	g_matProjection._12 = 0.0f;
	g_matProjection._22 = 1.0f;
	g_matProjection._32 = 0.0f;
	g_matProjection._42 = 0.0f;
	g_matProjection._13 = 0.0f;
	g_matProjection._23 = 0.0f;
	g_matProjection._33 = 1.0f;
	g_matProjection._43 = -POLY_ZCLIP_PLANE;
	g_matProjection._14 = 0.0f;
	g_matProjection._24 = 0.0f;
	g_matProjection._34 = 1.0f;
	g_matProjection._44 = 0.0f;
#endif

	hres = (the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_PROJECTION, &g_matProjection );


	// And set up the viewport.
	memset(&g_viewData, 0, sizeof(D3DVIEWPORT2));
	g_viewData.dwSize = sizeof(D3DVIEWPORT2);
	float fMyMulX = POLY_screen_mul_x * POLY_ZCLIP_PLANE;
	float fMyMulY = POLY_screen_mul_y * POLY_ZCLIP_PLANE;
#if 1
	g_dw3DStuffHeight	= fMyMulY * PolyPage::s_YScale * 2;
	g_dw3DStuffY		= ( POLY_screen_mid_y - fMyMulY ) * PolyPage::s_YScale;
	g_viewData.dwWidth  = fMyMulX * PolyPage::s_XScale * 2;
	g_viewData.dwHeight = fMyMulY * PolyPage::s_YScale * 2;
	g_viewData.dwX = ( POLY_screen_mid_x - fMyMulX ) * PolyPage::s_XScale;
	g_viewData.dwY = ( POLY_screen_mid_y - fMyMulY ) * PolyPage::s_YScale;
	g_viewData.dvClipX  = -1.0f;
	g_viewData.dvClipY  =  1.0;
	g_viewData.dvClipWidth  = 2.0f;
	g_viewData.dvClipHeight = 2.0f;
	g_viewData.dvMinZ = 0.0f;
	g_viewData.dvMaxZ = 1.0f;
#else
#ifdef TARGET_DC
	// A horrible hack for letterbox mode, otherwise the text doesn't get drawn.
	g_dw3DStuffHeight	= fMyMulY * PolyPage::s_YScale * 2;
	g_dw3DStuffY		= ( POLY_screen_mid_y - fMyMulY ) * PolyPage::s_YScale;
	g_viewData.dwWidth  = 640;
	g_viewData.dwHeight = 480;
	g_viewData.dwX = 0;
	g_viewData.dwY = 0;
	g_viewData.dvClipX  = -1.0f;
	g_viewData.dvClipY  =  1.0f;
	g_viewData.dvClipWidth  = 2.0f;
	g_viewData.dvClipHeight = 2.0f;
	g_viewData.dvMinZ = 0.0f;
	g_viewData.dvMaxZ = 1.0f;
#else
	// A horrible hack for letterbox mode.
	g_dw3DStuffHeight	= fMyMulY * PolyPage::s_YScale * 2;
	g_dw3DStuffY		= ( POLY_screen_mid_y - fMyMulY ) * PolyPage::s_YScale;
	g_viewData.dwWidth  = POLY_screen_width;
	g_viewData.dwHeight = POLY_screen_height;
	g_viewData.dwX = 0;
	g_viewData.dwY = 0;
	g_viewData.dvClipX  = -1.0f;
	g_viewData.dvClipY  =  1.0f;
	g_viewData.dvClipWidth  = 2.0f;
	g_viewData.dvClipHeight = 2.0f;
	g_viewData.dvMinZ = 0.0f;
	g_viewData.dvMaxZ = 1.0f;
#endif
#endif
#ifdef TARGET_DC
	// Viewport must be 32-pixel aligned on DC, apparently.
	// But sod it for now - actually seems to work just fine!
	ASSERT ( ( g_viewData.dwWidth & 31 ) == 0 );
	ASSERT ( ( g_viewData.dwHeight & 31 ) == 0 );
	ASSERT ( ( g_viewData.dwX & 31 ) == 0 );
	/*
	if ( ( g_viewData.dwY & 31 ) != 0 )
	{
		// Round to nearest.
		g_viewData.dwY = ( g_viewData.dwY + 16 ) & ~31;
	}
	*/
#endif

	hres = (the_display.lp_D3D_Viewport)->SetViewport2 ( &g_viewData );

#endif //#if USE_TOMS_ENGINE_PLEASE_BOB

	
#ifdef TARGET_DC
	m_iCurrentCombo = COMBO_DIRTY;
	SetupFTRVMatrix ( COMBO_FALSE );
#endif

	SUPERFACET_start_frame();

}

//
// set clipping flags assuming point is not near- or far-clipped
// --- DON'T just set pt->clip to TRANSFORMED! ---
//

extern inline void POLY_setclip(POLY_Point* pt)
{
	pt->clip = POLY_CLIP_TRANSFORMED;

	if (pt->X < POLY_screen_clip_left)			pt->clip |= POLY_CLIP_LEFT;
	else if (pt->X > POLY_screen_clip_right)	pt->clip |= POLY_CLIP_RIGHT;

	if (pt->Y < POLY_screen_clip_top)			pt->clip |= POLY_CLIP_TOP;
	else if (pt->Y > POLY_screen_clip_bottom)	pt->clip |= POLY_CLIP_BOTTOM;
}

//
// project camera coords onto screen
//

inline void POLY_perspective(POLY_Point *pt, UBYTE wibble_key)
{
	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	if (pt->z > 1.0F)
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

		//
		// Wibble!
		//

		if (wibble_key)
		{
			SLONG offset;
			SLONG angle1;
			SLONG angle2;

			angle1  = wibble_key * POLY_wibble_y1 >> 2;
			angle2  = wibble_key * POLY_wibble_y2 >> 2;
			angle1 += POLY_wibble_dangle1;
			angle2 += POLY_wibble_dangle2;

			angle1 &= 2047;
			angle2 &= 2047;

			offset  = SIN(angle1) * POLY_wibble_s1 >> 19;
			offset += COS(angle2) * POLY_wibble_s2 >> 19;

			pt->X += offset;
		}

		//
		// Set the clipping flags.
		//

		POLY_setclip(pt);
	}
}





#ifdef TARGET_DC



void POLY_flush_local_rot ( void )
{
	m_iCurrentCombo = COMBO_DIRTY;
	SetupFTRVMatrix ( COMBO_FALSE );
}


void POLY_transform_c(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bResetTheFTRV)
{


	LOG_ENTER ( Poly_Transform_c )

	if ( bResetTheFTRV )
	{
		SetupFTRVMatrix ( COMBO_FALSE );
	}
	else
	{
		EnsureFTRVMatrix ( COMBO_FALSE );
	}
	pt->x = world_x - POLY_cam_x;
	pt->y = world_y - POLY_cam_y;
	pt->z = world_z - POLY_cam_z;
	_XDXform3dV( &(pt->x), &(pt->x) );

#ifdef DEBUG
	// Check the results.
	POLY_Point mypt;
	mypt.x = world_x - POLY_cam_x;
	mypt.y = world_y - POLY_cam_y;
	mypt.z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		mypt.x,
		mypt.y,
		mypt.z);

	// If these trigger, you probably need to call POLY_set_local_rotation_none() sometime before.
	ASSERT ( fabs ( mypt.x - pt->x ) < 0.01f );
	ASSERT ( fabs ( mypt.y - pt->y ) < 0.01f );
	ASSERT ( fabs ( mypt.z - pt->z ) < 0.01f );

#endif


	static int iCount = 0;
	iCount++;


	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else if (pt->z > 1.0F)
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

		//
		// Set the clipping flags.
		//

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < POLY_screen_clip_left)			pt->clip |= POLY_CLIP_LEFT;
		else if (pt->X > POLY_screen_clip_right)	pt->clip |= POLY_CLIP_RIGHT;

		if (pt->Y < POLY_screen_clip_top)			pt->clip |= POLY_CLIP_TOP;
		else if (pt->Y > POLY_screen_clip_bottom)	pt->clip |= POLY_CLIP_BOTTOM;
	}

	LOG_EXIT ( Poly_Transform_c )
}




#else //#ifdef TARGET_DC






void POLY_transform_c(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bUnused)
{
	pt->x = world_x - POLY_cam_x;
	pt->y = world_y - POLY_cam_y;
	pt->z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);

#if 0
	POLY_perspective(pt);
#else
	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	if (pt->z > 1.0F)
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

#if 1
		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

		//
		// Set the clipping flags.
		//
		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < POLY_screen_clip_left)			pt->clip |= POLY_CLIP_LEFT;
		else if (pt->X > POLY_screen_clip_right)	pt->clip |= POLY_CLIP_RIGHT;

		if (pt->Y < POLY_screen_clip_top)			pt->clip |= POLY_CLIP_TOP;
		else if (pt->Y > POLY_screen_clip_bottom)	pt->clip |= POLY_CLIP_BOTTOM;
#else
		ASSERT(POLY_CLIP_LEFT == 1);
		ASSERT(POLY_CLIP_RIGHT == 2);
		ASSERT(POLY_CLIP_TOP == 4);
		ASSERT(POLY_CLIP_BOTTOM == 8);

		float	xml, rmx, ymt, bmy;

		pt->clip = POLY_CLIP_TRANSFORMED;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		xml = pt->X - POLY_screen_clip_left;
		rmx = POLY_screen_clip_right - pt->X;

		pt->clip |= *((ULONG*)&xml) >> 31;
		pt->clip |= (*((ULONG*)&rmx) >> 31) << 1;

		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;
		ymt = pt->Y - POLY_screen_clip_top;
		bmy = POLY_screen_clip_bottom - pt->Y;

		pt->clip |= (*((ULONG*)&ymt) >> 31) << 2;
		pt->clip |= (*((ULONG*)&bmy) >> 31) << 3;
#endif
	}
#endif
}

#endif //#else //#ifdef TARGET_DC


void POLY_transform_c_saturate_z(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt)
{
	LOG_ENTER ( Poly_Transform_c_sat_z )

	pt->x = world_x - POLY_cam_x;
	pt->y = world_y - POLY_cam_y;
	pt->z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);

#if 0
	POLY_perspective(pt);
#else
	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	{

		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

#if 1
		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;


		//
		// Set the clipping flags.
		//
		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < POLY_screen_clip_left)			pt->clip |= POLY_CLIP_LEFT;
		else if (pt->X > POLY_screen_clip_right)	pt->clip |= POLY_CLIP_RIGHT;

		if (pt->Y < POLY_screen_clip_top)			pt->clip |= POLY_CLIP_TOP;
		else if (pt->Y > POLY_screen_clip_bottom)	pt->clip |= POLY_CLIP_BOTTOM;
#else
		ASSERT(POLY_CLIP_LEFT == 1);
		ASSERT(POLY_CLIP_RIGHT == 2);
		ASSERT(POLY_CLIP_TOP == 4);
		ASSERT(POLY_CLIP_BOTTOM == 8);

		float	xml, rmx, ymt, bmy;

		pt->clip = POLY_CLIP_TRANSFORMED;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		xml = pt->X - POLY_screen_clip_left;
		rmx = POLY_screen_clip_right - pt->X;

		pt->clip |= *((ULONG*)&xml) >> 31;
		pt->clip |= (*((ULONG*)&rmx) >> 31) << 1;

		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;
		ymt = pt->Y - POLY_screen_clip_top;
		bmy = POLY_screen_clip_bottom - pt->Y;

		pt->clip |= (*((ULONG*)&ymt) >> 31) << 2;
		pt->clip |= (*((ULONG*)&bmy) >> 31) << 3;
#endif
	}
#endif
	LOG_EXIT ( Poly_Transform_c_sat_z )
}





void POLY_transform_from_view_space(POLY_Point *pt)
{
	LOG_ENTER ( Poly_Transform_from_view_space )
	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

		//
		// Set the clipping flags.
		//

		pt->clip = POLY_CLIP_TRANSFORMED;

		     if (pt->X < POLY_screen_clip_left )  {pt->clip |= POLY_CLIP_LEFT; }
		else if (pt->X > POLY_screen_clip_right)  {pt->clip |= POLY_CLIP_RIGHT;}

		     if (pt->Y < POLY_screen_clip_top   ) {pt->clip |= POLY_CLIP_TOP;   }
		else if (pt->Y > POLY_screen_clip_bottom) {pt->clip |= POLY_CLIP_BOTTOM;}
	}
	LOG_EXIT ( Poly_Transform_from_view_space )
}




void POLY_transform_abs(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt)
{
	pt->x = world_x; //- POLY_cam_x;
	pt->y = world_y; //- POLY_cam_y;
	pt->z = world_z; //- POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		pt->x,
		pt->y,
		pt->z);

	POLY_perspective(pt);
}

SLONG POLY_get_screen_pos(
		float  world_x,
		float  world_y,
		float  world_z,
		float *screen_x,
		float *screen_y)
{
	float vx = world_x - POLY_cam_x;
	float vy = world_y - POLY_cam_y;
	float vz = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		vx,
		vy,
		vz);

	if (vz < POLY_Z_NEARPLANE)
	{
		return FALSE;
	}
	else
	{
		float Z = POLY_ZCLIP_PLANE / vz;

		*screen_x = POLY_screen_mid_x - POLY_screen_mul_x * vx * Z;
		*screen_y = POLY_screen_mid_y - POLY_screen_mul_y * vy * Z;

		return TRUE;
	}
}





//
// The combined rotation matrix.
//

#if USE_TOMS_ENGINE_PLEASE_BOB
D3DMATRIX g_matWorld;
#endif

void POLY_set_local_rotation(
		float off_x,
		float off_y,
		float off_z,
		float matrix[9])
{
	LOG_ENTER ( Poly_set_local_rotation )

	POLY_cam_off_x = off_x - POLY_cam_x;
	POLY_cam_off_y = off_y - POLY_cam_y;
	POLY_cam_off_z = off_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		POLY_cam_off_x,
		POLY_cam_off_y,
		POLY_cam_off_z);

	MATRIX_3x3mul(
		POLY_cam_matrix_comb,
		POLY_cam_matrix,
		matrix);


#if USE_TOMS_ENGINE_PLEASE_BOB
	// Dump into the WORLD matrix.
	g_matWorld._11 = POLY_cam_matrix_comb[0];
	g_matWorld._21 = POLY_cam_matrix_comb[1];
	g_matWorld._31 = POLY_cam_matrix_comb[2];
	g_matWorld._41 = POLY_cam_off_x;
	g_matWorld._12 = POLY_cam_matrix_comb[3];
	g_matWorld._22 = POLY_cam_matrix_comb[4];
	g_matWorld._32 = POLY_cam_matrix_comb[5];
	g_matWorld._42 = POLY_cam_off_y;
	g_matWorld._13 = POLY_cam_matrix_comb[6];
	g_matWorld._23 = POLY_cam_matrix_comb[7];
	g_matWorld._33 = POLY_cam_matrix_comb[8];
	g_matWorld._43 = POLY_cam_off_z;
	g_matWorld._14 = 0.0f;
	g_matWorld._24 = 0.0f;
	g_matWorld._34 = 0.0f;
	g_matWorld._44 = 1.0f;
	HRESULT hres = (the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_WORLD, &g_matWorld );
#endif

#ifdef TARGET_DC
	m_iCurrentCombo = COMBO_DIRTY;
	SetupFTRVMatrix ( COMBO_TRUE );
#endif

	LOG_EXIT ( Poly_set_local_rotation )

}

// Sets up a null local rotation, i.e. none.
// Only useful for setting the current camera rotation into the D3D ones.
void POLY_set_local_rotation_none ( void )
{
	LOG_ENTER ( Poly_set_local_rotation )

	POLY_cam_off_x = -POLY_cam_x;
	POLY_cam_off_y = -POLY_cam_y;
	POLY_cam_off_z = -POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		POLY_cam_off_x,
		POLY_cam_off_y,
		POLY_cam_off_z);

#if USE_TOMS_ENGINE_PLEASE_BOB
	// Dump into the WORLD matrix.
	g_matWorld._11 = POLY_cam_matrix[0];
	g_matWorld._21 = POLY_cam_matrix[1];
	g_matWorld._31 = POLY_cam_matrix[2];
	g_matWorld._41 = POLY_cam_off_x;
	g_matWorld._12 = POLY_cam_matrix[3];
	g_matWorld._22 = POLY_cam_matrix[4];
	g_matWorld._32 = POLY_cam_matrix[5];
	g_matWorld._42 = POLY_cam_off_y;
	g_matWorld._13 = POLY_cam_matrix[6];
	g_matWorld._23 = POLY_cam_matrix[7];
	g_matWorld._33 = POLY_cam_matrix[8];
	g_matWorld._43 = POLY_cam_off_z;
	g_matWorld._14 = 0.0f;
	g_matWorld._24 = 0.0f;
	g_matWorld._34 = 0.0f;
	g_matWorld._44 = 1.0f;
	HRESULT hres = (the_display.lp_D3D_Device)->SetTransform ( D3DTRANSFORMSTATE_WORLD, &g_matWorld );
#endif

#ifdef TARGET_DC
	m_iCurrentCombo = COMBO_DIRTY;
	SetupFTRVMatrix ( COMBO_FALSE );
#endif

	LOG_EXIT ( Poly_set_local_rotation )
}


#ifdef TARGET_DC

void POLY_transform_using_local_rotation_c(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt)
{
	LOG_ENTER ( Poly_transform_using_local_rotation_c )

#if 0
	pt->x = local_x;
	pt->y = local_y;
	pt->z = local_z;

	MATRIX_MUL(
		POLY_cam_matrix_comb,
		pt->x,
		pt->y,
		pt->z);

	pt->x += POLY_cam_off_x;
	pt->y += POLY_cam_off_y;
	pt->z += POLY_cam_off_z;
#else
	float fVec[4];
	EnsureFTRVMatrix ( COMBO_TRUE );
	fVec[0] = local_x;
	fVec[1] = local_y;
	fVec[2] = local_z;
	fVec[3] = 1.0f;

	_XDXform4dV( fVec, fVec );

	pt->x = fVec[0];
	pt->y = fVec[1];
	pt->z = fVec[2];
#endif


	static int iCount = 0;
	iCount++;


	if (pt->z < POLY_Z_NEARPLANE)
	{
		pt->clip = POLY_CLIP_NEAR;
	}
	else if (pt->z > 1.0F)
	{
		pt->clip = POLY_CLIP_FAR;
	}
	else
	{
		//
		// The z-range of the point is okay.
		//

		pt->Z = POLY_ZCLIP_PLANE / pt->z;

		pt->X = POLY_screen_mid_x - POLY_screen_mul_x * pt->x * pt->Z;
		pt->Y = POLY_screen_mid_y - POLY_screen_mul_y * pt->y * pt->Z;

		//
		// Set the clipping flags.
		//

		pt->clip = POLY_CLIP_TRANSFORMED;

		if (pt->X < POLY_screen_clip_left)			pt->clip |= POLY_CLIP_LEFT;
		else if (pt->X > POLY_screen_clip_right)	pt->clip |= POLY_CLIP_RIGHT;

		if (pt->Y < POLY_screen_clip_top)			pt->clip |= POLY_CLIP_TOP;
		else if (pt->Y > POLY_screen_clip_bottom)	pt->clip |= POLY_CLIP_BOTTOM;
	}
	LOG_EXIT ( Poly_transform_using_local_rotation_c )

}


#else



void POLY_transform_using_local_rotation_c(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt)
{
	pt->x = local_x;
	pt->y = local_y;
	pt->z = local_z;

	MATRIX_MUL(
		POLY_cam_matrix_comb,
		pt->x,
		pt->y,
		pt->z);

	pt->x += POLY_cam_off_x;
	pt->y += POLY_cam_off_y;
	pt->z += POLY_cam_off_z;

	POLY_perspective(pt);
}


#endif

void POLY_transform_using_local_rotation_and_wibble(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt,
		UBYTE       wibble_key)
{
	pt->x = local_x;
	pt->y = local_y;
	pt->z = local_z;

	MATRIX_MUL(
		POLY_cam_matrix_comb,
		pt->x,
		pt->y,
		pt->z);

	pt->x += POLY_cam_off_x;
	pt->y += POLY_cam_off_y;
	pt->z += POLY_cam_off_z;

	POLY_perspective(pt, wibble_key);
}



SLONG POLY_sphere_visible(
		float world_x,
		float world_y,
		float world_z,
		float radius)
{
	float view_x;
	float view_y;
	float view_z;

	//
	// Rotate into viewspace.
	//
	
	view_x = world_x - POLY_cam_x;
	view_y = world_y - POLY_cam_y;
	view_z = world_z - POLY_cam_z;

	MATRIX_MUL(
		POLY_cam_matrix,
		view_x,
		view_y,
		view_z);

	if (view_z + radius <= POLY_Z_NEARPLANE)
	{
		//
		// Behind the view pyramid.
		//

		return FALSE;
	}

	if (view_x + radius < -view_z ||
	    view_x - radius > +view_z)
	{
		return FALSE;
	}

	if (view_y + radius * 1.4F < -view_z ||
	    view_y - radius * 1.4F > +view_z)
	{
		return FALSE;
	}

	return TRUE;
}


void POLY_fadeout_buffer()
{
	SLONG i;

	for (i = 0; i < POLY_buffer_upto; i++)
	{
		POLY_fadeout_point(&POLY_buffer[i]);
	}
}


extern	UWORD	fade_black;



void POLY_frame_init(SLONG keep_shadow_page, SLONG keep_text_page)
{
	SLONG i;
//	TRACE("poly frame init\n");



#ifdef TARGET_DC
	ASSERT (!keep_shadow_page);
	ASSERT (!keep_text_page);
	for (i = 0; i < POLY_NUM_PAGES; i++)
	{
		{
			// Check that the page is empty.
			ASSERT ( POLY_Page[i].m_VBUsed == 0 );
			ASSERT ( POLY_Page[i].m_iNumIndicesUsed == 0 );
		}
	}


#else

	// This is going to cost serious performance - it bins all the VBs and IBs that we allocate,
	// so they all need allocating again. Madness.
	for (i = 0; i < POLY_NUM_PAGES; i++)
	{
		if (keep_shadow_page && i == POLY_PAGE_SHADOW)
		{
			//
			// Keep this stuff...
			//
		}
		else
		if (keep_text_page && i == POLY_PAGE_TEXT)
		{
			//
			// Keep this stuff...
			//
		}
		else
		{
			POLY_Page[i].Clear();
		}
	}
//	TRACE("poly frame init EXIT\n");
#endif

	// SPONG
#if USE_TOMS_ENGINE_PLEASE_BOB
	// Start the frame - we may draw polys at any time.
	//TRACE("Dodgy one!\n");
	POLY_init_render_states();

	ULONG fog_colour;
	if ((GAME_FLAGS & GF_SEWERS) || (GAME_FLAGS & GF_INDOORS))
	{
		fog_colour = 0;
	}
	else
	{
		if(fade_black)
		{
			fog_colour=0;
		}
		else
		{
			fog_colour = 
				(NIGHT_sky_colour.red   << 16) |
				(NIGHT_sky_colour.green <<  8) |
				(NIGHT_sky_colour.blue  <<  0);
		}

		if (draw_3d)
		{
			SLONG white = NIGHT_sky_colour.red + NIGHT_sky_colour.green + NIGHT_sky_colour.blue;

			white /= 3;

			fog_colour = 
				(white << 16) |
				(white <<  8) |
				(white <<  0);
		}
	}


	// set default render state
	DefRenderState.InitScene(fog_colour);
//	BreakTime("FRAMEDRAW init scene");

	BEGIN_SCENE;
#endif

}

SLONG POLY_valid_triangle(POLY_Point *pp[3])
{
	// all points must be either near-clipped or fully transformed
	if (!pp[0]->MaybeValid())		return FALSE;
	if (!pp[1]->MaybeValid())		return FALSE;
	if (!pp[2]->MaybeValid())		return FALSE;

	// if all points are clipped in one direction, polygon is invalid
	if ((pp[0]->clip & pp[1]->clip & pp[2]->clip) & POLY_CLIP_OFFSCREEN)
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_quad(POLY_Point *pp[4])
{
	// all points must be either near-clipped or fully transformed
	if (!pp[0]->MaybeValid())		return FALSE;
	if (!pp[1]->MaybeValid())		return FALSE;
	if (!pp[2]->MaybeValid())		return FALSE;
	if (!pp[3]->MaybeValid())		return FALSE;

	// if all points are clipped in one direction, polygon is invalid
	if ((pp[0]->clip & pp[1]->clip & pp[2]->clip & pp[3]->clip) & POLY_CLIP_OFFSCREEN)
	{
		return FALSE;
	}

	return TRUE;
}

SLONG POLY_valid_line(POLY_Point *p1, POLY_Point *p2)
{
	// all points must be either near-clipped or fully transformed
	if (!p1->IsValid())			return FALSE;
	if (!p2->IsValid())			return FALSE;

	// if all points are clipped in one direction, line is invalid
	// (wrong: line thickness might revalidate it; but we don't care too much)
	if ((p1->clip & p2->clip) & POLY_CLIP_OFFSCREEN)
	{
		return FALSE;
	}

	return TRUE;
}

// POLY_tri_backfacing
//
// returns true if triangle is backfacing

inline bool POLY_tri_backfacing(POLY_Point *pp1, POLY_Point *pp2, POLY_Point *pp3)
{
	float	x12,y12,z12;	// 1->2 vector
	float	x13,y13,z13;	// 1->3 vector
	float	cx,cy,cz;		// normal vector (not normalized)
	float	dp;				// dot product

	LOG_ENTER ( Poly_tri_backfacing )

	ASSERT(pp1->MaybeValid());
	ASSERT(pp2->MaybeValid());
	ASSERT(pp3->MaybeValid());

	// get 1->2 vector
	x12 = pp2->x - pp1->x;
	y12 = pp2->y - pp1->y;
	z12 = pp2->z - pp1->z;

	// get 1->3 vector
	x13 = pp3->x - pp1->x;
	y13 = pp3->y - pp1->y;
	z13 = pp3->z - pp1->z;

	// get cross product
	cx = y12 * z13 - z12 * y13;
	cy = z12 * x13 - x12 * z13;
	cz = x12 * y13 - y12 * x13;

	// dot with eye vector
	dp = cx * pp1->x + cy * pp1->y + cz * pp1->z;

	LOG_EXIT ( Poly_tri_backfacing )

	return (dp < 0);
}

// TweenD3DColour
//
// create a new tweened colour from two given colours
//
// (supercedes POLY_interpolate_colour - bugs fixed and speed increased)

static inline ULONG TweenD3DColour(ULONG c1, ULONG c2, ULONG lambda8)
{
	// quick check
	if (c1 == c2)					return c1;
	if (lambda8 == 0)				return c1;
	if (lambda8 == 256)				return c2;

	// lerp R & B
	SLONG	rb1 = c1 & 0x00FF00FF;
	SLONG	rb2 = c2 & 0x00FF00FF;

	if (rb1 != rb2)
	{
		rb2 -= rb1;
		rb2 *= lambda8;
		rb1 += rb2 >> 8;
		rb1 &= 0x00FF00FF;
	}

	// lerp A & G
	SLONG	ag1 = (c1 >> 8) & 0x00FF00FF;
	SLONG	ag2 = (c2 >> 8) & 0x00FF00FF;

	if (ag1 != ag2)
	{
		ag2 -= ag1;
		ag2 *= lambda8;
		ag1 += ag2 >> 8;
		ag1 &= 0x00FF00FF;
	}

	return rb1 + (ag1 << 8);
}

//
// add a polygon (defined by a string of vertices)
// no backface culling is performed here, but nearplane and splitscreen clipping is
//

static POLY_Point	s_PointBuffer[32];
static ULONG		s_PointBufferOffset;

// NewTweenVertex3D
//
// create and project a new vertex between two others

POLY_Point* NewTweenVertex3D(POLY_Point* p1, POLY_Point* p2, float lambda)
{
	ULONG	lambda8;

	// extract 8-bit modulation index using fast float cast
	// note: lambda *must* be between 0.0F and 1.0F inclusive
	// mapping to 0 and 256 inclusive
	*(float*)&lambda8 = lambda + 32768.0F;
	lambda8 &= 0x1FF;

	POLY_Point*	np = &s_PointBuffer[s_PointBufferOffset++];

	np->x = p1->x + lambda * (p2->x - p1->x);
	np->y = p1->y + lambda * (p2->y - p1->y);
//	np->z = p1->z + lambda * (p2->z - p1->z);
	np->z = POLY_Z_NEARPLANE;

	np->u = p1->u + lambda * (p2->u - p1->u);
	np->v = p1->v + lambda * (p2->v - p1->v);

	np->colour   = TweenD3DColour(p1->colour, p2->colour, lambda8);
	np->specular = TweenD3DColour(p1->specular, p2->specular, lambda8);

	// project
	POLY_perspective(np);
	ASSERT(np->IsValid());

	return np;
}

// NewTweenVertex2D_X
//
// create a new vertex between two others

POLY_Point* NewTweenVertex2D_X(POLY_Point* p1, POLY_Point* p2, float lambda, float xcoord)
{
	ULONG	lambda8;

	// extract 8-bit modulation index using fast float cast
	// note: lambda *must* be between 0.0F and 1.0F inclusive
	// mapping to 0 and 256 inclusive
	*(float*)&lambda8 = lambda + 32768.0F;
	lambda8 &= 0x1FF;

	POLY_Point*	np = &s_PointBuffer[s_PointBufferOffset++];

	np->X = xcoord;
	np->Y = p1->Y + lambda * (p2->Y - p1->Y);
	np->Z = p1->Z + lambda * (p2->Z - p1->Z);

	// do u,v perspective correct
	float	u1 = p1->u * p1->Z;
	float	v1 = p1->v * p1->Z;
	float	u2 = p2->u * p2->Z;
	float	v2 = p2->v * p2->Z;
	float	rz = 1.0F / np->Z;

	np->u = rz * (u1 + lambda * (u2 - u1));
	np->v = rz * (v1 + lambda * (v2 - v1));

	np->colour   = TweenD3DColour(p1->colour, p2->colour, lambda8);
	np->specular = TweenD3DColour(p1->specular, p2->specular, lambda8);

	// set clip flags
	POLY_setclip(np);

	return np;
}

// NewTweenVertex2D_Y
//
// create a new vertex between two others

POLY_Point* NewTweenVertex2D_Y(POLY_Point* p1, POLY_Point* p2, float lambda, float ycoord)
{
	ULONG	lambda8;

	// extract 8-bit modulation index using fast float cast
	// note: lambda *must* be between 0.0F and 1.0F inclusive
	// mapping to 0 and 256 inclusive
	*(float*)&lambda8 = lambda + 32768.0F;
	lambda8 &= 0x1FF;

	POLY_Point*	np = &s_PointBuffer[s_PointBufferOffset++];

	np->X = p1->X + lambda * (p2->X - p1->X);
	np->Y = ycoord;
	np->Z = p1->Z + lambda * (p2->Z - p1->Z);

	// do u,v perspective correct
	float	u1 = p1->u * p1->Z;
	float	v1 = p1->v * p1->Z;
	float	u2 = p2->u * p2->Z;
	float	v2 = p2->v * p2->Z;
	float	rz = 1.0F / np->Z;

	np->u = rz * (u1 + lambda * (u2 - u1));
	np->v = rz * (v1 + lambda * (v2 - v1));

	np->colour   = TweenD3DColour(p1->colour, p2->colour, lambda8);
	np->specular = TweenD3DColour(p1->specular, p2->specular, lambda8);

	// set clip flags
	POLY_setclip(np);

	return np;
}

// POLY_clip_against_nearplane
//
// clip poly against near clipping plane

SLONG POLY_clip_against_nearplane(POLY_Point** rptr, float* dptr, SLONG count, POLY_Point** wbuf)
{
	POLY_Point**	wptr = wbuf;

	POLY_Point*		p1;
	POLY_Point*		p2;

	SLONG ii;
	for (ii = 0; ii < count - 1; ii++)
	{
		p1 = rptr[ii];
		p2 = rptr[ii+1];

		if (dptr[ii] >= 0)
		{
			// good side
			*wptr++ = p1;
			if (dptr[ii+1] >= 0)
			{
				// also on good side
				continue;
			}
		}
		else
		{
			if (dptr[ii+1] < 0)
			{
				// also on bad side
				continue;
			}
		}

		*wptr++ = NewTweenVertex3D(p1, p2, dptr[ii] / (dptr[ii] - dptr[ii+1]));
	}

	p1 = rptr[ii];
	p2 = rptr[0];

	if (dptr[ii] >= 0)
	{
		*wptr++ = p1;
		if (dptr[0] < 0)
		{
			*wptr++ = NewTweenVertex3D(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]));
		}
	}
	else
	{
		if (dptr[0] >= 0)
		{
			*wptr++ = NewTweenVertex3D(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]));
		}
	}

	return wptr - wbuf;
}



#ifndef TARGET_DC

// POLY_clip_against_side_X
//
// clip poly against a side (left or right)

SLONG POLY_clip_against_side_X(POLY_Point** rptr, float* dptr, SLONG count, POLY_Point** wbuf, float xcoord)
{
	POLY_Point**	wptr = wbuf;

	POLY_Point*		p1;
	POLY_Point*		p2;

	SLONG ii;
	for (ii = 0; ii < count - 1; ii++)
	{
		p1 = rptr[ii];
		p2 = rptr[ii+1];

		if (dptr[ii] >= 0)
		{
			// good side
			*wptr++ = p1;
			if (dptr[ii+1] >= 0)
			{
				// also on good side
				continue;
			}
		}
		else
		{
			if (dptr[ii+1] < 0)
			{
				// also on bad side
				continue;
			}
		}

		*wptr++ = NewTweenVertex2D_X(p1, p2, dptr[ii] / (dptr[ii] - dptr[ii+1]), xcoord);
	}

	p1 = rptr[ii];
	p2 = rptr[0];

	if (dptr[ii] >= 0)
	{
		*wptr++ = p1;
		if (dptr[0] < 0)
		{
			*wptr++ = NewTweenVertex2D_X(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]), xcoord);
		}
	}
	else
	{
		if (dptr[0] >= 0)
		{
			*wptr++ = NewTweenVertex2D_X(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]), xcoord);
		}
	}

	return wptr - wbuf;
}

// POLY_clip_against_side_Y
//
// clip poly against a side (top or bottom)

SLONG POLY_clip_against_side_Y(POLY_Point** rptr, float* dptr, SLONG count, POLY_Point** wbuf, float ycoord)
{
	POLY_Point**	wptr = wbuf;

	POLY_Point*		p1;
	POLY_Point*		p2;

	SLONG ii;
	for (ii = 0; ii < count - 1; ii++)
	{
		p1 = rptr[ii];
		p2 = rptr[ii+1];

		if (dptr[ii] >= 0)
		{
			// good side
			*wptr++ = p1;
			if (dptr[ii+1] >= 0)
			{
				// also on good side
				continue;
			}
		}
		else
		{
			if (dptr[ii+1] < 0)
			{
				// also on bad side
				continue;
			}
		}

		*wptr++ = NewTweenVertex2D_Y(p1, p2, dptr[ii] / (dptr[ii] - dptr[ii+1]), ycoord);
	}

	p1 = rptr[ii];
	p2 = rptr[0];

	if (dptr[ii] >= 0)
	{
		*wptr++ = p1;
		if (dptr[0] < 0)
		{
			*wptr++ = NewTweenVertex2D_Y(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]), ycoord);
		}
	}
	else
	{
		if (dptr[0] >= 0)
		{
			*wptr++ = NewTweenVertex2D_Y(p1, p2, dptr[ii] / (dptr[ii] - dptr[0]), ycoord);
		}
	}

	return wptr - wbuf;
}

#endif //#ifndef TARGET_DC



#ifndef TARGET_DC
extern UBYTE sw_hack;
#endif



static float		s_DistBuffer[128];
static POLY_Point*	s_PtrBuffer[128];


// POLY_add_poly no longer works - system's been changed.
#if 0

// POLY_add_poly
//
// clip poly and write to the vertex buffer

void POLY_add_poly(POLY_Point** poly, SLONG poly_points, SLONG page)
{

	UBYTE	clip_or;
	UBYTE	clip_and;
	SLONG	ii;

	// get aggregate clip flags
	clip_or = 0;
	clip_and = 0xFF;
	for (ii = 0; ii < poly_points; ii++)
	{
		ASSERT(poly[ii]->MaybeValid());
		clip_or |= poly[ii]->clip;
		clip_and &= poly[ii]->clip;
	}

	if (clip_and & (POLY_CLIP_NEAR | POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM))
	{
		return;		  // is this triggering?
	}

	// initialize state for clipping
	POLY_Point**	rptr = poly;
	POLY_Point**	wptr = s_PtrBuffer;

	s_PointBufferOffset = 0;

	// remove flags that we don't care about
	clip_or &= s_ClipMask;

	// clip to nearplane
	if (clip_or & POLY_CLIP_NEAR)
	{
		for (ii = 0; ii < poly_points; ii++)
		{
			s_DistBuffer[ii] = rptr[ii]->z - POLY_Z_NEARPLANE;
		}

		poly_points = POLY_clip_against_nearplane(rptr, s_DistBuffer, poly_points, wptr);
		rptr = wptr;
		wptr += poly_points;

		if (!poly_points)
		{
			return;
		}

		// refresh clip flags
		clip_or = 0;
		clip_and = 0xFF;
		for (ii = 0; ii < poly_points; ii++)
		{
			ASSERT(rptr[ii]->MaybeValid());
			clip_or |= rptr[ii]->clip;
			clip_and &= rptr[ii]->clip;
		}
		clip_or &= s_ClipMask;
	}
	
#ifndef TARGET_DC
	if (sw_hack)
	{
		SLONG R[16];
		SLONG G[16];
		SLONG B[16];
		SLONG X[16];
		SLONG Y[16];
		SLONG Z[16];

		SLONG fog;

		SLONG fogout[16];

		ASSERT(poly_points <= 16);

		for (ii = 0; ii < poly_points; ii++)
		{
			fog = rptr[ii]->specular >> 24;

			R[ii] = ((rptr[ii]->colour >> 16) & 0xff) * fog >> 8;
			G[ii] = ((rptr[ii]->colour >>  8) & 0xff) * fog >> 8;
			B[ii] = ((rptr[ii]->colour >>  0) & 0xff) * fog >> 8;

			extern float not_private_smiley_xscale;
			extern float not_private_smiley_yscale;

			X[ii] = rptr[ii]->X * not_private_smiley_xscale;
			Y[ii] = rptr[ii]->Y * not_private_smiley_yscale;
			Z[ii] = rptr[ii]->z * 1024.0F;

			fogout[ii] = (fog == 0);
		}

		//
		// Add the fan a triangle at a time.
		//

		for (ii = 1; ii < poly_points - 1; ii++)
		{
			if (fogout[0] & fogout[ii] & fogout[ii + 1])
			{
				continue;
			}

			SW_add_triangle(
				X[     0], Y[     0], Z[     0], R[     0],G[     0],B[     0], rptr[     0]->u * 256, rptr[     0]->v * 256,
				X[ii + 0], Y[ii + 0], Z[ii + 0], R[ii + 0],G[ii + 0],B[ii + 0], rptr[ii + 0]->u * 256, rptr[ii + 0]->v * 256,
				X[ii + 1], Y[ii + 1], Z[ii + 1], R[ii + 1],G[ii + 1],B[ii + 1], rptr[ii + 1]->u * 256, rptr[ii + 1]->v * 256,
				page,
				rptr[ii]->colour >> 24);
		}

		return;
	}
#endif

#if _DEBUG
	// check that clip flags are correctly set
	for (ii = 0; ii < poly_points; ii++)
	{
		UBYTE	tmp = rptr[ii]->clip;
		POLY_setclip(rptr[ii]);
		if (tmp != rptr[ii]->clip)
		{
			TRACE("ERROR!  Polygon vertex clip flags not set\n(Hint: add ,TRUE parameter to the add polygon call if you set screen coordinates by hand, or call POLY_setclip())\n");
 			ASSERT(0);
			// now trace through the call - I've just realized this won't necessarily work ;-(
			POLY_transform(rptr[ii]->x, rptr[ii]->y, rptr[ii]->z, rptr[ii]);
		}
	}
#endif

#if !NO_CLIPPING_TO_THE_SIDES_PLEASE_BOB
	if (clip_or & (POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM))
	{
		if (clip_or & POLY_CLIP_LEFT)
		{
			LOG_ENTER ( POLY_side_clip )
			for (ii = 0; ii < poly_points; ii++)
			{
				s_DistBuffer[ii] = rptr[ii]->X - POLY_screen_clip_left;
			}

			poly_points = POLY_clip_against_side_X(rptr, s_DistBuffer, poly_points, wptr, POLY_screen_clip_left);
			rptr = wptr;
			wptr += poly_points;

			LOG_EXIT ( POLY_side_clip )
			if (!poly_points)	return;
		}

		if (clip_or & POLY_CLIP_RIGHT)
		{
			LOG_ENTER ( POLY_side_clip )
			for (ii = 0; ii < poly_points; ii++)
			{
				s_DistBuffer[ii] = POLY_screen_clip_right - rptr[ii]->X;
			}

			poly_points = POLY_clip_against_side_X(rptr, s_DistBuffer, poly_points, wptr, POLY_screen_clip_right);
			rptr = wptr;
			wptr += poly_points;

			LOG_EXIT ( POLY_side_clip )
			if (!poly_points)	return;
		}

		if (clip_or & POLY_CLIP_TOP)
		{
			LOG_ENTER ( POLY_side_clip )
			for (ii = 0; ii < poly_points; ii++)
			{
				s_DistBuffer[ii] = rptr[ii]->Y - POLY_screen_clip_top;
			}

			poly_points = POLY_clip_against_side_Y(rptr, s_DistBuffer, poly_points, wptr, POLY_screen_clip_top);
			rptr = wptr;
			wptr += poly_points;

			LOG_EXIT ( POLY_side_clip )
			if (!poly_points)	return;
		}

		if (clip_or & POLY_CLIP_BOTTOM)
		{
			LOG_ENTER ( POLY_side_clip )
			for (ii = 0; ii < poly_points; ii++)
			{
				s_DistBuffer[ii] = POLY_screen_clip_bottom - rptr[ii]->Y;
			}

			poly_points = POLY_clip_against_side_Y(rptr, s_DistBuffer, poly_points, wptr, POLY_screen_clip_bottom);
			rptr = wptr;
			wptr += poly_points;

			LOG_EXIT ( POLY_side_clip )
			if (!poly_points)	return;
		}
	}
#endif //#if !NO_CLIPPING_TO_THE_SIDES_PLEASE_BOB

	//
	// add to appropriate pages
	//

	POLY_Page[page].AddFan(rptr, poly_points);

	if (POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS)
	{
		POLY_Page[page + 1].AddFan(rptr, poly_points);
	}
}

#endif	// #if 1 ????!


void POLY_add_nearclipped_triangle(POLY_Point *pt[3], SLONG page, SLONG backface_cull)
{
	//
	// initialize state for clipping
	//


	POLY_Point** rptr = pt;
	POLY_Point** wptr = s_PtrBuffer;

	s_PointBufferOffset = 0;

	{
		SLONG i;
		SLONG j;
		SLONG laura;

		SLONG poly_points;

		s_DistBuffer[0] = rptr[0]->z - POLY_Z_NEARPLANE;
		s_DistBuffer[1] = rptr[1]->z - POLY_Z_NEARPLANE;
		s_DistBuffer[2] = rptr[2]->z - POLY_Z_NEARPLANE;

		poly_points = POLY_clip_against_nearplane(rptr, s_DistBuffer, 3, wptr);

		if (!poly_points)
		{
			return;
		}

		rptr = wptr;

		//
		// refresh clip flags
		//

		SLONG clip_and = 0xffffffff;

		for (i = 0; i < poly_points; i++)
		{
			POLY_setclip(rptr[i]);

			clip_and &= rptr[i]->clip;
		}

		if (clip_and & POLY_CLIP_OFFSCREEN)
		{
			return;
		}

		if (backface_cull && POLY_tri_backfacing(rptr[0], rptr[1], rptr[2]))
		{
			//
			// This triangle is backface culled.
			//

			return;
		}

	  second_page:;

#if WE_NEED_POLYBUFFERS_PLEASE_BOB

		PolyPage    *pp = &POLY_Page[page];
#ifdef TEX_EMBED
		// Do the indirection to the real poly page.
		PolyPage    *ppDrawn = pp->pTheRealPolyPage;
#else
		PolyPage    *ppDrawn = pp;
#endif

		PolyPoint2D *pv = ppDrawn->PointAlloc(3 + (poly_points - 3) * 3);

		POLY_Point  *ppt;
		PolyPoint2D *pv_first = pv;

		ppt = rptr[0];

		pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
		pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
		pv->SetUV      (ppt->u, ppt->v);
#endif
		pv->SetColour  (ppt->colour);
		pv->SetSpecular(ppt->specular);
		
		pv++;

		laura = 2;

		while(1)
		{
			ppt = rptr[laura - 1];

			pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
			pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
			pv->SetUV      (ppt->u, ppt->v);
#endif
			pv->SetColour  (ppt->colour);
			pv->SetSpecular(ppt->specular);
			
			pv++;

			ppt = rptr[laura];

			pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
			pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
			pv->SetUV      (ppt->u, ppt->v);
#endif
			pv->SetColour  (ppt->colour);
			pv->SetSpecular(ppt->specular);
			
			laura++;

			if (laura >= poly_points)
			{
				break;
			}

			pv++;

		   *pv = *pv_first;

			pv++;
		}

#else //#if WE_NEED_POLYBUFFERS_PLEASE_BOB
// The version with index buffers

		PolyPage    *pp = &POLY_Page[page];
#ifdef TEX_EMBED
		// Do the indirection to the real poly page.
		PolyPage    *ppDrawn = pp->pTheRealPolyPage;
#else
		PolyPage    *ppDrawn = pp;
#endif

		PolyPoint2D *pv = ppDrawn->FanAlloc( poly_points );

		POLY_Point  *ppt;
		PolyPoint2D *pv_first = pv;


		for ( i = 0; i < poly_points; i++ )
		{
			ppt = rptr[i];

			pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
	#ifdef TEX_EMBED
			pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
	#else
			pv->SetUV      (ppt->u, ppt->v);
	#endif
			pv->SetColour  (ppt->colour);
			pv->SetSpecular(ppt->specular);
			
			pv++;
		}

#endif //#else //#if WE_NEED_POLYBUFFERS_PLEASE_BOB

		if (POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS)
		{
			page += 1;

			goto second_page;
		}

	}

	return;
}



#ifdef TARGET_DC
void POLY_add_triangle(POLY_Point *pt[3], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
#else
void POLY_add_triangle_fast(POLY_Point *pt[3], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
#endif
{

	LOG_ENTER ( POLY_add_tri )

	if (generate_clip_flags)
	{
		POLY_setclip(pt[0]);
		POLY_setclip(pt[1]);
		POLY_setclip(pt[2]);
	}

	if (pt[0]->clip & pt[1]->clip & pt[2]->clip & POLY_CLIP_OFFSCREEN)
	{
		//
		// Offscreen.
		//

		LOG_EXIT ( POLY_add_tri )
		return;
	}

	if ((pt[0]->clip | pt[1]->clip | pt[2]->clip) & POLY_CLIP_NEAR)
	{
		//
		// Needs near-clipping...
		//

		POLY_add_nearclipped_triangle(pt, page, backface_cull);

		LOG_EXIT ( POLY_add_tri )
		return;
	}

	if (backface_cull && POLY_tri_backfacing(pt[0], pt[1], pt[2]))
	{
		//
		// This triangle is backface culled.
		//

		LOG_EXIT ( POLY_add_tri )
		return;
	}

  second_page:;

	PolyPage    *pp = &POLY_Page[page];
#ifdef TEX_EMBED
	// Do the indirection to the real poly page.
	PolyPage    *ppDrawn = pp->pTheRealPolyPage;
#else
	PolyPage    *ppDrawn = pp;
#endif

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoint2D *pv = ppDrawn->PointAlloc(3);
#else
	PolyPoint2D *pv = ppDrawn->FanAlloc(3);
#endif

	POLY_Point  *ppt;
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoly*		ppoly = ppDrawn->PolyBufAlloc();
	if (!ppoly)	return;
	ppoly->first_vertex = pv - ppDrawn->m_VertexPtr;
	ppoly->num_vertices = 3;
#endif

	ASSERT(pv);

	ppt = pt[0];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[1];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[2];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);


	if (POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS)
	{
		page += 1;

		goto second_page;
	}
	LOG_EXIT ( POLY_add_tri )
}

#ifdef TARGET_DC
void POLY_add_quad(POLY_Point *pt[4], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
#else
void POLY_add_quad_fast(POLY_Point *pt[4], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
#endif
{

	LOG_ENTER ( POLY_add_quad )
	
	if (generate_clip_flags)
	{
		POLY_setclip(pt[0]);
		POLY_setclip(pt[1]);
		POLY_setclip(pt[2]);
		POLY_setclip(pt[3]);
	}

	if (pt[0]->clip & pt[1]->clip & pt[2]->clip & pt[3]->clip & POLY_CLIP_OFFSCREEN)
	{
		//
		// Offscreen.
		//

		LOG_EXIT ( POLY_add_quad )
		return;
	}

	if ((pt[0]->clip | pt[1]->clip | pt[2]->clip | pt[3]->clip) & POLY_CLIP_NEAR)
	{
		POLY_Point *pt2[3] = {pt[1],pt[3],pt[2]};

		//
		// Needs near-clipping...
		//

		POLY_add_triangle(pt,  page, backface_cull, FALSE);
		POLY_add_triangle(pt2, page, backface_cull, FALSE);

		LOG_EXIT ( POLY_add_quad )
		return;
	}

	if (backface_cull)
	{
		SLONG cull;

		cull = 0;

		if (POLY_tri_backfacing(pt[0], pt[1], pt[2])) {cull |= 1;}
		if (POLY_tri_backfacing(pt[1], pt[3], pt[2])) {cull |= 2;}

		if (cull == 0)
		{
			//
			// Draw the whole quad.
			//
		}
		else
		if (cull == 3)
		{
			//
			// Backface cull the whole quad.
			//

			LOG_EXIT ( POLY_add_quad )
			return;
		}
		else
		if (cull == 1)
		{
			//
			// Draw just the second triangle.
			//

			LOG_EXIT ( POLY_add_quad )
			POLY_add_triangle(pt + 1, page, FALSE, FALSE);

			return;
		}
		else
		if (cull == 2)
		{
			//
			// Draw just the first triangle.
			//

			LOG_EXIT ( POLY_add_quad )
			POLY_add_triangle(pt, page, FALSE, FALSE);

			return;
		}
	}

  second_page:;

	PolyPage    *pp = &POLY_Page[page];
#ifdef TEX_EMBED
	// Do the indirection to the real poly page.
	PolyPage    *ppDrawn = pp->pTheRealPolyPage;
#else
	PolyPage    *ppDrawn = pp;
#endif


#if WE_NEED_POLYBUFFERS_PLEASE_BOB

	PolyPoint2D *pv = ppDrawn->PointAlloc(6);
	POLY_Point  *ppt;
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoly*		ppoly = ppDrawn->PolyBufAlloc();
	if (!ppoly)	return;
	ppoly->first_vertex = pv - ppDrawn->m_VertexPtr;
	ppoly->num_vertices = 3;
	ppoly = pp->PolyBufAlloc();
	if (!ppoly)	return;
	ppoly->first_vertex = pv - ppDrawn->m_VertexPtr + 3;
	ppoly->num_vertices = 3;
#endif

	ASSERT(pv);

	ppt = pt[0];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[1];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[2];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[3];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);


	pv[1] = pv[-1];
	pv[2] = pv[-2];

#else //#if WE_NEED_POLYBUFFERS_PLEASE_BOB
// The version with index buffers.

	PolyPoint2D *pv = ppDrawn->FanAlloc(4);
	POLY_Point  *ppt;
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoly*		ppoly = ppDrawn->PolyBufAlloc();
	if (!ppoly)	return;
	ppoly->first_vertex = pv - ppDrawn->m_VertexPtr;
	ppoly->num_vertices = 3;
	ppoly = ppDrawn->PolyBufAlloc();
	if (!ppoly)	return;
	ppoly->first_vertex = pv - ppDrawn->m_VertexPtr + 3;
	ppoly->num_vertices = 3;
#endif

	ASSERT(pv);

	ppt = pt[0];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[1];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[3];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

	pv++;

	ppt = pt[2];

	pv->SetSC      (ppt->X * pp->s_XScale, ppt->Y * pp->s_YScale, 1.0F - ppt->Z);
#ifdef TEX_EMBED
	pv->SetUV2      (ppt->u * pp->m_UScale + pp->m_UOffset, ppt->v * pp->m_VScale + pp->m_VOffset);
#else
	pv->SetUV      (ppt->u, ppt->v);
#endif
	pv->SetColour  (ppt->colour);
	pv->SetSpecular(ppt->specular);

#endif //#else //#if WE_NEED_POLYBUFFERS_PLEASE_BOB

	if (POLY_page_flag[page] & POLY_PAGE_FLAG_2PASS)
	{
		page += 1;

		goto second_page;
	}
	LOG_EXIT ( POLY_add_quad )
}




#if 0
//
// add a triangle to the poly list
//

void POLY_add_triangle_slow(POLY_Point *pp[3], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
{
	ULONG counter_start;
	ULONG counter_end;

	_asm
	{
		rdtsc
		mov	counter_start, eax
	}

	{
		if (generate_clip_flags)
		{
			POLY_setclip(pp[0]);
			POLY_setclip(pp[1]);
			POLY_setclip(pp[2]);
		}
		else
		{
			ASSERT(pp[0]->MaybeValid());
			ASSERT(pp[1]->MaybeValid());
			ASSERT(pp[2]->MaybeValid());
		}

		if (backface_cull && POLY_tri_backfacing(pp[0], pp[1], pp[2]))
		{
			//
			// This triangle is backface culled.
			//
		}
		else
		{
			POLY_add_poly(pp, 3, page);
		}
	}

	_asm
	{
		rdtsc
		mov	counter_end, eax
	}

	extern ULONG AENG_poly_add_quad_time;

	AENG_poly_add_quad_time += counter_end - counter_start;
}

//
// add a quad to the poly list; note that vertices clockwise go 0,1,3,2 in this call
//

extern	SLONG TEXTURE_set;
extern	UBYTE TEXTURE_dontexist[];

void POLY_add_quad_slow(POLY_Point *pp[4], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
{

	LOG_ENTER ( POLY_add_quad )

#if 0	// or #ifdef _DEBUG or something, you lazy gits
	if(ShiftFlag && Keys[KB_Q])
	if(pp[0]->z<0.3)
	if(page<64*8)
	{
		CBYTE	str[10];
		if(page<256)
			sprintf(str,"W%d %d",TEXTURE_set,page);
		else
			sprintf(str,"W%d S%d",TEXTURE_set,page);
//extern	FONT2D_DrawString(CBYTE*chr, ULONG x, ULONG y, ULONG rgb=0xffffff, SLONG scale=16, SLONG page=POLY_PAGE_FONT2D, SWORD fade=0);
		FONT2D_DrawString(str,pp[0]->X,pp[0]->Y,0xff0000);
	}
#endif
	

/*
	if(ShiftFlag)
	{
		if (TEXTURE_texture[page].Type == D3DTEXTURE_TYPE_UNUSED)
			ASSERT(0);
		if(TEXTURE_dontexist[page])
			ASSERT(0);
	}
*/

//	if(page!=128)
//		return;

	{
		if (generate_clip_flags)
		{
			POLY_setclip(pp[0]);
			POLY_setclip(pp[1]);
			POLY_setclip(pp[2]);
			POLY_setclip(pp[3]);
		}
		else
		{
			ASSERT(pp[0]->MaybeValid());
			ASSERT(pp[1]->MaybeValid());
			ASSERT(pp[2]->MaybeValid());
			ASSERT(pp[3]->MaybeValid());
		}

		if (backface_cull)
		{
			bool	first;
			bool	second;

			first  = POLY_tri_backfacing(pp[0], pp[1], pp[2]);
			second = POLY_tri_backfacing(pp[1], pp[3], pp[2]);

			if (first && second)
			{
			}
			else
			{
				if (!first)		POLY_add_poly(pp, 3, page);
				if (!second)	POLY_add_poly(pp+1, 3, page);
			}
		}
		else
		{
			#if USE_D3D_VBUF
				// bent quads break clipping - submit as two triangles
				POLY_add_poly(pp, 3, page);
				POLY_add_poly(pp+1, 3, page);
			#else
				POLY_Point*	tmp;

				tmp = pp[2]; pp[2] = pp[3]; pp[3] = tmp;
				POLY_add_poly(pp, 4, page);
				tmp = pp[2]; pp[2] = pp[3]; pp[3] = tmp;
			#endif
		}
	}

}


#endif //#if 0


#ifndef TARGET_DC

void POLY_add_quad(POLY_Point *pp[4], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
{
	ULONG counter_start;
	ULONG counter_end;

	_asm
	{
		rdtsc
		mov	counter_start, eax
	}

#if 0
	if (!Keys[KB_F8])
	{
		POLY_add_quad_slow(pp, page, backface_cull, generate_clip_flags);
	}
	else
#endif
	{
		POLY_add_quad_fast(pp, page, backface_cull, generate_clip_flags);
	}

	_asm
	{
		rdtsc
		mov	counter_end, eax
	}

	extern ULONG AENG_poly_add_quad_time;

	AENG_poly_add_quad_time += counter_end - counter_start;
}

void POLY_add_triangle(POLY_Point *pp[4], SLONG page, SLONG backface_cull, SLONG generate_clip_flags)
{
	ULONG counter_start;
	ULONG counter_end;

	_asm
	{
		rdtsc
		mov	counter_start, eax
	}

#if 0
	if (!Keys[KB_F8])
	{
		POLY_add_triangle_slow(pp, page, backface_cull, generate_clip_flags);
	}
	else
#endif
	{
		POLY_add_triangle_fast(pp, page, backface_cull, generate_clip_flags);
	}

	_asm
	{
		rdtsc
		mov	counter_end, eax
	}

	extern ULONG AENG_poly_add_quad_time;

	AENG_poly_add_quad_time += counter_end - counter_start;
}

#endif //#ifndef TARGET_DC




float POLY_world_length_to_screen(float world_length)
{
	float view_length   = world_length * POLY_cam_over_view_dist;
	float screen_length = view_length  * POLY_screen_mul_x;

	return screen_length;
}

float POLY_approx_len(float dx, float dy)
{
	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	return (fabsf(dx) > fabsf(dy)) ? fabsf(dx) + 0.414F * fabsf(dy) : fabsf(dy) + 0.414F * fabsf(dx);
}

//
// create 4 points for a cylinder; only X,Y,Z, colour & specular are set up
//

void POLY_create_cylinder_points(POLY_Point* p1, POLY_Point* p2, float width, POLY_Point* pout)
{
	float	dx,dy;			// screen normal vector for line
	float	len,overlen;	// line length/reciprocal
	float	dx1,dy1;		// perturbation vector for p1
	float	dx2,dy2;		// perturbation vector for p2

	ASSERT(p1->IsValid());
	ASSERT(p2->IsValid());

	width *= POLY_cam_over_view_dist;	// move to view space

	//
	// get normalized vector along the line
	//

	dx = p2->X - p1->X;
	dy = p2->Y - p1->Y;

	len = POLY_approx_len(dx, dy);
	overlen = 1.0F / len;

	dx *= overlen;
	dy *= overlen;

	//
	// get perturbation vectors
	//

	dx1 = -dy * width * p1->Z;
	dy1 =  dx * width * p1->Z;

	dx2 = -dy * width * p2->Z;
	dy2 =  dx * width * p2->Z;

	//
	// copy and shift points
	//

	pout[0] = *p1;
	pout[0].X += dx1;
	pout[0].Y += dy1;

	pout[1] = *p1;
	pout[1].X -= dx1;
	pout[1].Y -= dy1;

	pout[2] = *p2;
	pout[2].X += dx2;
	pout[2].Y += dy2;

	pout[3] = *p2;
	pout[3].X -= dx2;
	pout[3].Y -= dy2;
}

void POLY_add_line_tex_uv(POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front)
{
	float dx;
	float dy;

	float dx1;
	float dy1;

	float dx2;
	float dy2;

	float vw1;
	float vw2;

	float sw1;
	float sw2;

	float len;
	float overlen;

	//
	// Both points must be transformed
	//

	if (p1->NearClip() || p2->NearClip())	return;

	ASSERT(p1->IsValid());
	ASSERT(p2->IsValid());

	POLY_Point	pt[4];
	POLY_Point*	ppt[4];

	dx = p2->X - p1->X;
	dy = p2->Y - p1->Y;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (fabsf(dx) > fabsf(dy)) ? fabsf(dx) + 0.414F * fabsf(dy) : fabsf(dy) + 0.414F * fabsf(dx);
	overlen = 1.0F / len;
	
	dx *= overlen;
	dy *= overlen;

	//
	// Convert widths in the world to widths in view space.
	//

	vw1  = width1 * POLY_cam_over_view_dist;
	vw2  = width2 * POLY_cam_over_view_dist;

	//
	// Convert widths in view space to widths on screen.
	//

	sw1 = POLY_screen_mul_x * vw1 * p1->Z;
	sw2 = POLY_screen_mul_x * vw2 * p2->Z;

	dx1 = -dy * sw1;
	dy1 = +dx * sw1;

	dx2 = -dy * sw2;
	dy2 = +dx * sw2;

	if (sort_to_front)
	{
		p1->Z = 1.0F;
		p1->z = 0.0F;

		p2->Z = 1.0F;
		p2->z = 0.0F;
	}

	//
	// Create the four points.
	//

	pt[0] = *p1;
	pt[1] = *p1;

	pt[0].X -= dx1;
	pt[0].Y -= dy1;

	pt[1].X += dx1;
	pt[1].Y += dy1;

	pt[0].u = p2->u;

	pt[2] = *p2;
	pt[3] = *p2;

	pt[2].X += dx2;
	pt[2].Y += dy2;
	
	pt[3].X -= dx2;
	pt[3].Y -= dx2;

	pt[2].u = p1->u;

	ppt[0] = &pt[0];
	ppt[1] = &pt[1];
	ppt[2] = &pt[3];
	ppt[3] = &pt[2];

#if 0
	POLY_setclip(ppt[0]);
	POLY_setclip(ppt[1]);
	POLY_setclip(ppt[2]);
	POLY_setclip(ppt[3]);
#endif

	POLY_add_quad(ppt, page, FALSE, TRUE);
}


void POLY_add_line_tex(POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front)
{
	p1->u = p1->v = 0;
	p2->u = p2->v = 1;

	POLY_add_line_tex_uv(p1, p2, width1, width2, page, sort_to_front);
}


void POLY_add_line(POLY_Point *p1, POLY_Point *p2, float width1, float width2, SLONG page, UBYTE sort_to_front)
{
	float dx;
	float dy;

	float dx1;
	float dy1;

	float dx2;
	float dy2;

	float vw1;
	float vw2;

	float sw1;
	float sw2;

	float len;
	float overlen;

	//
	// Both points must be transformed
	//

	if (p1->NearClip() || p2->NearClip())	return;

	ASSERT(p1->IsValid() & p2->IsValid());

	POLY_Point	pt[4];
	POLY_Point*	ppt[4];

	dx = p2->X - p1->X;
	dy = p2->Y - p1->Y;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (fabsf(dx) > fabsf(dy)) ? fabsf(dx) + 0.414F * fabsf(dy) : fabsf(dy) + 0.414F * fabsf(dx);
	overlen = 1.0F / len;
	
	dx *= overlen;
	dy *= overlen;

	//
	// Convert widths in the world to widths in view space.
	//

	vw1  = width1 * POLY_cam_over_view_dist;
	vw2  = width2 * POLY_cam_over_view_dist;

	//
	// Convert widths in view space to widths on screen.
	//

	sw1 = POLY_screen_mul_x * vw1 * p1->Z;
	sw2 = POLY_screen_mul_x * vw2 * p2->Z;

	dx1 = -dy * sw1;
	dy1 = +dx * sw1;

	dx2 = -dy * sw2;
	dy2 = +dx * sw2;

	if (sort_to_front)
	{
#ifdef TARGET_DC
		p1->Z = 0.9999F;
		p1->z = 0.0001F;

		p2->Z = 0.9999F;
		p2->z = 0.0001F;
#else
		p1->Z = 1.0F;
		p1->z = 0.0F;

		p2->Z = 1.0F;
		p2->z = 0.0F;
#endif
	}

	//
	// Create the four points.
	//

	pt[0] = *p1;
	pt[1] = *p1;
	pt[2] = *p2;
	pt[3] = *p2;

	for (int ii = 0; ii < 4; ii++)
	{
		pt[ii].u = 0;
		pt[ii].v = 0;
	}

	pt[0].X -= dx1;
	pt[0].Y -= dy1;
	pt[1].X += dx1;
	pt[1].Y += dy1;

	pt[2].X += dx2;
	pt[2].Y += dy2;
	pt[3].X -= dx2;
	pt[3].Y -= dy2;

	ppt[0] = &pt[0];
	ppt[1] = &pt[1];
	ppt[2] = &pt[3];
	ppt[3] = &pt[2];

#if 0
	POLY_setclip(ppt[0]);
	POLY_setclip(ppt[1]);
	POLY_setclip(ppt[2]);
	POLY_setclip(ppt[3]);
#endif

	POLY_add_quad(ppt, page, FALSE, TRUE);
}

//
// p1 is top left
//
void POLY_add_rect(POLY_Point *p1, SLONG width,SLONG height,  SLONG page, UBYTE sort_to_front)
{

	//
	// Both points must be transformed
	//

	if (p1->NearClip() )	return;

	ASSERT(p1->IsValid() );

	POLY_Point	pt[4];
	POLY_Point*	ppt[4];


	if (sort_to_front)
	{
		p1->Z = 1.0F;
		p1->z = 0.0F;

	}

	//
	// Create the four points.
	//

	pt[0] = *p1;
	pt[1] = *p1;
	pt[2] = *p1;
	pt[3] = *p1;

	for (int ii = 0; ii < 4; ii++)
	{
		pt[ii].u = 0;
		pt[ii].v = 0;
	}



	pt[1].X += (float)width;
	pt[3].Y += (float)height;
	pt[2].X += (float)width;
	pt[2].Y += (float)height;

	ppt[0] = &pt[0];
	ppt[1] = &pt[1];
	ppt[2] = &pt[3];
	ppt[3] = &pt[2];

#if 0
	POLY_setclip(ppt[0]);
	POLY_setclip(ppt[1]);
	POLY_setclip(ppt[3]);
#endif

	POLY_add_quad(ppt, page, FALSE, TRUE);
}

void  POLY_add_line_2d(float sx1, float sy1, float sx2, float sy2, ULONG colour)
{
	float dx;
	float dy;

	float len;
	float overlen;

	POLY_Point	pt[4];
	POLY_Point*	ppt[4];

	dx = sx2 - sx1;
	dy = sy2 - sy1;

	//
	// Hmm... I guess that .414F is better than 0.500F
	//

	len     = (fabsf(dx) > fabsf(dy)) ? fabsf(dx) + 0.414F * fabsf(dy) : fabsf(dy) + 0.414F * fabsf(dx);
	overlen = 0.5F / len;
	
	dx *= overlen;
	dy *= overlen;

	//
	// Create the four points.
	//

	for (int ii = 0; ii < 4; ii++)
	{
		pt[ii].u = 0;
		pt[ii].v = 0;
		pt[ii].colour = colour;
		pt[ii].specular = 0;
	}

	pt[0].X = sx1 - dy;
	pt[0].Y = sy1 + dx;
	pt[1].X = sx1 + dy;
	pt[1].Y = sy1 - dx;
	pt[2].X = sx2 + dy;
	pt[2].Y = sy2 - dx;
	pt[3].X = sx2 - dy;
	pt[3].Y = sy2 + dx;

	ppt[0] = &pt[0];
	ppt[1] = &pt[1];
	ppt[2] = &pt[3];
	ppt[3] = &pt[2];

#if 0
	POLY_setclip(ppt[0]);
	POLY_setclip(ppt[1]);
	POLY_setclip(ppt[3]);
#endif

	POLY_add_quad(ppt, POLY_PAGE_COLOUR, FALSE, TRUE);
}


float POLY_clip_left;
float POLY_clip_right;
float POLY_clip_top;
float POLY_clip_bottom;

void POLY_clip_line_box(float sx1, float sy1, float sx2, float sy2)
{
	POLY_clip_left	 = sx1;
	POLY_clip_right	 = sx2;
	POLY_clip_top	 = sy1;
	POLY_clip_bottom = sy2;
}

void POLY_clip_line_add(float sx1, float sy1, float sx2, float sy2, ULONG colour)
{
	UBYTE clip1 = 0;
	UBYTE clip2 = 0;
	UBYTE clip_and;
	UBYTE clip_or;
	UBYTE clip_xor;

	float along;

	     if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
	else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

	     if (sx2 < POLY_clip_left)   {clip2 |= POLY_CLIP_LEFT;}
	else if (sx2 > POLY_clip_right)  {clip2 |= POLY_CLIP_RIGHT;}

	     if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
	else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

	     if (sy2 < POLY_clip_top)    {clip2 |= POLY_CLIP_TOP;}
	else if (sy2 > POLY_clip_bottom) {clip2 |= POLY_CLIP_BOTTOM;}

	clip_and = clip1 & clip2;
	clip_or  = clip1 | clip2;
	clip_xor = clip1 ^ clip2;

	if (clip_and)
	{
		//
		// Reject the line.
		//

		return;
	}

	#define SWAP_UB(q,w) {UBYTE temp = (q); (q) = (w); (w) = temp;}

	if (clip_or)
	{
		//
		// The line needs clipping.
		//

		if (clip_xor & POLY_CLIP_LEFT)
		{
			if (clip2 & POLY_CLIP_LEFT)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_left - sx1) / (sx2 - sx1);
			sx1    =  POLY_clip_left;
			sy1    =  sy1 + along * (sy2 - sy1);
			clip1 &= ~POLY_CLIP_LEFT;

				 if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
			else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_RIGHT)
		{
			if (clip2 & POLY_CLIP_RIGHT)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_right - sx1) / (sx2 - sx1);
			sx1    =  POLY_clip_right;
			sy1    =  sy1 + along * (sy2 - sy1);
			clip1 &= ~POLY_CLIP_RIGHT;

				 if (sy1 < POLY_clip_top)    {clip1 |= POLY_CLIP_TOP;}
			else if (sy1 > POLY_clip_bottom) {clip1 |= POLY_CLIP_BOTTOM;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_TOP)
		{
			if (clip2 & POLY_CLIP_TOP)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_top - sy1) / (sy2 - sy1);
			sx1    =  sx1 + along * (sx2 - sx1);
			sy1    =  POLY_clip_top;
			clip1 &= ~POLY_CLIP_TOP;

				 if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
			else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}

		if (clip_xor & POLY_CLIP_BOTTOM)
		{
			if (clip2 & POLY_CLIP_BOTTOM)
			{
				SWAP_FL(sx1, sx2);
				SWAP_FL(sy1, sy2);
				SWAP_UB(clip1, clip2);
			}

			along  = (POLY_clip_bottom - sy1) / (sy2 - sy1);
			sx1    =  sx1 + along * (sx2 - sx1);
			sy1    =  POLY_clip_bottom;
			clip1 &= ~POLY_CLIP_BOTTOM;

				 if (sx1 < POLY_clip_left)   {clip1 |= POLY_CLIP_LEFT;}
			else if (sx1 > POLY_clip_right)  {clip1 |= POLY_CLIP_RIGHT;}

			if (clip1 & clip2)
			{
				return;
			}			

			clip_xor = clip1 ^ clip2;
		}
	}

	if (clip1 | clip2)
	{
		return;
	}

	//
	// Phew! Add the clipped line.
	//

	POLY_add_line_2d(sx1, sy1, sx2, sy2, colour);
}

#if 0

SLONG POLY_shared_page;
SLONG POLY_shared_base_index;

void POLY_add_shared_start(SLONG page)
{
	POLY_Page *pa;
	D3DTLVERTEX *tl;

	ASSERT(WITHIN(page, 0, POLY_NUM_PAGES - 1));

	pa = &POLY_page[page];

	POLY_shared_page       = page;
	POLY_shared_base_index = pa->vertex_upto;
}

void POLY_add_shared_point(POLY_Point *pp)
{
	POLY_Page *pa;
	D3DTLVERTEX *tl;

	ASSERT(WITHIN(POLY_shared_page, 0, POLY_NUM_PAGES - 1));

	pa = &POLY_page[POLY_shared_page];

	//
	// Is there enough room in the buffers for this point?
	//

	if (pa->vertex_upto >= pa->size)
	{
		if (!POLY_grow_page(pa))
		{
			return;
		}
	}

	tl = &pa->vertex[pa->vertex_upto];

	tl->sx       = pp->X;
	tl->sy       = pp->Y;
	tl->sz       = 1.0F - pp->Z;	// 1 - 1/z z-buffer...
	tl->rhw      = pp->Z;
	tl->tu       = pp->u;
	tl->tv       = pp->v;
	tl->color    = pp->colour;	// Bloody Americans!
	tl->specular = pp->specular;

	pa->vertex_upto += 1;
}

void POLY_add_shared_tri(UWORD p1, UWORD p2, UWORD p3)
{
	POLY_Page *pa;
	D3DTLVERTEX *tl;

	ASSERT(WITHIN(POLY_shared_page, 0, POLY_NUM_PAGES - 1));

	pa = &POLY_page[POLY_shared_page];

	//
	// Is there enough room in the buffers for this point?
	//

	if (pa->index_upto + 3 > pa->size)
	{
		if (!POLY_grow_page(pa))
		{
			return;
		}
	}

	ASSERT(WITHIN(POLY_shared_base_index + p1, 0, pa->vertex_upto - 1));
	ASSERT(WITHIN(POLY_shared_base_index + p2, 0, pa->vertex_upto - 1));
	ASSERT(WITHIN(POLY_shared_base_index + p3, 0, pa->vertex_upto - 1));

	pa->index[pa->index_upto + 0] = POLY_shared_base_index + p1;
	pa->index[pa->index_upto + 1] = POLY_shared_base_index + p2;
	pa->index[pa->index_upto + 2] = POLY_shared_base_index + p3;

	pa->index_upto += 3;
}

#endif

// POLY_frame_draw
//
// draw all the poly pages

#ifdef TEX_EMBED
extern SLONG PageOrder[POLY_NUM_PAGES];
#endif


void POLY_frame_draw(SLONG draw_shadow_page, SLONG draw_text_page)
{
	SLONG	i;
	SLONG	j;
	SLONG	k;

	PolyPage	*pa;


	static int iPageNumberToClear = 0;


	// SPONG
#if USE_TOMS_ENGINE_PLEASE_BOB


// These will have been done in POLY_frame_init().
	//POLY_init_render_states();
//#ifndef TARGET_DC
	//BEGIN_SCENE;
//#endif


#else
	ULONG fog_colour;

	// Already done if using Tom's engine.
//	BreakTime("FRAMEDRAW start");
	POLY_init_render_states();
//	BreakTime("FRAMEDRAW init render states");

	//
	// Start the scene.
	//
	BEGIN_SCENE;

	if ((GAME_FLAGS & GF_SEWERS) || (GAME_FLAGS & GF_INDOORS))
	{
		fog_colour = 0;
	}
	else
	{
		if(fade_black)
		{
			fog_colour=0;
		}
		else
		{
			fog_colour = 
				(NIGHT_sky_colour.red   << 16) |
				(NIGHT_sky_colour.green <<  8) |
				(NIGHT_sky_colour.blue  <<  0);
		}

		if (draw_3d)
		{
			SLONG white = NIGHT_sky_colour.red + NIGHT_sky_colour.green + NIGHT_sky_colour.blue;

			white /= 3;

			fog_colour = 
				(white << 16) |
				(white <<  8) |
				(white <<  0);
		}
	}


	// set default render state
	DefRenderState.InitScene(fog_colour);
//	BreakTime("FRAMEDRAW init scene");


#endif

	//
	// Draw the sky first...
	// 

#ifndef TARGET_DC
	pa = &POLY_Page[POLY_PAGE_SKY];

	if (pa->NeedsRendering())
	{
		pa->RS.SetChanged();
		pa->Render(the_display.lp_D3D_Device);
	}
#endif
//	BreakTime("FRAMEDRAW done sky");

#ifndef TARGET_DC
// DC sorts for us.

	if (PolyPage::AlphaSortEnabled())
	{
		//
		// Draw opaque polygon pages
		//
//		BreakTime("FRAMEDRAW start alphasort");

		
#ifdef TEX_EMBED
		for (i = 0; i <= iPolyNumPagesRender; i++)		// <= because we skip POLY_PAGE_COLOUR...
		{
			k = PageOrder[i];
#else
		for (i = 0; i <= POLY_NUM_PAGES; i++)		// <= because we skip POLY_PAGE_COLOUR...
		{
			k = i;
#endif
			//
			// Do POLY_PAGE_COLOUR first!
			//

			if (i == 0)
			{
				k =  POLY_PAGE_COLOUR;
			}
			else
			{
				k = i - 1;

				if (k == POLY_PAGE_COLOUR)
				{
					continue;
				}
			}

			pa = &POLY_Page[k];

			if (!pa->NeedsRendering())
			{
				continue;
			}

			if (k == POLY_PAGE_TEXT && !draw_text_page)
			{
				//
				// Ignore the text page.
				//

				continue;
			}

			if (!pa->RS.NeedsSorting() || (k == POLY_PAGE_PUDDLE))
			{
				// set render state
				pa->RS.SetChanged();

				if (POLY_force_additive_alpha)
				{
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZBIAS, 2);
				}
/*
				if(INDOORS_INDEX)
				{
				  //poo poo poo for fadeing current floor of building



					SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATEALPHA);
					SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
					SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
					SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE,FALSE);
				}
*/

#ifndef TARGET_DC
#ifndef	FINAL
#ifdef EDITOR
				if (Keys[KB_P1]&&!CUTSCENE_edit_wnd)
#else
				if (Keys[KB_P1])//&&allow_debug_keys)
#endif
				{
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE,FALSE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,FALSE);
				}
#endif
#endif

				//
				// and render the polygons
				//

				pa->Render(the_display.lp_D3D_Device);
			}
		}
//		BreakTime("FRAMEDRAW end alphasort");

		//
		// now draw the alpha polygons
		//

#if 1	// do it the bucket-sort way globally

		//
		// generate the buckets
		//

//		BreakTime("FRAMEDRAW start buckets");
		PolyPoly*	buckets[2048];

		for (i = 0; i < 2048; i++)	buckets[i] = NULL;

		for (i = 0; i < POLY_NUM_PAGES; i++)
		{
			pa = &POLY_Page[i];

			if (!pa->NeedsRendering())						continue;
			if (i == POLY_PAGE_SHADOW && !draw_shadow_page)	continue;
			if (!pa->RS.NeedsSorting())						continue;
			if (i == POLY_PAGE_PUDDLE)						continue;

			pa->AddToBuckets(buckets, 2048);
		}
//		BreakTime("FRAMEDRAW mid buckets");

		//
		// render the buckets
		//

		for (i = 0; i < 2048; i++)
		{
			PolyPoly*	p = buckets[i];

			while (p)
			{
				p->page->RS.SetChanged();
				p->page->DrawSinglePoly(p, the_display.lp_D3D_Device);
				p = p->next;
			}
		}

		for (i = 0; i < POLY_NUM_PAGES; i++)
		{
			pa = &POLY_Page[i];

			if (pa->RS.NeedsSorting())	pa->RS.ResetTempTransparent();
		}
//		BreakTime("FRAMEDRAW end buckets");

#else	// do it page by page
//		BreakTime("FRAMEDRAW start page by page");


#ifdef TEX_EMBED
		for (i = 0; i <= iPolyNumPagesRender; i++)		// <= because we skip POLY_PAGE_COLOUR...
		{
			k = PageOrder[i];
#else
		for (i = 0; i < POLY_NUM_PAGES; i++)
		{
			k = i;
#endif

			pa = &POLY_Page[k];

			if (!pa->NeedsRendering())
			{
				continue;
			}

			if (k == POLY_PAGE_SHADOW && !draw_shadow_page)
			{
				//
				// Ignore the shadow page.
				//

				continue;
			}

			if (pa->RS.NeedsSorting())
			{
				// set render state
				pa->RS.SetChanged();

				if (POLY_force_additive_alpha)
				{
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZBIAS, 2);
				}

				if(INDOORS_INDEX)
				{
				  //poo poo poo for fadeing current floor of building



					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATEALPHA);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE,FALSE);
				}

#ifdef EDITOR
				if (Keys[KB_P1]&&!CUTSCENE_edit_wnd)
				{
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE,FALSE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
					REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,FALSE);
				}
#endif

				//
				// sort and render the polygons
				//

				pa->SortBackFirst();
				pa->Render(the_display.lp_D3D_Device);
			}
		}
//		BreakTime("FRAMEDRAW end page by page");

#endif

	}
	else
#endif //#ifndef TARGET_DC
	{
		//
		// draw all the polygons at once
		//
//		BreakTime("FRAMEDRAW start all polys at once");


#ifdef TEX_EMBED
		for (i = 0; i < iPolyNumPagesRender; i++)
		{
			k = PageOrder[i];
#else
		for (i = 0; i < POLY_NUM_PAGES; i++)
		{
			k = i;
#endif

			pa = &POLY_Page[k];

			if (!pa->NeedsRendering())
			{
				continue;
			}

//			ASSERT ( draw_text_page );
//			ASSERT ( draw_shadow_page );

			// set render state
			pa->RS.SetChanged();

			//
			// render the polygons
			//

			pa->Render(the_display.lp_D3D_Device);
		}
//		BreakTime("FRAMEDRAW end all polys at once");

	}

	END_SCENE;


	// And clear out a few pages' VBs and IBs.
	// This stops a page that was drawing a lot a while ago from
	// hogging all the memory when it goes out of scene.
	for ( i = 0; i < 3; i++ )
	{
		iPageNumberToClear++;
		if ( iPageNumberToClear >= POLY_NUM_PAGES )
		{
			iPageNumberToClear = 0;
		}
		POLY_Page[iPageNumberToClear].Clear();
	}


	/*

//
//	Guy Demo Dodge!!!
//

	if(GAME_STATE&GS_ATTRACT_MODE)
	{
extern	void	draw_text_at(float x,float y,CBYTE *message,SLONG font_id);
	extern BOOL  text_fudge;
	extern ULONG text_colour;

		text_fudge  = FALSE;
		text_colour = 0x00ffffff;
		draw_text_at(200,150,"Press Anything To Play",0);

extern void	show_text(void);
		show_text();
	}

	*/
//	TRACE("poly frame draw EXIT\n");
}


void POLY_frame_draw_odd()
{
	SLONG i;
	SLONG j;

	PolyPage	*pa;


#ifdef TARGET_DC
	// I'd like to know.
	ASSERT ( FALSE );
#endif

	//
	// Start the scene.
	//


	BEGIN_SCENE;


	// Sets the actual hardware RS, and also keeps the cache informed.
#define FORCE_SET_RENDER_STATE(t,s) RenderState::s_State.SetRenderState(t,s); REALLY_SET_RENDER_STATE(t,s)
#define FORCE_SET_TEXTURE(s) RenderState::s_State.SetTexture(s); REALLY_SET_TEXTURE(s)

	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE,TRUE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE,TRUE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE,TRUE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_SUBPIXEL,TRUE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,FALSE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);//ALPHA);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);//SRCALPHA);
	FORCE_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);//INVSRCALPHA);

	for (i = 0; i < TEXTURE_page_num_standard; i++)
	{
		ASSERT(WITHIN(i, 0, POLY_NUM_PAGES - 1));

		pa = &POLY_Page[i];

		if (!pa->NeedsRendering())
		{
			continue;
		}

		//
		// The is one of the TEXTURE modules pages...
		//

		FORCE_SET_TEXTURE(TEXTURE_get_handle(i));

		//
		// Do the actual draw primitive call.
		//

		pa->Render(the_display.lp_D3D_Device);
	}

	if (POLY_Page[POLY_PAGE_SKY].NeedsRendering())
	{
		FORCE_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FALSE);
		FORCE_SET_TEXTURE(TEXTURE_get_handle(TEXTURE_page_sky));

		POLY_Page[POLY_PAGE_SKY].Render(the_display.lp_D3D_Device);
	}

	if (POLY_Page[POLY_PAGE_MOON].NeedsRendering())
	{
		FORCE_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FALSE);
		FORCE_SET_TEXTURE(TEXTURE_get_handle(TEXTURE_page_moon));

		POLY_Page[POLY_PAGE_MOON].Render(the_display.lp_D3D_Device);
	}

	END_SCENE;
}


void POLY_frame_draw_puddles()
{


	PolyPage    *pp = &POLY_Page[POLY_PAGE_PUDDLE];
#ifdef TEX_EMBED
	// Do the indirection to the real poly page.
	PolyPage    *ppDrawn = pp->pTheRealPolyPage;
#else
	PolyPage    *ppDrawn = pp;
#endif

#ifdef TARGET_DC
	// I'd like to know.
	ASSERT ( FALSE );
#endif


	if (pp->NeedsRendering())
	{
		BEGIN_SCENE;

		// state in polyrenderstate.cpp
		pp->RS.InitScene(0);

		//
		// Do the actual draw primitive call.
		//

		pp->Render(the_display.lp_D3D_Device);

		END_SCENE;
	}
}


// No sewers - just here for error-checking.
#ifdef DEBUG

//
// Comparison function for sewer water polygons.
//

void POLY_sort_sewater_page()
{
#ifndef TARGET_DC
	POLY_Page[POLY_PAGE_SEWATER].SortBackFirst();
#endif
}

void POLY_frame_draw_sewater()
{
	PolyPage *pa = &POLY_Page[POLY_PAGE_SEWATER];

	if (pa->NeedsRendering())
	{
#if 1
		// Shouldn't be any sewers.
		ASSERT(FALSE);
#else
		BEGIN_SCENE;

		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,TRUE);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATEALPHA);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
		REALLY_SET_TEXTURE(TEXTURE_get_handle(TEXTURE_page_water));
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
		REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_WRAP);

		//
		// Do the actual draw primitive call.
		//

		pa->Render(the_display.lp_D3D_Device);

		END_SCENE;
#endif
	}
}

#endif


SLONG POLY_get_sphere_circle(
		float  world_x,
		float  world_y,
		float  world_z,
		float  world_radius,
		SLONG *screen_x,
		SLONG *screen_y,
		SLONG *screen_radius)
{
	float vw;
	float width;

	POLY_Point pp;

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &pp);

	if (pp.IsValid())
	{
		//
		// Find the screen width for this world width.
		//

		vw    = world_radius * POLY_cam_over_view_dist;
		width = vw * pp.Z * POLY_screen_mul_x;

		*screen_x      = SLONG(pp.X);
		*screen_y      = SLONG(pp.Y);
		*screen_radius = SLONG(width);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#if 0

void POLY_frame_draw_focused(float focus)
{
	SLONG i;
	SLONG j;

	float df;

	PolyPage	*pa;

	//
	// Set-up renderstates.
	// 

	SET_RENDER_STATE(D3DRENDERSTATE_SHADEMODE,D3DSHADE_GOURAUD);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG,FILTER_TYPE); //l
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN,FILTER_TYPE); //l
	SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_SUBPIXEL,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE,TRUE);
	SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL);
	SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND,D3DTBLEND_MODULATE);
	SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR,  0x00000000);
	SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FALSE);
#ifndef TARGET_DC
	SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,FALSE);
#endif
	SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
	SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS,D3DTADDRESS_CLAMP);

	SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE);
	SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
	SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);

	//
	// Draw each standard texture page unfocused one way.
	//

	for (i = 0; i < TEXTURE_page_num_standard; i++)
	{	
		ASSERT(WITHIN(i, 0, POLY_NUM_PAGES - 1));

		pa = &POLY_Page[i];

		//
		// Darken all the vertices and move them to the left.
		//

		for (j = 0; j < pa->vertex_upto; j++)
		{
			tl = &pa->vertex[j];

			//tl->color    &= 0x7f7f7f7f;
			//tl->specular &= 0xff7f7f7f;

			tl->sz -= 1.0F / 65536.0F;

			df = fabsf(tl->sz - focus);

			tl->sx -= df * 80.0F;
			tl->sy -= df * 80.0F;

		}

		//
		// The correct texture page.
		//

		REALLY_SET_TEXTURE(TEXTURE_get_handle(i));

		if (POLY_page_flag[i] & POLY_PAGE_FLAG_TRANSPARENT)
		{
#ifndef TARGET_DC
			TEXTURE_set_colour_key(i);
			SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,TRUE);
#endif
		}

		//
		// Render the polys.
		// 

		the_display.lp_D3D_Device->DrawIndexedPrimitive(
										D3DPT_TRIANGLELIST,
										D3DFVF_TLVERTEX,
										(LPVOID) pa->vertex,
										pa->vertex_upto,
										pa->index,
										pa->index_upto,
										D3DDP_DONOTUPDATEEXTENTS);

		if (POLY_page_flag[i] & POLY_PAGE_FLAG_TRANSPARENT)
		{
#ifndef TARGET_DC
			SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,FALSE);
#endif
		}
	}

	//
	// Draw each standard texture page unfocused the other way.
	//

	for (i = 0; i < TEXTURE_page_num_standard; i++)
	{	
		ASSERT(WITHIN(i, 0, POLY_NUM_PAGES - 1));

		pa = &POLY_page[i];

		//
		// Darken all the vertices and move them to the right
		//

		for (j = 0; j < pa->vertex_upto; j++)
		{
			tl = &pa->vertex[j];

			df = fabsf(tl->sz - focus);

			tl->sx += df * 160.0F;
			tl->sy += df * 160.0F;
		}

		//
		// The correct texture page.
		//

		REALLY_SET_TEXTURE(TEXTURE_get_handle(i));

		if (POLY_page_flag[i] & POLY_PAGE_FLAG_TRANSPARENT)
		{
#ifndef TARGET_DC
			TEXTURE_set_colour_key(i);
			SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,TRUE);
#endif
		}


		//
		// Render the polys.
		// 

		the_display.lp_D3D_Device->DrawIndexedPrimitive(
										D3DPT_TRIANGLELIST,
										D3DFVF_TLVERTEX,
										(LPVOID) pa->vertex,
										pa->vertex_upto,
										pa->index,
										pa->index_upto,
										D3DDP_DONOTUPDATEEXTENTS);

		if (POLY_page_flag[i] & POLY_PAGE_FLAG_TRANSPARENT)
		{
#ifndef TARGET_DC
			SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE,FALSE);
#endif
		}
	}
}

#endif


SLONG POLY_inside_quad(
		float       screen_x,
		float       screen_y,
		POLY_Point *quad[3],
		float      *along_01,
		float      *along_02)
{
	float ax = quad[1]->X - quad[0]->X;
	float ay = quad[1]->Y - quad[0]->Y;

	float bx = quad[2]->X - quad[0]->X;
	float by = quad[2]->Y - quad[0]->Y;

	float dx = screen_x   - quad[0]->X;
	float dy = screen_y   - quad[0]->Y;

	float dproda = ax*dx + ay*dy;
	float dprodb = bx*dx + by*dy;

	float lena2 = ax*ax + ay*ay;
	float lenb2 = bx*bx + by*by;

	float alonga = dproda / lena2;
	float alongb = dprodb / lenb2;

	if (WITHIN(alonga, 0.0f, 1.0f) &&
		WITHIN(alongb, 0.0f, 1.0f))
	{
	   *along_01 = alonga;
	   *along_02 = alongb;

		return TRUE;
	}
	else
	{
	   *along_01 = alonga;
	   *along_02 = alongb;

		return FALSE;
	}
}





#if !defined(TARGET_DC)



// ftol replacements
extern "C"
{

// The Borg's original version

__declspec(naked) long _ftol_borg(float arg)
{
	__asm
	{
		push	ebp
		mov		ebp,esp
		add		esp, -12
		wait
		fnstcw	WORD PTR [ebp-2]
		wait
		mov		ax, WORD PTR [ebp-2]
		or		ah, 0ch
		mov		WORD PTR [ebp-4], ax
		fldcw	WORD PTR [ebp-4]
		fistp	QWORD PTR [ebp-12]
		fldcw	WORD PTR [ebp-2]
		mov		eax, DWORD PTR [ebp-12]
		mov		edx, DWORD PTR [ebp-8]
		leave
		ret
	}
}

// Quick-and-dirty version

__declspec(naked) long _ftol_fasteddie(float arg)
{
	static double	temp = 0;

	__asm
	{
		fistp	QWORD PTR temp;
		mov		eax,DWORD PTR temp
		mov		edx,DWORD PTR temp+4
		ret
	}
}

// Intel's crappy "I don't work" version
// -- fixed so it does! ;-)
//
// I mean, Jesus, it's all very well giving us
// code that does some quite clever manipulations
// of IEEE floats as ints, BUT the code as provided
// had 2 typos and 1 major bug - for fucks sake!

__declspec(naked) long _ftol(float arg)
{
	static double	temp = 0;

	__asm
	{
		// store as a quadword int and reload
		fld		st(0)					// X X
		fistp	QWORD PTR temp			// X
		fild	QWORD PTR temp			// X [X]
		mov		edx,DWORD PTR temp+4
		mov		eax,DWORD PTR temp
		test	eax,eax
		je		maybe_zero

		// number isn't zero, so get X - [X]
not_zero:
		fsubp	st(1),st				// X - [X]
		test	edx,edx
		jns		positive

		// number < 0 - inc eax if X - [X] is >0
		fstp	DWORD PTR temp
		mov		ecx,DWORD PTR temp		// get IEEE rep
		xor		ecx,80000000h			// now <0 if diff >0
		add		ecx,7FFFFFFFh			// carry if it was 00000001 to 7FFFFFFF
		adc		eax,0					// add carry in
		ret

positive:
		// number > 0 - dec eax if X - [X] is <0
		fstp	DWORD PTR temp
		mov		ecx,DWORD PTR temp		// get IEEE rep
		add		ecx,7FFFFFFFh			// carry if it was 80000001 to FFFFFFFF
		sbb		eax,0					// sub carry
		ret

maybe_zero:
		test	edx,7FFFFFFFh
		jnz		not_zero

		// number is zero - clear the stack
		fstp	st(0)
		fstp	st(0)
		ret
	}
}

}

// from the Intel compiler:

void POLY_transform(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bUnused)
{
	static const float	_nearz	= POLY_Z_NEARPLANE;
	static const float	_farz	= 1.0F;
	static const float	_zplane = POLY_ZCLIP_PLANE;

	__asm
	{
		fld		world_x							// wx
		mov		eax, pt
		fsub	POLY_cam_x						// cx
		mov		edx,eax
		fld		world_z							// cx | wz
		fld		world_y							// cx | wz | wy
		fxch	st(2)							// wy | wz | cx
		fstp	DWORD PTR [edx]					// wy | wz
		fld		DWORD PTR [edx]					// wy | wz | cx
		fxch	st(2)							// cx | wz | wy
		fsub	POLY_cam_y						// cx | wz | cy
		fld		_nearz							// cx | wz | cy | Zn
		fxch	st(1)							// cx | wz | Zn | cy
		fstp	DWORD PTR [edx+4]				// cx | wz | Zn
		fld		DWORD PTR [edx+4]				// cx | wz | Zn | cy
		fxch	st(2)							// cx | cy | Zn | wz
		fsub	POLY_cam_z						// cx | cy | Zn | cz
		fld		DWORD PTR [edx+4]				// cx | cy | Zn | cz | cy
		fxch	st(1)							// cx | cy | Zn | cy | cz
		fst		DWORD PTR [edx+8]				// cx | cy | Zn | cy | cz
		fld		DWORD PTR POLY_cam_matrix + 32	// cx | cy | Zn | cy | cz | M[8]
		fmul	st,st(1)						// cx | cy | Zn | cy | cz | cz * M[8]
		fld		DWORD PTR [edx]					// cx | cy | Zn | cy | cz | cz * M[8] | cx
		fmul	DWORD PTR POLY_cam_matrix		// cx | cy | Zn | cy | cz | cz * M[8] | cx * M[0]
		fld		DWORD PTR POLY_cam_matrix + 8	// cx | cy | Zn | cy | cz | cz * M[8] | cx * M[0] | M[2]
		fmul	st,st(3)						// cx | cy | Zn | cy | cz | cz * M[8] | cx * M[0] | cz * M[2]
		fxch	st(6)							// cx | cz * M[2] | Zn | cy | cz | cz * M[8] | cx * M[0] | cy
		fmul	DWORD PTR POLY_cam_matrix + 28	// cx | cz * M[2] | Zn | cy | cz | cz * M[8] | cx * M[0] | cy * M[7]
		fxch	st(7)							// cy * M[7] | cz * M[2] | Zn | cy | cz | cz * M[8] | cx * M[0] | cx
		fmul	DWORD PTR POLY_cam_matrix + 24	// cy * M[7] | cz * M[2] | Zn | cy | cz | cz * M[8] | cx * M[0] | cx * M[6]
		faddp	st(7),st						// cx * M[6] + cy * M[7] | cz * M[2] | Zn | cy | cz | cz * M[8] | cx * M[0]
		fxch	st(3)							// cx * M[6] + cy * M[7] | cz * M[2] | Zn | cx * M[0] | cz | cz * M[8] | cy
		fmul	DWORD PTR POLY_cam_matrix + 4	// cx * M[6] + cy * M[7] | cz * M[2] | Zn | cx * M[0] | cz | cz * M[8] | cy * M[1]
		faddp	st(3),st						// cx * M[6] + cy * M[7] | cz * M[2] | Zn | cx * M[0] + cy * M[1] | cz | cz * M[8]
		faddp	st(5),st						// Z | cz * M[2] | Zn | cx * M[0] + cy * M[1] | cz
		fmul	DWORD PTR POLY_cam_matrix + 20	// Z | cz * M[2] | Zn | cx * M[0] + cy * M[1] | cz * M[5]
		fxch	st(2)							// Z | cz * M[2] | cz * M[5] | cx * M[0] + cy * M[1] | Zn
		fcomp	st(4)							// Z | cz * M[2] | cz * M[5] | cx * M[0] + cy * M[1] (COMPARE WITH Z)
		fnstsw	ax
		faddp	st(2),st						// Z | X | cz * M[5]
		fld		DWORD PTR POLY_cam_matrix + 12	// Z | X | cz * M[5] | M[3]
		fmul	DWORD PTR [edx]					// Z | X | cz * M[5] | cx * M[3]
		fld		DWORD PTR POLY_cam_matrix + 16	// Z | X | cz * M[5] | cx * M[3] | M[4]
		fmul	DWORD PTR [edx+4]				// Z | X | cz * M[5] | cx * M[3] | cz * M[4]
		faddp	st(1),st						// Z | X | cz * M[5] | cx * M[3] + cz * M[4]
		faddp	st(1),st						// Z | X | Y
		fxch	st(1)							// Z | Y | X
		fstp	DWORD PTR [edx]					// Z | Y
		fstp	DWORD PTR [edx+4]				// Z
		fst		DWORD PTR [edx+8]				// Z
		sahf
		jbe		LABEL4

LABEL2:	// too near
		fstp	st(0)
		mov		eax,64
		mov		WORD PTR [edx+24],ax
	
LABEL3: // return

#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL4: // not too near
		fld		_farz							// Z | Zf
		fcomp									// Z
		fnstsw	ax
		sahf
		jae		LABEL6

LABEL5: // too far
		fstp	st(0)
		mov		eax,32
		mov		WORD PTR [edx+24],ax


#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL6: // not too near or too far
		fdivr	DWORD PTR _zplane				// sz
		fld		DWORD PTR [edx]					// sz | X
		fld		DWORD PTR [edx+4]				// sz | X | Y
		fxch	st(2)							// Y | X | sz
		mov		eax,16
		fmul	st(1),st						// Y | X * sz | sz
		fstp	DWORD PTR [edx+20]				// Y | X * sz
		fmul	POLY_screen_mul_x				// Y | X * sz * smx
		fsubr	POLY_screen_mid_x				// Y | sx
		fstp	DWORD PTR [edx+12]				// Y
		fmul	DWORD PTR [edx+20]				// Y * sz
		fld		POLY_screen_mid_y				// Y * sz | smy
		fxch	st(1)							// smidy | Y * sz
		fmul	POLY_screen_mul_y				// smidy | Y * sz * smy
		fsubp	st(1),st						// sy
		mov		WORD PTR [edx+24],ax
		fstp	DWORD PTR [edx+16]
		fld		POLY_screen_clip_left
		fcomp	DWORD PTR [edx+12]
		fnstsw	ax
		fld		DWORD PTR [edx+12]				// sx
		sahf
		jbe		LABEL12

LABEL7:	// left clip
		fstp	st(0)
		movzx	eax,WORD PTR [edx+24]
		or		eax,1
		mov		WORD PTR [edx+24],ax

LABEL8: // try top clip
		fld		DWORD PTR [edx+16]				// sy
		fld		POLY_screen_clip_top			// sy | top
//		fldz
		fcomp									// sy
		fnstsw	ax
		sahf
		jbe		LABEL10

LABEL9: // top clip
		fstp	st(0)
		movzx	eax,WORD PTR [edx+24]
		or		eax,4
		mov		WORD PTR [edx+24],ax


#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL10: // try bottom clip
		fcomp	POLY_screen_clip_bottom
		fnstsw	ax
		sahf
		jbe		LABEL3

LABEL11: // bottom clip
		movzx	eax,WORD PTR [edx+24]
		or		eax,8
		mov		WORD PTR [edx+24],ax
		
#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL12: // try right clip
		fcomp	POLY_screen_clip_right
		fnstsw	ax
		sahf
		jbe		LABEL8

LABEL13: // right clip
		movzx	eax,WORD PTR [edx+24]
		or		eax,2
		mov		WORD PTR [edx+24],ax
		jmp		LABEL8
	}
}

void POLY_transform_using_local_rotation(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt)
{
	static const float	_nearz	= POLY_Z_NEARPLANE;
	static const float	_farz	= 1.0F;
	static const float	_zplane = POLY_ZCLIP_PLANE;

	__asm
	{
//		push	ebx
		mov		edx,pt
		mov		eax,local_y
		mov		ecx,local_z
		fld		local_z								// lz
		fld		local_z								// lz | lz
		fld		local_z								// lz | lz | lz
		mov		ebx,local_x
		mov		DWORD PTR [edx+4],eax	// <- ly
		fld		DWORD PTR [edx+4]					// lz | lz | lz | ly
		mov		DWORD PTR [edx+8],ecx	// <- lz
		mov		DWORD PTR [edx],ebx		// <- lx
		fmul	DWORD PTR POLY_cam_matrix_comb + 4	// lz | lz | lz | ly * M[1]
		fld		DWORD PTR [edx]						// lz | lz | lz | ly * M[1] | lx
		fmul	DWORD PTR POLY_cam_matrix_comb		// lz | lz | lz | ly * M[1] | lx * M[0]
		faddp	st(1),st							// lz | lz | lz | ly * M[1] + lx * M[0]
		fld		DWORD PTR POLY_cam_matrix_comb + 12	// lz | lz | lz | ly * M[1] + lx * M[0] | M[3]
		fmul	DWORD PTR [edx]						// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3]
		fld		DWORD PTR POLY_cam_matrix_comb + 24	// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] | M[6]
		fmul	DWORD PTR [edx]						// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] | lx * M[6]
		fld		DWORD PTR POLY_cam_matrix_comb + 16	// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] | lx * M[6] | M[4]
		fmul	DWORD PTR [edx+4]					// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] | lx * M[6] | ly * M[4]
		faddp	st(2),st							// lz | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] + ly * M[4] | lx * M[6]
		fxch	st(5)								// lx * M[6] | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] + ly * M[4] | lz
		fmul	DWORD PTR POLY_cam_matrix_comb + 8	// lx * M[6] | lz | lz | ly * M[1] + lx * M[0] | lx * M[3] + ly * M[4] | lz * M[2]
		faddp	st(2),st							// lx * M[6] | lz | lz | rx | lx * M[3] + ly * M[4]
		fld		DWORD PTR POLY_cam_matrix_comb + 28	// lx * M[6] | lz | lz | rx | lx * M[3] + ly * M[4] | M[7]
		fmul	DWORD PTR [edx+4]					// lx * M[6] | lz | lz | rx | lx * M[3] + ly * M[4] | ly * M[7]
		faddp	st(5),st							// lx * M[6] + ly * M[7] | lz | lz | rx | lx * M[3] + ly * M[4]
		fld		DWORD PTR _nearz					// lx * M[6] + ly * M[7] | lz | lz | rx | lx * M[3] + ly * M[4] | Zn
		fxch	st(3)								// lx * M[6] + ly * M[7] | lz | Zn | rx | lx * M[3] + ly * M[4] | lz
		fmul	DWORD PTR POLY_cam_matrix_comb + 20	// lx * M[6] + ly * M[7] | lz | Zn | rx | lx * M[3] + ly * M[4] | lz * M[5]
		faddp	st(1),st							// lx * M[6] + ly * M[7] | lz | Zn | rx | ry
		fxch	st(3)								// lx * M[6] + ly * M[7] | ry | Zn | rx | lz
		fmul	DWORD PTR POLY_cam_matrix_comb + 32	// lx * M[6] + ly * M[7] | ry | Zn | rx | lz * M[8]
		faddp	st(4),st							// rz | ry | Zn | rx
		fstp	DWORD PTR [edx]						// rz | ry | Zn
		fld		DWORD PTR [edx]						// rz | ry | Zn | rx
		fxch	st(3)								// rx | ry | Zn | rz
		fstp	DWORD PTR [edx+8]					// rx | ry | Zn
		fld		DWORD PTR [edx+8]					// rx | ry | Zn | rz
		fxch	st(2)								// rx | rz | Zn | ry
		fstp	DWORD PTR [edx+4]					// rx | rz | Zn
		fld		DWORD PTR [edx+4]					// rx | rz | Zn | ry
		fxch	st(3)								// ry | rz | Zn | rx
		fadd	DWORD PTR POLY_cam_off_x			// ry | rz | Zn | trx
		fstp	DWORD PTR [edx]						// ry | rz | Zn
		fxch	st(2)								// Zn | rz | ry
		fadd	DWORD PTR POLY_cam_off_y			// Zn | rz | try
		fstp	DWORD PTR [edx+4]					// Zn | rz
		fadd	DWORD PTR POLY_cam_off_z			// Zn | trz
		fxch	st(1)								// trz | Zn
		fcomp										// trz
		fnstsw	ax
		fst		DWORD PTR [edx+8]					// trz
		sahf
		jbe		LABEL4

LABEL2:
		fstp	st(0)								// <empty>
		mov		eax,64
		mov		WORD PTR [edx+24],ax

LABEL3:
		pop		ebx

#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL4:
		fld		_farz								// trz | Zf
		fcomp										// trz
		fnstsw	ax
		sahf
		jae		LABEL6

LABEL5:
		fstp	st(0)
		mov		eax,32
		mov		WORD PTR [edx+24],ax
		
		pop		ebx

#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL6:
		fdivr	_zplane								// sz
		fld		DWORD PTR [edx]						// sz | trx
		fld		DWORD PTR [edx+4]					// sz | trx | try
		fxch	st(2)								// try | trx | sz
		mov		eax,16
		fmul	st(1),st							// try | trx * sz | sz
		fstp	DWORD PTR [edx+20]					// try | trx * sz
		fmul	POLY_screen_mul_x					// try | trx * sz * smx
		fsubr	POLY_screen_mid_x					// try | sx
		fstp	DWORD PTR [edx+12]					// try
		fmul	DWORD PTR [edx+20]					// try * sz
		fld		POLY_screen_mid_y					// try * sz | smidy
		fxch	st(1)								// smidy | try * sz
		fmul	POLY_screen_mul_y					// smidy | try * sz * smy
		fsubp	st(1),st							// sy
		mov		WORD PTR [edx+24],ax
		fstp	DWORD PTR [edx+16]					// <empty>
		fld		POLY_screen_clip_left
		fcomp	DWORD PTR [edx+12]
		fnstsw	ax
		fld		DWORD PTR [edx+12]					// sx
		sahf
		jbe		LABEL12

LABEL7:
		fstp	st(0)
		movzx	eax,WORD PTR [edx+24]
		or		eax,1
		mov		WORD PTR [edx+24],ax

LABEL8:
		fld		DWORD PTR [edx+16]					// sy
		fld		POLY_screen_clip_top				// sy | top
		fcomp										// sy
		fnstsw	ax
		sahf
		jbe		LABEL10

LABEL9:
		fstp	st(0)
		movzx	eax,WORD PTR [edx+24]
		or		eax,4
		mov		WORD PTR [edx+24],ax
		pop		ebx

#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL10:
		fcomp	POLY_screen_clip_bottom
		fnstsw	ax
		sahf
		jbe		LABEL3

LABEL11:
		movzx	eax,WORD PTR [edx+24]
		or		eax,8
		mov		WORD PTR [edx+24],ax
		pop		ebx

#ifdef _DEBUG
		pop	edi
		pop	esi
		pop	ebx
		mov	esp, ebp
#endif
		pop		ebp
		ret

LABEL12:
		fcomp	POLY_screen_clip_right
		fnstsw	ax
		sahf
		jbe		LABEL8

LABEL13:
		movzx	eax,WORD PTR [edx+24]
		or		eax,2
		mov		WORD PTR [edx+24],ax
		jmp		LABEL8
	}
}


#else //#if !defined(TARGET_DC)


// DC version.


// These are all just wrapped up for now, just to get it working.

long _ftol_borg(float arg)
{
	return ( (int)arg );
}

long _ftol_fasteddie(float arg)
{
	return ( (int)arg );
}

long _ftol(float arg)
{
	return ( (int)arg );
}


#if 0
// Now done in poly.h and inlined.
void POLY_transform(
		float       world_x,
		float       world_y,
		float       world_z,
		POLY_Point *pt,
		bool		bUnused)
{
	POLY_transform_c(world_x,world_y,world_z,pt);
}

void POLY_transform_using_local_rotation(
		float       local_x,
		float       local_y,
		float       local_z,
		POLY_Point *pt)
{
	POLY_transform_using_local_rotation_c(local_x,local_y,local_z,pt);
}
#endif


#endif //#else //#if !defined(TARGET_DC)







