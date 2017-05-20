// Engine.h
// Guy Simmons, 18th October 1997

#ifndef	ENGINE_H
#define	ENGINE_H

#include	"Game.h"
#include	"DDLib.h"

#include	"Gamut.h"
#include	"Bucket.h"
#include	"Message.h"
#include	<math.h>

void		DebugText(CBYTE *error, ...);


//
// This is the palette used by the gouraud-shaded
// faces. Their 'Col' field is an index into this
// array.
//


//
// Loads the palette
//

//void ENGINE_load_palette(CBYTE *palette);


//
// Given a ULONG packed d3d colour or specalar value, it
// returns the ULONG multiplied by (r,g,b). r, g and b should
// be from 0 - 255
//

inline ULONG ENGINE_multiply_colour(ULONG colour, SLONG r, SLONG g, SLONG b)
{
	ULONG ans;

	SLONG cr = (colour >>  8) & 0xff;
	SLONG cg = (colour >> 16) & 0xff;
	SLONG cb = (colour >> 24) & 0xff;

	cr *= r;
	cg *= g;
	cb *= b;

	cr >>= 8;
	cg >>= 8;
	cb >>= 8;

	ans = (cr << 0) | (cg << 8) | (cb << 16);
#ifdef TARGET_DC
	ans |= 0xff000000;
#endif

	return ans;
}



//---------------------------------------------------------------
// Engine Maths stuff.

struct 	M31
{
	float	R[3];
};

typedef struct	M31		M31;

#define		UV_X					R[0]
#define		UV_Y					R[1]
#define		UV_Z	 				R[2]

#define	SET_M31(v,x,y,z)			(v)->UV_X=x;(v)->UV_Y=y;(v)->UV_Z=z;
#define	VECTOR_LENGTH_SQUARED(v)	(float)(((v)->UV_X*(v)->UV_X)+((v)->UV_Y*(v)->UV_Y)+((v)->UV_Z*(v)->UV_Z))
#define	VECTOR_LENGTH(v)			(float)sqrt(VECTOR_LENGTH_SQUARED(v))
#define	DOT_PRODUCT(v,w)			((v)->UV_X*(w)->UV_X)+((v)->UV_Y*(w)->UV_Y)+((v)->UV_Z*(w)->UV_Z)
#define	CROSS_PRODUCT(r,v,w)		(r)->UV_X=(((v)->UV_Y*(w)->UV_Z)-((v)->UV_Z*(w)->UV_Y));	\
									(r)->UV_Y=(((v)->UV_Z*(w)->UV_X)-((v)->UV_X*(w)->UV_Z));	\
									(r)->UV_Z=(((v)->UV_X*(w)->UV_Y)-((v)->UV_Y*(w)->UV_X));

inline float	approx_vector_length(M31 *v)
{
	float	max	=	(float)fabs((v->UV_X)),
			med	=	(float)fabs((v->UV_Y)),
			min	=	(float)fabs((v->UV_Z)),
			temp;


	if(max<min)
	{
		temp	=	max;
		max		=	min;
		min		=	temp;
	}
	if(med<min)
	{
		temp	=	med;
		med		=	min;
		min		=	temp;
	}
	if(max<med)
	{
		temp	=	max;
		max		=	med;
		med		=	temp;	
	}

// approx length is max + (11/32)med + (1/4)min (this has +- 8% error)

	return	(float)(max + (med*0.3f) + (min*0.25f));
}

inline void		vector_normalisation(M31 *v,float length)
{
	length	=	(1.0f/length);
	v->UV_X	=	(v->UV_X)*length;
	v->UV_Y	=	(v->UV_Y)*length;
	v->UV_Z	=	(v->UV_Z)*length;
}

//---------------------------------------------------------------

struct 	M33
{
	M31		R0,
			R1,
			R2;
};

typedef struct	M33		M33;

#define		UV_XX					R0.R[0]
#define		UV_YX					R0.R[1]
#define		UV_ZX					R0.R[2]

#define		UV_XY					R1.R[0]
#define		UV_YY					R1.R[1]
#define		UV_ZY					R1.R[2]

#define		UV_XZ					R2.R[0]
#define		UV_YZ					R2.R[1]
#define		UV_ZZ					R2.R[2]


inline void		init_M33(M33 *m)	{	m->UV_XX=(float)1.0;m->UV_XY=(float)0.0;m->UV_XZ=(float)0.0;
										m->UV_YX=(float)0.0;m->UV_YY=(float)1.0;m->UV_YZ=(float)0.0;
										m->UV_ZX=(float)0.0;m->UV_ZY=(float)0.0;m->UV_ZZ=(float)1.0;
									}

inline void		rotate_on_x(SLONG angle,M33 *matrix) 
{
	float	sin	=	SIN_F(angle & 2047),
			cos	=	COS_F(angle & 2047);
	float	t10	=	matrix->UV_XY,
			t11	=	matrix->UV_YY,
			t12	=	matrix->UV_ZY,
			t20	=	matrix->UV_XZ,
			t21	=	matrix->UV_YZ,
			t22	=	matrix->UV_ZZ;

	matrix->UV_XY	=	((cos*t10)	+	(-sin*t20));
	matrix->UV_YY	=	((cos*t11)	+	(-sin*t21));
	matrix->UV_ZY	=	((cos*t12)	+	(-sin*t22));
	matrix->UV_XZ	=	((sin*t10)	+	(cos*t20));
	matrix->UV_YZ	=	((sin*t11)	+	(cos*t21));
	matrix->UV_ZZ	=	((sin*t12)	+	(cos*t22));
}

inline void		rotate_on_y(SLONG angle,M33 *matrix) 
{
	float	sin	=	SIN_F(angle & 2047),
			cos	=	COS_F(angle & 2047);
	float	t00	=	matrix->UV_XX,
			t01	=	matrix->UV_YX,
			t02	=	matrix->UV_ZX,
			t20	=	matrix->UV_XZ,
			t21	=	matrix->UV_YZ,
			t22	=	matrix->UV_ZZ;

	matrix->UV_XX	=	((cos*t00)	+	(sin*t20));
	matrix->UV_YX	=	((cos*t01)	+	(sin*t21));
	matrix->UV_ZX	=	((cos*t02)	+	(sin*t22));
	matrix->UV_XZ	=	((-sin*t00)	+	(cos*t20));
	matrix->UV_YZ	=	((-sin*t01)	+	(cos*t21));
	matrix->UV_ZZ	=	((-sin*t02)	+	(cos*t22));
}

inline void		rotate_on_z(SLONG angle,M33 *matrix) 
{
	float	sin =	SIN_F(angle & 2047),
			cos =	COS_F(angle & 2047);
	float	t00 =	matrix->UV_XX,
			t01 =	matrix->UV_YX,
			t02 =	matrix->UV_ZX,
			t10	=	matrix->UV_XY,
			t11	=	matrix->UV_YY,
			t12	=	matrix->UV_ZY;

	matrix->UV_XX	=	((cos*t00)	+	(-sin*t10));
	matrix->UV_YX	=	((cos*t01)	+	(-sin*t11));
	matrix->UV_ZX	=	((cos*t02)	+	(-sin*t12));
	matrix->UV_XY	=	((sin*t00)	+	(cos*t10));
	matrix->UV_YY	=	((sin*t01)	+	(cos*t11));
	matrix->UV_ZY	=	((sin*t02)	+	(cos*t12));
}

inline void		angle_vector(SLONG angle, M31 *matrix)
{
	float	sin	=	SIN_F(angle),
			cos	=	COS_F(angle);
	float	t_x	=	matrix->UV_X,
			t_z	=	matrix->UV_Z;


	matrix->UV_X	=	((cos*t_x)	+	(sin*t_z));
	matrix->UV_Z	=	((-sin*t_x)	+	(cos*t_z));
}

inline void		roll_vector(SLONG roll, M31 *matrix)
{
	float	sin		=	SIN_F(roll),
			cos		=	COS_F(roll);
	float	t_x	=	matrix->UV_X,
			t_y	=	matrix->UV_Y;


	matrix->UV_X	=	((cos*t_x)	+	(sin*t_y));
	matrix->UV_Y	=	((-sin*t_x)	+	(cos*t_y));
}

inline void		tilt_vector(SLONG tilt, M31 *matrix)
{
	float	sin		=	SIN_F(tilt),
			cos		=	COS_F(tilt);
	float	t_y	=	matrix->UV_Y,
			t_z	=	matrix->UV_Z;


	matrix->UV_Y	=	((cos*t_y)	+	(sin*t_z));
	matrix->UV_Z	=	((-sin*t_y)	+	(cos*t_z));
}

//---------------------------------------------------------------
// Engine defines.

#define	EF_OFF_LEFT		(1<<0)
#define	EF_OFF_RIGHT	(1<<1)
#define	EF_OFF_TOP		(1<<2)
#define	EF_OFF_BOTTOM	(1<<3)
#define	EF_FAR_OUT		(1<<4)
#define	EF_BEHIND_YOU	(1<<5)
#define	EF_TRANSLATED	(1<<6)
#define	EF_TOO_BIG		(1<<7)

#define	EF_CLIPFLAGS 	(EF_OFF_LEFT+EF_OFF_RIGHT+EF_OFF_TOP+EF_OFF_BOTTOM)
#define	EF_CLIPZFLAGS 	(EF_FAR_OUT+EF_BEHIND_YOU+EF_TOO_BIG)

//---------------------------------------------------------------

typedef	struct
{
	float	X,
			Y,
			Z;

	float RotX;	// The view-space coordinates.
	float RotY;
	float RotZ;

}SVECTOR_F;


typedef	struct
{
	float	X,
			Y,
			Z;
}Coord;

//---------------------------------------------------------------

typedef	struct
{
	float		CameraX,
				CameraY,
				CameraZ;
	SLONG		CameraAngle,
				CameraRoll,
				CameraTilt;
}Camera;
					  

//---------------------------------------------------------------

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
	Coord		CameraPos;

}Engine;

extern Engine				the_engine;

//---------------------------------------------------------------

typedef	struct
{
	float		Distance,
				ScrX,
				ScrY,
				ScrZ;
				
}DDEnginePoint;


struct EnginePointF
{
	float		ScrX,
				ScrY,
				ScrZ,
				X,
				Y,
				Z;
};

//---------------------------------------------------------------

#define	MAX_VERTICES		(32000)
#define	ELE_SHIFT			(8)
#define	ELE_SHIFT_F			(1.0f/(float)(ELE_SIZE))
#define	ELE_SIZE			(1<<ELE_SHIFT)
#define	ELE_SIZE_F			((float)(1<<ELE_SHIFT))
#define	TEXTURE_MUL			(0.00390625f)
#define	SHADE_MUL			((float)(1.0f/128.0f))

#define	ALT_SHIFT			(3)

#define	MAX_Z				10000.0F
#define	MIN_Z				32.0F
#define SCALE_SZ			(MIN_Z / MAX_Z)


//---------------------------------------------------------------

extern UWORD				current_vertex;
extern D3DTLVERTEX			vertex_pool[MAX_VERTICES];

//---------------------------------------------------------------

ULONG	transform_point(SVECTOR_F *v,SVECTOR_F *r);
void	set_camera(Camera *the_camera);
void	render_buckets(void);
void	do_map_who(THING_INDEX thing, SLONG map_x, SLONG map_z);

//---------------------------------------------------------------

//
// Debugging aids.
//

void	e_draw_3d_line           (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void	e_draw_3d_line_dir       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2);
void	e_draw_3d_line_col       (SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void	e_draw_3d_line_col_sorted(SLONG x1,SLONG y1,SLONG z1,SLONG x2,SLONG y2,SLONG z2,SLONG r,SLONG g,SLONG b);
void	e_draw_3d_mapwho		 (SLONG x1,SLONG z1);
void	e_draw_3d_mapwho_y		 (SLONG x1,SLONG y1,SLONG z1);


#endif
