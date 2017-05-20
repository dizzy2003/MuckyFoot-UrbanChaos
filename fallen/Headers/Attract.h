// Attract.h
// Guy Simmons, 20th November 1997

#ifndef	ATTRACT_H
#define	ATTRACT_H

//---------------------------------------------------------------

void	game_attract_mode(void);
void	level_won(void);
void	level_lost(void);


//
// The loading screen counter to completion.
//

void ATTRACT_loadscreen_init(void);
void ATTRACT_loadscreen_draw(SLONG completion);	// completion is in 8-bit fixed point from 0 to 256.



//---------------------------------------------------------------

#endif
