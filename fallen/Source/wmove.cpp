//
// Moving walkable faces.
//

#include "game.h"
#include "wmove.h"
#include "build2.h"
#include "memory.h"
#include "fmatrix.h"
#include "statedef.h"

//#include "prim.h"

extern BOOL allow_debug_keys;
extern	SLONG	save_psx;

WMOVE_Face *WMOVE_face; //[WMOVE_MAX_FACES];
SLONG       WMOVE_face_upto;

#ifndef PSX
void WMOVE_init()
{
	memset(WMOVE_face, 0, sizeof(WMOVE_Face) * WMOVE_MAX_FACES);

	WMOVE_face_upto = 1;
}
#endif

//
// Returns how many wmove faces a given thing needs.
//

SLONG WMOVE_get_num_faces(Thing *p_thing)
{
	switch(p_thing->Class)
	{
		case CLASS_PERSON:
		case CLASS_PLAT:
			return 1;
			break;

		case CLASS_VEHICLE:
			switch (p_thing->Genus.Vehicle->Type)
			{
			case VEH_TYPE_VAN:
			case VEH_TYPE_AMBULANCE:
			case VEH_TYPE_WILDCATVAN:
				return 1;

			case VEH_TYPE_JEEP:
			case VEH_TYPE_MEATWAGON:
				return 4;

			case VEH_TYPE_CAR:
			case VEH_TYPE_TAXI:
			case VEH_TYPE_POLICE:
			case VEH_TYPE_SEDAN:
				return 5;
			}

			ASSERT(0);
			break;

		default:
			ASSERT(0);
			break;
	}

	return 0;
}


//
// This function gives back the position of the walkable face for the given thing.
// It only works for certain kinds of things.
//

SLONG WMOVE_matrix[9];
UWORD WMOVE_matrix_thing;
UWORD WMOVE_matrix_turn;

void WMOVE_get_pos(
		Thing      *p_thing,
		WMOVE_Point pos[3],
		SLONG       number)
{
	SLONG i;
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dz;
	SLONG xo;
	SLONG zo;
	SLONG xa;
	SLONG za;
	SLONG xb;
	SLONG zb;
	SLONG txo;
	SLONG tzo;
	SLONG txa;
	SLONG tza;
	SLONG txb;
	SLONG tzb;
	SLONG prim;
	SLONG useangle;
	SLONG sin_yaw;
	SLONG cos_yaw;
	SLONG matrix[4];

	PrimInfo *pi;

	UWORD get_vehicle_body_prim(SLONG type);

	switch(p_thing->Class)
	{
		case CLASS_PERSON:
			
			//
			// A square on top of this person's head.
			//

			dx = SIN(p_thing->Draw.Tweened->Angle) >> 10;
			dz = COS(p_thing->Draw.Tweened->Angle) >> 10;

			xo = (p_thing->WorldPos.X >> 8) + dx + (-dz);
			zo = (p_thing->WorldPos.Z >> 8) + dz + (+dx);

			xa = xo + (-dx << 1);
			za = zo + (-dz << 1);

			xb = xo + (+dz << 1);
			zb = zo + (-dx << 1);

			y  = (p_thing->WorldPos.Y >> 8) + 0xa0;

			//
			// Store this triangle in the array.
			//

			pos[0].x = xo;
			pos[0].y = y;
			pos[0].z = zo;

			pos[1].x = xa;
			pos[1].y = y;
			pos[1].z = za;

			pos[2].x = xb;
			pos[2].y = y;
			pos[2].z = zb;

			break;

		case CLASS_VEHICLE:

			{
				typedef struct
				{
					UBYTE p[3];

				} WMOVE_Tri;
				
				const static WMOVE_Tri tri_van[1] =
				{
					{{1,2,21}}
				};

				const static WMOVE_Tri tri_car[5] =
				{
					{{33,11,34}},
					{{34,12,35}},
					{{35,13,37}},
					{{37,15,38}},
					{{38,16,39}}
				};

				const static WMOVE_Tri tri_police[5] =
				{
					{{28, 29, 50}},
					{{27, 28, 49}},
					{{25, 27, 47}},
					{{24, 25, 46}},
					{{23, 24, 45}}
				};


				const static WMOVE_Tri tri_ambulance[1] =
				{
					{{119, 120, 121}}
				};


				const static WMOVE_Tri tri_jeep[4] =
				{
					{{14,33, 2}},
					{{ 2, 4,11}},
					{{11,30, 9}},
					{{ 9,28,19}}
				};
				
				/*

				I think the meatwagon is the same mesh as the jeep nowadays...

				const static WMOVE_Tri tri_meatwagon[4] = 
				{
					{{25,44, 4}},
					{{ 4, 6,22}},
					{{22,41,20}},
					{{20,39,30}}
				};

				*/

				const static WMOVE_Tri *tri_vehicle[VEH_TYPE_NUMBER] =
				{
					tri_van,
					tri_car,
					tri_car,
					tri_police,
					tri_ambulance,
					tri_jeep,
					tri_jeep,	// tri_meatwagon, Meatwagon model is the same as the jeep nowadays.
					tri_car,
					tri_van
				};

				const WMOVE_Tri *use = &((tri_vehicle[p_thing->Genus.Vehicle->Type])[number]);

				//
				// Find this things matrix.
				//

				if (WMOVE_matrix_thing != THING_NUMBER(p_thing) ||
					WMOVE_matrix_turn  != GAME_TURN)
				{
					WMOVE_matrix_thing = THING_NUMBER(p_thing);
					WMOVE_matrix_turn  = GAME_TURN;

					FMATRIX_calc(
						WMOVE_matrix,
						p_thing->Genus.Vehicle->Angle,
						p_thing->Genus.Vehicle->Tilt,
						p_thing->Genus.Vehicle->Roll);
				}

				//
				// The offset of the vehicle's body prim from the vehicle position and
				// the prim to use.
				//

				SLONG offy = get_vehicle_body_offset(p_thing->Genus.Vehicle->Type);
				SLONG prim = get_vehicle_body_prim  (p_thing->Genus.Vehicle->Type);

				PrimObject *po = &prim_objects[prim];

				for (i = 0; i < 3; i++)
				{
					x = prim_points[po->StartPoint + use->p[i]].X;
					y = prim_points[po->StartPoint + use->p[i]].Y;
					z = prim_points[po->StartPoint + use->p[i]].Z;

					FMATRIX_MUL_BY_TRANSPOSE(
						WMOVE_matrix,
						x,
						y,
						z);

					x += p_thing->WorldPos.X >> 8;
					y += p_thing->WorldPos.Y >> 8;
					z += p_thing->WorldPos.Z >> 8;

					y -= offy >> 8;

					pos[i].x = x;
					pos[i].y = y;
					pos[i].z = z;
				}
			}

			break;

		case CLASS_PLAT:

			if (p_thing->Class == CLASS_VEHICLE)
			{
				prim      = get_vehicle_body_prim(p_thing->Genus.Vehicle->Type);
				useangle  = -p_thing->Genus.Vehicle->Angle;
				useangle &=  2047;
			}
			else
			{
				prim      =  p_thing->Draw.Mesh->ObjectId;
				useangle  = -p_thing->Draw.Mesh->Angle;
				useangle &=  2047;
			}

			pi   = get_prim_info(prim);

			sin_yaw = SIN(useangle);
			cos_yaw = COS(useangle);

			matrix[0] =  cos_yaw;
			matrix[1] = -sin_yaw;
			matrix[2] =  sin_yaw;
			matrix[3] =  cos_yaw;

			//
			// Rotate the positions.
			//

			txo  = pi->minx;
			tzo  = pi->minz;

			txa  = pi->maxx;
			tza  = pi->minz;

			txb  = pi->minx;
			tzb  = pi->maxz;

			xo = MUL64(txo, matrix[0]) + MUL64(tzo, matrix[1]);
			zo = MUL64(txo, matrix[2]) + MUL64(tzo, matrix[3]);

			xa = MUL64(txa, matrix[0]) + MUL64(tza, matrix[1]);
			za = MUL64(txa, matrix[2]) + MUL64(tza, matrix[3]);

			xb = MUL64(txb, matrix[0]) + MUL64(tzb, matrix[1]);
			zb = MUL64(txb, matrix[2]) + MUL64(tzb, matrix[3]);

			y = (p_thing->WorldPos.Y >> 8) + pi->maxy + 0x10;
			
			if (p_thing->Class == CLASS_VEHICLE)
			{
				//
				// Oh well. It's a strange world!
				//

				y -= get_vehicle_body_offset(p_thing->Genus.Vehicle->Type) >> 8;
			}

			//
			// Relative to the object.
			//

			xo += (p_thing->WorldPos.X >> 8);
			zo += (p_thing->WorldPos.Z >> 8);

			xa += (p_thing->WorldPos.X >> 8);
			za += (p_thing->WorldPos.Z >> 8);

			xb += (p_thing->WorldPos.X >> 8);
			zb += (p_thing->WorldPos.Z >> 8);

			//
			// Store this triangle in the array.
			//

			pos[0].x = xo;
			pos[0].y = y;
			pos[0].z = zo;

			pos[1].x = xa;
			pos[1].y = y;
			pos[1].z = za;

			pos[2].x = xb;
			pos[2].y = y;
			pos[2].z = zb;

			break;

		default:
			ASSERT(0);
			break;
	}
}


void WMOVE_create(Thing *p_thing)
{
	SLONG i;

	SLONG dax;
	SLONG day;
	SLONG daz;

	SLONG dbx = 0;	// check your warnings, guys!
	SLONG dby = 0;
	SLONG dbz = 0;

	SLONG face4;
	SLONG wmove;
	SLONG number;

	PrimPoint  *pp;
	PrimFace4  *f4;
	WMOVE_Face *wf;


#ifdef TARGET_DC
	// This doesn't work yet - MarkZA needs to fix it so it doesn't scribble on Darci's mesh.
	// So for now I disable it, but this ASSERT is to remind me that it doesn't work. Then I
	// won't forget.
	// MarkZA - when you fix this, give me a yell and I'll test it and all that.
	// Actually, I think you may be able to reproduce it on the PC as well, since it now uses the
	// same character system. Oh, but only if you're loading from DADs. So you'll need to do that.
	//ASSERT ( FALSE );
	//return;
#endif


	
//	#ifdef PSX
#ifndef PSX
	if(save_psx)
#endif
	if (p_thing->Class == CLASS_VEHICLE)
	{
		//
		// The PSX is too slow for our code to let you climb on cars.
		//
		// Hacked to allow the creation of walkable faces on Vans so that they
		// can be climbed upon to allow easier access to places (just like on
		// the PC)
		//
		if ((p_thing->Genus.Vehicle->Type!=VEH_TYPE_VAN)&&(p_thing->Genus.Vehicle->Type!=VEH_TYPE_WILDCATVAN)&&(p_thing->Genus.Vehicle->Type!=VEH_TYPE_AMBULANCE))
			return;
	}

//	#endif

	//
	// How many wmove faces does this thing have?
	//

	number = WMOVE_get_num_faces(p_thing);

	for (i = 0; i < number; i++)
	{
		ASSERT(WITHIN(WMOVE_face_upto, 1, WMOVE_MAX_FACES - 1));

		wf = &WMOVE_face[WMOVE_face_upto];

		//
		// Find the position of this face.
		//

		WMOVE_get_pos(
			p_thing,
			wf->last,
			i);

		dax = wf->last[1].x - wf->last[0].x;
		day = wf->last[1].y - wf->last[0].y;
		daz = wf->last[1].z - wf->last[0].z;

		dax = wf->last[2].x - wf->last[0].x;
		day = wf->last[2].y - wf->last[0].y;
		daz = wf->last[2].z - wf->last[0].z;

		//
		// Create four primpoints using these positions.
		//

		ASSERT(next_prim_point + 4 <= MAX_PRIM_POINTS);	

		pp = &prim_points[next_prim_point];

		pp[0].X = wf->last[0].x;
		pp[0].Y = wf->last[0].y;
		pp[0].Z = wf->last[0].z;

		pp[1].X = wf->last[1].x;
		pp[1].Y = wf->last[1].y;
		pp[1].Z = wf->last[1].z;

		pp[2].X = wf->last[2].x;
		pp[2].Y = wf->last[2].y;
		pp[2].Z = wf->last[2].z;

		pp[3].X = wf->last[0].x + dax + dbx;
		pp[3].Y = wf->last[0].y + day + dby;
		pp[3].Z = wf->last[0].z + daz + dbz;

		//
		// Create a walkable face.
		//

		ASSERT(next_prim_face4 < MAX_PRIM_FACES4);

		f4 = &prim_faces4[next_prim_face4];

		memset(f4, 0, sizeof(PrimFace4));

		f4->Points[0]  = next_prim_point + 0;
		f4->Points[1]  = next_prim_point + 1;
		f4->Points[2]  = next_prim_point + 2;
		f4->Points[3]  = next_prim_point + 3;
		f4->ThingIndex = WMOVE_face_upto;

		f4->FaceFlags  =
			FACE_FLAG_WALKABLE |
			FACE_FLAG_WMOVE;

		//
		// If we want to be able to grab...
		//

		if (p_thing->Class != CLASS_VEHICLE || (p_thing->Class == CLASS_VEHICLE && (p_thing->Genus.Vehicle->Type == VEH_TYPE_VAN || p_thing->Genus.Vehicle->Type == VEH_TYPE_WILDCATVAN || p_thing->Genus.Vehicle->Type == VEH_TYPE_AMBULANCE)))
		{
			f4->FaceFlags |=
				FACE_FLAG_SLIDE_EDGE_0 |
				FACE_FLAG_SLIDE_EDGE_1 |
				FACE_FLAG_SLIDE_EDGE_2 |
				FACE_FLAG_SLIDE_EDGE_3;
		}

		//
		// Finish initialising everything.
		//

		face4 = next_prim_face4;
		wmove = WMOVE_face_upto;

		next_prim_face4 += 1;
		next_prim_point += 4;
		WMOVE_face_upto += 1;
		DebugText(" next_prim_point %d primface3 %d primface4 %d   WMOVE FACE %d \n",next_prim_point,next_prim_face3,next_prim_face4,WMOVE_face_upto);

		wf->face4  = face4;
		wf->thing  = THING_NUMBER(p_thing);
		wf->number = i;

		//
		// Add the face to the walkable mapwho.
		//

		attach_walkable_to_map(face4);
	}
};

#if	0
void WMOVE_remove(UBYTE which_class)
{
	SLONG i;

	WMOVE_Face *wf;
	Thing      *p_thing;

	for (i = 1; i < WMOVE_face_upto; i++)
	{
		wf = &WMOVE_face[i];

		p_thing = TO_THING(wf->thing);

		if (p_thing->Class == which_class)
		{
			//
			// Remove this WMOVE_Face's walkable face from the map.
			//

			remove_walkable_from_map(wf->face4);

			//
			// Copy over this face.
			// 

			memmove((UBYTE*)wf, (UBYTE *)(wf + 1), (WMOVE_face_upto - i - 1) * sizeof(WMOVE_Face));

			//
			// Do the same face number again because it will have changed.
			//

			i               -= 1;
			WMOVE_face_upto -= 1;
		}
	}
}
#endif


void WMOVE_process()
{
	SLONG i;

	WMOVE_Point now[3];

	SLONG dax;
	SLONG day;
	SLONG daz;

	SLONG dbx;
	SLONG dby;
	SLONG dbz;

	WMOVE_Face *wf;
	PrimFace4  *f4;
	Thing      *p_thing;

	//
	// Update the positions of all the walkable faces.
	//

	for (i = 1; i < WMOVE_face_upto; i++)
	{
		wf = &WMOVE_face[i];

		if (wf->thing == NULL)
		{
			//
			// This face is no longer used.
			//

			continue;
		}

		p_thing = TO_THING(wf->thing);

		ASSERT(WITHIN(wf->face4, 1, next_prim_face4 - 1));
		
		f4 = &prim_faces4[wf->face4];

		ASSERT(WITHIN(f4->Points[0], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[1], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[2], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[3], 1, next_prim_point - 1));

		//
		// Store the old position.
		//

		wf->last[0].x = prim_points[f4->Points[0]].X;
		wf->last[0].y = prim_points[f4->Points[0]].Y;
		wf->last[0].z = prim_points[f4->Points[0]].Z;

		wf->last[1].x = prim_points[f4->Points[1]].X;
		wf->last[1].y = prim_points[f4->Points[1]].Y;
		wf->last[1].z = prim_points[f4->Points[1]].Z;

		wf->last[2].x = prim_points[f4->Points[2]].X;
		wf->last[2].y = prim_points[f4->Points[2]].Y;
		wf->last[2].z = prim_points[f4->Points[2]].Z;

		//
		// If the thing that owns this face is a stationary vehicle then there is no
		// need to process the face.
		//

		if (p_thing->Class == CLASS_VEHICLE)
		{
			if (p_thing->State == STATE_DEAD)
			{
				if (!(p_thing->Flags & FLAGS_ON_MAPWHO))
				{
					//
					// This car has disappeared from the map!
					//

					remove_walkable_from_map(wf->face4);
					
					//
					// Mark as unused.
					//

					wf->thing = NULL;

					return;
				}
			}

			if (p_thing->Genus.Vehicle->still >= 64)
			{
				continue;
			}
		}

		//
		// Remove from map.
		//

		remove_walkable_from_map(wf->face4);

		//
		// Update the new position.
		//

		WMOVE_get_pos(
			p_thing,
			now,
			wf->number);

		dax = now[1].x - now[0].x;
		day = now[1].y - now[0].y;
		daz = now[1].z - now[0].z;

		dbx = now[2].x - now[0].x;
		dby = now[2].y - now[0].y;
		dbz = now[2].z - now[0].z;

		prim_points[f4->Points[0]].X = now[0].x;
		prim_points[f4->Points[0]].Y = now[0].y;
		prim_points[f4->Points[0]].Z = now[0].z;

		prim_points[f4->Points[1]].X = now[1].x;
		prim_points[f4->Points[1]].Y = now[1].y;
		prim_points[f4->Points[1]].Z = now[1].z;

		prim_points[f4->Points[2]].X = now[2].x;
		prim_points[f4->Points[2]].Y = now[2].y;
		prim_points[f4->Points[2]].Z = now[2].z;

		prim_points[f4->Points[3]].X = now[0].x + dax + dbx;
		prim_points[f4->Points[3]].Y = now[0].y + day + dby;
		prim_points[f4->Points[3]].Z = now[0].z + daz + dbz;

		//
		// Add to map.
		//

		attach_walkable_to_map(wf->face4);
	}
}


void WMOVE_relative_pos(
		UBYTE  wmove_index,	// The WMOVE face stood on.
		SLONG  last_x,
		SLONG  last_y,
		SLONG  last_z,
		SLONG *now_x,
		SLONG *now_y,
		SLONG *now_z,
		SLONG *now_dangle)
{
	SLONG xo;
	SLONG yo;
	SLONG zo;

	SLONG dax;
	SLONG day;
	SLONG daz;

	SLONG dbx;
	SLONG dby;
	SLONG dbz;

	SLONG along_a;
	SLONG along_b;

	SLONG alen;
	SLONG blen;

	SLONG rx;
	SLONG rz;

	SLONG dy;
	SLONG wy;

	SLONG angle_old;
	SLONG angle_new;

	SLONG dangle;

	WMOVE_Face *wf;
	PrimFace4  *f4;
	PrimPoint  *pp0;
	PrimPoint  *pp1;
	PrimPoint  *pp2;
	PrimPoint  *pp3;

	ASSERT(WITHIN(wmove_index, 1, WMOVE_face_upto - 1));

	wf = &WMOVE_face[wmove_index];

	//
	// The vectors now.
	//

	ASSERT(WITHIN(wf->face4, 1, next_prim_face4 - 1));
	
	f4 = &prim_faces4[wf->face4];

	ASSERT(WITHIN(f4->Points[0], 1, next_prim_point - 1));
	ASSERT(WITHIN(f4->Points[1], 1, next_prim_point - 1));
	ASSERT(WITHIN(f4->Points[2], 1, next_prim_point - 1));
	ASSERT(WITHIN(f4->Points[3], 1, next_prim_point - 1));

	pp0 = &prim_points[f4->Points[0]];
	pp1 = &prim_points[f4->Points[1]];
	pp2 = &prim_points[f4->Points[2]];
	pp3 = &prim_points[f4->Points[3]];

	/*

	//
	// If the face hasn't moved- then don't update the position.
	//

	if (wf->last[0].x == pp0->X &&
		wf->last[0].y == pp0->Y &&
		wf->last[0].z == pp0->Z &&
		wf->last[1].x == pp1->X &&
		wf->last[1].y == pp1->Y &&
		wf->last[1].z == pp1->Z)
	{
	   *now_x = last_x;
	   *now_y = last_y;
	   *now_z = last_z;

	   *now_dangle = 0;

		return;
	}

	*/

	//
	// The last vectors.
	//

	xo = wf->last[0].x;
	yo = wf->last[0].y;
	zo = wf->last[0].z;

	dax = wf->last[1].x - wf->last[0].x;
	day = wf->last[1].y - wf->last[0].y;
	daz = wf->last[1].z - wf->last[0].z;

	dbx = wf->last[2].x - wf->last[0].x;
	dby = wf->last[2].y - wf->last[0].y;
	dbz = wf->last[2].z - wf->last[0].z;

	angle_old = calc_angle(dax,daz);

	//
	// Find the point (last_x,last_z) in terms of the origin and the
	// two vectors a and b.
	//

	rx = (last_x >> 8) - xo;
	rz = (last_z >> 8) - zo;

	SLONG acrossb = dax*dbz - daz*dbx + 1;
	SLONG rcrossa = rx *daz - rz *dax;
	SLONG rcrossb = rx *dbz - rz *dbx;

	along_a = (( rcrossb << 13) + (1 << 12)) / acrossb;
	along_b = ((-rcrossa << 13) + (1 << 12)) / acrossb;

	//
	// What is the y of the face at the last position?
	//

	wy = (yo << 8) + (day*along_a + dby*along_b + (1 << 9) >> 5);

	//
	// The last y offset.
	//

	dy = last_y - wy;

	//
	// Stop sliding up off the face.
	//

	if (abs(dy) < 0x200)
	{
		dy = 0;
	}

	//
	// Find the new (x,z) from these vectors.
	//

	xo  = pp0->X;
	yo  = pp0->Y;
	zo  = pp0->Z;

	dax = pp1->X - pp0->X;
	day = pp1->Y - pp0->Y;
	daz = pp1->Z - pp0->Z;

	dbx = pp2->X - pp0->X;
	dby = pp2->Y - pp0->Y;
	dbz = pp2->Z - pp0->Z;

   *now_x = (xo << 8) + (along_a*dax + along_b*dbx + (1 << 9) >> 5);
   *now_y = (yo << 8) + (along_a*day + along_b*dby + (1 << 9) >> 5);
   *now_z = (zo << 8) + (along_a*daz + along_b*dbz + (1 << 9) >> 5);

   *now_y += dy;

	angle_new = calc_angle(dax,daz);

	//
	// How much has the face turned by?
	//

	dangle = angle_new - angle_old;

	if (dangle < -1024) {dangle += 2048;}
	if (dangle > +1024) {dangle -= 2048;}

   *now_dangle = dangle;

	if (abs(*now_x - last_x) > 0x10000) {*now_x = last_x;}
	if (abs(*now_z - last_z) > 0x10000) {*now_z = last_z;}
}


#ifndef PSX
#ifndef TARGET_DC
void WMOVE_draw()
{
	SLONG i;

	WMOVE_Face *wf;
	PrimFace4  *f4;
	PrimPoint  *pp0;
	PrimPoint  *pp1;
	PrimPoint  *pp2;
	PrimPoint  *pp3;
	
	ULONG colour;
	
	if ((!allow_debug_keys)||!ControlFlag)
	{
		return;
	}

	for (i = 1; i < WMOVE_face_upto; i++)
	{
		wf = &WMOVE_face[i];

		ASSERT(WITHIN(wf->face4, 1, next_prim_face4 - 1));
		
		f4 = &prim_faces4[wf->face4];

		ASSERT(WITHIN(f4->Points[0], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[1], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[2], 1, next_prim_point - 1));
		ASSERT(WITHIN(f4->Points[3], 1, next_prim_point - 1));

		pp0 = &prim_points[f4->Points[0]];
		pp1 = &prim_points[f4->Points[1]];
		pp2 = &prim_points[f4->Points[2]];
		pp3 = &prim_points[f4->Points[3]];

		colour  = 0x12345678 * i;
		colour |= 10101010;

		AENG_world_line(
			pp0->X, pp0->Y, pp0->Z, 0x6, colour,
			pp1->X, pp1->Y, pp1->Z, 0x6, colour,
			TRUE);

		AENG_world_line(
			pp1->X, pp1->Y, pp1->Z, 0x6, colour,
			pp3->X, pp3->Y, pp3->Z, 0x6, colour,
			TRUE);

		AENG_world_line(
			pp3->X, pp3->Y, pp3->Z, 0x6, colour,
			pp2->X, pp2->Y, pp2->Z, 0x6, colour,
			TRUE);

		AENG_world_line(
			pp2->X, pp2->Y, pp2->Z, 0x6, colour,
			pp0->X, pp0->Y, pp0->Z, 0x6, colour,
			TRUE);
	}
}

#endif
#endif
