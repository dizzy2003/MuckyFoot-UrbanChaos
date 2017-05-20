//
// Precalculates shadows on a built city.
//

#include "game.h"
#include "shadow.h"
#include "pap.h"
#include "supermap.h"
#include "mav.h"
#include	"memory.h"


SLONG shadow_dist = 32;

#define SHADOW_DIR_X (+147)
#define SHADOW_DIR_Y (-148)
#define SHADOW_DIR_Z (-147)

#define SHADOW_DIST shadow_dist;


SLONG SHADOW_in(SLONG x1, SLONG y1, SLONG z1)
{
	x1 -= SHADOW_DIR_X >> 5;
	y1 -= SHADOW_DIR_Y >> 5;
	z1 -= SHADOW_DIR_Z >> 5;

	SLONG x2 = x1 - SHADOW_DIR_X * SHADOW_DIST;
	SLONG y2 = y1 - SHADOW_DIR_Y * SHADOW_DIST;
	SLONG z2 = z1 - SHADOW_DIR_Z * SHADOW_DIST;

	return !there_is_a_los(
				x1, y1, z1,
				x2, y2, z2,
				0);
}


void SHADOW_do()
{
	SLONG i;

	SLONG mx;
	SLONG mz;

	SLONG map_x;
	SLONG map_z;

	SLONG height;

	SLONG s1;
	SLONG s2;
	SLONG s3;

	SLONG which;

	RoofFace4 *rf;

	SLONG shadow[8] = 
	{
		0, 1, 7, 5, 3, 2, 4, 5
	};

	//
	// Shadow the floor using the FLOOR_HIDDEN flags.
	//

	for (mx = 1; mx < PAP_SIZE_HI - 1; mx++)
	for (mz = 1; mz < PAP_SIZE_HI - 1; mz++)
	{
		PAP_2HI(mx,mz).Flags &= ~(PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3);

		if (PAP_2HI(mx,mz).Flags & PAP_FLAG_HIDDEN)
		{
			//
			// This ground square is inside a building- so we can ignore it.
			//

			continue;
		}

		s1 = PAP_2HI(mx-1,mz+0).Flags & PAP_FLAG_HIDDEN;
		s2 = PAP_2HI(mx-0,mz+1).Flags & PAP_FLAG_HIDDEN;
		s3 = PAP_2HI(mx-1,mz+1).Flags & PAP_FLAG_HIDDEN;

		which = 0;

		if (s1) {which |= 1;}
		if (s2) {which |= 2;}
		if (s3) {which |= 4;}

		PAP_2HI(mx,mz).Flags |= shadow[which];
	}

	//
	// Shadow the cunning 1 bit roof tiles  (store the data in the floor)
	//
		
	for (mx = 1; mx < PAP_SIZE_HI - 1; mx++)
	for (mz = 1; mz < PAP_SIZE_HI - 1; mz++)
	{

		if ((PAP_2HI(mx,mz).Flags & (PAP_FLAG_HIDDEN|PAP_FLAG_ROOF_EXISTS)) ==(PAP_FLAG_HIDDEN|PAP_FLAG_ROOF_EXISTS))
		{
			PAP_2HI(mx,mz).Flags &= ~(PAP_FLAG_SHADOW_1|PAP_FLAG_SHADOW_2|PAP_FLAG_SHADOW_3);

			height = MAVHEIGHT(mx,mz);
			s1 = (MAVHEIGHT(mx-1,mz+0) > height);
			s2 = (MAVHEIGHT(mx-0,mz+1) > height);
			s3 = (MAVHEIGHT(mx-1,mz+1) > height);


			which = 0;

			if (s1) {which |= 1;}
			if (s2) {which |= 2;}
			if (s3) {which |= 4;}

			PAP_2HI(mx,mz).Flags |= shadow[which];
		}
	}

	//
	// Shadow the roof faces.
	//

	for (i = 1; i < next_roof_face4; i++)
	{
		rf = &roof_faces4[i];

		map_x = rf->RX&127;
		map_z = rf->RZ&127;

		if (WITHIN(map_x, 1, PAP_SIZE_HI - 2) &&
			WITHIN(map_z, 1, PAP_SIZE_HI - 2))
		{
			height = MAVHEIGHT(map_x,map_z);

			s1 = (MAVHEIGHT(map_x-1,map_z+0) > height);
			s2 = (MAVHEIGHT(map_x-0,map_z+1) > height);
			s3 = (MAVHEIGHT(map_x-1,map_z+1) > height);

			which = 0;

			if (s1) {which |= 1;}
			if (s2) {which |= 2;}
			if (s3) {which |= 4;}

			rf->DrawFlags &= ~0x7;
			rf->DrawFlags |=  shadow[which];
			PAP_2HI(map_x,map_z).Flags |= shadow[which];
/*
			if(rf->DY[0]==0&&rf->DY[1]==0&&rf->DY[2]==0)
			{
//				ASSERT(PAP_2HI(map_x,map_z).Flags & PAP_FLAG_HIDDEN);

				PAP_2HI(map_x,map_z).Flags |= PAP_FLAG_FLAT_ROOF;

			}
*/
		}
	}


	//
	// Shadow the walkable faces.
	//

	{
		SLONG i;

		PrimFace4 *f4;

		PrimPoint *p0;
		PrimPoint *p1;
		PrimPoint *p2;
		PrimPoint *p3;

		SLONG x;
		SLONG y;
		SLONG z;

		SLONG shadow_1;
		SLONG shadow_2;

		for (i = 0; i < number_of_walkable_prim_faces4; i++)
		{
			ASSERT(WITHIN(first_walkable_prim_face4 + i, 1, next_prim_face4 - 1));

			f4 = &prim_faces4[first_walkable_prim_face4 + i];

			ASSERT(WITHIN(f4->Points[0], first_walkable_prim_point, next_prim_point - 1));
			ASSERT(WITHIN(f4->Points[1], first_walkable_prim_point, next_prim_point - 1));
			ASSERT(WITHIN(f4->Points[2], first_walkable_prim_point, next_prim_point - 1));
			ASSERT(WITHIN(f4->Points[3], first_walkable_prim_point, next_prim_point - 1));

			p0 = &prim_points[f4->Points[0]];
			p1 = &prim_points[f4->Points[1]];
			p2 = &prim_points[f4->Points[2]];
			p3 = &prim_points[f4->Points[3]];

			x = (p0->X + p1->X + p2->X + p3->X) >> 2;
			y = (p0->Y + p1->Y + p2->Y + p3->Y) >> 2;
			z = (p0->Z + p1->Z + p2->Z + p3->Z) >> 2;

			f4->FaceFlags &= ~(FACE_FLAG_SHADOW_1 | FACE_FLAG_SHADOW_2 | FACE_FLAG_SHADOW_3);

			if ((x & 0xff) == 0x80 &&
				(z & 0xff) == 0x80)
			{
				//
				// A quick check to make sure this is a normal building walkable face.
				// 

				map_x = x >> 8;
				map_z = z >> 8;

				if (WITHIN(map_x, 1, PAP_SIZE_HI - 2) &&
					WITHIN(map_z, 1, PAP_SIZE_HI - 2))
				{
					height = MAVHEIGHT(map_x,map_z);

					s1 = (MAVHEIGHT(map_x-1,map_z+0) > height);
					s2 = (MAVHEIGHT(map_x-0,map_z+1) > height);
					s3 = (MAVHEIGHT(map_x-1,map_z+1) > height);

					which = 0;

					if (s1) {which |= 1;}
					if (s2) {which |= 2;}
					if (s3) {which |= 4;}

					ASSERT(FACE_FLAG_SHADOW_1 == 1 << 2);	// The hardcoding in the next line.

					f4->FaceFlags |= shadow[which] << 2;
				}
			}
		}
	}
}







