//
// Drawing sprites...
//

#ifndef _SPRITE_
#define _SPRITE_



#define SPRITE_SORT_NORMAL	1
#define SPRITE_SORT_FRONT	2

void SPRITE_draw(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort);

void SPRITE_draw_tex(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		float	u,float v,float w,float h,
		SLONG sort);


#ifdef TARGET_DC

// Need to slim down the number of parameters.
struct SPRITE_draw_tex_distorted_params
{
	float	u, v, w, h;
	float   wx1, wy1, wx2, wy2, wx3, wy3, wx4, wy4;
};

void SPRITE_draw_tex_distorted(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort,
		SPRITE_draw_tex_distorted_params *pParams);

#else //#ifdef TARGET_DC

void SPRITE_draw_tex_distorted(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		float	u,float v,float w,float h,
		float   wx1, float wy1, float wx2, float wy2, float wx3, float wy3, float wx4, float wy4,
		SLONG sort);

#endif //#else //#ifdef TARGET_DC

#endif

