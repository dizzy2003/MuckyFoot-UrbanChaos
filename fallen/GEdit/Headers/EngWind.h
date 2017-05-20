//	EngWind.cpp
//	Guy Simmons, 27th July 1998.

#ifndef	ENGWIND_H
#define	ENGWIND_H

//---------------------------------------------------------------

BOOL		init_ewind(void);
void		fini_ewind(void);
BOOL		open_map(MDICREATESTRUCT *mdi_create);
void		process_ewind(void);

//---------------------------------------------------------------

#endif
