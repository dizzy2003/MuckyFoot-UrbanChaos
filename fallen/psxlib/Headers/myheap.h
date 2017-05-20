#ifndef _MYHEAP_H
#define _MYHEAP_H

#ifndef	FS_ISO9660
#define		INITIAL_HEAP_SIZE		(1536*1024)
//#define		INITIAL_HEAP_SIZE		((990*1024)+768)
#else
#define		INITIAL_HEAP_SIZE		((984*1024)+512)
#endif
#define		MAXIMUM_HEAP_SIZE		(16*1024*1024)

extern UBYTE	my_heap[40*1024];
#endif