//
// Tells you where to put staircases in a building.
//

#ifndef STAIR_H
#define STAIR_H


#include "id.h"


//
// Clear all the current building info.
//

void STAIR_init(void);

//
// Tell it about the bounding box of the new building. The bounding box
// is that of the walls, so for a 1-square building the bounding box
// should be (0,0)-(1,1).
//

void STAIR_set_bounding_box(UBYTE x1, UBYTE z1, UBYTE x2, UBYTE z2);

//
// Tell it about all the storeys of the building.
// When adding a wall, if (opposite) then the stair module tries to
// put staircases opposite that wall but not touching it. It only
// uses the last opposite wall it is given.
//
// STAIR_storey_finish() returns FALSE if the walls added for the storey
// are invalid and you can't create stairs for this building.
//

void  STAIR_storey_new   (SLONG handle, UBYTE height);				// 0 => Ground floor, 1 => First floor
void  STAIR_storey_wall  (UBYTE x1, UBYTE z1, UBYTE x2, UBYTE z2, SLONG opposite);
SLONG STAIR_storey_finish(void);

//
// Calculates where the stairs should go for each storey.
//

void STAIR_calculate(UWORD seed);

//
// Returns the stairs for the given floor.  The 'id' of the ID_stair is
// not filled in.  Returns FALSE if it couldn't find a storey with
// the given handle.
//

SLONG STAIR_get(SLONG handle, ID_Stair **stair, SLONG *num_stairs);



#endif
