// Mem.cpp
// Guy Simmons, 10th February 1997.


#include	<MFStdLib.h>
#include	"libapi.h"

#include	"myheap.h"
#include	"psxeng.h"

#if 0
#else
void *dud_malloc_init(void* base,void* end);
void *dud_malloc(SLONG size);
void dud_free(void *p);
#endif

ULONG	heap_index=0;
SLONG total_mem_size;
SLONG bucket_mem_size;

//---------------------------------------------------------------

BOOL	SetupMemory(void)
{

#ifdef FS_ISO9660
#ifndef VERSION_DEMO
	total_mem_size=0x80200000-(SLONG)my_heap;
#else
	total_mem_size=0x801fff00-(SLONG)my_heap;
#endif
#else
	total_mem_size=0x80285b08-(SLONG)my_heap;
#endif

	InitHeap3((void*)&my_heap[0],total_mem_size);
	printf("Allocated Memory: %d\n",total_mem_size);

	return(TRUE);

}

//---------------------------------------------------------------

void	ResetMemory(void)
{
}

//---------------------------------------------------------------

void	*MemAlloc(ULONG size)
{
	/*
	SLONG	pos;
	pos=heap_index;
	if(heap_index+size<INITIAL_HEAP_SIZE)
		heap_index+=size;
	*/
#if 1
	size+=8;

	return((void*)malloc3(size));
#else
	return((void*)dud_malloc(size));
#endif
}

//---------------------------------------------------------------

void	MemFree(void *mem_ptr)
{
#if 1
	free3(mem_ptr);
#else
	dud_free(mem_ptr);
#endif
}

//---------------------------------------------------------------

void	ZeroMemory(void *mem_ptr,ULONG size)
{
	memset((UBYTE*)mem_ptr,0,size);

}


//---------------------------------------------------------------

