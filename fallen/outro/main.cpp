//
// The mnain outro loop.
//

#include "always.h"
#include "back.h"
#include "cam.h"
#include "credits.h"
#include "key.h"
#include "font.h"
#include "os.h"
#include "wire.h"


void MAIN_main()
{
	SLONG time1 = OS_ticks();
	SLONG time2;

	extern UBYTE the_end;

	//
	// Quickly clear the screen.
	//

	FONT_init();
	OS_clear_screen();
	OS_scene_begin();

	{
		CBYTE *str;

		if (the_end)
		{
			str = "THE END";
		}
		else
		{
			str = "Loading...";
		}

		FONT_draw(FONT_FLAG_JUSTIFY_CENTRE, 0.5F, 0.4F, 0xffffff, 1.5F, -1, 0.0F, str);
	}

	OS_scene_end();
	OS_show();

	//
	// Continue with initialisation...
	//	

	CAM_init();
	BACK_init();
	CREDITS_init();
	WIRE_init();

	//
	// Make sure we display the word, "THE END" for at least 4 seconds...
	//

	if (the_end)
	{
		while (OS_ticks() < time1 + 4000);
	}

	OS_ticks_reset();
	OS_sound_loop_start();

	while(1)
	{
		OS_sound_loop_process();

		if (OS_process_messages() == OS_QUIT_GAME)
		{
			return;
		}

		if (KEY_on[KEY_ESCAPE])
		{
			return;
		}

		CAM_process();
	
		OS_camera_set(
			CAM_x,
			CAM_y,
			CAM_z,
			256.0F,
			CAM_yaw,
			CAM_pitch,
			0.0F,
			CAM_lens,
			0.3F, 0.3F,
			1.0F, 1.0F);

		OS_scene_begin();

		//
		// Clear the screen...
		//

		BACK_draw();

		#if 0

		//
		// Draw some text...
		//

		{
			FONT_draw(
				FONT_FLAG_JUSTIFY_CENTRE,
				0.5F, 0.3F,
				0xffffff,
				2.0F,
			   -1,
				0.0F,
				"Fin");

			FONT_draw(
				FONT_FLAG_JUSTIFY_LEFT,
				FONT_end_x,
				FONT_end_y,
				0xffffff,
				2.0F,
			   -1,
				(KEY_on[KEY_S]) ? 0.0F : ((OS_ticks() & 0xfff) * 1.0F / 4096.0F),
				"?");
		}

		#endif

		WIRE_draw();
		CREDITS_draw();

		if (the_end)
		{
			float shimmer = 0.0F;

			if (OS_ticks() > 4096)
			{
				shimmer = (OS_ticks() - 4096) * (1.0F / 4096.0F);
			}

			if (WITHIN(shimmer, 0.0F, 1.0F))
			{
				FONT_draw(FONT_FLAG_JUSTIFY_CENTRE, 0.503F, 0.403F, 0x000000, 1.5F, -1, shimmer, "THE END");
				FONT_draw(FONT_FLAG_JUSTIFY_CENTRE, 0.500F, 0.400F, 0xffffff, 1.5F, -1, shimmer, "THE END");
			}
		}

		OS_scene_end();
		OS_show();
	}	
}
