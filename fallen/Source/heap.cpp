//
// A memory heap with defragmentation.
//

#include "game.h"
#include <MFStdLib.h>
#include "heap.h"

//
// The scratch pad.
//

UBYTE HEAP_pad[HEAP_PAD_SIZE];


//
// The heap.
// 

#if defined(PSX)
#define HEAP_SIZE (1024 * 16)
#elif defined(TARGET_DC)
#define HEAP_SIZE (1024 * 64)
#else
#define HEAP_SIZE (1024 * 128)
#endif

UBYTE HEAP_heap[HEAP_SIZE];

//
// The free list is sorted by size.
//

typedef struct heap_free
{
	UBYTE            *start;
	UBYTE            *end;
	SLONG             size;
	struct heap_free *next;

} HEAP_Free;

HEAP_Free *HEAP_free;


//
// All memory blocks must be multiples of this. It is
// also the minimum size that can be allocated.
//

#define HEAP_QUANTISE 16



//
// Error checks the heap- makes sure it is all okay.
//

void HEAP_check()
{
	HEAP_Free *hf;

	for (hf = HEAP_free; hf; hf = hf->next)
	{
		ASSERT(hf->size <= HEAP_SIZE);
		ASSERT(hf->start + hf->size == hf->end);
	}
}



void HEAP_init()
{
	//
	// Easy...
	//

	HEAP_free = (HEAP_Free *) HEAP_heap;

	HEAP_free->start = (UBYTE *) &HEAP_heap[0];
	HEAP_free->end   = (UBYTE *) &HEAP_heap[HEAP_SIZE];
	HEAP_free->size  = HEAP_SIZE;
	HEAP_free->next  = NULL;

	HEAP_check();
}

void HEAP_add_to_free(HEAP_Free *bit)
{
	HEAP_Free  *next;
	HEAP_Free **prev;

	#ifndef NDEBUG
	HEAP_check();
	#endif

	//
	// Go through the free list looking for free blocks
	// of memory we can merge with bit.
	//

  start_again_with_a_bigger_bit:;

	#ifndef NDEBUG
	HEAP_check();
	#endif

	ASSERT(bit->size <= HEAP_SIZE);
	ASSERT(bit->start + bit->size == bit->end);

	prev = &HEAP_free;
	next =  HEAP_free;

	while(next)
	{
		//
		// Can we merge bit and next?
		//

		if (bit->end == next->start)
		{
			//
			// Take next out of the free list.
			//

		   *prev = next->next;
		 
		    //
		    // Lengthen our bit.
		    //
		   
			ASSERT(next->size <= HEAP_SIZE);

			bit->end    = next->end;
			bit->size  += next->size;

			goto start_again_with_a_bigger_bit;
		}
		else
		if (next->end == bit->start)
		{
			//
			// Take next out of the free list.
			//

		   *prev = next->next;

		    //
		    // Lengthen 'next'.
		    //

			ASSERT(bit->size <= HEAP_SIZE);

		    next->end   = bit->end;
			next->size += bit->size; 

			ASSERT(next->size <= HEAP_SIZE);

			//
			// Now add 'next', not 'bit'.
			//

			bit = next;

			goto start_again_with_a_bigger_bit;
		}

//		ASSERT((ULONG(next->next) & 0xff000000) == 0);

		prev = &next->next;
		next =  next->next;
	}

	#ifndef NDEBUG
	HEAP_check();
	#endif

	//
	// Now add 'bit' in its correct position ordered
	// by size.
	//

	prev = &HEAP_free;
	next =  HEAP_free;

	while(1)
	{
		if (next == NULL || next->size <= bit->size)
		{
			//
			// This is where we insert our 'bit'.
			//

//			ASSERT((ULONG(next) & 0xff000000) == 0);

		   *prev      = bit;
		    bit->next = next;

			break;
		}

		prev = &next->next;
		next =  next->next;
	}

	#ifndef NDEBUG
	HEAP_check();
	#endif

	return;
}



void *HEAP_get(SLONG size)
{
	void      *ans;
	HEAP_Free  bit;
	HEAP_Free *onheap;

	#ifndef NDEBUG
	HEAP_check();
	#endif

	if (HEAP_free == NULL)
	{
		//
		// No more free memory!
		//

		return NULL;
	}

	//
	// Round up the size to the nearby 16-byte boundary.
	//

	size +=  (HEAP_QUANTISE - 1);
	size &= ~(HEAP_QUANTISE - 1);

	ASSERT(WITHIN((UBYTE *) HEAP_free, &HEAP_heap[0], &HEAP_heap[HEAP_SIZE - sizeof(HEAP_Free)]));

	//
	// Always take memory from the biggest block.
	//

	if (HEAP_free->size < size)
	{
		//
		// No large-enough block.
		//

		return NULL;
	}

	//
	// The first block is big enough to use.
	// Build the left-over chunk.
	//

	bit.start = HEAP_free->start + size;
	bit.end   = HEAP_free->end;
	bit.size  = HEAP_free->size  - size;
	bit.next  = NULL;

	ASSERT(bit.start + bit.size == bit.end);

	//
	// Remember the answer.
	//

	ans = HEAP_free;

	//
	// Take out the first block from the free list.
	//

	HEAP_free = HEAP_free->next;

	#ifndef NDEBUG
	HEAP_check();
	#endif

	if (bit.size == 0)
	{
		//
		// This makes matters easier.
		//
	
		return ans;
	}
	else
	{
		//
		// There should always be enough room...
		//

		ASSERT(bit.size >= sizeof(HEAP_Free));

		#ifndef NDEBUG
		HEAP_check();
		#endif

		//
		// Put the new bit on the heap.
		//

		onheap = (HEAP_Free *) bit.start;
	   *onheap = bit;

		//
		// Add the bit to the free list.
		//
  
		HEAP_add_to_free(onheap);

		#ifndef NDEBUG
		HEAP_check();
		#endif

		return ans;
	}
}


SLONG	HEAP_max_free(void)
{
	if(HEAP_free)
	{
		return(HEAP_free->size);
	}
	else
	{
		return(0);
	}

}

//
// Gives back an unused block of memory.
//

void HEAP_give(void *mem, SLONG num_bytes)
{
	HEAP_Free *onheap = (HEAP_Free *) mem;

	#ifndef NDEBUG
	HEAP_check();
	#endif

	//
	// Valid memory block?
	//

	num_bytes +=  (HEAP_QUANTISE - 1);
	num_bytes &= ~(HEAP_QUANTISE - 1);

	ASSERT(WITHIN((UBYTE *) onheap, &HEAP_heap[0], &HEAP_heap[HEAP_SIZE - sizeof(HEAP_Free)]));

	//
	// Add the header.
	//

//	ASSERT((((ULONG)mem) & 0xff000000) == 0);
	ASSERT(num_bytes <= HEAP_SIZE);

	onheap->start = (UBYTE *) mem;
	onheap->end   = onheap->start + num_bytes;
	onheap->size  = num_bytes;
	onheap->next  = NULL;

	//
	// Add it to the free list.
	//

	HEAP_add_to_free(onheap);

	#ifndef NDEBUG
	HEAP_check();
	#endif
}
