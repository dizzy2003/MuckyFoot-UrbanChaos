//
// Mathy stuff...
//

#ifndef MATHS_H
#define MATHS_H



//
// Returns TRUE if vector v intersects vector w
// If the two lines touch then that counts as an intersection.
//
// Make sure that the cross product of the two vectors doesn't overflow!
//

SLONG MATHS_seg_intersect(
			SLONG vx1, SLONG vz1, SLONG vx2, SLONG vz2,
			SLONG wx1, SLONG wz1, SLONG wx2, SLONG wz2);




#endif
