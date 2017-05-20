//
// A general clipper.
//

#ifndef _CLIP_
#define _CLIP_


//
// Clips a polygon against an edge. On exit *polygon is an array of points and
// *polygon_num_points gives the length of the array.
//

void CLIP_do(
		void ***polygon,		// Points to an array of (point *)s of length (*polygon_num_points)
		SLONG  *polygon_num_points,
		SLONG   sizeof_polygon_point,
		void  (*interpolate)(void *new_point, void *point1, void *point2, float amount_along_from_1_to_2),
		float   signed_distance_from_edge(void *point));	// +'ve => On the good side of the edge.


#endif
