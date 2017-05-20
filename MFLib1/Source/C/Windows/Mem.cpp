// Mem.cpp
// Guy Simmons, 10th February 1997.


#include	<MFHeader.h>

#define		INITIAL_HEAP_SIZE		(18*1024*1024)
#define		MAXIMUM_HEAP_SIZE		(24*1024*1024)

HANDLE	MFHeap	=	NULL;

//---------------------------------------------------------------

BOOL	SetupMemory(void)
{
	if(MFHeap==NULL)
	{
	   MFHeap	=	HeapCreate(0,INITIAL_HEAP_SIZE,MAXIMUM_HEAP_SIZE);
	}
	ERROR_MSG(MFHeap,"Can't setup memory.")
	if(MFHeap)
		return	TRUE;
	else
		return	FALSE;
}

//---------------------------------------------------------------

void	ResetMemory(void)
{
	if(MFHeap)
	{
		HeapDestroy(MFHeap);
		MFHeap	=	NULL;
	}
}

//---------------------------------------------------------------

void	*MemAlloc(ULONG size)
{
	size	=	(size+3)&0xfffffffc;
	return (void*)HeapAlloc(MFHeap,HEAP_ZERO_MEMORY,size);
}

//---------------------------------------------------------------

void	MemFree(void *mem_ptr)
{
	HeapFree(MFHeap,0,mem_ptr);
}

//---------------------------------------------------------------

void	MemClear(void *mem_ptr,ULONG size)
{
/*
	ULONG	c0;

	for(c0=0;c0<size;c0++)
		*(mem_ptr++)	=	0;
*/
}


//---------------------------------------------------------------

