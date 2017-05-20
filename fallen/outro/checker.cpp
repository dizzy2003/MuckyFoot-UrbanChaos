//
// The checkered way Darci and Roper appear on screen.
//

#include "always.h"
#include "checker.h"
#include "os.h"


//
// The textures.
// 

OS_Texture *CHECKER_ot_darci;
OS_Texture *CHECKER_ot_roper;



//
// Our single checker.
//

typedef struct
{
	float x;
	float u;

	//
	// The bitfields for the start and end...
	//

	ULONG start;
	ULONG end;

	OS_Texture *ot;

	float fuzz;

} CHECKER_Checker;

#define CHECKER_NUM_CHECKERS 1

CHECKER_Checker CHECKER_checker[CHECKER_NUM_CHECKERS];




void CHECKER_init()
{
	SLONG i;
	SLONG j;

	CHECKER_Checker *cc;

	CHECKER_ot_darci = OS_texture_create("strip1b.tga");
	CHECKER_ot_roper = OS_texture_create("strip2b.tga");

	//
	// Start off with the checkers off screen to the left so that
	// it is reinitialised by the first process...
	//

	for (i = 0; i < CHECKER_NUM_CHECKERS; i++)
	{
		cc = &CHECKER_checker[i];
		
		cc->x = -1000.0F;
	}
}




void CHECKER_draw_one(CHECKER_Checker *cc, SLONG colour, SLONG os_draw, float stretch)
{
}





void CHECKER_draw()
{
	SLONG i;
	SLONG which;

	CHECKER_Checker *cc;

	static SLONG last;
	static SLONG now  = OS_ticks();
	static SLONG turn;

	//
	// Never be more than 1/2 a second behind...
	//

	if (last < now - 500)
	{
		last = now - 500;
	}

	while(last < now)
	{
		turn += 1;

		//
		// Process 20 times a second.
		//

		last += 1000 / 20;

 		for (i = 0; i < CHECKER_NUM_CHECKERS; i++)
		{
			cc = &CHECKER_checker[i];
		
			if (cc->x < -1.0F)
			{
				//
				// Reinitialise the checker.
				//

				if (cc->ot == CHECKER_ot_darci)
				{
					cc->ot = CHECKER_ot_roper;
				}
				else
				{
					cc->ot = CHECKER_ot_darci;
				}

				cc->start = 0;
				cc->end   = 0xffffffff;
				cc->fuzz  = 0.0F;
				cc->x     = 1.0F;
				cc->u     = 0.0F;
			}

			//
			// Process the checker...
			//

			cc->x -= 0.001F;
			cc->u -= 0.001F;

			if (cc->fuzz > 0.0F)
			{
				cc->fuzz -= 0.02F;

				if (cc->fuzz < 0.0F)
				{
					cc->fuzz = 0.0F;
				}
			}

			if ((turn & 0x7) == 0)
			{
				which   = rand() & 0xf;
				which  *= which;
				which >>= 5;
				which <<= 2;
				which  |= rand() & 0x3;

				if (cc->x > 0.5F)
				{
					cc->start |= 1 << which;
				}
				else
				{
					cc->end &= ~(1 << which);
				}
	
				if (cc->fuzz == 0.0F)
				{
					if (rand() & 0x3)
					{
						//
						// Start it fuzzing!
						//

						cc->fuzz = frand();
					}
				}
			}
		}
	}

	//
	// Draw all the checkers.
	//

 	for (i = 0; i < CHECKER_NUM_CHECKERS; i++)
	{
		cc = &CHECKER_checker[i];
		
		if (cc->fuzz == 0.0F)
		{
			CHECKER_draw_one(cc, 0xffffff, OS_DRAW_NORMAL, 1.0F);
		}
		else
		{
			//
			// Oh bugger it!
			//
		}
	}
}









