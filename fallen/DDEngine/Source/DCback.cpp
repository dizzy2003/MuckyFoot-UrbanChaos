#if 0

//
// The background...
//

#include <ddlib.h>
#include "back.h"





D3DTexture BACK_ot_roper;
D3DTexture BACK_ot_darci;
D3DTexture BACK_ot_mib;
D3DTexture BACK_ot_line;
D3DTexture BACK_ot_logo;

SLONG BACK_starttime;

void BACK_init()
{
	BACK_ot_roper.LoadTextureTGA("RoperEdgeNail.tga", -1);
	BACK_ot_darci.LoadTextureTGA("GunshotNail.tga",   -1);
	BACK_ot_mib  .LoadTextureTGA("ThreeAmigos4.tga",  -1);
	BACK_ot_line .LoadTextureTGA("Bumpyline.tga",     -1);
	BACK_ot_logo .LoadTextureTGA("uclogo.tga",        -1);

	BACK_starttime = GetTickCount();
}




void BACK_draw()
{
	ULONG colour;

	OS_Buffer *ob;

	float between = 0.0F;

	D3DTexture *ot1;
	D3DTexture *ot2;

	SLONG now = GetTickCount() - BACK_starttime;

	if (now < 2048)
	{
		//
		// Draw nothing for a while...
		//

		colour = 0;
	}
	else
	if (now < 4096)
	{
		//
		// Then fade in over the next two seconds...
		//

		colour  = now - 2048 >> 4;
		colour |= colour << 8;
		colour |= colour << 8;
	}
	else
	{
		now   -= 4000;
		colour = 0x808080;
	}

	switch((now >> 14) % 3)
	{
		case 0:
			ot1 = &BACK_ot_roper;
			ot2 = &BACK_ot_darci;
			break;

		case 1:
			ot1 = &BACK_ot_darci;
			ot2 = &BACK_ot_mib;
			break;

		case 2:
			ot1 = &BACK_ot_mib;
			ot2 = &BACK_ot_roper;
			break;

		default:
			ASSERT(0);
			break;
	}

	now &= 0x3fff;

	if (now < 6000)
	{
		between = 0.0F;
	}
	else
	if (now < 10000)
	{
		between = (now - 6000) * (1.0F / 4000.0F);
	}
	else
	{
		between = 1.0F;
	}

	if (between < 1.0F)
	{
		ob = OS_buffer_new();

		OS_buffer_add_sprite(
			ob,
			0.0F, 0.0F, 1.0F - between, 1.0F,
			0.0F, 1.0F,	1.0F - between, 0.0F,
			0.9999F,
			colour,
			0x00000000,
			OS_FADE_RIGHT);

		OS_buffer_draw(ob, ot1, NULL, OS_DRAW_ZALWAYS);
	}

	if (between > 0.0F)
	{
		ob = OS_buffer_new();

		OS_buffer_add_sprite(
			ob,
			1.0F - between, 0.0F, 1.0F, 1.0F,
			1.0F - between, 1.0F, 1.0F, 0.0F,
			0.9999F,
			colour,
			0x00000000,
			OS_FADE_RIGHT);

		OS_buffer_draw(ob, ot2, NULL, OS_DRAW_ZALWAYS);
	}

	if (between > 0.0F && between < 1.0F)
	{
		ob = OS_buffer_new();

		OS_buffer_add_line_2d(
			ob,
			1.0F - between, 0.0F,
			1.0F - between, 1.0F,
			0.04F * frand(),
			0.0F, 0.0F - frand(),
			1.0F, 2.0F + frand(),
			0.0F,
			ftol((1.0F - between) * 255) | 0x5522ff);

		OS_buffer_draw(ob, BACK_ot_line, NULL, OS_DRAW_ADD | OS_DRAW_NOZWRITE | OS_DRAW_ZALWAYS);
	}

	//
	// Draw the UC logo.
	//

	static SLONG start = 2000;
	static SLONG fade  = 2100;
	static SLONG end   = 9000;
	static SLONG which = 0;
	static float v1    = 1.0F - 0.00F;
	static float v2    = 1.0F - 0.22F;

	if (OS_ticks() < start)
	{
		//
		// Do nothing...
		//
	}
	else
	if (OS_ticks() < fade || OS_ticks() > end)
	{
		SLONG i;
		ULONG colour = 0x404040;

		ob = OS_buffer_new();

		for (i = 0; i < 4; i++)
		{
			OS_buffer_add_sprite(
				ob,
				0.70F + frand() * 0.02F - 0.01F, 0.9F + frand() * 0.02F - 0.01F - (v1 - v2) * 0.17F,
				0.96F + frand() * 0.02F - 0.01F, 0.9F + frand() * 0.02F - 0.01F + (v1 - v2) * 0.17F,
				0.0F, v1,
				1.0F, v2,
				0.0F,
				colour);
		}

		OS_buffer_draw(ob, BACK_ot_logo, NULL, OS_DRAW_ADD | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);

		if (OS_ticks() > end + 100)
		{
			//
			// Start again!
			//

			start = end   + 5000;
			fade  = start +  100;
			end   = fade  + 5000;

			which += 1;

			if (which == 3)
			{
				which = 0;
			}

			switch(which)
			{
				case 0:
					v1 = 1.0F - 0.57F;
					v2 = 1.0F - 1.00F;
					break;

				case 1:
					v1 = 1.0F -  63.0F / 256.0F;
					v2 = 1.0F - 142.0F / 256.0F;
					break;

				case 2:
					v1 = 1.0F - 0.00F;
					v2 = 1.0F - 0.22F;
					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}
	else
	{
		ob = OS_buffer_new();

		OS_buffer_add_sprite(
			ob,
			0.70F, 0.9F - (v1 - v2) * 0.17F,
			0.96F, 0.9F + (v1 - v2) * 0.17F,
			0.0F, v1,
			1.0F, v2,
			0.0F,
			0xffffff);

		OS_buffer_draw(ob, BACK_ot_logo, NULL, OS_DRAW_ADD | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);
	}
}

#endif
