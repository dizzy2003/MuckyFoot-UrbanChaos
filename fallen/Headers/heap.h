//
// A memory heap with defragmentation. It is designed to
// given largish blocks of memory out...
//

#ifndef HEAP_H
#define HEAP_H

//
// Initialises the heap.
// Returns a block of memory.
// Gives back an unused block of memory.
//

void  HEAP_init(void);
void *HEAP_get(SLONG num_bytes);
void  HEAP_give(void *, SLONG num_bytes);


//
// A chunk of useful memory for you to do with as you please.
//

#define HEAP_PAD_SIZE (1024 * 4)

extern UBYTE HEAP_pad[HEAP_PAD_SIZE];




#endif
