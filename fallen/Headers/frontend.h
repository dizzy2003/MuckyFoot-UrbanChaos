//
// frontend.h
//
// matthew rosenfeld 8 july 99
//
// this is our new front end thingy to replace the hideous startscr.cpp
//

#ifndef _FRONTEND_H_
#define _FRONTEND_H_

#include "MFStdLib.h"

void	FRONTEND_init ( bool bGoToTitleScreen = FALSE );
SBYTE	FRONTEND_loop();
void	FRONTEND_level_won();
void	FRONTEND_level_lost();

#ifdef TARGET_DC
// Unload frontend gubbins to save memory.
void FRONTEND_unload ( void );
#endif


extern UBYTE	IsEnglish;


#endif