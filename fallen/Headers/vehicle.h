// vehicle.h
//
// vehicle physics

#ifndef	VEHICLE_H
#define	VEHICLE_H

#define	FLAG_VEH_DRIVING	 (1<<0)
#define	FLAG_VEH_WHEEL1_GRIP (1<<1)
#define	FLAG_VEH_WHEEL2_GRIP (1<<2)
#define	FLAG_VEH_WHEEL3_GRIP (1<<3)
#define	FLAG_VEH_WHEEL4_GRIP (1<<4)
#define	FLAG_VEH_ANIMATING	 (1<<5)
#define FLAG_VEH_FX_STATE	 (1<<6)
#define FLAG_VEH_SHOT_AT	 (1<<7)
#define FLAG_VEH_STALLED	 (1<<8)	// Not allowed to be driven any more.
#define FLAG_VEH_IN_AIR		 (1<<9)

// Vehicle structure

typedef	struct	
{
	UWORD	Compression;
	UWORD	Length;
} Suspension;

// DButtons bits

#define	VEH_ACCEL		1
#define VEH_DECEL		2
#define VEH_FASTER		4
#define VEH_SIREN		8

// speed limits

#define	VEH_SPEED_LIMIT		750		// the speed limit ("35 mph")
#define VEH_REVERSE_SPEED	300		// the speed limit for reversing

typedef struct 
{
	DrawTween	Draw;

	Suspension	Spring[4];		// spring data
	SLONG		DY[4];			// DY for each car point

	SLONG		Angle;
	SLONG		Tilt;
	SLONG		Roll;

	SBYTE		Steering;		// -64 to +64 or -1 to +1
	UBYTE		IsAnalog;		// specifies range of steering
	UBYTE		DControl;		// digital control input
	UBYTE		GrabAction;		// set if we've grabbed the action key

	SWORD		Wheel;			// steering wheel position
	UWORD		Allocated;		// 0 if not allocated

	UWORD		Flags;
	UWORD		Driver;

	UWORD		Passenger;		// A linked list of passengers in this vehicle.
	UWORD		Type;

	UBYTE		still;			// How long has the car been at zero velocity for?
	UBYTE		dlight;			// dynamic light thingy
	UBYTE		key;			// key to unlock, or SPECIAL_NONE
	UBYTE		Brakelight;		// brakelight state

	UBYTE		damage[6];		// damage counts
	SWORD		Spin;

	SLONG		VelX;			// X velocity
	SLONG		VelY;			// Y velocity
	SLONG		VelZ;			// Z velocity
	SLONG		VelR;			// rotational velocity

	SWORD		WheelAngle;		// wheel angle
	UBYTE		Siren;			// siren on/off?
	UBYTE		LastSoundState;	// for comparison with the current one

	SBYTE		Dir;			// +2 = driving forwards, +1 = driving forwards but braking, -1,-2 same for backwards, 0 = stopped
	UBYTE		Skid;			// skidding? 0 = no, 1 - SKID_START-1 = maybe; >=SKID_START = yes
	UBYTE		Stable;			// stable?
	UBYTE		Smokin;			// smoking?

	UBYTE		Scrapin;		// scraping metal against a wall?
	UBYTE		OnRoadFlags;	// flag for each wheel - is it on the road?
	SWORD		Health;			// How close the car is to blowing up... (Starts at 200 like people)

//#ifndef PSX
	SLONG	oldX[4],oldZ[4];
//#endif


}Vehicle;

typedef	Vehicle* VehiclePtr;

#define RMAX_VEHICLES	40
#define MAX_VEHICLES	(save_table[SAVE_TABLE_VEHICLE].Maximum)
#define	VEH_NULL		65000

// state

void VEH_driving(Thing *);
extern StateFunction VEH_statefunctions[];

void init_vehicles(void);

#define VEH_TYPE_VAN		0
#define VEH_TYPE_CAR		1
#define VEH_TYPE_TAXI		2
#define VEH_TYPE_POLICE		3
#define VEH_TYPE_AMBULANCE	4
#define VEH_TYPE_JEEP		5
#define	VEH_TYPE_MEATWAGON	6
#define VEH_TYPE_SEDAN		7
#define VEH_TYPE_WILDCATVAN 8
#define VEH_TYPE_NUMBER		9

THING_INDEX VEH_create(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG yaw,
		SLONG pitch,
		SLONG roll,
		SLONG type,
		UBYTE key,		// The special needed to unlock the car or SPECIAL_NONE if you don't need a key
		UBYTE colour);	// The tint colour of the car.


//
// The main prim used for the body of the given type of vehicle.
//

UWORD get_vehicle_body_prim(SLONG type);


//
// The offset in Y of the positino of the body prim relative to the (x,y,z) of the vehicle.
// This is returned in large 16-bits per mapsquare coordinates.
//

SLONG get_vehicle_body_offset(SLONG type);


//
// What is this vehicle going to run-over?  This function fills out
// the given array with the thing it thinks it is going to run over.
// Returns the number of runover candidates.
//

//
// THIS FUNCTION WORKS FOR ALL BIKES AS WELL AS CARS!
// 

SLONG VEH_find_runover_things(Thing *p_vehicle, UWORD thing_index[], SLONG max_number, SLONG dangle);


// hit a person with a car

extern SLONG GetRunoverHP(Thing* p_car, Thing* p_person, SLONG min_hp);

//
// This collide function will be useful elsewhere.
//

#define VEH_COL_TYPE_BBOX     0
#define VEH_COL_TYPE_CYLINDER 1

typedef struct
{
	UWORD type;
	UWORD ob_index;	// If this collision came from an OB this is its OB_index and position.
	Thing* veh;		// if this collision came from a vehicle or a Balrog, this is a pointer to it
	UWORD mid_x;
	UWORD mid_y;
	UWORD mid_z;
	UWORD height;
	SWORD min_x;
	SWORD max_x;
	SWORD min_z;
	SWORD max_z;
	UWORD radius_or_yaw;

} VEH_Col;

#define VEH_MAX_COL 8

extern VEH_Col VEH_col[VEH_MAX_COL];
extern SLONG   VEH_col_upto;

//
// Finds all the things that will be collided with by VEH_collide_line().
//

void VEH_collide_find_things(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG radius,
		SLONG ignore_thing_index,
		SLONG ignore_prims = FALSE);

//
// Returns the position where one enter/exists a vehicle.
//

void VEH_find_door(
		Thing *p_vehicle,
		SLONG  i_am_a_passenger,
		SLONG *x,
		SLONG *z);



//
// Returns NULL if there isn't a driver...
//

Thing *get_vehicle_driver(Thing *p_vehicle);

//
// initialize crumple zones - calc_prim_info() must have been called
//

void VEH_init_vehinfo();

//
// get vertex -> zone assignment map
//

UBYTE* VEH_get_assignments(ULONG prim);

//
// reinit after teleport
//

void reinit_vehicle(Thing* p_thing);



//
// Getting the position of each of the vehicle wheels.
//

void vehicle_wheel_pos_init(Thing *p_vehicle);
void vehicle_wheel_pos_get(
		SLONG  which,
		SLONG *wx,
		SLONG *wy,
		SLONG *wz);



#endif










