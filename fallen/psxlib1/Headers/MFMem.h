// MFMem.h
// Guy Simmons, 10th February 1997.

#ifndef _MF_MEM_H_
#define _MF_MEM_H_

BOOL	SetupMemory(void);
void	ResetMemory(void);
void	*MemAlloc(ULONG size);
void	MemFree(void *mem_ptr);

#endif
