//
// A general clipper.
//

#include "always.h"



//
// The buffer in which we create new points.
// 

#define CLIP_BUFFER_SIZE 2048

UBYTE  CLIP_buffer[CLIP_BUFFER_SIZE];
UBYTE *CLIP_buffer_upto = CLIP_buffer;


//
// Returns the address of a block of memory of the
// given size.
//

inline void *CLIP_malloc(ULONG size)
{
	void *ans;

	ASSERT(size < CLIP_BUFFER_SIZE);

	if (CLIP_buffer_upto + size * 2 > &CLIP_buffer[CLIP_BUFFER_SIZE])
	{
		CLIP_buffer_upto = CLIP_buffer;
	}

	ans               = CLIP_buffer_upto;
	CLIP_buffer_upto += size * 2;

	return ans;
}



void CLIP_do(
		void ***polygon,
		SLONG  *polygon_num_points,
		SLONG   sizeof_polygon_point,
		void  (*interpolate)(void *new_point, void *point1, void *point2, float amount_along_from_1_to_2),
		float (*signed_distance_from_edge)(void *point))
{

	SLONG i;

	SLONG i_p1;
	SLONG i_p2;

	void *p1;
	void *p2;

	float along;

	//
	// The output buffer.
	//

	void **output = (void **) CLIP_malloc(2 * sizeof(void *) * *polygon_num_points);
	SLONG  output_upto = 0;

	//
	// Work out the signed distance of each point from the edge.
	//

	float *distance = (float *) CLIP_malloc(sizeof(float) * *polygon_num_points);

	for (i = 0; i < *polygon_num_points; i++)
	{
		distance[i] = signed_distance_from_edge((*polygon)[i]);
	}

	//
	// Go through the lines of the poly and build up the output polygon.
	//

	for (i = 0; i < *polygon_num_points; i++)
	{
		//
		// The two points of the line.
		//

		i_p1 = i + 0;
		i_p2 = i + 1;

		if (i_p2 == *polygon_num_points)
		{
			i_p2 = 0;
		}

		p1 = (*polygon)[i_p1];
		p2 = (*polygon)[i_p2];

		if (distance[i_p1] >= 0)
		{
			//
			// This point is on the 'good' positive side of the line... add
			// it to the output polygon.
			// 

			output[output_upto++] = p1;

			if (distance[i_p2] >= 0)
			{
				//
				// The other end of the line is also on the right side of the line...
				// no need to create a clipped point.
				//

				continue;
			}
		}
		else
		{
			if (distance[i_p2] < 0)
			{
				//
				// Both points are offscreen, so don't create a clipped point between them.
				//

				continue;
			}
		}

		along = distance[i_p1] / (distance[i_p1] - distance[i_p2]);

		//
		// Create a clipped point 'along' the way from point 1 to point 2.
		//

		output[output_upto] = CLIP_malloc(sizeof_polygon_point);

		interpolate(
			output[output_upto],
			p1,
			p2,
			along);

		output_upto += 1;
	}

	//
	// Return the new polygon.
	//

   *polygon            = output;
   *polygon_num_points = output_upto;
}

