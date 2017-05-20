//
// The background...
//

#include "always.h"
#include "back.h"
#include "os.h"





OS_Texture *BACK_ot_roper;
OS_Texture *BACK_ot_darci;
OS_Texture *BACK_ot_mib;
OS_Texture *BACK_ot_line;



void BACK_init()
{
	static SLONG done;

	if (done)
	{
		return;
	}

	BACK_ot_roper = OS_texture_create("RoperEdgeNail.tga");
	BACK_ot_darci = OS_texture_create("GunshotNail.tga");
	BACK_ot_mib   = OS_texture_create("ThreeAmigos4.tga");
	BACK_ot_line  = OS_texture_create("Bumpyline.tga");

	done = TRUE;
}




void BACK_draw()
{
	ULONG colour;

	OS_Buffer *ob;

	float between = 0.0F;

	OS_Texture *ot1;
	OS_Texture *ot2;

	SLONG now = OS_ticks();

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
			ot1 = BACK_ot_roper;
			ot2 = BACK_ot_darci;
			break;

		case 1:
			ot1 = BACK_ot_darci;
			ot2 = BACK_ot_mib;
			break;

		case 2:
			ot1 = BACK_ot_mib;
			ot2 = BACK_ot_roper;
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
			0.0F, 0.0F + frand(),
			1.0F, 8.0F + frand(),
			0.0F,
			ftol((1.0F - between) * 255) | 0x5522ff);

		OS_buffer_draw(ob, BACK_ot_line, NULL, OS_DRAW_ADD | OS_DRAW_NOZWRITE | OS_DRAW_ZALWAYS);
	}
}
