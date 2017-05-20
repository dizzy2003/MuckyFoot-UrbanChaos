//
// The new bikes are called dikes.
//

#ifndef TARGET_DC


#ifndef _DIKE_
#define _DIKE_



typedef struct
{
	//
	// The front wheel
	//

	SLONG fx;
	SLONG fy;
	SLONG fz;
	SLONG fdx;
	SLONG fdy;
	SLONG fdz;

	//
	// The back wheel
	//

	SLONG bx;
	SLONG by;
	SLONG bz;
	SLONG bdx;
	SLONG bdy;
	SLONG bdz;

	//
	// Front suspension.
	//

	SLONG fsy;
	SLONG fsdy;

	//
	// Back suspension.
	//

	SLONG bsy;
	SLONG bsdy;
	
	//
	// Bike control
	//

	#define DIKE_CONTROL_ACCEL   (1 << 0)
	#define DIKE_CONTROL_LEFT    (1 << 1)
	#define DIKE_CONTROL_RIGHT   (1 << 2)
	#define DIKE_CONTROL_BRAKE   (1 << 3)
	#define DIKE_CONTORL_WHEELIE (1 << 4)

	UBYTE control;
	SBYTE steer;
	SBYTE power;

	//
	// Flags
	//

	#define DIKE_FLAG_ON_GROUND_FRONT (1 << 0)
	#define DIKE_FLAG_ON_GROUND_BACK  (1 << 1)

	UBYTE flag;

	//
	// The angle of the bike.
	//

	UWORD yaw;
	UWORD pitch;

} DIKE_Dike;

#define DIKE_MAX_DIKES 8

extern DIKE_Dike DIKE_dike[DIKE_MAX_DIKES];
extern SLONG     DIKE_dike_upto;


//
// Initialises the dikes.
//

void DIKE_init(void);



//
// Creates a new dike.
//

DIKE_Dike *DIKE_create(
			SLONG x,
			SLONG z,
			SLONG yaw);


//
// Processes a dike. Write directly into the 'control' field to
// move the dike.
//

void DIKE_process(DIKE_Dike *dd);



//
// A line-drawn representation of the dike.
//

void DIKE_draw(DIKE_Dike *dd);



#endif

#endif //#ifndef TARGET_DC
