//
// Motobikes.
//

#ifndef _BIKE_
#define _BIKE_

#ifdef BIKE

//
// The different modes the bikes can be in.
//

#define BIKE_MODE_PARKED		0
#define BIKE_MODE_MOUNTING		1		// Somebody is getting on this bike.
#define BIKE_MODE_DRIVING		2		// Somebody is driving this bike.
#define BIKE_MODE_DISMOUNTING	3		// Somebody is getting off this bike.


//
// So we dont have to have to BIKE Genus in the header file.
//

typedef struct 
{
	UWORD yaw;
	UWORD pitch;

	UBYTE flag;
	UBYTE mode;
	SBYTE accel;
	SBYTE steer;

	SLONG back_x;
	SLONG back_y;
	SLONG back_z;

	SLONG back_dx;
	SLONG back_dy;
	SLONG back_dz;

	SLONG front_x;
	SLONG front_y;
	SLONG front_z;

	SLONG front_dy;

	//
	// Suspension on the wheels.
	//

	SLONG wheel_y_back;
	SLONG wheel_y_front;

	SLONG wheel_dy_back;
	SLONG wheel_dy_front;

	//
	// The wheel rotations.
	//

	UWORD wheel_rot_front;
	UWORD wheel_rot_back;

	// Tyre tracks

	UWORD tyrelast;
	UWORD ribbon;
	UWORD ribbon2;

	//
	// Who is driving the bike.
	//

	UWORD driver;

	SWORD SlideTimer;
	UBYTE dirt;				// If the bike is kicking up dirt...
	UBYTE padding;

} BIKE_Bike;


//struct bike_bike;
//typedef struct bike_bike BIKE_Bike;
typedef BIKE_Bike *BikePtr;

#define BIKE_MAX_BIKES 2 //8

#define TO_BIKE(t)				(&BIKE_bike[t])
#define	BIKE_NUMBER(t)			(COMMON_INDEX)(t-TO_BIKE(0))


extern	BIKE_Bike *BIKE_bike; //[BIKE_MAX_BIKES];


//
// Initialises all the bike structures.
//

void BIKE_init(void);


//
// Creates a new bike. Bikes are always on the ground.
//

UWORD BIKE_create(
		SLONG x,
		SLONG z,
		SLONG yaw);

//
// Returns the THING_INDEX of a bike the person can get onto.
//

SLONG BIKE_person_can_mount(Thing *p_person);


//
// Tells the bike that somebody is getting on it.  It starts animating.
//

void BIKE_set_mounting(Thing *p_bike, Thing *p_person);

//
// Tells the bike that nobody is on it any more.
// 

void BIKE_set_parked(Thing *p_bike);


//
// How to steer a bike.
//

typedef struct
{
	SBYTE steer;
	SBYTE accel;
	SBYTE brake;
	UBYTE wheelie;

} BIKE_Control;

BIKE_Control BIKE_control_get(Thing *p_bike);
void         BIKE_control_set(Thing *p_bike, BIKE_Control bc);


//
// Returns the roll of the bike.
//

SLONG BIKE_get_roll(Thing *p_bike);


//
// Returns the speed of the bike.
//

SLONG BIKE_get_speed(Thing *p_bike);


//
// Bike drawing info
// 

typedef struct
{
	UWORD yaw;
	UWORD pitch;
	UWORD roll;
	UWORD steer;

	UWORD front_x;
	SWORD front_y;
	UWORD front_z;
	UWORD front_rot;

	UWORD back_x;
	SWORD back_y;
	UWORD back_z;
	UWORD back_rot;

} BIKE_Drawinfo;

BIKE_Drawinfo BIKE_get_drawinfo(Thing *p_bike);


#endif

#endif
