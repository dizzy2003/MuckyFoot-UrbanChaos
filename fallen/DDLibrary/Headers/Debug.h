// Debug.h
// Guy Simmons, 15th November 1997.

#ifndef	DEBUG_H
#define	DEBUG_H

//---------------------------------------------------------------

#ifndef NDEBUG

HANDLE		InitDebugLog(void);
void		FiniDebugLog(void);
void		DebugText(CBYTE *error, ...);
void		dd_error(HRESULT dd_err);
void		d3d_error(HRESULT dd_err);
void		di_error(HRESULT di_err);

#else

#define	dd_error
#define d3d_error
#define di_error

#endif

//---------------------------------------------------------------

#endif
