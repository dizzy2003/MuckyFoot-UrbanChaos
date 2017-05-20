//
// Dog leads. One end is attached somewhere, the other end it attached to a Thing and
// restricts its movements.
//

#ifndef _LEAD_
#define _LEAD_

//
// The lead structure. The first point of the lead is (attach_x,attach_y,attach_z).
//

typedef struct
{
	UBYTE p_num;		// Number of points.
	UBYTE p_index;		// Index into the LEAD_point array.
	UWORD attach_x;		// Where the lead is tied to
	UWORD attach_y;
	UWORD attach_z;
	UWORD attach_thing;	// The thing on the other end of the leash.

} LEAD_Lead;

#define LEAD_MAX_LEADS 32

extern LEAD_Lead LEAD_lead[LEAD_MAX_LEADS];
extern SLONG     LEAD_lead_upto;

//
// The points.
//

typedef struct
{
	SLONG x;
	SLONG y;
	SLONG z;
	SLONG dx;
	SLONG dy;
	SLONG dz;

} LEAD_Point;

#define LEAD_MAX_POINTS 256

extern LEAD_Point LEAD_point[LEAD_MAX_POINTS];
extern SLONG      LEAD_point_upto;



//
// Removes all the leads.
//

void LEAD_init(void);


//
// Creates a lead of the given length.
//

#define LEAD_LEN_SHORT	1
#define LEAD_LEN_MEDIUM	2
#define LEAD_LEN_LONG	3

void LEAD_create(
		SLONG len,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z);


//
// Attaches all the leads to nearby lamposts and dogs.
//

void LEAD_attach(void);


//
// Processes all the leads.
//

void LEAD_process(void);



#endif
