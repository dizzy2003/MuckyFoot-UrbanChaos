//
// This module expects the following definitions
//
//  PQ_Type	(typedef)
//	PQ_HEAP_MAX_SIZE
//  SLONG PQ_better(PQ_Type *a, PQ_Type *b);
//

#define PQ_BETTER(x,y) (PQ_better(&PQ_heap[x],&PQ_heap[y]))

#define PQ_BEST(x,y,z)						\
	(										\
		(PQ_BETTER(x,y))					\
		?	((PQ_BETTER(x,z)) ? x : z)		\
		:	((PQ_BETTER(y,z)) ? y : z) 		\
	)


#define PQ_SISTER(x)   ((x) ^ 1)
#define PQ_MOTHER(x)   ((x) >> 1)
#define PQ_DAUGHTER(x) ((x) << 1)


// We don't use PQ_heap[0] ... PQ_heap_end points at the current last heap element.

static PQ_Type PQ_heap[PQ_HEAP_MAX_SIZE + 1];
static SLONG   PQ_heap_end;

//----------------------------------------------------------------------------
static void PQ_init(void)
{
	PQ_heap_end = 0;
}


//----------------------------------------------------------------------------
static PQ_Type PQ_best(void)
{
	return PQ_heap[1];
}


//----------------------------------------------------------------------------
static SLONG   PQ_empty (void)
{
	return PQ_heap_end == 0;
}

//----------------------------------------------------------------------------

static void PQ_add(PQ_Type newh)
{
	SLONG	i, best, mum;
	PQ_Type	spare;
	
	if (PQ_heap_end < PQ_HEAP_MAX_SIZE)
	{
		PQ_heap_end++;
	}
	else
	{
		// Can do NOP, in which case the worst looking routes are lost
		// this can lead to some routes NOT being found at all, so
		// we throw up an error for now
		// ERROR("navigate.PQ_add : heap overflow");
	}

	PQ_heap[PQ_heap_end] = newh;

	// Now perculate the new element up the heap.

	i = PQ_heap_end;

	if (i == 1) return;

	if (!(i & 1))
	{
		// The element we have just added is even so it is sisterless.

		if (PQ_BETTER(PQ_MOTHER(i), i)) return;
		spare 			      = PQ_heap[PQ_MOTHER(i)];
		PQ_heap[PQ_MOTHER(i)] = PQ_heap[i];
		PQ_heap[i] 		      = spare;
		i = PQ_MOTHER(i);
	}

	while(i != 1)
	{
		mum = PQ_MOTHER(i);
		best = PQ_BEST(i, PQ_SISTER(i), mum);
		if (best == mum) return;
		i			  = mum;
		spare		  = PQ_heap[i];
		PQ_heap[i]	  = PQ_heap[best];
		PQ_heap[best] = spare;
	}

	return;
}

//----------------------------------------------------------------------------
static void PQ_remove(void)
{
	SLONG i = 1, best;
	PQ_Type spare;

	PQ_heap[1] = PQ_heap[PQ_heap_end];

	PQ_heap_end--;
	if (PQ_heap_end == 0) return;
	
	while(PQ_SISTER(PQ_DAUGHTER(i)) <= PQ_heap_end)
	{
		// We are definitely in a family of three.

		best = PQ_BEST(i, PQ_DAUGHTER(i), PQ_SISTER(PQ_DAUGHTER(i)));
		if (best == i) return;

		spare 	      = PQ_heap[i];
		PQ_heap[i]    = PQ_heap[best];
		PQ_heap[best] = spare;
		
		i = best;
	}

	if (i == PQ_heap_end) return;

	// We are in a group of two, so the daughter must be PQ_heap_end.
	
	if (PQ_BETTER(i,PQ_heap_end)) return;

	spare 		         = PQ_heap[i];
	PQ_heap[i] 	         = PQ_heap[PQ_heap_end];
	PQ_heap[PQ_heap_end] = spare ;

	return;
}



