//
// Sky...
//

#include <MFStdLib.h>
#include <DDLib.h>
#include "game.h"
#include "matrix.h"
#include "poly.h"
#include "aeng.h"
#include <math.h>
#include "cam.h"



#define SKY_STAR_T_DIM		1
#define SKY_STAR_T_MEDIUM	2
#define SKY_STAR_T_BRIGHT	3
#define SKY_STAR_T_PLANET	4

#ifndef TARGET_DC
typedef struct
{
	UBYTE colour;
	UBYTE spread;
	UWORD shit;
	float yaw;
	float pitch;
	float vector[3];

} SKY_Star;

#define SKY_MAX_STARS 4096

SKY_Star SKY_star[SKY_MAX_STARS];
SLONG    SKY_star_upto;

//
// Each cloud texture...
//


typedef struct
{
	float u1, v1;
	float u2, v2;

} SKY_Texture;

#define SKY_NUM_TEXTURES 5

SKY_Texture SKY_texture[SKY_NUM_TEXTURES] =
{
	{0.000F, 0.000F, 1.000F, 0.234F},
	{0.000F, 0.234F, 0.566F, 0.375F},
	{0.566F, 0.234F, 1.000F, 0.375F},
	{0.000F, 0.375F, 1.000F, 0.648F},
	{0.000F, 0.648F, 1.000F, 1.000F}
};


typedef struct
{
	UBYTE texture;
	UBYTE flip;		// 1 => Reflect the cloud texture in u.
	UBYTE width;
	UBYTE height;
	float yaw;
	float pitch;
	float dyaw;

} SKY_Cloud;

#define SKY_NUM_CLOUDS 200

SKY_Cloud SKY_cloud[SKY_NUM_CLOUDS];

#endif



#define SKY_wibble_y1 62
#define SKY_wibble_y2 137
#define SKY_wibble_g1 17
#define SKY_wibble_g2 78
#define SKY_wibble_s1 40
#define SKY_wibble_s2 45





void SKY_init(CBYTE *star_file)
{

#ifndef TARGET_DC

	SLONG i;

	float twidth;
	float theight;

	SLONG yaw;
	SLONG pitch;
	SLONG bright;
	SLONG red;
	SLONG green;
	SLONG blue;
	ULONG colour;
	ULONG spread;
	SLONG match;

	FILE *handle;

	CBYTE line[128];

	SKY_Cloud   *sc;
	SKY_Texture *st;
	
	//
	// Create all the clouds.
	//

	for (i = 0; i < SKY_NUM_CLOUDS; i++)
	{
		sc = &SKY_cloud[i];

		sc->texture = rand() % SKY_NUM_TEXTURES;
		sc->flip    = rand() & 0x8;
		sc->yaw     = (float(rand()) / float(RAND_MAX)) * (2.0F * PI);
		sc->pitch   = (float(rand()) / float(RAND_MAX)) * (PI / 3.0F) + (PI / 64.0F);
		sc->dyaw    = (float(rand()) / float(RAND_MAX)) * 0.0005F     + 0.0001F;
		
		ASSERT(WITHIN(sc->texture, 0, SKY_NUM_TEXTURES - 1));

		st = &SKY_texture[sc->texture];

		twidth  = (st->u2 - st->u1) * 256.0F;
		theight = (st->v2 - st->v1) * 256.0F;

		//
		// Randomise the height and width of the cloud, but always make the
		// texels more than one pixel so we get the benefit of filtering.
		//

		twidth  *= 0.3F + (float(rand()) * 0.5F / float(RAND_MAX));
		theight *= 0.3F + (float(rand()) * 0.5F / float(RAND_MAX));

		sc->width  = UBYTE(twidth);
		sc->height = UBYTE(theight);
	}

	//
	// Place down the stars.
	//

	if (star_file == NULL)
	{
		handle = NULL;
	}
	else
	{
		handle = MF_Fopen(star_file, "rb");
	}

	if (handle == NULL)
	{
		//
		// Randomly generate the stars...
		//

		for (i = 0; i < SKY_MAX_STARS; i++)
		{
			yaw    = rand() % 360;
			pitch  = rand() % 60;
			pitch += 10;

			bright  = rand() % (pitch + 0x3f);
			bright += 0x1f;

			SKY_star[i].colour =  bright;
			SKY_star[i].spread = (bright < 100) ? 0 : bright >> 2;
			SKY_star[i].yaw    = float(yaw)   * 2.0F * PI / 360.0F;
			SKY_star[i].pitch  = float(pitch) * 2.0F * PI / 360.0F;
		}

		SKY_star_upto = SKY_MAX_STARS;
	}
	else
	{
		SKY_star_upto = 0;

		while(fgets(line, 128, handle))
		{
			if (SKY_star_upto >= SKY_MAX_STARS)
			{
				//
				// Can't read in any more stars.
				//

				break;
			}

			match = sscanf(line, "Star: %d, %d, %d", &yaw, &pitch, &bright);

			if (match == 3)
			{
				//
				// Make sure that the brightness isn't out of range.
				//

				SATURATE(bright, 0, 255);

				SKY_star[SKY_star_upto].colour =  bright;
				SKY_star[SKY_star_upto].spread = (bright < 100) ? 0 : bright >> 2;
				SKY_star[SKY_star_upto].yaw    = float(yaw)   * 2.0F * PI / 2048.0F;
				SKY_star[SKY_star_upto].pitch  = float(pitch) * 2.0F * PI / 2048.0F;

				SKY_star_upto += 1;
			}
		}

		MF_Fclose(handle);
	}

	//
	// London, England.
	//

	SKY_Star *ss;

	float dpitch = 39.0F * 2.0F * PI / 360.0F;

	for (i = 0; i < SKY_star_upto; i++)
	{
		ss = &SKY_star[i];
		
		MATRIX_vector(
			ss->vector,
			ss->yaw,
			ss->pitch + dpitch);
	}
#endif
}


#ifndef TARGET_DC
void SKY_draw_stars(
		float mid_x,	// The world camera position
		float mid_y,
		float mid_z,
		float max_dist)	// How far away anything is drawn.
{
	SLONG i;

	float yaw;
	float pitch;

	SVector_F  temp;
	SVector_F  pos;
	POLY_Point pp;

	SKY_Star *ss;

	float	xmul = float(RealDisplayWidth) / float(DisplayWidth);
	float	ymul = float(RealDisplayHeight) / float(DisplayHeight);

	for (i = 0; i < SKY_star_upto; i++)
	{
		ss = &SKY_star[i];

		//
		// Draw it.
		// 

		#define SKY_STAR_DIST (max_dist - 256.0F)

		temp.X = ss->vector[0] * SKY_STAR_DIST +  mid_x;
		temp.Y = ss->vector[1] * SKY_STAR_DIST + (mid_y * 0.5F);
		temp.Z = ss->vector[2] * SKY_STAR_DIST +  mid_z;

		POLY_transform(
			temp.X,
			temp.Y,
			temp.Z,
		   &pp);

		if (!(pp.clip & (POLY_CLIP_LEFT | POLY_CLIP_RIGHT | POLY_CLIP_TOP | POLY_CLIP_BOTTOM | POLY_CLIP_NEAR | POLY_CLIP_FAR)))
		{
			SLONG px = SLONG(pp.X * xmul);
			SLONG py = SLONG(pp.Y * ymul);

			if ((rand() & 0x7f) == (i & 0x7f))
			{
				//
				// Make the star twinkle!
				//
			}
			else
			{
				the_display.PlotPixel(
					px, py,
					ss->colour,
					ss->colour,
					ss->colour);

				if (ss->spread)
				{
					ULONG col = the_display.GetFormattedPixel(ss->spread, ss->spread, ss->spread);

					the_display.PlotFormattedPixel(px - 1, py, col);
					the_display.PlotFormattedPixel(px + 1, py, col);
					the_display.PlotFormattedPixel(px, py - 1, col);
					the_display.PlotFormattedPixel(px, py + 1, col);
				}
			}
		}
	}
}
#endif

#ifndef TARGET_DC
void SKY_draw_poly_clouds(
		float mid_x,
		float mid_y,
		float mid_z,
		float max_dist)
{
	
	SLONG i;
	SLONG j;

	float yaw;
	float pitch;
	float vector[3];

	SVector_F temp;
	SVector_F pos;
	ULONG     flag;

	float screen_width  = float(DisplayWidth);
	float screen_height = float(DisplayHeight);

	float width;
	float height;

	SKY_Cloud   *sc;
	SKY_Texture *st;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Draw and animate the cloud quads.
	//

	#define SKY_CLOUD_DIST (max_dist - 512.0F)

	for (i = 0; i < SKY_NUM_CLOUDS; i++)
	{
		sc = &SKY_cloud[i];

		//
		// Animate it.
		//

		sc->yaw += sc->dyaw;

		//
		// Draw it.
		// 

		yaw   = sc->yaw;
		pitch = sc->pitch;

		MATRIX_vector(
			vector,
			yaw,
			pitch);

		temp.X = vector[0] * SKY_CLOUD_DIST + mid_x;
		temp.Y = vector[1] * SKY_CLOUD_DIST + mid_y * 0.5F;
		temp.Z = vector[2] * SKY_CLOUD_DIST + mid_z;

		POLY_transform(
			temp.X,
			temp.Y,
			temp.Z,
		   &mid);

		if (!mid.IsValid())
		{
			//
			// Abandon this cloud.
			//

			continue;
		}

		width  = float(sc->width);
		height = float(sc->height);

		if (mid.X + width  < 0 || mid.X - width > screen_width ||
			mid.Y + height < 0 || mid.Y - width > screen_height)
		{
			//
			// Abandon this cloud.
			//

			continue;
		}

		//
		// The very end of the zbuffer...
		//

		#define SKY_VERY_FAR_AWAY (1.0F / 65536.0F)

		mid.Z        = SKY_VERY_FAR_AWAY;
		mid.colour   = 0xff333333 + 0x00010101 * SLONG(sc->dyaw * (128.0F / 0.0005F));;
		mid.specular = 0x000000;

		for (j = 0; j < 4; j++)
		{
			pp[j] = mid;

			pp[j].X += (j  & 1) ? width  : -width;
			pp[j].Y += (j >> 1) ? height : -height;
		}

		//
		// The sky texture.
		//

		SKY_Texture *st;

		ASSERT(WITHIN(sc->texture, 0, SKY_NUM_TEXTURES - 1));

		st = &SKY_texture[sc->texture];

		if (sc->flip)
		{
			pp[0].u = st->u1;
			pp[0].v = st->v1;
			pp[1].u = st->u2;
			pp[1].v = st->v1;
			pp[2].u = st->u1;
			pp[2].v = st->v2;
			pp[3].u = st->u2;
			pp[3].v = st->v2;
		}
		else
		{
			pp[0].u = st->u2;
			pp[0].v = st->v1;
			pp[1].u = st->u1;
			pp[1].v = st->v1;
			pp[2].u = st->u2;
			pp[2].v = st->v2;
			pp[3].u = st->u1;
			pp[3].v = st->v2;
		}

		POLY_add_quad(quad, POLY_PAGE_CLOUDS, FALSE, TRUE);
	}

	return;
}
#endif





void SKY_draw_poly_moon(
		float mid_x,
		float mid_y,
		float mid_z,
		float max_dist)
{
	static SLONG on_screen_for = 0;
	static SLONG last_cam_dyaw = 0;
	static SLONG last_cam_dpitch = 0;
	static SLONG draw_man = 0;
	
	SLONG i;
	SLONG j;

	float yaw;
	float pitch;
	float vector[3];

	SVector_F temp;
	SVector_F pos;
	ULONG     flag;

	float screen_width  = float(DisplayWidth);
	float screen_height = float(DisplayHeight);

	float width;
	float height;

	//SKY_Cloud   *sc;
	//SKY_Texture *st;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	//
	// Create the moon quad.
	//

	#define SKY_MOON_YAW	(0)
	#define SKY_MOON_PITCH  (PI / 8.0F)
	#define SKY_MOON_DIST	(max_dist - 128.0F)
#ifdef TARGET_DC
	// This is slightly more sensible.
	#define SKY_MOON_RADIUS (screen_width * 0.09F)
#else
	// This is hooooooooooj.
	#define SKY_MOON_RADIUS (screen_width * 0.15F)
#endif
	#define SKY_MOON_UV_IN  (0.02F)

	const struct
	{
		float u;
		float v;

	} moon_uv[4] =
	{
		{       SKY_MOON_UV_IN,        SKY_MOON_UV_IN},
		{1.0F - SKY_MOON_UV_IN,        SKY_MOON_UV_IN},
		{       SKY_MOON_UV_IN, 1.0F - SKY_MOON_UV_IN},
		{1.0F - SKY_MOON_UV_IN, 1.0F - SKY_MOON_UV_IN}
	};

	yaw   = SKY_MOON_YAW;
	pitch = SKY_MOON_PITCH;

	MATRIX_vector(
		vector,
		yaw,
		pitch);

	temp.X = vector[0] * SKY_MOON_DIST + mid_x;
	temp.Y = vector[1] * SKY_MOON_DIST + mid_y * 0.5f;
	temp.Z = vector[2] * SKY_MOON_DIST + mid_z;


#ifdef TARGET_DC
	// Workaround.
	POLY_flush_local_rot();
#endif

	POLY_transform(
		temp.X,
		temp.Y,
		temp.Z,
	   &mid);

	if (!mid.IsValid())
	{
		//
		// Abandon the moon. - you can't see the whole of it <g>
		//

		on_screen_for = 0;
	}
	else
	{
		if (mid.X + SKY_MOON_RADIUS < 0 || mid.X - SKY_MOON_RADIUS > screen_width ||
			mid.Y + SKY_MOON_RADIUS < 0 || mid.Y - SKY_MOON_RADIUS > screen_height)
		{
			//
			// Abandon the moon.
			//

			on_screen_for = 0;
		}
		else
		{
#ifdef TARGET_DC
			// Got to get it just in front of the sky, but behind the far facets.
			mid.Z        = 0.001f;
			mid.z        = 0.999f;
#else
			mid.Z        = SKY_VERY_FAR_AWAY;
#endif
			mid.colour   = 0xffaaaa88;
			mid.specular = 0x00000000;

			for (j = 0; j < 4; j++)
			{
				pp[j] = mid;

				pp[j].X += (j  & 1) ? SKY_MOON_RADIUS : -SKY_MOON_RADIUS;
				pp[j].Y += (j >> 1) ? SKY_MOON_RADIUS : -SKY_MOON_RADIUS;

				pp[j].u = moon_uv[j].u;
				pp[j].v = moon_uv[j].v;
			}

			POLY_add_quad(quad, POLY_PAGE_MOON, FALSE, TRUE);

			on_screen_for += 1;

#if 0
// This no longer works.
			if (draw_man)
			{
				//
				// Fade in and out.
				//

				SLONG man_alpha;
				SLONG man_colour;

				if (draw_man > 120)
				{
					man_alpha = 180 - draw_man;
				}
				else
				if (draw_man < 60)
				{
					man_alpha = draw_man;
				}
				else
				{
					man_alpha = 60;
				}

				man_colour = (man_alpha << 24) | 0x00ffffff;

				//
				// Set the new colours.
				//

				for (j = 0; j < 4; j++)
				{
					pp[j].colour = man_colour;
				}

				//
				// Draw the man on the moon.
				// 

				POLY_add_quad(quad, POLY_PAGE_MANONMOON, FALSE, TRUE);
			}
#endif
		}
	}

#if 0
// No longer works.

	//
	// Is the player looking at the moon?
	//
#ifdef	CAM_OLD
	if (CAM_get_mode() == CAM_MODE_FIRST_PERSON)
	{
		SLONG cam_dyaw;
		SLONG cam_dpitch;	

		CAM_get_dangle(&cam_dyaw, &cam_dpitch);

		if (cam_dyaw   == last_cam_dyaw &&
			cam_dpitch == last_cam_dpitch)
		{
			if (on_screen_for == 200)
			{
				draw_man = 180;
			}
		}
		else
		{
			on_screen_for = 0;
		}

		last_cam_dyaw   = cam_dyaw;
		last_cam_dpitch = cam_dpitch;
	}
	else
#endif
	{
		on_screen_for = 0;
	}

	if (draw_man > 0)
	{
		draw_man -= 1;
	}
#endif

	return;
}



#ifndef TARGET_DC
SLONG SKY_draw_moon_reflection(
		float  mid_x,
		float  mid_y,
		float  mid_z,
		float  max_dist,
		float *moon_x1,
		float *moon_y1,
		float *moon_x2,
		float *moon_y2)
{
	
	SLONG i;
	SLONG j;

	float x;
	float y;
	float v;

	float yaw;
	float pitch;
	float vector[3];

	SVector_F temp;
	SVector_F pos;
	ULONG     flag;

	float screen_width  = float(DisplayWidth);
	float screen_height = float(DisplayHeight);

	float width;
	float height;

	//SKY_Cloud   *sc;
	//SKY_Texture *st;

	POLY_Point  mid;
	POLY_Point  pp[4];
	POLY_Point *quad[4];

	SLONG angle1;
	SLONG angle2;
	SLONG offset1;
	SLONG offset2;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];
	

	//
	// Create the moon quad.
	//

	const struct
	{
		float u;
		float v;

	} moon_uv[4] =
	{
		{       SKY_MOON_UV_IN,        SKY_MOON_UV_IN},
		{1.0F - SKY_MOON_UV_IN,        SKY_MOON_UV_IN},
		{       SKY_MOON_UV_IN, 1.0F - SKY_MOON_UV_IN},
		{1.0F - SKY_MOON_UV_IN, 1.0F - SKY_MOON_UV_IN}
	};

	yaw   = SKY_MOON_YAW;
	pitch = SKY_MOON_PITCH;

	MATRIX_vector(
		vector,
		yaw,
		pitch);

	temp.X =  vector[0] * SKY_MOON_DIST + mid_x;
	temp.Y = -vector[1] * SKY_MOON_DIST + mid_y * 0.5f;
	temp.Z =  vector[2] * SKY_MOON_DIST + mid_z;

	POLY_transform(
		temp.X,
		temp.Y,
		temp.Z,
	   &mid);

	if (!mid.IsValid())
	{
		//
		// Abandon the moon.
		//

		return FALSE;
	}
	else
	{
		if (mid.X + SKY_MOON_RADIUS < 8 || mid.X - SKY_MOON_RADIUS > screen_width  + 8 ||
			mid.Y + SKY_MOON_RADIUS < 8 || mid.Y - SKY_MOON_RADIUS > screen_height + 8)
		{
			//
			// Abandon the moon.
			//

			return FALSE;
		}
		else
		{
#ifdef TARGET_DC
			mid.Z        = 0.9999f;
			mid.z        = 0.0001f;
#else
			mid.Z        = SKY_VERY_FAR_AWAY;
#endif
			mid.colour   = 0xffaaaa88;
			mid.specular = 0x00000000;

			#define SKY_MOON_SEGMENTS 16

			y = mid.Y - SKY_MOON_RADIUS;
			v = moon_uv[0].v;

			//
			// What's the first lines' wibble?
			//

			angle1  = SLONG(y) * SKY_wibble_y1;
			angle2  = SLONG(y) * SKY_wibble_y2;
			angle1 += GAME_TURN * SKY_wibble_g1;
			angle2 += GAME_TURN * SKY_wibble_g2;

			angle1 &= 2047;
			angle2 &= 2047;

			offset2  = SIN(angle1) * SKY_wibble_s1 >> 19;
			offset2 += COS(angle2) * SKY_wibble_s2 >> 19;

			//
			// The amount of 'y' and 'v' in each segment.
			//

			#define SKY_MOON_SEG_DY (float(SKY_MOON_RADIUS) / float(SKY_MOON_SEGMENTS))
			#define SKY_MOON_SEG_DV ((moon_uv[2].v - moon_uv[0].v) / float(SKY_MOON_SEGMENTS))

			for (i = 0; i < SKY_MOON_SEGMENTS; i++)
			{
				offset1 = offset2;

				//
				// What's the next lines' wibble?
				//

				angle1  = SLONG(y + SKY_MOON_SEG_DY) * SKY_wibble_y1;
				angle2  = SLONG(y + SKY_MOON_SEG_DY) * SKY_wibble_y2;
				angle1 += GAME_TURN * SKY_wibble_g1;
				angle2 += GAME_TURN * SKY_wibble_g2;

				angle1 &= 2047;
				angle2 &= 2047;

				offset2  = SIN(angle1) * SKY_wibble_s1 >> 19;
				offset2 += COS(angle2) * SKY_wibble_s2 >> 19;

				for (j = 0; j < 4; j++)
				{
					pp[j] = mid;

					pp[j].X += (j  & 1) ? SKY_MOON_RADIUS : -SKY_MOON_RADIUS;
					pp[j].u  = moon_uv[j].u;

					if (j & 2)
					{
						pp[j].Y  = y + SKY_MOON_SEG_DY;
						pp[j].v  = v + SKY_MOON_SEG_DV;
						pp[j].X += offset2;
					}
					else
					{
						pp[j].Y = y;
						pp[j].v = v;
						pp[j].X += offset1;
					}
				}

				POLY_add_quad(quad, POLY_PAGE_MOON, FALSE, TRUE);

				if (i == 0)
				{
				   *moon_x1 = pp[0].X;
				   *moon_y1 = pp[2].Y;
				}

				if (i == SKY_MOON_SEGMENTS - 1)
				{
				   *moon_x2 = pp[1].X;
				   *moon_y2 = pp[0].Y;
				}

				y += SKY_MOON_SEG_DY;
				v += SKY_MOON_SEG_DV;
			}
		}
	}

	return TRUE;
}
#endif //#ifndef TARGET_DC


#ifndef TARGET_DC
void SKY_draw_poly_sky(
		float world_camera_x,
		float world_camera_y,
		float world_camera_z,
		float world_camera_yaw,
		float max_dist,
		ULONG bot_colour,
		ULONG top_colour)
{
	float px;
	float py;
	float pz;

	float screen_x;
	float screen_y;

	float vector[3];

	MATRIX_vector(
		vector,
		world_camera_yaw,
		0.0F);

	px =  world_camera_x + vector[0] * max_dist;
	py = -256.0F;
	pz =  world_camera_z + vector[2] * max_dist;

	if (!POLY_get_screen_pos(
			px, py, pz,
		   &screen_x,
		   &screen_y))
	{
		//
		// Can't see the horizon.
		//

		return;
	}
	
	POLY_Point  pp  [4];
	POLY_Point *quad[4];

	pp[0].X        = 0.0F;
	pp[0].Y        = screen_y - 256.0F;
	pp[0].Z        = 0.5F;
	pp[0].z        = 0.5F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = bot_colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = 640.0F;
	pp[1].Y        = screen_y - 256.0F;
	pp[1].Z        = 0.5F;
	pp[1].z        = 0.5F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = bot_colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = 0.0F;
	pp[2].Y        = screen_y;
	pp[2].Z        = 0.5F;
	pp[2].z        = 0.5F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = bot_colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = 640.0F;
	pp[3].Y        = screen_y;
	pp[3].Z        = 0.5F;
	pp[3].z        = 0.5F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = bot_colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_SKY, FALSE, TRUE);

	pp[0].X        = 0.0F;
	pp[0].Y        = screen_y - 1024.0F;
	pp[0].Z        = 0.5F;
	pp[0].z        = 0.5F;
	pp[0].u        = 0.0F;
	pp[0].v        = 0.0F;
	pp[0].colour   = top_colour;
	pp[0].specular = 0xff000000;

	pp[1].X        = 640.0F;
	pp[1].Y        = screen_y - 1024.0F;
	pp[1].Z        = 0.5F;
	pp[1].z        = 0.5F;
	pp[1].u        = 0.0F;
	pp[1].v        = 0.0F;
	pp[1].colour   = top_colour;
	pp[1].specular = 0xff000000;

	pp[2].X        = 0.0F;
	pp[2].Y        = screen_y - 256.0F;
	pp[2].Z        = 0.5F;
	pp[2].z        = 0.5F;
	pp[2].u        = 0.0F;
	pp[2].v        = 0.0F;
	pp[2].colour   = bot_colour;
	pp[2].specular = 0xff000000;

	pp[3].X        = 640.0F;
	pp[3].Y        = screen_y - 256.0F;
	pp[3].Z        = 0.5F;
	pp[3].z        = 0.5F;
	pp[3].u        = 0.0F;
	pp[3].v        = 0.0F;
	pp[3].colour   = bot_colour;
	pp[3].specular = 0xff000000;

	quad[0] = &pp[0];
	quad[1] = &pp[1];
	quad[2] = &pp[2];
	quad[3] = &pp[3];

	POLY_add_quad(quad, POLY_PAGE_SKY, FALSE, TRUE);

}
#endif


//  0  1
//
//	2  3

void SKY_draw_poly_sky_old(float world_camera_x,float world_camera_y,float world_camera_z,float world_camera_yaw,float max_dist,ULONG bot_colour,ULONG top_colour)
{
	SLONG i;

	SLONG p1;
	SLONG p2;

	float x;
	float y;
	float z;

	float angle;

	#define SKY_CIRCLE_STEPS 30
	//30

	#define SKY_HORIZON 0.0F
	//(world_camera_y * 0.0 -  256.0F)
	#define SKY_MAXUP  (world_camera_y * 0.0f + 12072.0F)

	POLY_Point pp_bot[SKY_CIRCLE_STEPS];
	POLY_Point pp_top[SKY_CIRCLE_STEPS];

	POLY_Point *quad[4];

	angle = 0.0F;
	max_dist=66.0F*256.0F;

//	max_dist-=2256;

	for (i = 0; i < SKY_CIRCLE_STEPS; i++)
	{
		x = world_camera_x + (float)sin(angle) * (max_dist);// - 378.0F);
		z = world_camera_z + (float)cos(angle) * (max_dist);// - 378.0F);

		POLY_transform_c_saturate_z(x,SKY_HORIZON,z,&pp_bot[i]);

		//
		// Only bother transforming the higher point if the lower point
		// wasn't behind you.
		//

		if (pp_bot[i].IsValid())
		{
			pp_bot[i].colour   = bot_colour;
			pp_bot[i].specular = 0xff000000;

			x = world_camera_x + (float)sin(angle) * (max_dist);// - 1024.0F);
			z = world_camera_z + (float)cos(angle) * (max_dist);// - 1024.0F);

			POLY_transform_c_saturate_z(
				 x,
				 SKY_MAXUP,
				 z,
				&pp_top[i]);

			pp_top[i].colour   = top_colour;
			pp_top[i].specular = 0xff000000;
		}
		else
			pp_top[i].clip=0;


#ifdef TARGET_DC
		// Bodge the RHW so that the moon actually shows up.
		pp_top[i].Z *= 0.05f;
		pp_bot[i].Z *= 0.05f;
#endif
		angle += 2.0F * PI / SKY_CIRCLE_STEPS;
	}

	//
	// Draw the sky quads.
	//

	for (i = 0; i < SKY_CIRCLE_STEPS; i++)
//	for (i = 0; i < 1; i++)
	{
		p1 = i + 0;
		p2 = i + 1;

		if (p2 == SKY_CIRCLE_STEPS) {p2 = 0;}


		quad[0] = &pp_top[p1];
		quad[1] = &pp_top[p2];
		quad[2] = &pp_bot[p1];
		quad[3] = &pp_bot[p2];
		if((quad[0]->clip & quad[1]->clip & quad[1]->clip & quad[2]->clip)&POLY_CLIP_TRANSFORMED)
		if (POLY_valid_quad(quad))
		{
			float	pos;

			switch(i%5)
			{
				case	0:
					pos=0.0F;
					break;
				case	1:
					pos=0.2F;
					break;
				case	2:
					pos=0.4F;
					break;
				case	3:
					pos=0.6F;
					break;
				case	4:
					pos=0.8F;
					break;

			}

			
			
			quad[0]->u = pos;
			quad[1]->u = pos+0.2F;
			quad[2]->u = pos;
			quad[3]->u = pos+0.2F;

			quad[0]->v = 0.0F;
			quad[1]->v = 0.0F;
			quad[2]->v = 1.0F;
			quad[3]->v = 1.0F;

#ifdef TARGET_DC
			// These can trip in mad cases - ignore them.
			//ASSERT ( quad[0]->Z < 0.0009f );
			//ASSERT ( quad[1]->Z < 0.0009f );
			//ASSERT ( quad[2]->Z < 0.0009f );
			//ASSERT ( quad[3]->Z < 0.0009f );
#endif

			POLY_add_quad(quad, POLY_PAGE_SKY, FALSE,1);
		}
	}
}

