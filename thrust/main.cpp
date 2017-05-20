//
// Where the game starts...
//

#include "always.h"
#include "font.h"
#include "game.h"
#include "gamestate.h"
#include "key.h"
#include "land.h"
#include "log.h"
#include "net.h"
#include "orb.h"
#include "os.h"
#include "ship.h"
#include "tb.h"



volatile float global_a;
volatile float global_b;
volatile float global_c;
volatile float global_d;
volatile float global_e;




void MAIN_main()
{
	#if 0

	LOG_init();

/*

Ship   523 = (0.000000,-10.111063) 0.000000 = -10.1110630,-1054750998,-1054750998
1: (0.000000,-10.111063) (0.000000,-0.028490)  (0,-1054750998) (0,-1125555314)
2: (0.000000,-10.111063) (0.000000,-0.028560)  (0,-1054750998) (0,-1125517734)
3: (0.000000,-10.111063) (0.000000,-0.028531)  (0,-1054750998) (0,-1125533067)
4: (0.000000,-10.139594) (0.000000,-0.028531)  (0,-1054721081) (0,-1125533067)
    0: Collision normal = (-0.759257,0.650791) (-1086169437,1059494467)
    1: Collision normal = (-0.759257,0.650791) (-1086169437,1059494467)
    2: Collision normal = (-0.759257,0.650791) (-1086169436,1059494468)
    Collision normal = (-0.759257,0.650791)
    SHIP COLLISION! Ship pos = (0.000000,-10.139594) vel = (0.000000,-0.028531)
5: -0.018568 -1130882112
6: (-0.759257,0.650791) (0.028196,-0.024168)  (-1086169436,1059494468) (1021770374,-1127875761)
7: (0.000000,-10.139594) (-0.028196,-0.004364)  (0,-1054721081) (-1125713274,-1148257129)
8: 0.018568 1016601536
9: (-0.028196,-10.143957) (-0.028196,-0.004364)  (-1125713274,-1054716506) (-1125713274,-1148257129)

*/

	union
	{
		float f;
		int   i;

	} dprod;

	dprod.i = -1130882112;

	union
	{
		float f;
		int   i;

	} dy;

	dy.i = -1125533067;

	union
	{
		float f;
		int   i;

	} ny;

	ny.i = 1059494468;

	/*

	LOG_file("(%f,%f,%f) (%d,%d,%d)\n", dprod.f, dy.f, ny.f, dprod.i, dy.i, ny.i);

	dy.f -= ny.f * dprod.f * 2.0F;

	LOG_file("(%f,%f,%f) (%d,%d,%d)\n", dprod.f, dy.f, ny.f, dprod.i, dy.i, ny.i);

	*/



	

	SHIP_ship[0].dy = dy.f;

	float mydprod = dprod.f;
	float myny    = ny.f;

	{
		union
		{
			float f;
			int   i;

		} v[1];

		v[0].f = SHIP_ship[0].dy;

		LOG_file("%f %d\n", v[0].f, v[0].i);
	}

	SHIP_ship[0].dy -= myny * mydprod * 2.0F;

	{
		union
		{
			float f;
			int   i;

		} v[1];

		v[0].f = SHIP_ship[0].dy;

		LOG_file("%f %d\n", v[0].f, v[0].i);
	}





	// 3: (-3.089566,-11.066875) (-0.025106,-0.011548) (-1610612736,-1073170578) (1610612736,-1071242691)
	// 4: (-3.114672,-11.078422) (-0.025106,-0.011548) (1610612736,-1073157415) (-536870912,-1071241178)


	// 3: (-3.089566,-11.066875) (-0.025106,-0.011548) (-1610612736,-1073170578) (1610612736,-1071242691)
	// 4: (-3.114672,-11.078423) (-0.025106,-0.011548) (1610612736,-1073157415) (0,-1071241177)


	// 3: (-3.089566,-11.066875) (-0.025106,-0.011548) (-1610612736,-1073170578) (1610612736,-1071242691)
	// 4: (-3.114672,-11.078422) (-0.025106,-0.011548) (1610612736,-1073157415) (-536870912,-1071241178)

	/*

	union
	{
		float f;
		int   i;

	} x;

	union
	{
		float f;
		int   i;

	} y;

	union
	{
		float f;
		int   i;

	} dx;

	union
	{
		float f;
		int   i;

	} dy;

	-1130882112





	x.i = -1610612736;
	y.i = -1073170578;

	dx.i = 1610612736;
	dy.i = -1071242691;

	LOG_file("(%f,%f) (%d,%d)\n", x.f, y.f, dx.f, dx.f, x.i,y.i, dx.i,dy.i);

	x.f += dx.f;
	y.f += dy.f;

	LOG_file("(%f,%f) (%d,%d)\n", x.f, y.f, dx.f, dx.f, x.i,y.i, dx.i,dy.i);

	*/

	/*

	void SHIP_process_one(SHIP_Ship *ss);
	void GAME_init_level(SLONG level);

	srand(0);

	SLONG i;

	global_a =  -0.112613F;
	global_b = -10.157440F;

	for (i = 0; i < 1000; i++)
	{
		LOG_file("%f + %f = %f\n", global_a, global_b, global_a + global_b);

		global_b -= 0.00007F;

		global_a *= 0.999F;
		global_b *= 0.999F;
	}

	*/

	/*

	GAME_init_level(0);
	SHIP_ship[0].flag = SHIP_FLAG_USED | SHIP_FLAG_ACTIVE;
	SHIP_ship[0].y = -4.0F;

	for (i = 0; i < 1000; i++)
	{
		SHIP_process_one(&SHIP_ship[0]);

		LOG_file("%f,%f\n", SHIP_ship[0].x, SHIP_ship[0].y);
	}

	*/

	LOG_fini();

	return;

	#endif

	SLONG size = sizeof(GAMESTATE_State);

	NET_init();
	FONT_init();
	LOG_init();
	GAME_do();
	LOG_fini();
	NET_kill();
}

#if 0

void MAIN_main_old()
{
	GAMESTATE_State *gs;

	float mid_x = 0.00F;
	float mid_y = 0.00F;
	float zoom  = 0.018F;

	SHIP_Ship *ss[2];
	ORB_Orb   *oo;
	TB_Tb     *tt[2] = {NULL, NULL};

	SHIP_init();
	ORB_init();
	TB_init();
	LAND_init();
	FONT_init();

	ss[0] = SHIP_create( 0.0F, -4.0F);
	ss[1] = SHIP_create(-5.0F, -4.0F);

	/*
	oo    = ORB_create (-14,-13, 0.6F);
	oo    = ORB_create (-19, -3, 0.8F);
	oo    = ORB_create ( 20,-13, 1.0F);
	*/

	{
		SLONG i;

		for (i = 0; i < 12; i++)
		{
			ORB_create(-19, -2, 0.50F + i * (0.025F));
		}
	}

	ORB_create(-19, -3, 1.05F);

	
	//
	// The landscape.
	//
	
	/*

	LAND_add_line( 15.0F, -12.0F, -10.0F, -10.0F);
	LAND_add_line( 12.0F,   9.0F,  15.0F, -12.0F);
	LAND_add_line(-13.0F,   9.0F,  12.0F,   9.0F);
	LAND_add_line(-10.0F, -10.0F, -13.0F,   9.0F);

	*/

	/*

	LAND_add_line( 10.0F, -10.0F, -10.0F, -10.0F);
	LAND_add_line( 10.0F,  10.0F,  10.0F, -10.0F);
	LAND_add_line(-10.0F,  10.0F,  10.0F,  10.0F);
	LAND_add_line(-10.0F, -10.0F, -10.0F,  10.0F);

	*/

	Point2d level1[25] =
	{
		{12,1},
		{14,8},
		{11,5},
		{8,6},
		{6,18},
		{27,18},
		{29,-19},
		{4,-16},
		{4,-7},
		{-2,-14},
		{-15,-17},
		{-18,-12},
		{-15,-9},
		{-10,-6},
		{-13,-3},
		{-12,4},
		{-17,3},
		{-16,-3},
		{-18,-5},
		{-24,-4},
		{-29,16},
		{-13,17},
		{-10,9},
		{-5,10},
		{12,1}
	};

	LAND_add_line(25, level1);

	LAND_calc_normals();

	SLONG last_process = OS_ticks();

	while(1)
	{
		if (OS_process_messages() == OS_QUIT_GAME)
		{
			return;
		}

		if (KEY_on[KEY_ESCAPE])
		{
			KEY_on[KEY_ESCAPE] = 0;

			return;
		}

		/*
		{
			if (KEY_on[KEY_1]) {KEY_on[KEY_1] = 0; gs = GAMESTATE_store();}
			if (KEY_on[KEY_2]) {KEY_on[KEY_2] = 0; if (gs) {GAMESTATE_restore(gs); gs = NULL;}}
		}
		*/

		{
			while(last_process < OS_ticks())
			{
				GAME_turn += 1;

				if ((GAME_turn % 16) == 0)
				{
					{
						ss[0]->flag &= ~(SHIP_FLAG_LEFT | SHIP_FLAG_RIGHT | SHIP_FLAG_THRUST);

						if (KEY_on[KEY_Z]) {ss[0]->flag |= SHIP_FLAG_LEFT; }
						if (KEY_on[KEY_X]) {ss[0]->flag |= SHIP_FLAG_RIGHT;}

						if (KEY_on[KEY_V]) {ss[0]->flag |= SHIP_FLAG_THRUST;}

						if (KEY_on[KEY_G])
						{
							KEY_on[KEY_G] = 0;

							//
							// Create/destroy the tractor beam.
							//

							if (tt[0] == NULL)
							{
								tt[0] = TB_create(ss[0], 5.0F);
							}
							else
							{
								TB_destroy(tt[0]);

								tt[0] = NULL;
							}
						}
					}

					{
						ss[1]->flag &= ~(SHIP_FLAG_LEFT | SHIP_FLAG_RIGHT | SHIP_FLAG_THRUST);

						if (KEY_on[KEY_P1]) {ss[1]->flag |= SHIP_FLAG_LEFT; }
						if (KEY_on[KEY_P2]) {ss[1]->flag |= SHIP_FLAG_RIGHT;}

						if (KEY_on[KEY_P6]) {ss[1]->flag |= SHIP_FLAG_THRUST;}

						if (KEY_on[KEY_PADD])
						{
							KEY_on[KEY_PADD] = 0;

							//
							// Create/destroy the tractor beam.
							//

							if (tt[1] == NULL)
							{
								tt[1] = TB_create(ss[1], 5.0F);
							}
							else
							{
								TB_destroy(tt[1]);

								tt[1] = NULL;
							}
						}
					}
				}

				SHIP_process_all(GAME_turn);
				ORB_process_all();
				TB_process_all();

				if (tt[0])
				{
					//
					// Has the ship or the orb collided?
					//

					if (tt[0]->ss->flag & SHIP_FLAG_COLLIDED)
					{
						TB_destroy(tt[0]);

						tt[0] = NULL;
					}
					else
					if (tt[0]->oo->flag & ORB_FLAG_COLLIDED)
					{
						float speed = qdist2(fabs(tt[0]->oo->dx), fabs(tt[0]->oo->dy));

						if (speed > 0.01F)
						{
							TB_destroy(tt[0]);

							tt[0] = NULL;
						}
					}
				}

				if (tt[1])
				{
					//
					// Has the ship or the orb collided?
					//

					if (tt[1]->ss->flag & SHIP_FLAG_COLLIDED)
					{
						TB_destroy(tt[1]);

						tt[1] = NULL;
					}
					else
					if (tt[1]->oo->flag & ORB_FLAG_COLLIDED)
					{
						float speed = qdist2(fabs(tt[1]->oo->dx), fabs(tt[1]->oo->dy));

						if (speed > 0.01F)
						{
							TB_destroy(tt[1]);

							tt[1] = NULL;
						}
					}
				}

				last_process += GAME_TICKS_PER_PROCESS;

				float want_mid_x;
				float want_mid_y;

				want_mid_x  = ss[0]->x + ss[1]->x;
				want_mid_y  = ss[0]->y + ss[1]->y;
				want_mid_x *= 0.5F;
				want_mid_y *= 0.5F;

				mid_x += (want_mid_x - mid_x) * 0.01F;
				mid_y += (want_mid_y - mid_y) * 0.01F;
			}
		}

		if (KEY_on[KEY_I]) {zoom *= 1.005F;}
		if (KEY_on[KEY_O]) {zoom *= 0.995F;}

		OS_scene_begin();
		OS_clear_screen();
		SHIP_draw_all(mid_x, mid_y, zoom);
		ORB_draw_all (mid_x, mid_y, zoom);
		TB_draw_all  (mid_x, mid_y, zoom);
		LAND_draw_all(mid_x, mid_y, zoom);
		OS_fps();
		OS_scene_end();
		OS_show();
	}
}

#endif
