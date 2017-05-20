#include <MFStdLib.h>
#include "poly.h"
#include "sprite.h"




#ifdef TARGET_DC
// The DC is having real problems with this many arguments.
void SPRITE_draw_tex_distorted(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort,
		SPRITE_draw_tex_distorted_params *pParams)
{
#define GET_FROM_PARAMS(argname) float argname = pParams->argname
		GET_FROM_PARAMS(u);
		GET_FROM_PARAMS(v);
		GET_FROM_PARAMS(w);
		GET_FROM_PARAMS(h);
		GET_FROM_PARAMS(wx1);
		GET_FROM_PARAMS(wy1);
		GET_FROM_PARAMS(wx2);
		GET_FROM_PARAMS(wy2);
		GET_FROM_PARAMS(wx3);
		GET_FROM_PARAMS(wy3);
		GET_FROM_PARAMS(wx4);
		GET_FROM_PARAMS(wy4);
#undef GET_FROM_PARAMS

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
		SLONG sort)
{
#endif //#else //#ifdef TARGET_DC

	float screen_size;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid);

	if (mid.IsValid())
	{
		screen_size = POLY_world_length_to_screen(world_size) * mid.Z;

		if (mid.X + screen_size < 0 ||
			mid.X - screen_size > POLY_screen_width ||
			mid.Y + screen_size < 0 ||
			mid.Y - screen_size > POLY_screen_height)
		{
			//
			// Off screen.
			// 
		}
		else
		{
			pp[0].X = mid.X - screen_size +wx1;
			pp[0].Y = mid.Y - screen_size +wy1;
			pp[1].X = mid.X + screen_size +wx2;
			pp[1].Y = mid.Y - screen_size +wy2;
			pp[2].X = mid.X - screen_size +wx3;
			pp[2].Y = mid.Y + screen_size +wy3;
			pp[3].X = mid.X + screen_size +wx4;
			pp[3].Y = mid.Y + screen_size +wy4;

			pp[0].u = u;
			pp[0].v = v;
			pp[1].u = u+w;
			pp[1].v = v;
			pp[2].u = u;
			pp[2].v = v+h;
			pp[3].u = u+w;
			pp[3].v = v+h;

			pp[0].colour = colour;
			pp[1].colour = colour;
			pp[2].colour = colour;
			pp[3].colour = colour;

			pp[0].specular = specular;
			pp[1].specular = specular;
			pp[2].specular = specular;
			pp[3].specular = specular;

			switch(sort)
			{
				case SPRITE_SORT_NORMAL:
					pp[0].z = mid.z;
					pp[0].Z = mid.Z;
					pp[1].z = mid.z;
					pp[1].Z = mid.Z;
					pp[2].z = mid.z;
					pp[2].Z = mid.Z;
					pp[3].z = mid.z;
					pp[3].Z = mid.Z;
					break;

				case SPRITE_SORT_FRONT:
					pp[0].z = 0.01F;
					pp[0].Z = 1.00F;
					pp[1].z = 0.01F;
					pp[1].Z = 1.00F;
					pp[2].z = 0.01F;
					pp[2].Z = 1.00F;
					pp[3].z = 0.01F;
					pp[3].Z = 1.00F;
					break;
				
				default:
					ASSERT(0);
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, page, FALSE, TRUE);
		}
	}
}





void SPRITE_draw(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		SLONG sort)
{
	float screen_size;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid);

	if (mid.IsValid())
	{
		screen_size = POLY_world_length_to_screen(world_size) * mid.Z;

		if (mid.X + screen_size < 0 ||
			mid.X - screen_size > POLY_screen_width ||
			mid.Y + screen_size < 0 ||
			mid.Y - screen_size > POLY_screen_height)
		{
			//
			// Off screen.
			// 
		}
		else
		{
			pp[0].X = mid.X - screen_size;
			pp[0].Y = mid.Y - screen_size;
			pp[1].X = mid.X + screen_size;
			pp[1].Y = mid.Y - screen_size;
			pp[2].X = mid.X - screen_size;
			pp[2].Y = mid.Y + screen_size;
			pp[3].X = mid.X + screen_size;
			pp[3].Y = mid.Y + screen_size;

			pp[0].u = 0.0F;
			pp[0].v = 0.0F;
			pp[1].u = 1.0F;
			pp[1].v = 0.0F;
			pp[2].u = 0.0F;
			pp[2].v = 1.0F;
			pp[3].u = 1.0F;
			pp[3].v = 1.0F;

			pp[0].colour = colour;
			pp[1].colour = colour;
			pp[2].colour = colour;
			pp[3].colour = colour;

			pp[0].specular = specular;
			pp[1].specular = specular;
			pp[2].specular = specular;
			pp[3].specular = specular;

			switch(sort)
			{
				case SPRITE_SORT_NORMAL:
					pp[0].z = mid.z;
					pp[0].Z = mid.Z;
					pp[1].z = mid.z;
					pp[1].Z = mid.Z;
					pp[2].z = mid.z;
					pp[2].Z = mid.Z;
					pp[3].z = mid.z;
					pp[3].Z = mid.Z;
					break;

				case SPRITE_SORT_FRONT:
					pp[0].z = 0.01F;
					pp[0].Z = 1.00F;
					pp[1].z = 0.01F;
					pp[1].Z = 1.00F;
					pp[2].z = 0.01F;
					pp[2].Z = 1.00F;
					pp[3].z = 0.01F;
					pp[3].Z = 1.00F;
					break;
				
				default:
					ASSERT(0);
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, page, FALSE, TRUE);
		}
	}
}

void SPRITE_draw_tex(
		float world_x,
		float world_y,
		float world_z,
		float world_size,
		ULONG colour,
		ULONG specular,
		SLONG page,
		float	u,float v,float w,float h,
		SLONG sort)
{
	float screen_size;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	POLY_transform(
		world_x,
		world_y,
		world_z,
	   &mid,
	   TRUE);

	if (mid.IsValid())
	{
		screen_size = POLY_world_length_to_screen(world_size) * mid.Z;

		if (mid.X + screen_size < 0 ||
			mid.X - screen_size > POLY_screen_width ||
			mid.Y + screen_size < 0 ||
			mid.Y - screen_size > POLY_screen_height)
		{
			//
			// Off screen.
			// 
		}
		else
		{
			pp[0].X = mid.X - screen_size;
			pp[0].Y = mid.Y - screen_size;
			pp[1].X = mid.X + screen_size;
			pp[1].Y = mid.Y - screen_size;
			pp[2].X = mid.X - screen_size;
			pp[2].Y = mid.Y + screen_size;
			pp[3].X = mid.X + screen_size;
			pp[3].Y = mid.Y + screen_size;

			pp[0].u = u;
			pp[0].v = v;
			pp[1].u = u+w;
			pp[1].v = v;
			pp[2].u = u;
			pp[2].v = v+h;
			pp[3].u = u+w;
			pp[3].v = v+h;

			pp[0].colour = colour;
			pp[1].colour = colour;
			pp[2].colour = colour;
			pp[3].colour = colour;

			pp[0].specular = specular;
			pp[1].specular = specular;
			pp[2].specular = specular;
			pp[3].specular = specular;

			switch(sort)
			{
				case SPRITE_SORT_NORMAL:
					pp[0].z = mid.z;
					pp[0].Z = mid.Z;
					pp[1].z = mid.z;
					pp[1].Z = mid.Z;
					pp[2].z = mid.z;
					pp[2].Z = mid.Z;
					pp[3].z = mid.z;
					pp[3].Z = mid.Z;
					break;

				case SPRITE_SORT_FRONT:
					pp[0].z = 0.01F;
					pp[0].Z = 1.00F;
					pp[1].z = 0.01F;
					pp[1].Z = 1.00F;
					pp[2].z = 0.01F;
					pp[2].Z = 1.00F;
					pp[3].z = 0.01F;
					pp[3].Z = 1.00F;
					break;
				
				default:
					ASSERT(0);
			}

			quad[0] = &pp[0];
			quad[1] = &pp[1];
			quad[2] = &pp[2];
			quad[3] = &pp[3];

			POLY_add_quad(quad, page, FALSE, TRUE);
		}
	}
}



