//	TrapSetup.h
//	Matthew Rosenfeld, 15th October 1998.

#ifndef	_TRAPSETUP_H_
#define	_TRAPSETUP_H_

#include	"Mission.h"


//---------------------------------------------------------------

void	do_trap_setup(EventPoint *ep);
CBYTE	*get_trap_message(EventPoint *ep, CBYTE *msg);

//---------------------------------------------------------------

#endif
