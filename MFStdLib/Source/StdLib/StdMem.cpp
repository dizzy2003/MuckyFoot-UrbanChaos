// StdMem.cpp
// Guy Simmons, 18th December 1997

#include	<MFStdLib.h>

#define		INITIAL_HEAP_SIZE		(8*1024*1024)
#define		MAXIMUM_HEAP_SIZE		0

#ifdef DEBUG
// Allows heap debugging - logs interesting info, e.g. for memory leak tracking.
//#define HEAP_DEBUGGING_PLEASE_BOB defined
#endif


HANDLE	MFHeap	=	NULL;

#ifdef HEAP_DEBUGGING_PLEASE_BOB
struct HeapDebugInfo
{
	HeapDebugInfo	*pPrev;
	HeapDebugInfo	*pNext;
	ULONG			ulSize;		// The size of this alloc.
	ULONG			ulSeqNum;	// The malloc number.
};

#define MAX_NUM_DELNEW_TRACES 2000

int iNumNewDelTraces = 0;

struct newdeltrace
{
	void *pvAddr;
	ULONG ulsize;
	ULONG ulSequenceNumber;
} ndtList[MAX_NUM_DELNEW_TRACES];



HeapDebugInfo *pFirst = NULL;
ULONG ulCurrentSequenceNumber = 0;

// Set this to 1 in a debugger to dump the info.
volatile bool bDumpDebug = FALSE;
// Set this to an ID you want to track in the debugger.
volatile ULONG ulSpotted = -1;


void DumpDebug ( void )
{
	bDumpDebug = FALSE;

	TRACE ( "\nMemory debug dump\n" );

	HeapDebugInfo *phdi = pFirst;
	while ( phdi != NULL )
	{
		TRACE ( "ID<0x%x> size<0x%x> \n", phdi->ulSeqNum, phdi->ulSize );

		phdi = phdi->pNext;
	}

	TRACE ( "NewDel debug bump\n" );
	for ( int i = 0; i < iNumNewDelTraces; i++ )
	{
		TRACE ( "ID<0x%x> size<0x%x> \n", ndtList[i].ulSequenceNumber, ndtList[i].ulsize );
	}

	TRACE ( "\n" );

}


#endif

//---------------------------------------------------------------

BOOL	SetupMemory(void)
{
#ifdef HEAP_DEBUGGING_PLEASE_BOB
	pFirst = NULL;
	ulCurrentSequenceNumber = 0;
#endif
	if(MFHeap==NULL)
	{
	   MFHeap	=	HeapCreate(0,INITIAL_HEAP_SIZE,MAXIMUM_HEAP_SIZE);
	}
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
#ifdef HEAP_DEBUGGING_PLEASE_BOB
	pFirst = NULL;
	// Sequence number is not reset.
#endif
}

//---------------------------------------------------------------

void	*MemAlloc(ULONG size)
{
#ifdef HEAP_DEBUGGING_PLEASE_BOB
	if ( bDumpDebug )
	{
		DumpDebug();
	}

	// Set up ulSpotted in a debugger to track interesting items.
	ASSERT ( ulSpotted != ulCurrentSequenceNumber );

	ULONG ulOriginalSize = size;
	size += sizeof ( HeapDebugInfo );
#endif
	size	=	(size+3)&0xfffffffc;
	void *ptr = (void*)HeapAlloc(MFHeap,HEAP_ZERO_MEMORY,size);
	ASSERT ( ptr != NULL );

#ifdef HEAP_DEBUGGING_PLEASE_BOB
	HeapDebugInfo *phdi = (HeapDebugInfo *)ptr;
	ptr = (void*)( (char*)ptr + sizeof ( HeapDebugInfo ) );

	phdi->ulSeqNum = ulCurrentSequenceNumber++;
	phdi->ulSize = ulOriginalSize;
	phdi->pNext = pFirst;
	phdi->pPrev = NULL;
	if ( pFirst != NULL )
	{
		ASSERT ( pFirst->pPrev == NULL );
		pFirst->pPrev = phdi;
	}
	pFirst = phdi;
#endif

	return ptr;
}

//---------------------------------------------------------------

void	MemFree(void *mem_ptr)
{
#ifdef HEAP_DEBUGGING_PLEASE_BOB
	if ( bDumpDebug )
	{
		DumpDebug();
	}

	mem_ptr = (void*)( (char*)mem_ptr - sizeof ( HeapDebugInfo ) );
	HeapDebugInfo *phdi = (HeapDebugInfo *)mem_ptr;

	// Set up ulSpotted in a debugger to track interesting items.
	ASSERT ( ulSpotted != phdi->ulSeqNum );

	if ( phdi->pPrev != NULL )
	{
		ASSERT ( phdi->pPrev->pNext == phdi );
		phdi->pPrev->pNext = phdi->pNext;
	}
	else
	{
		ASSERT ( pFirst == phdi );
		pFirst = phdi->pNext;
	}
	if ( phdi->pNext != NULL )
	{
		ASSERT ( phdi->pNext->pPrev == phdi );
		phdi->pNext->pPrev = phdi->pPrev;
	}
#endif

	HeapFree(MFHeap,0,mem_ptr);
}

//---------------------------------------------------------------

/*
void	MemClear(void *mem_ptr,ULONG size)
{
}
*/

//---------------------------------------------------------------


#ifdef DEBUG
// Support for MFnew and MFdelete
#ifndef HEAP_DEBUGGING_PLEASE_BOB

// Do nothing.
void MFnewTrace ( void *pvAddr, size_t size )
{
}

void MFdeleteTrace ( void *pvAddr )
{
}

#else //#ifndef HEAP_DEBUGGING_PLEASE_BOB

void MFnewTrace ( void *pvAddr, size_t size )
{
	if ( bDumpDebug )
	{
		DumpDebug();
	}

	ASSERT ( iNumNewDelTraces < MAX_NUM_DELNEW_TRACES - 1 );

	ndtList[iNumNewDelTraces].pvAddr = pvAddr;
	ndtList[iNumNewDelTraces].ulsize = (ULONG)size;
	ndtList[iNumNewDelTraces].ulSequenceNumber = ulCurrentSequenceNumber++;
	iNumNewDelTraces++;
}

void MFdeleteTrace ( void *pvAddr )
{
	if ( bDumpDebug )
	{
		DumpDebug();
	}

	// Search for this entry.
	bool bFound = FALSE;
	for ( int i = 0; i < iNumNewDelTraces; i++ )
	{
		if ( ndtList[i].pvAddr == pvAddr )
		{
			iNumNewDelTraces--;
			ndtList[i] = ndtList[iNumNewDelTraces];
			bFound = TRUE;
			break;
		}
	}
	ASSERT ( bFound );
}

#endif //#else //#ifndef HEAP_DEBUGGING_PLEASE_BOB


#endif





