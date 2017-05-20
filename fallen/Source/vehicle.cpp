// vehicle.cpp
//
// vehicle physics

// tops of walls - colliding with them???
// need to properly handle coming out of skidding so bouncing off walls works better
// tune for each vehicle
// skidmarks
// new meshes ???

// include files

#define	DUMP_COORDS	0

#ifndef PSX
#include <math.h>
#endif

#include "game.h"
#ifndef	PSX
#include "c:\fallen\ddengine\headers\matrix.h"
#include "c:\fallen\ddengine\headers\poly.h"
#include "c:\fallen\ddengine\headers\oval.h"
#include "c:\fallen\ddlibrary\headers\ddlib.h"
#else
#include "c:\fallen\psxeng\headers\poly.h"
#endif
#include "pap.h"
#include "fmatrix.h"
#include "statedef.h"
#include "pcom.h"

#ifndef	PSX
#include "c:\fallen\ddengine\headers\aeng.h"
#include "c:\fallen\ddengine\headers\mesh.h"
#else
#include "c:\fallen\psxeng\headers\mesh.h"
#endif

#include "pow.h"

#include "interfac.h"
#include "dirt.h"
#include "mist.h"
#include "c:\fallen\editor\headers\prim.h"
#include "animate.h"
#include "sound.h"
#include "barrel.h"
#include "interact.h"
#include "ob.h"
#include "night.h"
#include "c:\fallen\ddengine\headers\drawxtra.h"
#include "psystem.h"

#include "mfx.h"

#include "memory.h"
#include "road.h"

#ifndef PSX
#include "font2d.h"
#endif






#if 0
#define ANNOYINGSCRIBBLECHECK ScribbleCheck()
static void ScribbleCheck ( void )
{
	ASSERT ( prim_faces4[1].Points[0] >= 48 );
	ASSERT ( prim_faces4[1].Points[0] < 62 );
	ASSERT ( prim_faces4[1].Points[1] >= 48 );
	ASSERT ( prim_faces4[1].Points[1] < 62 );
	ASSERT ( prim_faces4[1].Points[2] >= 48 );
	ASSERT ( prim_faces4[1].Points[2] < 62 );
	ASSERT ( prim_faces4[1].Points[3] >= 48 );
	ASSERT ( prim_faces4[1].Points[3] < 62 );
}

#else
#define ANNOYINGSCRIBBLECHECK
#endif






// Some externs
extern	SLONG	is_person_ko(Thing *p_person);


// constants for physics

#define	MIN_COMPRESSION		(13 << 8)
#define	MAX_COMPRESSION		(115 << 8)

#define	UNITS_PER_METER		128
#define	TICK_LOOP			(4)
#define	TICKS_PER_SECOND	(20*TICK_LOOP)
#define	GRAVITY				(-(UNITS_PER_METER*10*256)/(TICKS_PER_SECOND*TICKS_PER_SECOND))

#define WHEELTIME			35			// time to full lock
#define WHEELRATIO			45			// WHEELRATIO/WHEELTIME = tan(max. steering angle)
#define SKID_START			3			// number of tics to slam the brakes on before we skid
#define SKID_FORCE			8500		// lateral force required to start a skid
#define NEAR_SKID_FORCE		5000		// not skidding, but enough to starsky the sound fx

#define STABLE_COUNT		16			// number of "stable" tics to actually be stable

#define CAR_VEL_SHIFT	4

#ifndef PSX
extern BOOL allow_debug_keys;
#endif

static void siren(Vehicle* veh, UBYTE play);
static inline void GetCarPoints(Thing* p_car, SLONG* x, SLONG* y, SLONG* z, SLONG step);
extern	SLONG	is_person_ko_and_lay_down(Thing *p_person);
//
// random vehicle types
//
UBYTE	vehicle_random[] =
{
	VEH_TYPE_VAN,	VEH_TYPE_CAR,	VEH_TYPE_TAXI,	VEH_TYPE_JEEP,
	VEH_TYPE_SEDAN,	VEH_TYPE_VAN,	VEH_TYPE_CAR,	VEH_TYPE_TAXI,
	VEH_TYPE_JEEP,	VEH_TYPE_SEDAN,	VEH_TYPE_VAN,	VEH_TYPE_CAR, 
	VEH_TYPE_TAXI,	VEH_TYPE_VAN,	VEH_TYPE_CAR,	VEH_TYPE_TAXI
};

// VehInfo
//
// vehicle information per type

struct	VehInfo
{
	SWORD	DX[4],DZ[4];

	SBYTE	FwdAccel;			// note, terminal velocity depends on this!
	SBYTE	BkAccel;
	SBYTE	SoftBrake;
	SBYTE	HardBrake;

	UWORD	Reserved;
	SWORD	BodyDy;
	UWORD	WheelPrim;
	UWORD	BodyPrim;
	SWORD   BodyOffset;
	UWORD	NumVertices;
	UBYTE*	VertexAssignments;

	SWORD	HLX;				// headlights x,y,z
	SWORD	HLY;
	SWORD	HLZ;

	SWORD	BLX;				// brakelights x,y,z; z = 0 ==> no brakelights
	SWORD	BLY;
	SWORD	BLZ;

	SWORD	FLX;				// flashing lights x,y,z; z = 0 ==> no flashing lights
	SWORD	FLY;
	SWORD	FLZ;
	SWORD	FLRED;				// if 1, the lights are both red (else blue & red)

	UBYTE	shad_size;
	UBYTE	shad_elongate;		// In 6-bit fixed point!
};


#define	WHEELBASE_VAN	{-120,120,-120,120},{150,150,-165,-165}
#define WHEELBASE_CAR	{- 85, 85,- 85, 85},{160,160,-120,-120}

#define	ENGINE_LGV		17, 10, 4, 8	// light goods vehicle - slow in both directions, normal brakes
#define ENGINE_CAR		21, 10, 4, 8	// car - faster than LGV, same brakes
#define ENGINE_PIG		25, 15, 5, 10	// cop car - faster and better brakes
#define ENGINE_AMB		25, 10, 5, 8	// ambulance - cop car speed forwards; LGV speed backwards; better than average brakes

struct VehInfo veh_info[VEH_TYPE_NUMBER] =
{
	{WHEELBASE_VAN, ENGINE_LGV, 0,30,PRIM_OBJ_VAN_WHEEL,PRIM_OBJ_VAN_BODY,        0x6800, 0, NULL, -248, 15, 90,    0,  0,  0,   0,   0,  0, 0, 185, 100},
	{WHEELBASE_CAR, ENGINE_CAR, 0,30,PRIM_OBJ_CAR_WHEEL,PRIM_OBJ_CAR_BODY,        0x4000, 0, NULL, -205,  0, 60,  235, -8, 60,   0,   0,  0, 0, 125, 160},
	{WHEELBASE_CAR, ENGINE_CAR, 0,30,PRIM_OBJ_CAR_WHEEL,PRIM_OBJ_TAXI_BODY,       0x4000, 0, NULL, -225,  0, 60,  245, -8, 60,   0,   0,  0, 0, 125, 160},
	{WHEELBASE_CAR, ENGINE_PIG, 0,30,PRIM_OBJ_CAR_WHEEL,PRIM_OBJ_POLICE_BODY,     0x4000, 0, NULL, -225,  0, 60,  205,-24, 60,   0,  70, 40, 0, 125, 160},
	{WHEELBASE_VAN, ENGINE_AMB, 0,30,PRIM_OBJ_VAN_WHEEL,PRIM_OBJ_AMBULANCE_BODY,  0x6800, 0, NULL, -248, 15, 90,  240, 15, 90, -40, 210, 50, 1, 185, 100},
	{WHEELBASE_VAN, ENGINE_CAR, 0,30,PRIM_OBJ_VAN_WHEEL,PRIM_OBJ_JEEP_BODY,		  0x6800, 0, NULL, -225,-10, 80,  240, -5, 90,   0,   0,  0, 0, 185, 100},
	{WHEELBASE_VAN, ENGINE_AMB, 0,30,PRIM_OBJ_VAN_WHEEL,PRIM_OBJ_MEATWAGON_BODY,  0x6800, 0, NULL, -240,-10, 80,  240, 15, 90,  50, 140, 40, 0, 185, 100},
	{WHEELBASE_CAR, ENGINE_PIG, 0,30,PRIM_OBJ_CAR_WHEEL,PRIM_OBJ_SEDAN_BODY,	  0x4000, 0, NULL, -225,-20, 60,  205,-24, 70,   0,   0,  0, 0, 125, 160},
	{WHEELBASE_VAN, ENGINE_LGV, 0,30,PRIM_OBJ_VAN_WHEEL,PRIM_OBJ_WILDCATVAN_BODY, 0x6800, 0, NULL, -248, 15, 90,    0,  0,  0,   0,   0,  0, 0, 185, 100},
};


// debug stuff
#if !defined(PSX) && !defined(TARGET_DC)
#ifndef NDEBUG

Thing*	SelectedThing = NULL;

void LookForSelectedThing()
{
	SLONG world_x;
	SLONG world_y;
	SLONG world_z;

	SelectedThing = NULL;

	RECT	client;

	GetClientRect(hDDLibWindow, &client);

	float	hitx = float(MouseX) * float(DisplayWidth) / float(client.right - client.left);
	float	hity = float(MouseY) * float(DisplayHeight) / float(client.bottom - client.top);

	AENG_raytraced_position(
		SLONG(hitx + 0.5f),
		SLONG(hity + 0.5f),
	   &world_x,
	   &world_y,
	   &world_z);

	if (WITHIN(world_x, 600, (PAP_SIZE_HI << PAP_SHIFT_HI) - 601) &&
		WITHIN(world_z, 600, (PAP_SIZE_HI << PAP_SHIFT_HI) - 601))
	{
		UWORD	found[8];

		SLONG	num = THING_find_sphere(world_x, world_y, world_z, 0x100, found, 8, 1 << CLASS_VEHICLE);

		if (num == 1)
		{
			SelectedThing = TO_THING(found[0]);
		}
	}
}

#endif
#endif

//
// I KNOW THIS IS NASTY!
//

UBYTE sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim;

void get_car_door_offsets(SLONG type, SLONG door, SLONG *dx,SLONG *dz)
{
	ASSERT(door == 0 || door == 1);

	if (sneaky_do_it_for_positioning_a_person_to_do_the_enter_anim)
	{
		*dx=(veh_info[type].DX[door + 1]*450)>>8;
	}
	else
	{
		*dx=(veh_info[type].DX[door + 1]*300)>>8;
	}

	*dz=(veh_info[type].DZ[door + 1]*50)>>8;
}

SLONG	VEH_collide_line_ignore_walls = 0;

UWORD get_vehicle_body_prim(SLONG type)
{
	ASSERT(WITHIN(type, 0, VEH_TYPE_NUMBER - 1));

	return(veh_info[type].BodyPrim);
}

SLONG get_vehicle_body_offset(SLONG type)
{
	ASSERT(WITHIN(type, 0, VEH_TYPE_NUMBER - 1));

	return(SLONG(veh_info[type].BodyOffset));
}

// state functions

StateFunction VEH_statefunctions[] =
{
	{STATE_INIT,            NULL},
	{STATE_NORMAL,          NULL},
	{STATE_COLLISION,       NULL},
	{STATE_ABOUT_TO_REMOVE, NULL},
	{STATE_REMOVE_ME,       NULL},
	{STATE_MOVEING,         NULL},
	{STATE_FDRIVING,        VEH_driving},
	{STATE_FDOOR,			NULL}
};

// utilities

// make car matrix from yaw,tilt,roll

static SLONG	car_matrix[9];

static void make_car_matrix(Vehicle* v)
{
//#ifndef PSX
	FMATRIX_calc(car_matrix, v->Angle, v->Tilt, v->Roll);
//#else
//	FMATRIX_calc(car_matrix, -v->Angle, v->Tilt, v->Roll);
//#endif
}

static void make_car_matrix_p(SLONG angle, SLONG tilt, SLONG roll)
{
	FMATRIX_calc(car_matrix, angle, tilt, roll);
}

static void	apply_car_matrix(SLONG *x,SLONG *y,SLONG *z)
{
	SLONG tx,ty,tz;

	tx=*x; ty=*y; tz=*z;
	FMATRIX_MUL_BY_TRANSPOSE(car_matrix,tx,ty,tz);
	*x=tx; *y=ty; *z=tz;
}

//
// Returns NULL if there isn't a driver...
//

Thing *get_vehicle_driver(Thing *p_vehicle)
{
	Thing *p_driver;

	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	if (p_vehicle->Genus.Vehicle->Driver)
	{
		p_driver = TO_THING(p_vehicle->Genus.Vehicle->Driver);
	}
	else
	{
		p_driver = NULL;
	}

	return p_driver;
}

// are we driving a car?

inline bool is_driven_by_player(Thing* p_car)
{
	if (p_car->Genus.Vehicle->Driver)
	{
		Thing* p_driver = TO_THING(p_car->Genus.Vehicle->Driver);

		ASSERT(p_driver->Class == CLASS_PERSON);

		if (p_driver->Genus.Person->PlayerID)
		{
			return true;
		}
	}

	return false;
}

// find things we might run over

SLONG VEH_find_runover_things(Thing *p_vehicle, UWORD thing_index[], SLONG max_number, SLONG dangle)
{
	SLONG i;

	SLONG dx;
	SLONG dz;

	SLONG cx;
	SLONG cy;
	SLONG cz;

	SLONG num;
	SLONG angle;
	SLONG infront;

	//
	// A vector pointing out in front of the car and how far infront
	// of the car shall we look for things.  The more the car is
	// turning the closer in we check.
	//

#define MAX_INFRONT		512

	switch(p_vehicle->Class)
	{
		case CLASS_VEHICLE:
			angle   = p_vehicle->Genus.Vehicle->Angle & 2047;
			infront = MAX_INFRONT;
			if (!dangle)	infront -= abs(p_vehicle->Genus.Vehicle->WheelAngle * 3);
			break;

		#ifdef BIKE

		case CLASS_BIKE:
			angle   = p_vehicle->Draw.Mesh->Angle & 2047;
			infront = MAX_INFRONT;
			if (!dangle)	infront -= abs(BIKE_control_get(p_vehicle).steer) * 8;
			break;

		#endif

		default:
			ASSERT(0);
			break;
	}

	dx = -SIN(angle);
	dz = -COS(angle);

	SATURATE(infront, 256, MAX_INFRONT);

	dx = dx * infront >> 8;
	dz = dz * infront >> 8;

	cx = p_vehicle->WorldPos.X + dx >> 8;
	cy = p_vehicle->WorldPos.Y      >> 8;
	cz = p_vehicle->WorldPos.Z + dz >> 8;

	//
	// Draw a line to where we are checking for people.
	//
#ifndef PSX
#ifndef TARGET_DC
	if (ControlFlag&&allow_debug_keys)
	{
		AENG_world_line(
			p_vehicle->WorldPos.X >> 8,
			p_vehicle->WorldPos.Y >> 8,
			p_vehicle->WorldPos.Z >> 8,
			32,
			0xffffff,
			cx,
			cy,
			cz,
			0,
			0xff0000,
			TRUE);
	}
#endif
#endif
	//
	// Look for things at this position.
	//

	num = THING_find_sphere(
			cx, cy, cz,
			infront,	// 0x140,
			thing_index,
			max_number,
			(1 << CLASS_PERSON) | (1 << CLASS_VEHICLE));

	//
	// Look for things ahead
	//

	cx += dx >> 8;
	cz += dz >> 8;

	num += THING_find_sphere(cx, cy, cz, infront, thing_index + num, max_number - num, (1 << CLASS_PERSON) | (1 << CLASS_VEHICLE));

	//
	// Make sure that we are not in the list!
	//

	for (i = 0; i < num; i++)
	{
		if (thing_index[i] == THING_NUMBER(p_vehicle))
		{
			thing_index[i] = thing_index[--num];

			break;
		}
	}

	return num;
}

// find door position

void VEH_find_door(Thing *p_vehicle, SLONG i_am_a_passenger, SLONG *door_x, SLONG *door_z)
{
	SLONG dx;
	SLONG dz;

	SLONG ix;
	SLONG iz;

	ASSERT(p_vehicle->Class == CLASS_VEHICLE);

	dx = -SIN(p_vehicle->Genus.Vehicle->Angle);
	dz = -COS(p_vehicle->Genus.Vehicle->Angle);

	ix = p_vehicle->WorldPos.X >> 8;
	iz = p_vehicle->WorldPos.Z >> 8;

	if (i_am_a_passenger)
	{
/*
		switch(p_vehicle->Genus.Vehicle->Type)
		{
			case VEH_TYPE_VAN:
			case VEH_TYPE_AMBULANCE:
			case VEH_TYPE_JEEP:
			case VEH_TYPE_MEATWAGON:
			case VEH_TYPE_WILDCATVAN:

				//
				// Get in the back of the van.
				//

				ix -= dx >> 10;
				iz -= dz >> 10;

				break;

			case VEH_TYPE_CAR:
			case VEH_TYPE_TAXI:
			case VEH_TYPE_POLICE:
			case VEH_TYPE_SEDAN:

				//
				// The back seat on the same side as what Darci gets in.
				//

				ix -= dx >> 10;
				iz -= dz >> 10;

				ix += dz >> 9;
				iz -= dx >> 9;

				break;

			default:
				ASSERT(0);
				break;
		}
*/

		ix += dx >> 10;
		iz += dz >> 10;

		ix -= dz >> 9;
		iz += dx >> 9;

		ix -= dz >> 11;
		iz += dx >> 11;

	}
	else
	{
		ix += dx >> 10;
		iz += dz >> 10;

		ix += dz >> 9;
		iz -= dx >> 9;

		ix += dz >> 11;
		iz -= dx >> 11;
	}

   *door_x = ix;
   *door_z = iz;
}

//
// Returns TRUE if the given car is completely on the road.
//

SLONG VEH_on_road(Thing *p_vehicle, SLONG step)
{
	SLONG x[4];
	SLONG y[4];
	SLONG z[4];

	// get rotated points
	GetCarPoints(p_vehicle, x, y, z, step);

	// check 8 points along each edge
	for (int i = 0; i < 4; i++)
	{
		SLONG	x1 = x[i];
		SLONG	z1 = z[i];

		SLONG	x2 = x[(i + 1) & 0x3];
		SLONG	z2 = z[(i + 1) & 0x3];

		SLONG	dx = (x2 - x1) >> 3;
		SLONG	dz = (z2 - z1) >> 3;

		SLONG	cx = x1;
		SLONG	cz = z1;

		for (int j = 0; j < 7; j++)
		{
			if (!ROAD_is_road(cx >> 8, cz >> 8))
			{
				return FALSE;
			}

			cx += dx;
			cz += dz;
		}
	}

	return TRUE;
}

// add damage to a specified area of a vehicle

void VEH_add_damage(Vehicle* vp, UBYTE area, UBYTE hp)
{
	// max out at 2hp
	if (hp > 2)	hp = 2;

	if (!hp)
	{
		// small amount of damage
		if (vp->damage[area] < 2)	vp->damage[area]++;
	}
	else
	{
		vp->damage[area] += hp;
		if (vp->damage[area] > 4)	vp->damage[area] = 4;
	}

	// even out damage across sides
	if (vp->damage[2] < (vp->damage[0] + vp->damage[4])/2)
	{
		vp->damage[2] = (vp->damage[0] + vp->damage[4])/2;
	}
	if (vp->damage[3] < (vp->damage[1] + vp->damage[5])/2)
	{
		vp->damage[3] = (vp->damage[1] + vp->damage[5])/2;
	}

	// reduce damage at opposite corner
	if ((vp->damage[0] + vp->damage[1] > 6) && (area == 0))	vp->damage[1]--;
	if ((vp->damage[0] + vp->damage[1] > 6) && (area == 1))	vp->damage[0]--;
	
	if ((vp->damage[4] + vp->damage[5] > 6) && (area == 4))	vp->damage[5]--;
	if ((vp->damage[4] + vp->damage[5] > 6) && (area == 5))	vp->damage[4]--;

	// randomly reduce damage at diagonally opposite corner
	if ((hp == 2) && (Random() > 25000) && (vp->damage[5-area]))	vp->damage[5-area]--;
}

// bounce a vehicle

void VEH_bounce(Vehicle* vp, UBYTE area, SLONG amount)
{
	amount *= 2;

	switch (area)
	{
	case 0:
		vp->DY[1] -= amount;
		break;

	case 1:
		vp->DY[0] -= amount;
		break;

	case 2:
		vp->DY[1] -= amount;
		vp->DY[3] -= amount;
		break;

	case 3:
		vp->DY[0] -= amount;
		vp->DY[2] -= amount;
		break;

	case 4:
		vp->DY[3] -= amount;
		break;

	case 5:
		vp->DY[2] -= amount;
		break;		
	}
}

// init the vehicle array

#ifndef PSX
void init_vehicles(void)
{
	SLONG i;

	for (i = 0; i < MAX_VEHICLES; i++)
	{
		TO_VEHICLE(i)->Spring[0].Compression = VEH_NULL;
	}
}
#endif

// VEH_init_vehinfo
//
// assign each vertex to a crumple zone and initialize the
// arctan table (for steering)
//
// prims must be loaded!

static void init_arctans(void);



void VEH_init_vehinfo()
{
	int	ii;

	init_arctans();

#ifndef PSX

#ifdef TARGET_DC
	// Just allocate these once.
	static bool bAllocatedVertexAssignments = FALSE;
	if ( !bAllocatedVertexAssignments )
	{
		for (ii = 0; ii < VEH_TYPE_NUMBER; ii++)
		{
			veh_info[ii].VertexAssignments = NULL;
		}
	}

#else
	for (ii = 0; ii < VEH_TYPE_NUMBER; ii++)
	{
		if (veh_info[ii].VertexAssignments)
		{
			MemFree(veh_info[ii].VertexAssignments);
		}
	}
#endif

	for (ii = 0; ii < VEH_TYPE_NUMBER; ii++)
	{
		PrimObject*	obj = &prim_objects[veh_info[ii].BodyPrim];
		PrimInfo*	inf = get_prim_info(veh_info[ii].BodyPrim);
		SLONG		px[6],pz[6];

		// create 6 crumple points
		px[0] = inf->minx;	pz[0] = inf->minz;
		px[1] = inf->maxx;	pz[1] = inf->minz;
		px[2] = inf->minx;	pz[2] = (inf->minz + inf->maxz) / 2;
		px[3] = inf->maxx;	pz[3] = (inf->minz + inf->maxz) / 2;
		px[4] = inf->minx;	pz[4] = inf->maxz;
		px[5] = inf->maxx;	pz[5] = inf->maxz;
				
		veh_info[ii].NumVertices = obj->EndPoint - obj->StartPoint;

#ifdef TARGET_DC
		if ( !bAllocatedVertexAssignments )
		{
			ASSERT ( veh_info[ii].VertexAssignments == NULL );
			veh_info[ii].VertexAssignments = (UBYTE*)MemAlloc(obj->EndPoint - obj->StartPoint);
		}
		else
		{
			ASSERT ( veh_info[ii].VertexAssignments != NULL );
		}
#else
		veh_info[ii].VertexAssignments = (UBYTE*)MemAlloc(obj->EndPoint - obj->StartPoint);
#endif

		// assign each vertex to the nearest crumple point
		for (int jj = obj->StartPoint; jj < obj->EndPoint; jj++)
		{
			SLONG	maxdist = 0x7FFFFFFF;
			SLONG	best = -1;

			SLONG	x = prim_points[jj].X;
			SLONG	z = prim_points[jj].Z;

			for (int kk = 0; kk < 6; kk++)
			{
				SLONG	dist = (x - px[kk])*(x - px[kk]) + (z - pz[kk])*(z - pz[kk]);
				
				if (dist < maxdist)
				{
					maxdist = dist;
					best = kk;
				}
			}

			ASSERT(best != -1);

			veh_info[ii].VertexAssignments[jj - obj->StartPoint] = best;
		}
	}

#ifdef TARGET_DC
	bAllocatedVertexAssignments = TRUE;
#endif

#endif
}

// find a free vehicle slot

Vehicle *VEH_alloc(void)
{
	SLONG i;

	for (i = 0; i < MAX_VEHICLES; i++)
	{
		if (TO_VEHICLE(i)->Spring[0].Compression == VEH_NULL)
		{
			Vehicle *ans = TO_VEHICLE(i);

			ans->Spring[0].Compression = MIN_COMPRESSION;

			return ans;
		}
	}

	ASSERT(0);

	return NULL;
}

// free a vehicle slot

void VEH_dealloc(Vehicle *veh)
{
	veh->Spring[0].Compression = VEH_NULL;
	if (veh->dlight) NIGHT_dlight_destroy(veh->dlight);
}

//
// Initialises a vehicles drawmesh structure.
//

static void set_vehicle_draw(Thing *p_thing)
{
	DrawTween	*draw;
	ASSERT(0);
	return;

	draw=&p_thing->Genus.Vehicle->Draw;

	draw->Angle				=	p_thing->Genus.Vehicle->Angle;
	draw->Roll				=	0;
	draw->Tilt				=	0;
	draw->AnimTween			=	0;
	draw->TweenStage		=	0;
	draw->CurrentFrame		=	game_chunk[5].AnimList[2];
	draw->NextFrame			=	draw->CurrentFrame->NextFrame;
	draw->QueuedFrame		=	NULL;
	draw->TheChunk			=	&game_chunk[5];
	draw->FrameIndex		=	0;
	draw->Flags				=	0;
}
THING_INDEX VEH_create(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG type,
		UBYTE key,
		UBYTE colour)
{
	DrawMesh*	dm;
	THING_INDEX ans = NULL;
	Thing*		p_thing;
	int			ii;

ANNOYINGSCRIBBLECHECK;

	ASSERT(WITHIN(type, 0, VEH_TYPE_NUMBER - 1));

	//
	// Looks like vehicles treat their yaw strangely...
	//

	yaw += 1024;
	yaw &= 2047;

#if defined(FAST_EDDIE) && 0

	//
	// OK, now we'll ask the road system about our position
	//

	SLONG	rn1,rn2;

	ROAD_find(x >> 8, z >> 8, &rn1, &rn2);

	SLONG	rd = ROAD_signed_dist(rn1, rn2, x >> 8, z >> 8);

	if (rd < 0)
	{
		SLONG	tmp = rn1; rn1 = rn2; rn2 = tmp;		
	}

	SLONG	x1,z1;
	SLONG	x2,z2;

ANNOYINGSCRIBBLECHECK;

	ROAD_node_pos(rn1, &x1, &z1);
	ROAD_node_pos(rn2, &x2, &z2);

ANNOYINGSCRIBBLECHECK;

	SLONG	dx = -SIN(yaw);
	SLONG	dz = -COS(yaw);

	if (dx * (x2 - x1) + dz * (z2 - z1) < 0)
	{
		// facing the wrong way
		yaw ^= 1024;
	}
#endif

	//
	// Get a DrawMesh for this thing.
	//
ANNOYINGSCRIBBLECHECK;
	dm = alloc_draw_mesh();
ANNOYINGSCRIBBLECHECK;

	if (dm)
	{
ANNOYINGSCRIBBLECHECK;
		ans = alloc_primary_thing(CLASS_VEHICLE);
ANNOYINGSCRIBBLECHECK;

		if (ans)
		{
			Vehicle*	vp;

ANNOYINGSCRIBBLECHECK;
			p_thing = TO_THING(ans);

			//
			// Initialise the thing.
			//

			p_thing->Class      = CLASS_VEHICLE;
			p_thing->State      = 0;
			p_thing->SubState   = 0;
			p_thing->DrawType   = DT_VEHICLE;
			p_thing->Flags      = 0;
			p_thing->WorldPos.X = x;
			p_thing->WorldPos.Y = y;
			p_thing->WorldPos.Z = z;

			p_thing->Draw.Mesh  = dm;
ANNOYINGSCRIBBLECHECK;

			dm->Angle = yaw;

			vp = p_thing->Genus.Vehicle = VEH_alloc();
ANNOYINGSCRIBBLECHECK;

			ASSERT(vp);

			vp->Angle  = yaw;
			vp->Roll   = roll;
			vp->Tilt   = pitch;
			vp->Flags  = 0;

ANNOYINGSCRIBBLECHECK;
			for (ii = 0; ii < 4; ii++)
			{
				vp->Spring[ii].Compression = MIN_COMPRESSION;
				vp->DY[ii] = 0;
			}

ANNOYINGSCRIBBLECHECK;
			vp->Type = type;

			if (!(NIGHT_flag & NIGHT_FLAG_DAYTIME)) {
ANNOYINGSCRIBBLECHECK;
				vp->dlight = NIGHT_dlight_create( p_thing->WorldPos.X>>8, p_thing->WorldPos.Y>>8, p_thing->WorldPos.Z>>8, 200, 35, 32, 10);
ANNOYINGSCRIBBLECHECK;
			} else {
ANNOYINGSCRIBBLECHECK;
				vp->dlight=0;
ANNOYINGSCRIBBLECHECK;
			}

			// set extension parameters
ANNOYINGSCRIBBLECHECK;
			reinit_vehicle(p_thing);
ANNOYINGSCRIBBLECHECK;

			set_state_function(p_thing, STATE_FDRIVING);
//			set_vehicle_draw(p_thing);

			//
			// Initialises more stuff...
			//

ANNOYINGSCRIBBLECHECK;
			p_thing->Genus.Vehicle->Roll	= 0;
			p_thing->Genus.Vehicle->Tilt	= 0;
			p_thing->Genus.Vehicle->key		= key;

			//
			// Place on the mapwho.
			//

ANNOYINGSCRIBBLECHECK;
			add_thing_to_map(p_thing);
ANNOYINGSCRIBBLECHECK;
		}
		else
		{
			ASSERT(0);
		}
	}
	else
	{
		ASSERT(0);
	}

ANNOYINGSCRIBBLECHECK;
	return ans;
}

void reinit_vehicle(Thing* p_thing)
{
	Vehicle*	vp = p_thing->Genus.Vehicle;

	vp->VelX        = 0;
	vp->VelY		= 0;
	vp->VelZ        = 0;
	vp->VelR        = 0;
	vp->Dir         = 0;
	vp->WheelAngle  = 0;
	vp->Skid        = 0;
	vp->Stable      = 0;
	vp->Smokin      = 0;
	vp->OnRoadFlags = 0;
	vp->Health      = 300;
	vp->Siren       = 0;
	vp->GrabAction	= 0;

	for (int ii = 0; ii < 6; ii++)
	{
		vp->damage[ii] = 0;
	}

	SLONG	height;
	
	for (ii = 0; ii < 4; ii++)
	{
		vp->DY[ii] = 0;
		vp->Spring[ii].Compression = 4300;
	}

	vp->Tilt = 0;
	vp->Roll = 0;
	height = PAP_calc_map_height_at(p_thing->WorldPos.X >> 8, p_thing->WorldPos.Z >> 8);
	//
	// 107 is length of suspension 128 *(100-%compressed)   =128*.84
	//
	p_thing->WorldPos.Y = (height + 107) << 8;
}

void free_vehicle(Thing *p_thing)
{
	ASSERT(0);
	if (p_thing->Genus.Vehicle)
	{
		VEH_dealloc(p_thing->Genus.Vehicle);
	}

	free_draw_mesh(p_thing->Draw.Mesh);

	free_thing(p_thing);
}



// static functions
//
// (don't static functions have the "static" keyword?)

SLONG	calc_car_collision_turn(Thing *p_car,SLONG angle,SLONG tilt,SLONG roll);
void	calc_car_normal(SLONG *p,SLONG *dy,SLONG *nx,SLONG *ny,SLONG *nz);
void	calc_car_vect(SLONG p1,SLONG p2,SLONG *dy,SLONG *vx,SLONG *vy,SLONG *vz);
void	normalise_val256(SLONG *vx,SLONG *vy,SLONG *vz);






void	animate_car(Thing *p_car)
{
	SLONG	tween_step;
	DrawTween	*draw_info;
	ASSERT(0);
	return;


	draw_info=&p_car->Genus.Vehicle->Draw;
//	tween_step=256/(draw_info->CurrentFrame->TweenStep+1);
	tween_step=draw_info->CurrentFrame->TweenStep<<1;

	tween_step = (tween_step*TICK_RATIO)>>TICK_SHIFT;
	if(tween_step==0)
		tween_step=1;
	draw_info->AnimTween += tween_step; //256/(draw_info->CurrentFrame->TweenStep+1);

	if(p_car->Genus.Vehicle->Draw.AnimTween>256)
	{
		p_car->Genus.Vehicle->Draw.AnimTween-=256;

SLONG	advance_keyframe(DrawTween *draw_info);
		advance_keyframe(&p_car->Genus.Vehicle->Draw);
	}
}


void	draw_car(Thing *p_car)
{
	SLONG	x[8],y[8],z[8];
	SLONG	vector[3];
	SLONG	dx,dy,dz;
	SLONG	c0=0;
	struct	VehInfo	*info;
	SLONG	tilt;
	Vehicle*	vp;

	vp = p_car->Genus.Vehicle;
	info=&veh_info[p_car->Genus.Vehicle->Type];

	make_car_matrix(p_car->Genus.Vehicle);
#ifdef	PSX
/*
	{
		CBYTE	str[30];
extern	FONT2D_DrawString_3d(CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

		sprintf(str,"S%d W%d A%d d %d",p_car->Genus.Vehicle->Steering,p_car->Genus.Vehicle->Wheel,p_car->Genus.Vehicle->IsAnalog,(p_car->Genus.Vehicle->Flags & FLAG_FURN_DRIVING));

		FONT2D_DrawString_3d(str,p_car->WorldPos.X>>8,p_car->WorldPos.Y>>8,p_car->WorldPos.Z>>8,0xffffff,512,0);
//			CBYTE*str, ULONG world_x, ULONG world_y,ULONG world_z, ULONG rgb, SLONG text_size, SWORD fade);

	}
*/
#endif
#if !defined(PSX) && !defined(TARGET_DC)
	if (0)
	{
		//
		// Draw the car as an animating prim.
		//

		extern	void ANIM_obj_draw(Thing *p_thing,DrawTween *dt);

		p_car->WorldPos.Y -= info->BodyOffset;

		p_car->Genus.Vehicle->Draw.Angle = p_car->Genus.Vehicle->Angle;
		p_car->Genus.Vehicle->Draw.Tilt  = p_car->Genus.Vehicle->Tilt;
		p_car->Genus.Vehicle->Draw.Roll  = p_car->Genus.Vehicle->Roll;

		ANIM_obj_draw(p_car,&p_car->Genus.Vehicle->Draw);

		p_car->WorldPos.Y += info->BodyOffset;
	}
	else
#endif
	{
		//
		// Draw the car as a normal prim.
		// 
#ifndef PSX
		// set crumples
		MESH_set_crumple(info->VertexAssignments, p_car->Genus.Vehicle->damage);
#endif

		if(MESH_draw_poly(

#ifdef	PSX
			info->BodyPrim|(1<<16),	// The van,
#else
			info->BodyPrim,	// The van,
#endif

//			info->BodyPrim,	// The van,
			p_car->WorldPos.X                                                       >> 8,
			p_car->WorldPos.Y - get_vehicle_body_offset(p_car->Genus.Vehicle->Type) >> 8,
			p_car->WorldPos.Z                                                       >> 8,
			p_car->Genus.Vehicle->Angle,
			p_car->Genus.Vehicle->Tilt,
			p_car->Genus.Vehicle->Roll,
			NULL,
#ifndef PSX
			0xff,
			-1)==NULL)	// means use MESH_set_crumple parameters
#else
			0)==NULL)
#endif
		{
			//
			// nowt, one day skip drawing wheels if body fails to be drawn
			//

		}


		#ifndef PSX

		//
		// Draw the car shadow...
		//
		if(!SOFTWARE)
		{
			OVAL_add(
				p_car->WorldPos.X                                                       >> 8,
				p_car->WorldPos.Y - get_vehicle_body_offset(p_car->Genus.Vehicle->Type) >> 8,
				p_car->WorldPos.Z                                                       >> 8,
				float(veh_info[p_car->Genus.Vehicle->Type].shad_size    ) * 2.0F,
				float(veh_info[p_car->Genus.Vehicle->Type].shad_elongate) * (1.0F / 64.0F),
				float(p_car->Genus.Vehicle->Angle) * (2.0F * PI / 2048.0F),
				OVAL_TYPE_SQUARE);
		}

		#endif

//#ifndef PSX
		// transpose matrix for some reason
		FMATRIX_TRANSPOSE(car_matrix);
//#endif
		if (p_car->Genus.Vehicle->Flags & FLAG_FURN_DRIVING)
		{
			// headlights
#ifndef PSX
			if ( (!(NIGHT_flag & NIGHT_FLAG_DAYTIME)) && (!SOFTWARE))
			{
				static SLONG	xyz[5][3] = { -255,0,0, -254,12,16, -251,24,32, -247,36,48, -243,48,60 };

				dz = xyz[p_car->Genus.Vehicle->damage[1]][0];
				dy = xyz[p_car->Genus.Vehicle->damage[1]][1];
				dx = xyz[p_car->Genus.Vehicle->damage[1]][2];
				FMATRIX_MUL(car_matrix,dx,dy,dz);
				
				vector[2] =  info->HLX;
				vector[1] =  info->HLY;
				vector[0] =  info->HLZ;
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);
				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2],dx,dy,dz,0x606040,BLOOM_FLENSFLARE|BLOOM_BEAM);

				dz = xyz[p_car->Genus.Vehicle->damage[0]][0];
				dy = -xyz[p_car->Genus.Vehicle->damage[0]][1];
				dx = -xyz[p_car->Genus.Vehicle->damage[0]][2];
				FMATRIX_MUL(car_matrix,dx,dy,dz);

				vector[2] =  info->HLX;
				vector[1] =  info->HLY;
				vector[0] = -info->HLZ;
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);
				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2],dx,dy,dz,0x606040,BLOOM_FLENSFLARE|BLOOM_BEAM);
			}
#endif
			// flashing lights
			if (info->FLZ && (vp->Siren == 1))
			{
				SLONG	rx, rz;

				rx = SIN((SLONG(p_car)+(GAME_TURN<<7))&2047)>>8;
				rz = COS((SLONG(p_car)+(GAME_TURN<<7))&2047)>>8;

				vector[2] =  info->FLX + (rx >> 6);
				vector[1] =  info->FLY;
				vector[0] =  info->FLZ + (rz >> 6);
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);

				SLONG	colour = info->FLRED ? 0xDF0000 : 0x0000DF;

				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2], rx, 0, rz, colour, 0);

				vector[2] =  info->FLX + (rx >> 6);
				vector[1] =  info->FLY;
				vector[0] = -info->FLZ + (rz >> 6);
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);

				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2], rz, 0, rx, 0xDF0000, 0);
			}
		}

		// smoke from bonnet

		/*

		if (vp->damage[0] || vp->damage[1] || vp->damage[2] || vp->damage[3] || vp->damage[4] || vp->damage[5])
		{
			UBYTE total = vp->damage[0] + vp->damage[1] + vp->damage[2] + 
						  vp->damage[3] + vp->damage[4] + vp->damage[5];

			total /= 6;

			if ((Random()&7)<total)
			{
		*/

		{
			if ((Random()&0x7f)>p_car->Genus.Vehicle->Health)
			{
				vector[2] = info->HLX * 0.7f;
				vector[1] = info->HLY;
				vector[0] = 0;
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);
				PARTICLE_Add(
					p_car->WorldPos.X+(vector[0]<<8) + (Random() & 0x3fff) - 0x1fff,
					p_car->WorldPos.Y+(vector[1]<<8),
					p_car->WorldPos.Z+(vector[2]<<8) + (Random() & 0x3fff) - 0x1fff,
					vp->VelX+(Random()&0xff)-0x7f,
					0x3ff+(Random()&0xff),
					vp->VelZ+(Random()&0xff)-0x7f,
					POLY_PAGE_SMOKECLOUD,2+((Random()&3)<<2),0x7FFFFFFF,
					PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING,
					75,40+(rand()&31),1,4,1);

				if (p_car->Genus.Vehicle->Health <= 0)
				{
					//
					// Smoke from out the back of the vehicle aswell...
					// 

					PARTICLE_Add(
						p_car->WorldPos.X-(vector[0]<<8) + (Random() & 0x7fff) - 0x3fff,
						p_car->WorldPos.Y-(vector[1]<<8),
						p_car->WorldPos.Z-(vector[2]<<8) + (Random() & 0x7fff) - 0x3fff,
						vp->VelX+(Random()&0xff)-0x7f,
						0x3ff+(Random()&0xff),
						vp->VelZ+(Random()&0xff)-0x7f,
						POLY_PAGE_SMOKECLOUD,2+((Random()&3)<<2),0x7FFFFFFF,
						PFLAG_SPRITEANI|PFLAG_SPRITELOOP|PFLAG_FADE2|PFLAG_RESIZE|PFLAG_DAMPING,
						75,40+(rand()&31),1,4,1);
				}
			}
		}

		// brake/reversing lights are always shown, day or night
		if (info->BLZ)	// else no lights
		{
			SLONG	colour = 0;

			switch (p_car->Genus.Vehicle->Dir)
			{
			case -1:
			case  1:
				p_car->Genus.Vehicle->Brakelight = is_driven_by_player(p_car) ? 1 : 10;	// timer for NPCs (non-player cars)
				break;

			case -2:
				p_car->Genus.Vehicle->Brakelight = 0;
				if (p_car->Genus.Vehicle->DControl & VEH_DECEL)	//	colour = 0x606060;	// reversing light
					colour=0x303030;
				break;

			case  2:
				p_car->Genus.Vehicle->Brakelight = 0;
				break;

			case 0:
				break;
			}

			if (p_car->Genus.Vehicle->Brakelight)
			{
				p_car->Genus.Vehicle->Brakelight--;
				colour = 0x600000;	// brake light
			}

			if (colour)
			{
				dx = 0; dy = 0; dz =255;
				FMATRIX_MUL(car_matrix,dx,dy,dz);

				vector[2] =  info->BLX;
				vector[1] =  info->BLY;
				vector[0] =  info->BLZ;
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);
				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2],dx,dy,dz,colour,0);

				vector[2] =  info->BLX;
				vector[1] =  info->BLY;
				vector[0] = -info->BLZ;
				FMATRIX_MUL(car_matrix,vector[0],vector[1],vector[2]);
				BLOOM_draw( (p_car->WorldPos.X>>8)+vector[0],(p_car->WorldPos.Y>>8)+vector[1],(p_car->WorldPos.Z>>8)+vector[2],dx,dy,dz,colour,0);
			}
		}

		// return to original
		FMATRIX_TRANSPOSE(car_matrix);
//#endif

	}

	tilt = p_car->Genus.Vehicle->Spin;


void AENG_set_bike_wheel_rotation(UWORD rot, UBYTE prim);
	// I know it says "bike" on the box, but it doesn't mean "bike" in the box.
	// Leave it! This is needed.
	AENG_set_bike_wheel_rotation(tilt, info->WheelPrim);


#if !defined(PSX) && !defined(TARGET_DC)
#ifndef NDEBUG
	if (p_car != SelectedThing)	// don't draw wheels when selected	
#endif
#endif
	for(c0=0;c0<4;c0++)
	{
		SLONG	wx,wy,wz;
		SLONG	angle;

		wx = info->DX[c0];
		wy = 51 - (((128 << 8) - p_car->Genus.Vehicle->Spring[c0].Compression) >> 8);
		wz = info->DZ[c0];

		apply_car_matrix(&wx,&wy,&wz);

		if (c0>=2)
		{
			angle=p_car->Genus.Vehicle->Angle - p_car->Genus.Vehicle->WheelAngle;
			angle=(angle+2048)&2047;
		}
		else
		{
			angle=p_car->Genus.Vehicle->Angle;
		}

#ifndef TARGET_DC
		MESH_draw_poly(
			info->WheelPrim,
			(p_car->WorldPos.X>>8)+wx,
			(p_car->WorldPos.Y>>8)+wy,
			(p_car->WorldPos.Z>>8)+wz,
			angle,
			0,
			p_car->Genus.Vehicle->Roll,
			NULL,
			0);
#else
		MESH_draw_poly(
			info->WheelPrim,
			(p_car->WorldPos.X>>8)+wx,
			(p_car->WorldPos.Y>>8)+wy,
			(p_car->WorldPos.Z>>8)+wz,
			angle,
			0,
			p_car->Genus.Vehicle->Roll,
			NULL,
			0xff);
#endif
	}

	if (vp->Smokin)
	{
		vp->Smokin = 0;	// must be set each time

		for(c0=0;c0<4;c0++)
		{
			SLONG	wx,wy,wz;
			SLONG	speed;

			wx = info->DX[c0];
			wy = 51 - (((128 << 8) - vp->Spring[c0].Compression) >> 8);
			wz = info->DZ[c0];

			apply_car_matrix(&wx,&wy,&wz);

			wx = p_car->WorldPos.X + (wx << 8);
			wy = p_car->WorldPos.Y + (wy << 8);
			wz = p_car->WorldPos.Z + (wz << 8);
/*
			PARTICLE_Add(wx,wy,wz,
						(rand()&7)-3,20,(rand()&7)-3,
						POLY_PAGE_STEAM,1+((rand()&3)<<2),
						0x3FC9B7A3,PFLAG_FADE|PFLAG_RESIZE,
						500,	// life
						60,		// size
						1,
						20,		// fade
						4);*/
			PARTICLE_Add(wx,wy,wz,
				(rand()&7)-3,20,(rand()&7)-3,
				POLY_PAGE_SMOKECLOUD2,2+((Random()&3)<<2),0x7Fc9B7A3,PFLAG_FADE|PFLAG_RESIZE|PFLAG_SPRITEANI|PFLAG_SPRITELOOP,30,60,1,16,10);
			speed=((vp->VelX>>CAR_VEL_SHIFT)*(vp->VelX>>CAR_VEL_SHIFT))
				 +((vp->VelZ>>CAR_VEL_SHIFT)*(vp->VelZ>>CAR_VEL_SHIFT));

//			TRACE("speed: %d\n",speed);
			if ((speed>200)&&(vp->Skid==SKID_START)) {
#ifdef PSX
				MFX_play_thing(THING_NUMBER(p_car),S_SKID_START,MFX_MOVING,p_car);
#else
				if (speed>300000) 
					MFX_play_thing(THING_NUMBER(p_car),S_SKID_START,MFX_MOVING,p_car);
				else
					if (speed>100000)
						MFX_play_thing(THING_NUMBER(p_car),S_SKID_START+1,MFX_MOVING,p_car);
					else
						MFX_play_thing(THING_NUMBER(p_car),S_SKID_END,MFX_MOVING,p_car);

#endif
			}

			if ((speed>200)&&(GAME_TURN&1)) {
#ifndef PSX
				SLONG dx,dz;
				if (vp->oldX[c0]&&vp->oldZ[c0]) {
					dx=(vp->oldX[c0]-wx)>>8;
					dz=(vp->oldZ[c0]-wz)>>8;
					TRACKS_Add(wx,(PAP_calc_map_height_at(wx>>8,wz>>8)+5)<<8,wz,dx,0,dz,TRACK_TYPE_TYRE_SKID,0);
				}
				vp->oldX[c0]=wx;
				vp->oldZ[c0]=wz;
#else
				TRACKS_Add(wx,(PAP_calc_map_height_at(wx>>8,wz>>8)+5)<<8,wz,-vp->VelX>>7,0,-vp->VelZ>>7,TRACK_TYPE_TYRE_SKID,0);
#endif
			}
		}
	} else {
#ifndef PSX
		vp->oldX[0]=vp->oldX[1]=vp->oldX[2]=vp->oldX[3]=0;
		vp->oldZ[0]=vp->oldZ[1]=vp->oldZ[2]=vp->oldZ[3]=0;
#endif
	}

}

#define	WHEEL_GRAV	(5<<8)

/*
	Suspension is a damped spring connecting each wheel to the car

   The extension of the spring goes from Min to Max
   If the car is in mid air then the spring is free to extend to max extension.

  the car at equilibrium, will exert a downward force on each spring of

  F=Ma     F= carMass*Gravity/4     (this assumes the mass is equaly dispersed)

  The spring will exert an upwards force on the car of 

  Force = k/Extension, (the force is inversely proportional to the extension of the spring)

  K is the strength of the spring

  so at equilibrium

  k/extension = carMass*Gravity/4

  extension= (k*4)/(carmass*gravity)

*/

/*

Suggested Readings: 

1 "Fundamental of Vehicle Dynamics," Thomas D. Gillespie, 1992, Society of Automotive Engineers, Inc.

2. "Theory of Ground Vehicles," J. Y. Wong, John Wiley & Sons, Inc, 1993.

3. MOTOR VEHICLE DYNAMICS: MODELING AND SIMULATION by G Genta (Poli. Torino, Italy) 

*/

// ========================================================
//
// VEHICLE COLLISION
//
// ========================================================

extern SLONG there_is_a_los_car(SLONG x1, SLONG y1, SLONG z1,SLONG x2, SLONG y2, SLONG z2);
extern SLONG should_i_collide_against_this_anim_prim(Thing *p_animprim);

VEH_Col VEH_col[VEH_MAX_COL];
SLONG   VEH_col_upto;

// VEH_collide_find_things
//
// Finds all the things that can possibly be collided with and stores
// their details int VEH_col[]

void VEH_collide_find_things(SLONG x, SLONG y, SLONG z, SLONG radius, SLONG ignore, SLONG ignore_prims)
{
	static UWORD	found[VEH_MAX_COL];

	SLONG i;
	SLONG num;
	SLONG prim;
	SLONG dx;
	SLONG dz;
	SLONG dist;

	Thing        *p_found;
	VEH_Col      *vc;
	PrimInfo     *pi;
	AnimPrimBbox *apb;
	OB_Info      *oi;
	
	//
	// Do we include bikes or not? 
	//

	ULONG collide_types;

	#if BIKE
	if (ignore && TO_THING(ignore)->Class == CLASS_BIKE)
	{
		// This is collision for a bike - include bikes.
		collide_types = (1 << CLASS_VEHICLE) | (1 << CLASS_ANIM_PRIM) | (1 << CLASS_BIKE);
	}
	else
	#else
	{
		// This is collision for a van or car - ignore bikes.
		collide_types = (1 << CLASS_VEHICLE) | (1 << CLASS_ANIM_PRIM) | (1 << CLASS_BAT);
	}
	#endif

	//
	// Find everything in our sphere
	//

	num = THING_find_sphere(
			x,y,z,
			radius + 0x200,
			found,
			VEH_MAX_COL,
			collide_types);

	//
	// Scan through
	//

	VEH_col_upto = 0;

	for (i = 0; i < num; i++)
	{
		if (found[i] == ignore)		continue;

		ASSERT(WITHIN(VEH_col_upto, 0, VEH_MAX_COL - 1));

		p_found =  TO_THING(found[i]);

		switch(p_found->Class)
		{
			case CLASS_VEHICLE:

				prim = get_vehicle_body_prim(p_found->Genus.Vehicle->Type);
				pi   = get_prim_info(prim);

				// Simple bounding circle rejection.

				dx = abs((p_found->WorldPos.X >> 8) - x);
				dz = abs((p_found->WorldPos.Z >> 8) - z);

				dist = QDIST2(dx,dz);

				if (dist <= radius + pi->radius + 0x10)
				{
					vc = &VEH_col[VEH_col_upto++];

					vc->type     = VEH_COL_TYPE_BBOX;
					vc->ob_index = NULL;
					vc->veh		 = p_found;
					vc->mid_x    = p_found->WorldPos.X >> 8;
					vc->mid_y    = p_found->WorldPos.Y >> 8;
					vc->mid_z    = p_found->WorldPos.Z >> 8;
					vc->height   = pi->maxy;
					vc->min_x    = pi->minx;
					vc->min_z    = pi->minz;
					vc->max_x    = pi->maxx;
					vc->max_z    = pi->maxz;

					vc->radius_or_yaw = p_found->Genus.Vehicle->Angle;	// (= yaw for a BBOX)
				}

				break;

			case CLASS_ANIM_PRIM:
#ifdef	ANIM_PRIMS
				if (should_i_collide_against_this_anim_prim(p_found))
				{
					apb = &anim_prim_bbox[p_found->Index];
					pi   = get_prim_info(p_found->Index);

					// Simple bounding circle rejection.

					dx = abs((p_found->WorldPos.X >> 8) - x);
					dz = abs((p_found->WorldPos.Z >> 8) - z);

					dist = QDIST2(dx,dz);

					if (dist <= radius + pi->radius + 0x10)
					{
						vc = &VEH_col[VEH_col_upto++];
					
						vc->type     = VEH_COL_TYPE_BBOX;
						vc->ob_index = NULL;
						vc->veh		 = NULL;
						vc->mid_x    = p_found->WorldPos.X >> 8;
						vc->mid_y    = p_found->WorldPos.Y >> 8;
						vc->mid_z    = p_found->WorldPos.Z >> 8;
						vc->height   = apb->maxy;
						vc->min_x    = apb->minx;
						vc->min_z    = apb->minz;
						vc->max_x    = apb->maxx;
						vc->max_z    = apb->maxz;

						vc->radius_or_yaw = p_found->Draw.Tweened->Angle;	// (= yaw for a BBOX)
					}
				}
#endif
				break;

			#if BIKE

			case CLASS_BIKE:

				vc = &VEH_col[VEH_col_upto++];
			
				vc->type     = VEH_COL_TYPE_CYLINDER;
				vc->ob_index = NULL;
				vc->veh		 = NULL;
				vc->mid_x    = p_found->WorldPos.X >> 8;
				vc->mid_y    = p_found->WorldPos.Y >> 8;
				vc->mid_z    = p_found->WorldPos.Z >> 8;
				vc->height   = 0x100;

				vc->radius_or_yaw = 0x40;	// (= radius for a cylinder)

				break;

			#endif

			case CLASS_BAT:

				if (p_found->Genus.Bat->type == BAT_TYPE_BALROG)
				{
					vc = &VEH_col[VEH_col_upto++];
				
					vc->type     = VEH_COL_TYPE_CYLINDER;
					vc->ob_index = NULL;
					vc->veh		 = p_found;
					vc->mid_x    = p_found->WorldPos.X >> 8;
					vc->mid_y    = p_found->WorldPos.Y >> 8;
					vc->mid_z    = p_found->WorldPos.Z >> 8;
					vc->height   = 0x100;

					vc->radius_or_yaw = 0x40;	// (= radius for a cylinder)
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}

	//
	// Now scan OBs
	//

	if (!ignore_prims)
	{
		SLONG mx1 = x - radius >> PAP_SHIFT_LO;
		SLONG mz1 = z - radius >> PAP_SHIFT_LO;
		SLONG mx2 = x + radius >> PAP_SHIFT_LO;
		SLONG mz2 = z + radius >> PAP_SHIFT_LO;

		SATURATE(mx1, 0, PAP_SIZE_LO - 1);
		SATURATE(mz1, 0, PAP_SIZE_LO - 1);
		SATURATE(mx2, 0, PAP_SIZE_LO - 1);
		SATURATE(mz2, 0, PAP_SIZE_LO - 1);

		for (SLONG mx = mx1; mx <= mx2; mx++)
		{
			for (SLONG mz = mz1; mz <= mz2; mz++)
			{
				for (oi = OB_find(mx,mz); oi->prim; oi++)
				{
					if (VEH_col_upto >= VEH_MAX_COL)	return;		// erk - out of room in array!

					if (oi->y >= y + 0x180)				continue;	// it's above us

					switch (prim_get_collision_model(oi->prim))
					{
						case PRIM_COLLIDE_BOX:
						case PRIM_COLLIDE_SMALLBOX:

							pi = get_prim_info(oi->prim);

							// Simple bounding circle rejection.

							dx = abs(oi->x - x);
							dz = abs(oi->z - z);

							dist = QDIST2(dx,dz);

							if (dist <= radius + pi->radius + 0x10)
							{
								vc = &VEH_col[VEH_col_upto++];
							
								vc->type     = VEH_COL_TYPE_BBOX;
								vc->ob_index = oi->index;
								vc->veh		 = NULL;
								vc->mid_x    = oi->x;
								vc->mid_y    = oi->y;
								vc->mid_z    = oi->z;
								vc->height   = pi->maxy;
								vc->min_x    = pi->minx;
								vc->min_z    = pi->minz;
								vc->max_x    = pi->maxx;
								vc->max_z    = pi->maxz;

								vc->radius_or_yaw = oi->yaw;	// (= yaw for BBOXs)
							}

							break;

						case PRIM_COLLIDE_NONE:
							break;

						case PRIM_COLLIDE_CYLINDER:

							pi = get_prim_info(oi->prim);

							// Simple bounding circle rejection.

							dx = abs(oi->x - x);
							dz = abs(oi->z - z);

							dist = QDIST2(dx,dz);

							if (dist <= radius + 0x40)
							{
								vc = &VEH_col[VEH_col_upto++];
							
								vc->type     = VEH_COL_TYPE_CYLINDER;
								vc->ob_index = oi->index;
								vc->veh		 = NULL;
								vc->mid_x    = oi->x;
								vc->mid_y    = oi->y;
								vc->mid_z    = oi->z;
								vc->height   = pi->maxy;

								vc->radius_or_yaw = 0x30;	// (= radius for cylinders)
							}

							break;

						default:
							ASSERT(0);
							break;
					}
				}
			}
		}
	}
}

// VEH_shake_fences
//
// Shakes all the fence facets that lie on the edge of the given mapsquare.

void VEH_shake_fences(SLONG mx, SLONG mz)
{
	SLONG exit;
	SLONG facet;
	SLONG f_list;

	DFacet *df;

	f_list = PAP_2LO(mx >> 2, mz >> 2).ColVectHead;

	if (f_list)
	{
		exit = FALSE;

		while(1)
		{
			ASSERT(WITHIN(f_list, 1, next_facet_link - 1));

			facet = facet_links[f_list];

			if (facet < 0)
			{
				facet = -facet;
				exit  =  TRUE;
			}

			ASSERT(WITHIN(facet, 1, next_dfacet - 1));

			df = &dfacets[facet];

			if (df->FacetType == STOREY_TYPE_FENCE      ||
				df->FacetType == STOREY_TYPE_FENCE_FLAT ||
				df->FacetType == STOREY_TYPE_FENCE_BRICK)
			{
				if (df->x[0] == df->x[1])
				{
					if (df->x[0] == mx || df->x[0] == mx + 1)
					{
						df->Shake = 255;
					}
				}
				else
				{
					ASSERT(df->z[0] == df->z[1]);

					if (df->z[0] == mz || df->z[0] == mz + 1)
					{
						df->Shake = 255;
					}
				}
			}

			if (exit)
			{
				return;
			}

			f_list += 1;
		}
	}
}

// find closest car point to x,y,z

static UBYTE	find_closest_car_point(SLONG x, SLONG y, SLONG z, Thing* car)
{
	Vehicle*	v = car->Genus.Vehicle;

	make_car_matrix(v);

	PrimInfo *inf;
	SLONG	xx[6],yy[6],zz[6];

	inf = get_prim_info(veh_info[car->Genus.Vehicle->Type].BodyPrim);

	xx[0]=inf->minx;
	xx[1]=inf->maxx;
	xx[2]=inf->minx;
	xx[3]=inf->maxx;
	xx[4]=inf->minx;
	xx[5]=inf->maxx;

	yy[0]=inf->miny;
	yy[1]=inf->miny;
	yy[2]=inf->miny;
	yy[3]=inf->miny;
	yy[4]=inf->miny;
	yy[5]=inf->miny;

	zz[0]=inf->minz;
	zz[1]=inf->minz;
	zz[2]=(inf->minz + inf->maxz)/2;
	zz[3]=(inf->minz + inf->maxz)/2;
	zz[4]=inf->maxz;
	zz[5]=inf->maxz;
	
	SLONG	best_manhattan_distance = 0x7FFFFFFF;
	SLONG	nearest = -1;

	for (int ii = 0; ii < 6; ii++)
	{
		apply_car_matrix(&xx[ii],&yy[ii],&zz[ii]);
		SLONG	manhattan = abs(xx[ii] + (car->WorldPos.X >> 8) - x) + abs(zz[ii] + (car->WorldPos.Z >> 8) - z);
		if (manhattan < best_manhattan_distance)
		{
			best_manhattan_distance = manhattan;
			nearest = ii;
		}
	}

	ASSERT(nearest != -1);

	return nearest;
}

// add car-car damage
//
// (only if one of the cars is driven by player)

void VEH_co_damage(Thing* v1, Thing* v2)
{
	if (!is_driven_by_player(v1) && !is_driven_by_player(v2))	return;

	UBYTE	c1 = find_closest_car_point(v2->WorldPos.X >> 8, v2->WorldPos.Y >> 8, v2->WorldPos.Z >> 8, v1);
	UBYTE	c2 = find_closest_car_point(v1->WorldPos.X >> 8, v1->WorldPos.Y >> 8, v1->WorldPos.Z >> 8, v2);
	SLONG	damage;

	MFX_play_thing(THING_NUMBER(v1),SOUND_Range(S_CAR_SMASH_START,S_CAR_SMASH_END),0,v1);
	// give most damage to slower vehicle (faster vehicle gets caned already)
	if (abs(v1->Velocity) > abs(v2->Velocity))
	{
		VEH_add_damage(v1->Genus.Vehicle, c1, (abs(v1->Velocity) + abs(v2->Velocity)) / 1024);
		VEH_add_damage(v2->Genus.Vehicle, c2, abs(v1->Velocity) / 512);
		VEH_bounce(v1->Genus.Vehicle, c1, abs(v1->Velocity));
		VEH_bounce(v2->Genus.Vehicle, c2, abs(v1->Velocity));
	}
	else
	{
		VEH_add_damage(v1->Genus.Vehicle, c1, abs(v2->Velocity) / 512);
		VEH_add_damage(v2->Genus.Vehicle, c2, (abs(v1->Velocity) + abs(v2->Velocity)) / 1024);
		VEH_bounce(v1->Genus.Vehicle, c1, abs(v2->Velocity));
		VEH_bounce(v2->Genus.Vehicle, c2, abs(v2->Velocity));
	}

	// set 2nd vehicle moving
	Vehicle* vv1 = v1->Genus.Vehicle;
	Vehicle* vv2 = v2->Genus.Vehicle;

	vv2->VelX = -vv2->VelX/8 + vv1->VelX/4;
	vv2->VelZ = -vv2->VelZ/8 + vv1->VelZ/4;
	
	SLONG torque = abs(vv1->VelX) + abs(vv1->VelZ);
	torque >>= 10;

	switch (c2)
	{
	case 0:
	case 5:
		vv2->VelR -= torque;
		break;

	case 1:
	case 4:
		vv2->VelR += torque;
		break;
	}

	vv2->Skid = SKID_START;
	// make unstable
	vv2->Stable = 0;
	v2->StateFn = VEH_driving;
}

// GetCarPoints
//
// generate corner points of the car

static inline void GetCarPoints(Thing* p_car, SLONG* x, SLONG* y, SLONG* z, SLONG step)
{
	Vehicle*	veh;
	PrimInfo*	pinfo;
	int			ii;

	veh = p_car->Genus.Vehicle;
	pinfo = get_prim_info(veh_info[veh->Type].BodyPrim);

	// get points in car frame
	x[0] = pinfo->minx;
	x[1] = pinfo->maxx;
	x[2] = pinfo->maxx;
	x[3] = pinfo->minx;

	y[0] = pinfo->miny;
	y[1] = pinfo->miny;
	y[2] = pinfo->miny;
	y[3] = pinfo->miny;

	z[0] = pinfo->minz;
	z[1] = pinfo->minz;
	z[2] = pinfo->maxz;
	z[3] = pinfo->maxz;

	// make car matrix for new position (ignore tilt & roll)
	make_car_matrix_p((veh->Angle + ((veh->VelR * step) >> TICK_SHIFT)) & 2047, 0, 0);

	// transform to world coordinates
	for (ii = 0; ii < 4; ii++)
	{
		apply_car_matrix(&x[ii],&y[ii],&z[ii]);

		x[ii] += (p_car->WorldPos.X + ((veh->VelX * step) >> TICK_SHIFT)) >> 8;
		y[ii] += (p_car->WorldPos.Y + ((veh->VelY * step) >> TICK_SHIFT)) >> 8;
		z[ii] += (p_car->WorldPos.Z + ((veh->VelZ * step) >> TICK_SHIFT)) >> 8;
	}
}

// DoDamage
//
// do damage to a prim after a collision

static void DoDamage(Thing* p_car, VEH_Col* col)
{
	// do car damage
	if (col->veh && col->veh->Class == CLASS_VEHICLE)	// Might be a Balrog!
	{
		// damage the other cars health
		{
			SLONG speed = p_car->Velocity >> 5;

			speed -= 16;

			if (speed > 0)
			{
				col->veh->Genus.Vehicle->Health -= speed>>1; 
			}
		}

		VEH_co_damage(p_car, col->veh);
	}
	// do ob damage
	if (col->ob_index)
	{
		Vehicle*	veh = p_car->Genus.Vehicle;
		SLONG		vel = Root(veh->VelX*veh->VelX + veh->VelZ*veh->VelZ);

		if (vel > 20000) {
			OB_damage(col->ob_index, 
				p_car->WorldPos.X >> 8, p_car->WorldPos.Y >> 8, 
				col->mid_x, col->mid_z, 
				get_vehicle_driver(p_car));
			MFX_play_thing(THING_NUMBER(p_car),SOUND_Range(S_CAR_SMASH_START,S_CAR_SMASH_END),0,p_car);
		} else {
			if (vel > 10000) 
			  MFX_play_thing(THING_NUMBER(p_car),SOUND_Range(S_CAR_SMASH_START,S_CAR_SMASH_END),0,p_car);
			else // find a wimpy minor scrape?
			if (vel>1000) // otherwise you get crunchy noises when 'stuck' against objects
			  MFX_play_thing(THING_NUMBER(p_car),SOUND_Range(S_CAR_SMASH_START,S_CAR_SMASH_START+1),0,p_car);
			else
				if (vel>250)
					MFX_play_thing(THING_NUMBER(p_car),S_CAR_BUMP,0,p_car);

		}
	}
}

// CollideCar
//
// check car for collisions

#define COLL_NONE	0x00

#define	COLL_FL		0x01		// front-left corner
#define COLL_FR		0x02		// front-right corner
#define COLL_BR		0x03		// back-right corner 
#define COLL_BL		0x04		// back-left corner
#define COLL_FRONT	0x05		// front edge
#define COLL_BACK	0x06		// back edge
#define COLL_LEFT	0x07		// left edge
#define COLL_RIGHT	0x08		// right edge
#define COLL_ALL	0x09		// all edges

extern UBYTE last_mav_square_x;
extern UBYTE last_mav_square_z;
extern SBYTE last_mav_dx;
extern SBYTE last_mav_dz;

static void CollideWithKerb(Thing* p_car);
static void process_car(Thing *p_car);

//		1
//   0    1
//	8	    2
//   3    2 
//     4
void	nudge_car(Thing* p_car,SLONG flags,SLONG *x,SLONG *z,SLONG neg)
{
	SLONG	dx=0,dz=0;
	switch(flags&15)
	{
		case	1+2:
			dx=x[3]-x[1];
			dz=z[3]-z[1];
			break;
		case	2+4:
			dx=x[0]-x[2];
			dz=z[0]-z[2];
			break;
		case	4+8:
			dx=x[1]-x[3];
			dz=z[1]-z[3];
			break;
		case	8+1:
			dx=x[2]-x[0];
			dz=z[2]-z[0];
			break;

		case	1:
		case	1+8+2:
			//front
			dx=x[2]-x[1];
			dz=z[2]-z[1];
			break;

		case	2:
		case	2+1+4:
			//rhs
			dx=x[0]-x[1];
			dz=z[0]-z[1];
			break;

		case	4:
		case	4+2+8:
			//back
			dx=x[1]-x[2];
			dz=z[1]-z[2];
			break;

		case	8:
		case	8+4+1:
			//lhs
			dx=x[1]-x[0];
			dz=z[1]-z[0];
			break;
	}

//	dx>>=4;
//	dz>>=4;
#ifndef	NDEBUG
	if(ShiftFlag)
	{
		dx<<=8;
		dz<<=8;
	}
#endif

	if(neg)
	{
		dx=-dx;
		dz=-dz;
	}

	if(dx||dz)
	{
		GameCoord	new_pos;

		new_pos.X = p_car->WorldPos.X + dx;
		new_pos.Y = p_car->WorldPos.Y;
		new_pos.Z = p_car->WorldPos.Z + dz;

		move_thing_on_map(p_car, &new_pos);

	}

}
SLONG	car_hit_flags;
static SLONG CollideCar(Thing* p_car, SLONG step)
{
	Vehicle*	veh = p_car->Genus.Vehicle;
	VehInfo*	vinfo = &veh_info[veh->Type];

	SLONG		x[4],y[4],z[4];
	int			ii;

	car_hit_flags=0;
	
#ifdef _DEBUG
	if (Keys[KB_H] && is_driven_by_player(p_car))	return 0;
#endif

	// hit the kerb?
	CollideWithKerb(p_car);

	// run suspension now and save results
	SLONG	old_y = p_car->WorldPos.Y;
	process_car(p_car);
	veh->VelY = p_car->WorldPos.Y - old_y;
	p_car->WorldPos.Y = old_y;

#if DUMP_COORDS
	GetCarPoints(p_car, x, y, z, 0);
	
	if (is_driven_by_player(p_car))
	{
		TRACE("Before move\n");
		for (ii = 0; ii < 4; ii++)
		{
			TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
		}
	}
#endif

	// generate corner points of the car, pushed out slightly
	GetCarPoints(p_car, x, y, z, step);

#if DUMP_COORDS
	if (is_driven_by_player(p_car))
	{
		TRACE("After move\n");
		for (ii = 0; ii < 4; ii++)
		{
			TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
		}
	}
#endif

	UBYTE	flags = 0,pflags=0;
	static UBYTE flags_to_code[16] =
	{
		COLL_NONE, COLL_FRONT, COLL_NONE,  COLL_FR, 
		COLL_BACK, COLL_LEFT,  COLL_BR,    COLL_RIGHT,
		COLL_NONE, COLL_FL,    COLL_FRONT, COLL_FRONT,
		COLL_BL,   COLL_LEFT,  COLL_BACK,  COLL_ALL
//		COLL_NONE, COLL_FRONT, COLL_RIGHT, COLL_FR, 
//		COLL_BACK, COLL_LEFT,  COLL_BR,    COLL_RIGHT,
//		COLL_LEFT, COLL_FL,    COLL_FRONT, COLL_FRONT,
//		COLL_BL,   COLL_LEFT,  COLL_BACK,  COLL_ALL
	};

	// check each edge against the walls
	if (!VEH_collide_line_ignore_walls)
	{
		for (ii = 0; ii < 4; ii++)
		{
			int	jj = (ii + 1) & 3;
			int	aa;

			if (aa = there_is_a_los_car(x[ii],y[ii],z[ii], x[jj],y[jj],z[jj]))
			{
				// hit a wall or fence
				flags |= 1 << ii;

				if (aa == 1)	flags |= 16;	// X wall
				if (aa == 2)	flags |= 32;	// Z wall

				if (veh->Scrapin<10) veh->Scrapin++;
				
//				if (is_driven_by_player(p_car))
//				AENG_world_line((x[ii] + x[jj]) / 2, (y[ii] + y[jj]) / 2, (z[ii] + z[jj]) / 2, 32, 0xffffff,
//								(x[ii] + x[jj]) / 2, (y[ii] + y[jj]) / 2 + 0xC00, (z[ii] + z[jj]) / 2, 0, 0xffffff,
//								TRUE);

				// shake fence
#ifndef	PSX
				VEH_shake_fences(last_mav_square_x, last_mav_square_z);
#endif
			}
		}
		nudge_car(p_car,flags,x,z,0);


		if (((flags & 15) == 8) || ((flags & 15) == 2) || ((flags & 15) == 10))
		{
//			if (is_driven_by_player(p_car))	TRACE("Facet my flags up [%2.2X]\n", flags);	
			flags |= 1;	// front
		}
	}
	if (veh->Scrapin>5) {
		SLONG	vel = (veh->VelX >> CAR_VEL_SHIFT) * (veh->VelX >> CAR_VEL_SHIFT) + (veh->VelZ >> CAR_VEL_SHIFT) * (veh->VelZ >> CAR_VEL_SHIFT);
		if (vel>300)
			MFX_play_thing(THING_NUMBER(p_car),SOUND_Range(S_CAR_SCRAPE_START,S_CAR_SCRAPE_END),MFX_MOVING,p_car);
		if (veh->Scrapin>0) veh->Scrapin-=2;

		if (flags & 15)
		{
			SLONG	px = 0;
			SLONG	pz = 0;
			SLONG	div = 0;

			for (ii = 0; ii < 4; ii++)
			{
				int jj = (ii + 1) & 3;
				
				if (flags & (1 << ii))
				{
					px += x[ii] + x[jj];
					pz += z[ii] + z[jj];
					div += 2;
				}
			}

			px /= div;
			pz /= div;

#ifndef	PSX
			DIRT_new_sparks(px,y[0],pz,2);
#endif
		
/*			AENG_world_line(px, y[0], pz, 32, 0xFFFFFF,
							(x[0] + x[1] + x[2] + x[3])/4, y[0], (z[0] + z[1] + z[2] + z[3])/4, 0, 0xFFFFFF,
							TRUE);*/
		}
	}

	// check against prims
	for (ii = 0; ii < VEH_col_upto; ii++)
	{
		VEH_Col* vc = &VEH_col[ii];
		int	inside = 1;

		if (vc->type == VEH_COL_TYPE_BBOX)
		{
			int	jj;

			for (jj = 0; jj < 4; jj++)
			{
				int kk = (jj + 1) & 3;

				if (collide_box_with_line(  vc->mid_x, 
											vc->mid_z, 
											vc->min_x,
											vc->min_z,
											vc->max_x,
											vc->max_z,
											vc->radius_or_yaw,
											x[jj], z[jj],
											x[kk], z[kk]))
				{
					pflags |= 1 << jj;

					pflags |= 48;	// just bounce

//					if (is_driven_by_player(p_car))
//					AENG_world_line((x[jj] + x[kk]) / 2, (y[jj] + y[kk]) / 2, (z[jj] + z[kk]) / 2, 32, 0xff0000,
//									(x[jj] + x[kk]) / 2, (y[jj] + y[kk]) / 2 + 0xC00, (z[jj] + z[kk]) / 2, 0, 0xff0000,
//									TRUE);

					DoDamage(p_car, vc);
				}

				SLONG	x1 = x[kk] - x[jj];
				SLONG	z1 = z[kk] - z[jj];
				SLONG	x2 = vc->mid_x - x[jj];
				SLONG	z2 = vc->mid_z - z[jj];

				if (x2*z1 > x1*z2)	inside = 0;
			}
		}
		else
		{
			int	jj;
			
			ASSERT(vc->type == VEH_COL_TYPE_CYLINDER);

			for (jj = 0; jj < 4; jj++)
			{
				int kk = (jj + 1) & 3;

				if (distance_to_line(x[jj], z[jj],
									 x[kk], z[kk],
									 vc->mid_x,
									 vc->mid_z) < vc->radius_or_yaw)
				{
					pflags |= 1 << jj;

					pflags |= 48;

//					if (is_driven_by_player(p_car))
//					AENG_world_line((x[kk] + x[jj]) / 2, (y[kk] + y[jj]) / 2, (z[kk] + z[jj]) / 2, 32, 0xff00,
//									(x[kk] + x[jj]) / 2, (y[kk] + y[jj]) / 2 + 0xC00, (z[kk] + z[jj]) / 2, 0, 0xff00,
//									TRUE);		

					DoDamage(p_car, vc);

					if (vc->veh && vc->veh->Class == CLASS_BAT)
					{
						//
						// This car has it a Balrog! The driver is terrified!
						//

						if (veh->Driver)
						{
							//
							// Make the car driver run away.
							//

							PCOM_make_driver_run_away(TO_THING(veh->Driver), vc->veh);

							//
							// Make the Balrog angry with the driver!
							//

							vc->veh->Genus.Bat->target = veh->Driver;
							vc->veh->Genus.Bat->timer  = 0;
						}
					}
				}

				SLONG	x1 = x[kk] - x[jj];
				SLONG	z1 = z[kk] - z[jj];
				SLONG	x2 = vc->mid_x - x[jj];
				SLONG	z2 = vc->mid_z - z[jj];

				if (x2*z1 > x1*z2)	inside = 0;
			}
		}

		if ((((flags|pflags) & 15) == 8) || (((flags|pflags) & 15) == 2) || (((flags|pflags) & 15) == 10))
		{
//			if (is_driven_by_player(p_car))	TRACE("Fix my flags up [%2.2X]\n", flags);	
			SLONG	dsf = ((x[0] + x[1])/2 - vc->mid_x) * ((x[0] + x[1])/2 - vc->mid_x) + ((z[0] + z[1])/2 - vc->mid_z) * ((z[0] + z[1])/2 - vc->mid_z);
			SLONG	dsb = ((x[2] + x[3])/2 - vc->mid_x) * ((x[2] + x[3])/2 - vc->mid_x) + ((z[2] + z[3])/2 - vc->mid_z) * ((z[2] + z[3])/2 - vc->mid_z);
			if (dsf < dsb)	pflags |= 1;	// front
			else			pflags |= 4;	// back
		}

		// prim is inside car - bounce off
		if (!(flags|pflags) && inside)
		{
//			if (is_driven_by_player(p_car))	TRACE("INSIDE\n");
			pflags = 49;
			DoDamage(p_car, vc);
		}
	}

	if(pflags)
	{
		if(flags)
			nudge_car(p_car,flags,x,z,1);
	}
	flags|=pflags;

	//
	// return if no collision
	//

	if (!flags)	return 0;

	//
	// There has been a collision...
	// Damage the car depending on how fast it is going
	// 

#if !defined(FAST_EDDIE) || !defined(_DEBUG)
	{
		SLONG speed = p_car->Velocity >> 5;

		speed -= 16;

		if (speed > 0)
		{
			veh->Health -= speed>>1;   //miked cars were blowing up too easy so I've halved the damage, Ive increased cars health to 300 so they are harder to shoot too
		}
	}
#endif

	//
	// set flag
	//

	UBYTE	code = flags_to_code[flags & 15];
	SLONG	torque;

	if (!is_driven_by_player(p_car))	veh->Wheel = 0;

	torque = 0;
	if (flags & 16)	torque += abs(veh->VelX);
	if (flags & 32)	torque += abs(veh->VelZ);
	torque >>= 9;

	switch (code)
	{
	case COLL_FRONT:
	case COLL_BACK:
		p_car->Flags |= FLAGS_COLLIDED;
		// Fallthrough!

	case COLL_LEFT:
	case COLL_RIGHT:
		if (flags & 16)	veh->VelX = -veh->VelX/4;
		if (flags & 32)	veh->VelZ = -veh->VelZ/4;
		veh->Skid = SKID_START;
		break;

	case COLL_FL:
	case COLL_BR:
		p_car->Flags |= FLAGS_COLLIDED;
		veh->VelR -= torque;
		if (veh->VelR > -10)	veh->VelR = -10;
		if (flags & 16)	veh->VelX = -veh->VelX/8;
		if (flags & 32)	veh->VelZ = -veh->VelZ/8;
//		veh->Skid = SKID_START;
		break;

	case COLL_FR:
	case COLL_BL:
		p_car->Flags |= FLAGS_COLLIDED;
		veh->VelR += torque;
		if (veh->VelR < +10)	veh->VelR = +10;
		if (flags & 16)	veh->VelX = -veh->VelX/8;
		if (flags & 32)	veh->VelZ = -veh->VelZ/8;
//		veh->Skid = SKID_START;
		break;

	case COLL_ALL:
		veh->VelX = 0;
		veh->VelZ = 0;
		veh->VelR = 0;
		p_car->Velocity = 0;
		veh->Skid = SKID_START;
		break;
	}

	// reduce velocity to prevent bounce
	p_car->Velocity >>= 1;

	// regenerate car points for new position
	GetCarPoints(p_car, x, y, z, step);

#if DUMP_COORDS
	if (is_driven_by_player(p_car))
	{
		TRACE("After bounce\n");
		for (ii = 0; ii < 4; ii++)
		{
			TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
		}
	}
#endif

	// check each edge against the walls
	if (!VEH_collide_line_ignore_walls)
	{
		for (ii = 0; ii < 4; ii++)
		{
			int	jj = (ii + 1) & 3;

			if (there_is_a_los_car(x[ii],y[ii],z[ii], x[jj],y[jj],z[jj]))
			{
				// fucked it all up - don't move anywhere or we'll be in the shite
//				if (is_driven_by_player(p_car))	TRACE("ARSE CUNT MOTHERFUCKER\n", flags);	
				veh->VelX = 0;
				veh->VelY = 0;
				veh->VelZ = 0;
				veh->VelR = 0;
				p_car->Velocity = 0;
				veh->Skid = SKID_START;

#if DUMP_COORDS
				if (is_driven_by_player(p_car))
				{
					GetCarPoints(p_car, x, y, z, step);
				
					TRACE("After halt vs walls\n");
					for (ii = 0; ii < 4; ii++)
					{
						TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
					}

					for (ii = 0; ii < 4; ii++)
					{
						int	jj = (ii + 1) & 3;
						ASSERT(!there_is_a_los_car(x[ii],y[ii],z[ii], x[jj],y[jj],z[jj]));
					}
				}
#endif
				return 1;
			}
		}
	}

	// check against prims
	for (ii = 0; ii < VEH_col_upto; ii++)
	{
		VEH_Col* vc = &VEH_col[ii];

		if (vc->type == VEH_COL_TYPE_BBOX)
		{
			int		jj;
			bool	slide;

			slide = false;

			do
			{
				for (jj = 0; jj < 4; jj++)
				{
					int kk = (jj + 1) & 3;

					if (collide_box_with_line(  vc->mid_x, 
												vc->mid_z, 
												vc->min_x + 0x10,	 
												vc->min_z + 0x10,	
												vc->max_x - 0x10,	 
												vc->max_z - 0x10,	
												vc->radius_or_yaw,
												x[jj], z[jj],
												x[kk], z[kk]))
					{
						// fucked up
						if (!slide)
						{
							// move us a *tiny* bit away from the other object
							veh->VelX = ((p_car->WorldPos.X >> 8) - vc->mid_x) >> 4;
							veh->VelZ = ((p_car->WorldPos.Z >> 8) - vc->mid_z) >> 4;
							veh->VelR = 0;
							slide = true;
							veh->Skid = SKID_START;
							break;
						}
						else
						{
							veh->VelX = 0;
							veh->VelZ = 0;
							veh->VelR = 0;
							p_car->Velocity = 0;
#if DUMP_COORDS
							if (is_driven_by_player(p_car))
							{
								GetCarPoints(p_car, x, y, z, step);
							
								TRACE("After halt vs square prim\n");
								for (ii = 0; ii < 4; ii++)
								{
									TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
								}

								for (jj = 0; jj < 4; jj++)
								{
									int	kk = (jj + 1) & 3;

									ASSERT(!collide_box_with_line(vc->mid_x, 
												vc->mid_z, 
												vc->min_x + 0x10,	 
												vc->min_z + 0x10,	
												vc->max_x - 0x10,	 
												vc->max_z - 0x10,	
												vc->radius_or_yaw,
												x[jj], z[jj],
												x[kk], z[kk]));
								}
							}
#endif
							return 1;
						}
					}
				}
				if (jj == 4)	break;

				if (slide)
				{
					GetCarPoints(p_car, x, y, z, step);

#if DUMP_COORDS
					if (is_driven_by_player(p_car))
					{
						TRACE("After slide (square prim)\n");
						for (ii = 0; ii < 4; ii++)
						{
							TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
						}
					}
#endif
				}
			} while (slide);
		}
		else
		{
			int		jj;
			bool	slide;
			
			ASSERT(vc->type == VEH_COL_TYPE_CYLINDER);

			slide = false;

			do 
			{
				for (jj = 0; jj < 4; jj++)
				{
					int kk = (jj + 1) & 3;

					if (distance_to_line(x[jj], z[jj],
										 x[kk], z[kk],
										 vc->mid_x,
										 vc->mid_z) < vc->radius_or_yaw - 0x10)
					{
						// fucked up
						if (!slide)
						{
							// try sliding car forwards
							SLONG	vel = Root((veh->VelX >> 4) * (veh->VelX >> 4) + (veh->VelZ >> 4) * (veh->VelZ >> 4));
							veh->VelR = 0;
							veh->VelX = (vel * SIN(veh->Angle & 2047)) >> 12;
							veh->VelZ = (vel * COS(veh->Angle & 2047)) >> 12;
							slide = true;
							break;
						}
						else
						{
							veh->VelX = 0;
							veh->VelZ = 0;
							veh->VelR = 0;
							p_car->Velocity = 0;
#if DUMP_COORDS
							if (is_driven_by_player(p_car))
							{
								GetCarPoints(p_car, x, y, z, step);
							
								TRACE("After halt vs round prim\n");
								for (ii = 0; ii < 4; ii++)
								{
									TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
								}

								for (jj = 0; jj < 4; jj++)
								{
									int	kk = (jj + 1) & 3;

									ASSERT(distance_to_line(x[jj],z[jj],x[kk],z[kk],vc->mid_x,vc->mid_z) >= vc->radius_or_yaw - 0x10);
								}
							}
#endif
							return 1;
						}
					}
				}
				if (jj == 4)	break;

				if (slide)
				{
					GetCarPoints(p_car, x, y, z, step);

#if DUMP_COORDS
					if (is_driven_by_player(p_car))
					{
						TRACE("After slide (round prim)\n");
						for (ii = 0; ii < 4; ii++)
						{
							TRACE("[%d] = %8.8X %8.8X %8.8X\n", ii, x[ii], y[ii], z[ii]);
						}
					}
#endif
				}
			} while (slide);
		}
	}

	return 1;
}

// CollideWithKerb
//
// nudge the car away from the kerb

static void CollideWithKerb(Thing* p_car)
{
	Vehicle*	veh = p_car->Genus.Vehicle;
	VehInfo*	vinfo = &veh_info[veh->Type];

	// make car matrix for new position (ignore tilt & roll)
	make_car_matrix_p((veh->Angle + ((veh->VelR * TICK_RATIO) >> TICK_SHIFT)) & 2047, 0, 0);

	// generate flags
	UBYTE	on_road = 0;

	for (SLONG wheel = 0; wheel < 4; wheel++)
	{
		SLONG wx = vinfo->DX[wheel];
		SLONG wy = 0;
		SLONG wz = vinfo->DZ[wheel];
		
		apply_car_matrix(&wx, &wy, &wz);

		SLONG	papx = wx + (p_car->WorldPos.X >> 8);
		SLONG	papz = wz + (p_car->WorldPos.Z >> 8);

		if (ROAD_is_road(papx >> 8, papz >> 8))		on_road |= (1 << wheel);
	}

	UBYTE	change = (on_road ^ veh->OnRoadFlags);

	if (change && !veh->DControl)
	{
#define	KERB_TURN	16

		// lower your shields and prepare to be boarded.  your angle will be assimilated and merged with ours.  resistance is futile.
		static SLONG	towards_table[8] = { 0x000, 0x200, 0x200, 0x400, 0x400, 0x600, 0x600, 0x800 };	// takes top 3 bits to nearest axis
		SLONG	towards = towards_table[veh->Angle >> 8];

		if ((towards - veh->Angle) < -(KERB_TURN * TICK_RATIO) >> TICK_SHIFT)
		{
			veh->VelR -= KERB_TURN;
		}
		else if ((towards - veh->Angle) > (KERB_TURN * TICK_RATIO) >> TICK_SHIFT)
		{
			veh->VelR += KERB_TURN;
		}
	}
	veh->OnRoadFlags = on_road;
}

// GetRunoverHP
//
// Get dot product of vehicle velocity with vector from vehicle centre
// to person - this gives an indication of the amount of HP the person
// must lose

SLONG GetRunoverHP(Thing* p_car, Thing* p_person)
{
	SLONG	tx = (p_person->WorldPos.X - p_car->WorldPos.X) >> 8;
	SLONG	tz = (p_person->WorldPos.Z - p_car->WorldPos.Z) >> 8;

	SLONG	tt = Root(tx*tx + tz*tz) * 200;	// 200 units per HP

	SLONG	hp = abs(p_car->Genus.Vehicle->VelX * tx + p_car->Genus.Vehicle->VelZ * tz) / tt;

	if (hp < 10)	hp = 10;

	return hp;
}


//
// Throws a person out of the car and knocks him over.
//

void VEH_throw_out_person(Thing *p_person, Thing *p_vehicle)
{
	set_person_exit_vehicle(p_person);

	knock_person_down(
		p_person,
		30,
		p_vehicle->WorldPos.X >> 8,
		p_vehicle->WorldPos.Z >> 8,
		NULL);
}



//===============================================================
//
// Car state function
//
//===============================================================

// VEH_driving
//
// vehicle state handler

static void do_car_input(Thing *p_thing);

void VEH_driving(Thing *p_thing)
{
	WaveParams	car;
	DrawMesh	*dm		= p_thing->Draw.Mesh;
	Vehicle		*veh	= p_thing->Genus.Vehicle;
	SLONG		dx,dy,dz;
	SLONG		coltype;
	SLONG		dwheel;

	dy = 0;

	//
	// Make sure that this is a furniture thing and everything is valid.
	//

	ASSERT(p_thing->Class == CLASS_VEHICLE);

	//
	// Forget you've been shot at every once in a while...
	//

	if ((GAME_TURN & 0x1f) == 0)
	{
		veh->Flags &= ~FLAG_VEH_SHOT_AT;
	}

	if (veh->Health <= 0)
	{
		veh->Steering = 0;
		veh->DControl = 0;

		if (p_thing->State != STATE_DEAD)	// Magic number... we've already blown up!
		{
			//
			// Blow up!
			//

#ifdef PSX
			POW_create(
				POW_CREATE_LARGE_SEMI,
				p_thing->WorldPos.X,
				p_thing->WorldPos.Y,
				p_thing->WorldPos.Z,0,0,0);
#else
			{
				Thing *pyro;
				SLONG wave;
				pyro=PYRO_create(p_thing->WorldPos,PYRO_FIREBOMB);
				if (pyro)
					pyro->Genus.Pyro->Flags|=PYRO_FLAGS_WAVE;
				wave=S_EXPLODE_MEDIUM;
				if (!(Random()&3)) wave++; // 25% chance of a bigger bang than usual
				MFX_play_xyz(THING_NUMBER(p_thing),wave,0,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
			}
#endif
			VEH_bounce(veh, 0, 4000);

			veh->damage[0] = 4;
			veh->damage[1] = 4;
			veh->damage[2] = 4;
			veh->damage[3] = 4;
			veh->damage[4] = 4;
			veh->damage[5] = 4;

			p_thing->State                = STATE_DEAD;
			p_thing->Genus.Person->Action = ACTION_DEAD;
			veh->Health                   = 0;

			//
			// Anyone driving the car gets thrown out of the car and knocked over.
			//

			if (veh->Driver)
			{	
				VEH_throw_out_person(TO_THING(veh->Driver), p_thing);
			}

			while(veh->Passenger)
			{
				//
				// This function removes the person from the passenger list.
				//

				VEH_throw_out_person(TO_THING(veh->Passenger), p_thing);
			}
		}
		else
		{
			//
			// Use health as a countdown timer...
			//

			if (!(p_thing->Flags & FLAGS_IN_VIEW))
			{
				veh->Health -= 1;

				if (veh->Health < -200)
				{
					//
					// Get rid of the vehicle.
					//

					remove_thing_from_map(p_thing);

					p_thing->StateFn = NULL;

					return;
				}
			}
		}
	}

	if (veh->Flags & FLAG_VEH_STALLED)
	{
		//
		// Can't control this car no more...
		//

		veh->Steering = 0;

		if (p_thing->Velocity > 0)
		{
			veh->DControl = VEH_DECEL;
		}
		else
		{
			veh->DControl = 0;
		}
	}

	//
	// Move it.
	//

	if (!(veh->Flags & FLAG_FURN_DRIVING) && 
		!p_thing->Genus.Vehicle->VelX && 
		!p_thing->Genus.Vehicle->VelZ && 
		(veh->Stable >= STABLE_COUNT))
	{
		if (p_thing->State != STATE_DEAD)
		{
			//
			// Dead cars need to execute the above code so they can dissapear
			// from the map after a while.
			// 

			siren(veh, 0);
			p_thing->StateFn = NULL;
		}

		return;
	}

	do_car_input(p_thing);

	//
	// If this car is on the road, there is no need to collide with the walls.
	// 

	VEH_collide_line_ignore_walls = VEH_on_road(p_thing, 0) && VEH_on_road(p_thing, TICK_RATIO);

	//
	// And maybe not prims to...
	//

	SLONG ignore_prims;

	if (GAME_FLAGS & GF_CARS_WITH_ROAD_PRIMS)
	{
		ignore_prims = FALSE;
	}
	else
	{
		ignore_prims = VEH_collide_line_ignore_walls;
	}

	{
		Thing	*p_driver;
		p_driver=TO_THING(p_thing->Genus.Vehicle->Driver);
		if(p_driver->Class==CLASS_PERSON)
		{
			if(p_driver->Genus.Person->PlayerID)
			{
				ignore_prims=0;
				VEH_collide_line_ignore_walls=0;
			}
		}
	}

	//
	// Mark the vehicle as not having collided this frame.
	//

	p_thing->Flags &= ~FLAGS_COLLIDED;

	//
	// Find all the things this vehicle can collide with.
	//

	SLONG	vel = Root((veh->VelX >> 4) * (veh->VelX >> 4) + (veh->VelZ >> 4) * (veh->VelZ >> 4)) >> 4;

	VEH_collide_find_things(
		p_thing->WorldPos.X >> 8,
		p_thing->WorldPos.Y >> 8,
		p_thing->WorldPos.Z >> 8,
		0x180 + vel,
		THING_NUMBER(p_thing),
		ignore_prims);

	//
	// Collide with them
	//

	coltype = CollideCar(p_thing, TICK_RATIO);



	//
	// set new position / angle
	//

	GameCoord	new_pos;

	new_pos.X = p_thing->WorldPos.X + ((veh->VelX * TICK_RATIO) >> TICK_SHIFT);
	new_pos.Y = p_thing->WorldPos.Y + veh->VelY;
	new_pos.Z = p_thing->WorldPos.Z + ((veh->VelZ * TICK_RATIO) >> TICK_SHIFT);

	move_thing_on_map(p_thing, &new_pos);
	veh->Angle += (veh->VelR * TICK_RATIO) >> TICK_SHIFT;
	veh->Angle &= 2047;

	//
	// Are there any people inside our bounding box? If there is
	// then we should run them over.
	//

	if ((veh->VelX || veh->VelZ) && veh->Driver)	// make sure we don't run ourselves over when we get out
	{
		SLONG i;

		#define MAX_RUNOVER 8

		UWORD people[MAX_RUNOVER];
		SLONG num;

		SLONG box_valid = FALSE;
		SLONG miny;
		SLONG maxy;
		SLONG prim;
		SLONG useangle;
		SLONG sin_yaw;
		SLONG cos_yaw;
		SLONG matrix[4];

		SLONG tx;
		SLONG tz;

		SLONG rx;
		SLONG rz;

		PrimInfo	*pi;
		Thing		*p_found;

		num = THING_find_sphere(
					p_thing->WorldPos.X >> 8,
					p_thing->WorldPos.Y >> 8,
					p_thing->WorldPos.Z >> 8,
					0x200,
					people,
					MAX_RUNOVER,
					1 << CLASS_PERSON);

		if (num)
		{
			prim  = get_vehicle_body_prim(p_thing->Genus.Vehicle->Type);
			pi    = get_prim_info(prim);
			miny = (p_thing->WorldPos.Y - get_vehicle_body_offset(p_thing->Genus.Vehicle->Type) >> 8) + pi->miny - 0x80;
			maxy = (p_thing->WorldPos.Y - get_vehicle_body_offset(p_thing->Genus.Vehicle->Type) >> 8) + pi->maxy - 0x80;
		}

		for (i = 0; i < num; i++)
		{
			p_found = TO_THING(people[i]);

			if (p_found->Genus.Person->Flags & FLAG_PERSON_DRIVING)
			{
				// You can't run yourself over!
			}
/*
			else if (p_found->State == STATE_DEAD || p_found->State == STATE_DYING)
			{
				// Ignore dead people.
			}
*/
			else if (p_found->OnFace)
			{
				// Ignore people on faces... they might be dangling on this car!
			}
			else if (!WITHIN((p_found->WorldPos.Y >> 8), miny, maxy))
			{
//				TRACE("foundy = %d (%d-%d)\n", p_found->WorldPos.Y, miny, maxy);
			}
			else
			{
				//
				// This person is a candidate for being run over.
				//

				if (!box_valid)
				{
					useangle  = -p_thing->Genus.Vehicle->Angle;
					useangle &=  2047;

					sin_yaw = SIN(useangle);
					cos_yaw = COS(useangle);

					matrix[0] =  cos_yaw;
					matrix[1] =  sin_yaw;
					matrix[2] = -sin_yaw;
					matrix[3] =  cos_yaw;

					box_valid = TRUE;
				}

				//
				// Rotate this person into the space of the prim.
				//
				
				tx = p_found->WorldPos.X - p_thing->WorldPos.X >> 8;
				tz = p_found->WorldPos.Z - p_thing->WorldPos.Z >> 8;

				rx = MUL64(tx, matrix[0]) + MUL64(tz, matrix[1]);
				rz = MUL64(tx, matrix[2]) + MUL64(tz, matrix[3]);

				if (WITHIN(rx, pi->minx - 0x18, pi->maxx + 0x18) &&
					WITHIN(rz, pi->minz - 0x18, pi->maxz + 0x18))
				{
					Thing*	p_driver = get_vehicle_driver(p_thing);

					//
					// Run this person over.
					//

					if (p_found->State == STATE_DEAD || p_found->State == STATE_DYING)
					{
						//
						// ran over a corpse or someone lay down
						//
						if(is_person_ko_and_lay_down(p_found))
						{
							SLONG anim;

							//
							// Is this person on their front or back?
							//

							switch(person_is_lying_on_what(p_found))
							{
								case PERSON_ON_HIS_FRONT:
									anim  =  ANIM_FIGHT_STOMPED_BACK;
									break;

								case PERSON_ON_HIS_BACK:
									anim  =  ANIM_FIGHT_STOMPED_FRONT;
									break;

								default:
									ASSERT(0);
									break;
							}

							if(p_found->Draw.Tweened->CurrentAnim!=anim || (p_found->Draw.Tweened->CurrentFrame->Flags&ANIM_FLAG_LAST_FRAME))
								set_person_ko_recoil(p_found,anim,0);

						}


					}
					else
					{
						// if(version_censored)
						if((!VIOLENCE)&&(p_found->Genus.Person->PersonType==PERSON_COP||p_found->Genus.Person->PersonType==PERSON_CIV))
						{
							if(p_found->SubState!=SUB_STATE_FLIPING)
							{
								set_person_flip(p_found, Random() & 0x1);
							}


						}
						else
						{
							knock_person_down(p_found,
								GetRunoverHP(p_thing, p_found),
								p_thing->WorldPos.X >> 8,
								p_thing->WorldPos.Z >> 8,
								p_driver);

							//
							// A nice sound!
							//

							MFX_play_thing(THING_NUMBER(p_found),S_THUMP_SQUISH,MFX_REPLACE,p_found);
						}

					}
				}
			}
		}
	}

	//
	// Rustle up some leaves.
	//

	DIRT_gust(
		p_thing,
		p_thing->WorldPos.X>>8,
		p_thing->WorldPos.Z>>8,
		new_pos.X>>8,
		new_pos.Z>>8);

	//
	// Swirl the mist.
	//
#ifndef PSX
	MIST_gust(
		p_thing->WorldPos.X>>8,
		p_thing->WorldPos.Z>>8,
		new_pos.X>>8,
		new_pos.Z>>8);
#endif
	//
	// Knock down barrels.
	//

	{
		SLONG prim = get_vehicle_body_prim(p_thing->Genus.Vehicle->Type);

		BARREL_hit_with_prim(
			prim,
			p_thing->WorldPos.X                                                         >> 8,
			p_thing->WorldPos.Y - get_vehicle_body_offset(p_thing->Genus.Vehicle->Type) >> 8,
			p_thing->WorldPos.Z                                                         >> 8,
			p_thing->Genus.Vehicle->Angle);
	}		

	//	Do the engine noise.

	car.Priority				=	0;
	car.Flags					=	WAVE_CARTESIAN|WAVE_LOOP|WAVE_SET_LOOP_POINTS;
	car.LoopStart				=	282624/2;
	car.LoopEnd					=	408574/2;
	car.Mode.Cartesian.Scale	=	(128<<8);
	car.Mode.Cartesian.X		=	p_thing->WorldPos.X;
	car.Mode.Cartesian.Y		=	p_thing->WorldPos.Y;
	car.Mode.Cartesian.Z		=	p_thing->WorldPos.Z;

	if (p_thing->Velocity > 500)
	{
		dx = -SIN(p_thing->Genus.Vehicle->Angle) << 1;
		dz = -COS(p_thing->Genus.Vehicle->Angle) << 1; 

		//
		// Scare nearby people.
		//

		PCOM_oscillate_tympanum(
			PCOM_SOUND_VAN,
			NULL,
			p_thing->WorldPos.X + dx >> 8,
			p_thing->WorldPos.Y      >> 8,
			p_thing->WorldPos.Z + dz >> 8);
	}
}

//===============================================================
//
// Handle input from car
//
//===============================================================

// init_arctans
//
// init arctan table (for steering)

static SLONG arctan_table[2*WHEELTIME + 1];
static SLONG arctan_table_ok = 0;

static void init_arctans(void)
{
	int	ii;

	if (arctan_table_ok)	return;

	for (ii = 0; ii <= WHEELTIME; ii++)
	{
		arctan_table[WHEELTIME - ii] = -Arctan(ii, -WHEELRATIO) & 2047;
		arctan_table[WHEELTIME + ii] = +Arctan(ii, -WHEELRATIO) & 2047;
	}

	for (ii = 0; ii <= 2*WHEELTIME + 1; ii++)
	{
		if (arctan_table[ii] > 1024)	arctan_table[ii] -= 2048;
	}

	arctan_table_ok = 1;
}

// steering_wheel
//
// run the steering wheel

//
// changed fro analogue input by MikeD  dx has a scalar value, really the wheel should equal dx in analogue mode
//

extern SLONG	analogue;

void steering_wheel(Vehicle* veh, SLONG velocity, bool player)
{
	SLONG	inc = TICK_RATIO;

	if (!(veh->Flags & FLAG_FURN_DRIVING) || (veh->Flags & FLAG_VEH_IN_AIR))
	{
		// bring wheel in
		if (veh->Wheel > inc)		
			veh->Wheel -= inc;
		else 
			if (veh->Wheel < -inc)	
				veh->Wheel += inc;
		else						
			veh->Wheel = 0;
	}
	else
	{
		if (player)
		{
			velocity = abs(velocity);

			if (velocity > 1000)	
				velocity = 1000;

			if (veh->IsAnalog)
			{
#if 0
				inc = (abs(veh->Steering)*inc * (256 - velocity / 8)) >> (6+7);
#else
				// The steering says where the steering wheel is directly.
				// It is damped by the input routine.
				//veh->Wheel = veh->Steering << 5;

				// Actually, I'd love to do that, but it becomes unmanageable at high speed,
				// and doesn't give a decent turning circle at low speed.
				// So inversely to speed it is...
				veh->Wheel = ( veh->Steering * ( 700 - ( velocity >> 1 ) ) ) >> ( 3 );

				if (veh->Wheel > (WHEELTIME << TICK_SHIFT))
				{
					veh->Wheel = WHEELTIME << TICK_SHIFT;
				}
				else if (veh->Wheel < -(WHEELTIME << TICK_SHIFT))
				{
					veh->Wheel = -(WHEELTIME << TICK_SHIFT);
				}
				goto steering_done;
#endif
			}
			else
			{
				inc = (inc * (256 - velocity / 8)) >> 7;
			}
		}

		{
			// scale inc according to velocity

			if (veh->Steering > 0)
			{
				// steer right
				if (veh->Wheel < 0)		veh->Wheel = 0;
				else					veh->Wheel += inc;

				if (veh->Wheel > (WHEELTIME << TICK_SHIFT))		veh->Wheel = WHEELTIME << TICK_SHIFT;
			}
			else if (veh->Steering < 0)
			{
				// steer left
				if (veh->Wheel > 0)		veh->Wheel = 0;
				else					veh->Wheel -= inc;

				if (veh->Wheel < -(WHEELTIME << TICK_SHIFT))	veh->Wheel = -(WHEELTIME << TICK_SHIFT);
			}
			else
			{
				// let go of wheel
				if (veh->Wheel > 0)			veh->Wheel >>= 1;
				else if (veh->Wheel < 0)	veh->Wheel = (veh->Wheel + 1) >> 1;
			}
		}
steering_done:;
	}

	// lookup in table
	veh->WheelAngle = arctan_table[(veh->Wheel >> TICK_SHIFT) + WHEELTIME];
}

// siren
//
// play/stop the siren

static void siren(Vehicle* veh, UBYTE play)
{
	if (veh->Siren == play)	return;

	if ((veh->Type==VEH_TYPE_POLICE)||
		(veh->Type==VEH_TYPE_AMBULANCE)||
		(veh->Type==VEH_TYPE_MEATWAGON)) 
	{
		if (play)
			MFX_play_ambient(VEHICLE_NUMBER(veh),S_CAR_SIREN1,MFX_LOOPED);
		else
			MFX_stop(VEHICLE_NUMBER(veh),S_CAR_SIREN1);
	}

	veh->Siren = play;
}

// pedals
//
// run the pedals

static inline void pedals(Vehicle* veh, VehInfo* vinfo, SLONG velocity, UBYTE& friction, UWORD& move_cancel, SWORD& accel,Thing *p_thing)
{

	if (veh->DControl & VEH_ACCEL)	veh->Skid = 0;

	// set GrabAction flag for preventing Darci getting out of the car
	if (veh->DControl & (VEH_ACCEL | VEH_DECEL))
	{
		veh->GrabAction = 1;
	}
	else if (!(veh->DControl & VEH_FASTER))
	{
		veh->GrabAction = 0;
	}

	if (!(veh->Flags & FLAG_FURN_DRIVING))
	{
		// no-one in the car - slow down with a big friction (engine braking + braking)
		siren(veh,0);
		veh->Dir = 0;
		friction -= 4;
	}
	else if (veh->DControl & VEH_ACCEL)
	{

		if (veh->Dir < 0)
		{
			// applying brakes while in reverse
			veh->Dir = -1;
			friction -= 4;
			accel = vinfo->SoftBrake;
			move_cancel = INPUT_CAR_ACCELERATE;
		}
		else
		{
			// accelerating forwards (terminal velocity is an emergent property
			// of constant acceleration and velocity-based friction)
			veh->Dir = +2;
			accel = vinfo->FwdAccel;
			if (veh->DControl & VEH_FASTER)
			{
				if (velocity < VEH_SPEED_LIMIT)			accel <<= 1;
				else if (velocity < VEH_SPEED_LIMIT*2)	accel += (accel >> 1);
			}
			else
			{
				if (velocity >= VEH_SPEED_LIMIT)	accel = 0;
			}

#ifdef FAST_EDDIE
			if (Keys[KB_T])	accel <<= 1;	// !$$! we're fucking Batman!
#endif

			if ((velocity < -200) || ((veh->DControl & VEH_FASTER) && (velocity < 400)))
			{
				// do wheelspin smoke
				veh->Smokin = 1;
			}
		}
	}
	else if (veh->DControl & VEH_DECEL)
	{
		if ((veh->Dir > 0) || (velocity > 200))	// 2nd check so you don't bust the engine going into reverse at 50mph
		{
			// applying brakes while going forwards
			veh->Dir = +1;
			friction -= 4;
			accel = -vinfo->SoftBrake;
			move_cancel = INPUT_CAR_DECELERATE;

			if (!veh->Skid)
			{
				if ((veh->DControl & VEH_FASTER) && (velocity > 1600))	veh->Skid = 1;
			}
			else
			{
				veh->Skid++;
				if (veh->Skid == SKID_START)
				{
					// skid in a 60s style
					if (!veh->VelR)
					{
						// Justin wants these lines removed
						// I don't, so what I've done is make sure they're included
						// in the final builds but not in the release build
						// Aren't I cunning? ;)
//#ifdef FINAL
						if (Random() & 128)		veh->VelR =  velocity >> 6;
						else					veh->VelR = -velocity >> 6;
//#endif
					}
					else
					{
						if (veh->VelR > 0)	veh->VelR =  velocity >> 5;
						else				veh->VelR = -velocity >> 5;
					}
				}
			}
		}
		else
		{
			// accelerating backwards
			veh->Dir = -2;
			accel = -vinfo->BkAccel;
		}
	}
	else
	{
		// slow down with a little friction (engine braking)
		friction -= 1;

		// reset state
		if ((veh->Dir == -1) || (veh->Dir == +1))	veh->Dir = 0;
	}

	// operate siren
	if (veh->DControl & VEH_SIREN)
	{
		siren(veh, !veh->Siren);
		move_cancel = INPUT_CAR_SIREN;
	}
}

// do_car_input
//
// handle inputs from the player

static void do_car_input(Thing *p_thing)
{
	Vehicle*	veh = p_thing->Genus.Vehicle;
	VehInfo*	vinfo = &veh_info[p_thing->Genus.Vehicle->Type];

#ifdef _DEBUG
	if (is_driven_by_player(p_thing))
	{
		if (Keys[KB_H] && ControlFlag)
		{
			Keys[KB_H] = 0;
			ASSERT(0);
		}

		if (Keys[KB_Y])	veh->Skid = SKID_START;
	}
#endif

	if (!(veh->Flags & FLAG_FURN_DRIVING) && !veh->VelX && !veh->VelZ)
	{
		siren(veh, 0);
		veh->VelR = 0;
		return;
	}

	//
	// move steering wheel
	//

	steering_wheel(veh, p_thing->Velocity, is_driven_by_player(p_thing));

	//
	// modify engine velocity
	//

	if (veh->Flags & FLAG_VEH_IN_AIR)
	{
		// do nothing
	}
	else if (veh->Skid < SKID_START)
	{
		UBYTE	friction;		// amount of friction (lower number = more friction)
		UWORD	move_cancel;	// move to cancel if velocity becomes 0
		SWORD	accel;			// acceleration

		//
		// get input from controls
		//


		pedals(veh, vinfo, p_thing->Velocity, friction = 7, move_cancel = 0, accel = 0,p_thing);

		if (move_cancel == INPUT_CAR_SIREN && p_thing->Genus.Vehicle->Driver)
		{
			// always cancel this key immediately
			Thing	*p_driver;
			p_driver=TO_THING(p_thing->Genus.Vehicle->Driver);
			if (p_driver->Genus.Person->PlayerID)
			{
				Thing	*p_player;
				p_player = NET_PLAYER(p_driver->Genus.Person->PlayerID-1);
				p_player->Genus.Player->InputDone = move_cancel;
			}
		}

		//
		// apply acceleration & friction
		//

		SLONG	oldvel = p_thing->Velocity;
		SLONG	oldmag = abs(oldvel);

		p_thing->Velocity = ((SLONG(p_thing->Velocity) << friction) - SLONG(p_thing->Velocity)) >> friction;
		p_thing->Velocity += accel;

		SLONG	newvel = p_thing->Velocity;
		SLONG	newmag = abs(newvel);

		SLONG	realacc = newvel - oldvel;

		//
		// apply forces to suspension from acceleration
		//

		if (realacc > 0)
		{
			if (abs(veh->DY[0]) < (realacc << 4))	veh->DY[0] -= realacc << 2;
			if (abs(veh->DY[1]) < (realacc << 4))	veh->DY[1] -= realacc << 2;
		}
		else
		{
			if (abs(veh->DY[0]) < (-realacc << 2))	veh->DY[0] += -realacc;
			if (abs(veh->DY[1]) < (-realacc << 2))	veh->DY[1] += -realacc;
		}

		//
		// detect if we've stopped
		//

		bool	stopped = false;

		if ((veh->Dir == +2) || (veh->Dir == -2))
		{
			if (!accel && (newmag < 10))	stopped = true;		// coast to a halt
		}
		else if ((veh->Dir == +1) || (veh->Dir == -1))
		{
			if ((newmag < 10) || (newmag > oldmag))	stopped = true;	// brake to a halt
		}
		else
		{
			ASSERT(accel == 0);
			if (newmag < 10)				stopped = true;		// coast to a halt
		}

		//
		// deal with stopping
		//

		if (stopped)
		{
			p_thing->Velocity = 0;
			veh->Dir = 0;

			if (move_cancel && p_thing->Genus.Vehicle->Driver)
			{
				// cancel keypress
				Thing	*p_driver;
				p_driver=TO_THING(p_thing->Genus.Vehicle->Driver);
				if (p_driver->Genus.Person->PlayerID)
				{
					Thing	*p_player;
					p_player = NET_PLAYER(p_driver->Genus.Person->PlayerID-1);
					p_player->Genus.Player->InputDone = move_cancel;
				}
			}
			if (!(veh->Flags & FLAG_FURN_DRIVING))
			{
				// kill velocity, reset wheel
				p_thing->Velocity	= 0;
				veh->Wheel			= 0;
				veh->WheelAngle	= 0;
			}

			veh->VelX = 0;
			veh->VelZ = 0;
			veh->VelR = 0;

			return;
		}
/*
		if (is_driven_by_player(p_thing))	
		{
			TRACE("Velocity = %d\n", p_thing->Velocity);
		}
*/
	}
	else
	{
		// skidding - slow car down
		veh->VelX = ((SLONG(veh->VelX) << 4) - SLONG(veh->VelX)) >> 4;
		veh->VelZ = ((SLONG(veh->VelZ) << 4) - SLONG(veh->VelZ)) >> 4;
		if (veh->VelR > 0)		veh->VelR = ((SLONG(veh->VelR) << 4) - SLONG(veh->VelR)) >> 4;
		else					veh->VelR = veh->VelR - (veh->VelR >> 4);

		if (veh->Steering < 0)
		{
			if (veh->VelR < 32)		veh->VelR += (abs(veh->VelX) + abs(veh->VelZ)) >> 12;
		}
		else if (veh->Steering > 0)
		{
			if (veh->VelR > -32)	veh->VelR -= (abs(veh->VelX) + abs(veh->VelZ)) >> 12;
		}

		// stop car if too slow
		if (abs(veh->VelX) < 2048)	veh->VelX = 0;
		if (abs(veh->VelZ) < 2048)	veh->VelZ = 0;

		if (!veh->VelX && !veh->VelZ)	veh->VelR = (veh->VelR + 1) >> 1;

		// slow engine down rapidly
		p_thing->Velocity = (p_thing->Velocity + 1) >> 1;

		if (abs(p_thing->Velocity) < 10)	p_thing->Velocity = 0;

		if (abs(veh->VelR) < 16)
		{
			// see how the skid angle compares to the car's direction
			SLONG	dx = veh->VelX >> (8 - CAR_VEL_SHIFT);
			SLONG	dz = veh->VelZ >> (8 - CAR_VEL_SHIFT);

			SLONG	mvx = dx * dx + dz * dz;
			SLONG	dp = (dx * -SIN(veh->Angle) + dz * COS(veh->Angle)) >> 16;

#if 0
			if (is_driven_by_player(p_thing))
			{
				TRACE("dx = %d, dz = %d\n", dx, dz);
				TRACE("SIN = %d, COS = %d\n", -SIN(veh->Angle), COS(veh->Angle));
				TRACE("MVX = %d, DP = %d, DP*DP = %d\n", mvx, dp, dp*dp);

				float cossq = float(dp*dp) / float(mvx);

				char	str[32];
				sprintf(str, "*cos2 = %f", cossq);
				char	*sp;
				if ((dp > 0) && (dp * dp > (mvx - (mvx >> 2))))
				{
					sp = str;
				}
				else
				{
					sp = str + 1;
				}
				CONSOLE_text(str,1000);
			}
#endif

			if ((dp > 0) && (dp * dp > (mvx - (mvx >> 2))))	// cos^2 angle > 15/16 => cos angle > 3/4
			{
				p_thing->Velocity = Root(mvx);
//				if (is_driven_by_player(p_thing))	TRACE("Set Velocity = %d\n", p_thing->Velocity);

				// come out of skid
//				if (p_thing->Velocity < 800)	veh->Skid = 0;
				veh->Skid = 0;
			}
		}		

		if (!veh->VelX && !veh->VelZ && !p_thing->Velocity)
		{
			veh->Skid = 0;
		}

		// still connect brake lights to brake pedal
		veh->Dir = (veh->DControl & VEH_DECEL) ? +1 : 0;

		// smokin!
		veh->Smokin = 1;

#ifdef PSX
		if (is_driven_by_player(p_thing))
		{
			SLONG shock=p_thing->Velocity>>1;
			SATURATE(shock,64,192);
			PSX_SetShock((shock>128)?1:0,shock);
		}
#endif
#ifdef TARGET_DC
		if (is_driven_by_player(p_thing))
		{
			Vibrate ( 30.0f, (float)( p_thing->Velocity ) * 0.01f, 2.0f );
		}
#endif

	}

	//
	// steer the car
	//

	SLONG	dangle;			// delta angle, nothing rude
	SLONG	dx,dz;			// delta position

	if (veh->WheelAngle && !(veh->Flags & FLAG_VEH_IN_AIR))
	{
		SLONG	tcx,tcz,tcr;	// turning circle x,y, radius
		SLONG	dx1,dz1;		// delta position (in car frame)

		//
		// find the car's turning circle
		//

		if (veh->WheelAngle < 0)
		{
			// turning circle centre is to left of car
			SLONG	angle = -veh->WheelAngle;
			SLONG	wheelbase = vinfo->DZ[0] - vinfo->DZ[2];

			// radius = wheelbase / sin(angle)
			tcr = DIV64(wheelbase, SIN(angle));

			// x offset = radius * cos(angle)
			tcx = vinfo->DX[0] - (tcr * COS(angle) >> 16);
			tcz = vinfo->DZ[0];
		}
		else
		{
			// turning circle centre is to right of car
			SLONG	angle = veh->WheelAngle;
			SLONG	wheelbase = vinfo->DZ[1] - vinfo->DZ[3];

			// radius = wheelbase / sin(angle)
			tcr = DIV64(wheelbase, SIN(angle));

			// x offset = radius * cos(angle)
			tcx = vinfo->DX[1] + (tcr * COS(angle) >> 16);
			tcz = vinfo->DZ[1];
		}

		//
		// get angle turned
		//

		// get turn angle in radians
		SLONG	radangle = DIV64(p_thing->Velocity<<(8 - CAR_VEL_SHIFT), tcr << 8);

		// convert to fraction of 1.0 (divide by 2*PI = multiply by 1/2*PI = multiply by 10430)
		SLONG	angle = MUL64(radangle, 10430);

		// convert to angle 0 to 2047
		angle >>= 5;

//		angle = (angle * TICK_RATIO) >> TICK_SHIFT;
		if (!angle)	angle = 1;

		//
		// apply forces to suspension
		//

		if (veh->WheelAngle > 0)
		{
			if (abs(veh->DY[1]) < (angle << 2))	veh->DY[1] += angle << 1;
			if (abs(veh->DY[3]) < (angle << 2))	veh->DY[3] += angle << 1;
		
			dangle = -angle;
		}
		else
		{
			if (abs(veh->DY[0]) < (angle << 2))	veh->DY[0] += angle << 1;
			if (abs(veh->DY[2]) < (angle << 2))	veh->DY[2] += angle << 1;

			dangle = +angle;
		}

		dangle &= 2047;

		//
		// Get the vector of the vehicle's movement, in the vehicle's own frame
		//

		if (angle > 2)
		{
			if (veh->WheelAngle < 0)
			{
				SLONG	c = COS(dangle) - 65536;
				SLONG	s = SIN(dangle);

				dx1 = (-tcx * c - -tcz * s) >> (16 - CAR_VEL_SHIFT);
				dz1 = -(-tcx * s + -tcz * c) >> (16 - CAR_VEL_SHIFT);
			}
			else
			{
				SLONG	c = COS(dangle) - 65536;
				SLONG	s = -SIN(dangle);

				dx1 = -(tcx * c - -tcz * s) >> (16 - CAR_VEL_SHIFT);
				dz1 = -(tcx * s + -tcz * c) >> (16 - CAR_VEL_SHIFT);
			}
		}
		else
		{
			// use simpler approximation to avoid the asymptote
			dx1 = 0;
//			dz1 = -SLONG(p_thing->Velocity * TICK_RATIO) >> TICK_SHIFT;
			dz1 = -SLONG(p_thing->Velocity);
		}

		//
		// convert to world frame
		//

		dx = (SIN(veh->Angle) * dz1 - COS(veh->Angle) * dx1) >> (8 + CAR_VEL_SHIFT);
		dz = (COS(veh->Angle) * dz1 + SIN(veh->Angle) * dx1) >> (8 + CAR_VEL_SHIFT);
		dangle = (dangle & 1024) ? dangle - 2048 : dangle;
	}
	else
	{
		//
		// not steering
		//

		SLONG dx1 = 0;
//		SLONG dz1 = -SLONG(p_thing->Velocity * TICK_RATIO) >> TICK_SHIFT;
		SLONG dz1 = -SLONG(p_thing->Velocity);

		dx = (SIN(veh->Angle) * dz1 - COS(veh->Angle) * dx1) >> (8 + CAR_VEL_SHIFT);
		dz = (COS(veh->Angle) * dz1 + SIN(veh->Angle) * dx1) >> (8 + CAR_VEL_SHIFT);
		dangle = 0;
	}

	if ((veh->VelX || veh->VelZ) && (veh->Skid < SKID_START))
	{
		SLONG	ax,az;		// acceleration
		SLONG	vx,vz,vv;	// velocity
		SLONG	av;			// |v x a|

		// get acceleration
		ax = dx - veh->VelX;
		az = dz - veh->VelZ;

		// get normalized velocity
		vx = veh->VelX;
		vz = veh->VelZ;
		vv = vx*vx + vz*vz;
		if (vv)
		{
			// get speed
			vv = Root(vv);

			// get acceleration component
			av = (ax * vz) - (az * vx);

			// if abs((av * 256 / vv) >> 8) > SKID_FORCE
			if (abs(av) > SKID_FORCE * vv)		
				veh->Skid = SKID_START;
			else
				if (abs(av) > ((NEAR_SKID_FORCE*vinfo->FwdAccel)>>5) * vv)
					MFX_play_thing(THING_NUMBER(p_thing), SOUND_Range(S_SKID_SLOW_START,S_SKID_SLOW_END),MFX_MOVING,p_thing);
		}
	}

	if (veh->Skid < SKID_START)
	{
		veh->VelX = dx;
		veh->VelZ = dz;
		veh->VelR = dangle;
	}

	// spin the wheels
	veh->Spin  = (veh->Spin + (p_thing->Velocity >> 2)) & 2047;

#define VEH_FWD_ACCEL (1)
#define VEH_FWD_DECEL (2)
#define VEH_REV_ACCEL (3)
#define VEH_REV_DECEL (4)

	if (is_driven_by_player(p_thing))
	{
		UBYTE state=0;

		if (veh->Dir>=0)
		{
			if ((veh->DControl & VEH_ACCEL) && !veh->Skid)
				state=VEH_FWD_ACCEL;
			else
				state=VEH_FWD_DECEL;
		} else {
			if ((veh->DControl & VEH_DECEL) && !veh->Skid)
				state=VEH_REV_ACCEL;
			else
				state=VEH_REV_DECEL;
		}

		if (state!=veh->LastSoundState)
		{
			MFX_stop(THING_NUMBER(p_thing),S_CARX_START);
			MFX_stop(THING_NUMBER(p_thing),S_CARX_CRUISE);
			MFX_stop(THING_NUMBER(p_thing),S_CARX_DECEL);
			MFX_stop(THING_NUMBER(p_thing),S_CARX_IDLE);
			MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_START);
			MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_LOOP);
			MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_END);
			switch(state)
			{
			case VEH_FWD_ACCEL:
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_START,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_CRUISE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
				break;
			case VEH_FWD_DECEL:
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_DECEL,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_IDLE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
				break;
			case VEH_REV_ACCEL:
#ifdef PSX
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_START,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_CRUISE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
#else
				MFX_play_ambient(THING_NUMBER(p_thing),S_CAR_REVERSE_START,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CAR_REVERSE_LOOP,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
#endif
				break;
			case VEH_REV_DECEL:
#ifdef PSX
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_DECEL,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_IDLE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
#else
				MFX_play_ambient(THING_NUMBER(p_thing),S_CAR_REVERSE_END,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_IDLE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
#endif
				break;
			}
		}

		veh->LastSoundState=state;
		

/*		UBYTE state=(veh->DControl==VEH_ACCEL)|((veh->DControl==VEH_DECEL)<<1)|((veh->Dir==-2)<<3);
		
		if ((veh->DControl == VEH_ACCEL) && !veh->Skid)
		{
			if (!(veh->Flags & FLAG_VEH_FX_STATE))
			{
				MFX_stop(THING_NUMBER(p_thing),S_CARX_DECEL);
				MFX_stop(THING_NUMBER(p_thing),S_CARX_IDLE);
				MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_START);
				MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_LOOP);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_START,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_CRUISE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);

				veh->Flags|=FLAG_VEH_FX_STATE;
			}
		}
		else
		{
			if ((veh->DControl == VEH_DECEL))
			{
				if ((veh->Dir==-2)&&!(veh->Flags & FLAG_VEH_FX_STATE))
				{
					MFX_stop(THING_NUMBER(p_thing),S_CARX_DECEL);
					MFX_stop(THING_NUMBER(p_thing),S_CARX_IDLE);
					MFX_play_ambient(THING_NUMBER(p_thing),S_CAR_REVERSE_START,MFX_MOVING|MFX_EARLY_OUT);
					MFX_play_ambient(THING_NUMBER(p_thing),S_CAR_REVERSE_LOOP,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);
					veh->Flags|=FLAG_VEH_FX_STATE;
				}
					
			} else

			if (veh->Flags & FLAG_VEH_FX_STATE)
			{
				MFX_stop(THING_NUMBER(p_thing),S_CARX_START);
				MFX_stop(THING_NUMBER(p_thing),S_CARX_CRUISE);
				MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_START);
				MFX_stop(THING_NUMBER(p_thing),S_CAR_REVERSE_LOOP);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_DECEL,MFX_MOVING|MFX_EARLY_OUT);
				MFX_play_ambient(THING_NUMBER(p_thing),S_CARX_IDLE,MFX_MOVING|MFX_QUEUED|MFX_SHORT_QUEUE|MFX_LOOPED);

				veh->Flags^=FLAG_VEH_FX_STATE;
			}
		} */
	}

//	TRACE("dx: %d  dz: %d  dir: %d  \n",veh->dx,veh->dz, veh->Dir);
}

//===============================================================
//
// Run suspension physics
//
//===============================================================

// apply_thrust_to_suspension
//
// work out effect on car of the suspension parameters given

inline static SLONG apply_thrust_to_suspension(Suspension *p_sus, SLONG velocity, SLONG penetrate_dist)
{
	SLONG	acc;
	SLONG	compression;

	// damp the velocity
	velocity = ((velocity << 4) - velocity) >> 4;

	// subtract velocity from compression, but if we're penetrating, reduce velocity to
	// handle the collision
	compression = p_sus->Compression - (velocity - penetrate_dist);

	// clamp
	if (compression < MIN_COMPRESSION)		compression = MIN_COMPRESSION;
	else if (compression > MAX_COMPRESSION)	compression = MAX_COMPRESSION;

	// get restoring acceleration (mass = 256)
	// NOTE: this is *totally* non-physical, but it works (linear is *very* unstable)
	acc = (compression >> 5) * (compression >> 5) >> 9;

	// add acceleration due to spring & gravity
	velocity += GRAVITY + acc;

	// if really compressed, we're going up a step, so make sure the
	// wheel stays above the surface
	if (compression == MAX_COMPRESSION)		velocity += penetrate_dist;

	p_sus->Compression = compression;

	return velocity;
}

// expand_suspension
//
// allow suspension to expand (when in air)

inline static void expand_suspension(Suspension *p_sus, SLONG size)
{
	ASSERT(size >= 0);

	size -= size >> 2;	// size = size * 200 / 256

	if (p_sus->Compression - size < MIN_COMPRESSION)		p_sus->Compression = MIN_COMPRESSION;
	else													p_sus->Compression -= size;
}

// process_car
//
// given where it is, work out the wheel's positions, the suspension
// action and the car's orientation

static void do_car_fall_and_tilt(Thing* p_car, SLONG *wx, SLONG *wy, SLONG *wz, SLONG *dy);

static void process_car(Thing *p_car)
{
	SLONG		count;
	SLONG		wheel;
	SLONG		c0;
	SLONG		wx[4],wy[4],wz[4];
	SLONG		dy[4];
	VehInfo*	info;
	Vehicle*	vp;
	BOOL		squeaky=0;
	BOOL		crunchy=0;
/*
	{
		SLONG	door;
extern SLONG in_right_place_for_car(Thing *p_person, Thing *p_vehicle, SLONG *door);
		in_right_place_for_car(NET_PERSON(0)
			,p_car, &door);
	}
*/

	vp = p_car->Genus.Vehicle;
	info = &veh_info[vp->Type];

	if (vp->Flags & FLAG_VEH_ANIMATING)
	{
//		animate_car(p_car);
	}

	make_car_matrix(vp);

	UBYTE	on_road = 0;

//	if((p_car->Flags & FLAGS_IN_VIEW) && !ShiftFlag)
	//if(!ShiftFlag)
	{
		int	in_air = 0;

		for (wheel = 0; wheel < 4; wheel++)
		{
			SLONG	y;
			SLONG	height;

			y = p_car->WorldPos.Y;

			dy[wheel]=0;

			ASSERT(p_car->Genus.Vehicle->Spring[wheel].Compression >= MIN_COMPRESSION);

			// Calculate wheel's position relative to origin of car
			// given that the car has an angle,tilt and roll
			//
			wx[wheel] = info->DX[wheel];
			wy[wheel] = 0;
			wz[wheel] = info->DZ[wheel];
			
			apply_car_matrix(&wx[wheel],&wy[wheel],&wz[wheel]);

			SLONG	papx = wx[wheel] + (p_car->WorldPos.X >> 8);
			SLONG	papz = wz[wheel] + (p_car->WorldPos.Z >> 8);

			height = PAP_calc_map_height_at(papx, papz) << 8;

			if (ROAD_is_road(papx >> 8, papz >> 8))		on_road |= (1 << wheel);
#ifndef		FINAL
#ifndef TARGET_DC
			if (Keys[KB_Q] && is_driven_by_player(p_car))
			{
				if (wheel < 2)	height += 0x4000;
				else			height += 0x8000;
			}
#endif
#endif

			//
			// iterate the suspension algorithm
			//

			SLONG	y_pos;

			count = TICK_LOOP;
			while (--count)
			{
				SLONG	size;

				// saturate the DY on the wheels to stop the car going mad!
				if (vp->DY[wheel] > 1536)	vp->DY[wheel] = 1536;

				// get size of spring
				size = (128 << 8) - vp->Spring[wheel].Compression;

				// get position of wheel
				y_pos =  y + (wy[wheel] << 8) - size;

				if (y_pos <= height)
				{
					// wheel on the floor - push car upwards
					vp->DY[wheel] = apply_thrust_to_suspension(&vp->Spring[wheel], vp->DY[wheel], height - y_pos);
	//				TRACE("s-floor: %d\n",vp->DY[wheel]);
					squeaky+=abs(vp->DY[wheel]);			
					crunchy+=vp->DY[wheel];
				}
				else
				{
					// in air
					if (vp->Spring[wheel].Compression > MIN_COMPRESSION)
					{
						// expand suspension
						expand_suspension(&vp->Spring[wheel], y_pos - height);

						if (vp->Spring[wheel].Compression > MIN_COMPRESSION)
						{
							// if still compressed then do an upthrust
							vp->DY[wheel] = apply_thrust_to_suspension(&vp->Spring[wheel],  vp->DY[wheel], 0);
	//						TRACE("s-up: %d\n",vp->DY[wheel]);
	//						squeaky=1;
						}
					}
					// falling
					vp->DY[wheel] += GRAVITY;
				}

				y += vp->DY[wheel];
				dy[wheel] += vp->DY[wheel];
			}

			// get size of spring
			SLONG	size = (128 << 8) - vp->Spring[wheel].Compression;

			// get final position of wheel
			y_pos =  y + (wy[wheel] << 8) - size;

			if (y_pos - height > 1024)	in_air++;
		}
#ifndef PSX
		if (squeaky>1600) MFX_play_thing(THING_NUMBER(p_car),SOUND_Range(S_CAR_SUSPENSION_START,S_CAR_SUSPENSION_END),MFX_MOVING,p_car);
#endif
		if (crunchy<-4000) MFX_play_thing(THING_NUMBER(p_car),S_CAR_SMASH_START+1,MFX_MOVING,p_car);
	//	TRACE("crunchy: %d\n",crunchy);

		do_car_fall_and_tilt(p_car, &wx[0], &wy[0], &wz[0], &dy[0]);

		if (in_air == 4)	vp->Flags |= FLAG_VEH_IN_AIR;
		else				vp->Flags &= ~FLAG_VEH_IN_AIR;
	}
	#if 0
	else
	{
		//
		// Back in from sourcesafe version 118
		//
		//
		// car's that aren't drawn have ultra cheap suspension
		//
		SLONG	height;
		for(c0=0;c0<4;c0++)
		{
			p_car->Genus.Vehicle->DY[c0]=0;
			p_car->Genus.Vehicle->Spring[c0].Compression=4300; //16%/84% compression
		}
		p_car->Genus.Vehicle->Tilt=0;
		p_car->Genus.Vehicle->Roll=0;
		height=PAP_calc_map_height_at((p_car->WorldPos.X>>8),(p_car->WorldPos.Z>>8));
		//
		// 107 is length of suspension 128 *(100-%compressed)   =128*.84
		//
		p_car->WorldPos.Y=(height+107)<<8;


	}
	#endif

	if (vp->dlight)
	{
		SLONG dx = -car_matrix[6] << 1;
		SLONG dy =  0x2000;
		SLONG dz = -car_matrix[8] << 1;

		SLONG lx;
		SLONG ly;
		SLONG lz;

		lx = p_car->WorldPos.X + dx >> 8;
		ly = p_car->WorldPos.Y + dy >> 8;
		lz = p_car->WorldPos.Z + dz >> 8;

		NIGHT_dlight_move(p_car->Genus.Vehicle->dlight, lx, ly,	lz);
	}
}

//===============================================================
//
// Calculate car orientation
//
//===============================================================

static void calc_tilt_and_roll(SLONG *tilt, SLONG *roll, SLONG *whx, SLONG *why, SLONG *whz, SLONG angle);

static void do_car_fall_and_tilt(Thing* car, SLONG *wx, SLONG *wy, SLONG *wz, SLONG *dy)
{
	SLONG	min_dy,max_dy;
	SLONG	c0,pos_count,neg_count;
	SLONG	remove;
	SLONG	tilt,roll;

	// find min,max dy and number of positive vectors
	min_dy = 0x7FFFFFFF;
	max_dy = 0x80000000;
	pos_count = 0;
	neg_count = 0;

	for (c0 = 0; c0 < 4; c0++)
	{
		if (dy[c0] > 0)			pos_count++;
		if (dy[c0] < 0)			neg_count++;
		if (dy[c0] > max_dy) 	max_dy = dy[c0];
		if (dy[c0] < min_dy)	min_dy = dy[c0];
	}

	if (pos_count == 4)			// all positive, whole car is rising
	{
		car->WorldPos.Y += max_dy;
		remove = max_dy;
	}
	else if (pos_count == 0)	// all negative, whole car is falling
	{
		car->WorldPos.Y += min_dy;
		remove = min_dy;
	}
	else
	{
		remove = 0;
	}

	// remove aggregate vector
	dy[0] -= remove;
	dy[1] -= remove;
	dy[2] -= remove;
	dy[3] -= remove;

	// add dy into wy
	wy[0] += dy[0] >> 8;
	wy[1] += dy[1] >> 8;
	wy[2] += dy[2] >> 8;
	wy[3] += dy[3] >> 8;

	// calculate tilt and roll for car
	calc_tilt_and_roll(&tilt, &roll, wx, wy, wz, car->Genus.Vehicle->Angle);

	// check stability
	if ((abs(car->Genus.Vehicle->Tilt - tilt) < 16) && (abs(car->Genus.Vehicle->Roll - roll) < 16) && !remove)
	{
		if (car->Genus.Vehicle->Stable != STABLE_COUNT)		car->Genus.Vehicle->Stable++;
	}
	else
	{
		car->Genus.Vehicle->Stable = 0;
	}

	car->Genus.Vehicle->Tilt = tilt;
	car->Genus.Vehicle->Roll = roll;
}

// fast_root
//
// perform a proper fast squareroot

#ifndef PSX
static inline SLONG fast_root(SLONG num)
{
#if 0
	SLONG	sh;
	SLONG	ans;
	SLONG	ans_sq;

	ASSERT(num >= 0);

	// do an approximate BSR-style thingy
	if (num & 0xFF000000)		sh = 15;
	else if (num & 0x00FF0000)	sh = 11;
	else if (num & 0x0000FF00)	sh = 7;
	else						sh = 3;

	// calculate using a bit iteration (much faster than
	// using a bloody DIVIDE in Newton-Raphson!)
	ans = 0;
	ans_sq = 0;

	do
	{
		// work out (ans + bit)^2 = (ans*ans + 2*ans*bit + bit*bit) where bit = (1 << sh)
		SLONG	newans = ans_sq + (ans << (sh + 1)) + (1 << (sh + sh));

		if (newans <= num)
		{
			ans_sq = newans;
			ans |= (1 << sh);
		}
	} while (sh--);

	return ans;
#else
	// OK, I've done it now ... but to be honest, I reckon this is the
	// fastest way on current Intel chips ...
	return (SLONG)sqrt((double)num);
#endif
}
#endif

// normalise_val256
//
// normalize a vector to magnitude 256

static inline void normalise_val256(SLONG *vx, SLONG *vy, SLONG *vz)
{
	SLONG	len;

	len = *vx * *vx + *vy * *vy + *vz * *vz;

#ifndef PSX
	len = fast_root(len);
#else
	len = Root(len);
#endif

	if (len)	len = 65536 / len;
	else		len = 65536;

	*vx = (*vx * len) >> 8;
	*vy = (*vy * len) >> 8;
	*vz = (*vz * len) >> 8;
}

// calc_tilt_n_roll_with_matrix
//
// calculate tilt & roll from 3 vectors

static inline void calc_tilt_n_roll_with_matrix(SLONG across_x,SLONG across_y,SLONG across_z,SLONG nose_x,SLONG nose_y,SLONG nose_z,SLONG nx,SLONG ny,SLONG nz,SLONG *angle,SLONG *tilt,SLONG *roll)
{
	SLONG	matrix[9];

	//
	// assumes nx,ny,nz is normalised 256 & is the normal out of the top of the car
	//

	matrix[0]=(across_x)<<8;
	matrix[1]=(across_y)<<8;
	matrix[2]=(across_z)<<8;

	matrix[3]=(nx)<<8;
	matrix[4]=(ny)<<8;
	matrix[5]=(nz)<<8;

	matrix[6]=(nose_x)<<8;
	matrix[7]=(nose_y)<<8;
	matrix[8]=(nose_z)<<8;

	FMATRIX_find_angles(matrix,angle,tilt,roll);
}

// calc_tilt_and_roll
//
// calculate tilt and roll for the car

static void calc_tilt_and_roll(SLONG *tilt, SLONG *roll, SLONG *whx, SLONG *why, SLONG *whz, SLONG angle)
{
	SLONG	nx,ny,nz;

	SLONG	vx,vy,vz;
	SLONG	wx,wy,wz;
	SLONG	wheel;

	SLONG	x02,y02,z02;
	SLONG	x10,y10,z10;
	SLONG	x31,y31,z31;
	SLONG	x23,y23,z23;

	SLONG	tt = 0;
	SLONG	tr = 0;

	// precalculate the normal vectors
	x02 = whx[0] - whx[2];
	y02 = -(why[0] - why[2]);
	z02 = whz[0] - whz[2];

	normalise_val256(&x02, &y02, &z02);

	x10 = whx[1] - whx[0];
	y10 = -(why[1] - why[0]);
	z10 = whz[1] - whz[0];

	normalise_val256(&x10, &y10, &z10);

	x31 = whx[3] - whx[1];
	y31 = -(why[3] - why[1]);
	z31 = whz[3] - whz[1];

	normalise_val256(&x31, &y31, &z31);

	x23 = whx[2] - whx[3];
	y23 = -(why[2] - why[3]);
	z23 = whz[2] - whz[3];

	normalise_val256(&x23, &y23, &z23);

	for (wheel = 0; wheel < 4; wheel++)
	{
		switch (wheel)
		{
		case 0:
			vx = x02; vy = y02; vz = z02;
			wx = x10; wy = y10; wz = z10;

			nx = (vy * wz - vz * wy) >> 8;
			ny = (vz * wx - vx * wz) >> 8;
			nz = (vx * wy - vy * wx) >> 8;

			calc_tilt_n_roll_with_matrix( wx, wy, wz, vx, vy, vz,nx,ny,nz,&angle,tilt,roll);
			break;

		case 1:
			vx = x10; vy = y10; vz = z10;
			wx = x31; wy = y31; wz = z31;

			nx = (vy * wz - vz * wy) >> 8;
			ny = (vz * wx - vx * wz) >> 8;
			nz = (vx * wy - vy * wx) >> 8;

			calc_tilt_n_roll_with_matrix( vx, vy, vz,-wx,-wy,-wz,nx,ny,nz,&angle,tilt,roll);
			break;

		case 2:
			vx = x23; vy = y23; vz = z23;
			wx = x02; wy = y02; wz = z02;

			nx = (vy * wz - vz * wy) >> 8;
			ny = (vz * wx - vx * wz) >> 8;
			nz = (vx * wy - vy * wx) >> 8;

			calc_tilt_n_roll_with_matrix(-vx,-vy,-vz, wx, wy, wz,nx,ny,nz,&angle,tilt,roll);
			break;

		case 3:
			vx = x31; vy = y31; vz = z31;
			wx = x23; wy = y23; wz = z23;

			nx = (vy * wz - vz * wy) >> 8;
			ny = (vz * wx - vx * wz) >> 8;
			nz = (vx * wy - vy * wx) >> 8;

			calc_tilt_n_roll_with_matrix(-wx,-wy,-wz,-vx,-vy,-vz,nx,ny,nz,&angle,tilt,roll);
			break;
		}

		if (*roll > 1024)		*roll = *roll - 2048;
		if (*tilt > 1024)		*tilt = *tilt - 2048;

		tt += *tilt;
		tr += *roll;
	}

	// take average
	tt /= 4;
	tr /= 4;

	// max out
	if (tt < -312)		tt = -312;
	else if (tt > 312)	tt = 312;

	if (tr < -312)		tr = -312;
	if (tr > 312)		tr = 312;

	*tilt = -tt & 2047;
	*roll = -tr & 2047;
}


void VEH_reduce_health(
		Thing *p_car,
		Thing *p_person,
		SLONG  damage)
{
	ASSERT(p_car->Class == CLASS_VEHICLE);

	p_car->Genus.Vehicle->Health -= damage >> 1;

	if (p_car->Genus.Vehicle->Health <= 0)
	{
		if (p_car->Genus.Vehicle->Driver)
		{
			//
			// The person has blown up a car with someone inside it.  So
			// they have killed that person!
			//

			if (p_person)
			{
				p_person->Genus.Person->Flags2 |= FLAG2_PERSON_IS_MURDERER;
			}
		}

		if (p_car->Genus.Vehicle->Type == VEH_TYPE_POLICE ||
			p_car->Genus.Vehicle->Type == VEH_TYPE_MEATWAGON)
		{
			//
			// This is a police car that has blown up.
			//

			if (p_person && p_person->Class == CLASS_PERSON && p_person->Genus.Person->PlayerID)
			{
				//
				// Give the player a red mark!
				//

				NET_PLAYER(0)->Genus.Player->RedMarks += 1;
			}
		}
	}

	//
	// Make sure the car processes this health info...
	//

	p_car->StateFn = VEH_driving;
}


Thing   *vehicle_wheel_pos_vehicle;
VehInfo *vehicle_wheel_pos_info;


void vehicle_wheel_pos_init(Thing *p_vehicle)
{
	vehicle_wheel_pos_vehicle =  p_vehicle;
	vehicle_wheel_pos_info    = &veh_info[p_vehicle->Genus.Vehicle->Type];;

	make_car_matrix(p_vehicle->Genus.Vehicle);
}

void vehicle_wheel_pos_get(
		SLONG  which,
		SLONG *wx,
		SLONG *wy,
		SLONG *wz)
{
	SLONG wheel_x;
	SLONG wheel_y;
	SLONG wheel_z;

	wheel_x = vehicle_wheel_pos_info->DX[which];
	wheel_y = 51 - (((128 << 8) - vehicle_wheel_pos_vehicle->Genus.Vehicle->Spring[which].Compression) >> 8);
	wheel_z = vehicle_wheel_pos_info->DZ[which];

	apply_car_matrix(
	   &wheel_x,
	   &wheel_y,
	   &wheel_z);

   *wx = wheel_x + (vehicle_wheel_pos_vehicle->WorldPos.X >> 8);
   *wy = wheel_y + (vehicle_wheel_pos_vehicle->WorldPos.Y >> 8);
   *wz = wheel_z + (vehicle_wheel_pos_vehicle->WorldPos.Z >> 8);
}
