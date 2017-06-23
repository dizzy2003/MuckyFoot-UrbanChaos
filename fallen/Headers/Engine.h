// Engine.h
// Guy Simmons, 22nd October 1997.

#ifndef	ENGINE_H
#define	ENGINE_H

//---------------------------------------------------------------

#ifdef	VERSION_D3D

typedef	struct
{
	float	CameraX,
			CameraY,
			CameraZ;
	SLONG	CameraAngle,
			CameraRoll,
			CameraTilt,
			CameraRAngle;
}Camera;

#elif defined(VERSION_GLIDE)

typedef	struct
{
}Camera;

#elif defined(VERSION_PSX)

typedef	struct
{
}Camera;

#endif

//---------------------------------------------------------------

struct 	M31
{
	float	R[3];
};

struct 	M33
{
	M31		R0,
			R1,
			R2;
};

#define		UV_XX					R0.R[0]
#define		UV_YX					R0.R[1]
#define		UV_ZX					R0.R[2]

#define		UV_XY					R1.R[0]
#define		UV_YY					R1.R[1]
#define		UV_ZY					R1.R[2]

#define		UV_XZ					R2.R[0]
#define		UV_YZ					R2.R[1]
#define		UV_ZZ					R2.R[2]

typedef	struct
{
	float		HalfViewHeight,
				HalfViewWidth,
				OriginX,
				OriginY,
				OriginZ,
				ViewHeight,
				ViewWidth,
				Lens;
	SLONG		BucketSize;
	M33			CameraMatrix;
}Engine;

BOOL	init_3d_engine(void);
void	fini_3d_engine(void);
void	game_engine(Camera *the_view);
void	engine_attract(void);
void	engine_win_level(void);
void	engine_lose_level(void);

//---------------------------------------------------------------

void	temp_setup_map(void);
BOOL	new_init_3d_engine(void);
void	new_engine(Camera *the_view);

//---------------------------------------------------------------

//
// After you have loaded all the prims, call this function. It
// fixes the texture coordinates of the prims if the engine has
// fiddled with the texture pages.
//

void engine_fiddle_prim_textures(void);

//
// Given a texture square coordinate on a page, it returns the
// page and texture square coordinates of the fiddled position.
//
// 'square_u' and 'square_v' should be between 0 and 7, and the
// fiddled position are returned.
//

SLONG TEXTURE_get_fiddled_position(
		SLONG  square_u,
		SLONG  square_v,
		SLONG  page,
		float *u,
		float *v);

//
// Debugging aids.
//

void	e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void	e_draw_3d_line_dir       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void	e_draw_3d_line_col       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void	e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void	e_draw_3d_mapwho         (SLONG x1,SLONG z1);
void	e_draw_3d_mapwho_y       (SLONG x1,SLONG y1,SLONG z1);

//
// Messages drawn straight to the screen.
//

void MSG_add(CBYTE *message, ...);

//
// Engine stuff to help you draw straight to the screen...
//

void  ENGINE_clear_screen(void);
void  ENGINE_flip(void);
SLONG ENGINE_lock(void);		// Locks the screen.. returns TRUE on success.
void  ENGINE_unlock(void);

#include "fallen/ddengine/headers/font.h"



#endif
