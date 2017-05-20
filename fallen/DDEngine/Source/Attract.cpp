// Attract.cpp
// Guy Simmons, 20th November 1997.

#include	"Engine.h"
#include	"font.h"

//---------------------------------------------------------------

void	engine_attract(void)
{
	static	count=0;
	count++;
	if (the_display.screen_lock())
	{
		CBYTE	str[100];
		sprintf(str,"%d",count);
		FONT_draw_coloured_text(220,5,128,128,128,str);
		the_display.screen_unlock();
	}

	FLIP(NULL,DDFLIP_WAIT);
	return;
}

//---------------------------------------------------------------

extern Camera		test_view;
void	game_engine(Camera *the_view);

void	engine_win_level(void)
{
}

//---------------------------------------------------------------

void	engine_lose_level(void)
{
}

//---------------------------------------------------------------
