//
// The camera.
//

#include "always.h"
#include "cam.h"
#include "key.h"
#include "matrix.h"
#include "os.h"



//
// The two different camera types.
// 

#define CAM_TYPE_LOCKED 0
#define CAM_TYPE_FREE   1
#define CAM_TYPE_NUMBER 2


//
// The camera.
//

SLONG CAM_type;
float CAM_x;
float CAM_y;
float CAM_z;
float CAM_yaw;
float CAM_pitch;
float CAM_lens;
float CAM_dist;
float CAM_matrix[9];
float CAM_focus_x;
float CAM_focus_y;
float CAM_focus_z;



void CAM_init(void)
{
	CAM_type  = CAM_TYPE_LOCKED;
	CAM_yaw   =   0.0F;
	CAM_pitch =  -1.0F;
	CAM_dist  = 150.0F;
	CAM_lens  =   2.5F;

	CAM_process();
}


void CAM_process(void)
{

	/*

	extern float SIREN_x;
	extern float SIREN_y;
	extern float SIREN_z;

	CAM_focus_x = SIREN_x;
	CAM_focus_y = SIREN_y;
	CAM_focus_z = SIREN_z;

	*/

	//
	// Pressing TAB toggles between the different camera types.
	//

	if (KEY_on[KEY_TAB])
	{
		KEY_on[KEY_TAB] = 0;

		CAM_type += 1;

		if (CAM_type >= CAM_TYPE_NUMBER)
		{	
			CAM_type = 0;
		}
	}

	#define CAM_SPEED_MOVE (2.000F)
	#define CAM_SPEED_TURN (0.060F)

	switch(CAM_type)
	{
		case CAM_TYPE_LOCKED:

			if (KEY_on[KEY_HOME]) {CAM_dist -= CAM_SPEED_MOVE;}
			if (KEY_on[KEY_END ]) {CAM_dist += CAM_SPEED_MOVE;}

			if (KEY_on[KEY_UP  ]) {CAM_pitch += CAM_SPEED_TURN;}
			if (KEY_on[KEY_DOWN]) {CAM_pitch -= CAM_SPEED_TURN;}

			if (KEY_on[KEY_LEFT ]) {CAM_yaw -= CAM_SPEED_TURN;}
			if (KEY_on[KEY_RIGHT]) {CAM_yaw += CAM_SPEED_TURN;}

			MATRIX_calc(
				CAM_matrix,
				CAM_yaw/* + float(OS_ticks()) * 0.001F*/,
				CAM_pitch,
				0.0F);

			CAM_x = CAM_focus_x - CAM_matrix[6] * CAM_dist;
			CAM_y = CAM_focus_y - CAM_matrix[7] * CAM_dist;
			CAM_z = CAM_focus_z - CAM_matrix[8] * CAM_dist;

			break;

		case CAM_TYPE_FREE:

			MATRIX_calc(
				CAM_matrix,
				CAM_yaw/* + float(OS_ticks()) * 0.001F*/,
				CAM_pitch,
				0.0F);

			if (KEY_on[KEY_UP])
			{
				if (KEY_shift)
				{
					CAM_x += CAM_matrix[3] * CAM_SPEED_MOVE;
					CAM_y += CAM_matrix[4] * CAM_SPEED_MOVE;
					CAM_z += CAM_matrix[5] * CAM_SPEED_MOVE;
				}
				else
				{
					CAM_x += CAM_matrix[6] * CAM_SPEED_MOVE;
					CAM_y += CAM_matrix[7] * CAM_SPEED_MOVE;
					CAM_z += CAM_matrix[8] * CAM_SPEED_MOVE;
				}
			}

			if (KEY_on[KEY_DOWN])
			{
				if (KEY_shift)
				{
					CAM_x -= CAM_matrix[3] * CAM_SPEED_MOVE;
					CAM_y -= CAM_matrix[4] * CAM_SPEED_MOVE;
					CAM_z -= CAM_matrix[5] * CAM_SPEED_MOVE;
				}
				else
				{
					CAM_x -= CAM_matrix[6] * CAM_SPEED_MOVE;
					CAM_y -= CAM_matrix[7] * CAM_SPEED_MOVE;
					CAM_z -= CAM_matrix[8] * CAM_SPEED_MOVE;
				}
			}

			if (KEY_on[KEY_LEFT])
			{
				CAM_x -= CAM_matrix[0] * CAM_SPEED_MOVE;
				CAM_y -= CAM_matrix[1] * CAM_SPEED_MOVE;
				CAM_z -= CAM_matrix[2] * CAM_SPEED_MOVE;
			}

			if (KEY_on[KEY_RIGHT])
			{
				CAM_x += CAM_matrix[0] * CAM_SPEED_MOVE;
				CAM_y += CAM_matrix[1] * CAM_SPEED_MOVE;
				CAM_z += CAM_matrix[2] * CAM_SPEED_MOVE;
			}

			if (KEY_on[KEY_HOME]) {CAM_pitch -= CAM_SPEED_TURN;}
			if (KEY_on[KEY_END ]) {CAM_pitch += CAM_SPEED_TURN;}

			if (KEY_on[KEY_DELETE  ]) {CAM_yaw -= CAM_SPEED_TURN;}
			if (KEY_on[KEY_PAGEDOWN]) {CAM_yaw += CAM_SPEED_TURN;}

			MATRIX_calc(
				CAM_matrix,
				CAM_yaw/* + float(OS_ticks()) * 0.001F*/,
				CAM_pitch,
				0.0F);

			break;

		default:
			ASSERT(0);
			break;
	}
}

