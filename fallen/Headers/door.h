//
// Doors
//

#ifndef _DOOR_
#define _DOOR_



//
// Doors in the process of opening or closing.
//

typedef struct
{
	UWORD facet;	// NULL => Unused.

} DOOR_Door;

#define DOOR_MAX_DOORS 4

extern DOOR_Door *DOOR_door;//[DOOR_MAX_DOORS];



//
// Opens\shuts a door near to the given position.
//

void DOOR_open(SLONG world_x, SLONG world_z);
void DOOR_shut(SLONG world_x, SLONG world_z);



//
// Processes the doors.
//

void DOOR_process(void);




#endif
